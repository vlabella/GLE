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

#include "circle.h"

// The constructor for the circle object
GLECircle::GLECircle(double resolution, QSize imageSize, GLEDrawingArea *area) :
	GLEDrawingObject(resolution, imageSize, area)
{
	// Make sure the circle is updated if a point changes or
	// the image size changes
	connect(this, SIGNAL(pointChanged()),
			this, SLOT(updateCircle()));
	connect(this, SIGNAL(imageChanged()),
			this, SLOT(updateCircle()));
	connect(this, SIGNAL(propertyChanged(int)),
			this, SLOT(updateFromProperty(int)));

	amove = false;

	// More to be added
	validProperties
		<< FillColour
		<< LineColour
		<< LineWidth
		<< LineStyle;
	properties[FillColour] = propertyDescriptions[FillColour].defaultValue;
	properties[LineColour] = propertyDescriptions[LineColour].defaultValue;
	properties[LineWidth] = propertyDescriptions[LineWidth].defaultValue;
	properties[LineStyle] = propertyDescriptions[LineStyle].defaultValue;
}

void GLECircle::createOTracks()
{
	if (isSet(CentrePoint) && isSet(Radius))
	{
		// We need to know the page size to be able to do this!
	}
}

void GLECircle::updateFromProperty(int)
{
	GLEEllipseDO* obj = (GLEEllipseDO*)getGLEObject();
	GLEPropertyStore* obj_prop = obj->getProperties();

	obj_prop->setRealProperty(GLEDOPropertyLineWidth, getProperty(LineWidth).toDouble());
	QColor color = getProperty(LineColour).value<QColor>();
	GLEColor* gle_color = obj->getProperties()->getColorProperty(GLEDOPropertyColor);
	gle_color->setRGB255(color.red(), color.green(), color.blue());
	QGLE::qtToGLEString(getProperty(LineStyle).toString(), obj_prop->getStringProperty(GLEDOPropertyLineStyle));

	GLEColor* gle_fill = obj->getProperties()->getColorProperty(GLEDOPropertyFillColor);
	if (properties[FillColour].value<QColor>().isValid()) {
		QColor fill = getProperty(FillColour).value<QColor>();
		gle_fill->setRGB255(fill.red(), fill.green(), fill.blue());
	} else {
		gle_fill->setTransparent(true);
	}

	((GLEDrawingArea*)parent())->setDirtyAndSave();
}

// Update the painter path
void GLECircle::updateCircle()
{
	QPointF p;
	QPair<QPointF,int> snap;

	// Only do this if both the centre and radius have been set
	if (isSet(CentrePoint) && isSet(Radius))
	{
		// This is guaranteed to have been created in the constructor
		// of GLEDrawingObject
		delete(paintPath);
		// Create a new path starting at circleCentre
		paintPath = new QPainterPath(getQtPoint(CentrePoint));
		// Add the circle based on the qrect
		paintPath->addEllipse(circleRect());

		// Update GLE object
		GLEEllipseDO* circle = (GLEEllipseDO*)getGLEObject();
		if (circle != NULL) {
			circle->setCenter(QGLE::QPointFToGLEPoint(getGLEPoint(CentrePoint)));
			circle->setRadius(getGLELength(Radius));
		}

		// Now we add the osnap handles
		osnapHandles.clear();
		for(int i=0;i<360;i+=90)
		{
			p.setX(getQtLength(Radius)*cos(QGLE::degreesToRadians(i)));
			p.setY(getQtLength(Radius)*sin(QGLE::degreesToRadians(i)));
			p += getQtPoint(CentrePoint);
			snap.first = p;
			snap.second = QGLE::QuadrantSnap;
			osnapHandles.append(snap);
		}
		snap.first = getQtPoint(CentrePoint);
		snap.second = QGLE::CentreSnap;
		osnapHandles.append(snap);
	}
}

void GLECircle::addRelativeOSnaps(QPointF p)
{
	if (isSet(CentrePoint) && isSet(Radius))
	{
		relativeOSnaps.clear();

		QList<QPointF> perpendiculars = getPerpendiculars(p);
		QPointF ppt;
		foreach(ppt, perpendiculars)
		{
			relativeOSnaps.append(QPair<QPointF,int>(ppt, QGLE::PerpendicularSnap));
		}

		QList<QPointF> tangents = getTangents(p);

		QPointF tpt;
		foreach(tpt, tangents)
		{
			relativeOSnaps.append(QPair<QPointF,int>(tpt, QGLE::TangentSnap));
		}

	}
}

