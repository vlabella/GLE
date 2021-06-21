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

#include "variantdelegate.h"
#include "propertymodel.h"
#include "colourpicker.h"
#include "qgle_statics.h"

VariantDelegate::VariantDelegate(QObject *parent)
	: QItemDelegate(parent)
{
}

QWidget *VariantDelegate::createEditor(QWidget *parent,
		const QStyleOptionViewItem&,
		const QModelIndex &index) const
{
	GLEPropertyModel *model = (GLEPropertyModel *) index.model();
	QVariant entry = model->data(index, Qt::DisplayRole);
	GLEDrawingObjectProperty property = model->getFullProperty(index);

	// Get the requested entry details
	if (index.column() == 0)
		return(new QLabel(entry.toString(), parent));

	QDoubleSpinBox *doubleSpinBox;
	QLineEdit *lineEditor;
	QComboBox *comboBox;
	ColourPicker *colourPicker;
	QVariant subentry;
	QString listEntry;
	QVariantMap entries;
	QList<QVariant> listentries;

	switch(entry.type())
	{
		case QVariant::Double:
			doubleSpinBox = new QDoubleSpinBox(parent);
			doubleSpinBox->setDecimals(3);
			doubleSpinBox->installEventFilter(const_cast<VariantDelegate*>(this));
			return(doubleSpinBox);
			break;

		case QVariant::Int:
			if (property.validValues.type() == QVariant::List)
			{
				listentries = property.validValues.value<QList<QVariant> >();
				comboBox = new QComboBox(parent);
				comboBox->setEditable(false);
				for (int i = 0; i < listentries.size(); i++) {
					comboBox->addItem(listentries[i].toString(), QVariant(i));
				}
				comboBox->installEventFilter(const_cast<VariantDelegate*>(this));
				return(comboBox);
			}
			else if (property.validValues.type() == QVariant::Map)
			{
				entries = property.validValues.value<QVariantMap>();
				comboBox = new QComboBox(parent);
				comboBox->setEditable(false);
				foreach(listEntry, entries.keys())
				{
					comboBox->addItem(listEntry, entries[listEntry]);
				}
				comboBox->installEventFilter(const_cast<VariantDelegate*>(this));
				return(comboBox);
			}
			break;

		case QVariant::String:
			lineEditor = new QLineEdit(parent);
			lineEditor->installEventFilter(const_cast<VariantDelegate*>(this));
			return(lineEditor);
			break;

		case QVariant::Color:
			colourPicker = new ColourPicker(parent, (VariantDelegate*)this);
			colourPicker->setColour(entry.value<QColor>());
			colourPicker->installEventFilter(const_cast<VariantDelegate*>(this));
			colourPicker->showPopup();
			return(colourPicker);
			break;

		case QVariant::List:
			subentry = entry.value<QVariantList>().at(0);
			switch(subentry.type())
			{
				case QVariant::Color:
					colourPicker = new ColourPicker(parent, (VariantDelegate*)this);
					colourPicker->setColour();
					colourPicker->installEventFilter(const_cast<VariantDelegate*>(this));
					colourPicker->showPopup();
					return(colourPicker);
					break;
				default:
					break;
			}
			break;

		default:
			return(new QLabel(entry.toString(), parent));
	}
	return(new QLabel());
}

void VariantDelegate::setEditorData(QWidget *editor,
		const QModelIndex &index) const
{
	GLEPropertyModel *model = (GLEPropertyModel *) index.model();
	QVariant value = model->data(index, Qt::DisplayRole);
	GLEDrawingObjectProperty property = model->getFullProperty(index);


	if (index.column() == 0)
		return;

	QDoubleSpinBox *doubleSpinBox;
	QComboBox *comboBox;
	QLineEdit *lineEditor;
	ColourPicker *colourPicker;
	QVariant subentry;

	switch(value.type())
	{
		case QVariant::Double:
			doubleSpinBox = static_cast<QDoubleSpinBox*>(editor);
			doubleSpinBox->setValue(value.value<double>());
			break;

		case QVariant::Int:
			if (property.validValues.type() == QVariant::List)
			{
				// This is going to need to be changed for
				// items with a list of valid values
				comboBox = static_cast<QComboBox*>(editor);
				comboBox->setCurrentIndex(value.value<int>());
			}
			else if (property.validValues.type() == QVariant::Map)
			{
				comboBox = static_cast<QComboBox*>(editor);
				// TODO: The following line causes a segmentation fault (on findData())
				comboBox->setCurrentIndex(comboBox->findData(value.value<int>()));
			}
			break;

		case QVariant::String:
			lineEditor = static_cast<QLineEdit*>(editor);
			lineEditor->setText(value.value<QString>());
			break;

		case QVariant::Color:
			colourPicker = static_cast<ColourPicker*>(editor);
			colourPicker->setColour(value.value<QColor>());
			break;

		case QVariant::List:
			subentry = value.value<QVariantList>().at(0);
			switch(subentry.type())
			{
				case QVariant::Color:
					colourPicker = static_cast<ColourPicker*>(editor);
					colourPicker->setColour();
					break;
				default:
					break;
			}
			break;

		default:
			return;
	}
}

