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

#include <QtGui>

#include "qgle_statics.h"
#include "dialogues.h"
#include "settings.h"
#include "consolewindow.h"

EvaluatorDialog::EvaluatorDialog(GLEMainWindow *parent) : QDialog(parent)
{
	mainWin = parent;
	setModal(false);
	setWindowTitle(tr("Expression Evaluator"));
	results = new ConsoleWindow(this, NULL);
	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(new QLabel(tr("Results:")));
	layout->addWidget(results);
	layout->addWidget(new QLabel(tr("Expression:")));
	field = new QLineEdit();
	layout->addWidget(field);
	QHBoxLayout *btnlayo = new QHBoxLayout();
	btnlayo->addStretch(1);
	QPushButton* closeBt = new QPushButton(tr("Close"));
	connect(closeBt, SIGNAL(clicked()), this, SLOT(hide()));
	btnlayo->addWidget(closeBt);
	QPushButton* clearBt = new QPushButton(tr("Clear"));
	connect(clearBt, SIGNAL(clicked()), this, SLOT(clearResults()));
	btnlayo->addWidget(clearBt);
	QPushButton* evalBt = new QPushButton(tr("Eval"));
	connect(evalBt, SIGNAL(clicked()), this, SLOT(eval()));
	btnlayo->addWidget(evalBt);
	evalBt->setDefault(true);
	layout->addLayout(btnlayo);
	setLayout(layout);
	QWidget::setTabOrder(field, evalBt);
	QWidget::setTabOrder(evalBt, clearBt);
	QWidget::setTabOrder(clearBt, closeBt);
	QWidget::setTabOrder(closeBt, results);
}

void EvaluatorDialog::eval()
{
	QString txt = field->text().trimmed();
	if (txt == "") return;
	GLEInterface* iface = mainWin->getGLEInterface();
	GLEScript* script = mainWin->getGLEScript();
	iface->setOutputStream(results->getOutput());
	results->append(QString("<font color=\"#FF0000\">&gt;</font> %1").arg(txt));
	iface->evalString(txt.toUtf8().constData(), script);
	results->append(tr(""));
	mainWin->restoreConsoleOutput();
}

void EvaluatorDialog::clearResults()
{
	results->clear();
}

