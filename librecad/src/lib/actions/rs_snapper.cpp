/****************************************************************************
**
** This file is part of the LibreCAD project, a 2D CAD program
**
** Copyright (C) 2010 R. van Twisk (librecad@rvt.dds.nl)
** Copyright (C) 2001-2003 RibbonSoft. All rights reserved.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file gpl-2.0.txt included in the
** packaging of this file.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**
** This copyright notice MUST APPEAR in all copies of the script!
**
**********************************************************************/


#include<cmath>

#include<QMouseEvent>

#include "rs_snapper.h"

#include "rs_circle.h"
#include "rs_coordinateevent.h"
#include "rs_debug.h"
#include "rs_dialogfactory.h"
#include "rs_entitycontainer.h"
#include "rs_graphicview.h"
#include "rs_grid.h"
#include "rs_information.h"
#include "rs_line.h"
#include "rs_overlayline.h"
#include "rs_pen.h"
#include "rs_point.h"
#include "rs_settings.h"

namespace {

    // whether a floating point is positive by tolerance
    bool isPositive(double x)
    {
        return x > RS_TOLERANCE;
    }

    // A size vector is valid with a positive size
    bool isSizeValid(const RS_Vector& sizeVector) {
        return isPositive(sizeVector.x) || isPositive(sizeVector.x);
    }

    // The valid size magnitude
    double getValidSize(const RS_Vector& sizeVector)
    {
        return std::hypot(std::max(sizeVector.x, RS_TOLERANCE), std::max(sizeVector.y, RS_TOLERANCE));
    }

    // get catching entity distance in graph distance
    double getCatchDistance(double catchDistance, int catchEntityGuiRange, RS_GraphicView* view)
    {
        return (view != nullptr) ? std::min(catchDistance, view->toGraphDX(catchEntityGuiRange)) : catchDistance;
    }
}

/**
  * Disable all snapping.
  *
  * This effectively puts the object into free snap mode.
  *
  * @returns A reference to itself.
  */
RS_SnapMode const & RS_SnapMode::clear()
{
    snapIntersection    = false;
    snapOnEntity        = false;
    snapCenter          = false;
    snapDistance        = false;
    snapMiddle          = false;
    snapEndpoint        = false;
    snapGrid            = false;
    snapFree            = false;
    snapAngle           = false;
    snapQuadrant = snapNode = snapInsertion = false;
    snapPerpendicular = snapTangent = snapGeometricCenter = false;
    snapApparentIntersection = snapExtension = snapParallel = false;
    snapTracking = false;

    restriction = RS2::RestrictNothing;

    return *this;
}

bool RS_SnapMode::operator ==(RS_SnapMode const& rhs) const
{
    return snapIntersection == rhs.snapIntersection
            && snapOnEntity == rhs.snapOnEntity
            && snapCenter   == rhs.snapCenter
            && snapDistance == rhs.snapDistance
            && snapMiddle   == rhs.snapMiddle
            && snapEndpoint == rhs.snapEndpoint
            && snapGrid     == rhs.snapGrid
            && snapFree     == rhs.snapFree
            && restriction  == rhs.restriction
            && snapAngle    == rhs.snapAngle
            && snapQuadrant == rhs.snapQuadrant
            && snapNode == rhs.snapNode
            && snapInsertion == rhs.snapInsertion
            && snapPerpendicular == rhs.snapPerpendicular
            && snapTangent == rhs.snapTangent
            && snapGeometricCenter == rhs.snapGeometricCenter
            && snapApparentIntersection == rhs.snapApparentIntersection
            && snapExtension == rhs.snapExtension
            && snapParallel == rhs.snapParallel
            && snapTracking == rhs.snapTracking;
}

bool RS_SnapMode::operator !=(RS_SnapMode const& rhs) const
{
    return ! this->operator ==(rhs);
}

/**
  * snap mode to a flag integer
  */
uint RS_SnapMode::toInt(const RS_SnapMode& s)
{
    uint ret {0};

    if (s.snapIntersection) ret |= RS_SnapMode::SnapIntersection;
    if (s.snapOnEntity)     ret |= RS_SnapMode::SnapOnEntity;
    if (s.snapCenter)       ret |= RS_SnapMode::SnapCenter;
    if (s.snapDistance)     ret |= RS_SnapMode::SnapDistance;
    if (s.snapMiddle)       ret |= RS_SnapMode::SnapMiddle;
    if (s.snapEndpoint)     ret |= RS_SnapMode::SnapEndpoint;
    if (s.snapGrid)         ret |= RS_SnapMode::SnapGrid;
    if (s.snapFree)         ret |= RS_SnapMode::SnapFree;
    if (s.snapAngle)        ret |= RS_SnapMode::SnapAngle;
    if (s.snapQuadrant) ret |= RS_SnapMode::SnapQuadrant;
    if (s.snapNode) ret |= RS_SnapMode::SnapNode;
    if (s.snapInsertion) ret |= RS_SnapMode::SnapInsertion;
    if (s.snapPerpendicular) ret |= RS_SnapMode::SnapPerpendicular;
    if (s.snapTangent) ret |= RS_SnapMode::SnapTangent;
    if (s.snapGeometricCenter) ret |= RS_SnapMode::SnapGeometricCenter;
    if (s.snapApparentIntersection) ret |= RS_SnapMode::SnapApparentIntersection;
    if (s.snapExtension) ret |= RS_SnapMode::SnapExtension;
    if (s.snapParallel) ret |= RS_SnapMode::SnapParallel;
    if (s.snapTracking) ret |= RS_SnapMode::SnapTracking;

    switch (s.restriction) {
    case RS2::RestrictHorizontal:
        ret |= RS_SnapMode::RestrictHorizontal;
        break;
    case RS2::RestrictVertical:
        ret |= RS_SnapMode::RestrictVertical;
        break;
    case RS2::RestrictOrthogonal:
        ret |= RS_SnapMode::RestrictOrthogonal;
        break;
    default:
        break;
    }

    return ret;
}

/**
  * integer flag to snapMode
  */
