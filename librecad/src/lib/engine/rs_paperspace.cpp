// SPDX-License-Identifier: GPL-2.0-only
#include "rs_paperspace.h"
#include <cmath>

namespace {
bool planar(const RS_Vector& p) {
    return static_cast<bool>(p) && std::isfinite(p.x) && std::isfinite(p.y) && p.z == 0;
}
bool positive(double value) { return std::isfinite(value) && value > 0; }
bool finite(const QTransform& m) {
    return std::isfinite(m.m11()) && std::isfinite(m.m12()) && std::isfinite(m.m21())
        && std::isfinite(m.m22()) && std::isfinite(m.dx()) && std::isfinite(m.dy());
}
// 1e-6 millimeters in paper space. Reject cameras whose double-precision
// coordinates cannot retain that accuracy instead of silently shifting a plot.
bool near(QPointF a, QPointF b) {
    return std::hypot(a.x() - b.x(), a.y() - b.y()) <= 1e-6;
}
}

std::optional<QRectF> RS_PaperViewport::paperFrame() const {
    if (!planar(center) || !positive(width) || !positive(height)) return std::nullopt;
    const QRectF frame(center.x - width / 2, center.y - height / 2, width, height);
    if (!std::isfinite(frame.left()) || !std::isfinite(frame.right())
        || !std::isfinite(frame.top()) || !std::isfinite(frame.bottom())
        || frame.left() >= frame.right() || frame.top() >= frame.bottom()
        || !near(QPointF(frame.right() - frame.left(), frame.bottom() - frame.top()), QPointF(width, height))
        || !near(frame.center(), QPointF(center.x, center.y))) return std::nullopt;
    return frame;
}

std::optional<QTransform> RS_PaperViewport::cameraTransform() const {
    const auto frame = paperFrame();
    if (!frame || !planar(viewCenter) || !positive(viewHeight) || !std::isfinite(twist)) return std::nullopt;
    const double scale = height / viewHeight;
    if (!positive(scale)) return std::nullopt;
    QTransform transform;
    transform.translate(center.x, center.y).rotateRadians(-twist).scale(scale, scale).translate(-viewCenter.x, -viewCenter.y);
    bool ok = false;
    const auto inverse = transform.inverted(&ok);
    if (!ok || !positive(transform.determinant()) || !finite(transform) || !finite(inverse)
        || !near(transform.map(QPointF(viewCenter.x, viewCenter.y)), QPointF(center.x, center.y))) return std::nullopt;
    for (const auto& point : {frame->center(), frame->topLeft(), frame->topRight(), frame->bottomLeft(), frame->bottomRight()})
        if (!near(transform.map(inverse.map(point)), point)) return std::nullopt;
    return transform;
}
