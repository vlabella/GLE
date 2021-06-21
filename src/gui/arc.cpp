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

#include "arc.h"

// The constructor for the line object
GLEArc::GLEArc(double resolution, QSize imageSize, GLEDrawingArea *area) :
	GLEDrawingObject(resolution, imageSize, area)
{
	directionReversed = false;
	// Make sure the line is updated if a point changes or
	// the image size changes
	connect(this, SIGNAL(pointChanged()),
			this, SLOT(updateArc()));
	connect(this, SIGNAL(imageChanged()),
			this, SLOT(updateArc()));
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

void GLEArc::createOTracks()
{
	if (isSet(CentrePoint) && isSet(StartPoint) && isSet(EndPoint))
	{
		// We need to know the page size to be able to do this!
	}
}

void GLEArc::updateFromProperty(int)
{
	GLEArcDO* obj = (GLEArcDO*)getGLEObject();
	GLEPropertyStore* obj_prop = obj->getProperties();

	obj_prop->setRealProperty(GLEDOPropertyLineWidth, getProperty(LineWidth).toDouble());
	QColor color = getProperty(LineColour).value<QColor>();
	GLEColor* gle_color = obj->getProperties()->getColorProperty(GLEDOPropertyColor);
	gle_color->setRGB255(color.red(), color.green(), color.blue());
	QGLE::qtToGLEString(getProperty(LineStyle).toString(), obj_prop->getStringProperty(GLEDOPropertyLineStyle));
	obj->setArrow((GLEHasArrow)getProperty(ArrowHeadPresence).toInt());

	((GLEDrawingArea*)parent())->setDirtyAndSave();
}

void GLEArc::getGLEArcT0T1(double* t0, double* t1)
{
	if (directionReversed) {
		*t0 = endAngleDeg();
		*t1 = startAngleDeg();
	} else {
		*t0 = startAngleDeg();
		*t1 = endAngleDeg();
	}
	*t1 = GLEArcNormalizedAngle2(*t0, *t1);
}

// Update the painter path
void GLEArc::updateArc()
{
	QPointF p;
	QPair<QPointF,int> snap;
	// Only do this if both the centre and radius have been set
	if (isSet(CentrePoint) && isSet(StartPoint) && isSet(EndPoint))
	{
		// This is guaranteed to have been created in the constructor
		// of GLEDrawingObject
		delete(paintPath);
		// Create a new path starting at one end of the arc
		if (!directionReversed)
		{
			paintPath = new QPainterPath(getQtPoint(StartPoint));
			// Add the circle based on the qrect
			paintPath->arcTo(arcRect(), startAngleDeg(), QGLE::radiansToDegrees(endAngleRadConstrained()-startAngleRad()));
		}
		else
		{
			paintPath = new QPainterPath(getQtPoint(EndPoint));
			paintPath->arcTo(arcRect(), endAngleDeg(), 360-QGLE::radiansToDegrees(endAngleRadConstrained()-startAngleRad()));
		}

		// Update the GLE Code
/*		gleCode->clear();
		if (directionReversed)
			gleCode->append(QString("! %1")
					.arg(tr("Clockwise arc added using QGLE", "Comment added to source file")));
		else
			gleCode->append(QString("! %1")
					.arg(tr("Anti-clockwise arc added using QGLE", "Comment added to source file")));

		gleCode->append(QString("asetpos %1")
				.arg(QGLE::GLEToStr(getGLEPoint(CentrePoint))));

		if (directionReversed)
			gleCode->append(QString("narc %1 %2 %3")
					.arg(QGLE::GLEToStr(getGLELength(Radius)))
					.arg(QGLE::GLEToStr(startAngleDeg()))
					.arg(QGLE::GLEToStr(endAngleDeg())));
		else
			gleCode->append(QString("arc %1 %2 %3")
					.arg(QGLE::GLEToStr(getGLELength(Radius)))
					.arg(QGLE::GLEToStr(startAngleDeg()))
					.arg(QGLE::GLEToStr(endAngleDeg())));

		gleCode->append(QString(""));*/

		// Update GLE object
		GLEArcDO* arc = (GLEArcDO*)getGLEObject();
		if (arc != NULL) {
			arc->setCenter(QGLE::QPointFToGLEPoint(getGLEPoint(CentrePoint)));
			arc->setRadius(getGLELength(Radius));
			double t0, t1;
			getGLEArcT0T1(&t0, &t1);
			arc->setAngle1(t0);
			arc->setAngle2(t1);
		}

		// Now we add the osnap handles
		osnapHandles.clear();
		for(int i=0;i<360;i+=90)
		{
			if (isOnArc(QGLE::degreesToRadians(i)))
			{
				p.setX(getGLELength(Radius)*cos(QGLE::degreesToRadians(i)));
				p.setY(getGLELength(Radius)*sin(QGLE::degreesToRadians(i)));
				p += getGLEPoint(CentrePoint);
				snap.first = QGLE::absGLEToQt(p,dpi,pixmap.height());
				snap.second = QGLE::QuadrantSnap;
				osnapHandles.append(snap);
			}
		}
		snap.first = getQtPoint(CentrePoint);
		snap.second = QGLE::CentreSnap;
		osnapHandles.append(snap);

		snap.first = getQtPoint(StartPoint);
		snap.second = QGLE::EndPointSnap;
		osnapHandles.append(snap);

		snap.first = getQtPoint(EndPoint);
		snap.second = QGLE::EndPointSnap;
		osnapHandles.append(snap);
	}
}

void GLEArc::addRelativeOSnaps(QPointF p)
{
	if (isSet(CentrePoint) && isSet(Radius) && isSet(StartPoint) && isSet(EndPoint))
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

//		double angle;
//		// The above is identical to the circle code, now check they're on the arc
//		for(int i=relativeOSnaps.size()-1;i>=0;i--)
//		{
//			angle = QGLE::angleBetweenTwoPoints(getGLEPoint(CentrePoint),
//					QGLE::absQtToGLE(relativeOSnaps[i].first,dpi,pixmap.height()));
//			if (!isOnArc(angle))
//				relativeOSnaps.removeAt(i);
//		}
	}
}

