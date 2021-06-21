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

#ifndef _GLEPOLARARRAY_H
#define _GLEPOLARARRAY_H

#include "drawingobject.h"

//! Class describing a line drawing object
class GLEPolarArray : public GLEDrawingObject
{
	Q_OBJECT

public:

	enum Points
	{
		SpanAngle,
		NumberOfObjects
	};
	//! Constructor used for initialising variables and connections
	GLEPolarArray(double resolution, QSize imageSize, GLEDrawingArea* area);

	//! Draw the line on the provided painter
	void draw(QPainter *p);
	//! Return the shortest distance between a given point and the line
	double distanceToPoint(const QPointF& p, QPointF *nearestPoint);

	//! Set one of the enumerated points
	void setPoint(int pointChoice, const QPointF& p, bool update = true);
	void addRelativeOSnaps(QPointF p);

	void setBaseObject(GLEDrawingObject *obj);

	void createOTracks();

	//! Find points where a line intersects the line
	// If vertical = true, m contains 'x'
	QList<QPointF> intersections(double qtm, double qtc, bool vertical = false);
	QList<QPointF> intersections(QPointF qtp1, double angle);
	QList<QPointF> intersections(QPointF qtp1, QPointF qtp2);
	QList<QPointF> getTangents(QPointF p);
	bool hasTangents();
	QList<QPointF> getPerpendiculars(QPointF p);
	bool hasPerpendiculars();

	//! Is the point inside the object (for a line, always false)
	bool isInside(QPointF p);

	void scaleBy(double xratio, double yratio);
	void moveBy(QPointF offset);
	void rotateBy(double radians);

	void updateObjects();

private:
	QList<GLEDrawingObject *> baseObject;
};

#endif