QList<QPointF> GLECircle::getTangents(QPointF p)
{
	QList<QPointF> tangents;
	QPointF np;
	if (!isInside(p))
	{
		double angleToCentre = QGLE::angleBetweenTwoPoints(p,getQtPoint(CentrePoint));
		double radius = getQtLength(Radius);
		// Now we need the tangential ones
		double distanceToCentre = QGLE::distance(p, getQtPoint(CentrePoint));
		double angleOffset = asin(radius/distanceToCentre);
		double distanceToTangent = sqrt(pow(distanceToCentre,2)-pow(radius,2));
		angleToCentre = QGLE::angleBetweenTwoPoints(p,getQtPoint(CentrePoint));

		double angle = angleToCentre + angleOffset;
		np = p + QPointF(distanceToTangent*cos(angle),distanceToTangent*sin(angle));
		tangents.append(np);

		angle = angleToCentre - angleOffset;
		np = p + QPointF(distanceToTangent*cos(angle),distanceToTangent*sin(angle));
		tangents.append(np);

	}
	return(tangents);
}

bool GLECircle::hasTangents()
{
	return(true);
}

QList<QPointF> GLECircle::getPerpendiculars(QPointF p)
{
	QList<QPointF> perpendiculars;

	// The first perpendicular osnap is defined as the nearest point
	QPointF np;
	distanceToPoint(p,&np);
	perpendiculars.append(np);

	// The second perpendicular osnap is diametrically opposite the first one
	double angleToCentre = QGLE::angleBetweenTwoPoints(np,getQtPoint(CentrePoint));
	double radius = getQtLength(Radius);
	double diameter = 2*radius;

	np = np + QPointF(diameter*cos(angleToCentre),diameter*sin(angleToCentre));
	perpendiculars.append(np);

	return(perpendiculars);
}

bool GLECircle::hasPerpendiculars()
{
	return(true);
}

void GLECircle::draw(QPainter *p)
{
	if (isSet(CentrePoint) && isSet(Radius))
	{
		p->save();
		QPen cpen;
		setSimplePenProperties(cpen);
		p->setPen(cpen);
		if (properties[FillColour].value<QColor>().isValid())
			p->setBrush(QBrush(properties[FillColour].value<QColor>()));
		else
			p->setBrush(Qt::NoBrush);
		p->drawEllipse(circleRect());
		p->restore();
	}
}

double GLECircle::distanceToPoint(const QPointF& p, QPointF *nearestPoint)
{
	QPointF c = getQtPoint(CentrePoint);
	if (nearestPoint)
	{
		double r = getQtLength(Radius);
		double theta = QGLE::angleBetweenTwoPoints(c,p);
		nearestPoint->setX(r*cos(theta)+c.x());
		nearestPoint->setY(r*sin(theta)+c.y());
	}
	// Calculations in QT coordinates
	return(fabs(QGLE::distance(c,p)-getQtLength(Radius)));
}

// Set a point (start or end in the case of a line)
void GLECircle::setPoint(int pointChoice, const QPointF& p, bool update)
{
	double radius;
	switch(pointChoice)
	{
		// Note that circumference point also runs
		// the centre point code: this is intentional
		case CircumferencePoint:
			if (!isSet(CentrePoint))
				return;
			radius = QGLE::distance(QGLE::absQtToGLE(p,dpi,pixmap.height()),getGLEPoint(CentrePoint));
			if (QGLE::relGLEToQt(radius,dpi) > MINIMUM_CIRCLE_RADIUS)
				pointHash[Radius] = QPointF(radius, 0);
			else
				pointHash.remove(Radius);
		case CentrePoint:
			pointHash[pointChoice] = QGLE::absQtToGLE(p, dpi, pixmap.height());
			break;
		case Radius:
			pointHash[pointChoice] = QGLE::relQtToGLE(p, dpi);
	}

	if (update) updateCircle();
}

QRectF GLECircle::circleRect()
{
	// Work in Qt coordinates
	QPointF c = getQtPoint(CentrePoint);
	double r = getQtLength(Radius);

	return(QRectF(c.x() - r,
				c.y() - r,
				2*r, 2*r));
}

QList<QPointF> GLECircle::intersections(double qtm, double qtc, bool vertical)
{
	QPointF one, two;
	// For circles, we'll deal with points individually:
	if (vertical)
	{
		one.setX(qtm);
		two.setX(qtm);
		one.setY(0.0);
		two.setY(1.0);
	}
	else
	{
		one.setX(0.0);
		one.setY(qtc);
		two.setX(1.0);
		two.setY(qtm + qtc);
	}

	return(intersections(one,two));
}


