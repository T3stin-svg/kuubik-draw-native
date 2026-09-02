/****************************************************************************
**
** Kuubik Draw compact ribbon.
**
** This file is part of Kuubik Draw, a GPLv2 fork of LibreCAD.
** It may be distributed and/or modified under the terms of the GNU General
** Public License version 2 as published by the Free Software Foundation.
**
****************************************************************************/

#include "kuubikribbon.h"
#include "kuubikiconregistry.h"
#include "kuubiktheme.h"

#include <QAction>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QMenu>
#include <QResizeEvent>
#include <QSizePolicy>
#include <QStyle>
#include <QTabWidget>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>

#include <algorithm>

namespace {
QLabel* makeLabel(const QString& text, const char* objectName, QWidget* parent)
{
    auto* label = new QLabel(text, parent);
    label->setObjectName(QString::fromLatin1(objectName));
    return label;
}
}

KuubikRibbon::KuubikRibbon(const QMap<QString, QAction*>& actionMap,
                           QToolBar* nativePenToolbar,
                           QToolBar* nativeOptionToolbar,
                           QWidget* parent)
    : QWidget(parent), actions(actionMap), penToolbar(nativePenToolbar), optionToolbar(nativeOptionToolbar)
{
    setObjectName("kuubikRibbon");
    // The outer toolbar must be able to assign a narrow width before the
    // resize-driven panel collapse can reduce the ribbon's own size hint.
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    setMinimumHeight(KuubikTheme::ribbonMinimumHeight());
    setMaximumHeight(KuubikTheme::ribbonMaximumHeight());

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    auto* quickBar = new QFrame(this);
    quickBar->setObjectName("kuubikQuickAccess");
    quickBar->setFixedHeight(28);
    auto* quickLayout = new QHBoxLayout(quickBar);
    quickLayout->setContentsMargins(6, 2, 8, 2);
    quickLayout->setSpacing(2);

    auto* applicationButton = new QToolButton(quickBar);
    applicationButton->setObjectName("kuubikApplicationButton");
    applicationButton->setText(tr("Application"));
    applicationButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
    applicationButton->setPopupMode(QToolButton::InstantPopup);
    applicationMenu = new QMenu(applicationButton);
    for (const QString& key : {QStringLiteral("FileNew"), QStringLiteral("FileOpen"),
                               QStringLiteral("FileSave"), QStringLiteral("FileSaveAs"),
                               QStringLiteral("FileExport"), QStringLiteral("FilePrintPreview"),
                               QStringLiteral("FilePrintPDF"), QStringLiteral("FilePrint")}) {
        if (QAction* action = actions.value(key, nullptr)) {
            applicationMenu->addAction(action);
        }
    }
    applicationButton->setMenu(applicationMenu);
    quickLayout->addWidget(applicationButton);

    for (const QString& key : {QStringLiteral("FileNew"), QStringLiteral("FileOpen"),
                               QStringLiteral("FileSave"), QStringLiteral("EditUndo"),
                               QStringLiteral("EditRedo"), QStringLiteral("FilePrint")}) {
        if (QToolButton* button = createActionButton(key, quickBar, ItemSize::Small, true)) {
            button->setObjectName("kuubikQuickButton");
            quickLayout->addWidget(button);
        }
    }
    quickLayout->addSpacing(8);
    quickLayout->addWidget(makeLabel(tr("2D Drafting & Annotation"), "kuubikWorkspaceLabel", quickBar));
    quickLayout->addStretch(1);
    optionToolbarHost = new QFrame(quickBar);
    optionToolbarHost->setObjectName("kuubikOptionToolbarHost");
    optionToolbarLayout = new QGridLayout(optionToolbarHost);
    optionToolbarLayout->setContentsMargins(0, 0, 0, 0);
    optionToolbarLayout->setSpacing(0);
    quickLayout->addWidget(optionToolbarHost, 1);
    quickLayout->addWidget(makeLabel(tr("Kuubik Draw · GPL"), "kuubikProductBadge", quickBar));
    root->addWidget(quickBar);

    tabs = new QTabWidget(this);
    tabs->setObjectName("kuubikRibbonTabs");
    tabs->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
    tabs->setDocumentMode(true);
    tabs->setMovable(false);
    tabs->setUsesScrollButtons(true);

    const QList<TabSpec> specs {
        {tr("Home"), {
            {tr("Draw"), {{"DrawLine", ItemSize::Large}, {"DrawPolyline", ItemSize::Large}, {"DrawLineRectangle", ItemSize::Small}, {"DrawCircle", ItemSize::Medium}, {"DrawArc", ItemSize::Small}, {"DrawHatch", ItemSize::Small}}, 1},
            {tr("Modify"), {{"ModifyMove", ItemSize::Large}, {"ModifyDuplicate", ItemSize::Large}, {"ModifyTrim", ItemSize::Large}, {"ModifyTrim2", ItemSize::Small}, {"ModifyCut", ItemSize::Small}, {"ModifyOffset", ItemSize::Medium}, {"ModifyRotate", ItemSize::Medium}, {"ModifyMirror", ItemSize::Small}, {"ModifyScale", ItemSize::Small}, {"ModifyRound", ItemSize::Small}, {"ModifyDeleteQuick", ItemSize::Small}}, 2},
            {tr("Annotation"), {{"DrawText", ItemSize::Medium}, {"DrawMText", ItemSize::Medium}, {"DimLinear", ItemSize::Small}, {"DimAligned", ItemSize::Small}, {"DimLinearHor", ItemSize::Small}, {"DimLinearVer", ItemSize::Small}, {"DimRadial", ItemSize::Small}, {"DimDiametric", ItemSize::Small}, {"DimAngular", ItemSize::Small}, {"DimLeader", ItemSize::Small}}, 200},
            {tr("Layers"), {{"LayersAdd", ItemSize::Medium}, {"LayersEdit", ItemSize::Medium}, {"LayersToggleView", ItemSize::Small}, {"LayersToggleLock", ItemSize::Small}}, 600},
            {tr("Block"), {{"BlocksInsert", ItemSize::Large}, {"BlocksCreate", ItemSize::Medium}, {"BlocksEdit", ItemSize::Small}, {"BlocksExplode", ItemSize::Small}, {"BlocksImport", ItemSize::Small}}, 300},
            {tr("Properties"), {{"ModifyEntity", ItemSize::Medium}}, 100},
            {tr("Utilities"), {{"InfoDist", ItemSize::Small}, {"InfoAngle", ItemSize::Small}, {"InfoArea", ItemSize::Small}, {"InfoTotalLength", ItemSize::Small}}, 400},
            {tr("Clipboard"), {{"EditCut", ItemSize::Small}, {"EditCopy", ItemSize::Small}, {"EditPaste", ItemSize::Small}}, 500}
        }},
        {tr("Insert"), {{tr("Blocks"), {{"BlocksInsert", ItemSize::Large}, {"BlocksCreate", ItemSize::Medium}, {"BlocksEdit", ItemSize::Small}, {"BlocksExplode", ItemSize::Small}, {"BlocksImport", ItemSize::Small}}, 200}}},
        {tr("Annotate"), {
            {tr("Text"), {{"DrawText", ItemSize::Large}, {"DrawMText", ItemSize::Medium}}, -1},
            {tr("Dimensions"), {{"DimLinear", ItemSize::Large}, {"DimAligned", ItemSize::Medium}, {"DimLinearHor", ItemSize::Small}, {"DimLinearVer", ItemSize::Small}, {"DimRadial", ItemSize::Small}, {"DimDiametric", ItemSize::Small}, {"DimAngular", ItemSize::Small}, {"DimLeader", ItemSize::Small}}, 100},
            {tr("Lines"), {{"DrawLine", ItemSize::Large}, {"DrawLineAngle", ItemSize::Small}, {"DrawLineHorizontal", ItemSize::Small}, {"DrawLineVertical", ItemSize::Small}, {"DrawLineParallel", ItemSize::Small}, {"DrawLineOrthogonal", ItemSize::Small}}, 200},
            {tr("Cuts & Details"), {{"DrawHatch", ItemSize::Large}, {"ModifyOffset", ItemSize::Medium}, {"ModifyTrim", ItemSize::Medium}, {"ModifyCut", ItemSize::Small}, {"DrawText", ItemSize::Small}, {"DrawMText", ItemSize::Small}, {"DimLinear", ItemSize::Small}, {"DimLeader", ItemSize::Small}, {"BlocksInsert", ItemSize::Small}}, 300}
        }},
        {tr("View"), {
            {tr("Navigate"), {{"ZoomAuto", ItemSize::Large}, {"ZoomIn", ItemSize::Medium}, {"ZoomOut", ItemSize::Medium}, {"ZoomPrevious", ItemSize::Small}, {"ZoomWindow", ItemSize::Small}, {"ZoomPan", ItemSize::Small}}, -1},
            {tr("Display"), {{"ViewGrid", ItemSize::Medium}}, 100},
            {tr("Snaps"), {{"SnapEnd", ItemSize::Small}, {"SnapMiddle", ItemSize::Small}, {"SnapCenter", ItemSize::Small}, {"SnapIntersection", ItemSize::Small}, {"RestrictOrthogonal", ItemSize::Small}}, 200}
        }},
        {tr("Manage"), {
            {tr("Layers"), {{"LayersAdd", ItemSize::Large}, {"LayersEdit", ItemSize::Medium}, {"LayersToggleView", ItemSize::Small}, {"LayersToggleLock", ItemSize::Small}}, -1},
            {tr("Blocks"), {{"BlocksCreate", ItemSize::Medium}, {"BlocksEdit", ItemSize::Small}, {"BlocksExplode", ItemSize::Small}}, 100},
            {tr("Properties"), {{"ModifyEntity", ItemSize::Medium}}, 200}
        }},
        {tr("Output"), {
            {tr("File"), {{"FileSaveAs", ItemSize::Medium}, {"FileExport", ItemSize::Medium}}, -1},
            {tr("Plot"), {{"FilePrintPreview", ItemSize::Medium}, {"FilePrintPDF", ItemSize::Medium}, {"FilePrint", ItemSize::Small}}, 100}
        }}
    };
    for (const TabSpec& spec : specs) {
        tabs->addTab(createPage(spec), spec.title);
    }
    connect(tabs, &QTabWidget::currentChanged, this, [this](int) {
        QTimer::singleShot(0, this, [this] { updateCollapsedPanels(); });
    });

    auto* homePage = tabs->widget(0);
    auto* homeLayout = qobject_cast<QHBoxLayout*>(homePage->layout());
    currentLayerHost = createCurrentLayerHost(homePage);
    homeLayout->insertWidget(4, currentLayerHost);
    penToolbarHost = createEmbeddedToolbarGroup(tr("Pen"), penToolbar, homePage);
    penToolbarLayout = qobject_cast<QGridLayout*>(penToolbarHost->layout());
    homeLayout->insertWidget(5, penToolbarHost);
    root->addWidget(tabs, 1);
    QTimer::singleShot(0, this, [this] { updateCollapsedPanels(); });
}

