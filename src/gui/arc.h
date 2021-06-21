/***********************************************************************************
 * QGLE - A Graphical Interface to GLE                                             *
 * Copyright (C) 2006  A. S. Budden & J. Struyf                                    *
 *                                                                                 *
 * This program is free software; you can redistribute it and/or                   *
 * modify it under the terms of the GNU General Public License                     *
 * as published by the Free Software Foundation; either version 2                  *
 * of the License, or (at your option) any later version.                          *
 *                                                                                 *
 * This program is distributed in the hope that it will be useful,                 *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of                  *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                   *
 * GNU General Public License for more details.                                    *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License               *
 * along with this program; if not, write to the Free Software                     *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA. *
 *                                                                                 *
 * Also add information on how to contact you by electronic and paper mail.        *
 ***********************************************************************************/

#ifndef _GLEARC_H
#define _GLEARC_H

#include "drawingobject.h"

//! A class describing an arc drawing object, daughter of the generic GLEDrawingObject class
class GLEArc : public GLEDrawingObject
{
	Q_OBJECT

public:

	//! An enumeration describing the points that are used to describe an arc
	enum Points
	{
		CentrePoint,
		StartPoint,
		EndPoint,
		Radius,
		MidPoint
	};

	//! Constructor used for initialising variables and connections
	GLEArc(double resolution, QSize imageSize, GLEDrawingArea *area);

	//! Draw the arc on the provided painter
	void draw(QPainter *p);
	//! Return the shortest distance between a given point and the arc
	double distanceToPoint(const QPointF& p, QPointF *nearestPoint);
	//! Set one of the enumerated points
	void setPoint(int pointChoice, const QPointF& p, bool update = true);
	//! Find points where a line intersects the arc
	// If vertical = true, m contains 'x'
	QList<QPointF> intersections(double qtm, double qtc, bool vertical = false);
	QList<QPointF> intersections(QPointF qtp1, QPointF qtp2);
	QList<QPointF> intersections(QPointF qtp1, double angle);
	//! Is the point inside the object?
	bool isInside(QPointF p);
	void addRelativeOSnaps(QPointF p);
	QList<QPointF> getTangents(QPointF p);
	bool hasTangents();
	QList<QPointF> getPerpendiculars(QPointF p);
	bool hasPerpendiculars();
	void createOTracks();

	//! Return the arc's bounding rectangle in Qt coordinates
	QRectF arcRect();
	//! Return the starting angle
	double startAngleDeg();
	//! Return the ending angle
	double endAngleDeg();
	//! As above, but guaranteed to be > start_angle
	double endAngleDegConstrained();
	//! Return the starting angle
	double startAngleRad();
	//! Return the ending angle
	double endAngleRad();
	//! As above, but guaranteed to be > start_angle
	double endAngleRadConstrained();

	bool findScaleOrigin(const QPointF& pt, QPointF* origin, int handle);
	double nearestPriorityOSnap(const QPointF& pt, QPointF *osnap);
	int supportedTransformMode();
	void linearTransform(const GLELinearEquation& ex, const GLELinearEquation& ey);
	void moveBy(QPointF offset);
	void rotateBy(double radians);

private slots:
	//! Update the painter path when the resolution or start/end points change
	void updateArc();
	void updateFromProperty(int property);

private:
	//! Member variable describing whether the arc runs clockwise or anticlockwise
	bool directionReversed;
	//! If you take the line between the pseudo-circle centre and a given point, does it cross the arc
	bool isOnArc(double angle);
	//! Is the point inside the full circle area (i.e. no tangent)
	bool isInsideCircle(QPointF p);
	//! What's the closest point to the full circle?
	double distanceToPointOnCircle(QPointF p, QPointF *nearestPoint);
	void drawArc(QPainter *p, double t1, double t2);
	void addBezier(QPainterPath *p, GLEBezier* bezier);
	void computeAndDraw(QPainter *p, GLEArcDO* obj, GLECurvedArrowHead* head);
	void getGLEArcT0T1(double* t0, double* t1);
};

#endif
