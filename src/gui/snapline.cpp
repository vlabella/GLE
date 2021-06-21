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

#include <math.h>
#include "snapline.h"


// The constructor for snap the line object
SnapLine::SnapLine(double resolution, QSize imageSize, QObject *parent) :
	GLEComponent(resolution, imageSize, parent)
{
	// Make sure the line is updated if a point changes or
	// the image size changes
	connect(this, SIGNAL(pointChanged()),
			this, SLOT(updateLine()));
	connect(this, SIGNAL(imageChanged()),
			this, SLOT(updateLine()));

	active = false;
}

// Update the snap line
void SnapLine::updateLine()
{
	key = QString("%1 %2 %3 %4 %5")
		.arg(getGLEPoint(StartPoint).x())
		.arg(getGLEPoint(StartPoint).y())
		.arg(getGLEPoint(EndPoint).x())
		.arg(getGLEPoint(EndPoint).y())
		.arg(getGLEAngle(Angle));

	if (isSet(StartPoint) && isSet(EndPoint) && isSet(Angle))
	{
		ready = true;
		active = true;
	}
	else
	{
		ready = false;
		active = false;
	}
}

QString SnapLine::getKey()
{
	return(key);
}

bool SnapLine::isEqual(SnapLine sn)
{
	if (key == sn.getKey())
		return(true);
	else
		return(false);
}

bool SnapLine::isEqual(SnapLine *sn)
{
	if (key == sn->getKey())
		return(true);
	else
		return(false);
}

void SnapLine::draw(QPainter *p)
{
	if (!isActive())
		return;
	p->setPen(QGLE::osnapLinePen());
	p->drawLine(getQtPoint(StartPoint), getQtPoint(EndPoint));
}

double SnapLine::distanceToPoint(const QPointF& p, QPointF *nearestPoint)
{
	// All calculations in QT coordinates
	// Distances between start, end and the point
	double se, sp, ep;
	QPointF s, e;
	// Angle between se and sp
	double pse;
	// Angle between se and ep
	double pes;


	/****************************************************************************
	 * This calculation is based on the triangle:                               *
	 *    p                                                                     *
	 *    /\                                                                    *
	 *   /  \                                                                   *
	 * s<____>e                                                                 *
	 *                                                                          *
	 * where s and e are the end points of the line and p is the point that has *
	 * been clicked.  If the angle pse (calculated using the cosine rule: a^2 = *
	 * b^2 + c^2 -2.b.c.cosA) is greater than 90º, the distance to the point is *
	 * the same as the line length s-p.  Likewise for the angle pes and the     *
	 * line length e-p.  If both angles are less than 90º, the distance can be  *
	 * calculated simply.  The shortest distance will be the length of the line *
	 * from p to the line se that meets the line at 90º, thus:                  *
	 *    p                                                                     *
	 *    /|                                                                    *
	 *   / |                                                                    *
	 * s<__|__e                                                                 *
	 * The distance is therefore sp*sin(pse).                                   *
	 ****************************************************************************/

	s = getQtPoint(StartPoint);
	e = getQtPoint(EndPoint);

	se = QGLE::distance(s,e);
	sp = QGLE::distance(s,p);
	ep = QGLE::distance(e,p);

	//qDebug() << "Distances: " << se << " " << sp << " " << ep;

	if ((sp == 0.0) || (ep == 0.0))
		return(0.0);

	if (se == 0.0)
		return(sp);

	double ac;

	// Calculated the bit inside the 'acos'
	ac = (se*se + sp*sp - ep*ep)/(2*se*sp);
	// Make sure it isn't greater than 1
	if (ac > 1.0)
		ac = 1.0;
	else if (ac < -1.0)
		ac = -1.0;
	pse = acos(ac);

	// Calculated the bit inside the 'acos'
	ac = (se*se + ep*ep - sp*sp)/(2*se*ep);
	// Make sure it isn't greater than 1
	if (ac > 1.0)
		ac = 1.0;
	else if (ac < -1.0)
		ac = -1.0;
	pes = acos(ac);

	//qDebug() << "Angles: " << pse << " " << pes;

	if ( pse >= M_PI/2.0 )
	{
		if (nearestPoint)
			*nearestPoint = s;
		return(sp);
	}
	else if ( pes >= M_PI/2.0 )
	{
		if (nearestPoint)
			*nearestPoint = e;
		return(ep);
	}
	else
	{
		if (nearestPoint)
			*nearestPoint = (sp*cos(pse)/se)*(e-s) + s;
		return(sp*sin(pse));
	}
}

