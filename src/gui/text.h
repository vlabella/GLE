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

#ifndef _GLETEXT_H
#define _GLETEXT_H

#include "drawingobject.h"

//! Class describing a text drawing object
class GLEText : public GLEDrawingObject
{
	Q_OBJECT

public:

	//! Enumeration of the points needed to draw a text object
	enum Points
	{
		ReferencePoint
	};

	//! Constructor used for initialising variables and connections
	GLEText(double resolution, QSize imageSize, GLEDrawingArea* area);

	//! Draw the text on the provided painter
	void draw(QPainter *p);
	//! Return the shortest distance between a given point and the text object
	double distanceToPoint(const QPointF& p, QPointF *nearestPoint);
	double nearestOSnap(const QPointF& p, QPointF *osnap);
	//! Set one of the enumerated points
	void setPoint(int pointChoice, const QPointF& p, bool update = true);
	void addRelativeOSnaps(QPointF p);

	//! Find points where a line intersects the text string (there are none)
	// If vertical = true, m contains 'x'
	QList<QPointF> intersections(double qtm, double qtc, bool vertical = false);
	QList<QPointF> intersections(QPointF qtp1, double angle);
	QList<QPointF> intersections(QPointF qtp1, QPointF qtp2);
	QList<QPointF> getTangents(QPointF p);
	bool hasTangents();
	QList<QPointF> getPerpendiculars(QPointF p);
	bool hasPerpendiculars();

	//! Is the point inside the object (for a text string, always false)
	bool isInside(QPointF p);

	//! Set the reference point (tl,bl,tc,bc,tr,br,...)
	bool setReferencePoint(const QPointF& pt);
	bool trySetReferencePoint(const QPointF& pt, double x, double y, GLEJustify just, double* minDist);

	void moveBy(QPointF offset);
	void rotateBy(double radians);
	void createOTracks();
	void updateFromPropertyNoDirty(int property);
	void renderPostscript();
public slots:
	void resChanged();

private slots:
	//! Update the painter path when the resolution or start/end points change
	void updateText();
	void updateFromProperty(int property);

private:
	QPixmap renderedText;
	QPointF drawPoint;
	GLERectangle rect;
	double lastdpi;
};

#endif
