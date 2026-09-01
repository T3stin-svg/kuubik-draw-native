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
#include "kuubiktheme.h"

#include <QAction>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QSizePolicy>
#include <QTabWidget>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>

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
    : QWidget(parent)
    , actions(actionMap)
    , penToolbar(nativePenToolbar)
    , optionToolbar(nativeOptionToolbar)
{
    setObjectName("kuubikRibbon");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
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

    const QStringList quickKeys {
        "FileNew", "FileOpen", "FileSave", "EditUndo", "EditRedo", "FilePrint"
    };
    for (const auto& key : quickKeys) {
        if (auto* button = createActionButton(key, quickBar, true)) {
            button->setObjectName("kuubikQuickButton");
            quickLayout->addWidget(button);
        }
    }
    quickLayout->addSpacing(8);
    quickLayout->addWidget(makeLabel(tr("2D Drafting & Annotation"),
                                     "kuubikWorkspaceLabel", quickBar));
    quickLayout->addStretch(1);

    optionToolbarHost = new QFrame(quickBar);
    optionToolbarHost->setObjectName("kuubikOptionToolbarHost");
    optionToolbarLayout = new QGridLayout(optionToolbarHost);
    optionToolbarLayout->setContentsMargins(0, 0, 0, 0);
    optionToolbarLayout->setSpacing(0);
    quickLayout->addWidget(optionToolbarHost, 1);
    quickLayout->addSpacing(8);
    quickLayout->addWidget(makeLabel(tr("Kuubik Draw · GPL"),
                                     "kuubikProductBadge", quickBar));
    root->addWidget(quickBar);

    tabs = new QTabWidget(this);
    tabs->setObjectName("kuubikRibbonTabs");
    tabs->setDocumentMode(true);
    tabs->setMovable(false);
    tabs->setUsesScrollButtons(true);

    tabs->addTab(createPage({
        {tr("Draw"), {"DrawLine", "DrawPolyline", "DrawLineRectangle", "DrawCircle", "DrawArc"}},
        {tr("Modify"), {"ModifyMove", "ModifyDuplicate", "ModifyRotate", "ModifyScale",
                        "ModifyMirror", "ModifyOffset", "ModifyTrim", "ModifyTrim2",
                        "ModifyRound", "ModifyDeleteQuick"}},
        {tr("Layers"), {"LayersAdd", "LayersEdit", "LayersToggleView", "LayersToggleLock"}},
        {tr("Properties"), {"ModifyEntity"}}
    }), tr("Home"));

    tabs->addTab(createPage({
        {tr("Text"), {"DrawText", "DrawMText"}},
        {tr("Dimensions"), {"DimLinear", "DimAligned", "DimLinearHor", "DimLinearVer",
                            "DimRadial", "DimDiametric", "DimAngular", "DimLeader"}},
        {tr("Detail"), {"DrawHatch"}}
    }), tr("Annotate"));

    tabs->addTab(createPage({
        {tr("Blocks"), {"BlocksCreate", "BlocksInsert", "BlocksEdit", "BlocksExplode"}},
        {tr("Content"), {"BlocksImport", "DrawImage"}}
    }), tr("Insert"));

    tabs->addTab(createPage({
        {tr("Display"), {"ViewGrid", "ViewDraft", "ZoomRedraw", "ZoomAuto"}},
        {tr("Navigate"), {"ZoomIn", "ZoomOut", "ZoomPrevious", "ZoomWindow", "ZoomPan"}},
        {tr("Snaps"), {"SnapEnd", "SnapMiddle", "SnapCenter", "SnapIntersection",
                       "RestrictOrthogonal"}}
    }), tr("View"));

    tabs->addTab(createPage({
        {tr("File"), {"FileSaveAs", "FileSaveAll", "FileExport"}},
        {tr("Print"), {"FilePrintPreview", "FilePrintPDF", "FilePrint"}}
    }), tr("Output"));

    // The native pen controls are real LibreCAD widgets. They are embedded in
    // Home rather than reimplemented so property changes retain upstream logic.
    auto* homePage = tabs->widget(0);
    auto* homeLayout = qobject_cast<QHBoxLayout*>(homePage->layout());
    penToolbarHost = createEmbeddedToolbarGroup(tr("Pen"), penToolbar, homePage);
    penToolbarLayout = qobject_cast<QGridLayout*>(penToolbarHost->layout());
    homeLayout->addWidget(penToolbarHost);
    homeLayout->addStretch(1);

    root->addWidget(tabs, 1);
}

QToolButton* KuubikRibbon::createActionButton(const QString& key,
                                              QWidget* parent,
                                              bool iconOnly)
{
    QAction* action = actions.value(key, nullptr);
    if (action == nullptr) {
        if (!missingKeys.contains(key)) {
            missingKeys.append(key);
        }
        return nullptr;
    }

    auto* button = new QToolButton(parent);
    button->setObjectName("kuubikRibbonButton");
    button->setProperty("kuubikActionKey", key);
    button->setDefaultAction(action);
    button->setAutoRaise(false);
    button->setIconSize(iconOnly ? QSize(16, 16) : QSize(18, 18));
    button->setToolButtonStyle(iconOnly ? Qt::ToolButtonIconOnly
                                        : Qt::ToolButtonTextBesideIcon);
    button->setAccessibleName(action->text().remove('&'));
    actionButtons.insert(key, button);
    return button;
}

QFrame* KuubikRibbon::createActionGroup(const GroupSpec& spec, QWidget* parent)
{
    auto* frame = new QFrame(parent);
    frame->setObjectName("kuubikRibbonGroup");
    auto* layout = new QVBoxLayout(frame);
    layout->setContentsMargins(4, 3, 4, 1);
    layout->setSpacing(1);

    auto* buttons = new QGridLayout;
    buttons->setContentsMargins(0, 0, 0, 0);
    buttons->setHorizontalSpacing(2);
    buttons->setVerticalSpacing(2);
    int visibleIndex = 0;
    for (const auto& key : spec.actionKeys) {
        if (auto* button = createActionButton(key, frame)) {
            buttons->addWidget(button, visibleIndex % 2, visibleIndex / 2);
            ++visibleIndex;
        }
    }
    layout->addLayout(buttons, 1);
    layout->addWidget(makeLabel(spec.title, "kuubikRibbonGroupLabel", frame));
    return frame;
}

QFrame* KuubikRibbon::createEmbeddedToolbarGroup(const QString& title,
                                                 QToolBar* toolbar,
                                                 QWidget* parent)
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

QWidget* KuubikRibbon::createPage(const QList<GroupSpec>& groups)
{
    auto* page = new QWidget(this);
    page->setObjectName("kuubikRibbonPage");
    auto* layout = new QHBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    for (const auto& group : groups) {
        layout->addWidget(createActionGroup(group, page));
    }
    layout->addStretch(1);
    return page;
}

QStringList KuubikRibbon::boundActionKeys() const
{
    return actionButtons.keys();
}

QStringList KuubikRibbon::missingActionKeys() const
{
    return missingKeys;
}

QToolButton* KuubikRibbon::buttonForAction(const QString& key) const
{
    return actionButtons.value(key, nullptr);
}

void KuubikRibbon::embedNativeToolbars(QMainWindow* mainWindow)
{
    if (mainWindow == nullptr) {
        return;
    }
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
    if (mainWindow == nullptr) {
        return;
    }
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