QList<QPointF> GLEArc::getTangents(QPointF p)
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

		double angle = angleToCentre + angleOffset;
		np = p + QPointF(distanceToTangent*cos(angle),distanceToTangent*sin(angle));
		if (isOnArc(QGLE::angleBetweenTwoPoints(
						getGLEPoint(CentrePoint),
						QGLE::absQtToGLE(np,dpi,pixmap.height()))))
			tangents.append(np);

		angle = angleToCentre - angleOffset;
		np = p + QPointF(distanceToTangent*cos(angle),distanceToTangent*sin(angle));
		if (isOnArc(QGLE::angleBetweenTwoPoints(
						getGLEPoint(CentrePoint),
						QGLE::absQtToGLE(np,dpi,pixmap.height()))))
			tangents.append(np);

	}
	return(tangents);
}

bool GLEArc::hasTangents()
{
	return(true);
}

QList<QPointF> GLEArc::getPerpendiculars(QPointF p)
{
	QList<QPointF> perpendiculars;

	// The first perpendicular osnap is defined as the nearest point
	QPointF np;
	distanceToPointOnCircle(p,&np);
	perpendiculars.append(np);

	// The second perpendicular osnap is diametrically opposite the first one
	double angleToCentre = QGLE::angleBetweenTwoPoints(np,getQtPoint(CentrePoint));
	double radius = getQtLength(Radius);
	double diameter = 2*radius;

	np = np + QPointF(diameter*cos(angleToCentre),diameter*sin(angleToCentre));
	perpendiculars.append(np);

	// Are the points on the arc?
	double angle;
	for(int i=perpendiculars.size()-1;i>=0;i--)
	{
		angle = QGLE::angleBetweenTwoPoints(getGLEPoint(CentrePoint),
				QGLE::absQtToGLE(perpendiculars[i],dpi,pixmap.height()));
		if (!isOnArc(angle))
			perpendiculars.removeAt(i);
	}

	return(perpendiculars);
}

bool GLEArc::hasPerpendiculars()
{
	return(true);
}

void GLEArc::drawArc(QPainter *p, double t1, double t2)
{
	p->drawArc(arcRect(), (int)(t1 * 16), (int)((t2 - t1) * 16));
}