ExportDialogue::ExportDialogue(GLEMainWindow *parent) {
	device = -1;
	mainWin = parent;
	setWindowTitle(tr("Export File"));
	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(new QLabel(tr("Export to:")));
	list = new QListWidget();
	GLEInterface* iface = mainWin->getGLEInterface();
	addFormat(tr("Portable Document Format (PDF)"), GLE_DEVICE_PDF);
	addFormat(tr("Encapsulated PostScript (EPS)"), GLE_DEVICE_EPS);
	addFormat(tr("PostScript Document (PS)"), GLE_DEVICE_PS);
	if (iface->isDeviceSupported(GLE_DEVICE_SVG)) {
		addFormat(tr("Scalable Vector Graphics (SVG)"), GLE_DEVICE_SVG);
	}
	if (iface->isDeviceSupported(GLE_DEVICE_EMF)) {
		addFormat(tr("Enhanced Metafile (EMF)"), GLE_DEVICE_EMF);
	}
	addFormat(tr("Portable Network Graphics (PNG)"), GLE_DEVICE_PNG);
	addFormat(tr("JPEG Interchange Format (JPEG)"), GLE_DEVICE_JPEG);
	list->setCurrentRow(mainWin->settings->getExportFormat());
	list->adjustSize();
	connect(list, SIGNAL(currentRowChanged(int)), this, SLOT(exportFormatChanged(int)));
	layout->addWidget(list);
	QHBoxLayout *p1 = new QHBoxLayout();
	p1->addWidget(new QLabel(tr("Page format:")));
	format = new QComboBox();
	format->addItem(tr("Figure"));
	format->addItem(tr("Portrait"));
	format->addItem(tr("Landscape"));
	if (mainWin->isFullPage()) {
		if (mainWin->isLandscape()) format->setCurrentIndex(2);
		else format->setCurrentIndex(1);
	} else {
		format->setCurrentIndex(mainWin->settings->getExportPageSize());
	}
	p1->addWidget(format);
	p1->addStretch(1);
	layout->addLayout(p1);
	QHBoxLayout *p2 = new QHBoxLayout();
	resolutionLabel = new QLabel(tr("Resolution (dpi):"));
	p2->addWidget(resolutionLabel);
	resolution = new QSpinBox();
	resolution->setMinimum(1);
	resolution->setMaximum(2000);
	connect(resolution, SIGNAL(valueChanged(int)), this, SLOT(resolutionChanged(int)));
	p2->addWidget(resolution);
	sizeLabel = new QLabel();
	p2->addWidget(sizeLabel);
	p2->addStretch(1);
	layout->addLayout(p2);
	layout->addWidget(new QLabel(tr("(For vector formats, 'resolution' specifies the bitmap fallback resolution.)")));
	transp = new QCheckBox(tr("Transparent"));
	transp->setChecked(mainWin->settings->isExportTransparent());
	layout->addWidget(transp);
	grayScale = new QCheckBox(tr("Grayscale"));
	grayScale->setChecked(mainWin->settings->isExportGrayScale());
	layout->addWidget(grayScale);
	openResult = new QCheckBox(tr("View resulting file"));
	openResult->setChecked(mainWin->settings->isOpenExportedFigure());
	layout->addWidget(openResult);
	layout->addWidget(new QLabel(tr("File:")));
	fname = new QLineEdit();
	fname->setText(QString::fromUtf8(parent->getGLEScript()->getLocation()->getFullPath().c_str()));
	layout->addWidget(fname);
	QPushButton *cancel = new QPushButton(tr("Cancel"));
	connect(cancel, SIGNAL(clicked()), this, SLOT(close()));
	QPushButton *expnow = new QPushButton(tr("Export"));
	connect(expnow, SIGNAL(clicked()), this, SLOT(exportClicked()));
	expnow->setDefault(true);
	QPushButton *expas = new QPushButton(tr("Export As"));
	connect(expas, SIGNAL(clicked()), this, SLOT(exportAsClicked()));
	QHBoxLayout *p3 = new QHBoxLayout();
	p3->addStretch(1);
	p3->addWidget(cancel);
	p3->addWidget(expnow);
	p3->addWidget(expas);
	layout->addLayout(p3);
	setLayout(layout);
	resize(mainWin->width()/2, mainWin->height()/2);
	exportFormatChangedImpl(mainWin->settings->getExportFormat(), true);
}

void ExportDialogue::addFormat(const QString& name, int gleFormatID) {
	list->addItem(name);
	fmtToDevice.append(gleFormatID);
}

int ExportDialogue::entryIDtoDevice(int id) {
	if (id < fmtToDevice.size()) {
		return fmtToDevice[id];
	}
	return GLE_DEVICE_EPS;
}

bool ExportDialogue::isBitmap()
{
	return device == GLE_DEVICE_PNG || device == GLE_DEVICE_JPEG;
}

void ExportDialogue::resolutionChanged(int) {
	if (isBitmap()) {
		updateResolution();
	}
}

void ExportDialogue::exportAsClicked() {
	const char* ext = mainWin->getGLEInterface()->getDeviceFilenameExtension(device);
	QString ftype = QString("Exported file (*.%1)").arg(ext);
	QString fileName = QFileDialog::getSaveFileName(this,
	                   tr("Choose a file name to save under"),
	                   mainWin->settings->pwd(),
	                   ftype);
	if (fileName.isEmpty()) return;
	performExport(fileName);
}

void ExportDialogue::exportClicked() {
	performExport(fname->text());
}

