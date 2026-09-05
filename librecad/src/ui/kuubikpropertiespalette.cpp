/****************************************************************************
**
** Kuubik Draw read-only native properties palette.
**
** This file is part of Kuubik Draw, a GPLv2 fork of LibreCAD.
** It may be distributed and/or modified under the terms of the GNU General
** Public License version 2 as published by the Free Software Foundation.
**
****************************************************************************/

#include "kuubikpropertiespalette.h"

#include <algorithm>
#include <QAction>
#include <QFormLayout>
#include <QFileInfo>
#include <QLabel>
#include <QObject>
#include <QToolButton>

#include "lc_peninforegistry.h"
#include "rs_document.h"
#include "rs_entity.h"
#include "rs_layer.h"
#include "rs_layerlist.h"
#include "rs_pen.h"

namespace {
QString entityTypeText(RS2::EntityType type)
{
    switch (type) {
    case RS2::EntityPoint: return QObject::tr("Point");
    case RS2::EntityLine: return QObject::tr("Line");
    case RS2::EntityPolyline: return QObject::tr("Polyline");
    case RS2::EntityArc: return QObject::tr("Arc");
    case RS2::EntityCircle: return QObject::tr("Circle");
    case RS2::EntityEllipse: return QObject::tr("Ellipse");
    case RS2::EntityHyperbola: return QObject::tr("Hyperbola");
    case RS2::EntityParabola: return QObject::tr("Parabola");
    case RS2::EntitySpline: return QObject::tr("Spline");
    case RS2::EntitySplinePoints: return QObject::tr("Spline points");
    case RS2::EntityText: return QObject::tr("Text");
    case RS2::EntityMText: return QObject::tr("Multiline text");
    case RS2::EntityInsert: return QObject::tr("Block insert");
    case RS2::EntityHatch: return QObject::tr("Hatch");
    case RS2::EntityImage: return QObject::tr("Image");
    case RS2::EntityDimAligned: return QObject::tr("Aligned dimension");
    case RS2::EntityDimLinear: return QObject::tr("Linear dimension");
    case RS2::EntityDimRadial: return QObject::tr("Radial dimension");
    case RS2::EntityDimDiametric: return QObject::tr("Diametric dimension");
    case RS2::EntityDimAngular: return QObject::tr("Angular dimension");
    case RS2::EntityDimArc: return QObject::tr("Arc dimension");
    case RS2::EntityDimLeader: return QObject::tr("Leader");
    case RS2::EntitySolid: return QObject::tr("Solid");
    case RS2::EntityConstructionLine: return QObject::tr("Construction line");
    default: return QObject::tr("Entity");
    }
}

QString formatLength(double length)
{
    return QString::number(length, 'g', 8);
}
}

KuubikPropertiesPalette::KuubikPropertiesPalette(QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("kuubikPropertiesPalette"));
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    formLayout = new QFormLayout(this);
    formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);

    auto addValue = [this](const QString& label) {
        auto* value = new QLabel(this);
        value->setTextInteractionFlags(Qt::TextSelectableByMouse);
        value->setWordWrap(true);
        formLayout->addRow(label, value);
        return value;
    };

    documentValue = addValue(tr("Document"));
    modifiedValue = addValue(tr("Modified"));
    entityCountValue = addValue(tr("Entities"));
    currentLayerValue = addValue(tr("Current layer"));
    selectionValue = addValue(tr("Selection"));
    totalLengthValue = addValue(tr("Total length"));
    entityTypeValue = addValue(tr("Type"));
    entityLayerValue = addValue(tr("Layer"));
    colorValue = addValue(tr("Color"));
    lineTypeValue = addValue(tr("Linetype"));
    lineWeightValue = addValue(tr("Lineweight"));

    setDocument(nullptr);
}

void KuubikPropertiesPalette::setModifyEntityAction(QAction* action)
{
    if (formLayout == nullptr || action == nullptr
        || findChild<QToolButton*>(QStringLiteral("kuubikOpenFullProperties")) != nullptr) {
        return;
    }
    auto* button = new QToolButton(this);
    button->setObjectName(QStringLiteral("kuubikOpenFullProperties"));
    button->setDefaultAction(action);
    button->setText(tr("Open Full Properties"));
    button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    button->setAccessibleName(action->text().remove('&'));
    formLayout->addRow(button);
}

void KuubikPropertiesPalette::setDocument(RS_Document* document)
{
    currentDocument = document;
    selectionCount = 0;
    selectionLength = 0.0;
    refreshDocument();
}

RS_Document* KuubikPropertiesPalette::document() const
{
    return currentDocument;
}

void KuubikPropertiesPalette::refreshSelection(int selectedCount, double totalLength)
{
    ++selectionRefreshGeneration;
    selectionCount = qMax(0, selectedCount);
    selectionLength = selectionCount > 0 ? qMax(0.0, totalLength) : 0.0;
    refreshDocument();
}