void GLEArc::addBezier(QPainterPath *p, GLEBezier* bezier) {
	QPointF p1(QGLE::absGLEToQt(bezier->getP1(), dpi, pixmap.height()));
	QPointF p2(QGLE::absGLEToQt(bezier->getP2(), dpi, pixmap.height()));
	QPointF p3(QGLE::absGLEToQt(bezier->getP3(), dpi, pixmap.height()));
	p->cubicTo(p1, p2, p3);
}

void GLEArc::computeAndDraw(QPainter *p, GLEArcDO* obj, GLECurvedArrowHead* head) {
	if (!head->isEnabled()) {
		return;
	}
	head->computeArrowHead();
	QPainterPath path;
	path.moveTo(QGLE::absGLEToQt(head->getSide1()->getP0(), dpi, pixmap.height()));
	addBezier(&path, head->getSide1());
	addBezier(&path, head->getSide2());
	if (head->getStyle() != GLEArrowStyleSimple) {
		path.closeSubpath();
		if (head->getStyle() == GLEArrowStyleEmpty) {
			p->fillPath(path, QBrush(Qt::white));
		} else {
			QColor col;
			GLEColor* color = obj->getProperties()->getColorProperty(GLEDOPropertyColor);
			col.setRgbF(color->getRed(), color->getGreen(), color->getBlue());
			p->fillPath(path, QBrush(col));
		}
	}
	if (!head->isSharp()) {
		p->drawPath(path);
	}
}

void GLEArc::draw(QPainter *p)
{
	QPen cpen;
	setPenProperties(cpen);
	p->setPen(cpen);
	if (!(isSet(StartPoint) && isSet(EndPoint))) {
		return;
	}
	if (isSet(CentrePoint)) {
		double t1, t2;
		getGLEArcT0T1(&t1, &t2);
		GLEArcDO* obj = (GLEArcDO*)getGLEObject();
		if (obj != NULL && obj->getArrow() != 0) {
			double r = getGLELength(Radius);
			GLEPoint orig(getGLEPoint(CentrePoint).x(), getGLEPoint(CentrePoint).y());
			GLECircleArc circle(orig, r, QGLE::degreesToRadians(t1), QGLE::degreesToRadians(t2));
			GLECurvedArrowHead head_start(&circle);
			GLECurvedArrowHead head_end(&circle);
			GLEArcUpdateCurvedArrowHeads(&head_start, &head_end, &t1, &t2, obj->getProperties(), 1.0, obj->getArrow());
			drawArc(p, t1, t2);
			QPen new_pen = cpen;
			new_pen.setJoinStyle(Qt::RoundJoin);
			new_pen.setMiterLimit(20);
			new_pen.setStyle(Qt::SolidLine);
			p->setPen(new_pen);
			computeAndDraw(p, obj, &head_start);
			computeAndDraw(p, obj, &head_end);
		} else {
			drawArc(p, t1, t2);
		}
	} else {
		// If we don't have a centre point, just draw a line
		p->drawLine(getQtPoint(StartPoint),getQtPoint(EndPoint));
	}
}

double GLEArc::distanceToPointOnCircle(QPointF p, QPointF *nearestPoint)
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

double GLEArc::distanceToPoint(const QPointF& p, QPointF *nearestPoint)
{
	// Calculations in GLE and Qt coordinates (to make the angles work!)
	double s, e, angle;
	QPointF cp, sp, ep;
	QPointF gleP = QGLE::absQtToGLE(p, dpi, pixmap.height());
	QPointF np;

	// We need all the points, so if any are missing, return a very large number!
	if (!(isSet(CentrePoint) && isSet(Radius) && isSet(StartPoint) && isSet(EndPoint)))
		return(1e6);

	sp = getQtPoint(StartPoint);
	ep = getQtPoint(EndPoint);

	// Get the centre point and calculate the angle between the current point and it
	cp = getGLEPoint(CentrePoint);
	angle = QGLE::angleBetweenTwoPoints(cp,gleP);

	// If the point is on the arc (in terms of angle), the calculation is simple
	if (isOnArc(angle))
	{
		if (nearestPoint)
		{
			double r = getGLELength(Radius);
			np.setX(r*cos(angle)+cp.x());
			np.setY(r*sin(angle)+cp.y());
			*nearestPoint = QGLE::absGLEToQt(np, dpi, pixmap.height());
		}

		cp = getQtPoint(CentrePoint);
		return(fabs(QGLE::distance(cp,p)-getQtLength(Radius)));
	}


	// Otherwise, the distance will be the shortest distance to one of the end points
	s = QGLE::distance(p,sp);
	e = QGLE::distance(p,ep);

	if (s < e)
	{
		if (nearestPoint)
			*nearestPoint = sp;
		return(s);
	}
	else
	{
		if (nearestPoint)
			*nearestPoint = ep;
		return(e);
	}
}

