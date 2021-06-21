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

#include "propertymodel.h"
#include "qgle_statics.h"

GLEPropertyModel::GLEPropertyModel(const QList<GLEDrawingObjectProperty> &props, QObject *parent)
	: QAbstractTableModel(parent)
{
	propertyList = props;
}


int GLEPropertyModel::rowCount(const QModelIndex&) const
{
	return(propertyList.size());
}

int GLEPropertyModel::columnCount(const QModelIndex&) const
{
	return(2);
}

GLEDrawingObjectProperty GLEPropertyModel::getFullProperty(const QModelIndex &index)
{
	return(propertyList.at(index.row()));
}

QVariant GLEPropertyModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (index.row() >= propertyList.size())
		return QVariant();

	if (index.column() >= 2)
		return QVariant();

	if (role == Qt::DisplayRole)
	{
		if (index.column() == 0)
			return(QVariant(propertyList.at(index.row()).description));
		else
			return(propertyList.at(index.row()).value);
	}

	return QVariant();
}

QVariant GLEPropertyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
		return QVariant();

	if (orientation == Qt::Horizontal)
	{
		if (section == 1)
			return QString("Value");
		else
			return QString("Property");
	}
	else
		return QVariant();
}


Qt::ItemFlags GLEPropertyModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::ItemIsEnabled;

	if (index.column() == 0)
		return QAbstractItemModel::flags(index);
	else
		return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool GLEPropertyModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (index.isValid() && role == Qt::EditRole)
	{
		propertyList[index.row()].value = value;
		emit dataChanged(index, index);
		return(true);

	}
	return(false);
}