// Set a point (start or end in the case of a line)
void SnapLine::setPoint(int pointChoice, const QPointF& p, bool)
{
	QPointF pt;
	double angle = p.x();
	switch(pointChoice)
	{
		case Angle:
			if (!isSet(StartPoint))
				return;
			pointHash[Angle] = p;
			// Now we need to know the point at which the line meets
			// the page border

			switch(QGLE::quadrant(angle))
			{
				case 1: // Either top or right border
					if (hitsTopBorder(&pt))
						pointHash[EndPoint] = pt;
					else if (hitsRightBorder(&pt))
						pointHash[EndPoint] = pt;
					break;

				case 2: // Either hits top or left border
					if (hitsTopBorder(&pt))
						pointHash[EndPoint] = pt;
					else if (hitsLeftBorder(&pt))
						pointHash[EndPoint] = pt;
					break;

				case 3: // Either hits left or bottom border
					if (hitsBottomBorder(&pt))
						pointHash[EndPoint] = pt;
					else if (hitsLeftBorder(&pt))
						pointHash[EndPoint] = pt;
					break;

				case 4: // Either hits right or bottom border
					if (hitsBottomBorder(&pt))
						pointHash[EndPoint] = pt;
					else if (hitsRightBorder(&pt))
						pointHash[EndPoint] = pt;
					break;
			}
			break;

		case EndPoint:
		case StartPoint:
			if (isSet(StartPoint) && isSet(EndPoint))
			{
				QPointF ang;
				ang.setX(QGLE::angleBetweenTwoPoints(getGLEPoint(StartPoint), getGLEPoint(EndPoint)));
				pointHash[Angle] = ang;
			}
			pointHash[pointChoice] = QGLE::absQtToGLE(p, dpi, pixmap.height());
			break;

	}
	updateLine();
}

bool SnapLine::hitsRightBorder(QPointF *pt)
{
	QPointF p;
	QPointF sp = getQtPoint(StartPoint);
	// CHECK THIS!!!!
	double angle = getGLEAngle(Angle);

	// If the line is vertical, it'll never hit the right border
	if (fmod(QGLE::radiansToDegrees(angle)-90.0,180.0) == 0.0)
		return(false);

	// If in quadrant 2 or 3, it'll never hit the right border
	int q = QGLE::quadrant(angle);
	if (q == 2)
		return(false);
	if (q == 3)
		return(false);

	p.setX(pixmap.width());
	p.setY(sp.y()-(pixmap.width()-sp.x())*tan(angle));

	if (QGLE::inOrder(0.0,p.y(),pixmap.height()))
	{
		*pt = QGLE::absQtToGLE(p,dpi,pixmap.height());
		return(true);
	}
	return(false);


}

bool SnapLine::hitsLeftBorder(QPointF *pt)
{
	QPointF p;
	QPointF sp = getQtPoint(StartPoint);
	double angle = getGLEAngle(Angle);

	// If the line is vertical, it'll never hit the left border
	if (fmod(QGLE::radiansToDegrees(angle)-90.0,180.0) == 0.0)
		return(false);

	int q = QGLE::quadrant(angle);
	if (q == 1)
		return(false);
	if (q == 4)
		return(false);

	// Since we're looking at the left hand side, lets have
	// the angle down to the -x axis
	angle = M_PI - angle;

	p.setX(0.0);
	p.setY(sp.y() - sp.x()*tan(angle));

	if (QGLE::inOrder(0.0,p.y(),pixmap.height()))
	{
		*pt = QGLE::absQtToGLE(p,dpi,pixmap.height());
		return(true);
	}
	return(false);
}

bool SnapLine::hitsTopBorder(QPointF *pt)
{
	QPointF p;
	QPointF sp = getQtPoint(StartPoint);
	double angle = getGLEAngle(Angle);

	p.setY(0.0); // This is the top border in Qt coordinates

	// If the line is horizontal, it'll never hit the top border
	if (fmod(QGLE::radiansToDegrees(angle),180.0) == 0.0)
	{
		return(false);
	}

	// If vertical, it's easy
	if (QGLE::radiansToDegrees(angle) == 90.0)
	{
		p.setX(sp.x());
		*pt = QGLE::absQtToGLE(p,dpi,pixmap.height());
		return(true);
	}

	// If in quadrant 3 or 4, it'll never hit the top border
	int q = QGLE::quadrant(angle);
	if (q >= 3)
		return(false);

	// If in quadrant 2, we want the angle to the -x axis
	if (q == 2)
		angle = M_PI - angle;

	if (q == 1)
		p.setX(sp.x() + sp.y()*tan((M_PI/2.0)-angle));
	if (q == 2)
		p.setX(sp.x() - sp.y()*tan((M_PI/2.0)-angle));

	if (QGLE::inOrder(0.0,p.x(),pixmap.width()))
	{
		*pt = QGLE::absQtToGLE(p,dpi,pixmap.height());
		return(true);
	}
	return(false);
}

