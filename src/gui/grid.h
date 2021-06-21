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

#ifndef _GRID_H
#define _GRID_H

#include <QtGui>
#include "qgle_definitions.h"

//! Class implementing a grid for drawing
class GLEGrid : public QObject
{
	Q_OBJECT

public:
	//! Constructor: initialise the grid
	GLEGrid(QObject *parent = 0);
	//! Find the nearest grid point to a given point (QT coordinates)
	QPointF nearestPoint(QPointF qt, double *distance = 0);
	//! Return all (QT) grid points
	QList<QPointF> allPoints();
	//! Return the pen with which to draw the grid
	QPen pen();
	//! Draw the grid
	void drawGrid(QPainter* paint);

public slots:
	//! Set the grid spacing
	void setGrid(QPointF newGrid);
	//! Set the drawing area
	void setArea(QSize area);
	//! Set the resolution
	void setDPI(double new_dpi);

private:
	//! Update the pen with a new resolution
	void updatePen();

	//! The grid X & Y spacing
	QPointF gridSpacing;
	//! The drawing area size
	QSizeF areaSize;
	//! The resolution
	double dpi;

	//! The pen with which to draw the grid
	QPen paintPen;
};

#endif
