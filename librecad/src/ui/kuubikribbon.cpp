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
#include "SARibbon.h"

#include <QAction>
#include <QComboBox>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QLabel>
#include <QMainWindow>
#include <QMenu>
#include <QResizeEvent>
#include <QSizePolicy>
#include <QStyle>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>

#include <algorithm>

namespace {
class KuubikRibbonTabBar : public SARibbonTabBar
{
public:
    using SARibbonTabBar::SARibbonTabBar;
    QSize tabSizeHint(int index) const override
    {
        QSize hint = SARibbonTabBar::tabSizeHint(index);
        // Upstream starts with height() and then adds the QSS border again.
        // Keep the actual tab hit rectangle inside the allocated tab row.
        hint.setHeight(height());
        return hint;
    }
};

class KuubikRibbonElementFactory : public SARibbonElementFactory
{
public:
    SARibbonTabBar* createRibbonTabBar(QWidget* parent) override
    {
        return new KuubikRibbonTabBar(parent);
    }
};

class KuubikRibbonToolButton : public SARibbonToolButton
{
public:
    explicit KuubikRibbonToolButton(QAction* action, QWidget* parent)
        // This upstream constructor retains the application's QSS style;
        // the widget-only constructor installs its own Fusion proxy style.
        : SARibbonToolButton(action, parent) {}
    QSize sizeHint() const override
    {
        QSize hint = SARibbonToolButton::sizeHint();
        if (toolButtonStyle() == Qt::ToolButtonTextUnderIcon) {
            hint.setWidth(qMax(largeIconSize().width() + 8,
                               fontMetrics().horizontalAdvance(text()) + 8));
        } else if (toolButtonStyle() == Qt::ToolButtonIconOnly) {
            hint.setWidth(26);
        } else {
            hint = hint.expandedTo(QToolButton::sizeHint());
        }
        return hint.expandedTo(minimumSize());
    }
};

// Only the size hint is Kuubik-specific. SARibbon still owns the panel layout.
class KuubikSizedRibbonPanel : public SARibbonPanel
{
public:
    using SARibbonPanel::SARibbonPanel;
    QSize sizeHint() const override
    {
        QSize result = SARibbonPanel::sizeHint();
        const bool collapsed = property("kuubikCollapsed").toBool();
        const int titleWidth = panelLayout()->panelTitleLabel()->fontMetrics().horizontalAdvance(panelName()) + 12;
        const int preferred = collapsed ? qMax(54, titleWidth)
                                        : property("kuubikReferenceWidth").toInt();
        result.setWidth(qMax(result.width(), qMax(titleWidth, preferred)));
        return result;
    }
};

QLabel* makeLabel(const QString& text, const char* objectName, QWidget* parent)
{
    auto* label = new QLabel(text, parent);
    label->setObjectName(QString::fromLatin1(objectName));
    return label;
}

bool containedThroughAncestors(const QWidget* widget, const QWidget* root)
{
    if (widget == nullptr || !widget->isVisibleTo(root)) return false;
    const QWidget* child = widget;
    for (const QWidget* parent = child->parentWidget(); parent != nullptr;
         child = parent, parent = parent->parentWidget()) {
        if (!parent->rect().contains(child->geometry())) return false;
        if (parent == root) return true;
    }
    return widget == root;
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
    applicationButton->setText(tr("Kuubik"));
    applicationButton->setIcon(QIcon(QStringLiteral(":/main/kuubikdraw.png")));
    applicationButton->setIconSize(QSize(18, 18));
    applicationButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
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
    // Let the native Tool Options toolbar claim its size hint first. The
    // preceding stretch then absorbs only the remaining space; sharing that
    // space equally clipped the combined DimLinear option widgets at 1280 px.
    quickLayout->addWidget(optionToolbarHost);
    quickLayout->addWidget(makeLabel(tr("Kuubik Draw · GPL"), "kuubikProductBadge", quickBar));
    root->addWidget(quickBar);

    // SARibbon documents this factory as the supported component extension
    // point. The manager owns it; all CAD commands remain native QActions.
    if (dynamic_cast<KuubikRibbonElementFactory*>(RibbonSubElementFactory) == nullptr) {
        RibbonSubElementMgr->setupFactory(new KuubikRibbonElementFactory);
    }
    tabs = new SARibbonBar(this);
    tabs->setObjectName("kuubikRibbonTabs");
    tabs->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
    tabs->setRibbonStyle(SARibbonBar::RibbonStyleLooseThreeRow);
    tabs->setTitleVisible(false);
    tabs->setTitleIconVisible(false);
    tabs->setApplicationButton(nullptr);
    tabs->quickAccessBar()->hide();
    // Kuubik already has its own quick-access row and native window title.
    // Hiding title text alone does not remove SARibbon's reserved title row.
    tabs->setTitleBarHeight(0);
    // The minimum-mode action is absent by default. In upstream 2.9.0,
    // showMinimumModeButton(false) dereferences that null action unless it was
    // previously created. Do not create or hide an unused control.
    tabs->setTabBarHeight(26);
    tabs->setCategoryHeight(96);
    tabs->setPanelTitleHeight(16);
    tabs->setPanelSpacing(1);
    tabs->setEnableWordWrap(false);
    tabs->setPanelToolButtonIconSize(QSize(16, 16), QSize(32, 32));
    tabs->setTabBarBaseLineColor(QColor("#3B4453"));

    const QList<TabSpec> specs {
        {tr("Home"), {
            // Keep the four everyday geometry commands visible, matching the
            // command hierarchy familiar from AutoCAD's Home ribbon.  The
            // panel is deliberately non-collapsible; secondary panels yield
            // first when the window is narrow.
            {tr("Draw"), {{"DrawLine", ItemSize::Large}, {"DrawPolyline", ItemSize::Large}, {"DrawCircle", ItemSize::Large}, {"DrawArc", ItemSize::Large}, {"DrawLineRectangle", ItemSize::Small}, {"DrawHatch", ItemSize::Small}}, -1},
            {tr("Modify"), {{"ModifyMove", ItemSize::Large}, {"ModifyDuplicate", ItemSize::Large}, {"ModifyTrim", ItemSize::Large}, {"ModifyTrim2", ItemSize::Small}, {"ModifyCut", ItemSize::Small}, {"ModifyOffset", ItemSize::Medium}, {"ModifyRotate", ItemSize::Medium}, {"ModifyMirror", ItemSize::Small}, {"ModifyScale", ItemSize::Small}, {"ModifyRound", ItemSize::Small}, {"ModifyDeleteQuick", ItemSize::Small}}, 1},
            {tr("Annotation"), {{"DrawText", ItemSize::Medium}, {"DrawMText", ItemSize::Medium}, {"DimLinear", ItemSize::Small}, {"DimAligned", ItemSize::Small}, {"DimLinearHor", ItemSize::Small}, {"DimLinearVer", ItemSize::Small}, {"DimRadial", ItemSize::Small}, {"DimDiametric", ItemSize::Small}, {"DimAngular", ItemSize::Small}, {"DimLeader", ItemSize::Small}}, 200},
            {tr("Layers"), {{"LayersAdd", ItemSize::Small}, {"LayersEdit", ItemSize::Small}, {"LayersToggleView", ItemSize::Small}, {"LayersToggleLock", ItemSize::Small}}, -1},
            {tr("Block"), {{"BlocksInsert", ItemSize::Large}, {"BlocksCreate", ItemSize::Medium}, {"BlocksEdit", ItemSize::Small}, {"BlocksExplode", ItemSize::Small}, {"BlocksImport", ItemSize::Small}}, 300},
            // The native pen selectors must remain accessible at review widths;
            // the action-only overflow menu cannot represent those widgets.
            {tr("Properties"), {{"ModifyEntity", ItemSize::Small}, {"PenSyncFromLayer", ItemSize::Small}, {"PenPick", ItemSize::Small}, {"PenPickResolved", ItemSize::Small}, {"PenApply", ItemSize::Small}, {"PenCopy", ItemSize::Small}}, -1},
            {tr("Groups"), {}, -1},
            {tr("Utilities"), {{"InfoDist", ItemSize::Small}, {"InfoAngle", ItemSize::Small}, {"InfoArea", ItemSize::Small}, {"InfoTotalLength", ItemSize::Small}}, 400},
            {tr("Clipboard"), {{"EditCut", ItemSize::Small}, {"EditCopy", ItemSize::Small}, {"EditPaste", ItemSize::Small}}, 500},
            {tr("View"), {{"ZoomAuto", ItemSize::Small}, {"ZoomWindow", ItemSize::Small}, {"ZoomPan", ItemSize::Small}}, 550}
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
        createPage(spec);
    }
    connect(tabs, &SARibbonBar::currentRibbonTabChanged, this, [this](int) {
        QTimer::singleShot(0, this, [this] { updateCollapsedPanels(); });
    });
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
    auto* button = new KuubikRibbonToolButton(action, parent);
    button->setButtonType(size == ItemSize::Large && !iconOnly
                            ? SARibbonToolButton::LargeButton
                            : SARibbonToolButton::SmallButton);
    button->setEnableWordWrap(false);
    button->setLargeIconSize(QSize(32, 32));
    button->setSpacing(2);
    button->setFocusPolicy(Qt::StrongFocus);
    button->setObjectName("kuubikRibbonButton");
    button->setProperty("kuubikActionKey", key);
    button->setProperty("kuubikItemSize", static_cast<int>(size));
    button->setDefaultAction(action);
    // QAction labels describe some LibreCAD construction variants (for
    // example "2 Points") rather than the familiar top-level CAD command.
    // Keep the QAction identity, but use concise ribbon-facing command names.
    const QMap<QString, QString> ribbonLabels {
        {QStringLiteral("DrawLine"), tr("Line")},
        {QStringLiteral("DrawPolyline"), tr("Polyline")},
        {QStringLiteral("DrawCircle"), tr("Circle")},
        {QStringLiteral("DrawArc"), tr("Arc")},
        {QStringLiteral("DrawLineRectangle"), tr("Rectangle")},
        {QStringLiteral("DrawHatch"), tr("Hatch")},
        {QStringLiteral("ModifyMove"), tr("Move")},
        {QStringLiteral("ModifyDuplicate"), tr("Copy")},
        {QStringLiteral("ModifyTrim"), tr("Trim")},
        {QStringLiteral("ModifyOffset"), tr("Offset")},
        {QStringLiteral("ModifyRotate"), tr("Rotate")}
    };
    const QString ribbonText = ribbonLabels.value(key, action->text().remove('&'));
    button->setText(ribbonText);
    const QIcon icon = KuubikIconRegistry::iconForAction(key);
    if (!icon.isNull()) button->setIcon(icon);
    // QToolButton::setDefaultAction() deliberately mirrors later QAction
    // text/icon changes. LibreCAD changes some action presentation while a
    // construction mode is active (for example LINE becomes "2 Points").
    // Preserve those native state/enabled/trigger connections, but keep the
    // Kuubik ribbon's command identity visually stable.
    connect(action, &QAction::changed, button,
            [button, ribbonText, icon] {
                button->setText(ribbonText);
                if (!icon.isNull()) button->setIcon(icon);
            }, Qt::QueuedConnection);
    button->setAutoRaise(true);
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
    button->setAccessibleName(ribbonText);
    button->setToolTip(ribbonText + QStringLiteral("\n") + action->toolTip());
    if (!actionButtons.contains(key)) actionButtons.insert(key, button);
    return button;
}

SARibbonPanel* KuubikRibbon::createActionGroup(const PanelSpec& spec,
                                             SARibbonCategory* page,
                                             int referenceWidth)
{
    auto* frame = new KuubikSizedRibbonPanel(spec.title, page);
    page->addPanel(frame);
    frame->setObjectName("kuubikRibbonGroup");
    frame->setProperty("kuubikCollapsePriority", spec.collapsePriority);
    frame->setProperty("kuubikPanelTitle", spec.title);
    QStringList actionKeys;
    for (const ItemSpec& item : spec.items) actionKeys.append(item.actionKey);
    frame->setProperty("kuubikActionKeys", actionKeys);
    frame->setProperty("kuubikCollapsed", false);
    frame->setProperty("kuubikReferenceWidth", referenceWidth);
    frame->setProperty("kuubikUnavailable", spec.items.isEmpty());
    frame->panelLayout()->setContentsMargins(3, 2, 3, 0);
    frame->panelLayout()->panelTitleLabel()->setObjectName("kuubikRibbonGroupLabel");
    QList<QToolButton*> itemButtons;
    QList<QAction*> presentationActions;
    const bool isHome = page->property("kuubikTabTitle").toString() == tr("Home");
    if (isHome && spec.title == tr("Layers")) {
        currentLayerHost = createCurrentLayerHost(frame);
        presentationActions.append(frame->addLargeWidget(currentLayerHost));
    }
    if (isHome && spec.title == tr("Properties")) {
        penToolbarHost = new QFrame(frame);
        penToolbarHost->setObjectName("kuubikPenToolbarHost");
        penToolbarLayout = new QGridLayout(penToolbarHost);
        penToolbarLayout->setContentsMargins(0, 0, 0, 0);
        penToolbarLayout->setSpacing(0);
        if (penToolbar != nullptr) {
            penToolbarOriginalOrientation = penToolbar->orientation();
            penToolbarOriginalMovable = penToolbar->isMovable();
            penToolbarOriginalFloatable = penToolbar->isFloatable();
            penToolbarOriginalActions = penToolbar->actions();
        }
        presentationActions.append(frame->addLargeWidget(penToolbarHost));
    }
    for (const ItemSpec& item : spec.items) {
        const bool compactIcon = isHome && item.size != ItemSize::Large
                                 && spec.title != tr("Clipboard");
        if (QToolButton* button = createActionButton(item.actionKey, frame, item.size, compactIcon)) {
            if (compactIcon) button->setProperty("kuubikIconOnly", true);
            itemButtons.append(button);
            const auto proportion = item.size == ItemSize::Large ? SARibbonPanelItem::Large
                                  : item.size == ItemSize::Medium ? SARibbonPanelItem::Medium
                                                                 : SARibbonPanelItem::Small;
            // Hiding the presentation action must never hide the shared native
            // command in another tab, menu or Classic workspace.
            QAction* presentation = frame->addWidget(button, proportion);
            QAction* native = button->defaultAction();
            connect(native, &QAction::changed, presentation, [native, presentation] {
                presentation->setEnabled(native->isEnabled());
            });
            presentationActions.append(presentation);
        }
    }
    if (spec.items.isEmpty()) {
        auto* unavailable = new QToolButton(frame);
        unavailable->setObjectName("kuubikRibbonUnavailable");
        unavailable->setText(tr("Group"));
        unavailable->setEnabled(false);
        unavailable->setToolTip(tr("Groups are unavailable in Kuubik Draw. Blocks are not groups."));
        unavailable->setAccessibleName(unavailable->toolTip());
        presentationActions.append(frame->addLargeWidget(unavailable));
    }
    auto* overflowButton = new QToolButton(frame);
    overflowButton->setObjectName("kuubikRibbonPanelOverflow");
    if (!spec.items.isEmpty()) {
        overflowButton->setIcon(KuubikIconRegistry::iconForAction(
            spec.items.constFirst().actionKey));
    }
    overflowButton->setIconSize(QSize(30, 30));
    overflowButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    overflowButton->setAccessibleName(tr("Show %1 tools").arg(spec.title));
    overflowButton->setToolTip(tr("Show %1 tools").arg(spec.title));
    overflowButton->setPopupMode(QToolButton::InstantPopup);
    auto* menu = new QMenu(overflowButton);
    for (const ItemSpec& item : spec.items) {
        if (QAction* action = actions.value(item.actionKey, nullptr)) menu->addAction(action);
    }
    overflowButton->setMenu(menu);
    overflowButton->setEnabled(!spec.items.isEmpty());
    QAction* overflowPresentation = frame->addLargeWidget(overflowButton);
    overflowPresentation->setVisible(false);
    panels.append({page, frame, itemButtons, presentationActions, overflowButton,
                   overflowPresentation, spec.collapsePriority, false});
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
    currentLayerLayout->addWidget(makeLabel(tr("Current layer"), "kuubikLayerCaption", frame), 1, 0);
    currentLayerLayout->setRowStretch(2, 1);
    return frame;
}

SARibbonCategory* KuubikRibbon::createPage(const TabSpec& spec)
{
    auto* page = tabs->addCategoryPage(spec.title);
    page->setObjectName("kuubikRibbonPage");
    page->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
    page->setProperty("kuubikTabTitle", spec.title);
    page->setUseAnimatingScroll(false);
    page->layout()->setContentsMargins(0, 0, 0, 0);
    const QList<int> homeWidths {225, 250, 189, 273, 161, 262, 72, 97, 91, 53};
    int index = 0;
    for (const PanelSpec& panel : spec.panels) {
        const int width = spec.title == tr("Home") ? homeWidths.value(index) : 0;
        createActionGroup(panel, page, width);
        ++index;
    }
    // Panel borders already draw the separator. Category layout reads each
    // separator's sizeHint(), not fixedWidth(), so detach its optional spacer
    // through the public layout item instead of leaving an invalid QFrame hint.
    for (int itemIndex = 0; itemIndex < page->layout()->count(); ++itemIndex) {
        auto* item = dynamic_cast<SARibbonCategoryLayoutItem*>(page->layout()->itemAt(itemIndex));
        if (item != nullptr && item->separatorWidget != nullptr) {
            item->separatorWidget->hide();
            item->separatorWidget->deleteLater();
            item->separatorWidget = nullptr;
        }
    }
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
    for (QAction* presentation : panel.presentationActions) presentation->setVisible(!collapsed);
    panel.overflowPresentation->setVisible(collapsed);
    panel.frame->layout()->invalidate();
    panel.frame->layout()->setGeometry(panel.frame->rect());
    panel.frame->updateGeometry();
}

void KuubikRibbon::updateCollapsedPanels()
{
    if (tabs == nullptr || updatingPanels) return;
    SARibbonCategory* page = tabs->categoryByIndex(tabs->currentIndex());
    if (page == nullptr) return;
    updatingPanels = true;
    QList<int> candidates;
    int requiredWidth = 0;
    for (int index = 0; index < panels.size(); ++index) {
        PanelInstance& panel = panels[index];
        if (panel.page != page) continue;
        setPanelCollapsed(panel, false);
        requiredWidth += panel.frame->sizeHint().width();
        if (panel.collapsePriority > 0) candidates.append(index);
    }
    if (page->layout() != nullptr) {
        const QMargins margins = page->layout()->contentsMargins();
        requiredWidth += margins.left() + margins.right();
    }
    std::sort(candidates.begin(), candidates.end(), [this](int left, int right) { return panels[left].collapsePriority > panels[right].collapsePriority; });
    for (int index : candidates) {
        if (requiredWidth <= page->contentsRect().width()) break;
        PanelInstance& panel = panels[index];
        const int before = panel.frame->sizeHint().width();
        setPanelCollapsed(panel, true);
        requiredWidth -= before - panel.frame->sizeHint().width();
    }
    page->updateItemGeometry();
    updatingPanels = false;
}

void KuubikRibbon::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    // The category child receives its new width after the outer resize event.
    // Coalesce against that final width, not the previous layout's geometry.
    if (!panelUpdatePending) {
        panelUpdatePending = true;
        QTimer::singleShot(0, this, [this] {
            panelUpdatePending = false;
            updateCollapsedPanels();
        });
    }
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
        // Keep the original three native selectors in the vertical toolbar.
        // Its five extra native commands are presented alongside it in SARibbon;
        // only remove their toolbar placement, never change QAction visibility.
        for (const QString& key : {QStringLiteral("PenSyncFromLayer"), QStringLiteral("PenPick"),
                                  QStringLiteral("PenPickResolved"), QStringLiteral("PenApply"),
                                  QStringLiteral("PenCopy")}) {
            if (QAction* action = actions.value(key, nullptr)) penToolbar->removeAction(action);
        }
        penToolbar->setOrientation(Qt::Vertical);
        penToolbar->setMovable(false);
        penToolbar->setFloatable(false);
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
        penToolbar->setOrientation(penToolbarOriginalOrientation);
        penToolbar->setMovable(penToolbarOriginalMovable);
        penToolbar->setFloatable(penToolbarOriginalFloatable);
        for (QAction* action : penToolbarOriginalActions) {
            if (!penToolbar->actions().contains(action)) penToolbar->addAction(action);
        }
        mainWindow->addToolBar(Qt::TopToolBarArea, penToolbar);
        penToolbar->show();
    }
}

QJsonObject KuubikRibbon::layoutContract() const
{
    QJsonObject result;
    result.insert("implementation", "SARibbon");
    result.insert("version", "2.9.0");
    result.insert("frameless", false);
    result.insert("width", width());
    result.insert("height", height());
    result.insert("barContained", containedThroughAncestors(tabs, this));
    result.insert("currentLayerInLayers", currentLayerHost != nullptr
                  && currentLayerHost->parentWidget()->property("kuubikPanelTitle").toString() == tr("Layers"));
    result.insert("penInProperties", penToolbarHost != nullptr && penToolbar != nullptr
                  && penToolbarHost->isAncestorOf(penToolbar)
                  && penToolbarHost->parentWidget()->property("kuubikPanelTitle").toString() == tr("Properties")
                  && penToolbar->orientation() == Qt::Vertical);
    QJsonArray tabNames;
    for (int index = 0; tabs->categoryByIndex(index) != nullptr; ++index) {
        tabNames.append(tabs->categoryByIndex(index)->property("kuubikTabTitle").toString());
    }
    result.insert("tabs", tabNames);
    QJsonArray records;
    for (const auto& panel : panels) {
        QJsonObject record;
        record.insert("tab", panel.page->property("kuubikTabTitle").toString());
        record.insert("title", panel.frame->property("kuubikPanelTitle").toString());
        record.insert("collapsed", panel.collapsed);
        record.insert("unavailable", panel.frame->property("kuubikUnavailable").toBool());
        record.insert("referenceWidth", panel.frame->property("kuubikReferenceWidth").toInt());
        record.insert("x", panel.frame->x());
        record.insert("y", panel.frame->y());
        record.insert("width", panel.frame->width());
        record.insert("height", panel.frame->height());
        record.insert("visible", panel.frame->isVisibleTo(this));
        record.insert("contained", containedThroughAncestors(panel.frame, this));
        record.insert("pageWidth", panel.page->width());
        record.insert("titleVisible", panel.frame->panelLayout()->panelTitleLabel()->isVisibleTo(this));
        record.insert("titleFits", panel.frame->panelLayout()->panelTitleLabel()->fontMetrics().horizontalAdvance(
                          panel.frame->panelName()) <= panel.frame->panelLayout()->panelTitleLabel()->width());
        if (auto* unavailable = panel.frame->findChild<QToolButton*>("kuubikRibbonUnavailable")) {
            record.insert("unavailableEnabled", unavailable->isEnabled());
            record.insert("unavailableTooltip", unavailable->toolTip());
        }
        QJsonArray controls;
        for (QToolButton* button : panel.itemButtons) {
            QJsonObject control;
            control.insert("key", button->property("kuubikActionKey").toString());
            control.insert("visible", button->isVisibleTo(this));
            control.insert("contained", panel.frame->rect().contains(button->geometry()));
            control.insert("enabledMatchesNative", button->isEnabled() == button->defaultAction()->isEnabled());
            control.insert("nativeIdentity", button->defaultAction() == actions.value(
                               button->property("kuubikActionKey").toString(), nullptr));
            control.insert("x", button->x());
            control.insert("y", button->y());
            control.insert("width", button->width());
            control.insert("height", button->height());
            controls.append(control);
        }
        record.insert("controls", controls);
        QJsonArray embeddedControls;
        for (QComboBox* combo : panel.frame->findChildren<QComboBox*>()) {
            QJsonObject control;
            control.insert("name", combo->objectName());
            control.insert("visible", combo->isVisibleTo(this));
            control.insert("contained", containedThroughAncestors(combo, this));
            control.insert("width", combo->width());
            control.insert("height", combo->height());
            embeddedControls.append(control);
        }
        record.insert("embeddedControls", embeddedControls);
        records.append(record);
    }
    result.insert("panels", records);
    return result;
}
