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

#include "amove.h"

// The constructor for the line object
GLEAmove::GLEAmove(double resolution, QSize imageSize, GLEDrawingArea *area) :
	GLEDrawingObject(resolution, imageSize, area)
{
	// Make sure the line is updated if a point changes or
	// the image size changes
	connect(this, SIGNAL(pointChanged()),
			this, SLOT(updateAmove()));
	connect(this, SIGNAL(imageChanged()),
			this, SLOT(updateAmove()));

	amove = true;
}

void GLEAmove::createOTracks()
{
}

// Update the painter path
void GLEAmove::updateAmove()
{
	// Only do this if both the start and end have been set
	if (isSet(CentrePoint))
	{
		// This is guaranteed to have been created in the constructor
		// of GLEDrawingObject
		delete(paintPath);
		// Create a new path starting at lineStart
		paintPath = new QPainterPath();
		QPointF qtAmove = getQtPoint(CentrePoint);

		paintPath->addRect(QRectF(qtAmove.x()-AMOVE_LENGTH,
					qtAmove.y()-AMOVE_LENGTH,
					2*AMOVE_LENGTH,
					2*AMOVE_LENGTH));

		osnapHandles.clear();
		osnapHandles.append(QPair<QPointF,int>(qtAmove,QGLE::CentreSnap));
	}
}

void GLEAmove::addRelativeOSnaps(QPointF p)
{
	UNUSED_ARG(p);
}

void GLEAmove::draw(QPainter *p)
{
	QGLE::drawCross(p, getQtPoint(CentrePoint), AMOVE_LENGTH, pen());
}

QList<QPointF> GLEAmove::getTangents(QPointF p)
{
	QList<QPointF> empty;
	UNUSED_ARG(p);
	return(empty);
}

bool GLEAmove::hasTangents()
{
	return(false);
}

QList<QPointF> GLEAmove::getPerpendiculars(QPointF p)
{
	QList<QPointF> empty;
	UNUSED_ARG(p);
	return(empty);
}

bool GLEAmove::hasPerpendiculars()
{
	return(false);
}




double GLEAmove::distanceToPoint(const QPointF& p, QPointF *nearestPoint)
{
	QPointF amovePoint = getQtPoint(CentrePoint);
	if (nearestPoint)
		*nearestPoint = amovePoint;
	return(QGLE::distance(amovePoint, p));
}

// Set a point (start or end in the case of a line)
void GLEAmove::setPoint(int pointChoice, const QPointF& p, bool)
{
	switch(pointChoice)
	{
		case CentrePoint:
			pointHash[pointChoice] = QGLE::absQtToGLE(p, dpi, pixmap.height());
			break;
	}
	updateAmove();
}

QList<QPointF> GLEAmove::intersections(double qtm, double qtc, bool vertical)
{
	UNUSED_ARG(qtm);
	UNUSED_ARG(qtc);
	UNUSED_ARG(vertical);
	return QList<QPointF>();
}

QList<QPointF> GLEAmove::intersections(QPointF qtp1, QPointF qtp2)
{
	UNUSED_ARG(qtp1);
	UNUSED_ARG(qtp2);
	return QList<QPointF>();
}

QList<QPointF> GLEAmove::intersections(QPointF qtp1, double angle)
{
	UNUSED_ARG(qtp1);
	UNUSED_ARG(angle);
	return QList<QPointF>();

}

bool GLEAmove::isInside(QPointF p)
{
	UNUSED_ARG(p);
	return(false);
}


void GLEAmove::scaleBy(double xratio, double yratio)
{
	pointHash.clear();

	QPointF oldOffset = getQtPoint(CentrePoint, true) - basePoint;
	QPointF newOffset;
	newOffset.setX(oldOffset.x() * xratio);
	newOffset.setY(oldOffset.y() * yratio);

	setPoint(CentrePoint, basePoint + newOffset);
}

void GLEAmove::moveBy(QPointF offset)
{
	pointHash.clear();
	// Moves relative to storedPointHash
	QPointF start = getQtPoint(CentrePoint, true);
	setPoint(CentrePoint, start + offset);
}

void GLEAmove::rotateBy(double radians)
{
	pointHash.clear();
	// Moves relative to storedPointHash
	QPointF start = getQtPoint(CentrePoint, true);
	setPoint(CentrePoint, QGLE::rotateAboutPoint(start, radians, basePoint));
}