RS_SnapMode RS_SnapMode::fromInt(unsigned int ret)
{
    RS_SnapMode s;

    if (RS_SnapMode::SnapIntersection   & ret) s.snapIntersection = true;
    if (RS_SnapMode::SnapOnEntity       & ret) s.snapOnEntity = true;
    if (RS_SnapMode::SnapCenter         & ret) s.snapCenter = true;
    if (RS_SnapMode::SnapDistance       & ret) s.snapDistance = true;
    if (RS_SnapMode::SnapMiddle         & ret) s.snapMiddle = true;
    if (RS_SnapMode::SnapEndpoint       & ret) s.snapEndpoint = true;
    if (RS_SnapMode::SnapGrid           & ret) s.snapGrid = true;
    if (RS_SnapMode::SnapFree           & ret) s.snapFree = true;
    if (RS_SnapMode::SnapAngle          & ret) s.snapAngle = true;
    if (RS_SnapMode::SnapQuadrant & ret) s.snapQuadrant = true;
    if (RS_SnapMode::SnapNode & ret) s.snapNode = true;
    if (RS_SnapMode::SnapInsertion & ret) s.snapInsertion = true;
    if (RS_SnapMode::SnapPerpendicular & ret) s.snapPerpendicular = true;
    if (RS_SnapMode::SnapTangent & ret) s.snapTangent = true;
    if (RS_SnapMode::SnapGeometricCenter & ret) s.snapGeometricCenter = true;
    if (RS_SnapMode::SnapApparentIntersection & ret) s.snapApparentIntersection = true;
    if (RS_SnapMode::SnapExtension & ret) s.snapExtension = true;
    if (RS_SnapMode::SnapParallel & ret) s.snapParallel = true;
    if (RS_SnapMode::SnapTracking & ret) s.snapTracking = true;

    switch (RS_SnapMode::RestrictOrthogonal & ret) {
    case RS_SnapMode::RestrictHorizontal:
        s.restriction = RS2::RestrictHorizontal;
        break;
    case RS_SnapMode::RestrictVertical:
        s.restriction = RS2::RestrictVertical;
        break;
    case RS_SnapMode::RestrictOrthogonal:
        s.restriction = RS2::RestrictOrthogonal;
        break;
    default:
        s.restriction = RS2::RestrictNothing;
        break;
    }

    return s;
}

/**
  * Methods and structs for class RS_Snapper
  */
struct RS_Snapper::Indicator
{
    bool lines_state = false;
    QString lines_type;
    RS_Pen lines_pen;

    bool shape_state = false;
    QString shape_type;
    RS_Pen shape_pen;
};

struct RS_Snapper::ImpData {
RS_Vector snapCoord;
RS_Vector snapSpot;
enum Kind { None, Endpoint, Center, Middle, Distance, Intersection, Nearest, Grid,
            Quadrant, Node, Insertion, Perpendicular, Tangent, GeometricCenter,
            ApparentIntersection, Extension, Parallel, Tracking } kind {None};
RS_Vector trackingAcquired;
RS_Vector trackingGuideEnd;
bool trackingGuideActive {false};
};

/**
 * Constructor.
 */
RS_Snapper::RS_Snapper(RS_EntityContainer& container, RS_GraphicView& graphicView)
    :container(&container)
    ,graphicView(&graphicView)
    ,pImpData(new ImpData)
    ,snap_indicator(new Indicator)
{}

RS_Snapper::~RS_Snapper() = default;

/**
 * Initialize (called by all constructors)
 */
void RS_Snapper::init() 
{
    snapMode = graphicView->getDefaultSnapMode();
	keyEntity = nullptr;
	pImpData->snapSpot = RS_Vector{false};
	pImpData->snapCoord = RS_Vector{false};
	clearTrackingAcquisition();
	finished = false;
	m_SnapDistance = 1.0;

    RS_SETTINGS->beginGroup("/Appearance");
    snap_indicator->lines_state = RS_SETTINGS->readNumEntry("/indicator_lines_state", 1);
    snap_indicator->lines_type = RS_SETTINGS->readEntry("/indicator_lines_type", "Crosshair");
    snap_indicator->shape_state = RS_SETTINGS->readNumEntry("/indicator_shape_state", 1);
    snap_indicator->shape_type = RS_SETTINGS->readEntry("/indicator_shape_type", "Circle");
    RS_SETTINGS->endGroup();

    RS_SETTINGS->beginGroup("Colors");
    QString snap_color = RS_SETTINGS->readEntry("/snap_indicator", RS_Settings::snap_indicator);
    RS_SETTINGS->endGroup();

	snap_indicator->lines_pen = RS_Pen(RS_Color(snap_color), RS2::Width00, RS2::DashLine2);
	snap_indicator->shape_pen = RS_Pen(RS_Color(snap_color), RS2::Width00, RS2::SolidLine);
	snap_indicator->shape_pen.setScreenWidth(1);
    auto guard = RS_SETTINGS->beginGroupGuard("/Snapping");
    catchEntityGuiRange=RS_SETTINGS->readNumEntry("/CatchEntityGuiDistance", 32);
}


void RS_Snapper::finish() {
    finished = true;
    clearTrackingAcquisition();
    deleteSnapper();
}


void RS_Snapper::setSnapMode(const RS_SnapMode& snapMode) {
    const bool clearTracking = !snapMode.snapTracking
        && (this->snapMode.snapTracking
            || pImpData->trackingAcquired.valid
            || pImpData->trackingGuideActive);
    this->snapMode = snapMode;
	if (clearTracking) {
		clearTrackingAcquisition();
		pImpData->snapSpot = RS_Vector{false};
		pImpData->snapCoord = RS_Vector{false};
		pImpData->kind = ImpData::None;
		deleteSnapper();
	}
	RS_DIALOGFACTORY->requestSnapDistOptions(m_SnapDistance, snapMode.snapDistance);
    RS_DIALOGFACTORY->requestSnapMiddleOptions(middlePoints, snapMode.snapMiddle);
//std::cout<<"RS_Snapper::setSnapMode(): middlePoints="<<middlePoints<<std::endl;
}


RS_SnapMode const* RS_Snapper::getSnapMode() const{
	return &(this->snapMode);
}

RS_SnapMode* RS_Snapper::getSnapMode() {
	return &(this->snapMode);
}

//get current mouse coordinates
RS_Vector RS_Snapper::snapFree(QMouseEvent* e) {
	if (!e) {
                RS_DEBUG->print(RS_Debug::D_WARNING,
						"RS_Snapper::snapFree: event is nullptr");
        return RS_Vector(false);
    }
	pImpData->snapSpot=graphicView->toGraph(e->x(), e->y());
	pImpData->snapCoord=pImpData->snapSpot;
    pImpData->trackingGuideActive = false;
    pImpData->trackingGuideEnd = RS_Vector(false);
    snap_indicator->lines_state=true;
	return pImpData->snapCoord;
}

/**
 * Snap to a coordinate in the drawing using the current snap mode.
 *
 * @param e A mouse event.
 * @return The coordinates of the point or an invalid vector.
 */
