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

#include "polararray.h"

// We need to make sure that we can pass all the parameters on to the
// children (e.g. base points of displacement etc)

// The constructor for the line object
GLEPolarArray::GLEPolarArray(double resolution, QSize imageSize, GLEDrawingArea* area) :
	GLEDrawingObject(resolution, imageSize, area)
{
	amove = false;
}

void GLEPolarArray::createOTracks()
{
//	if (isSet(StartPoint) && isSet(EndPoint))
//	{
//		// TODO: create some snap lines!
//		// Also need to implement the necessary functionality
//		// in gledrawing.cpp
//	}
}

void GLEPolarArray::addRelativeOSnaps(QPointF)
{
//	if (isSet(StartPoint) && isSet(EndPoint))
//	{
//		qDebug() << "Adding osnap relative to " << QGLE::absQtToGLE(p,dpi,pixmap.height());
//		QGLE::flushIO();
//
//		relativeOSnaps.clear();
//		QPointF np;
//		distanceToPoint(p,&np);
//		qDebug() << "Nearest point: " << QGLE::absQtToGLE(np,dpi,pixmap.height());
//		if ((np == getQtPoint(StartPoint)) || (np == getQtPoint(EndPoint)))
//			return;
//		relativeOSnaps.append(QPair<QPointF,int>(np,QGLE::PerpendicularSnap));
//	}
//	qDebug() << "Added relative OSNAP";
//	QGLE::flushIO();
}

QList<QPointF> GLEPolarArray::getTangents(QPointF)
{
	// This depends on what the base object is
	QList<QPointF> empty;
	return(empty);
}

bool GLEPolarArray::hasTangents()
{
	// This depends on what the base object is
	return(false);
}

QList<QPointF> GLEPolarArray::getPerpendiculars(QPointF)
{
	// This depends on what the base object is
	QList<QPointF> empty;
	return(empty);
//	QList<QPointF> perpendiculars;
//	QPointF np;
//	distanceToPoint(p,&np);
//	if ((np == getQtPoint(StartPoint)) || (np == getQtPoint(EndPoint)))
//		return(perpendiculars);
//
//	perpendiculars.append(np);
//	return(perpendiculars);
}

bool GLEPolarArray::hasPerpendiculars()
{
	// This depends on what the base object is
	return(false);
}


void GLEPolarArray::draw(QPainter *)
{
	// Call the draw function of the base object and
	// repeat for rotated versions
}

void GLEPolarArray::updateObjects()
{
}


double GLEPolarArray::distanceToPoint(const QPointF&, QPointF*)
{
	// Call base object functions
	return(1e6);
}

void GLEPolarArray::setBaseObject(GLEDrawingObject *obj)
{
	baseObject.clear();
	baseObject.append(obj);
	// Now connect all the signals and slots
	// and/or call 'update object' just in case
}

// Set a point (start or end in the case of a line)
void GLEPolarArray::setPoint(int pointChoice, const QPointF&, bool)
{
	if (baseObject.size() < 1)
		return;

	switch(pointChoice)
	{
		case SpanAngle:
		case NumberOfObjects:
//			pointHash[pointChoice] = QGLE::absQtToGLE(p, dpi, pixmap.height());
//			Once both have been set, update the objects: regenerate
//			all but the first one based on the parameters
			break;
	}
	updateObjects();
}

QList<QPointF> GLEPolarArray::intersections(double, double, bool)
{
	// Call base object functionality
	QList<QPointF> pointArray;
	return(pointArray);
}





QList<QPointF> GLEPolarArray::intersections(QPointF, QPointF)
{
	// Call base object functionality
	QList<QPointF> pointArray;
	return(pointArray);
}

QList<QPointF> GLEPolarArray::intersections(QPointF, double)
{
	// Call base object functionality
	QList<QPointF> pointArray;
	return(pointArray);
}

bool GLEPolarArray::isInside(QPointF p)
{
	// Call base objection functionality
	UNUSED_ARG(p);
	return(false);
}

void GLEPolarArray::scaleBy(double, double)
{
	// Call base object functionality
}

void GLEPolarArray::moveBy(QPointF)
{
	// Call base object functionality
}

void GLEPolarArray::rotateBy(double)
{
	// Call base object functionality
}
