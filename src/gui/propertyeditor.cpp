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

#include "propertyeditor.h"
#include "variantdelegate.h"
#include <QtDebug>
#include "qgle_statics.h"
#include "drawingobject.h"
#include "circle.h"


GLEPropertyEditor::GLEPropertyEditor(QWidget *parent, int size)
	: QTableView(parent)
{
	defaultSize = size;
	hasModel = false;
	// setSizeIncrement(1, 1);
	// setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

void GLEPropertyEditor::propertyChanged(const QModelIndex &index, const QModelIndex&)
{
	if (hasModel)
	{
		GLEDrawingObjectProperty prop;
		prop = model->getFullProperty(index);
		GLEDrawingObject *obj;
		foreach(obj, objects)
		{
			obj->setProperty(prop.index, prop.value);
		}

		emit propertiesHaveChanged();

	}
}

void GLEPropertyEditor::objectsSelected(QList<GLEDrawingObject *> objs)
{
	objects = objs;

	bool first = true;
	GLEDrawingObject *obj;

	QSet<int> propertySet;
	foreach(obj, objs)
	{
		if (first)
		{
			propertySet = obj->getValidProperties();
		}
		else
		{
			first = false;
			// Better to use union?
			propertySet.intersect(obj->getValidProperties());
		}
	}

	// propertySet now contains a list of properties that are
	// common to each object
	QList<GLEDrawingObjectProperty> properties;

	PropertyDescriptor thisProperty;
	GLEDrawingObjectProperty prop;

	if (objs.size() == 1 && objects[0]->getGLEObject() != NULL)
	{
		// No union supported on extra arguments yet - how to best implement this?
		GLEDrawObject* gleobj = objects[0]->getGLEObject();
		GLEPropertyStore *store = gleobj->getProperties();
		GLEPropertyStoreModel* model = store->getModel();
		for (int i = 0; i < model->getNumberOfExtraProperties(); i++)
		{
			prop.index = GLEDrawingObject::PropertyUser+i;
			GLEProperty* gleprop = model->getProperty(i);
			prop.description = QGLE::stlToQString(gleprop->getName());
			if (prop.description.size() > 1)
			{
				// Adjust case
				prop.description = prop.description.left(1).toUpper() + prop.description.mid(1).toLower();
				prop.description.replace('_', ' ');
			}
			if (gleprop->getType() == GLEPropertyTypeString)
			{
				GLEString* strvalue = store->getStringProperty(gleprop);
				prop.value = QGLE::gleToQString(strvalue);
			}
			else if  (gleprop->getType() == GLEPropertyTypeReal)
			{
				prop.value = store->getRealProperty(gleprop);
			}
			prop.validValues = QList<QVariant>();
			properties.append(prop);
		}
	}

	int index;
	foreach(index, propertySet)
	{
		thisProperty = objects[0]->getPropertyDescription(index);
		prop.index = index;
		prop.description = thisProperty.description;
		prop.value = getPropertyValue(index, objs);
		prop.validValues = thisProperty.validValues;
		properties.append(prop);
	}

	if (hasModel)
		delete(model);

	model = new GLEPropertyModel(properties, this);
	connect(model, SIGNAL(dataChanged(QModelIndex, QModelIndex)),
			this, SLOT(propertyChanged(QModelIndex, QModelIndex)));
	hasModel = true;
	setModel(model);
	setItemDelegate(new VariantDelegate(this));
	setSelectionBehavior(QAbstractItemView::SelectRows);
	setSelectionMode(QAbstractItemView::NoSelection);

}

QVariant GLEPropertyEditor::getPropertyValue(int index, QList<GLEDrawingObject *> objs)
{
	// Find the value.  This involves iterating through all the
	// objects and identifying whether they all have the same value.
	// If not, return a QVariant::List containing a single entry of the
	// appropriate type.

	QVariant value, compare;
	value = objs[0]->getProperty(index);
	int i;

	for(i=1;i<objs.size();i++)
	{
		compare = objs[i]->getProperty(index);
		if (compare != value)
		{
			QList<QVariant> blank;
			blank.append(QVariant(value.type()));
			value = QVariant(blank);
			break;
		}
	}

	return(value);
}

/*
QSize GLEPropertyEditor::sizeHint() const
{
	qDebug() << "Default size property editor: " << defaultSize;
	if (defaultSize == 0) return QTableView::sizeHint();
	return QSize(250, defaultSize);
}

int GLEPropertyEditor::heightForWidth(int) const
{
	return -1;
}
*/
