/****************************************************************************
**
** Kuubik Draw visual design tokens and theme application.
**
** This file is part of Kuubik Draw, a GPLv2 fork of LibreCAD.
** It may be distributed and/or modified under the terms of the GNU General
** Public License version 2 as published by the Free Software Foundation.
**
****************************************************************************/

#include "kuubiktheme.h"

#include <QApplication>
#include <QFile>
#include <QFont>
#include <QStyleFactory>

#include "rs_settings.h"

QMap<QString, QString> KuubikTheme::colors()
{
    return {
        {"shell", "#222933"},
        {"ribbon", "#3B4453"},
        {"control", "#4E5A6E"},
        {"canvas", "#1E2225"},
        {"accent", "#168DCE"},
        {"active", "#176F9F"},
        {"highlight", "#4CC2FF"},
        {"primaryText", "#E8EBEF"},
        {"mutedText", "#A7B0B7"}
    };
}

QString KuubikTheme::color(const QString& name)
{
    return colors().value(name);
}

bool KuubikTheme::apply()
{
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    QFile file(":/main/kuubik-dark.qss");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    qApp->setStyleSheet(QString::fromUtf8(file.readAll()));
    qApp->setFont(QFont("Segoe UI", 9));
    return true;
}

void KuubikTheme::applyCanvasSettings()
{
    RS_SETTINGS->beginGroup("Colors");
    RS_SETTINGS->writeEntry("/background", color("canvas"));
    RS_SETTINGS->writeEntry("/grid", QStringLiteral("#3A4650"));
    RS_SETTINGS->writeEntry("/meta_grid", QStringLiteral("#52606B"));
    RS_SETTINGS->writeEntry("/select", color("accent"));
    RS_SETTINGS->writeEntry("/highlight", color("highlight"));
    RS_SETTINGS->writeEntry("/start_handle", color("highlight"));
    RS_SETTINGS->writeEntry("/handle", color("accent"));
    RS_SETTINGS->writeEntry("/end_handle", color("highlight"));
    RS_SETTINGS->endGroup();
}

int KuubikTheme::ribbonMinimumHeight()
{
    // Logical pixels: keep two compact command rows usable at 100-150% DPI.
    return 148;
}

int KuubikTheme::ribbonMaximumHeight()
{
    return 152;
}

int KuubikTheme::commandMinimumHeight()
{
    return 70;
}

int KuubikTheme::commandMaximumHeight()
{
    return 82;
}

int KuubikTheme::statusBarHeight()
{
    return 27;
}
