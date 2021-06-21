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

// Implementation of GLEDrawingObject, the base class
// for all drawing objects in QGLE
#include "drawingobject.h"

GLEDrawingObject::GLEDrawingObject(double resolution, QSize imageSize, GLEDrawingArea *area)
	: GLEComponent(resolution, imageSize, area)
{
	drawingArea = area;
	otrackList.clear();
	selected = false;
	gleObject = NULL;
	initialiseProperties();
}

PropertyDescriptor GLEDrawingObject::getPropertyDescription(int property)
{
	if (propertyDescriptions.contains(property))
		return(propertyDescriptions[property]);
	else
	{
		PropertyDescriptor x;
		return(x);
	}
}

void GLEDrawingObject::initialiseProperties()
{
	QVariantMap validList;

	// This property describes the fill colour for a solid object
	// or for text
	propertyDescriptions[FillColour].description = tr("Fill Colour");
	propertyDescriptions[FillColour].type = QVariant::Color;
	propertyDescriptions[FillColour].defaultValue = QColor();
	propertyDescriptions[FillColour].validValues = QList<QVariant>();

	// The line colour
	propertyDescriptions[LineColour].description = tr("Line Colour");
	propertyDescriptions[LineColour].type = QVariant::Color;
	propertyDescriptions[LineColour].defaultValue = QColor();
	propertyDescriptions[LineColour].validValues = QList<QVariant>();

	// The line width
	propertyDescriptions[LineWidth].description = tr("Line Width");
	propertyDescriptions[LineWidth].type = QVariant::Double;
	propertyDescriptions[LineWidth].defaultValue = 0.05;
	propertyDescriptions[LineWidth].validValues = QList<QVariant>();

	// The line style
	propertyDescriptions[LineStyle].description = tr("Line Style");
	propertyDescriptions[LineStyle].type = QVariant::String;
	propertyDescriptions[LineStyle].defaultValue = QString("0");
	propertyDescriptions[LineStyle].validValues = QList<QVariant>();

	// The string contained in a text object (and possible some other
	// objects as well such as boxes in a flow chart)
	propertyDescriptions[Text].description = tr("Text");
	propertyDescriptions[Text].type = QVariant::String;
	propertyDescriptions[Text].defaultValue = QString("");
	propertyDescriptions[Text].validValues = QList<QVariant>();

	// The font used for a given text string
	propertyDescriptions[FontName].description = tr("Font Name");
	propertyDescriptions[FontName].type = QVariant::Int;
	propertyDescriptions[FontName].defaultValue = 0;
	propertyDescriptions[FontName].validValues = getFontList();

	// The font used for a given text string
	propertyDescriptions[FontStyle].description = tr("Font Style");
	propertyDescriptions[FontStyle].type = QVariant::Int;
	propertyDescriptions[FontStyle].defaultValue = 0;
	propertyDescriptions[FontStyle].validValues = getFontStyles();

	// The font size
	propertyDescriptions[FontSize].description = tr("Font Size");
	propertyDescriptions[FontSize].type = QVariant::Double;
	propertyDescriptions[FontSize].defaultValue = 0.3633;
	propertyDescriptions[FontSize].validValues = QList<QVariant>();

	// The text alignment (e.g. CC)
	propertyDescriptions[Alignment].description = tr("Text Alignment");
	propertyDescriptions[Alignment].type = QVariant::Int;
	propertyDescriptions[Alignment].defaultValue = 0;
	validList.clear();
	validList[tr("Object Centre")] = GLEJustifyCC;
	validList[tr("Top Right")] = GLEJustifyTR;
	validList[tr("Top Left")] = GLEJustifyTL;
	validList[tr("Bottom Right")] = GLEJustifyBR;
	validList[tr("Bottom Left")] = GLEJustifyBL;
	validList[tr("Left Centre")] = GLEJustifyLC;
	validList[tr("Right Centre")] = GLEJustifyRC;
	validList[tr("Top Centre")] = GLEJustifyTC;
	validList[tr("Bottom Centre")] = GLEJustifyBC;
	validList[tr("Left Baseline")] = GLEJustifyLeft;
	validList[tr("Right Baseline")] = GLEJustifyRight;
	validList[tr("Centre Baseline")] = GLEJustifyCenter;
	propertyDescriptions[Alignment].validValues = validList;

	// The reference point name
	propertyDescriptions[RefPointName].description = tr("Reference");
	propertyDescriptions[RefPointName].type = QVariant::String;
	propertyDescriptions[RefPointName].defaultValue = QString("bl");
	propertyDescriptions[RefPointName].validValues = QList<QVariant>();

	// The centre of any object (by bounding box)
	propertyDescriptions[ObjectCentre].description = tr("Object Centre Point");
	propertyDescriptions[ObjectCentre].type = QVariant::PointF;
	propertyDescriptions[ObjectCentre].defaultValue = QPointF(0.0,0.0);
	propertyDescriptions[ObjectCentre].validValues = QList<QVariant>();

	// The centre of a circle or arc
	propertyDescriptions[CircleCentre].description = tr("Circle Centre Point");
	propertyDescriptions[CircleCentre].type = QVariant::PointF;
	propertyDescriptions[CircleCentre].defaultValue = QPointF(0.0,0.0);
	propertyDescriptions[CircleCentre].validValues = QList<QVariant>();

	// The radius for a circle or arc
	propertyDescriptions[Radius].description = tr("Radius");
	propertyDescriptions[Radius].type = QVariant::Double;
	propertyDescriptions[Radius].defaultValue = 0.0;
	propertyDescriptions[Radius].validValues = QList<QVariant>();

	// The major axis radius for an ellipse
	propertyDescriptions[MajorAxisRadius].description = tr("Major Axis Radius");
	propertyDescriptions[MajorAxisRadius].type = QVariant::Double;
	propertyDescriptions[MajorAxisRadius].defaultValue = 0.0;
	propertyDescriptions[MajorAxisRadius].validValues = QList<QVariant>();

	// The minor axis radius for an ellipse
	propertyDescriptions[MinorAxisRadius].description = tr("Minor Axis Radius");
	propertyDescriptions[MinorAxisRadius].type = QVariant::Double;
	propertyDescriptions[MinorAxisRadius].defaultValue = 0.0;
	propertyDescriptions[MinorAxisRadius].validValues = QList<QVariant>();

	// The major axis angle for an ellipse
	propertyDescriptions[MajorAxisAngle].description = tr("Major Axis Angle");
	propertyDescriptions[MajorAxisAngle].type = QVariant::Double;
	propertyDescriptions[MajorAxisAngle].defaultValue = 0.0;
	propertyDescriptions[MajorAxisAngle].validValues = QList<QVariant>();

	// This hasn't been decided on yet, but will probably be an int
	// (enumerated) from a list of PNGs.
	propertyDescriptions[ArrowHeadPresence].description = tr("Arrow Heads");
	propertyDescriptions[ArrowHeadPresence].type = QVariant::Int;
	propertyDescriptions[ArrowHeadPresence].defaultValue = 0;
	validList.clear();
	validList[tr("None")] = 0;
	validList[tr("Start")] = GLEHasArrowStart;
	validList[tr("End")] = GLEHasArrowEnd;
	validList[tr("Both")] = GLEHasArrowStart | GLEHasArrowEnd;
	propertyDescriptions[ArrowHeadPresence].validValues = validList;

	propertyDescriptions[ArrowHeadSize].description = tr("Arrow Head Size");
	propertyDescriptions[ArrowHeadSize].type = QVariant::Double;
	// Check the default size
	propertyDescriptions[ArrowHeadSize].defaultValue = 1.0;
	propertyDescriptions[ArrowHeadSize].validValues = QList<QVariant>();

	// This hasn't been decided on yet, but will probably be an int
	// (enumerated) from a list of PNGs.
	propertyDescriptions[ArrowHeadStyle].description = tr("Arrow Head Style");
	propertyDescriptions[ArrowHeadStyle].type = QVariant::Invalid;
	propertyDescriptions[ArrowHeadStyle].defaultValue = QVariant();
	propertyDescriptions[ArrowHeadStyle].validValues = QList<QVariant>();

	// The start point for a line
	propertyDescriptions[StartPoint].description = tr("Start Point");
	propertyDescriptions[StartPoint].type = QVariant::PointF;
	propertyDescriptions[StartPoint].defaultValue = QPointF(0.0,0.0);
	propertyDescriptions[StartPoint].validValues = QList<QVariant>();

	// The end point for a line
	propertyDescriptions[EndPoint].description = tr("End Point");
	propertyDescriptions[EndPoint].type = QVariant::PointF;
	propertyDescriptions[EndPoint].defaultValue = QPointF(0.0,0.0);
	propertyDescriptions[EndPoint].validValues = QList<QVariant>();

	// Object width
	propertyDescriptions[Width].description = tr("Width");
	propertyDescriptions[Width].type = QVariant::Double;
	propertyDescriptions[Width].defaultValue = 0.0;
	propertyDescriptions[Width].validValues = QList<QVariant>();

	// Object height
	propertyDescriptions[Height].description = tr("Height");
	propertyDescriptions[Height].type = QVariant::Double;
	propertyDescriptions[Height].defaultValue = 0.0;
	propertyDescriptions[Height].validValues = QList<QVariant>();

}

