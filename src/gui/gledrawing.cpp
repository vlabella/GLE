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

#include <QtGui>

#include "mainwindow.h"
#include "drawingobject.h"
#include "line.h"
#include "circle.h"
#include "ellipse.h"
#include "arc.h"
#include "text.h"
#include "snapline.h"
#include "grid.h"
#include "amove.h"

// The constructor for the drawing area
GLEDrawingArea::GLEDrawingArea(GLEInterface *iface, GLEMainWindow* main) : QWidget(main)
{
	mainWin = main;
	gleInterface = iface;

	// Clear dirty flag (that indicates script should be saved)
	clearDirty();
	// Initialize all instance variables
	initAll();

	// Grid uses QT units, but grid spacing is set in GLE units
	grid = new GLEGrid(this);
	grid->setGrid(gridSpacing);
	grid->setDPI(dpi);

	// This ensures that Mouse Move events happen
	// even when the mouse buttons aren't pressed, which
	// is essential for updating the status bar and using
	// snap.
	setMouseTracking(true);

	// Make some handy messages to let the user know what to do
	statusMessages[NoTool][StartPoint] = tr("");
	statusMessages[PointerTool][StartPoint] = tr("Select an object, or hold shift to select multiple objects");
	statusMessages[LineTool][StartPoint] = tr("Select start point");
	statusMessages[TextTool][StartPoint] = tr("Select location for text");
	statusMessages[TanLineTool][StartPoint] = tr("Select start object");
	statusMessages[PerpLineTool][StartPoint] = tr("Select start object");
	statusMessages[CircleTool][StartPoint] = tr("Select centre point");
	statusMessages[EllipseTool][StartPoint] = tr("Select bounding box corner point");
	statusMessages[ArcTool][StartPoint] = tr("Select start point");
	statusMessages[AMoveTool][StartPoint] = tr("Select final point for GLE script");

	statusMessages[LineTool][EndPoint] = tr("Select end point");
	statusMessages[TanLineTool][EndPoint] = tr("Select end point");
	statusMessages[PerpLineTool][EndPoint] = tr("Select end point");
	statusMessages[ArcTool][EndPoint] = tr("Select end point");
	statusMessages[ArcTool][MidPoint] = tr("Select a point somewhere on the arc");
	statusMessages[CircleTool][CircumferencePoint] = tr("Select a point somewhere on the circle");
	statusMessages[EllipseTool][EndPoint] = tr("Select opposite corner point");

	CornerHandles.append(TopLeft);
	CornerHandles.append(TopRight);
	CornerHandles.append(BottomLeft);
	CornerHandles.append(BottomRight);
	SideHandles.append(MiddleLeft);
	SideHandles.append(MiddleRight);
	SideHandles.append(TopMiddle);
	SideHandles.append(BottomMiddle);
}

//! Initialize all instance variables
void GLEDrawingArea::initAll()
{
	dpi = 0.0;
	gridVisible = false;
	gridSpacing = QPointF(1.0,1.0);
	hasAMove = false;
	amoveIndex = 0;
	newObjects = false;
	isEditMode = false;
	// Snap types
	snaps[ObjectSnap] = false;
	snaps[OrthoSnap] = false;
	snaps[PolarSnap] = false;
	snaps[PolarTrack] = false;
	snaps[GridSnap] = false;
	// Polar snap settings
	polarStartAngle = 0.0;
	polarAngleIncrement = 30.0;
	// Assume that keys aren't pressed at start up
	modifier[ShiftKey] = false;
	modifier[AltKey] = false;
	modifier[CtrlKey] = false;
	// Default to no tool
	currentTool = NoTool;
	objectSelected = false;
	// Initialize orther variables
	initVars();
	// Remove all polar snaps
	clearPolarSnaps();
}

//! Initialize instance variables related to selection
void GLEDrawingArea::initSelectionVars()
{
	// Drawing modes
	isScaling = false;
	isMoving = false;
	isRotating = false;
	// Selection
	selectionMode = SelectionMove;
	objectMoved = false;
	// Which handle of the selection was grabbed?
	grabbedHandle = -1;
	// Selected object snap indicates snapping to points in selected object before move/scale
	isSelectedObjectSnap = false;
	// Selection handles
	handleList.clear();
	// Used for rotating objectss
	baseAngle = 0.0;
	lastAngle = 0.0;
	// Reference point for scale / move
	basePoint.setX(0.0); basePoint.setX(0.0);
	// Origin for scale operation (this is the point that does not move during scaling)
	scaleOrigin.setX(-1.0); scaleOrigin.setY(-1.0);
}

void GLEDrawingArea::initVars()
{
	// Busy drawing new object?
	isDrawing = false;
	// Index of object that is being created (in object list)
	currentIndex = 0;
	// Index of reference object when creating, e.g., a line orthogonal to an object
	baseObject = 0;
	// Cursor position and a copy that is snapped to the current snaps
	cursorPosition.setX(0.0); cursorPosition.setY(0.0);
	snappedPosition.setX(0.0); snappedPosition.setY(0.0);
	lastPoint.setX(0.0); lastPoint.setY(0.0);
	// Initialize instance variables related to selection
	initSelectionVars();
}

void GLEDrawingArea::setPolarSnapStartAngle(double newAngle)
{
	polarStartAngle = newAngle;
}

void GLEDrawingArea::setPolarSnapIncAngle(double newAngle)
{
	polarAngleIncrement = newAngle;
}

void GLEDrawingArea::modifierToggle(int mod, bool state)
{
	modifier[mod] = state;
}

// To allow us to notice whether changes have occurred
bool GLEDrawingArea::isDirty()
{
	return(dirty);
}

// To allow us to notice whether changes have occurred
void GLEDrawingArea::clearDirty()
{
	dirty = false;
	// The following sets the '*' on the title bar
	mainWin->setWindowModified(false);
	emit dirtyFlagChanged(dirty);
}

// To allow us to notice whether changes have occurred
void GLEDrawingArea::setDirtyAndSave()
{
	setDirty();
	newObjects = true;
}

// To allow us to notice whether changes have occurred
void GLEDrawingArea::setDirty()
{
	if (dirty) return;
	dirty = true;
	mainWin->setWindowModified(true);
	emit dirtyFlagChanged(dirty);
}

// Convert GLE point to Qt point
QPointF GLEDrawingArea::gleToQt(GLEPoint& pt)
{
	return QGLE::absGLEToQt(pt.getX(), pt.getY(), dpi, pixmap.height());
}

// Create new line object based on GLE object
void GLEDrawingArea::createNewLineFromScript(GLELineDO* line) {
	objectList.append(new GLELine(dpi, pixmap.size(), this));
	currentIndex = objectList.size() - 1;
	connect(this, SIGNAL(sizeChanged(QSize,double)), objectList[currentIndex], SLOT(setPixmapSize(QSize,double)));
	objectList[currentIndex]->setPoint(GLELine::StartPoint, gleToQt(line->getP1()));
	objectList[currentIndex]->setPoint(GLELine::EndPoint, gleToQt(line->getP2()));
	objectList[currentIndex]->setGLEObject(line);
	objectList[currentIndex]->setLineProperties();
	objectList[currentIndex]->setPropertyNoUpdate(GLELine::ArrowHeadPresence, line->getArrow());
}

// Create new ellipse object based on GLE object
void GLEDrawingArea::createNewEllipseFromScript(GLEEllipseDO* ellipse) {
	if (ellipse->isCircle()) objectList.append(new GLECircle(dpi, pixmap.size(), this));
	else objectList.append(new GLEEllipse(dpi, pixmap.size(), this));
	currentIndex = objectList.size() - 1;
	connect(this, SIGNAL(sizeChanged(QSize,double)), objectList[currentIndex], SLOT(setPixmapSize(QSize,double)));
	if (ellipse->isCircle()) {
		GLEPoint edge = ellipse->getPoint(GLEJustifyRC);
		objectList[currentIndex]->setPoint(GLECircle::CentrePoint, gleToQt(ellipse->getCenter()));
		objectList[currentIndex]->setPoint(GLECircle::CircumferencePoint, gleToQt(edge));
	} else {
		GLEPoint c1 = ellipse->getPoint(GLEJustifyTL);
		GLEPoint c2 = ellipse->getPoint(GLEJustifyBR);
		objectList[currentIndex]->setPoint(GLEEllipse::BBoxCornerA, gleToQt(c1));
		objectList[currentIndex]->setPoint(GLEEllipse::BBoxCornerB, gleToQt(c2));
	}
	objectList[currentIndex]->setGLEObject(ellipse);
	objectList[currentIndex]->setLineProperties();
	objectList[currentIndex]->setShapeProperties();
}

// Create new ellipse object based on GLE object
void GLEDrawingArea::createNewArcFromScript(GLEArcDO* arc) {
	GLEPoint dummy;
	objectList.append(new GLEArc(dpi, pixmap.size(), this));
	currentIndex = objectList.size() - 1;
	connect(this, SIGNAL(sizeChanged(QSize,double)), objectList[currentIndex], SLOT(setPixmapSize(QSize,double)));
	objectList[currentIndex]->setPoint(GLEArc::CentrePoint, gleToQt(arc->getCenter()));
	objectList[currentIndex]->setPoint(GLEArc::StartPoint, gleToQt(arc->getPoint1(dummy)));
	objectList[currentIndex]->setPoint(GLEArc::EndPoint, gleToQt(arc->getPoint2(dummy)));
	objectList[currentIndex]->setPoint(GLEArc::MidPoint, gleToQt(arc->getPointMid(dummy)));
	objectList[currentIndex]->setGLEObject(arc);
	objectList[currentIndex]->setLineProperties();
	objectList[currentIndex]->setPropertyNoUpdate(GLELine::ArrowHeadPresence, arc->getArrow());
}

// Create new text object based on GLE object
void GLEDrawingArea::createNewTextFromScript(GLETextDO* text) {
	QColor color;
	GLEPoint dummy;
	GLEText* textobj = new GLEText(dpi, pixmap.size(), this);
	textobj->setGLEObject(text);
	objectList.append(textobj);
	connect(this, SIGNAL(sizeChanged(QSize,double)), textobj, SLOT(setPixmapSize(QSize,double)));
	textobj->setPoint(GLEText::ReferencePoint, gleToQt(text->getPosition()), false);
	textobj->setPropertyNoUpdate(GLEText::Alignment, text->getProperties()->getIntProperty(GLEDOPropertyJustify));
	textobj->setPropertyNoUpdate(GLEText::Text, QString::fromUtf8(text->getTextC()));
	textobj->setColorProperty();
	textobj->setTextProperties();
	textobj->updateFromPropertyNoDirty(GLEText::Text);
}

