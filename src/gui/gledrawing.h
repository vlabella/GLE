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

#ifndef __GLEDRAWING_H
#define __GLEDRAWING_H

#include <QtGui>
#include "grid.h"
#include "snapline.h"
#include "../gle/gle-interface/gle-interface.h"

class GLEMainWindow;
class GLEDrawingObject;
class GLEObjectBlock;

//! Class implementing the drawing area for displaying GLE drawings
/*!
 * This class controls the paintable area of the window.  For the purposes of
 * the initial previewer, it is largely unnecessary, as all that is required
 * is for the picture to be drawn.  However, for future versions, there will
 * be additional usage, including user interaction and this will be cleaner
 * if implemented in a separate class.
 */
class GLEDrawingArea : public QWidget
{
	Q_OBJECT

public:
	//! Constructor
	GLEDrawingArea(GLEInterface *iface, GLEMainWindow* main);

	//! Enumerator describing the drawing tools
	enum DrawingTools
	{
		NoTool,
		PointerTool,
		LineTool,
		TanLineTool,
		PerpLineTool,
		CircleTool,
		EllipseTool,
		ArcTool,
		TextTool,
		AMoveTool
	};

	enum Handles
	{
		TopLeft,
		TopMiddle,
		TopRight,
		MiddleLeft,
		MiddleRight,
		BottomLeft,
		BottomMiddle,
		BottomRight
	};


	//! Enumerator used to list the modifiers that we use
	enum
	{
		ShiftKey,
		AltKey,
		CtrlKey
	};

	enum Snaps
	{
		ObjectSnap,
		PolarSnap,
		PolarTrack,
		OrthoSnap,
		GridSnap
	};

	//! Return the current resolution
	double getDPI();

	//! Has the image been changed
	bool isDirty();
	//! We've saved, so clear the dirty flag
	void clearDirty();
	//! Set the dirty flag
	void setDirty();
	// To allow us to notice whether changes have occurred
	void setDirtyAndSave();
	//! Objects have been rendered
	void clearNewObjectsFlag();
	void setNewObjectsFlag();
	//! Are there new objects since the last render?
	bool thereAreNewObjects();
	//! Are there any objects at all?
	bool thereAreObjects();

	//! We've opened a new file, so clear the objects
	void clearObjects();

	//! Handle key events
	void hitKey(int key, int modifiers);

	//! Convert GLE point to Qt point
	QPointF gleToQt(GLEPoint& pt);

	//! Set line properties
	void setLineProperties(int currentIndex, GLEDrawObject* obj);
	//! Create new line object based on GLE object
	void createNewLineFromScript(GLELineDO* line);
	//! Create new ellipse object based on GLE object
	void createNewEllipseFromScript(GLEEllipseDO* ellipse);
	//! Create new arc object based on GLE object
	void createNewArcFromScript(GLEArcDO* arc);
	//! Create new text object based on GLE object
	void createNewTextFromScript(GLETextDO* text);
	//! Create new object block based on GLE object
	void createNewObjectFromScript(GLEObjectDO* obj);
	//! Create new object block
	GLEObjectBlock* createNewObjectBlock(GLEObjectDOConstructor* cons);
	//! Select given object
	void selectObject(GLEDrawingObject* obj);
	//! Create objects from given GLE script
	void updateFromScript(GLEScript* script);
	//! Render Postscript code to QImage
	void renderPostscript(const char* ps, const GLERectangle& rect, double dpi, QImage* result);
	//! Initialize all instance variables
	void initAll();
	//! Initialize transient instance variables
	void initVars();
	//! Initialize instance variables related to selection
	void initSelectionVars();
	//! Return GLEInterface
	inline GLEInterface* getGLEInterface() { return gleInterface; }
protected:
	//! Automatically called when repainting is needed
	void paintEvent(QPaintEvent *event);
	//! Automatically called when a mouse button is pressed
	void mousePressEvent(QMouseEvent *event);
	//! Automatically called when the mouse is moved
	void mouseMoveEvent(QMouseEvent *event);
	//! Automatically called when a mouse button is released
	void mouseReleaseEvent(QMouseEvent *event);

public slots:
	//! Slot used to update the display when GS is finished
	void updateDisplay(QImage image);
	//! Slot used to toggle the visibility of the grid
	void gridToggle(bool state);
	//! Slot used to toggle the grid snap
	void gridSnapToggle(bool state);
	//! Slot used to update the resolution of the drawing
	void setDPI(double new_dpi);
	//! Slot used to change the grid X spacing
	void setGrid(QPointF newGrid);
	//! Slot used to select a new drawing tool
	void setTool(int newTool);
	//! Slot used to select a mode
	void setEditMode(bool state);
	//! Slot used to toggle orthosnap
	void orthoSnapToggle(bool state);
	//! Slot used to toggle osnap
	void osnapToggle(bool state);
	//! Slot used to toggle polar snap
	void polarSnapToggle(bool state);
	//! Slot used to toggle polar tracking
	void polarTrackToggle(bool state);
	//! Slot used to handle keyboard modifiers
	void modifierToggle(int mod, bool state);
	//! Slot used to update the polar start angle
	void setPolarSnapStartAngle(double newAngle);
	//! Slot used to update the polar increment angle
	void setPolarSnapIncAngle(double newAngle);

signals:
	//! Notify children of change of resolution
	void dpiChanged(double new_dpi);
	//! Notify children of change of image size
	void sizeChanged(QSize new_size, double new_dpi);
	//! Notify parent's status bar of mouse move events
	void mouseMoved(QPointF gle);
	//! Notify parent of base point
	void basePointChanged(QPointF qt);
	//! Update status bar as things are done
	void updateStatusBar(QString msg);
	//! Notify of changes to the dirty flag
	void dirtyFlagChanged(bool state);
	//! Disable tools
	void disableTools();
	//! The selection has changed: tell the property editor
	void selectionChanged(QList<GLEDrawingObject *> newSelection);
	//! Render an image please!
	void renderPostscript(QString psCode, double dpi, GLEDrawingObject *obj);

private:
	//! The rendered pixmap
	QPixmap pixmap;
	//! The visibility of the grid
	bool gridVisible;
	//! The grid spacing
	QPointF gridSpacing;
	//! The resolution
	double dpi;

