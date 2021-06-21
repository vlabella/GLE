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

#ifndef __OBJECTBLOCKS_H
#define __OBJECTBLOCKS_H

#include "drawingobject.h"

class GLEMainWindow;

class GLEObjectBlocksList : public QTreeView
{
	Q_OBJECT

public:
	//! Constructor
	GLEObjectBlocksList(QWidget *parent, GLEMainWindow* main, GLEDrawingArea* area, int size);
	void updateList();
	void updateFile(QStandardItem *parent, GLESourceFile* file);
	// virtual QSize sizeHint() const;
	// virtual int heightForWidth(int w) const;
public slots:
	void selectEntry(const QModelIndex&);
protected:
	int defaultSize;
	GLEMainWindow* mainWin;
	GLEDrawingArea* drawingArea;
	QStandardItemModel* model;
	GLERCVector<GLEObjectDOConstructor> consList;
};

//! Class describing a text drawing object
class GLEObjectBlock : public GLEDrawingObject
{
	Q_OBJECT

public:

	//! Enumeration of the points needed to draw a text object
	enum Points
	{
		ReferencePoint,
		ObjectWidth,
		ObjectHeight
	};

	//! Constructor used for initialising variables and connections
	GLEObjectBlock(double resolution, QSize imageSize, GLEDrawingArea* area);
	//! Create the list with supported properties
	void createProperties();
	//! Draw the text on the provided painter
	void draw(QPainter *p);
	//! Return the shortest distance between a given point and the text object
	double nearestOSnap(const QPointF& p, QPointF *osnap);
	double distanceToPoint(const QPointF& p, QPointF *nearestPoint);
	void distanceToPointRec(GLEObjectRepresention* obj, const GLEPoint& orig, const QPointF& p, double* minDist, QPointF *nearestPoint);
	//! Set the reference point (tl,bl,tc,bc,tr,br,...)
	bool setReferencePoint(const QPointF& pt);
	bool setReferencePointRec(const QString& ref, GLEObjectRepresention* obj, const GLEPoint& orig, const QPointF& p, double* minDist);
	bool setReferencePointUpdate(double x, double y, const char* suff, const QString& ref, const QPointF& p, double* minDist);
	//! Set one of the enumerated points
	void setPoint(int pointChoice, const QPointF& p, bool update = true);
	void addRelativeOSnaps(QPointF p);
	void renderPostscript();
	void renderPostscriptNoUpdate();

	//! Find points where a line intersects the text string (there are none)
	// If vertical = true, m contains 'x'
	QList<QPointF> intersections(double qtm, double qtc, bool vertical = false);
	QList<QPointF> intersections(QPointF qtp1, double angle);
	QList<QPointF> intersections(QPointF qtp1, QPointF qtp2);
	QList<QPointF> getTangents(QPointF p);
	bool hasTangents();
	QList<QPointF> getPerpendiculars(QPointF p);
	bool hasPerpendiculars();

	//! Is the point inside the object (for a text string, always false)
	bool isInside(QPointF p);
	int supportedTransformMode();
	void linearTransform(const GLELinearEquation& ex, const GLELinearEquation& ey);
	void linearTransformDone();
	void moveBy(QPointF offset);
	void rotateBy(double radians);
	void createOTracks();
	void computeInitialPosition();
	void updateFromPropertyNoDirty(int property);
	//! Update the painter path
	void updateObjectScale(double sx, double sy);

public slots:
	void resChanged();
	//! Update the painter path when the resolution or start/end points change
	void updateObject();

private slots:
	void updateFromProperty(int property);

private:
	QPointF scaleOffs;
	QPixmap origImage;
	QPixmap image;
	QPointF drawPoint;
	GLEPoint referenceOffset;
	double lastdpi;
};


#endif
