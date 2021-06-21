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
 * about.cpp: The class implementation for the about box.      *
 ***************************************************************/

#include <QtGui>
#include "about.h"
#include "qgle_statics.h"
#include "qgle_definitions.h"
#include "../gle/gle-interface/gle-interface.h"

class MyQTextBrowser : public QTextBrowser {
public:
   MyQTextBrowser();
   virtual ~MyQTextBrowser();
   virtual QSize sizeHint () const;
};

MyQTextBrowser::MyQTextBrowser() {
}

MyQTextBrowser::~MyQTextBrowser() {
}

QSize MyQTextBrowser::sizeHint() const {
   QSizeF size = document()->size();
   return QSize((int)size.width()+5, (int)size.height()+10);
}

AboutBox::AboutBox(QWidget* par, GLEInterface* gleInterface) : QDialog(par) {
	m_gleInterface = gleInterface;

	// Set the window title and size
	setWindowTitle(tr("About %1").arg(APP_NAME));

	QHBoxLayout *top = new QHBoxLayout();
	QVBoxLayout *left = new QVBoxLayout();
	QLabel* icon = new QLabel(this);
	icon->setPixmap(QPixmap(":/images/gle_shadow.png").scaled(QSize(64, 64)));
	left->addWidget(icon);
	left->addStretch(1);
	top->addLayout(left);

   QWidget* license = createLicensePanel();
	QTabWidget* tabWidget = new QTabWidget();
	tabWidget->addTab(createAboutPanel(), tr("About"));
	tabWidget->addTab(createContributorsPanel(), tr("Contributors"));
	tabWidget->addTab(license, tr("License"));
	top->addWidget(tabWidget);

	QPushButton *okButton = new QPushButton(tr("Close"));
	connect(okButton, SIGNAL(clicked()), this, SLOT(close()));
	QHBoxLayout *buttonLayout = new QHBoxLayout();
	buttonLayout->addStretch(1);
	buttonLayout->addWidget(okButton);

	QVBoxLayout *layout = new QVBoxLayout();
	layout->addLayout(top);
	layout->addLayout(buttonLayout);
	setLayout(layout);
}

// SLOT: show a given URL
void AboutBox::showURL(const QUrl& url) {
	QDesktopServices::openUrl(url);
}

QTextBrowser* AboutBox::createAboutPanel()
{
	// Create a label to display readme.txt
	MyQTextBrowser* label = new MyQTextBrowser();
	label->setOpenLinks(false);
	connect(label, SIGNAL(anchorClicked(const QUrl&)), this, SLOT(showURL(const QUrl&)));
	QFile file(":/about.html");
	// Open the file as a read only text file
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QString html(file.readAll().constData());
		html.replace(tr("*VERSION*"), QString::fromUtf8(m_gleInterface->getGLEVersion().c_str()));
		html.replace(tr("*DATE*"), QString::fromUtf8(m_gleInterface->getGLEBuildDate().c_str()));
		label->setHtml(html);
	}
   label->document()->setTextWidth(m_minWidth);
	return label;
}

QWidget* AboutBox::createContributorsPanel()
{
	QString html(tr(""
	"<h2>Contributors</h2>"
	"<p>"
	"The following people have given their time and talent to help support the GLE/QGLE effort (in alphabetical order)."
	"</p>"
	""
	"<ul>"
	"<li>A. S. Budden: programming (QGLE), packager for Arch Linux</li>"
	"<li>Andrey G. Grozin: packager for Gentoo Linux</li>"
	"<li>Axel Rohde: 3.3f-h versions (these were 32 bit DOS and OS/2 ports)</li>"
	"<li>Bryn Jeffries: programming (Linux)</li>"
	"<li>Chris Pugmire: original program creation and design</li>"
	"<li>Christoph Brendes: programming</li>"
	"<li>David Parfitt: documentation (GLE users guide)</li>"
	"<li>Edd Edmondson: packager for Mac OS/X"
	"<li>Jan Struyf: programming (and current 4.x series maintainer)</li>"
	"<li>Laurence Abbott: programming</li>"
	"<li>Michal Vyskocil: packager for openSUSE</li>"
	"<li>Stephen Blundell: documentation (GLE users guide)</li>"
	"<li>Steve Wilkinson: programming (user interface)</li>"
	"<li>Terje R&oslash;sten: packager for Fedora Core</li>"
	"<li>Vincent LaBella: resurrected 3.3h to GLE 4.0 C++ code base</li>"
	"<li>Zbigniew Kisiel: testing</li>"
	"</ul>"
	"<p>&nbsp;</p>"
	""));
	QTextBrowser* label = new QTextBrowser();
	label->setOpenLinks(false);
	label->setHtml(html);
	return label;
}

QWidget* AboutBox::createLicensePanel()
{
	// Create a label to display readme.txt
	QTextEdit* label = new QTextEdit();
	label->setReadOnly(true);
	QString fileName(QString::fromUtf8(m_gleInterface->getManualLocation().c_str()));
	fileName.resize(fileName.lastIndexOf(QDir::separator()));
	fileName += QDir::separator();
	fileName += tr("LICENSE.txt");
   GLEInterface* iface = GLEGetInterfacePointer();
   std::string licenseFileTxt;
   bool res = iface->readFileOrGZIPTxt(fileName.toUtf8().constData(), &licenseFileTxt);
   if (res) {
		QFont font;
		// Set the font to be fixed pitch
		font.setFixedPitch(true);
		font.setFamily("Courier");
      font.setStretch(QFont::Condensed);
		label->setLineWrapMode(QTextEdit::NoWrap);
		label->setFont(font);
		label->setTextColor(Qt::black);
		// Get the text and put it in the label
		label->setPlainText(licenseFileTxt.c_str());
		QFontMetrics fm(font);
		m_minWidth = fm.width("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
	} else {
		label->setPlainText(tr("File not found: '%1'").arg(fileName));
	}
	return label;
}