// Set text properties
void GLEDrawingObject::setTextProperties() {
	GLEDrawObject* obj = getGLEObject();
	double fontsize = obj->getProperties()->getRealProperty(GLEDOPropertyFontSize);
	setPropertyNoUpdate(FontSize, fontsize);
	GLEFont* font = obj->getProperties()->getFontProperty(GLEDOPropertyFont);
	GLEFont* base = font->getBaseFont();
	setPropertyNoUpdate(FontName, base->getNumber());
	setPropertyNoUpdate(FontStyle, base->checkStyle(font));
}

void GLEDrawingObject::setColorProperty() {
	QColor col;
	GLEDrawObject* obj = getGLEObject();
	GLEColor* color = obj->getProperties()->getColorProperty(GLEDOPropertyColor);
	col.setRgbF(color->getRed(), color->getGreen(), color->getBlue());
	setPropertyNoUpdate(LineColour, col);
}

void GLEDrawingObject::setLineProperties() {
	QColor col;
	GLEDrawObject* obj = getGLEObject();
	GLEColor* color = obj->getProperties()->getColorProperty(GLEDOPropertyColor);
	col.setRgbF(color->getRed(), color->getGreen(), color->getBlue());
	setPropertyNoUpdate(LineColour, col);
	setPropertyNoUpdate(LineWidth, obj->getProperties()->getRealProperty(GLEDOPropertyLineWidth));
	setPropertyNoUpdate(LineStyle, QGLE::gleToQString(obj->getProperties()->getStringProperty(GLEDOPropertyLineStyle)));
}