QToolButton* KuubikRibbon::createActionButton(const QString& key, QWidget* parent, ItemSize size, bool iconOnly)
{
    QAction* action = actions.value(key, nullptr);
    if (action == nullptr) {
        if (!missingKeys.contains(key)) missingKeys.append(key);
        return nullptr;
    }
    auto* button = new QToolButton(parent);
    button->setObjectName("kuubikRibbonButton");
    button->setProperty("kuubikActionKey", key);
    button->setProperty("kuubikItemSize", static_cast<int>(size));
    button->setDefaultAction(action);
    const QIcon icon = KuubikIconRegistry::iconForAction(key);
    if (!icon.isNull()) button->setIcon(icon);
    button->setAutoRaise(false);
    if (iconOnly) {
        button->setIconSize(QSize(16, 16));
        button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    } else if (size == ItemSize::Large) {
        button->setIconSize(QSize(32, 32));
        button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    } else {
        const int pixels = size == ItemSize::Medium ? 20 : 16;
        button->setIconSize(QSize(pixels, pixels));
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    }
    button->setAccessibleName(action->text().remove('&'));
    if (!actionButtons.contains(key)) actionButtons.insert(key, button);
    return button;
}

QFrame* KuubikRibbon::createActionGroup(const PanelSpec& spec, QWidget* parent, QWidget* page)
{
    auto* frame = new QFrame(parent);
    frame->setObjectName("kuubikRibbonGroup");
    frame->setProperty("kuubikCollapsePriority", spec.collapsePriority);
    frame->setProperty("kuubikPanelTitle", spec.title);
    QStringList actionKeys;
    for (const ItemSpec& item : spec.items) actionKeys.append(item.actionKey);
    frame->setProperty("kuubikActionKeys", actionKeys);
    frame->setProperty("kuubikCollapsed", false);
    auto* layout = new QVBoxLayout(frame);
    layout->setContentsMargins(4, 3, 4, 1);
    layout->setSpacing(1);
    auto* grid = new QGridLayout;
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setHorizontalSpacing(2);
    grid->setVerticalSpacing(2);
    QList<QToolButton*> itemButtons;
    int largeColumn = 0;
    int compactIndex = 0;
    for (const ItemSpec& item : spec.items) {
        if (QToolButton* button = createActionButton(item.actionKey, frame, item.size)) {
            itemButtons.append(button);
            if (item.size == ItemSize::Large) grid->addWidget(button, 0, largeColumn++, 2, 1);
            else {
                grid->addWidget(button, compactIndex % 2, largeColumn + compactIndex / 2);
                ++compactIndex;
            }
        }
    }
    layout->addLayout(grid, 1);
    auto* overflowButton = new QToolButton(frame);
    overflowButton->setObjectName("kuubikRibbonPanelOverflow");
    overflowButton->setText(tr("More"));
    overflowButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    overflowButton->setPopupMode(QToolButton::InstantPopup);
    auto* menu = new QMenu(overflowButton);
    for (const ItemSpec& item : spec.items) {
        if (QAction* action = actions.value(item.actionKey, nullptr)) menu->addAction(action);
    }
    overflowButton->setMenu(menu);
    overflowButton->hide();
    layout->addWidget(overflowButton, 1, Qt::AlignHCenter);
    layout->addWidget(makeLabel(spec.title, "kuubikRibbonGroupLabel", frame));
    panels.append({page, frame, itemButtons, overflowButton, spec.collapsePriority, false});
    return frame;
}

