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

#ifndef __PROPERTYEDITOR_H
#define __PROPERTYEDITOR_H

#include <QtGui>
#include "drawingobject.h"
#include "propertymodel.h"

class GLEPropertyEditor : public QTableView
{
	Q_OBJECT

public:
	//! Constructor
	GLEPropertyEditor(QWidget *parent, int size);
	// virtual QSize sizeHint() const;
	// virtual int heightForWidth(int w) const;
public slots:
	void objectsSelected(QList<GLEDrawingObject *> objs);
	void propertyChanged(const QModelIndex &index, const QModelIndex &index2);

signals:
	void propertiesHaveChanged();


private:
	QVariant getPropertyValue(int index, QList<GLEDrawingObject *> objs);
	QList<GLEDrawingObject *> objects;
	bool hasModel;
	GLEPropertyModel *model;
	int defaultSize;
};

#endif