bool GLEArc::isOnArc(double angle)
{
	// Decide whether an angle is on the drawn part of an arc
	double s, e;

	if (!(isSet(CentrePoint) && isSet(Radius) && isSet(StartPoint) && isSet(EndPoint)))
		return(false);


	// Choose the start and end angles according to the direction of drawing
	if (!directionReversed)
	{
		s = startAngleRad();
		e = endAngleRad();
	}
	else
	{
		s = endAngleRad();
		e = startAngleRad();
	}

	// Ensure that the end point is greater than the start point
	while (e < s)
		e += 2*M_PI;

	int i;

	// Decide whether the angle is in between the two points
	for (i=0;i<2;i++)
	{
		angle += i*2*M_PI;
		if ((angle >= s) && (angle <= e))
			return(true);
	}

	return(false);

}

// Set a point (start or end in the case of a line)
void GLEArc::setPoint(int pointChoice, const QPointF& p, bool update)
{
	QPointF s,e,m,c,mid1,mid2;
	double mg1,mg2,x,y;
	bool ok = false;
	switch(pointChoice)
	{

		// Set the radius based on a centre point
		case CentrePoint:
			s = getGLEPoint(StartPoint);
			pointHash[CentrePoint] = QGLE::absQtToGLE(p, dpi, pixmap.height());
			x = QGLE::distance(getGLEPoint(CentrePoint),s);
			if (x > 0.0)
				pointHash[Radius] = QPointF(x,0);
			break;

		case MidPoint:
			// Find the centre point and radius based on a random
			// point on the arc
			if (!(isSet(StartPoint) && isSet(EndPoint)))
				return;

			// Get the basic points in GLE coordinates
			s = getGLEPoint(StartPoint);
			e = getGLEPoint(EndPoint);
			m = QGLE::absQtToGLE(p, dpi,pixmap.height());

			// Find the halfway points
			mid1 = QPointF((s.x()+m.x())/2.0,
					(s.y()+m.y())/2.0);
			mid2 = QPointF((e.x()+m.x())/2.0,
					(e.y()+m.y())/2.0);

			if ((m.y() == s.y()) && (m.x() != s.x()) && (m.y() != e.y()))
			{
				// start and mid point are on a horizontal line, but
				// points are not coincident or on the same line as
				// s - e
				//qDebug() << "1: " << s << m << e;

				mg2 = (e.x()-m.x())/(m.y()-e.y());
				x = mid1.x();
				y = mg2*x + mid2.y() - mg2*mid2.x();

				c = QPointF(x,y);

				pointHash[CentrePoint] = c;
				pointHash[Radius] = QPointF(QGLE::distance(c,m),0);

				if (!isOnArc(QGLE::angleBetweenTwoPoints(c,m)))
					directionReversed = !directionReversed;

				ok = true;

			}
			else if ((m.y() == e.y()) && (m.x() != e.x()) && (m.y() != s.y()))
			{
				//qDebug() << "2: " << s << m << e;
				// end and mid point are on a horizontal line, but
				// points are not coincident or on the same line as
				// s - e
				mg1 = (s.x()-m.x())/(m.y()-s.y());
				x = mid2.x();
				y = mg1*x + mid1.y() - mg1*mid1.x();

				c = QPointF(x,y);

				pointHash[CentrePoint] = c;
				pointHash[Radius] = QPointF(QGLE::distance(c,m),0);

				if (!isOnArc(QGLE::angleBetweenTwoPoints(c,m)))
					directionReversed = !directionReversed;

				ok = true;
			}
			else if ((m.y() != e.y()) && (m.y() != s.y()))
			{
				//qDebug() << "3";
				// The most common case: points are nicely spaced

				// Find the gradients of the normals to the intersecting lines
				mg1 = (s.x()-m.x())/(m.y()-s.y());
				mg2 = (e.x()-m.x())/(m.y()-e.y());

				if (!(mg1 == mg2))
				{
					x = (mid2.y() - mg2*mid2.x() - mid1.y() + mg1*mid1.x())/
						(mg1 - mg2);
					y = mg1*x + mid1.y() - mg1*mid1.x();

					c = QPointF(x,y);

					pointHash[CentrePoint] = c;
					pointHash[Radius] = QPointF(QGLE::distance(c,m),0);

					if (!isOnArc(QGLE::angleBetweenTwoPoints(c,m)))
						directionReversed = !directionReversed;

					ok = true;
				}
			}

			if (!ok)
			{
				pointHash.remove(CentrePoint);
				pointHash.remove(Radius);
			}



			// The simple cases:
		case StartPoint:
		case EndPoint:
			pointHash[pointChoice] = QGLE::absQtToGLE(p, dpi, pixmap.height());
			break;
	}

	// Update the arc
	if (update) updateArc();
}

