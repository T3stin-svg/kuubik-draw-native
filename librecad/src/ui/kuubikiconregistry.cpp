/*
 * Kuubik Draw original technical-line icon registry.
 * SPDX-License-Identifier: GPL-2.0-only
 */
#include "kuubikiconregistry.h"

#include <QByteArray>
#include <cstring>

namespace {
struct IconMapping {
    const char* actionKey;
    const char* resource;
};

// Grouped by compact-ribbon panel.  This table is deliberately data-only: it
// does not change, connect, trigger, or otherwise alter the underlying action.
const IconMapping ICON_MAPPINGS[] = {
    {"FileNew", ":/icons/kuubik/qat/file-new.svg"},
    {"FileOpen", ":/icons/kuubik/qat/file-open.svg"},
    {"FileSave", ":/icons/kuubik/qat/file-save.svg"},
    {"FileSaveAs", ":/icons/kuubik/qat/file-save-as.svg"},
    {"EditUndo", ":/icons/kuubik/qat/edit-undo.svg"},
    {"EditRedo", ":/icons/kuubik/qat/edit-redo.svg"},
    {"FilePrint", ":/icons/kuubik/qat/file-print.svg"},

    {"DrawLine", ":/icons/kuubik/draw/draw-line.svg"},
    {"DrawPolyline", ":/icons/kuubik/draw/draw-polyline.svg"},
    {"DrawLineRectangle", ":/icons/kuubik/draw/draw-rectangle.svg"},
    {"DrawCircle", ":/icons/kuubik/draw/draw-circle.svg"},
    {"DrawArc", ":/icons/kuubik/draw/draw-arc.svg"},
    {"DrawHatch", ":/icons/kuubik/draw/draw-hatch.svg"},

    {"ModifyMove", ":/icons/kuubik/modify/modify-move.svg"},
    {"ModifyDuplicate", ":/icons/kuubik/modify/modify-duplicate.svg"},
    {"ModifyTrim", ":/icons/kuubik/modify/modify-trim.svg"},
    {"ModifyTrim2", ":/icons/kuubik/modify/modify-trim2.svg"},
    {"ModifyCut", ":/icons/kuubik/modify/modify-cut.svg"},
    {"ModifyOffset", ":/icons/kuubik/modify/modify-offset.svg"},
    {"ModifyRotate", ":/icons/kuubik/modify/modify-rotate.svg"},
    {"ModifyMirror", ":/icons/kuubik/modify/modify-mirror.svg"},
    {"ModifyScale", ":/icons/kuubik/modify/modify-scale.svg"},
    {"ModifyRound", ":/icons/kuubik/modify/modify-round.svg"},
    {"ModifyDeleteQuick", ":/icons/kuubik/modify/modify-delete-quick.svg"},

    {"DrawMText", ":/icons/kuubik/annotation/draw-mtext.svg"},
    {"DrawText", ":/icons/kuubik/annotation/draw-text.svg"},
    {"DimLinear", ":/icons/kuubik/annotation/dim-linear.svg"},
    {"DimAligned", ":/icons/kuubik/annotation/dim-aligned.svg"},
    {"DimLinearHor", ":/icons/kuubik/annotation/dim-linear-hor.svg"},
    {"DimLinearVer", ":/icons/kuubik/annotation/dim-linear-ver.svg"},
    {"DimRadial", ":/icons/kuubik/annotation/dim-radial.svg"},
    {"DimDiametric", ":/icons/kuubik/annotation/dim-diametric.svg"},
    {"DimAngular", ":/icons/kuubik/annotation/dim-angular.svg"},
    {"DimLeader", ":/icons/kuubik/annotation/dim-leader.svg"},

    {"LayersAdd", ":/icons/kuubik/layers/layers-add.svg"},
    {"LayersEdit", ":/icons/kuubik/layers/layers-edit.svg"},
    {"LayersToggleView", ":/icons/kuubik/layers/layers-toggle-view.svg"},
    {"LayersToggleLock", ":/icons/kuubik/layers/layers-toggle-lock.svg"},

    {"BlocksInsert", ":/icons/kuubik/blocks/blocks-insert.svg"},
    {"BlocksCreate", ":/icons/kuubik/blocks/blocks-create.svg"},
    {"BlocksEdit", ":/icons/kuubik/blocks/blocks-edit.svg"},
    {"BlocksExplode", ":/icons/kuubik/blocks/blocks-explode.svg"},
    {"BlocksImport", ":/icons/kuubik/blocks/blocks-import.svg"},

    {"ModifyEntity", ":/icons/kuubik/properties/modify-entity.svg"},
    {"PenSyncFromLayer", ":/icons/kuubik/properties/pen-sync-layer.svg"},
    {"PenPick", ":/icons/kuubik/properties/pen-pick.svg"},
    {"PenPickResolved", ":/icons/kuubik/properties/pen-pick-resolved.svg"},
    {"PenApply", ":/icons/kuubik/properties/pen-apply.svg"},
    {"PenCopy", ":/icons/kuubik/properties/pen-copy.svg"},

    {"InfoDist", ":/icons/kuubik/utilities/info-dist.svg"},
    {"InfoAngle", ":/icons/kuubik/utilities/info-angle.svg"},
    {"InfoArea", ":/icons/kuubik/utilities/info-area.svg"},
    {"InfoTotalLength", ":/icons/kuubik/utilities/info-total-length.svg"},

    {"EditCut", ":/icons/kuubik/clipboard/edit-cut.svg"},
    {"EditCopy", ":/icons/kuubik/clipboard/edit-copy.svg"},
    {"EditPaste", ":/icons/kuubik/clipboard/edit-paste.svg"},

    {"ViewGrid", ":/icons/kuubik/view/view-grid.svg"},
    {"RestrictOrthogonal", ":/icons/kuubik/view/restrict-orthogonal.svg"},
    {"SnapEnd", ":/icons/kuubik/view/snap-end.svg"},
    {"SnapMiddle", ":/icons/kuubik/view/snap-middle.svg"},
    {"SnapCenter", ":/icons/kuubik/view/snap-center.svg"},
    {"SnapIntersection", ":/icons/kuubik/view/snap-intersection.svg"},
    {"ZoomIn", ":/icons/kuubik/view/zoom-in.svg"},
    {"ZoomOut", ":/icons/kuubik/view/zoom-out.svg"},
    {"ZoomPrevious", ":/icons/kuubik/view/zoom-previous.svg"},
    {"ZoomWindow", ":/icons/kuubik/view/zoom-window.svg"},
    {"ZoomPan", ":/icons/kuubik/view/zoom-pan.svg"},
    {"ZoomAuto", ":/icons/kuubik/view/zoom-auto.svg"},

    {"FileExport", ":/icons/kuubik/output/file-export.svg"},
    {"FilePrintPreview", ":/icons/kuubik/output/file-print-preview.svg"},
    {"FilePrintPDF", ":/icons/kuubik/output/file-print-pdf.svg"}
};
}

QString KuubikIconRegistry::resourceForAction(const QString& actionKey)
{
    const QByteArray key = actionKey.toLatin1();
    for (const IconMapping& mapping : ICON_MAPPINGS) {
        if (std::strcmp(mapping.actionKey, key.constData()) == 0) {
            return QString::fromLatin1(mapping.resource);
        }
    }
    return QString();
}

QIcon KuubikIconRegistry::iconForAction(const QString& actionKey)
{
    const QString resource = resourceForAction(actionKey);
    return resource.isEmpty() ? QIcon() : QIcon(resource);
}