// Create new object block based on GLE object
void GLEDrawingArea::createNewObjectFromScript(GLEObjectDO* obj) {
	GLEObjectBlock *newObj = new GLEObjectBlock(dpi, pixmap.size(), this);
	connect(this, SIGNAL(sizeChanged(QSize,double)), newObj, SLOT(setPixmapSize(QSize,double)));
	objectList.append(newObj);
	newObj->setGLEObject(obj);
	newObj->setPropertyNoUpdate(GLEDrawingObject::RefPointName, QGLE::gleToQString(obj->getRefPointString()));
	// FIXME: render object in GLE thread instead of GUI thread
	obj->render();
	newObj->computeInitialPosition();
	GLEPropertyStore* props = obj->getProperties();
	double fontsize = props->getRealProperty(GLEDOPropertyFontSize);
	newObj->setPropertyNoUpdate(GLEObjectBlock::FontSize, fontsize);
	GLEFont* font = props->getFontProperty(GLEDOPropertyFont);
	GLEFont* base = font->getBaseFont();
	newObj->setPropertyNoUpdate(GLEObjectBlock::FontName, base->getNumber());
	newObj->setPropertyNoUpdate(GLEObjectBlock::FontStyle, base->checkStyle(font));
	if (props->getModel()->isSupportScale()) {
		GLEArrayImpl* arr = props->getArray();
		newObj->setLength(GLEObjectBlock::ObjectWidth, arr->getDouble(0), false);
		newObj->setLength(GLEObjectBlock::ObjectHeight, arr->getDouble(1), false);
	}
	newObj->setTextProperties();
	newObj->setLineProperties();
	newObj->setShapeProperties();
	newObj->renderPostscript();
}

// Create objects from given GLE script
void GLEDrawingArea::updateFromScript(GLEScript* script) {
	clearObjects();
	gleScript = script;
	for (int i = 0; i < script->getNumberObjects(); i++) {
		GLEDrawObject* obj = script->getObject(i);
		GLEDrawObjectType type = obj->getType();
		switch (type) {
			case GDOLine:
				createNewLineFromScript((GLELineDO*)obj);
				break;
			case GDOEllipse:
				createNewEllipseFromScript((GLEEllipseDO*)obj);
				break;
			case GDOArc:
				createNewArcFromScript((GLEArcDO*)obj);
				break;
			case GDOText:
				createNewTextFromScript((GLETextDO*)obj);
				break;
			case GDOObject:
				createNewObjectFromScript((GLEObjectDO*)obj);
				break;
			default:
				break;
		}
	}
	isDrawing = false;
	update();
}

void GLEDrawingArea::paintEvent(QPaintEvent *event)
{
	// Create a QPainter to draw on the widget
	QPainter painter(this);
	// Fill the widget with white.
	painter.setRenderHint(QPainter::Antialiasing);

	QPointF nearestPoint;
	createSelectionHandles();

	// If there is a pixmap available to draw,
	if (pixmap.isNull())
	{
		painter.fillRect(event->rect(), Qt::white);
		return;
	}
	// Always draw the pixmap first as it'll cover anything else
	// Just update the part from event->rect() for efficiency
	painter.drawPixmap(event->rect(), pixmap, event->rect());

	if (isEditMode)
	{
		QPen cursorPen = grid->pen();
		cursorPen.setColor(Qt::black);
		cursorPen.setWidth(1);

		// Always draw the grid next, so that objects are over
		// the top of the grid.
		painter.save();
		if ((snaps[GridSnap] || snaps[ObjectSnap] || polarSnapFound || isSelectedObjectSnap) &&
				((currentTool != PointerTool) || objectSelected)
				&& (currentTool != NoTool))
		{
			// Draw the cursor unless we're using the selection tool
			painter.setPen(cursorPen);
			int crossLength = 4;
			// Draw a cross at the nearest grid point
			QGLE::drawCross(&painter, snappedPosition, crossLength, cursorPen);
		}
		painter.restore();

		QPen osnapPen;
		osnapPen.setColor(Qt::darkYellow);
		osnapPen.setWidth(1);

		if (snaps[PolarSnap] &&
				!snaps[GridSnap] &&
				(!snaps[OrthoSnap] || snaps[PolarTrack] || !isDrawing))
		{
			SnapLine *snap;
			foreach(snap,snapLineList)
			{
				snap->draw(&painter);
			}
			QPointF snapPoint;
			foreach(snapPoint, snapLinePoints)
			{
				QGLE::drawBox(&painter, snapPoint, OSNAP_BOX_SIZE, osnapPen);
			}
		}

		// If we have any objects, draw them.  At this level, we have
		// no information as to whether the object is a line, circle,
		// arc or whatever: we just use the common functionality.
		for(int i=0;i<objectList.size();i++)
		{
			if (!objectList[i]->isAmove())
				objectList[i]->draw(&painter);

			if (snaps[ObjectSnap] && ((currentTool != PointerTool) || isScaling || isMoving))
			{
				if ((snaps[OrthoSnap] || polarSnapFound) && (osnapIntersectPoints.size() > 0))
				{
					QPointF p;
					foreach(p,osnapIntersectPoints)
					{
						QGLE::drawOSnap(&painter, p, QGLE::IntersectSnap);
					}
				}
				else
				{
					if ((i != currentIndex) || !isDrawing)
					{
						if (objectList[i]->isSelected())
							continue;
							if (objectList[i]->distanceToPoint(cursorPosition, &nearestPoint) < MAX_OSNAP_DRAW_DISTANCE)
						{
							objectList[i]->drawOSnaps(&painter);
							QGLE::drawBox(&painter, nearestPoint, OSNAP_BOX_SIZE, osnapPen);
						}
						else if (objectList[i]->isInside(cursorPosition))
							objectList[i]->drawOSnaps(&painter);
					}
				}
			}
			else if (currentTool == PointerTool && objectList[i]->isSelected())
			{
				if (objectList[i]->distanceToPoint(cursorPosition, &nearestPoint) < MAX_OSNAP_DRAW_DISTANCE)
				{
					objectList[i]->drawOSnaps(&painter);
					QGLE::drawBox(&painter, nearestPoint, OSNAP_BOX_SIZE, osnapPen);
				} else if (objectList[i]->isInside(cursorPosition))
				{
					objectList[i]->drawOSnaps(&painter);
				}
			}
		}

		// Draw a cross to show where the final point is
		if (hasAMove)
		{
			objectList[amoveIndex]->draw(&painter);
		}

		if (objectSelected)
		{
			if (selectionMode == SelectionMove)
				drawSelectionHandles(&painter);
			else if (selectionMode == SelectionScale)
				drawScaleHandles(&painter);
			else
				drawRotationHandles(&painter);
		}

		if (objectSelected && selectionMode == SelectionScale && scaleOrigin.x() != -1.0)
		{
			QBrush blackBrush(Qt::red);
			QGLE::fillBox(&painter, scaleOrigin, OSNAP_BOX_SIZE, blackBrush);
			QGLE::drawBox(&painter, scaleOrigin, OSNAP_BOX_SIZE, cursorPen);
		}

		// FOR DEBUGGING
		QPen extraPen;
		extraPen.setColor(Qt::red);
		extraPen.setWidth(2);
		QPointF pt;
		foreach(pt, extraPoints)
		{
			QGLE::drawCross(&painter, pt, AMOVE_LENGTH, extraPen);
		}

	}

}

// Are there any objects to draw?
bool GLEDrawingArea::thereAreObjects()
{
	return(!objectList.isEmpty());
}


// Get rid of any objects that have been drawn
void GLEDrawingArea::clearObjects()
{
	objectSelected = false;
	hasAMove = false;
	amoveIndex = -1;
	GLEDrawingObject *obj;
	foreach (obj, objectList)
	{
		delete obj;
	}
	objectList.clear();
    updateSelection();
}

// Update the resolution and notify children
void GLEDrawingArea::setDPI(double new_dpi)
{
	if (dpi != new_dpi)
	{
		dpi = new_dpi;
		// Emit a signal to all daughters
		emit dpiChanged(dpi);
	}
}

// Update the grid spacing
void GLEDrawingArea::setGrid(QPointF newGrid)
{
	if (gridSpacing != newGrid)
	{
		gridSpacing = newGrid;
		grid->setGrid(gridSpacing);
		if (gridVisible)
		{
			mainWin->refreshDisplay();
		}
	}
}

// This slot receives an image from the server thread
// and updates the display accordingly.
void GLEDrawingArea::updateDisplay(QImage image)
{
	if (!image.isNull()) pixmap = QPixmap::fromImage(image);
	else pixmap = QPixmap();

	grid->setDPI(dpi);
	grid->setArea(pixmap.size());

	if (!pixmap.isNull() && isEditMode && gridVisible)
	{
		// Draw grid on backing pixmap
		// This is more efficient then redrawing it on each paint event
		// Future: create separate backing pixmap for grid and objects
		QPainter painter(&pixmap);
		painter.setPen(grid->pen());
		grid->drawGrid(&painter);
	}

	// Notify children if the height has changed:
	// used for coordinate transforms
	emit sizeChanged(pixmap.size(), dpi);

	// Resize the drawing area to the size of the pixmap
	resize(pixmap.width(),pixmap.height());

	// Update the display
	update();
}

// Toggle the visibility of the grid
void GLEDrawingArea::gridToggle(bool state)
{
	gridVisible = state;
	mainWin->refreshDisplay();
}

void GLEDrawingArea::osnapToggle(bool state)
{
	snaps[ObjectSnap] = state;
	update();
}

void GLEDrawingArea::polarSnapToggle(bool state)
{
	snaps[PolarSnap] = state;
	if (state == false)
		snaps[PolarTrack] = false;
	update();
}

void GLEDrawingArea::polarTrackToggle(bool state)
{
	snaps[PolarTrack] = state;
	if (state == true)
		snaps[PolarSnap] = true;
	update();
}

void GLEDrawingArea::orthoSnapToggle(bool state)
{
	snaps[OrthoSnap] = state;
	// Will probably have to edit the current line here too!
	update();
}

double GLEDrawingArea::getDPI()
{
	return(dpi);
}

// Toggle grid snap
void GLEDrawingArea::gridSnapToggle(bool state)
{
	snaps[GridSnap] = state;
	update();
}

void GLEDrawingArea::createPolarSnaps(QPointF qtPoint)
{
	SnapLine *snap;
	SnapLine *sn;
	bool duplicate;
	for(double i = polarStartAngle; i < 360.0 ; i += polarAngleIncrement)
	{
		snap = new SnapLine(dpi, pixmap.size(), this);
		snap->setPoint(SnapLine::StartPoint, qtPoint);
		snap->setPoint(SnapLine::Angle, QPointF(QGLE::degreesToRadians(i),0.0));

		duplicate = false;
		foreach(sn, snapLineList)
		{
			if (snap->isEqual(sn))
			{
				duplicate = true;
				break;
			}
		}

		if (!duplicate)
			snapLineList.append(snap);
	}
}

