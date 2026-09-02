/****************************************************************************
**
** Kuubik Draw native current-layer selector.
**
** This file is part of Kuubik Draw, a GPLv2 fork of LibreCAD.
** It may be distributed and/or modified under the terms of the GNU General
** Public License version 2 as published by the Free Software Foundation.
**
****************************************************************************/

#ifndef KUUBIKCURRENTLAYERSELECTOR_H
#define KUUBIKCURRENTLAYERSELECTOR_H

#include <QComboBox>

#include "rs_layerlistlistener.h"

class RS_Layer;
class RS_LayerList;

/**
 * A native view of the active document's layer list.
 *
 * The selector owns no layer state. Selecting an item activates the stored
 * RS_Layer through RS_LayerList, which remains the authoritative source and
 * notifies all existing LibreCAD layer listeners.
 */
class KuubikCurrentLayerSelector : public QComboBox, public RS_LayerListListener
{
    Q_OBJECT

public:
    explicit KuubikCurrentLayerSelector(QWidget* parent = nullptr);
    ~KuubikCurrentLayerSelector() override;

    void setLayerList(RS_LayerList* layerList);
    RS_LayerList* layerList() const;
    QString currentLayerName() const;

    void layerActivated(RS_Layer* layer) override;
    void layerAdded(RS_Layer*) override;
    void layerEdited(RS_Layer*) override;
    void layerRemoved(RS_Layer*) override;
    void layerToggled(RS_Layer*) override;
    void layerToggledLock(RS_Layer*) override;
    void layerToggledPrint(RS_Layer*) override;
    void layerToggledConstruction(RS_Layer*) override;
    void layerListModified(bool) override;

signals:
    void layerStateChanged();

private:
    void rebuildItems();
    void activateIndex(int index);

    RS_LayerList* currentLayerList {nullptr};
};

#endif // KUUBIKCURRENTLAYERSELECTOR_H