void ExportDialogue::performExport(const QString& file) {
	GLEInterface* iface = mainWin->getGLEInterface();
	mainWin->clearConsoleWindow();
	iface->clearAllCmdLine();
	iface->setMakeDrawObjects(false);
	QString dpi = QString("%1").arg(resolution->value());
	iface->setCmdLineOptionString("resolution", dpi.toLatin1().constData());
	if (device == GLE_DEVICE_PNG && transp->isChecked()) {
		iface->setCmdLineOption("transparent");
	}
	if (grayScale->isChecked()) {
		iface->setCmdLineOption("nocolor");
	}
	if (format->currentIndex() != 0) {
		iface->setCmdLineOption("fullpage");
	}
	if (format->currentIndex() == 2) {
		iface->setCmdLineOption("landscape");
	}
	if (mainWin->settings->isRenderUsingCairo()) {
		iface->setCmdLineOption("cairo");
	}
	iface->setCompatibilityMode(mainWin->getCompatibility().toLatin1().constData());
	mainWin->settings->setExportPageSize(format->currentIndex());
	iface->renderGLE(mainWin->getGLEScript(), file.toLatin1().constData(), device);
	iface->clearAllCmdLine();
	mainWin->settings->setOpenExportedFigure(openResult->isChecked());
	if (isBitmap()) {
		mainWin->settings->setExportBitmapResolution(resolution->value());
	} else {
		mainWin->settings->setExportVectorResolution(resolution->value());
	}
	mainWin->settings->setExportGrayScale(grayScale->isChecked());
	mainWin->settings->setExportTransparent(transp->isChecked());
	if (openResult->isChecked()) {
		mainWin->showLocalFile(file);
	}
	close();
}

void ExportDialogue::updateResolution() {
	double dpi = (double)resolution->value();
	const GLEPoint& bb = mainWin->getGLEScript()->getBoundingBox();
	int img_wd = (int)floor((double)dpi/PS_POINTS_PER_INCH*bb.getX()+1);
	int img_hi = (int)floor((double)dpi/PS_POINTS_PER_INCH*bb.getY()+1);
	sizeLabel->setText(QString("(%1 x %2 pixels)").arg(img_wd).arg(img_hi));
}

void ExportDialogue::exportFormatChanged(int idx) {
	exportFormatChangedImpl(idx, false);
}

void ExportDialogue::exportFormatChangedImpl(int idx, bool updateAll) {
	device = entryIDtoDevice(idx);
	transp->setEnabled(device == GLE_DEVICE_PNG);
	if (device == GLE_DEVICE_PS && format->currentIndex() == 0) {
		// "Figure" output not compatible with PS
		format->setCurrentIndex(1);
	}
	const char* ext = mainWin->getGLEInterface()->getDeviceFilenameExtension(device);
	fname->setText(QGLE::addFileNameExtension(fname->text(), ext));
	if (isBitmap()) {
		updateResolution();
	} else {
		const GLEPoint& size = mainWin->getGLEScript()->getSize();
		sizeLabel->setText(QString("(%1 x %2 cm)").arg(size.getX()).arg(size.getY()));
	}
	mainWin->settings->setExportFormat(idx);
	if (previousWasBitmap != isBitmap() || updateAll) {
		if (isBitmap()) {
			resolution->setValue(mainWin->settings->getExportBitmapResolution());
		} else {
			resolution->setValue(mainWin->settings->getExportVectorResolution());
		}
	}
	previousWasBitmap = isBitmap();
}

SoftwareLocateThread::SoftwareLocateThread(SoftwareLocateDialogue* parent, const QString& root) {
	parentWindow = parent;
	rootDir = root;
}

SoftwareLocateThread::~SoftwareLocateThread() {
}

void SoftwareLocateThread::run() {
	parentWindow->getGLEInterface()->findDependencies(rootDir.toUtf8().constData());
	emit threadDone();
}

SoftwareLocateOutput::SoftwareLocateOutput() {
}

SoftwareLocateOutput::~SoftwareLocateOutput() {
}

void SoftwareLocateOutput::printflush(const char* str) {
	emit print(QString::fromUtf8(str));
}

void SoftwareLocateOutput::println(const char* str) {
	emit print(QString::fromUtf8(str) + QString::fromUtf8("\n"));
}