RS_Vector RS_Snapper::snapPoint(QMouseEvent* e)
{
	pImpData->snapSpot = RS_Vector(false);
    pImpData->kind = ImpData::None;
    pImpData->trackingGuideActive = false;
    pImpData->trackingGuideEnd = RS_Vector(false);
    RS_Vector t(false);

	if (!e) {
                RS_DEBUG->print(RS_Debug::D_WARNING,
						"RS_Snapper::snapPoint: event is nullptr");
		return pImpData->snapSpot;
    }

    RS_Vector mouseCoord = graphicView->toGraph(e->x(), e->y());
    double ds2Min=RS_MAXDOUBLE*RS_MAXDOUBLE;
    const auto consider = [&](const RS_Vector& candidate, ImpData::Kind kind) {
        const double ds2 = mouseCoord.squaredTo(candidate);
        if (candidate.valid && ds2 < ds2Min) {
            ds2Min = ds2;
            pImpData->snapSpot = candidate;
            pImpData->kind = kind;
        }
    };

    if (snapMode.snapEndpoint) {
        t = snapEndpoint(mouseCoord);
		double ds2=mouseCoord.squaredTo(t);

        if (ds2 < ds2Min){
            ds2Min=ds2;
			pImpData->snapSpot = t;
            pImpData->kind = ImpData::Endpoint;
        }
    }
    if (snapMode.snapCenter) {
        t = snapCenter(mouseCoord);
		double ds2=mouseCoord.squaredTo(t);
        if (ds2 < ds2Min){
            ds2Min=ds2;
			pImpData->snapSpot = t;
            pImpData->kind = ImpData::Center;
        }
    }
    if (snapMode.snapMiddle) {
        //this is still brutal force
        //todo: accept value from widget QG_SnapMiddleOptions
		RS_DIALOGFACTORY->requestSnapMiddleOptions(middlePoints, snapMode.snapMiddle);
        t = snapMiddle(mouseCoord);
		double ds2=mouseCoord.squaredTo(t);
        if (ds2 < ds2Min){
            ds2Min=ds2;
			pImpData->snapSpot = t;
            pImpData->kind = ImpData::Middle;
        }
    }
    if (snapMode.snapDistance) {
        //this is still brutal force
        //todo: accept value from widget QG_SnapDistOptions
		RS_DIALOGFACTORY->requestSnapDistOptions(m_SnapDistance, snapMode.snapDistance);
        t = snapDist(mouseCoord);
		double ds2=mouseCoord.squaredTo(t);
        if (ds2 < ds2Min){
            ds2Min=ds2;
			pImpData->snapSpot = t;
            pImpData->kind = ImpData::Distance;
        }
    }
    if (snapMode.snapIntersection) {
        t = snapIntersection(mouseCoord);
		double ds2=mouseCoord.squaredTo(t);
        if (ds2 < ds2Min){
            ds2Min=ds2;
			pImpData->snapSpot = t;
            pImpData->kind = ImpData::Intersection;
        }
    }
    if (snapMode.snapQuadrant) consider(snapQuadrant(mouseCoord), ImpData::Quadrant);
    if (snapMode.snapNode) consider(snapNode(mouseCoord), ImpData::Node);
    if (snapMode.snapInsertion) consider(snapInsertion(mouseCoord), ImpData::Insertion);
    if (snapMode.snapPerpendicular) consider(snapPerpendicular(mouseCoord), ImpData::Perpendicular);
    if (snapMode.snapTangent) consider(snapTangent(mouseCoord), ImpData::Tangent);
    if (snapMode.snapGeometricCenter) consider(snapGeometricCenter(mouseCoord), ImpData::GeometricCenter);
    if (snapMode.snapApparentIntersection) consider(snapApparentIntersection(mouseCoord), ImpData::ApparentIntersection);
    if (snapMode.snapExtension) consider(snapExtension(mouseCoord), ImpData::Extension);
    if (snapMode.snapParallel) consider(snapParallel(mouseCoord), ImpData::Parallel);

    if (snapMode.snapOnEntity &&
		pImpData->snapSpot.distanceTo(mouseCoord) > snapMode.distance) {
        t = snapOnEntity(mouseCoord);
		double ds2=mouseCoord.squaredTo(t);
        if (ds2 < ds2Min){
            ds2Min=ds2;
			pImpData->snapSpot = t;
            pImpData->kind = ImpData::Nearest;
        }
    }

    if (snapMode.snapGrid) {
        t = snapGrid(mouseCoord);
		double ds2=mouseCoord.squaredTo(t);
        if (ds2 < ds2Min){
//            ds2Min=ds2;
			pImpData->snapSpot = t;
            pImpData->kind = ImpData::Grid;
        }
    }

	if( !pImpData->snapSpot.valid ) {
		pImpData->snapSpot=mouseCoord; //default to snapFree
        pImpData->kind = ImpData::None;
    } else {

        //retreat to snapFree when distance is more than quarter grid
        // issue #1631: snapFree issues: defines getSnapFree as the minimum graph distance to allow SnapFree
        if(snapMode.snapFree){
            // compare the current graph distance to the closest snap point to the minimum snapping free distance
            if((mouseCoord - pImpData->snapSpot).magnitude() >= getSnapRange())
            {
                pImpData->snapSpot = mouseCoord;
                pImpData->kind = ImpData::None;
            }
        }
    }

    const bool objectCandidate = pImpData->kind != ImpData::None
        && pImpData->kind != ImpData::Grid
        && pImpData->kind != ImpData::Tracking
        && pImpData->snapSpot.valid
        && mouseCoord.distanceTo(pImpData->snapSpot) <= getSnapRange();
    if (snapMode.snapTracking && objectCandidate) {
        // Acquisition is deliberately overlay-only state. Merely hovering a
        // working object-snap candidate must never modify the document.
        pImpData->trackingAcquired = pImpData->snapSpot;
    } else if (snapMode.snapTracking
               && pImpData->kind == ImpData::None
               && snapMode.restriction == RS2::RestrictNothing
               && !snapMode.snapGrid
               && pImpData->trackingAcquired.valid) {
        const RS_Vector tracked = projectToTrackingGuide(mouseCoord);
        if (tracked.valid) {
            pImpData->snapSpot = tracked;
            pImpData->kind = ImpData::Tracking;
            pImpData->trackingGuideEnd = tracked;
            pImpData->trackingGuideActive = true;
            keyEntity = nullptr;
        }
    }
    //if (snapSpot.distanceTo(mouseCoord) > snapMode.distance) {
    // handle snap restrictions that can be activated in addition
    //   to the ones above:
    //apply restriction
    RS_Vector rz = graphicView->getRelativeZero();
	RS_Vector vpv(rz.x, pImpData->snapSpot.y);
	RS_Vector vph(pImpData->snapSpot.x,rz.y);
    switch (snapMode.restriction) {
    case RS2::RestrictOrthogonal:
		pImpData->snapCoord= ( mouseCoord.distanceTo(vpv)< mouseCoord.distanceTo(vph))?
                    vpv:vph;
        break;
    case RS2::RestrictHorizontal:
		pImpData->snapCoord = vph;
        break;
    case RS2::RestrictVertical:
		pImpData->snapCoord = vpv;
        break;

    //case RS2::RestrictNothing:
    default:
        if (snapMode.snapAngle && pImpData->kind == ImpData::None) {
            auto settingsGuard = RS_SETTINGS->beginGroupGuard("/Snap");
            double increment = RS_SETTINGS->readEntry(
                "/AngleIncrement", "15").toDouble();
            if (!(increment > RS_TOLERANCE && increment <= 180.0)) {
                increment = 15.0;
            }
            pImpData->snapCoord = snapToAngle(
                pImpData->snapSpot, rz, increment);
        } else {
            pImpData->snapCoord = pImpData->snapSpot;
        }
        break;
    }
    //}
    //else snapCoord = snapSpot;

	snapPoint(pImpData->snapSpot, false);

	return pImpData->snapCoord;
}


