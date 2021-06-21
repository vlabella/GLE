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

#include "consolewindow.h"

#include <iostream>

using namespace std;

ConsoleWindow::ConsoleWindow(QWidget* parent, QSplitter* splitter) : 
	QTextEdit(parent), 
	output(),
	autoShowSize(0)
{
	QFont font;
	font.setFamily("Courier");
	font.setFixedPitch(true);
	font.setPointSize(10);
	setFont(font);
	setReadOnly(true);
	splitWindow = splitter;
	connect(&output, SIGNAL(print(const QString)), this, SLOT(print(const QString)));
}

ConsoleWindow::~ConsoleWindow() {
}

void ConsoleWindow::hideTriggered() {
	autoShowSize = height();
	QWidget* first = splitWindow->widget(0);
	QList<int> splitSizes;
	splitSizes.append(first->height());
	splitSizes.append(0);
	splitWindow->setSizes(splitSizes);
}

void ConsoleWindow::shouldAutoShow() {
	if (output.getExitCode() != 0) {
		QWidget* first = splitWindow->widget(0);
		QList<int> splitSizes;
		splitSizes.append(first->height());
		if (autoShowSize != 0) {
			splitSizes.append(autoShowSize);
		} else {
			splitSizes.append(75);
		}
		splitWindow->setSizes(splitSizes);
	}
}

void ConsoleWindow::println(const QString& str) {
	append(str);
}

void ConsoleWindow::print(QString msg) {
	append(msg);
}

void ConsoleWindow::contextMenuEvent(QContextMenuEvent * e) {
	QMenu *menu = createStandardContextMenu();
	menu->addAction(getHideAction());
	menu->exec(e->globalPos());
	delete menu;
}

ConsoleWindowOutput::ConsoleWindowOutput() {
}

ConsoleWindowOutput::~ConsoleWindowOutput() {
}

void ConsoleWindowOutput::println() {
	// emit print(QString::fromLatin1("\n"));
}

void ConsoleWindowOutput::println(const char* str) {
	emit print(QString::fromUtf8(str));
}

void ConsoleWindowOutput::error(GLEErrorMessage* msg) {
	QString file = QString::fromUtf8(msg->getFile());
	QString abbrev = QString::fromUtf8(msg->getLineAbbrev());
	abbrev.replace('<', "&lt;");
	abbrev.replace('>', "&gt;");
	QString line = QString("%1").arg(msg->getLine());
	QString fline = QString(">> <b>%1</b> (%2)").arg(file).arg(msg->getLine());
	if (abbrev.length() > 0) {
		abbrev = QString(" <font color=\"#0000FF\">|</font>%1<font color=\"#0000FF\">|</font>").arg(abbrev);
	}
	QString outstr = fline+abbrev;
	if (msg->getColumn() != -1) {
		QString point = ">> ";
		int nbspc = file.length() + line.length() + 4 + msg->getColumn() - msg->getDelta();
		for (int i = 0; i < nbspc; i++) {
			point += "&nbsp;";
		}
		point += "<font color=\"#FF0000\">^</font>";
		outstr += "<br>" + point;
	}
	emit print(outstr);
}
