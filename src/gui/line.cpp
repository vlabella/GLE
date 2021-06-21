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

#include "line.h"

// The constructor for the line object
GLELine::GLELine(double resolution, QSize imageSize, GLEDrawingArea *area) :
	GLEDrawingObject(resolution, imageSize, area)
{
	// Make sure the line is updated if a point changes or
	// the image size changes
	connect(this, SIGNAL(pointChanged()),
			this, SLOT(updateLine()));
	connect(this, SIGNAL(imageChanged()),
			this, SLOT(updateLine()));
	connect(this, SIGNAL(propertyChanged(int)),
			this, SLOT(updateFromProperty(int)));

	validProperties
		<< LineColour
		<< LineWidth
		<< LineStyle
		<< ArrowHeadPresence;
	properties[LineColour] = propertyDescriptions[LineColour].defaultValue;
	properties[LineWidth] = propertyDescriptions[LineWidth].defaultValue;
	properties[LineStyle] = propertyDescriptions[LineStyle].defaultValue;
	properties[ArrowHeadPresence] = propertyDescriptions[ArrowHeadPresence].defaultValue;

	amove = false;
}

// Update the painter path
void GLELine::updateLine()
{
	// Only do this if both the start and end have been set
	if (isSet(StartPoint) && isSet(EndPoint))
	{
		// This is guaranteed to have been created in the constructor
		// of GLEDrawingObject
		delete(paintPath);
		// Create a new path starting at lineStart
		paintPath = new QPainterPath(getQtPoint(StartPoint));
		// Make it a line ending at lineEnd
		paintPath->lineTo(getQtPoint(EndPoint));

		// Update GLE object
		GLELineDO* line = (GLELineDO*)getGLEObject();
		if (line != NULL) {
			line->setP1(QGLE::QPointFToGLEPoint(getGLEPoint(StartPoint)));
			line->setP2(QGLE::QPointFToGLEPoint(getGLEPoint(EndPoint)));
		}

		osnapHandles.clear();
		QPointF s = getQtPoint(StartPoint);
		QPointF e = getQtPoint(EndPoint);
		osnapHandles.append(QPair<QPointF,int>(s,QGLE::EndPointSnap));
		osnapHandles.append(QPair<QPointF,int>(e,QGLE::EndPointSnap));
		osnapHandles.append(QPair<QPointF,int>((s+e)/2,QGLE::MidPointSnap));
	}
}

void GLELine::updateFromProperty(int prop)
{
	GLELineDO* obj = (GLELineDO*)getGLEObject();
	GLEPropertyStore* obj_prop = obj->getProperties();

	double lwd = getProperty(LineWidth).toDouble();
	obj_prop->setRealProperty(GLEDOPropertyLineWidth, lwd);
	QColor color = getProperty(LineColour).value<QColor>();
	GLEColor* gle_color = obj_prop->getColorProperty(GLEDOPropertyColor);
	gle_color->setRGB255(color.red(), color.green(), color.blue());
	QGLE::qtToGLEString(getProperty(LineStyle).toString(), obj_prop->getStringProperty(GLEDOPropertyLineStyle));
	obj->setArrow((GLEHasArrow)getProperty(ArrowHeadPresence).toInt());
	if (prop == LineWidth) {
		GLESetDefaultArrowProperties(lwd, obj_prop);
	}
	((GLEDrawingArea*)parent())->setDirtyAndSave();
}

void GLELine::createOTracks()
{
	if (isSet(StartPoint) && isSet(EndPoint))
	{
		// TODO: create some snap lines!
		// Also need to implement the necessary functionality
		// in gledrawing.cpp
	}
}

void GLELine::addRelativeOSnaps(QPointF p)
{
	if (isSet(StartPoint) && isSet(EndPoint))
	{
		// qDebug() << "Adding osnap relative to " << QGLE::absQtToGLE(p,dpi,pixmap.height());
		relativeOSnaps.clear();
		QPointF np;
		distanceToPoint(p,&np);
		// qDebug() << "Nearest point: " << QGLE::absQtToGLE(np,dpi,pixmap.height());
		if ((np == getQtPoint(StartPoint)) || (np == getQtPoint(EndPoint)))
			return;
		relativeOSnaps.append(QPair<QPointF,int>(np,QGLE::PerpendicularSnap));
	}
	// qDebug() << "Added relative OSNAP";
}

QList<QPointF> GLELine::getTangents(QPointF)
{
	QList<QPointF> empty;
	return(empty);
}

bool GLELine::hasTangents()
{
	return(false);
}

QList<QPointF> GLELine::getPerpendiculars(QPointF p)
{
	QList<QPointF> perpendiculars;
	QPointF np;
	distanceToPoint(p,&np);
	if ((np == getQtPoint(StartPoint)) || (np == getQtPoint(EndPoint)))
		return(perpendiculars);

	perpendiculars.append(np);
	return(perpendiculars);
}

bool GLELine::hasPerpendiculars()
{
	return(true);
}


