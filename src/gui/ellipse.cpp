/***********************************************************************************
 * QGLE - A Graphical Interface to GLE                                             *
 * Copyright (C) 2006  A. S. Budden, S. Wilkinson & J. Struyf                      *
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

#include "ellipse.h"

// The constructor for the ellipse object
GLEEllipse::GLEEllipse(double resolution, QSize imageSize, GLEDrawingArea *area) :
	GLEDrawingObject(resolution, imageSize, area)
{
	// Make sure the ellipse is updated if a point changes or
	// the image size changes
	connect(this, SIGNAL(pointChanged()),
			this, SLOT(updateEllipse()));
	connect(this, SIGNAL(imageChanged()),
			this, SLOT(updateEllipse()));
	connect(this, SIGNAL(propertyChanged(int)),
			this, SLOT(updateFromProperty(int)));

	amove = false;

	pointHash[Angle] = QPointF(0.0, 0.0);

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

void GLEEllipse::createOTracks()
{
}

void GLEEllipse::updateFromProperty(int)
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
void GLEEllipse::updateEllipse()
{
	QPointF p;
	QPair<QPointF,int> snap;
	double angle;

	// Only do this if both the centre and radius have been set
	if (isSet(CentrePoint) && isSet(RadiusX) && isSet(RadiusY) && isSet(Angle))
	{
		// This is guaranteed to have been created in the constructor
		// of GLEDrawingObject
		delete(paintPath);
		// Create a new path starting at ellipseCentre
		paintPath = new QPainterPath(getQtPoint(CentrePoint));
		// Add the ellipse based on the qrect
		// This needs to be modified to suit the angle!
		// CHECK: I think we only use this for bounding boxes,
		// so just make a rectangle of the bounding box

		angle = getGLEAngle(Angle);

		// Get the bounding box of the ellipse
		QRectF rect;
		rect.setTopLeft(getQtPoint(BBoxCornerA));
		rect.setBottomRight(getQtPoint(BBoxCornerB));
		paintPath->addRect(rect);

		// Update GLE object
		GLEEllipseDO* ellipse = (GLEEllipseDO*)getGLEObject();
		if (ellipse != NULL) {
			ellipse->setCenter(QGLE::QPointFToGLEPoint(getGLEPoint(CentrePoint)));
			ellipse->setRadiusX(getGLELength(RadiusX));
			ellipse->setRadiusY(getGLELength(RadiusY));
		}

		// Update the calculated equation parameters
		updateEquationParameters();

		// Now we add the osnap handles
		osnapHandles.clear();

		p.setX(getQtLength(RadiusX)*cos(M_PI-angle));
		p.setY(getQtLength(RadiusX)*sin(M_PI-angle));
		p += getQtPoint(CentrePoint);
		snap.first = p;
		snap.second = QGLE::QuadrantSnap;
		osnapHandles.append(snap);

		p.setX(-getQtLength(RadiusX)*cos(M_PI-angle));
		p.setY(-getQtLength(RadiusX)*sin(M_PI-angle));
		p += getQtPoint(CentrePoint);
		snap.first = p;
		snap.second = QGLE::QuadrantSnap;
		osnapHandles.append(snap);

		p.setX(-getQtLength(RadiusY)*sin(angle));
		p.setY(-getQtLength(RadiusY)*cos(angle));
		p += getQtPoint(CentrePoint);
		snap.first = p;
		snap.second = QGLE::QuadrantSnap;
		osnapHandles.append(snap);

		p.setX(getQtLength(RadiusY)*sin(angle));
		p.setY(getQtLength(RadiusY)*cos(angle));
		p += getQtPoint(CentrePoint);
		snap.first = p;
		snap.second = QGLE::QuadrantSnap;
		osnapHandles.append(snap);

		snap.first = getQtPoint(CentrePoint);
		snap.second = QGLE::CentreSnap;
		osnapHandles.append(snap);
	}
}

void GLEEllipse::addRelativeOSnaps(QPointF p)
{
	// This should be very similar to the circle one
	if (isSet(CentrePoint) && isSet(RadiusX) && isSet(RadiusY) && isSet(Angle))
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

QList<QPointF> GLEEllipse::getTangents(QPointF p)
{
	QList<QPointF> tangents;
	QPointF tangentPoint;
	int i;
	double A,B,C,delta,x,y;
	double m[2], c[2]; // slope and y-intercept

	if (!isInside(p))
	{
		QHash<char, double> coeffs = equationParameters;
		// coeffs['A'], coeffs['B'] etc
		// Ax^2 + By^2 + Cxy + Dx + Ey + F = 0

		// Tangent line equation y = mx + c is satisfied by p.x() and p.y()
		// Slope of a tangents, m, satisfies a quadratic: A*m^2 + B*m + C = 0
		// where A, B and C are:
		A = + pow(coeffs['E'],2)
			- 4*coeffs['B']*coeffs['F']
			+ ( 2*coeffs['C']*coeffs['E'] - 4*coeffs['B']*coeffs['D'] ) *p.x()
			+ ( pow(coeffs['C'],2) - 4*coeffs['A']*coeffs['B'] )*pow(p.x(),2);
		B = - 4*coeffs['C']*coeffs['F']
			+ 2*coeffs['D']*coeffs['E']
			+ ( 4*coeffs['B']*coeffs['D'] - 2*coeffs['C']*coeffs['E'] ) * p.y()
			+ ( 4*coeffs['A']*coeffs['E'] - 2*coeffs['C']*coeffs['D'] ) * p.x()
			+ ( 8*coeffs['A']*coeffs['B'] - 2*pow((coeffs['C']),2) ) * p.y() * p.x();
		C = + pow(coeffs['D'],2)
			- 4*coeffs['A']*coeffs['F']
			+ ( pow(coeffs['C'],2) - 4*coeffs['A']*coeffs['B'] ) * pow(p.y(),2)
			+ ( 2*coeffs['C']*coeffs['D'] - 4*coeffs['A']*coeffs['E'] ) * p.y();

		if (A != 0)
		{
			// Solving the quadratic in m gives 2 tangents:
			delta = pow(B,2) - 4*A*C;
			m[0] = 1/(2*A) * (-B + sqrt(delta));
			m[1] = 1/(2*A) * (-B - sqrt(delta));
			// Knowing the slope and a point on the tangent we find the y-intercept
			c[0] = p.y() - m[0] * p.x();
			c[1] = p.y() - m[1] * p.x();

			// Solve the quadratic which gives the intersection between ellipse and line
			// Either call GLEEllipse::lineIntersection, or do it here
			// In this case the intersection quadratic equation has one repeated real root
			// Single real root of A*x^2 + B*x + C = 0 is x=-B/(2*A)
			for (i=0;i<2;i++)
			{
				// Coefficients of the quadratic in x
				A = + coeffs['A']
					+ coeffs['C'] * m[i]
					+ coeffs['B'] * pow(m[i],2);
				B = + coeffs['D']
					+ coeffs['C'] * c[i]
					+ coeffs['E'] * m[i]
					+ 2*coeffs['B'] * m[i] * c[i];

				x = -B/(2*A);
				y = m[i] * x + c[i];

				// Work in Qt Coordinates: p is the 'point'
				tangentPoint = QPointF(x,y);

				// For each tangentPoint
				tangents.append(tangentPoint);
			}
		}
		else
		{
			// The quadratic in m reduces to a linear equation in m, giving one tangent
			// The other tangent is a vertical line x = p.x();
			m[0] = -C/B;
			c[0] = p.y() - m[0] * p.x();
			// Coefficients of the quadratic in x
			A = + coeffs['A']
				+ coeffs['C'] * m[0]
				+ coeffs['B'] * pow(m[0],2);
			B = + coeffs['D']
				+ coeffs['C'] * c[0]
				+ coeffs['E'] * m[0]
				+ 2*coeffs['B'] * m[0] * c[0];
			x = -B/(2*A);
			y = m[0] * x + c[0];
			tangentPoint = QPointF(x,y);
			tangents.append(tangentPoint);
			x = p.x();
			// can't use y=m*x+c here to work out the value of y!
			// instead plut x into the ellipse equation
			// solve yet another quadratic, this time in y
			// A*y^2 + B*y + C = 0
			// The desired value of y is a repeated real root, so need only A and B
			A = coeffs['B'];
			B = coeffs['E']
				+ coeffs['C'] * x;
			y = -B/(2*A);
			tangentPoint = QPointF(x,y);
			tangents.append(tangentPoint);
		}
	}
	return(tangents);
}

bool GLEEllipse::hasTangents()
{
	return(true);
}

QList<QPointF> GLEEllipse::getPerpendiculars(QPointF p)
{
	QList<QPointF> perpendiculars;

	QPointF c = getQtPoint(CentrePoint);
	double rx = getQtLength(RadiusX);
	double ry = getQtLength(RadiusY);
	double angle = getQtAngle(Angle);

	// Change coordinate system to be centred on origin
	QPointF translated = p - c;

	// Rotate about the origin to align the ellipse with the axis
	QPointF rotated = QGLE::parkTransform(translated, angle);

	QPointF perpPoint;

	double delta = M_PI;
	double t = -M_PI;
	int iterations = 0;
	double s, ds, tn;
	int i,j;

	// Search for the perpendicular points
	for (i=0; i<4; i++)
	{
		while ((fabs(delta) > ELLIPSE_PERP_ACCURACY) &&
				(iterations < MAX_ELLIPSE_PERP_ITERATIONS))
		{
			ds = -rx*rotated.x()*cos(t)
				- ry*rotated.y()*sin(t)
				+ (pow(rx,2)-pow(ry,2))*(pow(cos(t),2) - pow(sin(t),2));

			if (ds != 0.0)
			{
				s = -rx*rotated.x()*sin(t)
					+ ry*rotated.y()*cos(t)
					+ (pow(rx,2)-pow(ry,2))*sin(t)*cos(t);
				tn = t - (s/ds);
				delta = tn - t;
				t = tn;
			}
			else
			{
				t = t + M_PI/2.0;
			}
			iterations++;
		}

		perpPoint = QPointF(rx*cos(t),ry*sin(t));
		t = -M_PI + (i+1)*M_PI/2.0;
		iterations = 0;
		delta = M_PI;

		// Now unrotate
		QPointF unrotated = QGLE::parkTransform(perpPoint, -angle);

		// Now Untranslate
		QPointF perpendicularPoint = unrotated + c;

		// Add to the list
		perpendiculars.append(perpendicularPoint);
	}

	// Now check for coincident points
	for(i=perpendiculars.size()-1;i>0;i--)
	{
		for(j=i-1;j>=0;j--)
		{
			if (perpendiculars[i] == perpendiculars[j])
			{
				perpendiculars.removeAt(i);
				break;
			}
		}
	}

	return(perpendiculars);
}

bool GLEEllipse::hasPerpendiculars()
{
	return(true);
}

void GLEEllipse::draw(QPainter *p)
{
	QPen cpen;
	double angle;
	if (isSet(RadiusX) && isSet(RadiusY) && isSet(Angle))
	{
		angle = getQtAngle(Angle);

		p->save();
		p->translate(getQtPoint(CentrePoint));
		if (angle != 0.0)
			p->rotate(QGLE::radiansToDegrees(angle));
		setSimplePenProperties(cpen);
		p->setPen(cpen);
		if (properties[FillColour].value<QColor>().isValid())
			p->setBrush(QBrush(properties[FillColour].value<QColor>()));
		else
			p->setBrush(Qt::NoBrush);
		p->drawEllipse(ellipseRect());
		p->restore();

	}
}

double GLEEllipse::distanceToPoint(const QPointF& p, QPointF *nearestPoint)
{
	QList<QPointF> perpPoints = getPerpendiculars(p);
	QPointF pp;

	double shortestDistance = 1e6;
	double dist;

	foreach(pp,perpPoints)
	{
		dist = QGLE::distance(pp,p);
		if (dist < shortestDistance)
		{
			shortestDistance = dist;
			if (nearestPoint)
			{
				nearestPoint->setX(pp.x());
				nearestPoint->setY(pp.y());
			}
		}
	}

	return(shortestDistance);
}

// Set a point (start or end in the case of a line)
void GLEEllipse::setPoint(int pointChoice, const QPointF& p, bool update)
{
	switch(pointChoice)
	{
		case BBoxCorner:
			if (isSet(BBoxCornerA))
				setPoint(BBoxCornerB, p);
			else
				setPoint(BBoxCornerA, p);
			break;

		case CentrePoint:
			pointHash[pointChoice] = QGLE::absQtToGLE(p, dpi, pixmap.height());
			updateBBox();
			break;

		case BBoxCornerA:
		case BBoxCornerB:
			pointHash[pointChoice] = QGLE::absQtToGLE(p, dpi, pixmap.height());
			updateFromBBox();
			break;

		case RadiusX:
			pointHash[pointChoice] = QGLE::relQtToGLE(p, dpi);
			updateBBox();
			break;

		case RadiusY:
			pointHash[pointChoice] = QGLE::relQtToGLE(p, dpi);
			updateBBox();
			break;

		case Angle:
			pointHash[pointChoice] = -p;
			break;
	}

	if (update) updateEllipse();
}

void GLEEllipse::updateBBox()
{
	QPointF ba,bb;

	QPointF c;
	double rx, ry;
	double theta;

	if (isSet(RadiusX) && isSet(RadiusY) && isSet(CentrePoint) && isSet(Angle))
	{
		c = getQtPoint(CentrePoint);
		rx = getQtLength(RadiusX);
		ry = getQtLength(RadiusY);
		theta = getQtAngle(Angle);

		// Convert to max lengths
		double x = sqrt(pow(rx*cos(theta),2)+pow(ry*sin(theta),2));
		double y = sqrt(pow(rx*sin(theta),2)+pow(ry*cos(theta),2));

		ba.setX(c.x() + x);
		ba.setY(c.y() - y);
		bb.setX(c.x() - x);
		bb.setY(c.y() + y);

		pointHash[BBoxCornerA] = QGLE::absQtToGLE(ba, dpi, pixmap.height());
		pointHash[BBoxCornerB] = QGLE::absQtToGLE(bb, dpi, pixmap.height());
	}
}

void GLEEllipse::updateFromBBox()
{
	QPointF c;
	double rx,ry;
	QPointF ba, bb;

	if (isSet(BBoxCornerA) && isSet(BBoxCornerB) && isSet(Angle))
	{
		// Get the parameters
		ba = getQtPoint(BBoxCornerA);
		bb = getQtPoint(BBoxCornerB);

		// Locate the centre point
		c.setX((ba.x() + bb.x())/2.0);
		c.setY((ba.y() + bb.y())/2.0);

		// Find the difference between the centre and the BBox
		rx = fabs(bb.x() - c.x());
		ry = fabs(bb.y() - c.y());

		pointHash[CentrePoint] = QGLE::absQtToGLE(c, dpi, pixmap.height());
		pointHash[RadiusX] = QPointF(QGLE::relQtToGLE(rx, dpi),0.0);
		pointHash[RadiusY] = QPointF(QGLE::relQtToGLE(ry, dpi),0.0);
	}
}

QRectF GLEEllipse::ellipseRect()
{

	// This returns the ellipse rectangle, but centred on the origin
	// and not rotated.  The rotation and translation are done with
	// painter->translate() and painter->rotate().

	QRectF rect;
	QPointF c;
	double rx, ry;
	if (isSet(RadiusX) && isSet(RadiusY))
	{
		rx = getQtLength(RadiusX);
		ry = getQtLength(RadiusY);

		rect.setBottomLeft(QPointF(-rx, ry));
		rect.setTopRight(QPointF(rx, -ry));
	}
	return(rect);
}

void GLEEllipse::updateEquationParameters()
{
	// Qt Coordinates!

	// Returns the parameters of:
	// Ax^2 + By^2 + Cxy + Dx + Ey + F = 0

	QPointF c = getQtPoint(CentrePoint);
	double angle = getQtAngle(Angle);
	double rx = getQtLength(RadiusX);
	double ry = getQtLength(RadiusY);

	// Ellipse equation with angle = 0.0 is
	//
	// x^2      y^2
	// ----  +  ----  =  1
	// rx^2     ry^2
	//
	// Translation and rotation with:
	// ( cos(a)   sin(a)  -c.x*cos(a)-c.y*sin(a) ) ( x )
	// ( -sin(a)  cos(a)   c.x*sin(a)-c.y*cos(a) ) ( y )
	// (   0        0                1           ) ( 1 )
	//
	// Solving this gives the result

	double cosine = cos(angle);
	double sine = sin(angle);

	equationParameters['A'] = pow(cosine/rx,2) + pow(sine/ry,2);
	equationParameters['B'] = pow(sine/rx,2) + pow(cosine/ry,2);
	equationParameters['C'] = 2*cosine*sine/pow(rx,2) - 2*cosine*sine/pow(ry,2);
	equationParameters['D'] = (-2*pow(cosine,2)*c.x() - 2*cosine*sine*c.y())/pow(rx,2) +
				  (-2*pow(sine,2)*c.x() + 2*sine*cosine*c.y())/pow(ry,2);
	equationParameters['E'] = (-2*pow(sine,2)*c.y() - 2*sine*cosine*c.x())/pow(rx,2) +
		          (-2*pow(cosine,2)*c.y() + 2*sine*cosine*c.x())/pow(ry,2);
	equationParameters['F'] = (pow(cosine,2)*pow(c.x(),2) + 2*sine*cosine*c.x()*c.y() + pow(sine,2)*pow(c.y(),2))/pow(rx,2) +
			      (pow(cosine,2)*pow(c.y(),2) - 2*sine*cosine*c.x()*c.y() + pow(sine,2)*pow(c.x(),2))/pow(ry,2) - 1;

//	qDebug() << "Calculated equation parameters as: \n"
//		<< "A: " << equationParameters['A'] << "\n"
//		<< "B: " << equationParameters['B'] << "\n"
//		<< "C: " << equationParameters['C'] << "\n"
//		<< "D: " << equationParameters['D'] << "\n"
//		<< "E: " << equationParameters['E'] << "\n"
//		<< "F: " << equationParameters['F'] << "\n"
//		<< "For centre: " << c << "; angle: " << angle << "; radii: " << QSizeF(rx,ry);

	// For debugging
	double A, B, C, D, E;
	A = equationParameters['A'];
	B = equationParameters['B'];
	C = equationParameters['C'];
	D = equationParameters['D'];
	E = equationParameters['E'];

	QPointF p1,p2,p3,p4;
	// Now work out the points where the ellipse touches
	// the old bounding box
	QPointF bba = getQtPoint(BBoxCornerA);
	QPointF bbb = getQtPoint(BBoxCornerB);
	p1.setX(bba.x());
	p1.setY(-(C*p1.x()+E)/(2*B));
	p2.setY(bba.y());
	p2.setX(-(C*p2.y()+D)/(2*A));
	p3.setX(bbb.x());
	p3.setY(-(C*p3.x()+E)/(2*B));
	p4.setY(bbb.y());
	p4.setX(-(C*p4.y()+D)/(2*A));
}


QList<QPointF> GLEEllipse::intersections(double qtm, double qtc, bool vertical)
{
	QPointF one, two;
	// For ellipses, we'll deal with points individually:
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


QList<QPointF> GLEEllipse::intersections(QPointF, QPointF)
{
	QList<QPointF> pointArray;

	return(pointArray);
}

QList<QPointF> GLEEllipse::intersections(QPointF, double)
{
	QList<QPointF> correctIntersections;
	return(correctIntersections);

}

bool GLEEllipse::isInside(QPointF p)
{
	double value =
		equationParameters['A']*p.x()*p.x() +
		equationParameters['B']*p.y()*p.y() +
		equationParameters['C']*p.x()*p.y() +
		equationParameters['D']*p.x() +
		equationParameters['E']*p.y() +
		equationParameters['F'];

	// If on or in, return true
	if (value > 0.0)
		return(false);
	else
		return(true);
}

int GLEEllipse::supportedTransformMode()
{
	return TransformModeFree;
}

void GLEEllipse::linearTransform(const GLELinearEquation& ex, const GLELinearEquation& ey)
{
	if (isSet(BBoxCornerA) && isSet(BBoxCornerB))
	{
		pointHash.clear();

		// Adjust the centre
		QPointF c = getQtPoint(CentrePoint, true);
		double angle = getQtAngle(Angle, true);
		double rx = getQtLength(RadiusX, true);
		double ry = getQtLength(RadiusY, true);

		QPointF p1,p2,p3,p4;
		// Now work out the points corresponding to the extremes
		// of the unrotated ellipse
		p1.setX(c.x() + rx*cos(angle));
		p1.setY(c.y() + rx*sin(angle));
		p2.setX(c.x() - rx*cos(angle));
		p2.setY(c.y() - rx*sin(angle));
		p3.setX(c.x() - ry*sin(angle));
		p3.setY(c.y() + ry*cos(angle));
		p4.setX(c.x() + ry*sin(angle));
		p4.setY(c.y() - ry*cos(angle));

		// Now scale them to their new positions
		p1.setX(ex.apply(p1.x()));
		p1.setY(ey.apply(p1.y()));
		p2.setX(ex.apply(p2.x()));
		p2.setY(ey.apply(p2.y()));
		p3.setX(ex.apply(p3.x()));
		p3.setY(ey.apply(p3.y()));
		p4.setX(ex.apply(p4.x()));
		p4.setY(ey.apply(p4.y()));
		c.setX(ex.apply(c.x()));
		c.setY(ey.apply(c.y()));

		// Now derive the parameters
		rx = QGLE::distance(c, p1);
		ry = QGLE::distance(c, p3);
		angle = QGLE::angleBetweenTwoPoints(c, p1);

		setLength(Angle, angle, false);
		setLength(RadiusX, rx, false);
		setLength(RadiusY, ry, false);
		setPoint(CentrePoint, c, false);
		updateEllipse();
	}
}

void GLEEllipse::moveBy(QPointF offset)
{
	double radiusx = getQtLength(RadiusX);
	double radiusy = getQtLength(RadiusY);
	double angle = getQtAngle(Angle);
	pointHash.clear();
	setLength(RadiusX, radiusx, false);
	setLength(RadiusY, radiusy, false);
	setLength(Angle, angle, false);

	// Moves relative to storedPointHash
	QPointF c = getQtPoint(CentrePoint, true);
	setPoint(CentrePoint, c + offset, false);

	updateBBox();
	updateEllipse();

}

void GLEEllipse::rotateBy(double radians)
{
	double radiusx = getQtLength(RadiusX);
	double radiusy = getQtLength(RadiusY);
	pointHash.clear();
	setPoint(RadiusX, QPointF(radiusx,0.0));
	setPoint(RadiusY, QPointF(radiusy,0.0));

	QPointF oldCentre = getQtPoint(CentrePoint, true);

	// TODO: There's a definite glitch somewhere with the rotation...
	QPointF c = QGLE::rotateAboutPoint(oldCentre, radians, basePoint);
	setPoint(CentrePoint, c);

	// Now update the angle, what's the angle change?
	// If the base point is at the centre, we just need to update
	// the angle, but if not it's less easy
	double oldAngle = getQtAngle(Angle, true);

	// Now work out where a point on the old ellipse is
	QPointF edgePoint;
	edgePoint.setX(oldCentre.x() + getQtLength(RadiusX)*cos(oldAngle));
	edgePoint.setY(oldCentre.y() + getQtLength(RadiusX)*sin(oldAngle));

	// Now rotate it and work out the new angle from there
	edgePoint = QGLE::rotateAboutPoint(edgePoint, radians, basePoint);
	double newAngle = QGLE::angleBetweenTwoPoints(c, edgePoint);

	setPoint(Angle, QPointF(newAngle, 0.0));

	updateBBox();
	updateEllipse();

}
