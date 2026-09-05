// SPDX-License-Identifier: GPL-2.0-only
#ifndef RS_PAPERSPACE_H
#define RS_PAPERSPACE_H

#include "rs_vector.h"
#include <QString>
#include <array>
#include <tuple>
#include <vector>

// Native 2D metadata only. Every viewport refers to the same RS_Graphic geometry.
// Paper coordinates are millimeters; camera coordinates use drawing units.
struct RS_PaperViewport {
    quint64 id = 0;
    RS_Vector center{0, 0};
    double width = 160, height = 160;
    RS_Vector viewCenter{0, 0};
    double viewHeight = 8000, twist = 0; // radians
    bool enabled = true, locked = false;

    bool operator==(const RS_PaperViewport& b) const {
        return std::tie(id, center.x, center.y, center.z, width, height,
                        viewCenter.x, viewCenter.y, viewCenter.z, viewHeight, twist, enabled, locked)
            == std::tie(b.id, b.center.x, b.center.y, b.center.z, b.width, b.height,
                        b.viewCenter.x, b.viewCenter.y, b.viewCenter.z, b.viewHeight, b.twist, b.enabled, b.locked);
    }
};

struct RS_PaperLayout {
    quint64 id = 0;
    QString name;
    double width = 420, height = 297;
    std::array<double, 4> margins{{0, 0, 0, 0}}; // left, bottom, right, top in mm
    std::vector<RS_PaperViewport> viewports;

    bool operator==(const RS_PaperLayout& b) const {
        return std::tie(id, name, width, height, margins, viewports)
            == std::tie(b.id, b.name, b.width, b.height, b.margins, b.viewports);
    }
};

using RS_PaperSpace = std::vector<RS_PaperLayout>;

#endif
