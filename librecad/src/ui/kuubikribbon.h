/****************************************************************************
**
** Kuubik Draw compact ribbon.
**
** This file is part of Kuubik Draw, a GPLv2 fork of LibreCAD.
** It may be distributed and/or modified under the terms of the GNU General
** Public License version 2 as published by the Free Software Foundation.
**
****************************************************************************/

#ifndef KUUBIKRIBBON_H
#define KUUBIKRIBBON_H

#include <QList>
#include <QJsonObject>
#include <QMap>
#include <QPointer>
#include <QStringList>
#include <QWidget>

class QAction;
class QFrame;
class QGridLayout;
class QMainWindow;
class QMenu;
class QResizeEvent;
class SARibbonBar;
class SARibbonCategory;
class SARibbonPanel;
class QToolBar;
class QToolButton;

class KuubikRibbon : public QWidget
{
public:
    KuubikRibbon(const QMap<QString, QAction*>& actions,
                 QToolBar* penToolbar,
                 QToolBar* optionToolbar,
                 QWidget* parent = nullptr);

    QStringList boundActionKeys() const;
    QStringList missingActionKeys() const;
    QToolButton* buttonForAction(const QString& key) const;
    QJsonObject layoutContract() const;
    // Automation-only: exercise visible tabs, keyboard input and native state mirroring.
    QJsonObject interactionContract();
    void setWorkspaceActions(QAction* kuubikAction, QAction* classicAction);

    void embedNativeToolbars(QMainWindow* mainWindow);
    void releaseNativeToolbars(QMainWindow* mainWindow);
    // Reparent a native selector into the host; nullptr restores parent and visibility.
    void setCurrentLayerSelector(QWidget* selector);
    // Restores it and returns it; caller re-inserts it when original layout placement matters.
    QWidget* takeCurrentLayerSelector();

private:
    enum class ItemSize { Large, Medium, Small };

    struct ItemSpec {
        QString actionKey;
        ItemSize size {ItemSize::Small};
    };
    struct PanelSpec {
        QString title;
        QList<ItemSpec> items;
        int collapsePriority {0};
    };
    struct TabSpec {
        QString title;
        QList<PanelSpec> panels;
    };
    struct PanelInstance {
        QWidget* page {nullptr};
        SARibbonPanel* frame {nullptr};
        QList<QToolButton*> itemButtons;
        QList<QAction*> presentationActions;
        QToolButton* overflowButton {nullptr};
        QAction* overflowPresentation {nullptr};
        int collapsePriority {0};
        bool collapsed {false};
    };

    SARibbonPanel* createActionGroup(const PanelSpec& spec, SARibbonCategory* page,
                                    int referenceWidth = 0);
    QToolButton* createActionButton(const QString& key, QWidget* parent,
                                    ItemSize size = ItemSize::Small,
                                    bool iconOnly = false);
    SARibbonCategory* createPage(const TabSpec& spec);
    QFrame* createCurrentLayerHost(QWidget* parent);
    void updateCollapsedPanels();
    void setPanelCollapsed(PanelInstance& panel, bool collapsed);
    void resizeEvent(QResizeEvent* event) override;

    const QMap<QString, QAction*>& actions;
    QMap<QString, QToolButton*> actionButtons;
    QStringList missingKeys;
    QList<PanelInstance> panels;
    SARibbonBar* tabs {nullptr};
    bool updatingPanels {false};
    bool panelUpdatePending {false};
    QMenu* applicationMenu {nullptr};
    bool workspaceActionsAdded {false};
    QToolBar* penToolbar {nullptr};
    QToolBar* optionToolbar {nullptr};
    Qt::Orientation penToolbarOriginalOrientation {Qt::Horizontal};
    bool penToolbarOriginalMovable {true};
    bool penToolbarOriginalFloatable {true};
    QList<QAction*> penToolbarOriginalActions;
    QFrame* penToolbarHost {nullptr};
    QFrame* optionToolbarHost {nullptr};
    QGridLayout* penToolbarLayout {nullptr};
    QGridLayout* optionToolbarLayout {nullptr};
    QFrame* currentLayerHost {nullptr};
    QGridLayout* currentLayerLayout {nullptr};
    QWidget* currentLayerSelector {nullptr};
    QPointer<QWidget> currentLayerSelectorOriginalParent;
    bool currentLayerSelectorWasVisible {false};
};

#endif // KUUBIKRIBBON_H