QFrame* KuubikRibbon::createEmbeddedToolbarGroup(const QString& title, QToolBar* toolbar, QWidget* parent)
{
    auto* frame = new QFrame(parent);
    frame->setObjectName("kuubikRibbonGroup");
    auto* layout = new QGridLayout(frame);
    layout->setContentsMargins(4, 3, 4, 1);
    layout->setSpacing(1);
    if (toolbar != nullptr) {
        toolbar->setMovable(false);
        toolbar->setFloatable(false);
        toolbar->setIconSize(QSize(16, 16));
        layout->addWidget(toolbar, 0, 0);
    }
    auto* label = makeLabel(title, "kuubikRibbonGroupLabel", frame);
    label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    layout->addWidget(label, 1, 0);
    return frame;
}

QFrame* KuubikRibbon::createCurrentLayerHost(QWidget* parent)
{
    auto* frame = new QFrame(parent);
    frame->setObjectName("kuubikCurrentLayerHost");
    currentLayerLayout = new QGridLayout(frame);
    currentLayerLayout->setContentsMargins(4, 3, 4, 1);
    currentLayerLayout->setSpacing(2);
    currentLayerLayout->addWidget(makeLabel(tr("Current Layer"), "kuubikCurrentLayerPlaceholder", frame), 0, 0);
    if (QToolButton* edit = createActionButton("LayersEdit", frame, ItemSize::Small)) {
        edit->setObjectName("kuubikCurrentLayerEditButton");
        currentLayerLayout->addWidget(edit, 0, 1);
    }
    auto* label = makeLabel(tr("Layer"), "kuubikRibbonGroupLabel", frame);
    label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    currentLayerLayout->addWidget(label, 1, 0, 1, 2);
    return frame;
}