QList<QPointF> GLECircle::intersections(QPointF qtp1, QPointF qtp2)
{
	QList<QPointF> pointArray;

	if (!(isSet(CentrePoint) && isSet(Radius)))
		return(pointArray);

	QPointF cp = getQtPoint(CentrePoint);
	double r = getQtLength(Radius);
	double a,b,c;
	double bac;
	double u;
	QPointF p;

	a = pow((qtp2.x() - qtp1.x()),2) + pow((qtp2.y() - qtp1.y()),2);
	b = 2 * ( (qtp2.x() - qtp1.x())*(qtp1.x()-cp.x()) + (qtp2.y()-qtp1.y())*(qtp1.y()-cp.y()));
	c = pow(cp.x(),2)+pow(cp.y(),2)+pow(qtp1.x(),2)+pow(qtp1.y(),2)
		- 2*(cp.x()*qtp1.x()+cp.y()*qtp1.y()) - pow(r,2);

	bac = pow(b,2)-4*a*c;
	if (bac == 0.0)
	{
		u = - b / (2*a);
		p.setX(qtp1.x()+u*(qtp2.x()-qtp1.x()));
		p.setY(qtp1.y()+u*(qtp2.y()-qtp1.y()));
		pointArray.append(p);
	}
	else if (bac > 0.0)
	{
		u = (-b + sqrt(bac))/(2*a);
		p.setX(qtp1.x() + u*(qtp2.x()-qtp1.x()));
		p.setY(qtp1.y() + u*(qtp2.y()-qtp1.y()));
		pointArray.append(p);
		u = (-b - sqrt(bac))/(2*a);
		p.setX(qtp1.x() + u*(qtp2.x()-qtp1.x()));
		p.setY(qtp1.y() + u*(qtp2.y()-qtp1.y()));
		pointArray.append(p);
	}

	return(pointArray);
}

QList<QPointF> GLECircle::intersections(QPointF qtp1, double angle)
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

bool GLECircle::isInside(QPointF p)
{
	QPointF cp = getQtPoint(CentrePoint);
	if (QGLE::distance(cp,p) < getQtLength(Radius))
		return(true);
	else
		return(false);
}

bool GLECircle::findScaleOrigin(const QPointF&, QPointF* origin, int handle)
{
	if (handle == GLEDrawingArea::TopLeft || handle == GLEDrawingArea::TopRight || handle == GLEDrawingArea::BottomLeft || handle == GLEDrawingArea::BottomRight)
	{
		*origin = getQtPoint(CentrePoint, true);
		return true;
	}
	return false;
}

int GLECircle::supportedTransformMode()
{
	return TransformModeConstrained;
}

void GLECircle::linearTransform(const GLELinearEquation& ex, const GLELinearEquation& ey)
{
	// Make sure we have a circumference point at 45 degrees
//	QPointF cp = getQtPoint(CentrePoint, true);
//	double radius = getQtLength(Radius, true);
//	QPointF edgePoint = cp + QPointF(
//			radius*cos(QGLE::degreesToRadians(45)),
//			radius*sin(QGLE::degreesToRadians(45)));

	// It would probably be much more sensible to do this after
	// converting it to an ellipse.
	pointHash.clear();
	double radius = getQtLength(Radius, true);
	double fac = ex.getA();
	if (fac == 1.0) fac = ey.getA();
	setLength(Radius, radius * fac, false);
	linearTransformPt(CentrePoint, ex, ey, false);
	if (storedPointHash.contains(CircumferencePoint))
	{
		linearTransformPt(CircumferencePoint, ex, ey, false);
	}
	updateCircle();
}

void GLECircle::moveBy(QPointF offset)
{
	pointHash.clear();
	double radius = getQtLength(Radius);
	setLength(Radius, radius, false);
	// Moves relative to storedPointHash
	QPointF pt = getQtPoint(CentrePoint, true);
	setPoint(CentrePoint, pt + offset, false);
	if (storedPointHash.contains(CircumferencePoint))
	{
		QPointF pt = getQtPoint(CircumferencePoint, true);
		setPoint(CircumferencePoint, pt + offset, false);
	}
	updateCircle();
}

void GLECircle::rotateBy(double radians)
{
	double radius = getQtLength(Radius);
	pointHash.clear();
	setPoint(Radius, QPointF(radius,0.0));

	QPointF c = getQtPoint(CentrePoint, true);
	c = QGLE::rotateAboutPoint(c, radians, basePoint);
	setPoint(CentrePoint, c);

	if (storedPointHash.contains(CircumferencePoint))
	{
		QPointF pt = getQtPoint(CircumferencePoint, true);
		setPoint(CircumferencePoint, QGLE::rotateAboutPoint(pt, radians, basePoint));
	}
}

