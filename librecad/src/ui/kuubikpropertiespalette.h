/****************************************************************************
**
** Kuubik Draw read-only native properties palette.
**
** This file is part of Kuubik Draw, a GPLv2 fork of LibreCAD.
** It may be distributed and/or modified under the terms of the GNU General
** Public License version 2 as published by the Free Software Foundation.
**
****************************************************************************/

#ifndef KUUBIKPROPERTIESPALETTE_H
#define KUUBIKPROPERTIESPALETTE_H

#include <QVariant>
#include <QWidget>

class QLabel;
class QFormLayout;
class QAction;
class RS_Document;

/**
 * Read-only summary of the active native document and its selection.
 *
 * RS_Entity instances are inspected only while refreshSelection() runs; the
 * palette deliberately does not retain entity pointers or edit document data.
 */
class KuubikPropertiesPalette : public QWidget
{
public:
    explicit KuubikPropertiesPalette(QWidget* parent = nullptr);

    void setDocument(RS_Document* document);
    RS_Document* document() const;
    void refreshSelection(int selectedCount, double totalLength);
    QVariantMap state() const;
    void setModifyEntityAction(QAction* action);

private:
    void refreshDocument();
    void clearEntityDetails();
    void setValue(QLabel* label, const QString& value);

    RS_Document* currentDocument {nullptr};
    int selectionCount {0};
    double selectionLength {0.0};
    int selectionRefreshGeneration {0};

    QLabel* documentValue {nullptr};
    QLabel* modifiedValue {nullptr};
    QLabel* entityCountValue {nullptr};
    QLabel* currentLayerValue {nullptr};
    QLabel* selectionValue {nullptr};
    QLabel* totalLengthValue {nullptr};
    QLabel* entityTypeValue {nullptr};
    QLabel* entityLayerValue {nullptr};
    QLabel* colorValue {nullptr};
    QLabel* lineTypeValue {nullptr};
    QLabel* lineWeightValue {nullptr};
    QFormLayout* formLayout {nullptr};
};

#endif // KUUBIKPROPERTIESPALETTE_H
