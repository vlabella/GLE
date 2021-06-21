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

#ifndef _GLECIRCLE_H
#define _GLECIRCLE_H

#include "drawingobject.h"

//! Class describing a circle drawing object
class GLECircle : public GLEDrawingObject
{
	Q_OBJECT

public:

	//! Enumeration of points used to describe a circle
	enum Points
	{
		CentrePoint,
		CircumferencePoint,
		Radius
	};

	//! Constructor used for initialising variables and connections
	GLECircle(double resolution, QSize imageSize, GLEDrawingArea *area);

	//! Draw the circle on the provided painter
	void draw(QPainter *p);
	//! Return the shortest distance between a given point and the circle
	double distanceToPoint(const QPointF& p, QPointF *nearestPoint);
	//! Set one of the enumerated points
	void setPoint(int pointChoice, const QPointF& p, bool update = true);
	//! Find points where a line intersects the circle
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

	//! Return the circle's bounding rectangle in Qt coordinates
	QRectF circleRect();

	bool findScaleOrigin(const QPointF& pt, QPointF* origin, int handle);
	int supportedTransformMode();
	void linearTransform(const GLELinearEquation& ex, const GLELinearEquation& ey);
	void moveBy(QPointF offset);
	void rotateBy(double radians);

private slots:
	//! Update the painter path when the resolution or start/end points change
	void updateCircle();
	void updateFromProperty(int property);

};

#endif