void VariantDelegate::setModelData(QWidget *editor,
		QAbstractItemModel *model,
		const QModelIndex &index) const
{
	GLEPropertyModel *glemodel = (GLEPropertyModel *) index.model();
	QVariant entry = glemodel->data(index, Qt::DisplayRole);
	GLEDrawingObjectProperty property = glemodel->getFullProperty(index);


	if (index.column() == 0)
		return;

	QDoubleSpinBox *doubleSpinBox;
	QComboBox *comboBox;
	QLineEdit *lineEditor;
	ColourPicker *colourPicker;
	double value;
	int integerVal;
	QVariant subentry;

	switch(entry.type())
	{
		case QVariant::Double:
			doubleSpinBox = static_cast<QDoubleSpinBox*>(editor);
			doubleSpinBox->interpretText();
			value = doubleSpinBox->value();
			model->setData(index, value);
			break;

		case QVariant::Int:
			if (property.validValues.type() == QVariant::List)
			{
				// This is going to need to be changed for
				// items with a list of valid values
				comboBox = static_cast<QComboBox*>(editor);
				integerVal = comboBox->currentIndex();
				model->setData(index, integerVal);
			}
			else if (property.validValues.type() == QVariant::Map)
			{
				comboBox = static_cast<QComboBox*>(editor);
				integerVal = comboBox->itemData(comboBox->currentIndex()).value<int>();
				model->setData(index, integerVal);
			}
			break;

		case QVariant::String:
			lineEditor = static_cast<QLineEdit*>(editor);
			model->setData(index, lineEditor->text());
			break;

		case QVariant::Color:
			colourPicker = static_cast<ColourPicker*>(editor);
			// if (colourPicker->hasChanged())
			model->setData(index, colourPicker->getColour());
			break;


		case QVariant::List:
			subentry = entry.value<QVariantList>().at(0);
			switch(subentry.type())
			{
				case QVariant::Color:
					colourPicker = static_cast<ColourPicker*>(editor);
					//if (colourPicker->hasChanged())
					model->setData(index, colourPicker->getColour());
					break;
				default:
					break;
			}
			break;




		default:
			return;
	}
}

void VariantDelegate::updateEditorGeometry(QWidget *editor,
		const QStyleOptionViewItem &option,
		const QModelIndex&) const
{
	editor->setGeometry(option.rect);
}

void VariantDelegate::paint(QPainter *painter,
		const QStyleOptionViewItem &option,
		const QModelIndex &index) const
{
	double x,y,width,height;
	int intVal;
	QColor colour;
	QPixmap colourSample;
	QString text;
	QString str;

	GLEPropertyModel *glemodel = (GLEPropertyModel *) index.model();
	QVariant entry = glemodel->data(index, Qt::DisplayRole);
	GLEDrawingObjectProperty property = glemodel->getFullProperty(index);
	QVariant subentry;
	QVariantMap entries;
	QList<QVariant> listentries;

	if (index.column() == 1)
	{
		QStyleOptionViewItem myOption = option;
		switch(entry.type())
		{
			case QVariant::Double:
				text = QString("%1").arg(entry.value<double>());
				myOption.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
				drawDisplay(painter, myOption, myOption.rect, text);
				drawFocus(painter, myOption, myOption.rect);
				break;

			case QVariant::PointF:
				x = entry.value<QPointF>().x();
				y = entry.value<QPointF>().y();
				text = QString("(%1,%2)").arg(x).arg(y);
				myOption.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
				drawDisplay(painter, myOption, myOption.rect, text);
				drawFocus(painter, myOption, myOption.rect);
				break;

			case QVariant::SizeF:
				width = entry.value<QSizeF>().width();
				height = entry.value<QSizeF>().height();
				text = QString("(%1x%2)")
					.arg(width).arg(height);
				myOption.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
				drawDisplay(painter, myOption, myOption.rect, text);
				drawFocus(painter, myOption, myOption.rect);
				break;

			case QVariant::Color:
				colour = entry.value<QColor>();
				colourSample = QPixmap(10,10);
				if (colour.isValid())
				{
					colourSample.fill(colour);
				}
				else
				{
					QPen pen;
					pen.setColor(Qt::black);
					pen.setWidth(1);
					QPainter p(&colourSample);
					colourSample.fill(Qt::white);
					QGLE::drawBox(&p, colourSample.rect().center(), 4.0, pen);
					QGLE::drawCross(&p, colourSample.rect().center(), 4.0, pen);
				}
				myOption.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
				drawDecoration(painter, myOption, myOption.rect, colourSample);
				drawFocus(painter, myOption, myOption.rect);
				break;

			case QVariant::List:
				subentry = entry.value<QVariantList>().at(0);
				switch(subentry.type())
				{
					case QVariant::Color:
						colourSample = QPixmap(":images/varyingcolours.png");
						myOption.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
						drawDecoration(painter, myOption, myOption.rect, colourSample);
						drawFocus(painter, myOption, myOption.rect);
						break;
					default:
						break;
				}
				break;

			case QVariant::Int:
				if (property.validValues.type() == QVariant::List)
				{
					intVal = entry.value<int>();
					listentries = property.validValues.value<QList<QVariant> >();
					text = listentries[intVal].toString();
					myOption.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
					drawDisplay(painter, myOption, myOption.rect, text);
					drawFocus(painter, myOption, myOption.rect);

				}
				else if (property.validValues.type() == QVariant::Map)
				{
					intVal = entry.value<int>();
					entries = property.validValues.value<QVariantMap>();
					text = "Unknown";
					foreach(str, entries.keys())
					{
						if (entries[str] == intVal)
						{
							text = str;
							break;
						}
					}
					myOption.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
					drawDisplay(painter, myOption, myOption.rect, text);
					drawFocus(painter, myOption, myOption.rect);
				}
				else
				{
					QItemDelegate::paint(painter, option, index);
				}
				break;

			default:
				QItemDelegate::paint(painter, option, index);
		}

	}
	else
	{
		QItemDelegate::paint(painter, option, index);
	}
}

void VariantDelegate::doneEditingColor(ColourPicker* picker) {
	commitData(picker);
	closeEditor(picker);
}
