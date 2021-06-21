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

#include "colourbutton.h"
#include "qgle_statics.h"

ColourButton::ColourButton(QColor col, QWidget *parent)
	: QAbstractButton(parent)
{
	resize(10,10);
	connect(this, SIGNAL(clicked()),
			this, SLOT(onClick()));
	setColour(col);
	hasPixmap = false;
}

ColourButton::ColourButton(QPixmap px, QWidget *parent)
	: QAbstractButton(parent)
{
	resize(10,10);
	connect(this, SIGNAL(clicked()),
			this, SLOT(onClick()));
	pixmap = px;
	hasPixmap = true;
}

QSize ColourButton::sizeHint() const
{
	return QSize(10,10);
}


void ColourButton::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	painter.fillRect(rect(), Qt::white);
	if (hasPixmap)
	{
		painter.drawPixmap(rect().center()-QPoint(5,5), pixmap);
	}
	else
	{
		QPixmap colourSwatch(10,10);
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
			QGLE::drawBox(&painter, rect().center(), 5.0, pen);
			QGLE::drawCross(&painter, rect().center(), 5.0, pen);
		}
	}

}

void ColourButton::setColour(QColor col)
{
	colour = col;
	update();
}

void ColourButton::onClick()
{
	if (!hasPixmap)
		emit colourChosen(colour);
}
