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

#include "qgle_definitions.h"
#include "qgle_statics.h"
#include "grid.h"
#include <math.h>

// The constructor, initialise variables and clear the grid
GLEGrid::GLEGrid(QObject *parent) : QObject(parent)
{
	gridSpacing = QPointF(0.0,0.0);
	dpi = 0;

	// don't compute all points: this is too slow for small grid sizes
	// because the number of points quadratically increases as grid size decreases

	paintPen.setStyle(Qt::SolidLine);
	paintPen.setCapStyle(Qt::RoundCap);
	paintPen.setColor(Qt::black);
	//paintPen.setWidth(0);
	paintPen.setWidth(1);

	areaSize = QSizeF(0.0,0.0);
}

// SLOT: Set the grid spacing
void GLEGrid::setGrid(QPointF newGrid)
{
	if (gridSpacing != newGrid)
	{
		gridSpacing = newGrid;
	}
}

// SLOT: Set the pixmap area
void GLEGrid::setArea(QSize area)
{
	if (area != areaSize)
	{
		areaSize = area;
	}
}

// SLOT: set the resolution
void GLEGrid::setDPI(double new_dpi)
{
	if (dpi != new_dpi)
	{
		dpi = new_dpi;
		updatePen();
	}
}

// Update the pen width according to the resolution
void GLEGrid::updatePen()
{
	//paintPen.setWidth((int) floor(dpi/100.0));
	paintPen.setWidth(1);
}

QPen GLEGrid::pen()
{
	return(paintPen);
}

//! Draw the grid
void GLEGrid::drawGrid(QPainter* paint)
{
	// Only set up the grid if we have the spacings and dpi
	if ((gridSpacing.x() == 0.0) || (gridSpacing.y() == 0.0) || (dpi == 0.0) || (areaSize.height() == 0))
	{
		return;
	}

	// Get the grid spacing in QT units
	QPointF qtGrid = QGLE::relGLEToQt(gridSpacing, dpi);

	// Create the grid
	int nbX = (int)floor(areaSize.width()/qtGrid.x()+1.0);
	int nbY = (int)floor(areaSize.height()/qtGrid.y()+1.0);
	for (int x = 0; x < nbX; x ++)
	{
		double xp = GS_OFFSET*dpi + x*qtGrid.x();
		for (int y = 0; y < nbY; y++)
		{
			double yp = GS_OFFSET*dpi + y*qtGrid.y();
			paint->drawPoint(QPointF(xp, areaSize.height() - yp));
		}
	}
}

// Find the nearest point to a given (QT) point: used for snap to grid
QPointF GLEGrid::nearestPoint(QPointF qt, double *distance)
{
	// Only set up the grid if we have the spacings and dpi
	if ((gridSpacing.x() == 0.0) || (gridSpacing.y() == 0.0) || (dpi == 0.0) || (areaSize.height() == 0))
	{
		if (distance)
			*distance = 1e6;
		return QPointF();
	}

	// Get the grid spacing in QT units
	QPointF qtGrid = QGLE::relGLEToQt(gridSpacing,dpi);

	// Find the nearest point
	double xp = qt.x() - GS_OFFSET*dpi;
	double yp = areaSize.height() - qt.y() - GS_OFFSET*dpi;

	int x = max((int)floor(xp / qtGrid.x() + 0.5), 0);
	int y = max((int)floor(yp / qtGrid.y() + 0.5), 0);

	double nearX = GS_OFFSET*dpi + x*qtGrid.x();
	double nearY = areaSize.height() - (GS_OFFSET*dpi + y*qtGrid.y());

	if (distance)
	{
		double dx = qt.x()-nearX;
		double dy = qt.y()-nearY;
		*distance = sqrt(dx*dx + dy*dy);
	}

	return QPointF(nearX, nearY);
}