/**manually set snapPoint*/
RS_Vector RS_Snapper::snapPoint(const RS_Vector& coord, bool setSpot)
{
    if(coord.valid){
		pImpData->snapSpot=coord;
		if(setSpot) pImpData->snapCoord = coord;
		drawSnapper();
		RS_DIALOGFACTORY->updateCoordinateWidget(
					pImpData->snapCoord,
					pImpData->snapCoord - graphicView->getRelativeZero());
    }
    return coord;
}


double RS_Snapper::getSnapRange() const
{
    // issue #1631: redefine this method to the minimum graph distance to allow "Snap Free"
    // When the closest of any other snapping point is beyond this distance, free snapping is used.
    constexpr double Min_Snap_Factor = 0.25;
    std::vector<double> distances(3, RS_MAXDOUBLE);
    double& minGui=distances[0];
    double& minGrid=distances[1];
    double& minSize=distances[2];
    if (graphicView != nullptr) {
        minGui = graphicView->toGraphDX(32);
        // if grid is on, less than one quarter of the cell vector
        if (graphicView->isGridOn())
            minGrid = graphicView->getGrid()->getCellVector().magnitude() * Min_Snap_Factor;
    }
    if (container != nullptr && isSizeValid(container->getSize())) {
        // The size bounding box
        minSize = getValidSize(container->getSize());
    }
    if (std::min(minGui, minGrid) < 0.99 * RS_MAXDOUBLE)
        return std::min(minGui, minGrid);
    if (minSize < 0.99 * RS_MAXDOUBLE)
        return minSize;
    // shouldn't happen: no graphicview or a valid size
    // Allow free snapping by returning the floating point tolerance
    return RS_TOLERANCE;
}

/**
 * Snaps to a free coordinate.
 *
 * @param coord The mouse coordinate.
 * @return The coordinates of the point or an invalid vector.
 */
RS_Vector RS_Snapper::snapFree(const RS_Vector& coord) {
	keyEntity = nullptr;
    return coord;
}



/**
 * Snaps to the closest endpoint.
 *
 * @param coord The mouse coordinate.
 * @return The coordinates of the point or an invalid vector.
 */
RS_Vector RS_Snapper::snapEndpoint(const RS_Vector& coord) {
    RS_Vector vec(false);

    vec = container->getNearestEndpoint(coord,
										nullptr/*, &keyEntity*/);
    return vec;
}



/**
 * Snaps to a grid point.
 *
 * @param coord The mouse coordinate.
 * @return The coordinates of the point or an invalid vector.
 */
RS_Vector RS_Snapper::snapGrid(const RS_Vector& coord) {

//    RS_DEBUG->print("RS_Snapper::snapGrid begin");

//    std::cout<<__FILE__<<" : "<<__func__<<" : line "<<__LINE__<<std::endl;
//    std::cout<<" mouse: = "<<coord<<std::endl;
//    std::cout<<" snapGrid: = "<<graphicView->getGrid()->snapGrid(coord)<<std::endl;
    return  graphicView->getGrid()->snapGrid(coord);
}



/**
 * Snaps to a point on an entity.
 *
 * @param coord The mouse coordinate.
 * @return The coordinates of the point or an invalid vector.
 */
RS_Vector RS_Snapper::snapOnEntity(const RS_Vector& coord) {

	RS_Vector vec{};
	vec = container->getNearestPointOnEntity(coord, true, nullptr, &keyEntity);
    return vec;
}



/**
 * Snaps to the closest center.
 *
 * @param coord The mouse coordinate.
 * @return The coordinates of the point or an invalid vector.
 */
RS_Vector RS_Snapper::snapCenter(const RS_Vector& coord) {
	RS_Vector vec{};

	vec = container->getNearestCenter(coord, nullptr);
    return vec;
}



/**
 * Snaps to the closest middle.
 *
 * @param coord The mouse coordinate.
 * @return The coordinates of the point or an invalid vector.
 */
RS_Vector RS_Snapper::snapMiddle(const RS_Vector& coord) {
//std::cout<<"RS_Snapper::snapMiddle(): middlePoints="<<middlePoints<<std::endl;
	return container->getNearestMiddle(coord,static_cast<double *>(nullptr),middlePoints);
}

RS_Vector RS_Snapper::snapQuadrant(const RS_Vector& coord) {
    RS_Vector best(false);
    double bestDistance = RS_MAXDOUBLE;
    for (RS_Entity* entity = container->firstEntity(RS2::ResolveAll);
         entity; entity = container->nextEntity(RS2::ResolveAll)) {
        const RS2::EntityType type = entity->rtti();
        if (!entity->isVisible() || (type != RS2::EntityCircle
            && type != RS2::EntityArc && type != RS2::EntityEllipse)) continue;
        double distance = RS_MAXDOUBLE;
        const RS_Vector point = entity->getNearestMiddle(coord, &distance, 0);
        if (point.valid && distance < bestDistance) {
            best = point;
            bestDistance = distance;
            keyEntity = entity;
        }
    }
    return best;
}

RS_Vector RS_Snapper::snapNode(const RS_Vector& coord) {
    RS_Vector best(false);
    double bestDistance = RS_MAXDOUBLE;
    for (RS_Entity* entity = container->firstEntity(RS2::ResolveAll);
         entity; entity = container->nextEntity(RS2::ResolveAll)) {
        if (!entity->isVisible() || entity->rtti() != RS2::EntityPoint) continue;
        const RS_Vector point = entity->getNearestEndpoint(coord);
        const double distance = coord.distanceTo(point);
        if (point.valid && distance < bestDistance) {
            best = point;
            bestDistance = distance;
            keyEntity = entity;
        }
    }
    return best;
}