void GLEDrawingArea::clearPolarSnaps()
{
	SnapLine *snap;
	foreach(snap, snapLineList)
	{
		delete snap;
	}
	snapLineList.clear();
	snapLinePoints.clear();
	polarSnapFound = false;
}

// Event called when a mouse button is pressed;
void GLEDrawingArea::mousePressEvent(QMouseEvent *event)
{
	// Move more of the functionality into the command
	// processor, for example setDirty() etc
	if (currentTool == NoTool)
		return;

	QPointF qtPoint;
	if (event->button() == Qt::LeftButton)
	{
		clearPolarSnaps();
		switch (currentTool)
		{
			case LineTool:
				// Click to start; click to end.
				// The end point is continuously updated
				// in the mouseMoveEvent function
				if (isDrawing == true)
				{
					// Drawing finished
					isDrawing = false;
					clearRelativeOsnaps();
					processCommand(QGLE::EndNewLine);
					changeLastPoint(QPointF(0.0,0.0));
					emit updateStatusBar(statusMessages[LineTool][StartPoint]);

					setDirtyAndSave();
				}
				else
				{
					// Register the start point
					qtPoint = snappedPoint(event->pos(), true);

					processCommand(QGLE::StartNewLineWithPoint, qtPoint);
					// Create snap lines from this point
					if(snaps[PolarSnap])
						createPolarSnaps(qtPoint);

					changeLastPoint(QGLE::absQtToGLE(qtPoint, dpi, pixmap.height()));
					emit updateStatusBar(statusMessages[LineTool][EndPoint]);

				}

				break;

			case TextTool:
				// Register the start point
				qtPoint = snappedPoint(event->pos(), true);
				processCommand(QGLE::CreateTextObject, qtPoint);
				setDirtyAndSave();
				// Not sure how necessary these two lines are...
				clearRelativeOsnaps();
				changeLastPoint(QPointF(0.0,0.0));
				break;

			case TanLineTool:
				// Click to start; click to end.
				// The end point is continuously updated
				// in the mouseMoveEvent function, AS IS THE START POINT
				if (isDrawing == true)
				{
					// Drawing finished
					isDrawing = false;
					clearRelativeOsnaps();
					processCommand(QGLE::EndNewTanLine);
					changeLastPoint(QPointF(0.0,0.0));
					emit updateStatusBar(statusMessages[TanLineTool][StartPoint]);

					setDirtyAndSave();
				}
				else
				{
					// Register the start point
					qtPoint = snappedPoint(event->pos(), true);

					processCommand(QGLE::StartNewTanLineWithPoint, qtPoint);
					// NO SNAP lines from start point?
					// Create snap lines from this point
					//if(snaps[PolarSnap])
					//	createPolarSnaps(qtPoint);


				}

				break;

			case PerpLineTool:
				// Click to start; click to end.
				// The end point is continuously updated
				// in the mouseMoveEvent function, AS IS THE START POINT
				if (isDrawing == true)
				{
					// Drawing finished
					isDrawing = false;
					clearRelativeOsnaps();
					processCommand(QGLE::EndNewPerpLine);
					changeLastPoint(QPointF(0.0,0.0));
					emit updateStatusBar(statusMessages[TanLineTool][StartPoint]);

					setDirtyAndSave();
				}
				else
				{
					// Register the start point
					qtPoint = snappedPoint(event->pos(), true);

					processCommand(QGLE::StartNewPerpLineWithPoint, qtPoint);
					// NO SNAP lines from start point?
					// Create snap lines from this point
					//if(snaps[PolarSnap])
					//	createPolarSnaps(qtPoint);


				}

				break;

			case ArcTool:
				// Click for start, click for end, click for mid point
				if (isDrawing == true)
				{
					if (objectList[currentIndex]->isSet(GLEArc::CentrePoint))
					{
						// Then we're finished, so stop drawing
						isDrawing = false;
						processCommand(QGLE::EndNewArc);
						clearRelativeOsnaps();
						changeLastPoint(QPointF(0,0));
						emit updateStatusBar(statusMessages[ArcTool][StartPoint]);
						setDirtyAndSave();
					}
					else
					{
						qtPoint = snappedPoint(event->pos());

						processCommand(QGLE::SetArcMidPoint, qtPoint);

						if(snaps[PolarSnap])
							createPolarSnaps(qtPoint);

						changeLastPoint(QGLE::absQtToGLE(qtPoint, dpi, pixmap.height()));
						emit updateStatusBar(statusMessages[ArcTool][MidPoint]);
						// Create snap lines from this point
					}

				}
				else
				{
					// Register the start point
					qtPoint = snappedPoint(event->pos(), true);
					processCommand(QGLE::StartNewArcWithPoint, qtPoint);
					// Create snap lines from this point
					if (snaps[PolarSnap])
						createPolarSnaps(qtPoint);

					changeLastPoint(QGLE::absQtToGLE(qtPoint, dpi, pixmap.height()));
					emit updateStatusBar(statusMessages[ArcTool][EndPoint]);

				}

				break;


			case CircleTool:
				// Click for centre, click for point on circumference
				if (isDrawing == true)
				{
					// Drawing finished
					isDrawing = false;
					clearRelativeOsnaps();
					processCommand(QGLE::EndNewCircle);
					changeLastPoint(QPointF(0,0));
					emit updateStatusBar(statusMessages[CircleTool][StartPoint]);
					setDirtyAndSave();
				}
				else
				{
					// Register the start point
					qtPoint = snappedPoint(event->pos(), true);
					processCommand(QGLE::StartNewCircleWithCentre, qtPoint);
					// Create snap lines from this point
					if (snaps[PolarSnap])
						createPolarSnaps(qtPoint);

					changeLastPoint(QGLE::absQtToGLE(qtPoint, dpi, pixmap.height()));
					emit updateStatusBar(statusMessages[CircleTool][CircumferencePoint]);

				}

				break;

			case EllipseTool:
				// Click for corner 1, click for corner 2
				if (isDrawing == true)
				{
					// Drawing finished
					isDrawing = false;
					clearRelativeOsnaps();
					processCommand(QGLE::EndNewEllipse);
					changeLastPoint(QPointF(0,0));
					emit updateStatusBar(statusMessages[EllipseTool][StartPoint]);
					setDirtyAndSave();
				}
				else
				{
					// Register the start point
					qtPoint = snappedPoint(event->pos(), true);
					processCommand(QGLE::StartNewEllipseWithCorner, qtPoint);
					// Create snap lines from this point
					if (snaps[PolarSnap])
						createPolarSnaps(qtPoint);

					changeLastPoint(QGLE::absQtToGLE(qtPoint, dpi, pixmap.height()));
					emit updateStatusBar(statusMessages[EllipseTool][EndPoint]);

				}

				break;


			case AMoveTool:
				// Add an 'amove' at the end of the file
				snappedPosition = snappedPoint(event->pos(), true);

				processCommand(QGLE::SetAMove, snappedPosition);

				setDirtyAndSave();
				emit updateStatusBar(statusMessages[AMoveTool][StartPoint]);

				break;

			case PointerTool:
				clearRelativeOsnaps();
				QPointF* dontFind = NULL;
				if (scaleOrigin.x() != -1.0) dontFind = &scaleOrigin;
				snappedPosition = snappedPointFromObject(event->pos(), dontFind);
				// Note: This is the only call to selectObjects
				selectObjects(snappedPosition, modifier[ShiftKey]);

				if (isMoving || isScaling || isRotating)
				{
					if(snaps[PolarSnap])
						createPolarSnaps(snappedPosition);
					changeLastPoint(QGLE::absQtToGLE(snappedPosition, dpi, pixmap.height()));
				}

				// Redraw the screen
		}
	}

#ifndef QT_NO_DEBUG
	else if (event->button() == Qt::RightButton)
	{
		if (objectSelected)
		{
			GLEDrawingObject *obj;
			bool first = true;
			QSet<int> propertySet;
			foreach(obj, objectList)
			{
				if (obj->isSelected())
				{
					if (first)
					{
						propertySet = obj->getValidProperties();
					}
					else
					{
						first = false;
						propertySet.intersect(obj->getValidProperties());
					}
				}
			}

			// We now have a list of the properties that are common to each object
			if (propertySet.contains(GLEDrawingObject::FillColour))
			{
				QList<QColor> colourList;
				colourList
					<< QColor()
					<< QColor(Qt::red)
					<< QColor(Qt::blue)
					<< QColor(Qt::green)
					<< QColor(Qt::yellow);

				QColor objectColour;
				foreach(obj, objectList)
				{
					if (obj->isSelected())
					{
						// Get the current colour
						objectColour = obj->getProperty(GLEDrawingObject::FillColour).value<QColor>();
						if (colourList.contains(objectColour))
						{
							obj->setProperty(GLEDrawingObject::FillColour,
									colourList[(colourList.indexOf(objectColour) + 1) %
										colourList.size()]);
						}
						else
						{
							obj->setProperty(GLEDrawingObject::FillColour,
									colourList[0]);
						}
					}
				}
			}
		}
	}
#endif // QT_NO_DEBUG not defined
	update();
}

//! Set the reference point of, e.g., text objects or GLE object blocks (tl,bl,tc,bc,tr,br,...)
void GLEDrawingArea::setReferencePoint(const QPointF& pt)
{
	// qDebug() << "Setting reference point: " << pt;
	bool modified = false;
	GLEDrawingObject* obj;
	foreach(obj, objectList)
	{
		if (obj->isSelected())
		{
			modified |= obj->setReferencePoint(pt);
		}
	}
	if (modified) updateSelection();
}

//! Get maximum supported transform mode
int GLEDrawingArea::minimumSupportedTransformMode()
{
	GLEDrawingObject* obj;
	int minimum = GLEDrawingObject::TransformModeFree;
	foreach(obj, objectList)
	{
		if (obj->isSelected()) {
			int mode = obj->supportedTransformMode();
			if (mode < minimum) minimum = mode;
		}
	}
	return minimum;
}

//! Inform objects that we are done with the transformation (they can re-render themselves now)
void GLEDrawingArea::linearTransformDone()
{
	GLEDrawingObject* obj;
	foreach(obj,objectList)
	{
		if (obj->isSelected()) {
			obj->linearTransformDone();
		}
	}
}

