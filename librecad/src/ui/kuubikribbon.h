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
#include <QMap>
#include <QPointer>
#include <QStringList>
#include <QWidget>

class QAction;
class QFrame;
class QGridLayout;
class QMainWindow;
class QResizeEvent;
class QTabWidget;
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
        QFrame* frame {nullptr};
        QList<QToolButton*> itemButtons;
        QToolButton* overflowButton {nullptr};
        int collapsePriority {0};
        bool collapsed {false};
    };

    QFrame* createActionGroup(const PanelSpec& spec, QWidget* parent, QWidget* page);
    QToolButton* createActionButton(const QString& key, QWidget* parent,
                                    ItemSize size = ItemSize::Small,
                                    bool iconOnly = false);
    QWidget* createPage(const TabSpec& spec);
    QFrame* createEmbeddedToolbarGroup(const QString& title,
                                       QToolBar* toolbar,
                                       QWidget* parent);
    QFrame* createCurrentLayerHost(QWidget* parent);
    void updateCollapsedPanels();
    void setPanelCollapsed(PanelInstance& panel, bool collapsed);
    void resizeEvent(QResizeEvent* event) override;

    const QMap<QString, QAction*>& actions;
    QMap<QString, QToolButton*> actionButtons;
    QStringList missingKeys;
    QList<PanelInstance> panels;
    QTabWidget* tabs {nullptr};
    QToolBar* penToolbar {nullptr};
    QToolBar* optionToolbar {nullptr};
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