void GLEDrawingObject::setShapeProperties() {
	QColor col;
	GLEDrawObject* obj = getGLEObject();
	GLEColor* color = obj->getProperties()->getColorProperty(GLEDOPropertyFillColor);
	if (color->isTransparent()) {
		setPropertyNoUpdate(FillColour, QColor());
	} else {
		col.setRgbF(color->getRed(), color->getGreen(), color->getBlue());
		setPropertyNoUpdate(FillColour, col);
	}
}

void GLEDrawingObject::setSimplePenProperties(QPen& pen) {
	QColor col;
	GLEDrawObject* obj = getGLEObject();
	GLEColor* color = obj->getProperties()->getColorProperty(GLEDOPropertyColor);
	col.setRgbF(color->getRed(), color->getGreen(), color->getBlue());
	pen.setColor(col);
	double lwidth = obj->getProperties()->getRealProperty(GLEDOPropertyLineWidth);
	pen.setWidthF(QGLE::relGLEToQt(lwidth, dpi));
	GLEString* lstyle = obj->getProperties()->getStringProperty(GLEDOPropertyLineStyle);
	if (lstyle->length() == 1 && lstyle->get(0) > (unsigned int)'1') {
		QVector<qreal> dashes;
		const char *defline[] = {"","","12","41","14","92","1282","9229","4114","54","73","7337","6261","2514"};
		const char *myline = defline[lstyle->get(0)-'0'];
		int len = strlen(myline);
		for (int i = 0; i < len; i++) {
			double value = (myline[i]-'0');
			dashes.append((qreal)QGLE::relGLEToQt(value*0.04, dpi));
		}
		pen.setDashPattern(dashes);
	} else if (lstyle->length() > 1) {
		QVector<qreal> dashes;
		for (unsigned int i = 0; i < lstyle->length(); i++) {
			double value = (lstyle->get(i)-'0');
			dashes.append((qreal)QGLE::relGLEToQt(value*0.04, dpi));
		}
		pen.setDashPattern(dashes);
	}
}