QWidget* KuubikRibbon::createPage(const TabSpec& spec)
{
    auto* page = new QWidget(this);
    page->setObjectName("kuubikRibbonPage");
    page->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
    page->setProperty("kuubikTabTitle", spec.title);
    auto* layout = new QHBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    for (const PanelSpec& panel : spec.panels) layout->addWidget(createActionGroup(panel, page, page));
    layout->addStretch(1);
    return page;
}

QWidget* KuubikRibbon::takeCurrentLayerSelector()
{
    if (currentLayerSelector == nullptr) return nullptr;
    QWidget* selector = currentLayerSelector;
    currentLayerLayout->removeWidget(selector);
    selector->setParent(currentLayerSelectorOriginalParent.data());
    selector->setVisible(currentLayerSelectorWasVisible);
    currentLayerSelector = nullptr;
    currentLayerSelectorOriginalParent.clear();
    currentLayerSelectorWasVisible = false;
    if (QLabel* placeholder = currentLayerHost->findChild<QLabel*>("kuubikCurrentLayerPlaceholder")) placeholder->show();
    return selector;
}

void KuubikRibbon::setCurrentLayerSelector(QWidget* selector)
{
    if (currentLayerLayout == nullptr || currentLayerHost == nullptr || selector == currentLayerSelector) return;
    takeCurrentLayerSelector();
    if (selector == nullptr) return;
    currentLayerSelectorOriginalParent = selector->parentWidget();
    currentLayerSelectorWasVisible = selector->isVisible();
    currentLayerSelector = selector;
    selector->setParent(currentLayerHost);
    currentLayerLayout->addWidget(selector, 0, 0);
    selector->show();
    if (QLabel* placeholder = currentLayerHost->findChild<QLabel*>("kuubikCurrentLayerPlaceholder")) placeholder->hide();
}

