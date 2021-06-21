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

#include "mainwindow.h"
#include "objectblocks.h"

GLEObjectBlocksList::GLEObjectBlocksList(QWidget *parent, GLEMainWindow* main, GLEDrawingArea* area, int size) : QTreeView(parent) {
	defaultSize = size;
	mainWin = main;
	drawingArea = area;
	model = new QStandardItemModel();
	setModel(model);
	header()->hide(); // = setHeaderHidden(true) in Qt 4.4
	connect(this, SIGNAL(clicked(const QModelIndex&)), this, SLOT(selectEntry(const QModelIndex&)));
	//setSizeIncrement(1, 1);
	//setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

void GLEObjectBlocksList::updateList() {
	model->clear();
	consList.clear();
	QStandardItem *parentItem = model->invisibleRootItem();
	GLEScript* script = mainWin->getGLEScript();
	script->updateObjectDOConstructors();
	GLEGlobalSource* source = script->getSource();
	updateFile(parentItem, source->getMainFile());
	for (int i = 0; i < source->getNbFiles(); i++) {
		updateFile(parentItem, source->getFile(i));
	}
}

void GLEObjectBlocksList::updateFile(QStandardItem *parent, GLESourceFile* file) {
	QStandardItem* item = new QStandardItem(QString::fromUtf8(file->getLocation()->getName().c_str()));
	parent->appendRow(item);
	for (int i = 0; i < file->getNbObjectDOConstructors(); i++) {
		GLEObjectDOConstructor* cons = file->getObjectDOConstructor(i);
		QStandardItem* child = new QStandardItem(QString::fromUtf8(cons->getName().c_str()).toLower());
		child->setData(qVariantFromValue((void*)cons));
		item->appendRow(child);
		consList.add(cons);
	}
}

void GLEObjectBlocksList::selectEntry(const QModelIndex& mi) {
	QStandardItem* item = model->itemFromIndex(mi);
	GLEObjectDOConstructor* cons = (GLEObjectDOConstructor*)item->data().value<void*>();
	if (cons == NULL) return;
	mainWin->clearConsoleWindow();
	GLEObjectBlock* block = drawingArea->createNewObjectBlock(cons);
	if (block == NULL) return;
	block->renderPostscriptNoUpdate();
	mainWin->shouldAutoShowConsole();
	// Center object on screen
	QSize scrollSize = mainWin->getScrollAreaSize();
	QSize drawSize = drawingArea->size();
	QPoint drawPos = drawingArea->pos();
	scrollSize.setWidth(min(scrollSize.width(), drawSize.width()));
	scrollSize.setHeight(min(scrollSize.height(), drawSize.height()));
	QPoint pos = QPoint(scrollSize.width()/2-drawPos.x(), scrollSize.height()/2-drawPos.y());
	block->setPoint(GLEObjectBlock::ReferencePoint, pos, false);
	GLEObjectDO* obj = (GLEObjectDO*)block->getGLEObject();
	GLEPropertyStore* props = obj->getProperties();
	if (props->getModel()->isSupportScale()) {
		GLEArrayImpl* arr = props->getArray();
		block->setLength(GLEObjectBlock::ObjectWidth, arr->getDouble(0), false);
		block->setLength(GLEObjectBlock::ObjectHeight, arr->getDouble(1), false);
	}
	block->updateFromPropertyNoDirty(GLEObjectBlock::RefPointName);
	// Select object
	mainWin->selectPointerTool();
	drawingArea->selectObject(block);
	drawingArea->update();
}

// The constructor for the line object
GLEObjectBlock::GLEObjectBlock(double resolution, QSize imageSize, GLEDrawingArea* area)
	: GLEDrawingObject(resolution, imageSize, area)
{
	lastdpi = -1;
	amove = false;
	connect(this, SIGNAL(pointChanged()), this, SLOT(updateObject()));
	connect(this, SIGNAL(imageChanged()), this, SLOT(resChanged()));
	connect(this, SIGNAL(propertyChanged(int)), this, SLOT(updateFromProperty(int)));
	validProperties
		<< FillColour
		<< LineColour
		<< LineWidth
		<< RefPointName
		<< FontName
		<< FontStyle
		<< FontSize;
	properties[FillColour] = propertyDescriptions[FillColour].defaultValue;
	properties[LineColour] = propertyDescriptions[LineColour].defaultValue;
	properties[LineWidth] = propertyDescriptions[LineWidth].defaultValue;
	properties[RefPointName] = propertyDescriptions[RefPointName].defaultValue;
	properties[FontName] = propertyDescriptions[FontName].defaultValue;
	properties[FontStyle] = propertyDescriptions[FontStyle].defaultValue;
	properties[FontSize] = propertyDescriptions[FontSize].defaultValue;
	scaleOffs = QPointF(1.0, 1.0);
}

void GLEObjectBlock::resChanged()
{
	if (dpi != lastdpi)
	{
		renderPostscript();
		lastdpi = dpi;
	}
	else
	{
		updateObject();
	}
}

void GLEObjectBlock::renderPostscript()
{
	renderPostscriptNoUpdate();
	scaleOffs = QPointF(1.0, 1.0);
	updateObject();
}

void GLEObjectBlock::renderPostscriptNoUpdate()
{
	QImage img;
	GLERectangle rect;
	GLEObjectDO* obj = (GLEObjectDO*)getGLEObject();
	obj->getPSBoundingBox(&rect);
	drawingArea->renderPostscript(obj->getPostScriptCode(), rect, dpi, &img);
	image = QPixmap::fromImage(img);
	origImage = image;
}

void GLEObjectBlock::createOTracks()
{
}

void GLEObjectBlock::computeInitialPosition()
{
	GLEObjectDO* obj = (GLEObjectDO*)getGLEObject();
	obj->computeReferencePoint(&referenceOffset);
	GLEPoint& pos = obj->getPosition();
	QPointF refPt(pos.getX()-referenceOffset.getX(), pos.getY()-referenceOffset.getY());
	pointHash[ReferencePoint] = refPt;
}

void GLEObjectBlock::updateFromPropertyNoDirty(int property)
{
	GLEObjectDO* obj = (GLEObjectDO*)getGLEObject();
	GLEPropertyStore* obj_prop = obj->getProperties();

	if (property == RefPointName)
	{
		GLERC<GLEString> refPtStrGLE(new GLEString());
		QString refPtStr = getProperty(RefPointName).toString();
		QGLE::qtToGLEString(refPtStr, refPtStrGLE.get());
		obj->setRefPointString(refPtStrGLE.get());
		obj->computeReferencePoint(&referenceOffset);
	}

	if (property == LineWidth || property == LineColour || property == FillColour || property == FontName ||
		property == FontStyle || property == FontSize || property >= PropertyUser)
	{
		obj_prop->setRealProperty(GLEDOPropertyLineWidth, getProperty(LineWidth).toDouble());
		QColor color = getProperty(LineColour).value<QColor>();
		GLEColor* gle_color = obj->getProperties()->getColorProperty(GLEDOPropertyColor);
		gle_color->setRGB255(color.red(), color.green(), color.blue());
		GLEColor* gle_fill = obj->getProperties()->getColorProperty(GLEDOPropertyFillColor);
		if (properties[FillColour].value<QColor>().isValid()) {
			QColor fill = getProperty(FillColour).value<QColor>();
			gle_fill->setRGB255(fill.red(), fill.green(), fill.blue());
		} else {
			gle_fill->setTransparent(true);
		}
		// Font is
		// fontList[properties[FontName].value<QString>]
		// (default "rm")
		GLEInterface* iface = obj->getConstructor()->getScript()->getGLEInterface();
		GLEFont* font = iface->getFont(getProperty(FontName).toInt());
		font = font->getStyle((GLEFontStyle)getProperty(FontStyle).toInt());
		obj_prop->setFontProperty(GLEDOPropertyFont, font);
		obj_prop->setRealProperty(GLEDOPropertyFontSize, getProperty(FontSize).toDouble());
		// Copy properties to lengths
		if (obj_prop->getModel()->isSupportScale()) {
			GLEArrayImpl* arr = obj_prop->getArray();
			setLength(GLEObjectBlock::ObjectWidth, arr->getDouble(0), false);
			setLength(GLEObjectBlock::ObjectHeight, arr->getDouble(1), false);
		}
		// Render the object
		obj->render();
		renderPostscript();
		// updateObject() is called after rendering is complete.
	}
	else
	{
		updateObject();
	}
}

void GLEObjectBlock::updateFromProperty(int property)
{
	updateFromPropertyNoDirty(property);
	((GLEDrawingArea*)parent())->setDirtyAndSave();
}

// Update the painter path
void GLEObjectBlock::updateObject()
{
	updateObjectScale(1.0, 1.0);
}

/**
 * referenceOffset: offset of reference point (justify setting) in cm measured from bottom-left corner
 * referencePoint:  reference point on screen
 * scriptPosition:  referencePoint + referenceOffset
 * drawpoint:       where to draw pixmap on screen
 *
 * screen:
 *
 * (0,0).............(maxx,0)
 * .                 .
 * .                 .
 * (0,maxy)..........(maxx,maxy)
 */

// Update the painter path
void GLEObjectBlock::updateObjectScale(double sx, double sy)
{
	delete(paintPath);
	paintPath = new QPainterPath();
	QPointF gleRefPt = getGLEPoint(ReferencePoint);
	GLEObjectDO* obj = (GLEObjectDO*)getGLEObject();
	obj->getPosition().setXY(gleRefPt.x()+referenceOffset.getX(), gleRefPt.y()+referenceOffset.getY());
	QPointF qtRefPt = QGLE::absGLEToQt(gleRefPt, dpi, pixmap.height());
	QPointF offset(-GS_OFFSET*dpi*sx, GS_OFFSET*dpi*sy-origImage.height()*sy);
	drawPoint = qtRefPt + offset;
	QSizeF qtSize;
	GLERectangle rect;
	getGLEObject()->getPSBoundingBox(&rect);
	qtSize.setWidth(QGLE::relGLEToQt(rect.getWidth()*sx, dpi));
	qtSize.setHeight(QGLE::relGLEToQt(rect.getHeight()*sy, dpi));
	// The space from the top of the image is the following:
	double yoffs = origImage.height()*sy-rect.getHeight()*sy*dpi/CM_PER_INCH-GS_OFFSET*dpi*sy;
	paintPath->addRect(QRectF(drawPoint.x()+GS_OFFSET*dpi*sx,
				drawPoint.y()+yoffs,
				qtSize.width(),
				qtSize.height()));
	osnapHandles.clear();
}

void GLEObjectBlock::addRelativeOSnaps(QPointF p)
{
	UNUSED_ARG(p);
}

void GLEObjectBlock::draw(QPainter *p)
{
	// This needs to be changed to plot an image object
	p->drawPixmap(drawPoint, image);
	// QGLE::drawCross(p, getQtPoint(ReferencePoint), AMOVE_LENGTH, pen());
}

QList<QPointF> GLEObjectBlock::getTangents(QPointF p)
{
	QList<QPointF> empty;
	UNUSED_ARG(p);
	return(empty);
}

bool GLEObjectBlock::hasTangents()
{
	return(false);
}

QList<QPointF> GLEObjectBlock::getPerpendiculars(QPointF p)
{
	QList<QPointF> empty;
	UNUSED_ARG(p);
	return(empty);
}

bool GLEObjectBlock::hasPerpendiculars()
{
	return(false);
}

double GLEObjectBlock::nearestOSnap(const QPointF& p, QPointF *osnap)
{
	return distanceToPoint(p, osnap);
}

double GLEObjectBlock::distanceToPoint(const QPointF& p, QPointF *nearestPoint)
{
	GLERectangle box;
	double minDist = 1e300;
	if (nearestPoint != NULL) *nearestPoint = p;
	GLEObjectDO* obj = (GLEObjectDO*)getGLEObject();
	GLEObjectRepresention* objrep = obj->getObjectRepresentation();
	box.copy(objrep->getRectangle());
	GLEPoint orig(box.getXMin(), box.getYMin());
	box.translate(1.0-orig.getX(), 1.0-orig.getY());
	box.scale(dpi/PS_POINTS_PER_INCH);
	box.subtractYFrom(image.height());
	box.translate(drawPoint.x(), drawPoint.y());
	box.normalize();
	box.grow(MAX_SELECT_DISTANCE);
	if (box.contains(p.x(), p.y())) {
		distanceToPointRec(objrep, orig, p, &minDist, nearestPoint);
		return 0.0;
	}
	return minDist;
}

void GLEObjectBlock::distanceToPointRec(GLEObjectRepresention* obj, const GLEPoint& orig, const QPointF& p, double* minDist, QPointF *nearestPoint)
{
	GLERectangle box;
	box.copy(obj->getRectangle());
	box.translate(1.0-orig.getX(), 1.0-orig.getY());
	box.scale(dpi/PS_POINTS_PER_INCH);
	box.subtractYFrom(image.height());
	box.translate(drawPoint.x(), drawPoint.y());
	distanceToPointUpdate(box.getXMin(), box.getYMin(), p, minDist, nearestPoint);
	distanceToPointUpdate(box.getXMin(), box.getYMid(), p, minDist, nearestPoint);
	distanceToPointUpdate(box.getXMin(), box.getYMax(), p, minDist, nearestPoint);
	distanceToPointUpdate(box.getXMid(), box.getYMin(), p, minDist, nearestPoint);
	distanceToPointUpdate(box.getXMid(), box.getYMid(), p, minDist, nearestPoint);
	distanceToPointUpdate(box.getXMid(), box.getYMax(), p, minDist, nearestPoint);
	distanceToPointUpdate(box.getXMax(), box.getYMin(), p, minDist, nearestPoint);
	distanceToPointUpdate(box.getXMax(), box.getYMid(), p, minDist, nearestPoint);
	distanceToPointUpdate(box.getXMax(), box.getYMax(), p, minDist, nearestPoint);
	GLEStringHash* children = obj->getChilds();
	if (children != NULL) {
		GLEStringHashData* hash = children->getHash();
		for (GLEStringHashData::const_iterator i = hash->begin(); i != hash->end(); i++) {
			GLEObjectRepresention* child = (GLEObjectRepresention*)children->getObject(i->second);
			distanceToPointRec(child, orig, p, minDist, nearestPoint);
		}
	}
}

//! Set the reference point (tl,bl,tc,bc,tr,br,...)
bool GLEObjectBlock::setReferencePoint(const QPointF& pt)
{
	GLERectangle box;
	double minDist = 1e300;
	GLEObjectDO* obj = (GLEObjectDO*)getGLEObject();
	GLEObjectRepresention* objrep = obj->getObjectRepresentation();
	box.copy(objrep->getRectangle());
	GLEPoint orig(box.getXMin(), box.getYMin());
	box.translate(1.0-orig.getX(), 1.0-orig.getY());
	box.scale(dpi/PS_POINTS_PER_INCH);
	box.subtractYFrom(image.height());
	box.translate(drawPoint.x(), drawPoint.y());
	box.normalize();
	box.grow(MAX_SELECT_DISTANCE);
	if (box.contains(pt.x(), pt.y())) {
		QString ref;
		return setReferencePointRec(ref, objrep, orig, pt, &minDist);
	}
	return true;
}

bool GLEObjectBlock::setReferencePointUpdate(double x, double y, const char* suff, const QString& ref, const QPointF& p, double* minDist)
{
	double crDist = QGLE::distance(x, y, p);
	if (crDist < MAX_SELECT_DISTANCE && crDist < *minDist) {
		*minDist = crDist;
		QString prop = ref;
		if (suff != NULL)
		{
			if (prop != "") prop += ".";
			prop += QString::fromUtf8(suff);
		}
		setProperty(RefPointName, prop);
		return true;
	}
	return false;
}

bool GLEObjectBlock::setReferencePointRec(const QString& ref, GLEObjectRepresention* obj, const GLEPoint& orig, const QPointF& p, double* minDist)
{
	GLERectangle box;
	box.copy(obj->getRectangle());
	box.translate(1.0-orig.getX(), 1.0-orig.getY());
	box.scale(dpi/PS_POINTS_PER_INCH);
	box.subtractYFrom(image.height());
	box.translate(drawPoint.x(), drawPoint.y());
	bool mod = false;
	if (box.isPoint())
	{
		mod |= setReferencePointUpdate(box.getXMin(), box.getYMin(), NULL, ref, p, minDist);
	}
	else
	{
		mod |= setReferencePointUpdate(box.getXMin(), box.getYMin(), "bl", ref, p, minDist);
		mod |= setReferencePointUpdate(box.getXMin(), box.getYMid(), "lc", ref, p, minDist);
		mod |= setReferencePointUpdate(box.getXMin(), box.getYMax(), "tl", ref, p, minDist);
		mod |= setReferencePointUpdate(box.getXMid(), box.getYMin(), "bc", ref, p, minDist);
		mod |= setReferencePointUpdate(box.getXMid(), box.getYMid(), "cc", ref, p, minDist);
		mod |= setReferencePointUpdate(box.getXMid(), box.getYMax(), "tc", ref, p, minDist);
		mod |= setReferencePointUpdate(box.getXMax(), box.getYMin(), "br", ref, p, minDist);
		mod |= setReferencePointUpdate(box.getXMax(), box.getYMid(), "rc", ref, p, minDist);
		mod |= setReferencePointUpdate(box.getXMax(), box.getYMax(), "tr", ref, p, minDist);
	}
	GLEStringHash* children = obj->getChilds();
	if (children != NULL) {
		GLEStringHashData* hash = children->getHash();
		for (GLEStringHashData::const_iterator i = hash->begin(); i != hash->end(); i++) {
			QString newRef = ref;
			if (ref != "") newRef += ".";
			newRef += QGLE::gleToQString(i->first.get());
			GLEObjectRepresention* child = (GLEObjectRepresention*)children->getObject(i->second);
			mod |= setReferencePointRec(newRef, child, orig, p, minDist);
		}
	}
	return mod;
}

// Set a point (start or end in the case of a line)
void GLEObjectBlock::setPoint(int pointChoice, const QPointF& p, bool update)
{
	switch(pointChoice)
	{
		case ReferencePoint:
			pointHash[pointChoice] = QGLE::absQtToGLE(p, dpi, pixmap.height());
			break;

		case ObjectWidth:
		case ObjectHeight:
			pointHash[pointChoice] = p;
			break;
	}
	if (update) updateObject();
}

QList<QPointF> GLEObjectBlock::intersections(double qtm, double qtc, bool vertical)
{
	UNUSED_ARG(qtm);
	UNUSED_ARG(qtc);
	UNUSED_ARG(vertical);
	return QList<QPointF>();
}

QList<QPointF> GLEObjectBlock::intersections(QPointF qtp1, QPointF qtp2)
{
	UNUSED_ARG(qtp1);
	UNUSED_ARG(qtp2);
	return QList<QPointF>();
}

QList<QPointF> GLEObjectBlock::intersections(QPointF qtp1, double angle)
{
	UNUSED_ARG(qtp1);
	UNUSED_ARG(angle);
	return QList<QPointF>();
}

bool GLEObjectBlock::isInside(QPointF p)
{
	return(paintPath->contains(p));
}

int GLEObjectBlock::supportedTransformMode()
{
	if (getGLEObject() != NULL && getGLEObject()->getProperties()->getModel()->isSupportScale())
	{
		return TransformModeFree;
	}
	return TransformModeNone;
}

void GLEObjectBlock::linearTransform(const GLELinearEquation& ex, const GLELinearEquation& ey)
{
	pointHash.clear();
	double wd = getGLELength(ObjectWidth, true)*ex.getA();
	double hi = getGLELength(ObjectHeight, true)*ey.getA();
	if (wd < 0 || hi < 0)
	{
		/* Something's wrong */
		return;
	}
	double sx = ex.getA()/scaleOffs.x();
	double sy = ey.getA()/scaleOffs.y();
	GLEObjectDO* obj = (GLEObjectDO*)getGLEObject();
	if (wd != getGLELength(ObjectWidth, false) || hi != getGLELength(ObjectHeight, false))
	{
		setLength(ObjectWidth, wd, false);
		setLength(ObjectHeight, hi, false);
		obj->getProperties()->getModel()->scale(obj, wd, hi);
		int int_wd = (int)floor(origImage.width()*sx+0.5);
		int int_hi = (int)floor(origImage.height()*sy+0.5);
		if (sx < 0.5 || sx > 2.0 || sy < 0.5 || sy > 2.0) {
			/* deviation too big -> rerender */
			obj->render();
			renderPostscriptNoUpdate();
			scaleOffs = QPointF(ex.getA(), ey.getA());
			sx = sy = 1.0;
		} else {
			image = origImage.scaled(int_wd, int_hi, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		}
	}
	linearTransformPt(ReferencePoint, ex, ey, false);
	updateObjectScale(sx, sy);
}

void GLEObjectBlock::linearTransformDone()
{
	GLEObjectDO* obj = (GLEObjectDO*)getGLEObject();
	obj->render();
	obj->computeReferencePoint(&referenceOffset);
	renderPostscript();
}

void GLEObjectBlock::moveBy(QPointF offset)
{
	pointHash.clear();
	GLEObjectDO* obj = (GLEObjectDO*)getGLEObject();
	if (obj->getProperties()->getModel()->isSupportScale())
	{
		setLength(ObjectWidth, getGLELength(ObjectWidth, true), false);
		setLength(ObjectHeight, getGLELength(ObjectHeight, true), false);
	}
	QPointF start = getQtPoint(ReferencePoint, true);
	setPoint(ReferencePoint, start + offset);
}

void GLEObjectBlock::rotateBy(double radians)
{
	// TODO: Also rotate the text.
	pointHash.clear();
	// Moves relative to storedPointHash
	QPointF start = getQtPoint(ReferencePoint, true);
	setPoint(ReferencePoint, QGLE::rotateAboutPoint(start, radians, basePoint));
}