void GLEDrawingObject::setPenProperties(QPen& pen) {
	setSimplePenProperties(pen);
	GLEDrawObject* obj = getGLEObject();
	int cap = obj->getIntProperty(GLEDOPropertyLineCap);
	switch (cap) {
		case GLELineCapButt:
			pen.setCapStyle(Qt::FlatCap);
			break;
		case GLELineCapRound:
			pen.setCapStyle(Qt::RoundCap);
			break;
		case GLELineCapSquare:
			pen.setCapStyle(Qt::SquareCap);
			break;
	}
}

QList<QVariant> GLEDrawingObject::getFontList()
{
	QList<QVariant> flv;
	GLEInterface* iface = getGLEInterface();
	int nbfonts = iface->getNumberOfFonts();
	for (int i = 0; i < nbfonts; i++) {
		GLEFont* font = iface->getFont(i);
		flv.push_back(QString::fromUtf8(font->getFullNameC()));
		// cout << "FONT: " << font->getFullNameC() << " -> " << i << endl;
	}
	return(flv);
}

QList<QVariant> GLEDrawingObject::getFontStyles()
{
	QList<QVariant> flv;
	flv.push_back(tr("Roman"));
	flv.push_back(tr("Bold"));
	flv.push_back(tr("Italic"));
	flv.push_back(tr("Bold Italic"));
	return flv;
}

void GLEDrawingObject::imageRendered(QImage)
{
	// This should be overridden by any children that use
	// rendered objects
}

//! Function to set a given property to a given value
void GLEDrawingObject::setProperty(int propertyIndex, QVariant value)
{
	setPropertyNoUpdate(propertyIndex, value);
	emit propertyChanged(propertyIndex);
}

//! Function to set a given property to a given value
void GLEDrawingObject::setPropertyNoUpdate(int propertyIndex, QVariant value) {
	if (propertyIndex >= GLEDrawingObject::PropertyUser && getGLEObject() != NULL)
	{
		// This case is for arguments of user defined objects
		int propid = propertyIndex - GLEDrawingObject::PropertyUser;
		GLEDrawObject* gleobj = getGLEObject();
		GLEPropertyStoreModel* model = gleobj->getProperties()->getModel();
		GLEProperty* gleprop = model->getProperty(propid);
		if (gleprop->getType() == GLEPropertyTypeString)
		{
			GLEString* str = gleobj->getProperties()->getStringProperty(gleprop);
			QGLE::qtToGLEString(value.toString(), str);
		}
		else if (gleprop->getType() == GLEPropertyTypeReal)
		{
			gleobj->getProperties()->setRealProperty(gleprop, value.toDouble());
		}
	}
	else
	{
		if (properties.contains(propertyIndex))
			properties[propertyIndex] = value;
	}
}

//! Function to get the value of a property
QVariant GLEDrawingObject::getProperty(int propertyIndex)
{
	if (properties.contains(propertyIndex))
		return(properties[propertyIndex]);
	else
		return QVariant();
}

// The selected property tells the object whether it has been
// selected by the pointer tool
void GLEDrawingObject::setSelected(bool sel)
{
	selected = sel;
}

bool GLEDrawingObject::isAmove()
{
	return(amove);
}

QList<SnapLine *> GLEDrawingObject::getOTracks()
{
	return(otrackList);
}

void GLEDrawingObject::clearOTracks()
{
	SnapLine *snap;
	foreach(snap, otrackList)
	{
		delete snap;
	}
	otrackList.clear();
}

