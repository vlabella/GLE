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

/********************************************************************
 * newfile.cpp: The class implementation for the new file dialogue. *
 ********************************************************************/

#include <QtGui>
#include "newfile.h"
#include "qgle_definitions.h"

NewFileBox::NewFileBox(QWidget *parent)
	: QDialog(parent)
{
	// Set the window title and size
	setWindowTitle(tr("New Diagram"));
	resize(150,150);
	save = false;

	// A Spin box to set the width of the new drawing
	width = new QDoubleSpinBox;
	width->setRange(0.0,21.0);
	width->setValue(DEFAULT_NEW_SIZE);
	// A spin box to set the height of the new drawing
	height = new QDoubleSpinBox;
	height->setRange(0.0,29.7);
	height->setValue(DEFAULT_NEW_SIZE);

	// Create a layout and some push buttons
	cancelCmd = new QPushButton(tr("Cancel"));
	saveCmd = new QPushButton(tr("Save As"));
	okCmd = new QPushButton(tr("OK"));
	okCmd->setDefault(true);
	connect(cancelCmd, SIGNAL(clicked()), this, SLOT(reject()));
	connect(saveCmd, SIGNAL(clicked()), this, SLOT(saveClicked()));
	connect(okCmd, SIGNAL(clicked()), this, SLOT(accept()));

	// Layout the dialogue
	QVBoxLayout* main = new QVBoxLayout();
	layout = new QGridLayout();
	layout->addWidget(new QLabel("Width (cm):"),0,0,1,1);
	layout->addWidget(width,0,1,1,2);
	layout->addWidget(new QLabel("Height (cm):"),1,0,1,1);
	layout->addWidget(height,1,1,1,2);
	main->addLayout(layout);
	QHBoxLayout* butt = new QHBoxLayout();
	butt->addStretch(1);
	butt->addWidget(cancelCmd);
	butt->addWidget(saveCmd);
	butt->addWidget(okCmd);
	main->addLayout(butt);
	setLayout(main);

}

// Return the selected sizes
QSizeF NewFileBox::size()
{
	return(QSizeF(width->value(), height->value()));
}

void NewFileBox::saveClicked()
{
	save = true;
	accept();
}
