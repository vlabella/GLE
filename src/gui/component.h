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

#ifndef _GLECOMPONENT_H
#define _GLECOMPONENT_H

#include <QtGui>
#include "qgle_definitions.h"
#include "qgle_statics.h"

// The intention is that this is the parent of GLEDrawingObject
// and of SnapLine etc
//
// A lot of the functionality of GLEDrawingObject will derive from
// here.  It might be simpler to change GLEDrawingObject into
// GLEComponent and reimplement GLEDrawingObject by extracting
// the bits that aren't needed by SnapLine.

class GLEComponent : public QObject
{
	Q_OBJECT

	friend class GLEDrawingObject;
	friend class SnapLine;
	friend class GLELine;
	friend class GLECircle;
	friend class GLEEllipse;
	friend class GLEArc;
	friend class GLEAmove;
	friend class GLEText;
	friend class GLEPolarArray;

public:
	GLEComponent(double resolution, QSize imageSize, QObject *parent);

	void setBasePoint(const QPointF& qtp);

	//! A function that returns a painter path that can be used to draw the object
	QPainterPath path();

	//! Set the pen for drawing
	void setPen(QPen newPen) { paintPen = newPen; };
	//! Return the pen used for drawing
	QPen pen() { return(paintPen); };
	//! Virtual function to update the component after, e.g., points have changed
	virtual void updateAll();
	//! Virtual function to draw the object on a provided painter
	virtual void draw(QPainter *p) = 0;
	//! Virtual function to return the shortest distance between a given point and the object
	virtual double distanceToPoint(const QPointF& p, QPointF *nearestPoint = 0) = 0;
	//! Virtual function to set a point
	virtual void setPoint(int pointChoice, const QPointF& p, bool update = true);
	//! Function to set a length
	inline void setLength(int pointChoice, double len, bool update = true) { setPoint(pointChoice, QPointF(len,0), update); }
	//! Virtual function to find points where a line intersects an object
	virtual QList<QPointF> intersections(QPointF qtp1, QPointF qtp2) = 0;
	virtual QList<QPointF> intersections(QPointF qtp1, double angle) = 0;
	virtual QList<QPointF> intersections(double qtm, double qtc, bool vertical) = 0;

	//! Get a point in GLE coordinates
	QPointF getGLEPoint(int pointChoice, bool stored = false);
	//! Get a point in Qt coordinates
	QPointF getQtPoint(int pointChoice, bool stored = false);
	//! Get a value in GLE coordinates
	double getGLELength(int pointChoice, bool stored = false);
	//! Get a value in Qt coordinates
	double getQtLength(int pointChoice, bool stored = false);
	//! Get an angle in GLE coordinates
	double getGLEAngle(int pointChoice, bool stored = false);
	//! Get an angle in Qt coordinates
	double getQtAngle(int pointChoice, bool stored = false);
	//! Check whether a point or value has been set
	bool isSet(int pointChoice);

	// Convert GLE to Qt coordinates
	QPointF getQtPoint(const QPointF& glePoint);

public slots:
	//! Update the resolution of drawing objects
	void setDPI(double newDPI);
	//! Update the image height used for coordinate transforms
	void setPixmapSize(QSize newPixmapSize, double newDPI);

signals:
	//! Notify the world that a point has changed
	void pointChanged();
	//! Notify subclasses that the dpi or image height had changed
	void imageChanged();

protected:
	//! Contains the painter path
	QPainterPath *paintPath;
	//! Contains the pen used to draw the path
	QPen paintPen;

	//! The list of points
	QHash <int, QPointF> pointHash;

	//! Image resolution
	double dpi;
	//! Image height
	QSize pixmap;

	//! Line Width
	double lineWidth;

	QHash <int, QPointF> storedPointHash;
	QPointF basePoint;
	QHash<char, double> equationParameters;
	QHash<char, double> storedEquationParameters;


};




#endif