	//! Hash holding modifier states
	QHash<int,bool> modifier;

	//! The Grid
	GLEGrid *grid;

	//! Choose a point based on all the snaps
	QPointF snappedPoint(QPointF p, bool ignoreOrtho = false);

	QPointF snappedPointFromObject(QPointF p, QPointF* dontFind = NULL);

	//! Check that "p" is not equal to "dontFind"
	bool checkDontFind(const QPointF& p, QPointF* dontFind);

	//! Change the base point
	void changeLastPoint(QPointF p);

	//! Unselect all objects
	void unselectObjects();

	//! Get maximum supported transform mode
	int minimumSupportedTransformMode();

	//! Perform scaling of objects
	void performScale(const QPointF& snappedPosition);

	//! Inform objects that we are done with the transformation (they can re-render themselves now)
	void linearTransformDone();

	QPointF lastPoint;
	QList<QPointF> osnapIntersectPoints;

	//! Are any objects selected?
	bool objectSelected;

	//! The current cursor position
	QPointF cursorPosition;

	//! The nearest grid point (equal to the above if no grid snap)
	QPointF snappedPosition;

	//! What snaps are activated?
	QHash<int, bool> snaps;

	//! The index of the current line in the line list
	int currentIndex;

	bool polarSnapFound;

	//! The current tool being used
	int currentTool;

	//! The list of lines
	QList<GLEDrawingObject *> objectList;

	//! The list of snap lines
	QList<SnapLine *> snapLineList;
	//! The list of points for the snap line list
	QList<QPointF> snapLinePoints;

	void createPolarSnaps(QPointF qtPoint);
	void clearPolarSnaps();
	void updateSnapLines(QPointF qtPoint);

	//! A list of status bar messages
	QHash<int, QHash<int, QString> > statusMessages;
	//! Is it present?
	bool hasAMove;
	//! What is the index in the list of objects
	int amoveIndex;

	//! Are we drawing?
	bool isDrawing;

	//! Are we in an editing mode
	bool isEditMode;

	//! Has the drawing been changed
	bool dirty;
	//! Has it been changed since the last render
	bool newObjects;

	//! A record of the parent
	GLEMainWindow* mainWin;

	//! Enumerator used to describe what we're drawing (for status bar messages)
	enum
	{
		StartPoint,
		MidPoint,
		EndPoint,
		CircumferencePoint
	};

	double polarStartAngle;
	double polarAngleIncrement;

	void processCommand(int command, QPointF pt = QPointF(0.0,0.0));
	void clearRelativeOsnaps();
	void completeNewLine();
	void setLineEndPoint(QPointF endPoint);
	void createNewLine(QPointF startPoint);
	void completeNewTanLine();
	void setTanLineEndPoint(QPointF endPoint);
	void createNewTanLine(QPointF startPoint);
	void completeNewPerpLine();
	void setPerpLineEndPoint(QPointF endPoint);
	void createNewPerpLine(QPointF startPoint);
	void createNewCircle(QPointF centrePoint);
	void setCircleEdgePoint(QPointF edgePoint);
	void completeNewCircle();
	void createNewEllipse(QPointF cornerPoint);
	void createNewTextObject(QPointF referencePoint);
	void setEllipseSecondCorner(QPointF cornerPoint);
	void completeNewEllipse();
	void createNewArc(QPointF startPoint);
	void setArcEndPoint(QPointF endPoint);
	void setArcMidPoint(QPointF midPoint);
	void completeNewArc();
	void setAmove(QPointF pt);
	void cancelCommand();
	void deleteSelectedObjects();
	GLEDrawingObject* getSingleSelectedObject();
	void selectObjects(QPointF pt, bool multiSelect);
	void startMoving(const QPointF& pt);
	void findScaleOrigin(const QPointF& pt, int handle);
	//! Set the reference point of, e.g., text objects or GLE object blocks (tl,bl,tc,bc,tr,br,...)
	void setReferencePoint(const QPointF& pt);
	void selectObjectsSub(const QPointF& pt, GLEDrawingObject* object, bool multiSelect);
	void updateSelection();

	void createSelectionHandles();
	void deleteSelectionHandles();
	void drawSelectionHandles(QPainter *p);
	void drawScaleHandles(QPainter *p);
	void drawRotationHandles(QPainter *p);
	bool isSelectedObjectSnap;
	bool isScaling;
	bool isMoving;
	bool isRotating;
	bool objectMoved;
	int selectionMode;
	double baseAngle;
	double lastAngle;
	//! The list of handles
	QHash<int, QPointF> handleList;
	QList<int> CornerHandles;
	QList<int> SideHandles;
	QPointF basePoint;
	QPointF scaleOrigin;

	QList<QPointF> extraPoints;
	GLEScript* gleScript;

	int baseObject;

	enum SelectionModes
	{
		SelectionMove,
		SelectionScale,
		SelectionRotate
	};

	int oppositeHandleTo(int handle, bool *scale = 0);

	int grabbedHandle;

	// Pointer to the GLE interface
	GLEInterface *gleInterface;
};


#endif
