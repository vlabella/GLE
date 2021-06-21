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

/***************************************************************
 * newfile.h: The class definition for the new file dialogue.  *
 ***************************************************************/
#ifndef __NEWFILE_H
#define __NEWFILE_H

#include <QtWidgets>

//! Class used to display a dialogue box asking the user for the size of the drawing
class NewFileBox : public QDialog
{
	Q_OBJECT

public:
	//! Class constructor
	NewFileBox(QWidget *parent = 0);
	//! Size of the drawing
	QSizeF size();
	//! Should save the new imdage?
	inline bool shouldSave() { return save; }

public slots:
	void saveClicked();

private:
	//! The layout of the dialogue box
	QGridLayout *layout;
	//! Spinbox for the width
	QDoubleSpinBox *width;
	//! Spinbox for the height
	QDoubleSpinBox *height;
	//! Create the drawing
	QPushButton *okCmd;
	//! Create the drawing and save it
	QPushButton *saveCmd;
	//! Give up.
	QPushButton *cancelCmd;
	//! Should save the new image?
	bool save;
};

#endif
