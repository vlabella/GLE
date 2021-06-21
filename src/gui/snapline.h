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

#ifndef _SNAPLINE_H
#define _SNAPLINE_H

#include "component.h"

class SnapLine : public GLEComponent
{
	Q_OBJECT


public:
	enum Points
	{
		StartPoint,
		EndPoint,
		Angle
	};

	SnapLine(double resolution, QSize imageSize, QObject *parent = 0);

	bool isEqual(SnapLine sn);
	bool isEqual(SnapLine *sn);

	QString getKey();

	//! Draw the line on the provided painter
	void draw(QPainter *p);
	//! Return the shortest distance between a given point and the line
	double distanceToPoint(const QPointF& p, QPointF *nearestPoint);
	//! Set one of the enumerated points
	void setPoint(int pointChoice, const QPointF& p, bool update = true);


	//! Find points where a line intersects the line
	// If vertical = true, m contains 'x'
	QList<QPointF> intersections(double qtm, double qtc, bool vertical = false);
	QList<QPointF> intersections(QPointF qtp1, double angle);
	QList<QPointF> intersections(QPointF qtp1, QPointF qtp2);

	void deactivate();
	bool isActive();
	void activate();

private slots:
	//! Update the painter path when the resolution or start/end points change
	void updateLine();

private:
	bool active;
	bool ready;

	QString key;

	bool hitsRightBorder(QPointF *pt);
	bool hitsLeftBorder(QPointF *pt);
	bool hitsTopBorder(QPointF *pt);
	bool hitsBottomBorder(QPointF *pt);

};


#endif
