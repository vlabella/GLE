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

#include "component.h"

// Constructor: store the resolution and image height; set up defaults
GLEComponent::GLEComponent(double resolution, QSize imageSize, QObject *parent) : QObject(parent)
{
	// Defaults:
	dpi = resolution;
	lineWidth = 0.02;
	pixmap = imageSize;
	// We don't actually use this at present, but it's kept
	// for possible future use
	paintPath = new QPainterPath();
	// Default pen
	paintPen = QPen(Qt::black,
			QGLE::relGLEToQt(lineWidth, 0, dpi).x(),
			Qt::SolidLine,
			Qt::SquareCap,
			Qt::BevelJoin);
}

//! Virtual function to update the component after, e.g., points have changed
void GLEComponent::updateAll()
{
}

//! Virtual function to set a point
void GLEComponent::setPoint(int pointChoice, const QPointF& p, bool)
{
	pointHash[pointChoice] = QGLE::absQtToGLE(p, dpi, pixmap.height());
}

// Determine whether a parameter has been set
bool GLEComponent::isSet(int pointChoice)
{
	return(pointHash.contains(pointChoice));
}

// Get the value of a parameter (as a point in GLE coordinates)
QPointF GLEComponent::getGLEPoint(int pointChoice, bool stored)
{
	if (stored)
		return(storedPointHash.value(pointChoice, QPointF(0,0)));
	else
		return(pointHash.value(pointChoice, QPointF(0,0)));
}

void GLEComponent::setBasePoint(const QPointF& qtp)
{
	basePoint = qtp;
	// Save the point hash
	storedPointHash = pointHash;
	// Only used in some objects (e.g. ellipse)
	storedEquationParameters = equationParameters;
}

// Convert GLE to Qt coordinates
QPointF GLEComponent::getQtPoint(const QPointF& glePoint)
{
	return(QGLE::absGLEToQt(glePoint, dpi, pixmap.height()));
}

// Get the value of a parameter (as a point in QT coordinates)
QPointF GLEComponent::getQtPoint(int pointChoice, bool stored)
{
	QPointF gle = getGLEPoint(pointChoice, stored);
	return(QGLE::absGLEToQt(gle, dpi, pixmap.height()));
}

// Get the value of a parameter (as a double in GLE coordinates)
double GLEComponent::getGLELength(int pointChoice, bool stored)
{
	if (stored)
		return(storedPointHash.value(pointChoice, QPointF(0,0)).x());
	else
		return(pointHash.value(pointChoice, QPointF(0,0)).x());
}

// Get the value of a parameter (as a double in QT coordinates)
double GLEComponent::getQtLength(int pointChoice, bool stored)
{
	double gleX = getGLELength(pointChoice, stored);
	return(QGLE::relGLEToQt(gleX, dpi));
}

// Get the value of a parameter (as a double in GLE coordinates)
double GLEComponent::getGLEAngle(int pointChoice, bool stored)
{
	if (stored)
		return(storedPointHash.value(pointChoice, QPointF(0,0)).x());
	else
		return(pointHash.value(pointChoice, QPointF(0,0)).x());
}

// Get the value of a parameter (as a double in QT coordinates)
double GLEComponent::getQtAngle(int pointChoice, bool stored)
{
	// Qt Angles go the other way round to GLE ones as
	// the Y axis is inverted
	double gleX = -getGLELength(pointChoice, stored);
	return(gleX);
}

// This returns a QPainterPath object that can be used to draw
// the requested object (although we should just use draw())
QPainterPath GLEComponent::path()
{
	return(*paintPath);
}

// A slot that is called whenever the resolution changes
void GLEComponent::setDPI(double newDPI)
{
	if (dpi != newDPI)
	{
		dpi = newDPI;
		paintPen.setWidthF(QGLE::relGLEToQt(lineWidth, 0, dpi).x());
	}
}

// A slot that is called whenever the image size changes
void GLEComponent::setPixmapSize(QSize newPixmapSize, double newDPI)
{
	setDPI(newDPI);
	pixmap = newPixmapSize;
	emit imageChanged();
}

