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

#ifndef __COLOURPICKER_H
#define __COLOURPICKER_H

#include <QtGui>
#include "colourbutton.h"

class VariantDelegate;

class ColourPicker : public QAbstractButton
{
	Q_OBJECT

public:
	ColourPicker(QWidget *parent, VariantDelegate* delegate);

	QColor getColour();
	bool hasChanged();
	void setColour();

public slots:
	void setColour(QColor col);
	void colourChosen(QColor col);
	void moreColours();
	void doShowPopup();

protected:
	void paintEvent(QPaintEvent *event);
	void mouseDoubleClickEvent(QMouseEvent *event);
public:
	void showPopup();

private:
	QColor colour;

	bool changed;
	bool hasColour;

	QDialog *chooserFrame;
	QList<ColourButton *> buttons;
	VariantDelegate* parentDelegate;
	QTimer* myTimer;
};

#endif
