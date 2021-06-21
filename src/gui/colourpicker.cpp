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

#include "colourpicker.h"
#include "variantdelegate.h"
#include "qgle_statics.h"

ColourPicker::ColourPicker(QWidget *parent, VariantDelegate *delegate)
	: QAbstractButton(parent)
{

	changed = false;
	hasColour = false;
	parentDelegate = delegate;
	// At some point, this will be changed to
	// show a little frame with a standard
	// choice of colours including "no fill"

	chooserFrame = new QDialog(parent);
	QGridLayout *chooserLayout = new QGridLayout();

	buttons.append(new ColourButton(QColor()));
	buttons.append(new ColourButton(Qt::white));
	buttons.append(new ColourButton(Qt::blue));
	buttons.append(new ColourButton(Qt::green));
	buttons.append(new ColourButton(Qt::red));
	buttons.append(new ColourButton(Qt::yellow));

	ColourButton *button;
	int i = 0;
	foreach(button, buttons)
	{
		chooserLayout->addWidget(button, 1, i);
		connect(button, SIGNAL(colourChosen(QColor)),
				this, SLOT(colourChosen(QColor)));
		i++;
	}
	QPixmap ellipsis(":images/ellipsis.png");
	ColourButton *moreButton = new ColourButton(ellipsis);
	chooserLayout->addWidget(moreButton, 1, i);
	connect(moreButton, SIGNAL(clicked()), this, SLOT(moreColours()));

	chooserFrame->setLayout(chooserLayout);
	chooserFrame->setBackgroundRole(QPalette::Button);
	chooserFrame->move(mapToGlobal(rect().bottomLeft()));
	chooserFrame->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
}

void ColourPicker::doShowPopup() {
	myTimer->stop();
	chooserFrame->exec();
}

void ColourPicker::showPopup() {
	myTimer = new QTimer(this);
	connect(myTimer, SIGNAL(timeout()), this, SLOT(doShowPopup()));
	myTimer->start(10);
}

void ColourPicker::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	painter.fillRect(rect(), Qt::white);
	QPixmap colourSwatch(10,10);
	if (hasColour)
	{
		if (colour.isValid())
		{
			colourSwatch.fill(colour);
			painter.drawPixmap(rect().center()-QPoint(5,5), colourSwatch);
		}
		else
		{
			QPen pen;
			pen.setColor(Qt::black);
			pen.setWidth(1);
			QGLE::drawBox(&painter, rect().center(), 4.0, pen);
			QGLE::drawCross(&painter, rect().center(), 4.0, pen);
		}
	}
	else
	{
		colourSwatch = QPixmap(":images/varyingcolours.png");
		painter.drawPixmap(rect().center()-QPoint(5,5), colourSwatch);
	}
}

void ColourPicker::mouseDoubleClickEvent(QMouseEvent *)
{
	chooserFrame->exec();
}

void ColourPicker::moreColours()
{
	chooserFrame->done(0);
	QColor newColour = QColorDialog::getColor(colour, this);
	if (newColour.isValid())
	{
		setColour(newColour);
		changed = true;
	}
	parentDelegate->doneEditingColor(this);
}

QColor ColourPicker::getColour()
{
	return(colour);
}

void ColourPicker::colourChosen(QColor col)
{
	chooserFrame->done(0);
	setColour(col);
	changed = true;
	parentDelegate->doneEditingColor(this);
}

void ColourPicker::setColour(QColor col)
{
	colour = col;
	hasColour = true;
	update();
}

void ColourPicker::setColour()
{
	hasColour = false;
}

bool ColourPicker::hasChanged()
{
	return(changed);
}