SoftwareLocateDialogue::SoftwareLocateDialogue(GLEMainWindow *parent, GLEInterface* iface, int buttons) : QDialog(parent) {
	found = false;
	modified = false;
	mainWin = parent;
	cancel = false;
	gleInterface = iface;
	searchButton = NULL;
	locButton = NULL;
	abortButton = NULL;
	rootButton = NULL;
	QVBoxLayout *layout = new QVBoxLayout();
	QHBoxLayout *top = new QHBoxLayout();
	QVBoxLayout *left = new QVBoxLayout();
	QLabel* icon = new QLabel(this);
	icon->setPixmap(QPixmap(":/images/gle_shadow.png").scaled(QSize(64, 64)));
	left->addWidget(icon);
	left->addStretch(1);
	top->addLayout(left);
	browser = new QTextBrowser();
	browser->setOpenLinks(false);
	top->addWidget(browser);
	QHBoxLayout *buttonLayout = new QHBoxLayout();
	rootButton = new QPushButton(tr("Set Search Root"));
	connect(rootButton, SIGNAL(clicked()), this, SLOT(setRootClicked()));
	buttonLayout->addWidget(rootButton);
	searchButton = new QPushButton(tr("Search Automatically"));
	connect(searchButton, SIGNAL(clicked()), this, SLOT(searchAuto()));
	buttonLayout->addWidget(searchButton);
	if ((buttons & QGLE_SOFT_DIALOG_SEARCH_MANUAL) != 0) {
		locButton = new QPushButton(tr("Manually Locate GhostScript"));
		connect(locButton, SIGNAL(clicked()), this, SLOT(locateManual()));
		buttonLayout->addWidget(locButton);
	}
	buttonLayout->addStretch(1);
	if ((buttons & QGLE_SOFT_DIALOG_SEARCH_ABORT) != 0) {
		abortButton = new QPushButton(tr("Abort"));
		connect(abortButton, SIGNAL(clicked()), mainWin, SLOT(abort()));
		buttonLayout->addWidget(abortButton);
	} else if ((buttons & QGLE_SOFT_DIALOG_SEARCH_CANCEL) != 0) {
		abortButton = new QPushButton(tr("Cancel"));
		connect(abortButton, SIGNAL(clicked()), this, SLOT(cancelClicked()));
		buttonLayout->addWidget(abortButton);
	}
	if (mainWin != NULL) {
		connect(browser, SIGNAL(anchorClicked(const QUrl&)), mainWin, SLOT(showURL(const QUrl&)));
	}
	okButton = new QPushButton(tr("Ok"));
	connect(okButton, SIGNAL(clicked()), this, SLOT(close()));
	okButton->setEnabled(false);
	okButton->setDefault(true);
	buttonLayout->addWidget(okButton);
	layout->addLayout(top);
	layout->addLayout(buttonLayout);
	setLayout(layout);
	QDesktopWidget* desk = QApplication::desktop();
	QRect size = desk->screenGeometry(this);
	resize(size.width()/2, size.height()/2);
	connect(&output, SIGNAL(print(const QString)), this, SLOT(print(const QString)));
	getGLEInterface()->setOutputStream(&output);
}

SoftwareLocateDialogue::~SoftwareLocateDialogue() {
}

void SoftwareLocateDialogue::showGhostScriptError(QString error, QString loc) {
	QString gswww = tr("http://www.ghostscript.com");
	QString guidewww = tr("http://glx.sourceforge.net/");
	#ifdef Q_WS_X11
		guidewww += tr("tut/linux.html");
	#endif
	#ifdef Q_OS_WIN32
		guidewww += tr("tut/windows.html");
	#endif
	#ifdef Q_WS_MAC
		guidewww += tr("downloads/mac.html");
	#endif
	QString err = tr("QGLE was unable to load the GhostScript software library, "
	                 "which it requires to render GLE's PostScript output.<br><br>"
	                 "Current location: '%1'.<br><br>"
	                 "Error: '%2'.<br><br>"
	                 "Make sure that GhostScript is correctly installed.<br><br>"
	                 "GhostScript is freely available"
	#ifdef Q_WS_X11
	                 " from your Linux distribution's software repository, or"
	#endif
	                 " from <a href=\"%3\">%3</a>.<br><br>"
	                 "More information is available in the <a href=\"%4\">GLE Installation Guide</a>.<br><br>"
	                 "Click 'Manually Locate' to manually locate the GhostScript library (%5), "
	                 "or 'Search Automatically' to make GLE search common locations for GhostScript.<br>"
	                 ).arg(loc).arg(error).arg(gswww).arg(guidewww).arg(QGLE::gsLibFileName());
	browser->setHtml(err);
}

void SoftwareLocateDialogue::disableAll() {
	QGLE::setEnabled(rootButton, false);
	QGLE::setEnabled(searchButton, false);
	QGLE::setEnabled(locButton, false);
	QGLE::setEnabled(abortButton, false);
}

void SoftwareLocateDialogue::enableOK(bool enable) {
	QGLE::setEnabled(okButton, enable);
}