void KuubikRibbon::setPanelCollapsed(PanelInstance& panel, bool collapsed)
{
    if (panel.collapsed == collapsed) return;
    panel.collapsed = collapsed;
    panel.frame->setProperty("kuubikCollapsed", collapsed);
    panel.frame->style()->unpolish(panel.frame);
    panel.frame->style()->polish(panel.frame);
    for (QToolButton* button : panel.itemButtons) button->setVisible(!collapsed);
    panel.overflowButton->setVisible(collapsed);
    panel.frame->layout()->invalidate();
    panel.frame->updateGeometry();
}

void KuubikRibbon::updateCollapsedPanels()
{
    if (tabs == nullptr || tabs->currentWidget() == nullptr) return;
    QWidget* page = tabs->currentWidget();
    QList<int> candidates;
    int requiredWidth = 0;
    for (int index = 0; index < panels.size(); ++index) {
        PanelInstance& panel = panels[index];
        if (panel.page != page) continue;
        setPanelCollapsed(panel, false);
        requiredWidth += panel.frame->sizeHint().width();
        if (panel.collapsePriority > 0) candidates.append(index);
    }
    for (QWidget* fixedHost : {static_cast<QWidget*>(currentLayerHost),
                               static_cast<QWidget*>(penToolbarHost)}) {
        if (fixedHost != nullptr && fixedHost->parentWidget() == page
            && fixedHost->isVisible()) {
            requiredWidth += fixedHost->sizeHint().width();
        }
    }
    if (page->layout() != nullptr) {
        const QMargins margins = page->layout()->contentsMargins();
        requiredWidth += margins.left() + margins.right();
        requiredWidth += qMax(0, page->layout()->count() - 1)
                         * page->layout()->spacing();
    }
    std::sort(candidates.begin(), candidates.end(), [this](int left, int right) { return panels[left].collapsePriority > panels[right].collapsePriority; });
    for (int index : candidates) {
        if (requiredWidth <= page->contentsRect().width()) break;
        PanelInstance& panel = panels[index];
        const int before = panel.frame->sizeHint().width();
        setPanelCollapsed(panel, true);
        requiredWidth -= before - panel.frame->sizeHint().width();
    }
}

void KuubikRibbon::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateCollapsedPanels();
}

QStringList KuubikRibbon::boundActionKeys() const { return actionButtons.keys(); }
QStringList KuubikRibbon::missingActionKeys() const { return missingKeys; }
QToolButton* KuubikRibbon::buttonForAction(const QString& key) const { return actionButtons.value(key, nullptr); }

void KuubikRibbon::setWorkspaceActions(QAction* kuubikAction, QAction* classicAction)
{
    if (applicationMenu == nullptr || workspaceActionsAdded) return;
    applicationMenu->addSeparator();
    if (kuubikAction != nullptr) applicationMenu->addAction(kuubikAction);
    if (classicAction != nullptr) applicationMenu->addAction(classicAction);
    workspaceActionsAdded = true;
}

void KuubikRibbon::embedNativeToolbars(QMainWindow* mainWindow)
{
    if (mainWindow == nullptr) return;
    if (optionToolbar != nullptr && optionToolbarLayout != nullptr) {
        mainWindow->removeToolBar(optionToolbar);
        optionToolbar->setParent(optionToolbarHost);
        optionToolbarLayout->addWidget(optionToolbar, 0, 0);
        optionToolbar->show();
    }
    if (penToolbar != nullptr && penToolbarLayout != nullptr) {
        mainWindow->removeToolBar(penToolbar);
        penToolbar->setParent(penToolbarHost);
        penToolbarLayout->addWidget(penToolbar, 0, 0);
        penToolbar->show();
    }
}

void KuubikRibbon::releaseNativeToolbars(QMainWindow* mainWindow)
{
    if (mainWindow == nullptr) return;
    if (optionToolbar != nullptr) {
        optionToolbarLayout->removeWidget(optionToolbar);
        optionToolbar->setParent(mainWindow);
        mainWindow->addToolBar(Qt::TopToolBarArea, optionToolbar);
        optionToolbar->show();
    }
    if (penToolbar != nullptr) {
        penToolbarLayout->removeWidget(penToolbar);
        penToolbar->setParent(mainWindow);
        mainWindow->addToolBar(Qt::TopToolBarArea, penToolbar);
        penToolbar->show();
    }
}