RS_Vector RS_Snapper::snapInsertion(const RS_Vector& coord) {
    RS_Vector best(false);
    double bestDistance = RS_MAXDOUBLE;
    for (RS_Entity* entity = container->firstEntity(RS2::ResolveAll);
         entity; entity = container->nextEntity(RS2::ResolveAll)) {
        const RS2::EntityType type = entity->rtti();
        if (!entity->isVisible() || (type != RS2::EntityInsert
            && type != RS2::EntityText && type != RS2::EntityMText)) continue;
        double distance = RS_MAXDOUBLE;
        const RS_Vector point = entity->getNearestRef(coord, &distance);
        if (point.valid && distance < bestDistance) {
            best = point;
            bestDistance = distance;
            keyEntity = entity;
        }
    }
    return best;
}

RS_Vector RS_Snapper::snapPerpendicular(const RS_Vector& coord) {
    RS_Entity* entity = container->getNearestEntity(coord, nullptr, RS2::ResolveAllButTextImage);
    if (!entity) return RS_Vector(false);
    keyEntity = entity;
    return entity->getNearestPointOnEntity(graphicView->getRelativeZero(), false);
}

RS_Vector RS_Snapper::snapTangent(const RS_Vector& coord) {
    RS_Entity* entity = container->getNearestEntity(coord, nullptr, RS2::ResolveAllButTextImage);
    if (!entity) return RS_Vector(false);
    RS_VectorSolutions points = entity->getTangentPoint(graphicView->getRelativeZero());
    if (points.getNumber() == 0) return RS_Vector(false);
    keyEntity = entity;
    return points.getClosest(coord);
}

RS_Vector RS_Snapper::snapGeometricCenter(const RS_Vector& coord) {
    RS_Vector best(false);
    double bestDistance = RS_MAXDOUBLE;
    for (RS_Entity* entity = container->firstEntity(RS2::ResolveAll);
         entity; entity = container->nextEntity(RS2::ResolveAll)) {
        if (!entity->isVisible() || entity->rtti() != RS2::EntityPolyline) continue;
        const RS_Vector point = entity->getCenter();
        const double distance = coord.distanceTo(point);
        if (point.valid && distance < bestDistance) {
            best = point;
            bestDistance = distance;
            keyEntity = entity;
        }
    }
    return best;
}

RS_Vector RS_Snapper::snapApparentIntersection(const RS_Vector& coord) {
    std::vector<RS_Entity*> entities;
    for (RS_Entity* entity = container->firstEntity(RS2::ResolveAllButTextImage);
         entity; entity = container->nextEntity(RS2::ResolveAllButTextImage)) {
        if (entity->isVisible()) entities.push_back(entity);
    }
    RS_Vector best(false);
    double bestDistance = RS_MAXDOUBLE;
    for (size_t i = 0; i < entities.size(); ++i) {
        for (size_t j = i + 1; j < entities.size(); ++j) {
            const RS_VectorSolutions points = RS_Information::getIntersection(
                entities[i], entities[j], false);
            double distance = RS_MAXDOUBLE;
            const RS_Vector point = points.getClosest(coord, &distance);
            if (point.valid && distance < bestDistance) {
                best = point;
                bestDistance = distance;
                keyEntity = entities[i];
            }
        }
    }
    return best;
}

RS_Vector RS_Snapper::snapExtension(const RS_Vector& coord) {
    RS_Entity* entity = container->getNearestEntity(coord, nullptr, RS2::ResolveAllButTextImage);
    if (!entity) return RS_Vector(false);
    keyEntity = entity;
    return entity->getNearestPointOnEntity(coord, false);
}

RS_Vector RS_Snapper::snapParallel(const RS_Vector& coord) {
    RS_Entity* entity = container->getNearestEntity(coord, nullptr, RS2::ResolveAllButTextImage);
    auto* line = dynamic_cast<RS_Line*>(entity);
    if (!line) return RS_Vector(false);
    keyEntity = entity;
    return snapToRelativeAngle(line->getAngle1(), coord, graphicView->getRelativeZero(), 180.0);
}



/**
 * Snaps to the closest point with a given distance to the endpoint.
 *
 * @param coord The mouse coordinate.
 * @return The coordinates of the point or an invalid vector.
 */
RS_Vector RS_Snapper::snapDist(const RS_Vector& coord) {
    RS_Vector vec;

//std::cout<<" RS_Snapper::snapDist(RS_Vector coord): distance="<<distance<<std::endl;
	vec = container->getNearestDist(m_SnapDistance,
                                    coord,
									nullptr);
    return vec;
}



/**
 * Snaps to the closest intersection point.
 *
 * @param coord The mouse coordinate.
 * @return The coordinates of the point or an invalid vector.
 */
RS_Vector RS_Snapper::snapIntersection(const RS_Vector& coord) {
	RS_Vector vec{};

    vec = container->getNearestIntersection(coord,
											nullptr);
    return vec;
}



/**
 * 'Corrects' the given coordinates to 0, 90, 180, 270 degrees relative to
 * the current relative zero point.
 *
 * @param coord The uncorrected coordinates.
 * @return The corrected coordinates.
 */
RS_Vector RS_Snapper::restrictOrthogonal(const RS_Vector& coord) {
    RS_Vector rz = graphicView->getRelativeZero();
    RS_Vector ret(coord);

    RS_Vector retx = RS_Vector(rz.x, ret.y);
    RS_Vector rety = RS_Vector(ret.x, rz.y);

    if (retx.distanceTo(ret) > rety.distanceTo(ret)) {
        ret = rety;
    } else {
        ret = retx;
    }

    return ret;
}

/**
 * 'Corrects' the given coordinates to 0, 180 degrees relative to
 * the current relative zero point.
 *
 * @param coord The uncorrected coordinates.
 * @return The corrected coordinates.
 */
RS_Vector RS_Snapper::restrictHorizontal(const RS_Vector& coord) {
    RS_Vector rz = graphicView->getRelativeZero();
    RS_Vector ret = RS_Vector(coord.x, rz.y);
    return ret;
}


/**
 * 'Corrects' the given coordinates to 90, 270 degrees relative to
 * the current relative zero point.
 *
 * @param coord The uncorrected coordinates.
 * @return The corrected coordinates.
 */
RS_Vector RS_Snapper::restrictVertical(const RS_Vector& coord) {
    RS_Vector rz = graphicView->getRelativeZero();
    RS_Vector ret = RS_Vector(rz.x, coord.y);
    return ret;
}


/**
 * Catches an entity which is close to the given position 'pos'.
 *
 * @param pos A graphic coordinate.
 * @param level The level of resolving for iterating through the entity
 *        container
 * @return Pointer to the entity or nullptr.
 */
