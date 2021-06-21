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

#include <QtDebug>
#include "text.h"

// The constructor for the line object
GLEText::GLEText(double resolution, QSize imageSize, GLEDrawingArea *area) :
	GLEDrawingObject(resolution, imageSize, area)
{
	lastdpi = -1;
	// Make sure the line is updated if a point changes or
	// the image size changes
	connect(this, SIGNAL(pointChanged()),
			this, SLOT(updateText()));
	connect(this, SIGNAL(imageChanged()),
			this, SLOT(resChanged()));
	connect(this, SIGNAL(propertyChanged(int)),
			this, SLOT(updateFromProperty(int)));

	amove = false;
	drawPoint = QPointF(0.0,0.0);

	// More to be added
	validProperties
		<< Text
		<< FontName
		<< FontStyle
		<< FontSize
		<< Alignment
		<< LineColour;
	properties[Text] = propertyDescriptions[Text].defaultValue;
	properties[FontName] = propertyDescriptions[FontName].defaultValue;
	properties[FontStyle] = propertyDescriptions[FontStyle].defaultValue;
	properties[FontSize] = propertyDescriptions[FontSize].defaultValue;
	properties[Alignment] = propertyDescriptions[Alignment].defaultValue;
	properties[LineColour] = propertyDescriptions[LineColour].defaultValue;
}

void GLEText::resChanged()
{
	if (dpi != lastdpi)
	{
		renderPostscript();
		lastdpi = dpi;
	}
	else
	{
		updateText();
	}
}

void GLEText::createOTracks()
{
}

void GLEText::updateFromPropertyNoDirty(int property)
{
	GLETextDO* text = (GLETextDO*)getGLEObject();
	GLEPropertyStore* text_settings = text->getProperties();
	switch(property)
	{
		case Text:
		case LineColour:
		case FontName:
		case FontStyle:
		case FontSize: {
			// Font is
			// fontList[properties[FontName].value<QString>]
			// (default "rm")
			GLEInterface* iface = getGLEInterface();
			QColor color = getProperty(LineColour).value<QColor>();
			GLEColor* gle_color = text_settings->getColorProperty(GLEDOPropertyColor);
			gle_color->setRGB255(color.red(), color.green(), color.blue());
			GLEFont* font = iface->getFont(getProperty(FontName).toInt());
			font = font->getStyle((GLEFontStyle)getProperty(FontStyle).toInt());
			text_settings->setFontProperty(GLEDOPropertyFont, font);
			text_settings->setRealProperty(GLEDOPropertyFontSize, getProperty(FontSize).toDouble());
			text->setText(getProperty(Text).toString().toUtf8());
			text->render(iface);
			// Get the bounding box (important!)
			text->getPSBoundingBox(&rect);
			// Now we need to pass this to the ghostscript renderer...
			renderPostscript();
			break;
		}
		case Alignment: {
			text_settings->setIntProperty(GLEDOPropertyJustify, getProperty(GLEText::Alignment).toInt());
			updateText();
			break;
		}
		default:
			updateText();
	}
}

void GLEText::renderPostscript()
{
	QImage img;
	GLETextDO* text = (GLETextDO*)getGLEObject();
	drawingArea->renderPostscript(text->getPostScriptCode(), rect, dpi, &img);
	renderedText = QPixmap::fromImage(img);
	updateText();
}

void GLEText::updateFromProperty(int property)
{
	if (property == Text) {
		GLETextDO* text = (GLETextDO*)getGLEObject();
		text->setModified(true);
	}
	updateFromPropertyNoDirty(property);
	((GLEDrawingArea*)parent())->setDirtyAndSave();
}