double GLEArc::startAngleDeg()
{
	// Return the angle between the centre and the start point
	// in degrees
	return(QGLE::radiansToDegrees(startAngleRad()));

}

double GLEArc::endAngleDeg()
{
	// Return the angle between the centre and the end point
	// in degrees
	return(QGLE::radiansToDegrees(endAngleRad()));

}

double GLEArc::endAngleDegConstrained()
{
	// Return the angle between the centre and the end point
	// in degrees and constrained to be greater than the
	// start point (by adding 2*pi)
	double e = endAngleDeg();
	double s = startAngleDeg();
	while (e < s)
		e += 360.0;
	return(e);
}

double GLEArc::startAngleRad()
{
	// Return the angle between the centre and the start point
	// in radians
	QPointF s = getGLEPoint(StartPoint);
	QPointF c = getGLEPoint(CentrePoint);
	return(QGLE::angleBetweenTwoPoints(c,s));
}

double GLEArc::endAngleRad()
{
	// Return the angle between the centre and the end point
	// in radians
	QPointF e = getGLEPoint(EndPoint);
	QPointF c = getGLEPoint(CentrePoint);
	return(QGLE::angleBetweenTwoPoints(c,e));
}

double GLEArc::endAngleRadConstrained()
{
	// Return the angle between the centre and the end point
	// in radians and constrained to be greater than the start
	// start point (by adding 2*pi)
	double e = endAngleRad();
	double s = startAngleRad();
	while (e < s)
		e += 2*M_PI;
	return(e);
}


QRectF GLEArc::arcRect()
{
	// Return the rectangle surrounding the circle upon which the arc is based

	// Work in Qt coordinates
	QPointF cp = getQtPoint(CentrePoint);
	double r = getQtLength(Radius);

	if ((isSet(Radius) && isSet(CentrePoint)))
		return(QRectF(cp.x()-r, cp.y() - r, 2*r, 2*r));
	else
		return(QRectF(0,0,0,0));

}


QList<QPointF> GLEArc::intersections(double qtm, double qtc, bool vertical)
{
	QPointF one, two;
	// For circles and arcs, we'll deal with points individually:
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


QList<QPointF> GLEArc::intersections(QPointF qtp1, QPointF qtp2)
{
	// Based on http://tinyurl.com/qtgum
	QList<QPointF> pointArray;

	QPointF cp = getQtPoint(CentrePoint);
	double r = getQtLength(Radius);
	double a,b,c;
	double bac;
	double u;
	double angle;
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

		angle = QGLE::angleBetweenTwoPoints(getGLEPoint(CentrePoint),
				QGLE::absQtToGLE(p,dpi,pixmap.height()));
		if (isOnArc(angle))
			pointArray.append(p);
	}
	else if (bac > 0.0)
	{
		u = (-b + sqrt(bac))/(2*a);
		p.setX(qtp1.x() + u*(qtp2.x()-qtp1.x()));
		p.setY(qtp1.y() + u*(qtp2.y()-qtp1.y()));
		angle = QGLE::angleBetweenTwoPoints(getGLEPoint(CentrePoint),
				QGLE::absQtToGLE(p,dpi,pixmap.height()));
		if (isOnArc(angle))
			pointArray.append(p);
		u = (-b - sqrt(bac))/(2*a);
		p.setX(qtp1.x() + u*(qtp2.x()-qtp1.x()));
		p.setY(qtp1.y() + u*(qtp2.y()-qtp1.y()));
		angle = QGLE::angleBetweenTwoPoints(getGLEPoint(CentrePoint),
				QGLE::absQtToGLE(p,dpi,pixmap.height()));
		if (isOnArc(angle))
			pointArray.append(p);
	}

	return(pointArray);
}

