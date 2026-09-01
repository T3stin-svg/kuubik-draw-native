/****************************************************************************
**
** Kuubik Draw visual design tokens and theme application.
**
** This file is part of Kuubik Draw, a GPLv2 fork of LibreCAD.
** It may be distributed and/or modified under the terms of the GNU General
** Public License version 2 as published by the Free Software Foundation.
**
****************************************************************************/

#ifndef KUUBIKTHEME_H
#define KUUBIKTHEME_H

#include <QMap>
#include <QString>

class KuubikTheme
{
public:
    static QMap<QString, QString> colors();
    static QString color(const QString& name);
    static bool apply();
    static void applyCanvasSettings();

    static int ribbonMinimumHeight();
    static int ribbonMaximumHeight();
    static int commandMinimumHeight();
    static int commandMaximumHeight();
    static int statusBarHeight();
};

#endif // KUUBIKTHEME_H
