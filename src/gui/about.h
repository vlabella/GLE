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
 * about.h: The class definition for the about box.            *
 ***************************************************************/
#ifndef __ABOUTBOX_H
#define __ABOUTBOX_H

#include <QtWidgets>

class GLEInterface;

//! Class describing an about box to give information about QGLE
class AboutBox : public QDialog
{
	Q_OBJECT

public:
	//! The class constructor
	AboutBox(QWidget* par,GLEInterface* gleInterface);

private:
	QTextBrowser* createAboutPanel();
	QWidget* createContributorsPanel();
	QWidget* createLicensePanel();

private slots:
	//! Open a given URL
	void showURL(const QUrl&);

private:
	GLEInterface* m_gleInterface;
	int m_minWidth;
};

#endif