bool SnapLine::hitsBottomBorder(QPointF *pt)
{
	QPointF p;
	QPointF sp = getQtPoint(StartPoint);
	double angle = getGLEAngle(Angle);

	p.setY(pixmap.height()); // This is the bottom border in Qt coordinates

	// If the line is horizontal, it'll never hit the bottom border
	if (fmod(QGLE::radiansToDegrees(angle),180.0) == 0.0)
		return(false);

	// If vertical, it's easy
	if (QGLE::radiansToDegrees(angle) == 270.0)
	{
		p.setX(sp.x());
		*pt = QGLE::absQtToGLE(p,dpi,pixmap.height());
		return(true);
	}

	// If in quadrant 1 or 2, it'll never hit the top border
	int q = QGLE::quadrant(angle);
	if (q <= 2)
		return(false);

	// If in quadrant 3, we want the angle to the -x axis
	if (q == 3)
		angle = angle - M_PI;

	// If in quadrant 4, we want the angle to the +x axis
	if (q == 4)
		angle = 2*M_PI - angle;

	if (q == 4)
		p.setX(sp.x() + (pixmap.height()-sp.y())*tan((M_PI/2)-angle));
	if (q == 3)
		p.setX(sp.x() - (pixmap.height()-sp.y())*tan((M_PI/2)-angle));

	if (QGLE::inOrder(0.0,p.x(),pixmap.width()))
	{
		*pt = QGLE::absQtToGLE(p,dpi,pixmap.height());
		return(true);
	}
	return(false);
}


QList<QPointF> SnapLine::intersections(double qtm, double qtc, bool vertical)
{
	QList<QPointF> pointArray;
	QPointF sp, ep;
	QPointF intersect;
	sp = getQtPoint(StartPoint);
	ep = getQtPoint(EndPoint);
	double mym, myc;
	if (vertical)
	{
		// If both lines are vertical, no intersection (or an infinite number)
		if ((!(sp.x() == ep.x())) && (QGLE::inOrder(sp.x(), qtm, ep.x())))
		{
			// Lines intersect
			intersect = QPointF(qtm, ((qtm-sp.x())/(ep.x()-sp.x()))*(ep.y()-sp.y())+sp.y());
			pointArray.append(intersect);
		}
	}
	else
	{

		// Not vertical
		if (sp.x() == ep.x())
		{
			// But this line is!
			intersect.setX(sp.x());
			intersect.setY(qtm*sp.x()+qtc);
			if (QGLE::inOrder(sp.y(),intersect.y(),ep.y()))
				pointArray.append(intersect);
		}
		else
		{
			// Neither line is vertical
			mym = (ep.y()-sp.y())/(ep.x()-sp.x());
			myc = ep.y() - mym*ep.x();

			intersect.setX((myc-qtc)/(qtm-mym));
			intersect.setY(mym*intersect.x()+myc);

			if ((QGLE::inOrder(sp.y(),intersect.y(),ep.y())) &&
					(QGLE::inOrder(sp.x(), intersect.x(), ep.x())))
				pointArray.append(intersect);
		}
	}

	return(pointArray);
}

QList<QPointF> SnapLine::intersections(QPointF qtp1, QPointF qtp2)
{
	double m, c;
	if (qtp2.x() == qtp1.x())
		return(intersections(qtp2.x(), 0, true));

	m = (qtp2.y() - qtp1.y())/(qtp2.x()-qtp1.x());
	c = qtp2.y() - m*qtp2.x();

	return(intersections(m,c,false));
}

QList<QPointF> SnapLine::intersections(QPointF qtp1, double angle)
{
	// This intersection code must determine the intersections in
	// a particular direction from a start point

	// First get a list based on an infinite line:
	QPointF qtp2 = qtp1 + QPointF(1.0*cos(angle),1.0*sin(angle));
	QList<QPointF> allIntersections = intersections(qtp1,qtp2);

	// Now go through the list and determine which are in the right
	// direction
	QList<QPointF> correctIntersections;
	QPointF pt;
	double ptAngle;

	foreach(pt, allIntersections)
	{
		ptAngle = QGLE::angleBetweenTwoPoints(qtp1, pt);
		if (QGLE::quadrant(ptAngle) == QGLE::quadrant(angle))
			correctIntersections.append(pt);
	}

	return(correctIntersections);

}

void SnapLine::deactivate()
{
	active = false;
}

void SnapLine::activate()
{
	active = true;
}

bool SnapLine::isActive()
{
	return(active);
}