QList<QPointF> GLEArc::intersections(QPointF qtp1, double angle)
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

bool GLEArc::isInside(QPointF p)
{
	QPointF cp = getGLEPoint(CentrePoint);
	QPointF gp = QGLE::absQtToGLE(p,dpi,pixmap.height());
	double angle;
	if (QGLE::distance(cp,gp) < getGLELength(Radius))
	{
		angle = QGLE::angleBetweenTwoPoints(cp,gp);
		if (isOnArc(angle))
			return(true);
	}

	return(false);
}

bool GLEArc::isInsideCircle(QPointF p)
{
	QPointF cp = getQtPoint(CentrePoint);
	if (QGLE::distance(cp,p) < getQtLength(Radius))
		return(true);
	else
		return(false);
}

double GLEArc::nearestPriorityOSnap(const QPointF& pt, QPointF *osnap)
{
	QPointF startPt = getQtPoint(StartPoint, true);
	QPointF endPt = getQtPoint(EndPoint, true);
	double d1 = QGLE::distance(pt, startPt);
	double d2 = QGLE::distance(pt, endPt);
	if (d1 < d2) {
		*osnap = startPt;
		return d1;
	} else {
		*osnap = endPt;
		return d2;
	}
}

bool GLEArc::findScaleOrigin(const QPointF& pt, QPointF* origin, int)
{
	QPointF startPt = getQtPoint(StartPoint, true);
	QPointF endPt = getQtPoint(EndPoint, true);
	if (QGLE::distance(pt, startPt) < QGLE::distance(pt, endPt))
		*origin = endPt;
	else
		*origin = startPt;
	return true;
}

int GLEArc::supportedTransformMode()
{
	return TransformModeFree;
}

void GLEArc::linearTransform(const GLELinearEquation& ex, const GLELinearEquation& ey)
{
	// The arc will need to support becoming an
	// elliptical_n?arc before this can be properly
	// supported.
	pointHash.clear();
	linearTransformPt(StartPoint, ex, ey, false);
	linearTransformPt(EndPoint, ex, ey, false);
	linearTransformPt(CentrePoint, ex, ey, false);
	if (storedPointHash.contains(MidPoint))
	{
		linearTransformPt(MidPoint, ex, ey, false);
	}
	updateArc();
}

void GLEArc::moveBy(QPointF offset)
{
	pointHash.clear();
	// Moves relative to storedPointHash
	QPointF pt = getQtPoint(StartPoint, true);
	setPoint(StartPoint, pt + offset, false);

	pt = getQtPoint(EndPoint, true);
	setPoint(EndPoint, pt + offset, false);

	pt = getQtPoint(CentrePoint, true);
	setPoint(CentrePoint, pt + offset, false);

	if (storedPointHash.contains(MidPoint))
	{
		pt = getQtPoint(MidPoint, true);
		setPoint(MidPoint, pt + offset, false);
	}
	updateArc();
}

void GLEArc::rotateBy(double radians)
{
	pointHash.clear();
	// Moves relative to storedPointHash
	QPointF pt = getQtPoint(CentrePoint, true);
	setPoint(CentrePoint, QGLE::rotateAboutPoint(pt, radians, basePoint));

	pt = getQtPoint(StartPoint, true);
	setPoint(StartPoint, QGLE::rotateAboutPoint(pt, radians, basePoint));

	pt = getQtPoint(EndPoint, true);
	setPoint(EndPoint, QGLE::rotateAboutPoint(pt, radians, basePoint));

	if (storedPointHash.contains(MidPoint))
	{
		pt = getQtPoint(MidPoint, true);
		setPoint(MidPoint, QGLE::rotateAboutPoint(pt, radians, basePoint));
	}
}