// Update the painter path
void GLEText::updateText()
{
	// Only do this if both the start and end have been set
	GLETextDO* text = (GLETextDO*)getGLEObject();
	if (isSet(ReferencePoint) && text != NULL)
	{
		// This is guaranteed to have been created in the constructor
		// of GLEDrawingObject
		delete(paintPath);
		// Create a new path starting at lineStart
		paintPath = new QPainterPath();
		QPointF qtText = getQtPoint(ReferencePoint);
		QPointF qtRelative;
		QPointF refPt;

		// Update GLE object
		text->setPosition(QGLE::QPointFToGLEPoint(getGLEPoint(ReferencePoint)));
		text->setText(getProperty(Text).toString().toUtf8());

		int alignProp = getProperty(Alignment).toInt();

		// Now work out the point at which the object is drawn:
		switch(alignProp)
		{
			case GLEJustifyTL:
				refPt = QPointF(0.0, -rect.getYMax());
				break;

			case GLEJustifyTR:
				refPt = QPointF(rect.getXMax(), -rect.getYMax());
				break;

			case GLEJustifyBL:
				refPt = QPointF(0.0, 0.0);
				break;

			case GLEJustifyBR:
				refPt = QPointF(rect.getXMax(), 0.0);
				break;

			case GLEJustifyLC:
				refPt = QPointF(0.0, -rect.getYMax()/2.0);
				break;

			case GLEJustifyRC:
				refPt = QPointF(rect.getXMax(), -rect.getYMax()/2.0);
				break;

			case GLEJustifyTC:
				refPt = QPointF(rect.getXMax()/2.0, -rect.getYMax());
				break;

			case GLEJustifyBC:
				refPt = QPointF(rect.getXMax()/2.0, 0.0);
				break;

			case GLEJustifyLeft:
				refPt = QPointF(0.0, -text->getBaseLine());
				break;

			case GLEJustifyRight:
				refPt = QPointF(rect.getXMax(), -text->getBaseLine());
				break;

			case GLEJustifyCenter:
				refPt = QPointF(rect.getXMax()/2.0, -text->getBaseLine());
				break;

			case GLEJustifyCC:
			default: // Case 0: CC
				// Centre point is at rect.getXMax()/2, rect.getYMax()/2
				// in GLE coordinates.  Therefore, the point at which to
				// draw the pixmap is the reference point minus the
				// X & Y values in Qt coordinates...
				refPt = QPointF(rect.getXMax()/2.0,
						-rect.getYMax()/2.0);
		}
		qtRelative = QGLE::relGLEToQt(refPt, dpi);
		QPointF offset(-GS_OFFSET*dpi,GS_OFFSET*dpi-renderedText.height());
		drawPoint = qtText - qtRelative + offset;

		// This needs to be changed to use the bounding box
		// of the string
		QSizeF qtSize;
		qtSize.setWidth(QGLE::relGLEToQt(rect.getXMax(), dpi));
		qtSize.setHeight(QGLE::relGLEToQt(rect.getYMax(), dpi));
		// The space from the top of the image is the following:
		double yoffs = renderedText.height()-rect.getYMax()*dpi/CM_PER_INCH-GS_OFFSET*dpi;
		paintPath->addRect(QRectF(drawPoint.x()+GS_OFFSET*dpi,
					drawPoint.y()+yoffs,
					qtSize.width(),
					qtSize.height()));

		osnapHandles.clear();
		osnapHandles.append(QPair<QPointF,int>(qtText,QGLE::CentreSnap));
	}
}

void GLEText::addRelativeOSnaps(QPointF p)
{
	UNUSED_ARG(p);
}

void GLEText::draw(QPainter *p)
{
	// This needs to be changed to plot an image object
	p->drawPixmap(drawPoint, renderedText);
	// QGLE::drawCross(p, getQtPoint(ReferencePoint), AMOVE_LENGTH, pen());
}

QList<QPointF> GLEText::getTangents(QPointF p)
{
	QList<QPointF> empty;
	UNUSED_ARG(p);
	return(empty);
}

bool GLEText::hasTangents()
{
	return(false);
}

QList<QPointF> GLEText::getPerpendiculars(QPointF p)
{
	QList<QPointF> empty;
	UNUSED_ARG(p);
	return(empty);
}

bool GLEText::hasPerpendiculars()
{
	return(false);
}

// Set a point (start or end in the case of a line)
void GLEText::setPoint(int pointChoice, const QPointF& p, bool update)
{
	switch(pointChoice)
	{
		case ReferencePoint:
			pointHash[pointChoice] = QGLE::absQtToGLE(p, dpi, pixmap.height());
			break;
	}
	if (update) updateText();
}

QList<QPointF> GLEText::intersections(double qtm, double qtc, bool vertical)
{
	UNUSED_ARG(qtm);
	UNUSED_ARG(qtc);
	UNUSED_ARG(vertical);
	return QList<QPointF>();
}

QList<QPointF> GLEText::intersections(QPointF qtp1, QPointF qtp2)
{
	UNUSED_ARG(qtp1);
	UNUSED_ARG(qtp2);
	return QList<QPointF>();
}

QList<QPointF> GLEText::intersections(QPointF qtp1, double angle)
{
	UNUSED_ARG(qtp1);
	UNUSED_ARG(angle);
	return QList<QPointF>();
}

bool GLEText::isInside(QPointF p)
{
	return(paintPath->contains(p));
}

double GLEText::nearestOSnap(const QPointF& p, QPointF *osnap)
{
	return distanceToPoint(p, osnap);
}