QVariantMap KuubikPropertiesPalette::state() const
{
    QVariantMap result;
    const QString mode = currentDocument == nullptr
                             ? QStringLiteral("none")
                             : selectionCount == 0
                                   ? QStringLiteral("document")
                                   : selectionCount == 1
                                         ? QStringLiteral("single")
                                         : QStringLiteral("multiple");
    result.insert(QStringLiteral("mode"), mode);
    result.insert(QStringLiteral("selectionCount"), selectionCount);
    result.insert(QStringLiteral("totalLength"), selectionLength);
    result.insert(QStringLiteral("selectionRefreshGeneration"), selectionRefreshGeneration);
    result.insert(QStringLiteral("document"), documentValue == nullptr ? QString() : documentValue->text());
    result.insert(QStringLiteral("modified"), modifiedValue == nullptr ? QString() : modifiedValue->text());
    result.insert(QStringLiteral("entityCount"), entityCountValue == nullptr ? QString() : entityCountValue->text());
    result.insert(QStringLiteral("currentLayer"), currentLayerValue == nullptr ? QString() : currentLayerValue->text());
    result.insert(QStringLiteral("type"), entityTypeValue == nullptr ? QString() : entityTypeValue->text());
    result.insert(QStringLiteral("layer"), entityLayerValue == nullptr ? QString() : entityLayerValue->text());
    result.insert(QStringLiteral("color"), colorValue == nullptr ? QString() : colorValue->text());
    result.insert(QStringLiteral("linetype"), lineTypeValue == nullptr ? QString() : lineTypeValue->text());
    result.insert(QStringLiteral("lineweight"), lineWeightValue == nullptr ? QString() : lineWeightValue->text());
    result.insert(QStringLiteral("summary"), selectionValue == nullptr ? QString() : selectionValue->text());
    return result;
}

void KuubikPropertiesPalette::refreshDocument()
{
    if (currentDocument == nullptr) {
        setValue(documentValue, tr("No document"));
        setValue(modifiedValue, QString());
        setValue(entityCountValue, QStringLiteral("0"));
        setValue(currentLayerValue, QString());
        setValue(selectionValue, QStringLiteral("0"));
        setValue(totalLengthValue, QString());
        clearEntityDetails();
        return;
    }

    const QString filename = currentDocument->getFilename();
    setValue(documentValue, filename.isEmpty() ? tr("Untitled") : QFileInfo(filename).fileName());
    setValue(modifiedValue, currentDocument->isModified() ? tr("Yes") : tr("No"));
    const auto& entities = currentDocument->getEntityList();
    setValue(entityCountValue, QString::number(std::count_if(
        entities.begin(), entities.end(), [](const RS_Entity* entity) {
            return entity != nullptr && !entity->isUndone();
        })));

    RS_LayerList* layerList = currentDocument->getLayerList();
    RS_Layer* activeLayer = layerList == nullptr ? nullptr : layerList->getActive();
    setValue(currentLayerValue, activeLayer == nullptr ? QString() : activeLayer->getName());
    setValue(selectionValue, QString::number(selectionCount));
    setValue(totalLengthValue, selectionCount > 0 ? formatLength(selectionLength) : QString());

    if (selectionCount != 1) {
        clearEntityDetails();
        return;
    }

    RS_Entity* selectedEntity = nullptr;
    for (RS_Entity* entity = currentDocument->firstEntity(RS2::ResolveAll);
         entity != nullptr;
         entity = currentDocument->nextEntity(RS2::ResolveAll)) {
        if (entity->isSelected()) {
            selectedEntity = entity;
            break;
        }
    }

    if (selectedEntity == nullptr) {
        clearEntityDetails();
        return;
    }

    setValue(entityTypeValue, entityTypeText(selectedEntity->rtti()));
    RS_Layer* entityLayer = selectedEntity->getLayer(true);
    setValue(entityLayerValue, entityLayer == nullptr ? QString() : entityLayer->getName());

    const RS_Pen pen = selectedEntity->getPen(true);
    LC_PenInfoRegistry* penInfo = LC_PenInfoRegistry::instance();
    setValue(colorValue, penInfo->getColorName(pen.getColor(), LC_PenInfoRegistry::HEX));
    setValue(lineTypeValue, penInfo->getLineTypeText(pen.getLineType()));
    setValue(lineWeightValue, penInfo->getLineWidthText(pen.getWidth()));
}

void KuubikPropertiesPalette::clearEntityDetails()
{
    setValue(entityTypeValue, QString());
    setValue(entityLayerValue, QString());
    setValue(colorValue, QString());
    setValue(lineTypeValue, QString());
    setValue(lineWeightValue, QString());
}

void KuubikPropertiesPalette::setValue(QLabel* label, const QString& value)
{
    if (label != nullptr) {
        label->setText(value);
    }
}