void GLEDrawingArea::performScale(const QPointF& snappedPosition)
{
	int minMode = minimumSupportedTransformMode();
	if (minMode == GLEDrawingObject::TransformModeNone)
		return;

	/*
	qDebug() << "-- Perform scale: " << snappedPosition;
	qDebug() << "   Origin: " << scaleOrigin;
	qDebug() << "   Base: " << basePoint;
	qDebug() << "   Handle: " << grabbedHandle;
	*/

	if (fabs(scaleOrigin.x()-basePoint.x()) < 1e-6 && fabs(scaleOrigin.y()-basePoint.y()) < 1e-6) {
		// Illegal scale: scaleOrigin and basePoint equal
		scaleOrigin = QPointF(-1.0, 0);
		return;
	}

	// Should do scale along one dimension?
	bool onlyX = (grabbedHandle == MiddleLeft || grabbedHandle == MiddleRight);
	bool onlyY = (grabbedHandle == TopMiddle || grabbedHandle == BottomMiddle);
	if (fabs(scaleOrigin.x()-basePoint.x()) < 1e-6)
	{
		// Can't scale in x-dimension: scaleOrigin and basePoint equal
		onlyX = false; onlyY = true;
	}
	else if (fabs(scaleOrigin.y()-basePoint.y()) < 1e-6)
	{
		// Can't scale in t-dimension: scaleOrigin and basePoint equal
		onlyX = true; onlyY = false;
	}

	GLELinearEquation transX, transY;
	if (minMode == GLEDrawingObject::TransformModeConstrained || modifier[CtrlKey])
	{
		// Scale factor in x- and y-dimension should be equal
		if (onlyX)
		{
			// only scale in x-direction possible
			transX.fit(scaleOrigin.x(), scaleOrigin.x(), basePoint.x(), snappedPosition.x());
			transY.setA(transX.getA());
			transY.fitB(scaleOrigin.y(), scaleOrigin.y());
		}
		else if (onlyY)
		{
			// only scale in y-direction possible
			transY.fit(scaleOrigin.y(), scaleOrigin.y(), basePoint.y(), snappedPosition.y());
			transX.setA(transY.getA());
			transX.fitB(scaleOrigin.x(), scaleOrigin.x());
		}
		else
		{
			double det = (scaleOrigin.x()-basePoint.x())*(scaleOrigin.x()-basePoint.x()) +
			             (scaleOrigin.y()-basePoint.y())*(scaleOrigin.y()-basePoint.y());
			if (det != 0.0) {
				double a = ((snappedPosition.x()-scaleOrigin.x())*(basePoint.x()-scaleOrigin.x()) +
						(snappedPosition.y()-scaleOrigin.y())*(basePoint.y()-scaleOrigin.y())) / det;
				transX.setA(a);
				transY.setA(a);
				transX.fitB(scaleOrigin.x(), scaleOrigin.x());
				transY.fitB(scaleOrigin.y(), scaleOrigin.y());
			}
		}
	}
	else
	{
		// Scale factor in x- and y-dimension can be different (and is determined independently)
		if (onlyX)
		{
			transX.fit(scaleOrigin.x(), scaleOrigin.x(), basePoint.x(), snappedPosition.x());
		}
		else if (onlyY)
		{
			transY.fit(scaleOrigin.y(), scaleOrigin.y(), basePoint.y(), snappedPosition.y());
		}
		else
		{
			transX.fit(scaleOrigin.x(), scaleOrigin.x(), basePoint.x(), snappedPosition.x());
			transY.fit(scaleOrigin.y(), scaleOrigin.y(), basePoint.y(), snappedPosition.y());
		}
	}

	// Division close to zero
	if (fabs(transX.getA()) > 1e10) return;
	if (fabs(transY.getA()) > 1e10) return;

	GLEDrawingObject *obj;
	objectMoved = true;
	foreach(obj,objectList)
	{
		if (obj->isSelected()) {
			obj->linearTransform(transX, transY);
			setDirtyAndSave();
		}
	}
	createSelectionHandles();
}

// Event called when the mouse moves over the drawing area
void GLEDrawingArea::mouseMoveEvent(QMouseEvent *event)
{
	// Get the mouse position in GLE coordinates
	QPointF glePosition;

	cursorPosition = event->pos();

//	if (isDrawing || isScaling || isMoving)
	updateSnapLines(cursorPosition);

/*(event->buttons() & Qt::LeftButton) == 0*/

	if (currentTool == PointerTool && objectSelected && (event->buttons() & Qt::LeftButton) == 0) {
		// snap to point of selected objects -> used to easily select basePoint
		isSelectedObjectSnap = true;
		QPointF* dontFind = NULL;
		if (scaleOrigin.x() != -1.0) dontFind = &scaleOrigin;
		snappedPosition = snappedPointFromObject(cursorPosition, dontFind);
	} else {
		isSelectedObjectSnap = false;
		snappedPosition = snappedPoint(cursorPosition);
	}

	glePosition = QGLE::absQtToGLE(snappedPosition, dpi, pixmap.height());

	// Notify the world (currently just used to update the status bar)
	emit mouseMoved(glePosition);

	if (currentTool == NoTool)
	{
		return;
	}

	if(isDrawing)
	{
		switch (currentTool)
		{
			case LineTool:
				// If we're drawing, set the end point of the line to the
				// current mouse position so that the line is drawn in the
				// paintEvent.
				processCommand(QGLE::SetLineEndPoint, snappedPosition);
				update();
				break;

			case TanLineTool:
				// If we're drawing, set the end point of the line to the
				// current mouse position so that the line is drawn in the
				// paintEvent.
				processCommand(QGLE::SetTanLineEndPoint, snappedPosition);
				update();
				break;

			case PerpLineTool:
				// If we're drawing, set the end point of the line to the
				// current mouse position so that the line is drawn in the
				// paintEvent.
				processCommand(QGLE::SetPerpLineEndPoint, snappedPosition);
				update();
				break;

			case CircleTool:
				// Set a point on the circumference of the circle
				processCommand(QGLE::SetCircleEdgePoint, snappedPosition);
				update();
				break;

			case EllipseTool:
				// Set a point on the circumference of the circle
				processCommand(QGLE::SetEllipseSecondCorner, snappedPosition);
				update();
				break;

			case ArcTool:
				// Either set the end point of the line or the midpoint
				if (objectList[currentIndex]->isSet(GLEArc::MidPoint))
					processCommand(QGLE::SetArcMidPoint, snappedPosition);
				else
					processCommand(QGLE::SetArcEndPoint, snappedPosition);
				update();
				break;

		}

	}
	else if (isScaling)
	{
		switch(currentTool)
		{
			case PointerTool:
				performScale(snappedPosition);
				update();
				break;
		}
	}
	else if (isMoving)
	{
		GLEDrawingObject *obj;
		objectMoved = true;
		foreach(obj, objectList)
		{
			if (obj->isSelected()) {
				obj->moveBy(snappedPosition - basePoint);
				setDirtyAndSave();
			}
		}
		createSelectionHandles();
		update();
	}
	else if (isRotating)
	{
		// TODO: rotate each selected object and rotate the handles
		double newAngle = QGLE::angleBetweenTwoPoints(basePoint, cursorPosition);
		double angleChange = newAngle - baseAngle;

		// Now rotate all objects by angle 'angleChange'
		GLEDrawingObject *obj;
		foreach(obj, objectList)
		{
			if (obj->isSelected()) {
				obj->rotateBy(-angleChange);
				setDirtyAndSave();
			}
		}

		angleChange = -(newAngle - lastAngle);

		// Update the handles
		handleList[TopLeft] = QGLE::rotateAboutPoint(handleList[TopLeft],
				angleChange,
				basePoint);
		handleList[BottomLeft] = QGLE::rotateAboutPoint(handleList[BottomLeft],
				angleChange,
				basePoint);
		handleList[BottomRight] = QGLE::rotateAboutPoint(handleList[BottomRight],
				angleChange,
				basePoint);
		handleList[TopRight] = QGLE::rotateAboutPoint(handleList[TopRight],
				angleChange,
				basePoint);

		lastAngle = newAngle;
		update();
	}
	else
	{
		if ((snaps[GridSnap] || snaps[ObjectSnap] || isSelectedObjectSnap) && ((currentTool != PointerTool) || objectSelected))
			update();

		if (currentTool == PointerTool && selectionMode == SelectionScale)
			emit updateStatusBar("Drag to scale object (ctrl locks aspect ratio) or click to select scale origin (red box)");
		else if (currentTool == PointerTool && selectionMode == SelectionMove && objectSelected)
		{
			int minMode = minimumSupportedTransformMode();
			if (minMode != GLEDrawingObject::TransformModeNone)
				emit updateStatusBar("Drag snap point from object to move; shift+click to multi-select; click to scale");
			else
				emit updateStatusBar("Drag snap point from object to move; shift+click to multi-select");
		}
		else
			emit updateStatusBar(statusMessages[currentTool][StartPoint]);
	}
}

// Event called when the mouse button is released.
void GLEDrawingArea::mouseReleaseEvent(QMouseEvent *)
{
	QStringList code;
	switch (currentTool)
	{
		case CircleTool:
		case EllipseTool:
		case ArcTool:
		case LineTool:
		case TextTool:
		case TanLineTool:
		case PerpLineTool:
			break;

		case PointerTool:
			if (selectionMode == SelectionScale)
			{
				if (objectMoved)
				{
					linearTransformDone();
				}
				else
				{
					// Select a different scale origin (marked with a red filled square)
					scaleOrigin = basePoint;
				}
			}

			if (selectionMode == SelectionMove && objectSelected) {
				int minMode = minimumSupportedTransformMode();
				if (minMode != GLEDrawingObject::TransformModeNone)
					emit updateStatusBar("Drag snap point from object to move; shift+click to multi-select; click to scale");
				else
					emit updateStatusBar("Drag snap point from object to move; shift+click to multi-select");
			}

			if (isMoving && !objectMoved)
			{
				int minMode = minimumSupportedTransformMode();

				// We haven't moved the object, so switch mode
				if (selectionMode == SelectionMove && minMode != GLEDrawingObject::TransformModeNone)
				{
					selectionMode = SelectionScale;
					emit updateStatusBar("Drag to scale object (ctrl locks aspect ratio) or click to select scale origin (red box)");
				}
				// NOTE: Currently disabled because generating GLE script code
				//       from rotated objects is not yet implemented
				//else if (selectionMode == SelectionScale)
				//	selectionMode = SelectionRotate;
				else
					selectionMode = SelectionMove;
			}

			if (isMoving && objectMoved)
			{
				setReferencePoint(snappedPosition);
			}

			isScaling = false;
			isMoving = false;
			isRotating = false;
			objectMoved = false;
			clearRelativeOsnaps();
			clearPolarSnaps();
			changeLastPoint(QPointF(0.0,0.0));
			if (objectSelected)
			{
				createSelectionHandles();
				if (selectionMode == SelectionScale)
				{
					// Make sure that object properties are updated
					updateSelection();
				}
			}
			update();
			break;
	}
}

