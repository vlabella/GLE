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
#include <iostream>
#include <string>
#include <regex>
#include <unordered_map>
#include "consolewindow.h"
using namespace std;


// Normal colors
const QString color_black   = "<span style=\"color:#000000;\">";
const QString color_red     = "<span style=\"color:#FF0000;\">";
const QString color_green   = "<span style=\"color:#00FF00;\">";
const QString color_yellow  = "<span style=\"color:#FFFF00;\">";
const QString color_blue    = "<span style=\"color:#0000FF;\">";
const QString color_magenta = "<span style=\"color:#FF00FF;\">";
const QString color_cyan    = "<span style=\"color:#00FFFF;\">";
const QString color_white   = "<span style=\"color:#FFFFFF;\">";

// High-intensity colors (bold)
const QString color_black_bold   = "<span style=\"color:#000000;font-weight:bold;\">";
const QString color_red_bold     = "<span style=\"color:#FF0000;font-weight:bold;\">";
const QString color_green_bold   = "<span style=\"color:#00FF00;font-weight:bold;\">";
const QString color_yellow_bold  = "<span style=\"color:#FFFF00;font-weight:bold;\">";
const QString color_blue_bold    = "<span style=\"color:#0000FF;font-weight:bold;\">";
const QString color_magenta_bold = "<span style=\"color:#FF00FF;font-weight:bold;\">";
const QString color_cyan_bold    = "<span style=\"color:#00FFFF;font-weight:bold;\">";
const QString color_white_bold   = "<span style=\"color:#FFFFFF;font-weight:bold;\">";

const QString color_end   = "</span>";
const QString bold       = "<b>";
const QString bold_end   = "</b>";

std::string ansiToHtml(const std::string &ansiText) {
    // Map ANSI color codes to hex colors with high intesity as bold
	struct ColorStyle {
    std::string color;
    bool bold;
	};
	std::unordered_map<std::string, ColorStyle> colorMap = {
	    {"30", {"#000000", false}}, {"31", {"#FF0000", false}}, {"32", {"#00FF00", false}}, {"33", {"#FFFF00", false}},
	    {"34", {"#0000FF", false}}, {"35", {"#FF00FF", false}}, {"36", {"#00FFFF", false}}, {"37", {"#FFFFFF", false}},
	    // High-intensity colors → same color, bold = true
	    {"90", {"#000000", true}}, {"91", {"#FF0000", true}}, {"92", {"#00FF00", true}}, {"93", {"#FFFF00", true}},
	    {"94", {"#0000FF", true}}, {"95", {"#FF00FF", true}}, {"96", {"#00FFFF", true}}, {"97", {"#FFFFFF", true}}
	};

    std::string html;
    std::regex ansiRegex("\033\\[([0-9;]+)m");
    std::smatch match;

    size_t lastPos = 0;
    bool spanOpen = false;

    std::string text = ansiText;

    while (std::regex_search(text, match, ansiRegex)) {
        // Append text before ANSI code
        html += text.substr(0, match.position());

        std::string codes = match[1];
        std::string style;

        // Parse codes
        size_t start = 0;
        while (start < codes.size()) {
            size_t end = codes.find(';', start);
            if (end == std::string::npos) end = codes.size();
            std::string code = codes.substr(start, end - start);

            if (code == "0") {
                // Reset: close any open span
                if (spanOpen) {
                    html += "</span>";
                    spanOpen = false;
                }
            } else if (code == "1") {
                style += "font-weight:bold;";
            } else if (code == "4") {
                style += "text-decoration:underline;";
            } else if (colorMap.find(code) != colorMap.end()) {
			    const auto &styleInfo = colorMap[code];
			    style += "color:" + styleInfo.color + ";";
			    if (styleInfo.bold) {
			        style += "font-weight:bold;";
			    }
			}
            start = end + 1;
        }

        // Open new span if style is set
        if (!style.empty()) {
            if (spanOpen) {
                html += "</span>"; // Close previous span
            }
            html += "<span style=\"" + style + "\">";
            spanOpen = true;
        }

        // Move past this match
        text = match.suffix();
    }

    // Append remaining text
    html += text;

    // Close any open span
    if (spanOpen) {
        html += "</span>";
    }

    return html;
}
// wrapper
QString ansiToHtml(const QString& input) {
    // Convert QString to std::string (UTF-8 recommended)
    std::string ansiStr = input.toUtf8().constData();
    // Call the original function
    std::string htmlStr = ansiToHtml(ansiStr);
    // Convert back to QString
    return QString::fromUtf8(htmlStr.c_str());
}

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
	append(ansiToHtml(str));
}

void ConsoleWindow::print(QString msg) {
	append(ansiToHtml(msg));
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
    QString file   = QString::fromUtf8(msg->getFile());
    QString abbrev = QString::fromUtf8(msg->getLineAbbrev());
	abbrev.replace('<', "&lt;");
	abbrev.replace('>', "&gt;");
	QString line = QString("%1").arg(msg->getLine());
	QString fline = color_red_bold + QString(">> %1 (%2%3%4)").arg(file).arg(color_blue_bold).arg(msg->getLine()).arg(color_end) + color_end;
	if (abbrev.length() > 0) {
		abbrev = QString(" ") + color_blue_bold + QString("|%1|").arg(abbrev) + color_end;
	}
	QString outstr = fline+abbrev;
	if (msg->getColumn() != -1) {
		QString point = ">> ";
		int nbspc = file.length() + line.length() + 4 + msg->getColumn() - msg->getDelta();
		point += QString("&nbsp;").repeated(nbspc) + "^";
		outstr += "<br>" + color_red_bold + point + color_end;
	}
	emit print(outstr);
}