double GLEText::distanceToPoint(const QPointF& p, QPointF *nearestPoint)
{
	GLERectangle box, bbox;
	double minDist = 1e300;
	if (nearestPoint != NULL) *nearestPoint = p;
	box.copy(&rect);
	box.translate(CM_PER_INCH/72, CM_PER_INCH/72);
	box.scale(dpi/CM_PER_INCH);
	box.subtractYFrom(renderedText.height());
	box.translate(drawPoint.x(), drawPoint.y());
	box.normalize();
	GLETextDO* text = (GLETextDO*)getGLEObject();
	double baseline = text->getBaseLine()*dpi/CM_PER_INCH;
	bbox.copy(&box);
	bbox.grow(MAX_SELECT_DISTANCE);
	if (bbox.contains(p.x(), p.y())) {
		distanceToPointUpdate(box.getXMin(), box.getYMin(), p, &minDist, nearestPoint);
		distanceToPointUpdate(box.getXMax(), box.getYMin(), p, &minDist, nearestPoint);
		distanceToPointUpdate(box.getXMin(), box.getYMax(), p, &minDist, nearestPoint);
		distanceToPointUpdate(box.getXMax(), box.getYMax(), p, &minDist, nearestPoint);
		distanceToPointUpdate(box.getXMin(), box.getYMid(), p, &minDist, nearestPoint);
		distanceToPointUpdate(box.getXMax(), box.getYMid(), p, &minDist, nearestPoint);
		distanceToPointUpdate(box.getXMid(), box.getYMin(), p, &minDist, nearestPoint);
		distanceToPointUpdate(box.getXMid(), box.getYMax(), p, &minDist, nearestPoint);
		distanceToPointUpdate(box.getXMid(), box.getYMid(), p, &minDist, nearestPoint);
		distanceToPointUpdate(box.getXMin(), box.getYMax()-baseline, p, &minDist, nearestPoint);
		distanceToPointUpdate(box.getXMax(), box.getYMax()-baseline, p, &minDist, nearestPoint);
		distanceToPointUpdate(box.getXMid(), box.getYMax()-baseline, p, &minDist, nearestPoint);
		return 0.0;
	}
	return minDist;
}

//! Set the reference point (tl,bl,tc,bc,tr,br,...)
bool GLEText::trySetReferencePoint(const QPointF& pt, double x, double y, GLEJustify just, double* minDist)
{
	double dist = QGLE::distance(x, y, pt);
	if (dist < MAX_SELECT_DISTANCE && dist < *minDist)
	{
		*minDist = dist;
		setProperty(Alignment, just);
		GLETextDO* text = (GLETextDO*)getGLEObject();
		text->setIntProperty(GLEDOPropertyJustify, just);
		setPoint(ReferencePoint, pt);
		updateText();
		return true;
	}
	return false;
}

//! Set the reference point (tl,bl,tc,bc,tr,br,...)
bool GLEText::setReferencePoint(const QPointF& pt)
{
	GLERectangle box;
	bool modified = false;
	double minDist = 1e300;
	box.copy(&rect);
	box.translate(CM_PER_INCH/72, CM_PER_INCH/72);
	box.scale(dpi/CM_PER_INCH);
	box.subtractYFrom(renderedText.height());
	box.translate(drawPoint.x(), drawPoint.y());
	box.normalize();
	GLETextDO* text = (GLETextDO*)getGLEObject();
	double baseline = text->getBaseLine()*dpi/CM_PER_INCH;
	modified |= trySetReferencePoint(pt, box.getXMin(), box.getYMin(), GLEJustifyTL, &minDist);
	modified |= trySetReferencePoint(pt, box.getXMax(), box.getYMin(), GLEJustifyTR, &minDist);
	modified |= trySetReferencePoint(pt, box.getXMin(), box.getYMax(), GLEJustifyBL, &minDist);
	modified |= trySetReferencePoint(pt, box.getXMax(), box.getYMax(), GLEJustifyBR, &minDist);
	modified |= trySetReferencePoint(pt, box.getXMin(), box.getYMid(), GLEJustifyLC, &minDist);
	modified |= trySetReferencePoint(pt, box.getXMax(), box.getYMid(), GLEJustifyRC, &minDist);
	modified |= trySetReferencePoint(pt, box.getXMid(), box.getYMin(), GLEJustifyTC, &minDist);
	modified |= trySetReferencePoint(pt, box.getXMid(), box.getYMax(), GLEJustifyBC, &minDist);
	modified |= trySetReferencePoint(pt, box.getXMid(), box.getYMid(), GLEJustifyCC, &minDist);
	modified |= trySetReferencePoint(pt, box.getXMin(), box.getYMax()-baseline, GLEJustifyLeft, &minDist);
	modified |= trySetReferencePoint(pt, box.getXMax(), box.getYMax()-baseline, GLEJustifyRight, &minDist);
	modified |= trySetReferencePoint(pt, box.getXMid(), box.getYMax()-baseline, GLEJustifyCenter, &minDist);
	return modified;
}

void GLEText::moveBy(QPointF offset)
{
	pointHash.clear();
	// Moves relative to storedPointHash
	QPointF start = getQtPoint(ReferencePoint, true);
	setPoint(ReferencePoint, start + offset);
}

void GLEText::rotateBy(double radians)
{
	// TODO: Also rotate the text.
	pointHash.clear();
	// Moves relative to storedPointHash
	QPointF start = getQtPoint(ReferencePoint, true);
	setPoint(ReferencePoint, QGLE::rotateAboutPoint(start, radians, basePoint));
}