void GLEDrawingArea::unselectObjects()
{
	if (objectSelected == true)
	{
		GLEDrawingObject *obj;
		foreach(obj, objectList)
		{
			if (obj->isSelected())
				obj->setSelected(false);
		}
	}
	objectSelected = false;
	initSelectionVars();
	updateSelection();
}

void GLEDrawingArea::changeLastPoint(QPointF p)
{
	lastPoint = QGLE::absGLEToQt(p,dpi,pixmap.height());
	emit basePointChanged(p);

	if (isDrawing || isMoving)
	{
		for(int i=0;i<objectList.size();i++)
		{
			if (i != currentIndex && !objectList[i]->isSelected())
				objectList[i]->addRelativeOSnaps(lastPoint);
		}
	}
	else
	{
		clearRelativeOsnaps();
	}
}


// Clear the New Objects flag
void GLEDrawingArea::clearNewObjectsFlag()
{
	newObjects = false;
}

void GLEDrawingArea::setNewObjectsFlag()
{
	newObjects = true;
}

// Identify whether there are new objects
bool GLEDrawingArea::thereAreNewObjects()
{
	return(newObjects);
}

void GLEDrawingArea::updateSnapLines(QPointF qtPoint)
{
	QPointF nearestPoint;
	snapLinePoints.clear();
	for(int i=snapLineList.size()-1;i>=0;i--)
	{
		// Are we too far away to keep it?
		if (snapLineList[i]->distanceToPoint(qtPoint, &nearestPoint) > MAX_SNAP_LINE_DISTANCE)
		{
			if (QGLE::distance(qtPoint,
						snapLineList[i]->getQtPoint(SnapLine::StartPoint)) > 3*MAX_SNAP_LINE_DISTANCE)
				snapLineList[i]->deactivate();
			else
			{
				delete snapLineList[i];
				snapLineList.removeAt(i);
			}
		}
		else
		{
			snapLineList[i]->activate();
			snapLinePoints.append(nearestPoint);

		}
	}

}

// Change the tool
void GLEDrawingArea::setTool(int newTool)
{
	currentTool = newTool;

	// If we're not using a pointer tool, nothing should be selected
	if (currentTool != PointerTool)
	{
		unselectObjects();
	}

	// Start with a clean slate
	initVars();

	update();
	emit updateStatusBar(statusMessages[currentTool][StartPoint]);
}

// Are we in edit mode?
void GLEDrawingArea::setEditMode(bool state)
{
	isEditMode = state;
}

// Handle key presses
void GLEDrawingArea::hitKey(int key, int modifiers)
{
	if (modifiers != Qt::NoModifier)
		return;

	switch(key)
	{
		// If we get an escape key, stop drawing
		case Qt::Key_Escape:
			if (objectSelected)
			{
				if (selectionMode == SelectionScale)
				{
					selectionMode = SelectionMove;
					emit updateStatusBar(statusMessages[PointerTool][StartPoint]);
					isScaling = false;
					isMoving = false;
					isRotating = false;
					objectMoved = false;
					clearRelativeOsnaps();
					clearPolarSnaps();
					changeLastPoint(QPointF(0.0,0.0));
					createSelectionHandles();
				}
				else {
					unselectObjects();
				}
				update();
			}
			else if (isDrawing == true)
			{
				isDrawing = false;
				processCommand(QGLE::Cancel);
				clearPolarSnaps();
				changeLastPoint(QPointF(0.0,0.0));
				update();
			}
			else
			{
				clearPolarSnaps();
				mainWin->selectPointerTool();
				update();
			}
			break;

		// If we get a delete key and something is selected,
		// delete it.
		case Qt::Key_Delete:
			processCommand(QGLE::DeleteSelected);
			update();
			break;

	}
}

QPointF GLEDrawingArea::snappedPoint(QPointF p, bool ignoreOrtho)
{
	// This function is intended to find the appropriate snapping
	// point (if there is one).

	QPointF orthoPoint;
	bool horiz; // Horizontal ortho?
	polarSnapFound = false;

	osnapIntersectPoints.clear();
	if (snaps[GridSnap])
		snappedPosition = grid->nearestPoint(p);
	else
		snappedPosition = p;

	bool orthoSnap;
	if (ignoreOrtho)
		orthoSnap = false;
	else
		orthoSnap = snaps[OrthoSnap];

	// Find the point where the orthoSnap meets the grid
	// ORTHO SNAP overrides everything else!
	if (orthoSnap && (isDrawing || isMoving || isScaling))
	{
		double upDist = QGLE::distance(p,lastPoint)*sin(QGLE::angleBetweenTwoPoints(p,lastPoint));
		double acrossDist = QGLE::distance(p,lastPoint)*cos(QGLE::angleBetweenTwoPoints(p,lastPoint));
		if (fabs(upDist) > fabs(acrossDist))
		{
			horiz = false;
			orthoPoint = lastPoint - QPointF(0.0,upDist);
		}
		else
		{
			horiz = true;
			orthoPoint = lastPoint - QPointF(acrossDist,0.0);
		}

		if (snaps[GridSnap])
		{
			if (horiz)
				orthoPoint.setX(snappedPosition.x());
			else
				orthoPoint.setY(snappedPosition.y());
		}
		else if (snaps[ObjectSnap])
		{
			// Where does the ortho line cross any objects - save regardless
			// of whether we're close enough

			if (thereAreObjects())
			{
				for(int i=0;i<objectList.size();i++)
				{
					if (objectList[i]->isSelected())
						continue;

					if ((i != currentIndex) || (!isDrawing))
					{
						if (horiz)
						{
							if (p.x() > lastPoint.x())
								osnapIntersectPoints += objectList[i]->intersections(lastPoint,
										QGLE::degreesToRadians(0.0));
							else
								osnapIntersectPoints += objectList[i]->intersections(lastPoint,
										QGLE::degreesToRadians(180.0));
						}
						else
						{
							if (p.y() > lastPoint.y())
								osnapIntersectPoints += objectList[i]->intersections(lastPoint,
										QGLE::degreesToRadians(90.0));
							else
								osnapIntersectPoints += objectList[i]->intersections(lastPoint,
										QGLE::degreesToRadians(270.0));
						}
					}
				}

				QPointF osnapPoint;
				QPointF closestPoint;
				double dist;
				double shortest_distance = 1e6;
				foreach(osnapPoint, osnapIntersectPoints)
				{
					dist = QGLE::distance(osnapPoint, orthoPoint);
					if (dist < shortest_distance)
					{
						shortest_distance = dist;
						closestPoint = osnapPoint;
					}
				}
				if (shortest_distance < MAX_OSNAP_DISTANCE)
				{
					if (snaps[PolarTrack] &&
							(currentTool != NoTool) &&
							((currentTool != PointerTool) || isScaling || isMoving) &&
							(currentTool != TanLineTool) &&
							(currentTool != PerpLineTool))
						createPolarSnaps(closestPoint);
					return(closestPoint);
				}
			}

		}

		return(orthoPoint);
	}
	else
	{
		// Either orthosnap is off or we're not yet drawing, grid comes first!
		if (snaps[GridSnap])
			return(snappedPosition);

		// Now we'll try polar snap
		if (snaps[PolarSnap])
		{
			SnapLine *snap;
			if (snaps[ObjectSnap])
			{
				if (thereAreObjects())
				{
					int i;
					for(i=0;i<objectList.size();i++)
					{
						if (objectList[i]->isSelected())
							continue;

						if ((i != currentIndex) || (!isDrawing))
						{
							foreach(snap,snapLineList)
							{
								if (snap->isActive())
								{
									osnapIntersectPoints += objectList[i]->intersections(
											snap->getQtPoint(SnapLine::StartPoint),
											snap->getQtAngle(SnapLine::Angle));
								}
							}
						}
					}

					// Look for intersections between multiple snap lines:
					for(i=0;i<snapLineList.size()-1;i++)
					{
						for(int j=i+1;j<snapLineList.size();j++)
						{
							if (snapLineList[i]->isActive() || snapLineList[j]->isActive())
							{
								osnapIntersectPoints += snapLineList[i]->intersections(
										snapLineList[j]->getQtPoint(SnapLine::StartPoint),
										snapLineList[j]->getQtAngle(SnapLine::Angle));
							}
						}
					}

					QPointF osnapPoint;
					QPointF closestPoint;
					double dist;
					double shortest_distance = 1e6;
					foreach(osnapPoint, osnapIntersectPoints)
					{
						dist = QGLE::distance(osnapPoint, p);
						if (dist < shortest_distance)
						{
							shortest_distance = dist;
							closestPoint = osnapPoint;
						}
					}
					if (shortest_distance < MAX_OSNAP_DISTANCE)
					{
						polarSnapFound = true;
//						if (isDrawing)
//							createPolarSnaps(closestPoint);
						return(closestPoint);
					}
				}
			}
			// One can now assume that we're not close to the osnapIntersectPoints
			// Therefore, see how close we are to the snapLines
			double dist;
			double shortest_distance = 1e6;
			QPointF closestPoint;
			QPointF pt;
			foreach(snap, snapLineList)
			{
				dist = snap->distanceToPoint(p,&pt);
				if (dist < shortest_distance)
				{
					shortest_distance = dist;
					closestPoint = pt;
				}
			}
			if (shortest_distance < MAX_OSNAP_DISTANCE)
			{
				polarSnapFound = true;
				return(closestPoint);
			}

		}

		if (snaps[ObjectSnap])
		{
			// If the grid snap is off, we move onto OSNAP.  This is where it gets interesting!
			// Let the paintEvent cope with drawing the handles, just work out which
			// is closest
			double dist;
			double shortest_distance = 1e6;
			QPointF closestPoint;
			QPointF point;
			if (thereAreObjects())
			{
				for(int i=0;i<objectList.size();i++)
				{
					if (objectList[i]->isSelected())
						continue;

					if ((i != currentIndex) || (!isDrawing))
					{
						dist = objectList[i]->nearestOSnap(p,&point);
						if (dist < shortest_distance)
						{
							shortest_distance = dist;
							closestPoint = point;
						}
					}
				}

				if (shortest_distance < MAX_OSNAP_DISTANCE)
				{
					if (snaps[PolarTrack] &&
							(currentTool != NoTool) &&
							((currentTool != PointerTool) || isScaling || isMoving) &&
							(currentTool != TanLineTool) &&
							(currentTool != PerpLineTool))
						createPolarSnaps(closestPoint);
					return(closestPoint);
				}

				// If the standard handle tests fail, try the nearest point on the object
				for(int i=0;i<objectList.size();i++)
				{
					if (objectList[i]->isSelected())
						continue;

					if ((i != currentIndex) || (!isDrawing))
					{
						dist = objectList[i]->distanceToPoint(p,&point);
						if (dist < shortest_distance)
						{
							shortest_distance = dist;
							closestPoint = point;
						}
					}
				}

				if (shortest_distance < MAX_OSNAP_DISTANCE)
					return(closestPoint);

			}
		}


	}

	// If nothing else has come up with a better option, return the point unchanged
	return(p);
}