void SoftwareLocateDialogue::searchAuto() {
	disableAll();
	okButton->setEnabled(false);
	QTextCursor cursor = browser->textCursor();
	cursor.movePosition(QTextCursor::End);
	browser->setTextCursor(cursor);
	browser->insertPlainText(tr("\n"));
	SoftwareLocateThread* thread = new SoftwareLocateThread(this, rootDir);
	connect(thread, SIGNAL(threadDone()), this, SLOT(threadDone()));
	thread->start();
}

void SoftwareLocateDialogue::tryGhostScriptLocation(QString loc) {
	QString error;
	if (mainWin->tryGhostScriptLocation(loc, error)) {
		found = true;
		browser->insertPlainText(tr("Found GhostScript: '%1', version: %2.\n").arg(loc).arg(mainWin->getGsLibVersion()));
		okButton->setEnabled(true);
	} else {
		browser->insertPlainText(tr("Location: '%1'.\n").arg(loc));
		browser->insertPlainText(tr("GhostScript not found: '%1'.\n").arg(error));
	}
}

void SoftwareLocateDialogue::locateManual() {
	okButton->setEnabled(false);
	QTextCursor cursor = browser->textCursor();
	cursor.movePosition(QTextCursor::End);
	browser->setTextCursor(cursor);
	browser->insertPlainText(tr("\n"));
	// don't use stock dialog; otherwise it's impossible to select a framework on macOS
	QFileDialog dialog(this, tr("Locate the GhostScript library"), "/", QGLE::libraryFilter());
	dialog.setFileMode(QFileDialog::AnyFile);
	if (dialog.exec()) {
		QStringList locs = dialog.selectedFiles();
		if (locs.size() > 0 && locs[0] != "") {
			tryGhostScriptLocation(locs[0]);
		}
	}
}

void SoftwareLocateDialogue::threadDone() {
	modified = true;
	QGLE::setEnabled(rootButton, true);
	QGLE::setEnabled(searchButton, true);
	QGLE::setEnabled(locButton, true);
	QGLE::setEnabled(abortButton, true);
	if (mainWin != NULL) {
		string gsloc = getGLEInterface()->getGhostScriptLocation();
		QString loc = QString::fromUtf8(gsloc.c_str());
		tryGhostScriptLocation(loc);
	} else {
		okButton->setEnabled(true);
	}
}

void SoftwareLocateDialogue::cancelClicked() {
	cancel = true;
	close();
}

void SoftwareLocateDialogue::setRootClicked() {
	rootDir = QFileDialog::getExistingDirectory(this, tr("Select Search Root Directory"), "/", QFileDialog::ShowDirsOnly);
	QTextCursor cursor = browser->textCursor();
	cursor.movePosition(QTextCursor::End);
	browser->setTextCursor(cursor);
	QString msg = tr("\nClick 'Search Automatically' to search root: %1.\n").arg(rootDir);
	print(msg);
}

void SoftwareLocateDialogue::print(const QString str) {
	browser->insertPlainText(str);
}

