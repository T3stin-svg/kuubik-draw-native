/****************************************************************************
**
** Kuubik Draw native current-layer selector.
**
** This file is part of Kuubik Draw, a GPLv2 fork of LibreCAD.
** It may be distributed and/or modified under the terms of the GNU General
** Public License version 2 as published by the Free Software Foundation.
**
****************************************************************************/

#include "kuubikcurrentlayerselector.h"

#include <QSignalBlocker>
#include "rs_layer.h"
#include "rs_layerlist.h"

KuubikCurrentLayerSelector::KuubikCurrentLayerSelector(QWidget* parent)
    : QComboBox(parent)
{
    setObjectName(QStringLiteral("kuubikCurrentLayerSelector"));
    setSizeAdjustPolicy(QComboBox::AdjustToContents);
    setMinimumContentsLength(12);
    setToolTip(tr("Current layer"));
    setEnabled(false);

    connect(this, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this](int index) { activateIndex(index); });
}

KuubikCurrentLayerSelector::~KuubikCurrentLayerSelector()
{
    setLayerList(nullptr);
}

void KuubikCurrentLayerSelector::setLayerList(RS_LayerList* layerList)
{
    if (currentLayerList == layerList) {
        rebuildItems();
        return;
    }

    if (currentLayerList != nullptr) {
        currentLayerList->removeListener(this);
    }

    currentLayerList = layerList;
    if (currentLayerList != nullptr) {
        currentLayerList->addListener(this);
    }
    rebuildItems();
    emit layerStateChanged();
}

RS_LayerList* KuubikCurrentLayerSelector::layerList() const
{
    return currentLayerList;
}

QString KuubikCurrentLayerSelector::currentLayerName() const
{
    RS_Layer* activeLayer = currentLayerList == nullptr
                                ? nullptr
                                : currentLayerList->getActive();
    return activeLayer == nullptr ? QString() : activeLayer->getName();
}

void KuubikCurrentLayerSelector::layerActivated(RS_Layer*)
{
    rebuildItems();
    emit layerStateChanged();
}

void KuubikCurrentLayerSelector::layerAdded(RS_Layer*)
{
    rebuildItems();
    emit layerStateChanged();
}

void KuubikCurrentLayerSelector::layerEdited(RS_Layer*)
{
    rebuildItems();
    emit layerStateChanged();
}

void KuubikCurrentLayerSelector::layerRemoved(RS_Layer*)
{
    rebuildItems();
    emit layerStateChanged();
}

void KuubikCurrentLayerSelector::layerToggled(RS_Layer*)
{
    rebuildItems();
}

void KuubikCurrentLayerSelector::layerToggledLock(RS_Layer*)
{
    rebuildItems();
}

void KuubikCurrentLayerSelector::layerToggledPrint(RS_Layer*)
{
    rebuildItems();
}

void KuubikCurrentLayerSelector::layerToggledConstruction(RS_Layer*)
{
    rebuildItems();
}

void KuubikCurrentLayerSelector::layerListModified(bool)
{
    rebuildItems();
    emit layerStateChanged();
}

void KuubikCurrentLayerSelector::rebuildItems()
{
    const QSignalBlocker signalBlocker(this);
    clear();

    if (currentLayerList == nullptr) {
        setEnabled(false);
        return;
    }

    RS_Layer* activeLayer = currentLayerList->getActive();
    int activeIndex = -1;
    for (unsigned index = 0; index < currentLayerList->count(); ++index) {
        RS_Layer* layer = currentLayerList->at(index);
        if (layer == nullptr) {
            continue;
        }
        addItem(layer->getName(), layer->getName());
        if (layer == activeLayer) {
            activeIndex = count() - 1;
        }
    }

    setEnabled(count() > 0);
    setCurrentIndex(activeIndex);
}

void KuubikCurrentLayerSelector::activateIndex(int index)
{
    if (currentLayerList == nullptr || index < 0) {
        return;
    }

    RS_Layer* layer = currentLayerList->find(itemData(index).toString());
    if (layer != nullptr && layer != currentLayerList->getActive()) {
        currentLayerList->activate(layer, true);
    }
}