bool GLEDrawingArea::checkDontFind(const QPointF& p, QPointF* dontFind)
{
	if (dontFind == NULL) return true;
	else return p != *dontFind;
}

QPointF GLEDrawingArea::snappedPointFromObject(QPointF p, QPointF* dontFind)
{
	double dist;
	double shortest_distance = 1e6;
	QPointF closestPoint;
	QPointF point;
	if (objectSelected)
	{
		for(int i = 0;i < objectList.size(); i++)
		{
			if (objectList[i]->isSelected())
			{
				dist = objectList[i]->nearestOSnap(p,&point);
				if (dist < shortest_distance && checkDontFind(point, dontFind))
				{
					shortest_distance = dist;
					closestPoint = point;
				}
			}
		}

		if (shortest_distance < MAX_OSNAP_DISTANCE) return closestPoint;

		for(int i = 0; i < handleList.size(); i++)
		{
			dist = QGLE::distance(p, handleList[i]);
			if (dist < shortest_distance && checkDontFind(handleList[i], dontFind))
			{
				shortest_distance = dist;
				closestPoint = handleList[i];
			}
		}

		if (shortest_distance < MAX_OSNAP_DISTANCE) return closestPoint;

		// If the standard handle tests fail, try the nearest point on the object
		for(int i = 0; i < objectList.size(); i++)
		{
			if (objectList[i]->isSelected())
			{
				dist = objectList[i]->distanceToPoint(p,&point);
				if (dist < shortest_distance && checkDontFind(point, dontFind))
				{
					shortest_distance = dist;
					closestPoint = point;
				}
			}
		}

		if (shortest_distance < MAX_OSNAP_DISTANCE) return closestPoint;
	}
	return p;
}

void GLEDrawingArea::processCommand(int command, QPointF pt)
{
	// pt is in Qt coordinates and is unsnapped
	switch(command)
	{
		case QGLE::StartNewLineWithPoint:
			createNewLine(pt);
			break;

		case QGLE::SetLineEndPoint:
			setLineEndPoint(pt);
			break;

		case QGLE::EndNewLine:
			completeNewLine();
			break;

		case QGLE::StartNewTanLineWithPoint:
			createNewTanLine(pt);
			break;

		case QGLE::EndNewTanLine:
			completeNewTanLine();
			break;

		case QGLE::SetTanLineEndPoint:
			setTanLineEndPoint(pt);
			break;

		case QGLE::StartNewPerpLineWithPoint:
			createNewPerpLine(pt);
			break;

		case QGLE::EndNewPerpLine:
			completeNewPerpLine();
			break;

		case QGLE::SetPerpLineEndPoint:
			setPerpLineEndPoint(pt);
			break;

		case QGLE::StartNewCircleWithCentre:
			createNewCircle(pt);
			break;

		case QGLE::SetCircleEdgePoint:
			setCircleEdgePoint(pt);
			break;

		case QGLE::EndNewCircle:
			completeNewCircle();
			break;

		case QGLE::StartNewEllipseWithCorner:
			createNewEllipse(pt);
			break;

		case QGLE::SetEllipseSecondCorner:
			setEllipseSecondCorner(pt);
			break;

		case QGLE::EndNewEllipse:
			completeNewEllipse();
			break;


		case QGLE::StartNewArcWithPoint:
			createNewArc(pt);
			break;

		case QGLE::SetArcEndPoint:
			setArcEndPoint(pt);
			break;

		case QGLE::SetArcMidPoint:
			setArcMidPoint(pt);
			break;

		case QGLE::EndNewArc:
			completeNewArc();
			break;

		case QGLE::SetAMove:
			setAmove(pt);
			break;

		case QGLE::Cancel:
			cancelCommand();
			break;

		case QGLE::CreateTextObject:
			createNewTextObject(pt);
			break;

		case QGLE::DeleteSelected:
			deleteSelectedObjects();
			break;

	//	default:
			// Do Nothing for now: create an error in future
	}
}

void GLEDrawingArea::setLineEndPoint(QPointF endPoint)
{
	// Double check currentIndex - if something went wrong this avoids a crash
	if (currentIndex < 0 || currentIndex >= objectList.size()) return;
	objectList[currentIndex]->setPoint(GLELine::EndPoint, endPoint);
}

void GLEDrawingArea::setTanLineEndPoint(QPointF endPoint)
{
	QList<QPointF> tanList;
	// Double check currentIndex - if something went wrong this avoids a crash
	if (currentIndex < 0 || currentIndex >= objectList.size()) return;
	objectList[currentIndex]->setPoint(GLELine::EndPoint, endPoint);

	QPointF oldStart = objectList[currentIndex]->getQtPoint(GLELine::StartPoint);

	tanList = objectList[baseObject]->getTangents(endPoint);

	if (tanList.size() > 0)
	{
		QPointF np = oldStart;
		QPointF tp;
		double shortest_distance = 1e6;
		double distance;

		foreach(tp, tanList)
		{
			distance = QGLE::distance(oldStart, tp);
			if (distance < shortest_distance)
			{
				shortest_distance = distance;
				np = tp;
			}
		}

		objectList[currentIndex]->setPoint(GLELine::StartPoint, np);

		for(int i=0;i<objectList.size();i++)
		{
			if (i != currentIndex)
				objectList[i]->addRelativeOSnaps(np);
		}
	}
}

void GLEDrawingArea::completeNewTanLine()
{
	// Here we log what we've done so that it can be undone later
}

void GLEDrawingArea::setPerpLineEndPoint(QPointF endPoint)
{
	QList<QPointF> perpList;
	// Double check currentIndex - if something went wrong this avoids a crash
	if (currentIndex < 0 || currentIndex >= objectList.size()) return;
	objectList[currentIndex]->setPoint(GLELine::EndPoint, endPoint);

	QPointF oldStart = objectList[currentIndex]->getQtPoint(GLELine::StartPoint);

	perpList = objectList[baseObject]->getPerpendiculars(endPoint);

	if (perpList.size() > 0)
	{
		QPointF np = oldStart;
		QPointF tp;
		double shortest_distance = 1e6;
		double distance;

		foreach(tp, perpList)
		{
			distance = QGLE::distance(oldStart, tp);
			if (distance < shortest_distance)
			{
				shortest_distance = distance;
				np = tp;
			}
		}

		objectList[currentIndex]->setPoint(GLELine::StartPoint, np);

		for(int i=0;i<objectList.size();i++)
		{
			if (i != currentIndex)
				objectList[i]->addRelativeOSnaps(np);
		}
	}
}

void GLEDrawingArea::completeNewPerpLine()
{
	// Here we log what we've done so that it can be undone later
}

void GLEDrawingArea::completeNewLine()
{
	// Here we log what we've done so that it can be undone later
}

void GLEDrawingArea::clearRelativeOsnaps()
{
	GLEDrawingObject *obj;
	foreach(obj,objectList)
	{
		obj->clearRelativeOSnaps();
	}
}

GLEObjectBlock* GLEDrawingArea::createNewObjectBlock(GLEObjectDOConstructor* cons)
{
	GLERC<GLEObjectDO> obj = cons->constructObject();
	if (obj->getObjectRepresentation()->getRectangle()->getXMin() != -1.0)
	{
		gleScript->addNewObject(obj.get());
		GLEObjectBlock *newObj = new GLEObjectBlock(dpi, pixmap.size(), this);
		objectList.append(newObj);
		currentIndex = objectList.size() - 1;
		// Make sure the line is updated when the image size changes
		newObj->setGLEObject(obj.get());
		connect(this, SIGNAL(sizeChanged(QSize,double)), newObj, SLOT(setPixmapSize(QSize,double)));
		newObj->setTextProperties();
		newObj->setLineProperties();
		newObj->setShapeProperties();
		return newObj;
	}
	return NULL;
}

void GLEDrawingArea::createNewTextObject(QPointF referencePoint)
{
	GLEText *thisTextObject = new GLEText(dpi, pixmap.size(), this);
	thisTextObject->setGLEObject(gleScript->newGLEObject(GDOText));
	objectList.append(thisTextObject);
	currentIndex = objectList.size() - 1;
	// Make sure the line is updated when the image size changes
	connect(this, SIGNAL(sizeChanged(QSize,double)), objectList[currentIndex], SLOT(setPixmapSize(QSize,double)));
	objectList[currentIndex]->setTextProperties();
	objectList[currentIndex]->setColorProperty();
	objectList[currentIndex]->setPoint(GLEText::ReferencePoint, referencePoint);
	objectList[currentIndex]->setProperty(GLEDrawingObject::Text, tr("X"));
}

void GLEDrawingArea::createNewLine(QPointF startPoint)
{
	QPointF qtPoint;

	// Create a line
	GLELine *currentLine = new GLELine(dpi, pixmap.size(), this);
	// Add it to the array of lines
	objectList.append(currentLine);
	// Get the index
	currentIndex = objectList.size() - 1;

	// Make sure the line is updated when the image size changes
	connect(this, SIGNAL(sizeChanged(QSize,double)),
			objectList[currentIndex], SLOT(setPixmapSize(QSize,double)));

	objectList[currentIndex]->setPoint(GLELine::StartPoint,
			startPoint);
	objectList[currentIndex]->setPoint(GLELine::EndPoint,
			startPoint);

	objectList[currentIndex]->setGLEObject(gleScript->newGLEObject(GDOLine));
	objectList[currentIndex]->setLineProperties();

	// Start drawing
	isDrawing = true;

}

void GLEDrawingArea::createNewTanLine(QPointF startPoint)
{
	// TODO Get the nearest object to the point, see how near it is to
	// the snapped point and whether it has a tangent.  If so, use the
	// nearest point on that object!

	double distance;
	double shortest_distance = 1e6;
	int shortest_index = -1;
	int i;
	// Find the nearest object and the distance to it
	for(i=0;i<objectList.size();i++)
	{
		distance = objectList[i]->distanceToPoint(startPoint);
		//qDebug() << "Distance to line " << i << ": " << distance;
		if (distance < shortest_distance)
		{
			shortest_distance = distance;
			shortest_index = i;
		}
	}

	if (shortest_distance < MAX_SELECT_DISTANCE)
	{
		if (objectList[shortest_index]->hasTangents())
		{
			// Create a line
			GLELine *currentLine = new GLELine(dpi, pixmap.size(), this);
			// Add it to the array of lines
			objectList.append(currentLine);
			// Get the index
			currentIndex = objectList.size() - 1;
			// Remember the object to which we are drawing
			baseObject = shortest_index;

			// Make sure the line is updated when the image size changes
			connect(this, SIGNAL(sizeChanged(QSize,double)),
					objectList[currentIndex], SLOT(setPixmapSize(QSize,double)));

			QPointF qtPoint;

			// Find the nearest point on the object
			objectList[baseObject]->distanceToPoint(startPoint, &qtPoint);

			objectList[currentIndex]->setPoint(GLELine::StartPoint,
					qtPoint);
			objectList[currentIndex]->setPoint(GLELine::EndPoint,
					qtPoint);

			objectList[currentIndex]->setGLEObject(gleScript->newGLEObject(GDOLine));
			objectList[currentIndex]->setLineProperties();

			changeLastPoint(QGLE::absQtToGLE(qtPoint, dpi, pixmap.height()));
			emit updateStatusBar(statusMessages[TanLineTool][EndPoint]);

			// Start drawing
			isDrawing = true;

		}
	}
}