CrashRecoverDialogue::CrashRecoverDialogue(const QString& file) : QDialog(NULL)
{
//	http = NULL;
	crashLog = file;
	requestID = -1;
	setWindowIcon(QIcon(":images/gle_shadow.png"));
	setWindowTitle(tr("GLE Internal Error"));
	crashLogLines.append(QString("The following is the error code and memory location of the error."));
	crashLogLines.append(QString("We use this to find the line in GLE's source code where the error occurred."));
	crashLogLines.append(QString());
	QFile myfile(file);
    if (myfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QRegExp glefile("GLE file:\\s*'(.*)'");
		QTextStream in(&myfile);
		while (!in.atEnd()) {
			QString line = in.readLine();
			if (glefile.exactMatch(line)) gleFileName.setFile(glefile.capturedTexts()[1]);
			else crashLogLines.append(line);
		}
		myfile.close();
	}
	GLEInterface* gleInterface = GLEGetInterfacePointer();
	QString version = QString::fromUtf8(gleInterface->getGLEVersion().c_str());
	crashLogLines.append(QString("GLE version: %1.").arg(version));
	QDateTime time = QDateTime::currentDateTime();
	crashLogLines.append(QString("Time: %1.").arg(time.toString("MM/dd/yyyy hh:mm:ss")));
	QHBoxLayout *top = new QHBoxLayout();
	QVBoxLayout *left = new QVBoxLayout();
	QLabel* icon = new QLabel(this);
	icon->setPixmap(QPixmap(":/images/gle_shadow.png").scaled(QSize(64, 64)));
	left->addWidget(icon);
	left->addStretch(1);
	top->addLayout(left);
	QVBoxLayout *right = new QVBoxLayout();
	right->addWidget(new QLabel("GLE has encountered an internal error and needed to close.\n\n"
	                            "GLE has created a report of this error.\n\n"
						        "Please send this report to the developers by clicking the send button below.\n"
						        "This will help improve the robustness and quality of future GLE versions."));
	right->addWidget(new QLabel("Please provide a brief description of what you where doing (optional)."));
	right->addWidget(description = new QTextEdit());
	right->addWidget(new QLabel("Please enter your e-mail address (optional).\n"
	                            "We may contact you if we have a solution or further questions."));
	right->addWidget(email = new QLineEdit());
	QSettings settings("gle", "qgle");
	email->setText(settings.value("application/email").toString());
	right->addWidget(new QLabel("The crash may be due to a specific construct in your GLE script."));
	right->addWidget(includeScript = new QCheckBox("Include GLE script ("+gleFileName.fileName()+")"));
	includeScript->setChecked(true);
	right->addWidget(new QLabel("Thank you for your cooperation."));
	top->addLayout(right);
	cancelButton = new QPushButton(tr("Cancel"));
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancelReport()));
	QHBoxLayout *buttonLayout = new QHBoxLayout();
	buttonLayout->addWidget(cancelButton);
	buttonLayout->addStretch(1);
	QPushButton *viewButton = new QPushButton(tr("View Report"));
	buttonLayout->addWidget(viewButton);
	connect(viewButton, SIGNAL(clicked()), this, SLOT(viewReport()));
	sendButton = new QPushButton(tr("Send Report"));
	sendButton->setDefault(true);
	buttonLayout->addWidget(sendButton);
	connect(sendButton, SIGNAL(clicked()), this, SLOT(sendReport()));
	QVBoxLayout *layout = new QVBoxLayout();
	layout->addLayout(top);
	layout->addLayout(buttonLayout);
	setLayout(layout);
}

void CrashRecoverDialogue::saveEmail()
{
	QSettings settings("gle", "qgle");
	settings.setValue("application/email", email->text());
}

void CrashRecoverDialogue::cancelReport() {
	saveEmail();
	close();
}

void CrashRecoverDialogue::foundFile(QStringList* list, const QDir& dir, const QString& file)  {
	QFileInfo info(dir, file);
	if (info.exists()) {
		list->append(info.absoluteFilePath());
	}
}

QString CrashRecoverDialogue::createReport()
{
	QString report = crashLogLines.join("\n");
	report += QString("\n\n%% Description:\n%1\n").arg(description->toPlainText());
	report += QString("\n%% EMail:\n%1\n").arg(email->text());
	if (includeScript->isChecked()) {
		QStringList lines;
		QStringList files;
		if (gleFileName.fileName() != "")
		{
			QDir mydir = gleFileName.absoluteDir();
			QFile myfile(gleFileName.absoluteFilePath());
			if (myfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
				lines.append(QString());
				lines.append(tr("%% File: '%1'").arg(gleFileName.fileName()));
				QRegExp glefile("^\\s*include\\s+\\\"([^\\\"]+)\\\"");
				QRegExp csvfile("^\\s*data\\s+\\\"([^\\\"]+)\\\"");
				QTextStream in(&myfile);
				while (!in.atEnd()) {
					QString line = in.readLine();
					if (glefile.indexIn(line) != -1) {
						foundFile(&files, mydir, glefile.capturedTexts()[1]);
					}
					if (csvfile.indexIn(line) != -1) {
						foundFile(&files, mydir, csvfile.capturedTexts()[1]);
					}
					lines.append(line);
				}
				myfile.close();
			}
			for (int i = 0; i < files.size(); i++) {
				QFileInfo info(files.at(i));
				QFile subfile(info.absoluteFilePath());
				if (subfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
					lines.append(QString());
					lines.append(tr("%% File: '%1'").arg(info.fileName()));
					QTextStream in(&subfile);
					while (!in.atEnd()) {
						lines.append(in.readLine());
					}
					subfile.close();
				}
			}
		}
		lines.append(QString());
		lines.append(tr("%% End"));
		report += lines.join("\n");
	}
	if (report.length() > 50000) {
		report = report.left(50000);
	}
	return report;
}