RS_Entity* RS_Snapper::catchEntity(const RS_Vector& pos,
                                   RS2::ResolveLevel level) {

    RS_DEBUG->print("RS_Snapper::catchEntity");

        // set default distance for points inside solids
    double dist (0.);
//    std::cout<<"getSnapRange()="<<getSnapRange()<<"\tsnap distance = "<<dist<<std::endl;

    RS_Entity* entity = container->getNearestEntity(pos, &dist, level);

    int idx = -1;
    if (entity != nullptr && entity->getParent()) {
        idx = entity->getParent()->findEntity(entity);
    }

    if (entity != nullptr && dist <= getCatchDistance(getSnapRange(), catchEntityGuiRange, graphicView)) {
        // highlight:
        RS_DEBUG->print("RS_Snapper::catchEntity: found: %d", idx);
        return entity;
    } else {
        RS_DEBUG->print("RS_Snapper::catchEntity: not found");
		return nullptr;
    }
    RS_DEBUG->print("RS_Snapper::catchEntity: OK");
}


/**
 * Catches an entity which is close to the given position 'pos'.
 *
 * @param pos A graphic coordinate.
 * @param level The level of resolving for iterating through the entity
 *        container
 * @enType, only search for a particular entity type
 * @return Pointer to the entity or nullptr.
 */
RS_Entity* RS_Snapper::catchEntity(const RS_Vector& pos, RS2::EntityType enType,
                                   RS2::ResolveLevel level) {

    RS_DEBUG->print("RS_Snapper::catchEntity");
//                    std::cout<<"RS_Snapper::catchEntity(): enType= "<<enType<<std::endl;

    // set default distance for points inside solids
	RS_EntityContainer ec(nullptr,false);
	//isContainer
	bool isContainer{false};
	switch(enType){
	case RS2::EntityPolyline:
	case RS2::EntityContainer:
	case RS2::EntitySpline:
		isContainer=true;
		break;
	default:
		break;
	}

	for(RS_Entity* en= container->firstEntity(level);en;en=container->nextEntity(level)){
        if(en->isVisible()==false) continue;
		if(en->rtti() != enType && isContainer){
            //whether this entity is a member of member of the type enType
            RS_Entity* parent(en->getParent());
			bool matchFound{false};
			while(parent ) {
//                    std::cout<<"RS_Snapper::catchEntity(): parent->rtti()="<<parent->rtti()<<" enType= "<<enType<<std::endl;
                if(parent->rtti() == enType) {
                    matchFound=true;
                    ec.addEntity(en);
                    break;
                }
                parent=parent->getParent();
            }
			if(!matchFound) continue;
        }
        if (en->rtti() == enType){
            ec.addEntity(en);
        }
    }
	if (ec.count() == 0 ) return nullptr;
    double dist(0.);

    RS_Entity* entity = ec.getNearestEntity(pos, &dist, RS2::ResolveNone);

    int idx = -1;
    if (entity != nullptr && entity->getParent()) {
        idx = entity->getParent()->findEntity(entity);
    }

    if (entity != nullptr && dist <= getCatchDistance(getSnapRange(), catchEntityGuiRange, graphicView)) {
        // highlight:
        RS_DEBUG->print("RS_Snapper::catchEntity: found: %d", idx);
        return entity;
    } else {
        RS_DEBUG->print("RS_Snapper::catchEntity: not found");
		return nullptr;
    }
}


/**
 * Catches an entity which is close to the mouse cursor.
 *
 * @param e A mouse event.
 * @param level The level of resolving for iterating through the entity
 *        container
 * @return Pointer to the entity or nullptr.
 */
RS_Entity* RS_Snapper::catchEntity(QMouseEvent* e,
                                   RS2::ResolveLevel level) {

    RS_Entity* entity = catchEntity(
               RS_Vector(graphicView->toGraphX(e->x()),
                         graphicView->toGraphY(e->y())),
               level);
    return entity;
}


/**
 * Catches an entity which is close to the mouse cursor.
 *
 * @param e A mouse event.
 * @param level The level of resolving for iterating through the entity
 *        container
 * @enType, only search for a particular entity type
 * @return Pointer to the entity or nullptr.
 */
RS_Entity* RS_Snapper::catchEntity(QMouseEvent* e, RS2::EntityType enType,
                                   RS2::ResolveLevel level) {
    return catchEntity(
			   {graphicView->toGraphX(e->x()), graphicView->toGraphY(e->y())},
				enType,
				level);
}

RS_Entity* RS_Snapper::catchEntity(QMouseEvent* e, const EntityTypeList& enTypeList,
                                   RS2::ResolveLevel level) {
	RS_Entity* pten = nullptr;
	RS_Vector coord{graphicView->toGraphX(e->x()), graphicView->toGraphY(e->y())};
    switch(enTypeList.size()) {
    case 0:
        return catchEntity(coord, level);
    default:
    {

		RS_EntityContainer ec(nullptr,false);
		for( auto t0: enTypeList){
			RS_Entity* en=catchEntity(coord, t0, level);
			if(en) ec.addEntity(en);
//			if(en) {
//            std::cout<<__FILE__<<" : "<<__func__<<" : lines "<<__LINE__<<std::endl;
//            std::cout<<"caught id= "<<en->getId()<<std::endl;
//            }
        }
        if(ec.count()>0){
            ec.getDistanceToPoint(coord, &pten, RS2::ResolveNone);
            return pten;
        }
    }

    }
	return nullptr;
}

void RS_Snapper::suspend() {
			// RVT Don't delete the snapper here!
	// RVT_PORT (can be deleted)();
	pImpData->snapSpot = pImpData->snapCoord = RS_Vector{false};
	clearTrackingAcquisition();
}

/**
 * Hides the snapper options. Default implementation does nothing.
 */
void RS_Snapper::hideOptions() {
    //not used any more, will be removed
}

/**
 * Shows the snapper options. Default implementation does nothing.
 */
void RS_Snapper::showOptions() {
    //not used any more, will be removed
}


/**
 * Deletes the snapper from the screen.
 */
void RS_Snapper::deleteSnapper()
{
    graphicView->getOverlayContainer(RS2::Snapper)->clear();
    graphicView->redraw(RS2::RedrawOverlay); // redraw will happen in the mouse movement event
}



/**
 * creates the snap indicator
 */