QPair<double, QPointF> GLEDrawingObject::updateOTracks(QPointF cursorPosition)
{
	QPair<double, QPointF> nearestPoint;
	nearestPoint.first = 1e6;
	nearestPoint.second = QPointF(0.0,0.0);
	for (int i=otrackList.size()-1;i>=0;i--)
	{
		double dist = otrackList[i]->distanceToPoint(cursorPosition, &nearestPoint.second);
		double objectDistance = distanceToPoint(cursorPosition);
		if ((dist > MAX_SNAP_LINE_DISTANCE) && (objectDistance > MAX_SNAP_LINE_DISTANCE))
		{
			if (QGLE::distance(cursorPosition,
						otrackList[i]->getQtPoint(SnapLine::StartPoint)) > 3*MAX_SNAP_LINE_DISTANCE)
				otrackList[i]->deactivate();
			else
			{
				delete otrackList[i];
				otrackList.removeAt(i);
			}
		}
		else
		{
			otrackList[i]->activate();
			nearestPoint.first = dist;
		}
	}
	return(nearestPoint);
}

void GLEDrawingObject::distanceToPointUpdate(double x, double y, const QPointF& p, double* minDist, QPointF *nearestPoint)
{
	double crDist = QGLE::distance(x, y, p);
	if (crDist < *minDist) {
		*minDist = crDist;
		if (nearestPoint != NULL) {
			*nearestPoint = QPointF(x, y);
		}
	}
}

//! Set the reference point of, e.g., text objects or GLE object blocks (tl,bl,tc,bc,tr,br,...)
bool GLEDrawingObject::setReferencePoint(const QPointF&)
{
	return false;
}

bool GLEDrawingObject::findScaleOrigin(const QPointF&, QPointF*, int)
{
	return false;
}

int GLEDrawingObject::supportedTransformMode() {
	return TransformModeNone;
}

void GLEDrawingObject::linearTransformPt(int pt, const GLELinearEquation& ex, const GLELinearEquation& ey, bool update)
{
	QPointF p = getQtPoint(pt, true);
	p.setX(ex.apply(p.x()));
	p.setY(ey.apply(p.y()));
	setPoint(pt, p, update);
}

void GLEDrawingObject::linearTransform(const GLELinearEquation&, const GLELinearEquation&)
{
}

void GLEDrawingObject::linearTransformDone()
{
}

bool GLEDrawingObject::isSelected()
{
	return(selected);
}

void GLEDrawingObject::drawOSnaps(QPainter *p)
{
	QPair<QPointF,int> osnap;
	foreach(osnap, osnapHandles)
	{
		//QGLE::drawBox(p,osnap.first,OSNAP_BOX_SIZE, pen);
		QGLE::drawOSnap(p,osnap);
	}
	foreach(osnap, relativeOSnaps)
	{
		//QGLE::drawBox(p,osnap.first,OSNAP_BOX_SIZE, pen);
		QGLE::drawOSnap(p,osnap);
	}
}

double GLEDrawingObject::nearestPriorityOSnap(const QPointF&, QPointF *)
{
	return 1e6;
}

double GLEDrawingObject::nearestOSnap(const QPointF& p, QPointF *osnap)
{
	QPair<QPointF,int> o;
	double dist;
	double shortest_distance = 1e6;
	*osnap = QPointF(0.0,0.0);
	foreach(o, osnapHandles)
	{
		dist = QGLE::distance(o.first,p);
		if (dist < shortest_distance)
		{
			shortest_distance = dist;
			*osnap = o.first;
		}
	}
	foreach(o, relativeOSnaps)
	{
		dist = QGLE::distance(o.first,p);
		if (dist < shortest_distance)
		{
			shortest_distance = dist;
			*osnap = o.first;
		}
	}
	return(shortest_distance);
}

void GLEDrawingObject::clearRelativeOSnaps()
{
	relativeOSnaps.clear();
}

QList< QPair<QPointF,int> > GLEDrawingObject::getRelativeOSnaps()
{
	return(relativeOSnaps);
}