void GLEDrawingArea::createNewPerpLine(QPointF startPoint)
{
	// TODO Get the nearest object to the point, see how near it is to
	// the snapped point and whether it has a perpendicular point.  If so, use the
	// nearest point on that object!

	double distance;
	double shortest_distance = 1e6;
	int shortest_index = -1;
	int i;
	// Find the nearest object and the distance to it
	for(i=0;i<objectList.size();i++)
	{
		distance = objectList[i]->distanceToPoint(startPoint);
		//qDebug() << "Distance to line " << i << ": " << distance;
		if (distance < shortest_distance)
		{
			shortest_distance = distance;
			shortest_index = i;
		}
	}

	if (shortest_distance < MAX_SELECT_DISTANCE)
	{
		if (objectList[shortest_index]->hasPerpendiculars())
		{
			// Create a line
			GLELine *currentLine = new GLELine(dpi, pixmap.size(), this);
			// Add it to the array of lines
			objectList.append(currentLine);
			// Get the index
			currentIndex = objectList.size() - 1;
			// Remember the object to which we are drawing
			baseObject = shortest_index;

			// Make sure the line is updated when the image size changes
			connect(this, SIGNAL(sizeChanged(QSize,double)),
					objectList[currentIndex], SLOT(setPixmapSize(QSize,double)));

			QPointF qtPoint;

			// Find the nearest point on the object
			objectList[baseObject]->distanceToPoint(startPoint, &qtPoint);

			objectList[currentIndex]->setPoint(GLELine::StartPoint,
					qtPoint);
			objectList[currentIndex]->setPoint(GLELine::EndPoint,
					qtPoint);

			objectList[currentIndex]->setGLEObject(gleScript->newGLEObject(GDOLine));
			objectList[currentIndex]->setLineProperties();

			changeLastPoint(QGLE::absQtToGLE(qtPoint, dpi, pixmap.height()));
			emit updateStatusBar(statusMessages[PerpLineTool][EndPoint]);

			// Start drawing
			isDrawing = true;

		}
	}
}

void GLEDrawingArea::createNewArc(QPointF startPoint)
{
	// Create an arc
	GLEArc *currentArc = new GLEArc(dpi, pixmap.size(), this);
	// Add it to the array of objects
	objectList.append(currentArc);
	// Get the index
	currentIndex = objectList.size() - 1;

	// Make sure the line is updated when the image size changes
	connect(this, SIGNAL(sizeChanged(QSize,double)),
			objectList[currentIndex], SLOT(setPixmapSize(QSize,double)));

	objectList[currentIndex]->setPoint(GLEArc::StartPoint,
			startPoint);
	objectList[currentIndex]->setPoint(GLEArc::EndPoint,
			startPoint);
	objectList[currentIndex]->setGLEObject(gleScript->newGLEObject(GDOArc));
	objectList[currentIndex]->setLineProperties();

	// Start drawing
	isDrawing = true;

}

void GLEDrawingArea::setArcEndPoint(QPointF endPoint)
{
	objectList[currentIndex]->setPoint(GLEArc::EndPoint, endPoint);
}

void GLEDrawingArea::setArcMidPoint(QPointF midPoint)
{
	objectList[currentIndex]->setPoint(GLEArc::MidPoint,
			midPoint);
}

void GLEDrawingArea::setAmove(QPointF pt)
{
	if (hasAMove)
	{
		// Delete the object
		delete(objectList[amoveIndex]);

		// Remove the pointer
		objectList.removeAt(amoveIndex);
	}

	GLEAmove *amove = new GLEAmove(dpi, pixmap.size(), this);
	objectList.append(amove);
	amoveIndex = objectList.size() - 1;

	connect(this, SIGNAL(sizeChanged(QSize,double)),
			objectList[amoveIndex], SLOT(setPixmapSize(QSize,double)));

	objectList[amoveIndex]->setPoint(GLEAmove::CentrePoint, pt);

	currentIndex = amoveIndex;

	hasAMove = true;
}

void GLEDrawingArea::completeNewArc()
{
	// Log creation of arc
}

void GLEDrawingArea::createNewCircle(QPointF centrePoint)
{
	// Create a circle
	GLECircle *currentCircle = new GLECircle(dpi, pixmap.size(), this);
	// Add it to the array of objects
	objectList.append(currentCircle);
	// Get the index
	currentIndex = objectList.size() - 1;

	// Make sure the line is updated when the image size changes
	connect(this, SIGNAL(sizeChanged(QSize,double)),
			objectList[currentIndex], SLOT(setPixmapSize(QSize,double)));
	objectList[currentIndex]->setPoint(GLECircle::CentrePoint, centrePoint);
	objectList[currentIndex]->setGLEObject(gleScript->newGLEObject(GDOEllipse));
	objectList[currentIndex]->setLineProperties();
	objectList[currentIndex]->setShapeProperties();

	// Start drawing
	isDrawing = true;
}

void GLEDrawingArea::setCircleEdgePoint(QPointF edgePoint)
{
	objectList[currentIndex]->setPoint(GLECircle::CircumferencePoint, edgePoint);
}

void GLEDrawingArea::completeNewCircle()
{
	// Log creation of circle
}

void GLEDrawingArea::createNewEllipse(QPointF cornerPoint)
{
	// Create a ellipse
	GLEEllipse *currentEllipse = new GLEEllipse(dpi, pixmap.size(), this);
	// Add it to the array of objects
	objectList.append(currentEllipse);
	// Get the index
	currentIndex = objectList.size() - 1;

	// Make sure the line is updated when the image size changes
	connect(this, SIGNAL(sizeChanged(QSize,double)),
			objectList[currentIndex], SLOT(setPixmapSize(QSize,double)));
	objectList[currentIndex]->setPoint(GLEEllipse::BBoxCorner, cornerPoint);
	objectList[currentIndex]->setGLEObject(gleScript->newGLEObject(GDOEllipse));
	objectList[currentIndex]->setLineProperties();
	objectList[currentIndex]->setShapeProperties();

	// Start drawing
	isDrawing = true;


}

void GLEDrawingArea::setEllipseSecondCorner(QPointF cornerPoint)
{
	objectList[currentIndex]->setPoint(GLEEllipse::BBoxCorner, cornerPoint);
}

void GLEDrawingArea::completeNewEllipse()
{
	// Log creation of ellipse
}

void GLEDrawingArea::startMoving(const QPointF& pt)
{
	objectMoved = false;
	isMoving = true;
	basePoint = pt;
	for(int i = 0; i < objectList.size(); i++)
	{
		objectList[i]->setBasePoint(basePoint);
	}
}

void GLEDrawingArea::findScaleOrigin(const QPointF& pt, int handle)
{

	if (scaleOrigin.x() == -1.0)
	{
		GLEDrawingObject* singleSelected = getSingleSelectedObject();
		if (singleSelected != NULL)
		{
			singleSelected->findScaleOrigin(pt, &scaleOrigin, handle);
		}
	}
}

void GLEDrawingArea::selectObjectsSub(const QPointF& pt, GLEDrawingObject* object, bool multiSelect)
{
	if (object->isSelected())
	{
		if (multiSelect)
		{
			object->setSelected(false);
			objectSelected = false;
			for(int i = 0; i < objectList.size(); i++)
			{
				if (objectList[i]->isSelected())
				{
					objectSelected = true;
					selectionMode = SelectionMove;
					emit updateStatusBar(statusMessages[PointerTool][StartPoint]);
					createSelectionHandles();
					break;
				}
			}
		}
		else
		{
			if (selectionMode == SelectionMove)
			{
				startMoving(pt);
			}
			else if (selectionMode == SelectionScale)
			{
				isScaling = true;
				objectMoved = false;
				findScaleOrigin(pt, -1);
				// No origin found or origin is equal to given point (which is illegal)
				if (scaleOrigin.x() == -1.0 || scaleOrigin == pt)
				{
					// origin not yet selected -> use furthest handle
					int max_index = -1;
					double max_distance = 0.0;
					for(int i = 0; i < handleList.size() ;i++)
					{
						double distance = QGLE::distance(pt, handleList[i]);
						if (distance > max_distance)
						{
							max_distance = distance;
							max_index = i;
						}
					}
					if (max_index != -1)
						scaleOrigin = handleList[max_index];
				}
				basePoint = pt;
				grabbedHandle = -1;

				// setBasePoint() copies pointHash to storedPointHash
				for(int i = 0; i < objectList.size(); i++)
					objectList[i]->setBasePoint(scaleOrigin);
			}
		}
	}
	else
	{
		if (!multiSelect)
			unselectObjects();
		objectSelected = true;
		selectionMode = SelectionMove;
		emit updateStatusBar(statusMessages[PointerTool][StartPoint]);
		object->setSelected(true);
		createSelectionHandles();
	}
}

GLEDrawingObject* GLEDrawingArea::getSingleSelectedObject()
{
	int cnt = 0;
	GLEDrawingObject* result = NULL;
	for(int i = 0; i < objectList.size(); i++)
	{
		if (objectList[i]->isSelected())
		{
			cnt++;
			result = objectList[i];
		}
	}
	return cnt == 1 ? result : NULL;
}