void RS_Snapper::drawSnapper()
{
    // We could properly speed this up by calling the draw function of this snapper within the paint event
    // this will avoid creating/deletion of the lines

    graphicView->getOverlayContainer(RS2::Snapper)->clear();
	if (!finished && pImpData->snapSpot.valid)
    {
        RS_EntityContainer *container=graphicView->getOverlayContainer(RS2::Snapper);

        if (pImpData->trackingGuideActive
            && pImpData->trackingAcquired.valid
            && pImpData->trackingGuideEnd.valid) {
            auto* guide = new RS_OverlayLine(
                nullptr,
                {graphicView->toGui(pImpData->trackingAcquired),
                 graphicView->toGui(pImpData->trackingGuideEnd)});
            guide->setPen(snap_indicator->lines_pen);
            container->addEntity(guide);
        }

        if (snap_indicator->lines_state)
        {
            QString type = snap_indicator->lines_type;

            if (type == "Crosshair")
            {
                RS_OverlayLine *line = new RS_OverlayLine(nullptr,
                    {{0., graphicView->toGuiY(pImpData->snapCoord.y)},
                    {double(graphicView->getWidth()),
                    graphicView->toGuiY(pImpData->snapCoord.y)}});

                line->setPen(snap_indicator->lines_pen);
                container->addEntity(line);

                line = new RS_OverlayLine(nullptr,
                    {{graphicView->toGuiX(pImpData->snapCoord.x),0.},
                    {graphicView->toGuiX(pImpData->snapCoord.x),
                    double(graphicView->getHeight())}});

                line->setPen(snap_indicator->lines_pen);
                container->addEntity(line);
            }
            else if (type == "Crosshair2")
            {
                double xenoRadius=16;

                double snapX=graphicView->toGuiX(pImpData->snapCoord.x);
                double snapY=graphicView->toGuiY(pImpData->snapCoord.y);

                double viewWidth=double(graphicView->getWidth());
                double viewHeight=double(graphicView->getHeight());

                RS_OverlayLine *line;

                // ----O     (Left)
                line=new RS_OverlayLine(nullptr, {
                    {0., snapY},
                    {snapX-xenoRadius, snapY}
                });
                {
                    line->setPen(snap_indicator->lines_pen);
                    container->addEntity(line);
                }

                //     O---- (Right)
                line=new RS_OverlayLine(nullptr, {
                    {snapX+xenoRadius, snapY},
                    {viewWidth, snapY}
                });
                {
                    line->setPen(snap_indicator->lines_pen);
                    container->addEntity(line);
                }

                // (Top)
                line=new RS_OverlayLine(nullptr, {
                    {snapX, 0.},
                    {snapX, snapY-xenoRadius}
                });
                {
                    line->setPen(snap_indicator->lines_pen);
                    container->addEntity(line);
                }

                // (Bottom)
                line=new RS_OverlayLine(nullptr, {
                    {snapX, snapY+xenoRadius},
                    {snapX, viewHeight}
                });
                {
                    line->setPen(snap_indicator->lines_pen);
                    container->addEntity(line);
                }
            }
            else if (type == "Isometric")
            {
                //isometric crosshair
                RS2::CrosshairType chType=graphicView->getCrosshairType();
                RS_Vector direction1;
                RS_Vector direction2(0.,1.);
                double l=graphicView->getWidth()+graphicView->getHeight();
                switch(chType){
                case RS2::RightCrosshair:
                    direction1=RS_Vector(M_PI*5./6.)*l;
                    direction2*=l;
                    break;
                case RS2::LeftCrosshair:
                    direction1=RS_Vector(M_PI*1./6.)*l;
                    direction2*=l;
                    break;
                default:
                    direction1=RS_Vector(M_PI*1./6.)*l;
                    direction2=RS_Vector(M_PI*5./6.)*l;
                }
                RS_Vector center(graphicView->toGui(pImpData->snapCoord));
                RS_OverlayLine *line=new RS_OverlayLine(container,
                {center-direction1,center+direction1});
                line->setPen(snap_indicator->lines_pen);
                container->addEntity(line);
                line=new RS_OverlayLine(nullptr,
                {center-direction2,center+direction2});
                line->setPen(snap_indicator->lines_pen);
                container->addEntity(line);
            }
            else if (type == "Spiderweb")
            {
                RS_OverlayLine* line;
                RS_Vector point1;
                RS_Vector point2;

                point1 = RS_Vector{0, 0};
                point2 = RS_Vector{graphicView->toGuiX(pImpData->snapCoord.x),
                                   graphicView->toGuiY(pImpData->snapCoord.y)};
                line=new RS_OverlayLine{nullptr, {point1, point2}};
                line->setPen(snap_indicator->lines_pen);
                container->addEntity(line);

                point1 = RS_Vector(0, graphicView->getHeight());
                line = new RS_OverlayLine{nullptr, {point1, point2}};
                line->setPen(snap_indicator->lines_pen);
                container->addEntity(line);

                point1 = RS_Vector(graphicView->getWidth(), 0);
                line = new RS_OverlayLine(nullptr, {point1, point2});
                line->setPen(snap_indicator->lines_pen);
                container->addEntity(line);

                point1 = RS_Vector(graphicView->getWidth(), graphicView->getHeight());
                line = new RS_OverlayLine(nullptr, {point1, point2});
                line->setPen(snap_indicator->lines_pen);
                container->addEntity(line);
            }
        }
        if (snap_indicator->shape_state)
        {
            QString type = snap_indicator->shape_type;

            switch (pImpData->kind) {
            case ImpData::Endpoint: type = "Square"; break;
            case ImpData::Middle: type = "Triangle"; break;
            case ImpData::Center: type = "Circle"; break;
            case ImpData::Intersection: type = "Intersection"; break;
            case ImpData::Nearest: type = "Diamond"; break;
            case ImpData::Distance: type = "Circle"; break;
            case ImpData::Grid: type = "Point"; break;
            case ImpData::Quadrant: type = "Diamond"; break;
            case ImpData::Node: type = "Circle"; break;
            case ImpData::Insertion: type = "Square"; break;
            case ImpData::Perpendicular: type = "Square"; break;
            case ImpData::Tangent: type = "Circle"; break;
            case ImpData::GeometricCenter: type = "Circle"; break;
            case ImpData::ApparentIntersection: type = "Intersection"; break;
            case ImpData::Extension: type = "Square"; break;
            case ImpData::Parallel: type = "Diamond"; break;
            case ImpData::Tracking: type = "Point"; break;
            default: break;
            }

            if (type == "Circle")
            {
                RS_Circle *circle=new RS_Circle(container,
                    {pImpData->snapCoord, 4./graphicView->getFactor().x});
                circle->setPen(snap_indicator->shape_pen);
                container->addEntity(circle);
            }
            else if (type == "Point")
            {
                RS_Point *point=new RS_Point(container, pImpData->snapCoord);
                point->setPen(snap_indicator->shape_pen);
                container->addEntity(point);
            }
            else if (type == "Square")
            {
                RS_Vector snap_point{graphicView->toGuiX(pImpData->snapCoord.x),
                                     graphicView->toGuiY(pImpData->snapCoord.y)};

                double a = 6.0;
                RS_Vector p1 = snap_point + RS_Vector(-a, a);
                RS_Vector p2 = snap_point + RS_Vector(a, a);
                RS_Vector p3 = snap_point + RS_Vector(a, -a);
                RS_Vector p4 = snap_point + RS_Vector(-a, -a);

                RS_OverlayLine* line;
                line=new RS_OverlayLine{nullptr, {p1, p2}};
                line->setPen(snap_indicator->shape_pen);
                container->addEntity(line);

                line = new RS_OverlayLine{nullptr, {p2, p3}};
                line->setPen(snap_indicator->shape_pen);
                container->addEntity(line);

                line = new RS_OverlayLine(nullptr, {p3, p4});
                line->setPen(snap_indicator->shape_pen);
                container->addEntity(line);

                line = new RS_OverlayLine(nullptr, {p4, p1});
                line->setPen(snap_indicator->shape_pen);
                container->addEntity(line);
            }
            else if (type == "Triangle" || type == "Diamond"
                     || type == "Intersection")
            {
                const RS_Vector center{graphicView->toGuiX(pImpData->snapCoord.x),
                                       graphicView->toGuiY(pImpData->snapCoord.y)};
                const double a = 7.0;
                QList<QPair<RS_Vector, RS_Vector>> segments;
                if (type == "Triangle") {
                    const RS_Vector top = center + RS_Vector(0., -a);
                    const RS_Vector left = center + RS_Vector(-a, a);
                    const RS_Vector right = center + RS_Vector(a, a);
                    segments = {{top, left}, {left, right}, {right, top}};
                } else if (type == "Diamond") {
                    const RS_Vector top = center + RS_Vector(0., -a);
                    const RS_Vector right = center + RS_Vector(a, 0.);
                    const RS_Vector bottom = center + RS_Vector(0., a);
                    const RS_Vector left = center + RS_Vector(-a, 0.);
                    segments = {{top, right}, {right, bottom},
                                {bottom, left}, {left, top}};
                } else {
                    segments = {{center + RS_Vector(-a, -a),
                                 center + RS_Vector(a, a)},
                                {center + RS_Vector(-a, a),
                                 center + RS_Vector(a, -a)}};
                }
                for (const auto& segment : segments) {
                    auto* line = new RS_OverlayLine(nullptr,
                                                    {segment.first,
                                                     segment.second});
                    line->setPen(snap_indicator->shape_pen);
                    container->addEntity(line);
                }
            }
        }
        graphicView->redraw(RS2::RedrawOverlay); // redraw will happen in the mouse movement event
    }
}

