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

#ifndef _GLEDRAWINGOBJECT_H
#define _GLEDRAWINGOBJECT_H

#include <math.h>
#include "component.h"
#include "snapline.h"
#include "qgle_definitions.h"
#include "qgle_statics.h"
#include "gledrawing.h"

struct PropertyDescriptor
{
	QString description;
	QVariant::Type type;
	QVariant defaultValue;
	QVariant validValues;
};

//! GLEDrawingObject class: base class for all drawing objects
class GLEDrawingObject : public GLEComponent
{
	Q_OBJECT

	//! Allow daughters access to private parts (so to speak)
	friend class GLELine;
	friend class GLECircle;
	friend class GLEEllipse;
	friend class GLEArc;
	friend class GLEAmove;
	friend class GLEPolarArray;
	friend class GLEText;
public:
	//! The class constructor
	GLEDrawingObject(double resolution, QSize imageSize, GLEDrawingArea* area);

	enum TransformModes
	{
		TransformModeNone,
		TransformModeConstrained,
		TransformModeFree
	};

	//! The list of all valid properties that any object can have
	enum Properties
	{
		// Common to most objects
		FillColour,
		LineColour,

		LineWidth,
		LineStyle,

		// For text objects
		Text,
		FontName,
		FontStyle,
		FontSize,
		Alignment,
		RefPointName,

		// Centres for arcs and circles?
		CircleCentre,

		// Centre of the bounding box
		ObjectCentre,

		// Lines
		ArrowHeadPresence,
		ArrowHeadStyle,
		ArrowHeadSize,
		StartPoint,
		EndPoint,

		// Arcs
		ArcStartAngle,
		ArcEndAngle,

		// Arcs & Circles
		Radius,

		// Ellipses
		MajorAxisRadius,
		MinorAxisRadius,
		MajorAxisAngle,

		// May change this to be "size"
		Width,
		Height,

		NumberOfProperties // This will hold the number of valid properties
			// e.g. for(i=0;i<NumberOfProperties;i++)
			//      {
			//          // Process Property i
			//      }
	};

	static const int PropertyUser = 100;

	enum TextAlignment
	{
		CC, TR, TL, BR, BL, LC, RC, TC, BC, LEFTTEXT, RIGHTTEXT, CENTRETEXT
	};


	//! Virtual function to determine whether we're inside an object
	virtual bool isInside(QPointF p) = 0;
	virtual QList<QPointF> getTangents(QPointF p) = 0;
	virtual bool hasTangents() = 0;

	virtual QList<QPointF> getPerpendiculars(QPointF p) = 0;
	virtual bool hasPerpendiculars() = 0;

	//! Scaling
	void linearTransformPt(int pt, const GLELinearEquation& ex, const GLELinearEquation& ey, bool update = true);
	virtual int supportedTransformMode();
	virtual bool findScaleOrigin(const QPointF& pt, QPointF* origin, int handle);
	virtual void linearTransform(const GLELinearEquation& ex, const GLELinearEquation& ey);
	virtual void linearTransformDone();

	//! Moving and rotating
	virtual void moveBy(QPointF offset) = 0;
	virtual void rotateBy(double radians) = 0;

	//! Set the reference point of, e.g., text objects or GLE object blocks (tl,bl,tc,bc,tr,br,...)
	virtual bool setReferencePoint(const QPointF& pt);

	//! Update distance to point given a new point of the object
	static void distanceToPointUpdate(double x, double y, const QPointF& p, double* minDist, QPointF *nearestPoint);

	QList<SnapLine *> getOTracks();

	//! Check whether the object is an amove
	bool isAmove();

	//! Function to draw the standard osnap handles
	void drawOSnaps(QPainter *p);

	//! Function to determine the distance to the nearest priority osnap handle (e.g., scale handles for arc)
	virtual double nearestPriorityOSnap(const QPointF& pt, QPointF *osnap);

	//! Function to determine the distance to the nearest standard osnap handle
	virtual double nearestOSnap(const QPointF& p, QPointF *osnap);

	//! Function to set a given property to a given value
	void setProperty(int propertyIndex, QVariant value);
	void setPropertyNoUpdate(int propertyIndex, QVariant value);
	//! Function to get the value of a property
	QVariant getProperty(int propertyIndex);

	//! Make this object selected
	void setSelected(bool sel);

	//! Is this object selected?
	bool isSelected();

	void clearOTracks();
	virtual void createOTracks() = 0;

	QPair<double, QPointF> updateOTracks(QPointF cursorPosition);

	//! Function to add osnaps relative to a given point
	virtual void addRelativeOSnaps(QPointF p) = 0;
	void clearRelativeOSnaps();
	QList< QPair<QPointF,int> > getRelativeOSnaps();

	inline GLEDrawObject* getGLEObject() { return gleObject.get(); }
	inline void setGLEObject(GLEDrawObject* obj) { gleObject = obj; }

	inline QSet<int> getValidProperties() { return validProperties; }
	PropertyDescriptor getPropertyDescription(int property);

	//! Set text properties
	void setTextProperties();
	//! Set color property
	void setColorProperty();
	//! Set line properties
	void setLineProperties();
	//! Set shape properties
	void setShapeProperties();
	//! Set pen properties for given GLE object
	void setSimplePenProperties(QPen& pen);
	//! Set pen properties for given GLE object
	void setPenProperties(QPen& pen);
	//! Return GLEInterface
	inline GLEInterface* getGLEInterface() { return drawingArea->getGLEInterface(); }
public slots:
	virtual void imageRendered(QImage newImage);

signals:
	void propertyChanged(int propertyIndex);

protected:
	//! Contains the standard osnap handles
	QList< QPair<QPointF,int> > osnapHandles;

	//! Contains the relative osnap points
	QList< QPair<QPointF,int> > relativeOSnaps;

	//! The list of properties for the object
	QHash<int, QVariant> properties;
	QHash<int, PropertyDescriptor> propertyDescriptions;
	void initialiseProperties();
	//! The set of properties that have a meaning for this object
	QSet<int> validProperties;

	//! Is the object selected?
	bool selected;

	//! Is this an amove?
	bool amove;

	QList<SnapLine *> otrackList;

	QPointF scaleOrigin;

	GLERC<GLEDrawObject> gleObject;

	QHash<QString, QString> fontList;
	QList<QVariant> getFontList();
	QList<QVariant> getFontStyles();

	GLEDrawingArea* drawingArea;
};

#endif
