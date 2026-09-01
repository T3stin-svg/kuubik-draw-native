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

#include <QMap>
#include <QList>
#include <QStringList>
#include <QWidget>

class QAction;
class QFrame;
class QGridLayout;
class QMainWindow;
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

private:
    struct GroupSpec {
        QString title;
        QStringList actionKeys;
    };

    QFrame* createActionGroup(const GroupSpec& spec, QWidget* parent);
    QToolButton* createActionButton(const QString& key, QWidget* parent,
                                    bool iconOnly = false);
    QWidget* createPage(const QList<GroupSpec>& groups);
    QFrame* createEmbeddedToolbarGroup(const QString& title,
                                       QToolBar* toolbar,
                                       QWidget* parent);

    const QMap<QString, QAction*>& actions;
    QMap<QString, QToolButton*> actionButtons;
    QStringList missingKeys;
    QTabWidget* tabs {nullptr};
    QToolBar* penToolbar {nullptr};
    QToolBar* optionToolbar {nullptr};
    QFrame* penToolbarHost {nullptr};
    QFrame* optionToolbarHost {nullptr};
    QGridLayout* penToolbarLayout {nullptr};
    QGridLayout* optionToolbarLayout {nullptr};
};

#endif // KUUBIKRIBBON_H
