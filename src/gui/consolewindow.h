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

#ifndef _CONSOLE_WINDOW_H
#define _CONSOLE_WINDOW_H

#include "../gle/gle-interface/gle-interface.h"
#include <QtWidgets>

class ConsoleWindow;

class ConsoleWindowOutput : public QObject, public GLEOutputStream
{
	Q_OBJECT

public:
	ConsoleWindowOutput();
	virtual ~ConsoleWindowOutput();
	virtual void println();
	virtual void println(const char* str);
	virtual void error(GLEErrorMessage* msg);
signals:
	void print(const QString str);
};

class ConsoleWindow : public QTextEdit
{
	Q_OBJECT

protected:
	ConsoleWindowOutput output;
	QSplitter* splitWindow;
	QAction* hideAction;
	int autoShowSize;

public slots:
	//! Slot used to toggle the auto-show functionality of the console window
	void hideTriggered();
	void print(QString msg);

public:
	ConsoleWindow(QWidget* parent, QSplitter* splitter);
	virtual ~ConsoleWindow();
	virtual void contextMenuEvent(QContextMenuEvent * e);
	void shouldAutoShow();
	void println(const QString& str);
	inline GLEOutputStream* getOutput() { return &output; }
	inline void setHideAction(QAction* act) { hideAction = act; }
	inline QAction* getHideAction() { return hideAction; }
	inline int getAutoShowSize() { return autoShowSize; }
	inline void setAutoShowSize(int s) { autoShowSize = s; }
};

#endif
