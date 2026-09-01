/*
 * Kuubik Draw original technical-line icon registry.
 * SPDX-License-Identifier: GPL-2.0-only
 */
#ifndef KUUBIKICONREGISTRY_H
#define KUUBIKICONREGISTRY_H

#include <QIcon>
#include <QString>

class KuubikIconRegistry
{
public:
    static QIcon iconForAction(const QString& actionKey);
    static QString resourceForAction(const QString& actionKey);
};

#endif // KUUBIKICONREGISTRY_H