void GLELine::draw(QPainter *p)
{
	QPen cpen;
	setPenProperties(cpen);
	GLELineDO* obj = (GLELineDO*)getGLEObject();
	if (obj != NULL && obj->getArrow() != 0) {
		GLEArrowPoints pts1, pts2;
		GLEPoint start_pt(getQtPoint(StartPoint).x(), getQtPoint(StartPoint).y());
		GLEPoint end_pt(getQtPoint(EndPoint).x(), getQtPoint(EndPoint).y());
		double dx = end_pt.getX()-start_pt.getX();
		double dy = end_pt.getY()-start_pt.getY();
		GLEGetArrowPoints(start_pt, dx, dy, obj->getProperties(), dpi/CM_PER_INCH, &pts1);
		GLEGetArrowPoints(end_pt, -dx, -dy, obj->getProperties(), dpi/CM_PER_INCH, &pts2);
		QPen new_pen = cpen;
		int tip = obj->getProperties()->getIntProperty(GLEDOPropertyArrowTip);
		int style = obj->getProperties()->getIntProperty(GLEDOPropertyArrowStyle);
		if (tip == GLEArrowTipRound) new_pen.setJoinStyle(Qt::RoundJoin);
		else new_pen.setJoinStyle(Qt::MiterJoin);
		new_pen.setMiterLimit(20);
		new_pen.setStyle(Qt::SolidLine);
		GLEHasArrow arrow = obj->getArrow();
		if ((arrow & GLEHasArrowStart) != 0) start_pt.setXY(pts1.xl, pts1.yl);
		if ((arrow & GLEHasArrowEnd) != 0) end_pt.setXY(pts2.xl, pts2.yl);
		p->setPen(cpen);
		p->drawLine(QPointF(start_pt.getX(), start_pt.getY()), QPointF(end_pt.getX(), end_pt.getY()));
		p->setPen(new_pen);
		if ((arrow & GLEHasArrowStart) != 0) {
			QPointF poly[3];
			poly[0] = QPointF(pts1.xa, pts1.ya);
			poly[1] = QPointF(pts1.xt, pts1.yt);
			poly[2] = QPointF(pts1.xb, pts1.yb);
			if (style == GLEArrowStyleFilled) {
				QColor col;
				GLEColor* color = obj->getProperties()->getColorProperty(GLEDOPropertyColor);
				col.setRgbF(color->getRed(), color->getGreen(), color->getBlue());
				p->setBrush(QBrush(col));
			} else {
				p->setBrush(QBrush(Qt::white));
			}
			if (style == GLEArrowStyleSimple) p->drawPolyline(poly, 3);
			else p->drawConvexPolygon(poly, 3);
		}
		if ((arrow & GLEHasArrowEnd) != 0) {
			QPointF poly[3];
			poly[0] = QPointF(pts2.xa, pts2.ya);
			poly[1] = QPointF(pts2.xt, pts2.yt);
			poly[2] = QPointF(pts2.xb, pts2.yb);
			if (style == GLEArrowStyleFilled) {
				QColor col;
				GLEColor* color = obj->getProperties()->getColorProperty(GLEDOPropertyColor);
				col.setRgbF(color->getRed(), color->getGreen(), color->getBlue());
				p->setBrush(QBrush(col));
			} else {
				p->setBrush(QBrush(Qt::white));
			}
			if (style == GLEArrowStyleSimple) p->drawPolyline(poly, 3);
			else p->drawConvexPolygon(poly, 3);
		}
	} else {
		p->setPen(cpen);
		p->drawLine(getQtPoint(StartPoint),getQtPoint(EndPoint));
	}
}


double GLELine::distanceToPoint(const QPointF& p, QPointF *nearestPoint)
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
	 * b^2 + c^2 -2.b.c.cosA) is greater than 90�, the distance to the point is *
	 * the same as the line length s-p.  Likewise for the angle pes and the     *
	 * line length e-p.  If both angles are less than 90�, the distance can be  *
	 * calculated simply.  The shortest distance will be the length of the line *
	 * from p to the line se that meets the line at 90�, thus:                  *
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
void GLELine::setPoint(int pointChoice, const QPointF& p, bool update)
{
	switch(pointChoice)
	{
		case EndPoint:
		case StartPoint:
			pointHash[pointChoice] = QGLE::absQtToGLE(p, dpi, pixmap.height());
			break;
	}
	if (update) updateLine();
}

QList<QPointF> GLELine::intersections(double qtm, double qtc, bool vertical)
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





QList<QPointF> GLELine::intersections(QPointF qtp1, QPointF qtp2)
{
	double m, c;
	if (qtp2.x() == qtp1.x())
		return(intersections(qtp2.x(), 0, true));

	m = (qtp2.y() - qtp1.y())/(qtp2.x()-qtp1.x());
	c = qtp2.y() - m*qtp2.x();

	return(intersections(m,c,false));
}

QList<QPointF> GLELine::intersections(QPointF qtp1, double angle)
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

bool GLELine::isInside(QPointF p)
{
	UNUSED_ARG(p);
	return(false);
}

int GLELine::supportedTransformMode()
{
	return TransformModeFree;
}

void GLELine::linearTransform(const GLELinearEquation& ex, const GLELinearEquation& ey)
{
	pointHash.clear();
	linearTransformPt(StartPoint, ex, ey, false);
	linearTransformPt(EndPoint, ex, ey, false);
	updateLine();
}

void GLELine::moveBy(QPointF offset)
{
	pointHash.clear();
	// Moves relative to storedPointHash
	QPointF start = getQtPoint(StartPoint, true);
	setPoint(StartPoint, start + offset, false);
	QPointF end = getQtPoint(EndPoint, true);
	setPoint(EndPoint, end + offset, false);
	updateLine();
}

void GLELine::rotateBy(double radians)
{
	pointHash.clear();
	// Moves relative to storedPointHash
	QPointF start = getQtPoint(StartPoint, true);
	setPoint(StartPoint, QGLE::rotateAboutPoint(start, radians, basePoint));
	QPointF end = getQtPoint(EndPoint, true);
	setPoint(EndPoint, QGLE::rotateAboutPoint(end, radians, basePoint));
}