void GLEDrawingArea::selectObjects(QPointF pt, bool multiSelect)
{
	// Select the nearest object (unless it's a long way away)
	double shortest_distance = 1e6;
	int shortest_index = -1;
	double distance;
	QPointF point, nearest_point;
	bool has_object = false;

	GLEDrawingObject* singleSelected = getSingleSelectedObject();
	if (singleSelected != NULL && singleSelected->nearestPriorityOSnap(pt, &point) < MAX_SELECT_DISTANCE)
	{
		selectObjectsSub(point, singleSelected, multiSelect);
		has_object = true;
	}

	// Next consider resize handles
	if (!has_object && objectSelected && (selectionMode == SelectionScale || selectionMode == SelectionRotate))
	{
		// Find the distance to the nearest handle
		for(int i = 0; i < handleList.size() ;i++)
		{
			distance = QGLE::distance(pt, handleList[i]);
			if (distance < shortest_distance)
			{
				shortest_distance = distance;
				shortest_index = i;
			}
		}
		if (shortest_distance < MAX_SELECT_DISTANCE)
		{
			grabbedHandle = shortest_index;
			if (selectionMode == SelectionScale)
			{
				isScaling = true;
				basePoint = handleList[grabbedHandle];
				findScaleOrigin(basePoint, grabbedHandle);
				if (scaleOrigin.x() == -1.0 || scaleOrigin == basePoint)
				{
					// origin not yet selected -> use opposite handle
					bool scale;
					scaleOrigin = handleList[oppositeHandleTo(grabbedHandle, &scale)];
				}

				// setBasePoint() copies pointHash to storedPointHash
				for(int i = 0; i < objectList.size(); i++)
					objectList[i]->setBasePoint(scaleOrigin);

				return;
			}
			else if (selectionMode == SelectionRotate)
			{
				// TODO: Set basePoint to centre of selection
				// NOTE: Currently disabled because generating GLE script code
				//       from rotated objects is not yet implemented
				isRotating = true;
				basePoint.setX((handleList[TopLeft].x() + handleList[BottomRight].x())/2);
				basePoint.setY((handleList[TopLeft].y() + handleList[BottomRight].y())/2);
				baseAngle = QGLE::angleBetweenTwoPoints(basePoint, pt);
				lastAngle = baseAngle;

				for(int i=0;i<objectList.size();i++)
					objectList[i]->setBasePoint(basePoint);

				return;
			}
		}
	}

	// Find the nearest object and the distance to it
	shortest_distance = 1e6;
	shortest_index = -1;

	// First consider snap points of already selected objects
	for(int i = 0; i < objectList.size(); i++)
	{
		if (objectList[i]->isSelected())
		{
			distance = objectList[i]->nearestOSnap(pt, &point);
			if (distance <= shortest_distance)
			{
				nearest_point = point;
				shortest_distance = distance;
				shortest_index = i;
			}
		}
	}

	if (shortest_distance < MAX_SELECT_DISTANCE)
	{
		selectObjectsSub(nearest_point, objectList[shortest_index], multiSelect);
		has_object = true;
	}

	// Then consider snap points of objects that are not yet selected
	if (!has_object)
	{
		for(int i = 0; i < objectList.size(); i++)
		{
			if (!objectList[i]->isSelected())
			{
				distance = objectList[i]->nearestOSnap(pt, &point);
				if (distance <= shortest_distance)
				{
					nearest_point = point;
					shortest_distance = distance;
					shortest_index = i;
				}
			}
		}

		if (shortest_distance < MAX_SELECT_DISTANCE)
		{
			selectObjectsSub(nearest_point, objectList[shortest_index], multiSelect);
			has_object = true;
		}
	}

	// Now consider selection handles in case of move mode (in other modes they take precedence)
	if (!has_object && selectionMode == SelectionMove && objectSelected)
	{
		for(int i = 0; i < handleList.size(); i++)
		{
			distance = QGLE::distance(pt, handleList[i]);
			if (distance < shortest_distance)
			{
				shortest_distance = distance;
				shortest_index = i;
			}
		}

		if (shortest_distance < MAX_SELECT_DISTANCE)
		{
			startMoving(handleList[shortest_index]);
			has_object = true;
		}
	}

	// Then consider all points on any object
	if (!has_object)
	{
		for(int i = 0; i < objectList.size(); i++)
		{
			distance = objectList[i]->distanceToPoint(pt, &point);
			if (distance < shortest_distance)
			{
				nearest_point = point;
				shortest_distance = distance;
				shortest_index = i;
			}
		}

		if (shortest_distance < MAX_SELECT_DISTANCE)
		{
			selectObjectsSub(nearest_point, objectList[shortest_index], multiSelect);
			has_object = true;
		}
	}

	if (!has_object && !multiSelect)
	{
		unselectObjects();
	}

	if (objectSelected)
		createSelectionHandles();

	updateSelection();
}

void GLEDrawingArea::updateSelection()
{
	QList<GLEDrawingObject *> selectedObjects;
	GLEDrawingObject *obj;

	foreach(obj, objectList)
	{
		if (obj->isSelected())
			selectedObjects.append(obj);
	}
	emit selectionChanged(selectedObjects);
}

int GLEDrawingArea::oppositeHandleTo(int handle, bool *scale)
{
	int baseHandle = handle;
	int scaling = false;
	switch(handle)
	{
		case TopLeft:
			baseHandle = BottomRight;
			scaling = true;
			break;
		case TopMiddle:
			baseHandle = BottomMiddle;
			scaling = false;
			break;
		case TopRight:
			baseHandle = BottomLeft;
			scaling = true;
			break;
		case BottomLeft:
			baseHandle = TopRight;
			scaling = true;
			break;
		case BottomMiddle:
			baseHandle = TopMiddle;
			scaling = false;
			break;
		case BottomRight:
			baseHandle = TopLeft;
			scaling = true;
			break;
		case MiddleLeft:
			baseHandle = MiddleRight;
			scaling = false;
			break;
		case MiddleRight:
			baseHandle = MiddleLeft;
			scaling = false;
			break;

	}
	if (scale)
		*scale = scaling;

	return(baseHandle);
}

void GLEDrawingArea::cancelCommand()
{
	GLEDrawObject* obj = objectList[currentIndex]->getGLEObject();
	if (obj != NULL) gleScript->cancelObject(obj);
	delete objectList[currentIndex];
	objectList.removeAt(currentIndex);
}

void GLEDrawingArea::deleteSelectedObjects()
{
	if (objectSelected)
	{
		for(int i=objectList.size()-1;i>=0;i--)
		{
			if (objectList[i]->isSelected())
			{
				if (objectList[i]->isAmove())
				{
					hasAMove = false;
					amoveIndex = -1;
				}

				GLEDrawObject* gleobj = objectList[i]->getGLEObject();
				if (gleobj != NULL) gleobj->setFlag(GDO_FLAG_DELETED);

				delete objectList[i];
				objectList.removeAt(i);
			}
		}
		objectSelected = false;
		setDirtyAndSave();
	}
	updateSelection();
}

//! Select given object
void GLEDrawingArea::selectObject(GLEDrawingObject* obj)
{
	unselectObjects();
	obj->setSelected(true);
	objectSelected = true;
	selectionMode = SelectionMove;
	createSelectionHandles();
	updateSelection();
}

void GLEDrawingArea::createSelectionHandles()
{
	int ch, sh, sh2;
	if (!objectSelected)
		return;

	QPainterPath paintPath;

	if (objectSelected)
	{
		GLEDrawingObject *obj;
		foreach(obj, objectList)
		{
			if (obj->isSelected())
				paintPath.addPath(obj->path());
		}
	}

	QRectF bbox = paintPath.boundingRect();

	handleList[TopLeft] = bbox.topLeft();
	handleList[TopRight] = bbox.topRight();
	handleList[BottomLeft] = bbox.bottomLeft();
	handleList[BottomRight] = bbox.bottomRight();

	foreach(ch,CornerHandles)
	{
		handleList[TopMiddle] = (handleList[TopLeft] + handleList[TopRight])/2;
		handleList[BottomMiddle] = (handleList[BottomLeft] + handleList[BottomRight])/2;
		handleList[MiddleLeft] = (handleList[TopLeft] + handleList[BottomLeft])/2;
		handleList[MiddleRight] = (handleList[TopRight] + handleList[BottomRight])/2;

		// Now go through the list of CornerHandles and if they are coincident
		// with the SideHandles, then delete them.

		foreach(ch,CornerHandles)
		{
			foreach(sh, SideHandles)
			{
				// Check whether ch is valid
				if (handleList.contains(ch))
				{
					if (handleList[ch] == handleList[sh])
					{
						// delete cause bug when selecting horiz/vertical lines
						// handleList.remove(ch);
						// qDebug() << "Removed a corner handle";
					}
				}
			}

		}

	}

	// Now check whether any side handles are coincident and
	// if they are, delete them both
	//
	// I'm sure there's a simpler algorithm to achieve this,
	// especially since we're only likely to have coincident
	// TopMiddle & BottomMiddle or MiddleLeft & MiddleRight
	foreach(sh, SideHandles)
	{
		foreach(sh2, SideHandles)
		{
			if (sh == sh2)
				continue;
			else
			{
				if (handleList.contains(sh) && handleList.contains(sh2))
				{
					if (handleList[sh] == handleList[sh2])
					{
						// handleList.remove(sh);
						// handleList.remove(sh2);
						// qDebug() << "Removed a side handle";
					}
				}
			}
		}
	}
}

void GLEDrawingArea::drawSelectionHandles(QPainter *p)
{
	QPointF pt;
	QPen pen = QGLE::handlePen();
	foreach(pt, handleList)
	{
		QGLE::drawBox(p, pt, HANDLE_SIZE, pen);
	}
}

void GLEDrawingArea::drawScaleHandles(QPainter *p)
{
	QPen pen = QGLE::handlePen();
	QGLE::drawScaleHandle(p, handleList[TopLeft],     -1, -1, HANDLE_SIZE, pen);
	QGLE::drawScaleHandle(p, handleList[TopRight],    +1, -1, HANDLE_SIZE, pen);
	QGLE::drawScaleHandle(p, handleList[BottomLeft],  -1, +1, HANDLE_SIZE, pen);
	QGLE::drawScaleHandle(p, handleList[BottomRight], +1, +1, HANDLE_SIZE, pen);
	QGLE::drawBox(p, handleList[TopMiddle],    HANDLE_SIZE, pen);
	QGLE::drawBox(p, handleList[BottomMiddle], HANDLE_SIZE, pen);
	QGLE::drawBox(p, handleList[MiddleLeft],   HANDLE_SIZE, pen);
	QGLE::drawBox(p, handleList[MiddleRight],  HANDLE_SIZE, pen);
}

void GLEDrawingArea::drawRotationHandles(QPainter *p)
{
	QPen pen = QGLE::handlePen();
	QGLE::drawRotator(p, handleList[TopLeft],     TopLeft,     HANDLE_SIZE, pen);
	QGLE::drawRotator(p, handleList[TopRight],    TopRight,    HANDLE_SIZE, pen);
	QGLE::drawRotator(p, handleList[BottomLeft],  BottomLeft,  HANDLE_SIZE, pen);
	QGLE::drawRotator(p, handleList[BottomRight], BottomRight, HANDLE_SIZE, pen);
}

void GLEDrawingArea::deleteSelectionHandles()
{
	handleList.clear();
}

void GLEDrawingArea::renderPostscript(const char* ps, const GLERectangle& rect, double dpi, QImage* result)
{
	mainWin->renderPostscript(ps, rect, dpi, result);
}