void RS_Snapper::clearTrackingAcquisition()
{
    pImpData->trackingAcquired = RS_Vector(false);
    pImpData->trackingGuideEnd = RS_Vector(false);
    pImpData->trackingGuideActive = false;
}

RS_Vector RS_Snapper::projectToTrackingGuide(const RS_Vector& coord) const
{
    if (!coord.valid || !pImpData->trackingAcquired.valid) {
        return RS_Vector(false);
    }

    double increment = 90.0;
    if (snapMode.snapAngle) {
        auto settingsGuard = RS_SETTINGS->beginGroupGuard("/Snap");
        increment = RS_SETTINGS->readEntry(
            "/AngleIncrement", "15").toDouble();
        if (!(increment > RS_TOLERANCE && increment <= 180.0)) {
            increment = 15.0;
        }
    }

    const RS_Vector delta = coord - pImpData->trackingAcquired;
    if (delta.magnitude() <= RS_TOLERANCE) {
        return RS_Vector(false);
    }
    const double rawDegrees = std::atan2(delta.y, delta.x) * 180.0 / M_PI;
    const double guideRadians = std::round(rawDegrees / increment)
        * increment * M_PI / 180.0;
    const RS_Vector direction(std::cos(guideRadians),
                              std::sin(guideRadians));
    const double along = delta.x * direction.x + delta.y * direction.y;
    const RS_Vector projected = pImpData->trackingAcquired
        + direction * along;
    if (pImpData->trackingAcquired.distanceTo(projected) <= RS_TOLERANCE
        || coord.distanceTo(projected) > getSnapRange()) {
        return RS_Vector(false);
    }
    return projected;
}

bool RS_Snapper::hasTrackingAcquisition() const
{
    return pImpData->trackingAcquired.valid;
}

bool RS_Snapper::hasTrackingGuide() const
{
    return pImpData->trackingGuideActive;
}

RS_Vector RS_Snapper::trackingAcquisition() const
{
    return pImpData->trackingAcquired;
}

RS_Vector RS_Snapper::trackingGuideEnd() const
{
    return pImpData->trackingGuideEnd;
}

RS_Vector RS_Snapper::snapToRelativeAngle(double baseAngle, const RS_Vector &currentCoord, const RS_Vector &referenceCoord, const double angularResolution)
{

    if(snapMode.restriction != RS2::RestrictNothing || snapMode.snapGrid)
    {
        return currentCoord;
    }

    double angle = referenceCoord.angleTo(currentCoord)*180.0/M_PI;
    angle -= std::remainder(angle,angularResolution);
    angle *= M_PI/180.;
    angle = angle + baseAngle; // add base angle, so snap is relative
    RS_Vector res = RS_Vector::polar(referenceCoord.distanceTo(currentCoord),angle);
    res += referenceCoord;

    if (snapMode.snapOnEntity)
    {
        RS_Vector t(false);
        //RS_Vector mouseCoord = graphicView->toGraph(currentCoord.x(), currentCoord.y());
        t = container->getNearestVirtualIntersection(res,angle,nullptr);

        pImpData->snapSpot = t;
        snapPoint(pImpData->snapSpot, true);
        return t;
    }
    else
    {
        snapPoint(res, true);
        return res;
    }
}

RS_Vector RS_Snapper::snapToAngle(const RS_Vector &currentCoord, const RS_Vector &referenceCoord, const double angularResolution)
{

    if(snapMode.restriction != RS2::RestrictNothing || snapMode.snapGrid)
    {
        return currentCoord;
    }

    double angle = referenceCoord.angleTo(currentCoord)*180.0/M_PI;
    angle -= std::remainder(angle,angularResolution);
    angle *= M_PI/180.;
    RS_Vector res = RS_Vector::polar(referenceCoord.distanceTo(currentCoord),angle);
    res += referenceCoord;

    if (snapMode.snapOnEntity)
    {
        RS_Vector t(false);
        //RS_Vector mouseCoord = graphicView->toGraph(currentCoord.x(), currentCoord.y());
        t = container->getNearestVirtualIntersection(res,angle,nullptr);

        pImpData->snapSpot = t;
        snapPoint(pImpData->snapSpot, true);
        return t;
    }
    else
    {
        snapPoint(res, true);
        return res;
    }
}