void CrashRecoverDialogue::viewReport()
{
	QString report = createReport();
	QDialog viewReportDial(this);
	viewReportDial.setWindowTitle("View Report");
	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(new QLabel(tr("Report:")));
	QTextEdit* txt = new QTextEdit();
	txt->setReadOnly(true);
	txt->insertPlainText(report);
	QTextCursor cursor = txt->textCursor();
	cursor.movePosition(QTextCursor::Start);
	txt->setTextCursor(cursor);
	layout->addWidget(txt);
	QPushButton *okButton = new QPushButton(tr("OK"));
	connect(okButton, SIGNAL(clicked()), &viewReportDial, SLOT(close()));
	QHBoxLayout *buttonLayout = new QHBoxLayout();
	buttonLayout->addStretch(1);
	buttonLayout->addWidget(okButton);
	layout->addLayout(buttonLayout);
	viewReportDial.setLayout(layout);
	QRect size = QApplication::desktop()->screenGeometry(&viewReportDial);
	viewReportDial.resize(size.width()/2, size.height()/2);
	viewReportDial.exec();
}

void CrashRecoverDialogue::requestFinished(int id, bool error)
{
	if (id == requestID)
	{
		requestID = -1;
		buffer->seek(0);
		QString result_txt = buffer->readAll();
		delete buffer;
		QRegExp redir("%%\\s+GLE-Redirect:\\s+(\\S+)\\s+(\\S+)");
		if (!error && redir.indexIn(result_txt) != -1)
		{
			QString url = redir.capturedTexts()[1];
			QString script = redir.capturedTexts()[2];
			performRequest(url, script);
		}
		else
		{
			QString err_s = ""; // QString err_s = http->errorString();
			QDialog mydial(this);
			mydial.setWindowTitle("GLE Internal Error");
			QVBoxLayout *layout = new QVBoxLayout();
			layout->addWidget(new QLabel(tr("Result of sending report:")));
			QTextBrowser* txt = new QTextBrowser();
			txt->setOpenLinks(false);
			txt->setReadOnly(true);
			if (error) txt->insertPlainText(err_s);
			else txt->setPlainText(result_txt);
			layout->addWidget(txt);
			QPushButton *okButton = new QPushButton(tr("Close"));
			connect(okButton, SIGNAL(clicked()), &mydial, SLOT(close()));
			QHBoxLayout *buttonLayout = new QHBoxLayout();
			buttonLayout->addStretch(1);
			buttonLayout->addWidget(okButton);
			layout->addLayout(buttonLayout);
			mydial.setLayout(layout);
			QRect size = QApplication::desktop()->screenGeometry(&mydial);
			mydial.resize(size.width()/3, size.height()/3);
			mydial.exec();
			close();
		}
	}
}

void CrashRecoverDialogue::performRequest(QString url, QString script)
{
/*	http->setHost(url, 80);
	QHttpRequestHeader header("POST", script);
	header.setValue("Host", url);
	header.setContentType("application/x-www-form-urlencoded");
	QString report = createReport();
	GLEInterface* gleInterface = GLEGetInterfacePointer();
	QByteArray version = QUrl::toPercentEncoding(QString::fromUtf8(gleInterface->getGLEVersion().c_str()));
	QString tosend_s = tr("code=5568&version=%1&report=").arg(QString::fromUtf8(version));
	tosend_s += QString::fromUtf8(QUrl::toPercentEncoding(report));
	tosend_s += "\n";
	QByteArray tosend = tosend_s.toUtf8().constData();
	buffer = new QBuffer();
	buffer->open(QBuffer::ReadWrite);
	requestID = http->request(header, tosend, buffer); */
}

void CrashRecoverDialogue::sendReport()
{
	cancelButton->setEnabled(false);
	sendButton->setEnabled(false);
	saveEmail();
//	http = new QHttp(this);
//	connect(http, SIGNAL(requestFinished(int,bool)), this, SLOT(requestFinished(int,bool)));
	performRequest("glx.sourceforge.net", "/gle-crash-report.php");
}
