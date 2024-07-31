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
 * mainwindow.cpp: The class implementation for GLEMainWindow. *
 ***************************************************************/

#include <QtGui>
#include <QtDebug>
#include <QMimeData>
#include <math.h>

#include "mainwindow.h"
#include "qgle_statics.h"
#include "settings.h"
#include "settings_dialogue.h"
#include "qgs.h"
#include "about.h"
#include "newfile.h"
#include "consolewindow.h"
#include "dialogues.h"
#include "3dviewer.h"
#include "../gle/cutils.h"
#include "../gle/gle-block.h"
#include "../gle/surface/gsurface.h"
// #include "downloader.h"

#include <iostream>
using namespace std;

void SetQGLEScriptNameBackup(QString name);

//! Class Constructor
GLEMainWindow::GLEMainWindow(int argc, char *argv[])
{
	// Initially enabled state is false
	enabledState = false;

	// Resize to an initial default size
	resize(500,300);

	// Set the Window Icon
	setWindowIcon(QIcon(":images/gle.png"));

	// Coordinate display default
	coordView = CoordinateCart;
	lastPoint = QPointF(0,0);
	evaluatorDialog = NULL;

	// GhostScript version number
	GsLibVersionNumber = "?";

	// Create the gle interface
	gleInterface = GLEGetInterfacePointer();
	gleScript = NULL;

	// Initialise temp file names
	// Should be unique names - otherwise can't run multiple QGLE instances
	tempFiles[PreviewModeOutput] = createTempFile(tr(".out"));
	tempFiles[EditModeOutput] = createTempFile(tr(".out"));
	tempFiles[NewGLE] = createTempFile(tr(".gle"));

	// Create the drawing area in a scroll widget.  For high resolutions,
	// this may be undesirable as the whole image must be rendered by GS
	drawingArea = new GLEDrawingArea(gleInterface, this);
	scrollArea = new QScrollArea(this);
	scrollArea->setWidget(drawingArea);

	splitWindow = new QSplitter(Qt::Vertical, this);
	splitWindow->addWidget(scrollArea);
	consoleWindow = new ConsoleWindow(this, splitWindow);
	splitWindow->addWidget(consoleWindow);
	splitWindow->setStretchFactor(0, 1);
	splitWindow->setStretchFactor(1, 0);
	gleInterface->setOutputStream(consoleWindow->getOutput());
	gleInterface->initializeGLE("qgle", argc, argv);

	QList<int> splitSizes;
	splitSizes.append(50);
	splitSizes.append(50);
	splitWindow->setSizes(splitSizes);

	setCentralWidget(splitWindow);

	// Create the component parts
	createActions();
	createMenus();
	createStatusBar();
	createToolBars();
	createEditModeToolBars();

	qRegisterMetaType<QList<GLEDrawingObject *> >("QList<GLEDrawingObject *>");

	// Read the application settings
	settings = new GLESettings(this);
	settings->readAll();
	// We always resize, we just only save the size if requested
	resize(settings->size());
	move(settings->position());
	splitWindow->restoreState(settings->splitterPosition());
	consoleWindow->setAutoShowSize(settings->getConsoleWindowAutoShowSize());
	createDockWindows();
	enableEditModeToolBars(false);

	// Connect signals
	connect(settings, SIGNAL(polarSnapStartAngleChanged(double)),
	        drawingArea, SLOT(setPolarSnapStartAngle(double)));
	connect(settings, SIGNAL(polarSnapIncAngleChanged(double)),
	        drawingArea, SLOT(setPolarSnapIncAngle(double)));
	connect(drawingArea, SIGNAL(selectionChanged(QList<GLEDrawingObject *>)),
	        propertyEditor, SLOT(objectsSelected(QList<GLEDrawingObject *>)));
	connect(propertyEditor, SIGNAL(propertiesHaveChanged()),
	        drawingArea, SLOT(update()));

	// Initialise snapping
	if (settings->polarSnapOnStart())
		polarSnapAct->setChecked(true);
	if (settings->polarTrackOnStart())
		polarTrackAct->setChecked(true);
	if (settings->gridSnapOnStart())
		gridSnapAct->setChecked(true);
	if (settings->osnapOnStart())
		osnapAct->setChecked(true);
	if (settings->orthoSnapOnStart())
		orthoSnapAct->setChecked(true);

	// As necessary, start the file monitor
	fileMonitorCount = 0;
	fileMonitorTimer = new QTimer(this);
	connect(fileMonitorTimer, SIGNAL(timeout()), this, SLOT(checkForFileUpdates()));
	updateFileMonitor(settings->monitorOpenFile());

	// The following line is important as it allows
	// QImage objects to be passed by the signal-slot
	// mechanism.
	qRegisterMetaType<QImage>("QImage");
	qRegisterMetaType<GLEFileInfo>("GLEFileInfo");

	// Initialise the serverRunning variable
	serverRunning = false;

	// Start in preview mode
	currentMode = GLESettings::PreviewMode;
	oldMode = currentMode;

	serverInPreviewMode = true;
	serverAct->setChecked(true);
	initRenderThread();

	// Setting connections

	// If the grid is changed, update the drawing
	connect(settings, SIGNAL(gridChanged(QPointF)),
			drawingArea, SLOT(setGrid(QPointF)));
	// If the file monitor is switched on or off,
	// sort this out
	connect(settings, SIGNAL(monitorFileChanged(bool)),
			this, SLOT(updateFileMonitor(bool)));

	// If a tool is selected, notify the drawing
	connect(this, SIGNAL(toolSelected(int)),
			drawingArea, SLOT(setTool(int)));
	// If a new image is rendered, update the drawing
	connect(this, SIGNAL(imageChanged(QImage)),
			drawingArea, SLOT(updateDisplay(QImage)));
	// If a point is drawn, set the base point used for
	// relative coordinates
	connect(drawingArea, SIGNAL(basePointChanged(QPointF)),
			this, SLOT(setRelativeBasePoint(QPointF)));
	// If the drawing area requests a status bar message,
	// display it
	connect(drawingArea, SIGNAL(updateStatusBar(QString)),
			this, SLOT(statusBarMessage(QString)));
	// If the dirty flag is set or unset, enable or disable
	// the 'save' button accordingly
	connect(drawingArea, SIGNAL(dirtyFlagChanged(bool)),
			this, SLOT(setSaveEnable(bool)));
	// If the dpi changes, update the status bar display
	connect(drawingArea, SIGNAL(dpiChanged(double)),
			this, SLOT(updateDPIDisplay(double)));
	// Allow the drawing area to disable all tools
	connect(drawingArea, SIGNAL(disableTools()),
			this, SLOT(noToolEnable()));

	// Get the default resolution
	drawingArea->setDPI((double) settings->dpi());

	// Update the grid spacing
	drawingArea->setGrid(settings->grid());

	// Accept keyboard events
	setFocusPolicy(Qt::StrongFocus);
	setFocus(Qt::OtherFocusReason);

	// Drag and drop
	setAcceptDrops(true);

	// Make sure are tools are enabled correctly
	updateEnableActions(true);

	// Display a title on the title bar
	updateWindowTitle();

	// Check for arguments
	QRegExp rx = QGLE::fileRegExp();

	for(int i=0;i<argc;i++)
	{
		if (rx.exactMatch(QString(argv[i])))
		{
			QFileInfo filename(rx.capturedTexts()[2]);
			openFile(filename.absoluteFilePath(), true);
			break;
		}
	}
}

QString GLEMainWindow::createTempFile(QString ext)
{
	// Rely on same mechanism GLE also uses internally
	string temp = gleInterface->getTempFile();
	return QString::fromUtf8(temp.c_str()) + ext;
}

// Switch on or off the file monitor
void GLEMainWindow::updateFileMonitor(bool state)
{
	if (state)
	{
		if (!fileMonitorTimer->isActive())
		{
			// One second timer
			fileMonitorTimer->start(500);
		}
	}
	else
	{
		if (fileMonitorTimer->isActive())
		{
			fileMonitorTimer->stop();
		}
	}
	// used to wait another "tick" before auto-reloading
	fileMonitorCount = 0;
}

bool GLEMainWindow::askAnywayBeforeReload()
{
	return false;
}

// Check whether the file has changed
void GLEMainWindow::checkForFileUpdates()
{
	int answer = 1;
	GLEFileInfo* currFile = getCurrentFile();
	if (enabledState && currFile->hasChanged())
	{
		if (settings->monitorAutoReloadFile() && !askAnywayBeforeReload())
		{
			if (fileMonitorCount < 1)
			{
				// wait another "tick" to make sure file saved correctly
				// don't want to load partially saved files
				fileMonitorCount++;
				return;
			}
		}
		// reset counter for next iteration
		fileMonitorCount = 0;

		// If the file has changed, update the stored time stamp and
		// stop the timer while we ask the user what to do.
		fileMonitorTimer->stop();

		// Ask the user whether to reload and, if we have any objects, should
		// we keep them?
		QString msg = QString(tr("File %1 has changed on disk.")).arg(currFile->primaryFile());

		if (drawingArea->isDirty())
		{
			QString msg2 = QString(tr("%1\n\nWarning: you have also modified this file in QGLE. If you reload then these changes will be lost.")).arg(msg);
			answer = QMessageBox::question(this, tr("%1 File Monitor").arg(APP_NAME),
					msg2, tr("Reload file"),
					tr("Ignore"),
					QString(),
					0,1);
		}
		else if (settings->monitorAutoReloadFile() && !askAnywayBeforeReload())
		{
			answer = 0;
		}
		else if (currFile->isEPS() || currFile->isGLE())
		{
			answer = QMessageBox::question(this, tr("%1 File Monitor").arg(APP_NAME),
					msg, tr("Reload file"),
					tr("Ignore"),
					QString(),
					0,1);
		}
		bool isOpen = false;
		switch(answer)
		{
			case 0:
				openFile(currFile->primaryFile());
				isOpen = true;
				break;
			default:
				break;
		}

		// Restart the timer
		currFile->updateTimeStamp();
		if (!isOpen) fileMonitorTimer->start();
	}

}

// Accept URI lists that have been dragged over the application
void GLEMainWindow::dragEnterEvent(QDragEnterEvent *event)
{
	if (event->mimeData()->hasFormat("text/uri-list"))
		event->acceptProposedAction();
}

// Handle dropped URIs
void GLEMainWindow::dropEvent(QDropEvent *event)
{
	// This will need to check that the file is a GLE or EPS
	// file and then try to open it, by stripping off the file:// part
	// and calling openFile(fileName).  Worth checking that the file
	// exists and is readable while we're at it.

	// We're only interested in the first URI as this isn't a
	// multi-document interface
	QString uri = event->mimeData()->urls().at(0).toString();

	QRegExp rx = QGLE::fileRegExp();

	if (rx.exactMatch(uri))
	{
		openFile(rx.capturedTexts()[2]);
	}
	else
		QMessageBox::information(this,
				"Drop Event",
				"Dropped file cannot be opened",
				QMessageBox::Ok);


	event->acceptProposedAction();
}

// User pressed a key
// Keyboard shortcuts are based on those used in AutoCAD
void GLEMainWindow::keyPressEvent(QKeyEvent *event)
{
	// Since we haven't any keyboard shortcuts that
	// use modifiers, if there are any modifiers,
	// ignore the key press
	if (event->modifiers() != Qt::NoModifier)
		event->ignore();

	switch (event->key())
	{
		// Delete or escape keys are handled by the drawing itself
		case Qt::Key_Delete:
		case Qt::Key_Escape:
			drawingArea->hitKey(event->key(), event->modifiers());
			event->accept();
			break;

		case Qt::Key_F9:
			// F9 is used to toggle grid snap
			gridSnapAct->toggle();
			event->accept();
			break;

		case Qt::Key_F3:
			// F3 is used to toggle OSNAP: snapping to objects
			osnapAct->toggle();
			event->accept();
			break;

		case Qt::Key_F10:
			// F10 is used to toggle polar snap: customisable snapping
			polarSnapAct->toggle();
			event->accept();
			break;

		case Qt::Key_F12:
			// F12 is used to toggle polar track: multiple polar snaps
			polarTrackAct->toggle();
			event->accept();
			break;

		case Qt::Key_F8:
			// F8 is used to toggle ortho snap: 0 or 90 degree lines
			orthoSnapAct->toggle();
			event->accept();
			break;

		case Qt::Key_F7:
			// F7 is used to toggle grid visibility
			gridAct->toggle();
			event->accept();
			break;

		case Qt::Key_F1:
			// F1 shows the about box
			aboutAct->trigger();
			event->accept();
			break;

		case Qt::Key_Equal:
		case Qt::Key_Plus:
			// The equals key is used to zoom in
			// as is plus.
			zoomInAct->trigger();
			event->accept();
			break;

		case Qt::Key_Minus:
			// The minus key is used to zoom out
			zoomOutAct->trigger();
			event->accept();
			break;

		case Qt::Key_F6:
			// F6 cycles through the various coordinate views
			switch (coordView)
			{
				case CoordinateOff:
					coordView = CoordinateCart;
					mousePosition->setVisible(true);
					break;
				case CoordinateCart:
					coordView = CoordinatePolar;
					mousePosition->setVisible(true);
					break;
				case CoordinatePolar:
					mousePosition->setVisible(true);
					coordView = CoordinateRelCart;
					break;
				case CoordinateRelCart:
					mousePosition->setVisible(true);
					coordView = CoordinateRelPolar;
					break;
				case CoordinateRelPolar:
					mousePosition->setVisible(false);
					coordView = CoordinateOff;
					break;
			}
			updateMousePosition();
			event->accept();
			break;

		case Qt::Key_Shift:
			drawingArea->modifierToggle(GLEDrawingArea::ShiftKey, true);
			break;

		case Qt::Key_Control:
			drawingArea->modifierToggle(GLEDrawingArea::CtrlKey, true);
			break;

		case Qt::Key_Alt:
			drawingArea->modifierToggle(GLEDrawingArea::AltKey, true);
			break;

		default:
			event->ignore();
	}

}

void GLEMainWindow::keyReleaseEvent(QKeyEvent *event)
{
	// Since we haven't any keyboard shortcuts that
	// use modifiers, if there are any modifiers,
	// ignore the key press
	if (event->modifiers() != Qt::NoModifier)
		event->ignore();

	switch (event->key())
	{
		case Qt::Key_Shift:
			drawingArea->modifierToggle(GLEDrawingArea::ShiftKey, false);
			break;

		case Qt::Key_Control:
			drawingArea->modifierToggle(GLEDrawingArea::CtrlKey, false);
			break;

		case Qt::Key_Alt:
			drawingArea->modifierToggle(GLEDrawingArea::AltKey, false);
			break;

		default:
			event->ignore();
	}
}


// Toggle the status of the server
void GLEMainWindow::serverToggle(bool switchOn)
{
	if (switchOn && (!serverRunning))
		startServer();
	else if ((!switchOn) && serverRunning)
		stopServer();
}

// Initialize render thread
void GLEMainWindow::initRenderThread()
{
	// Remember to update the server as well if it's changed anywhere else
	renderThread = new GLERenderThread(this);

	// Connect server signals
	connect(renderThread, SIGNAL(serverMessage(QString)),
			this, SLOT(updateServerStatus(QString)));

	connect(renderThread, SIGNAL(serverError(QString)),
			this, SLOT(showConsoleError(QString)));

	// Notify the rendering code that an image is ready
	connect(renderThread, SIGNAL(renderComplete(QImage)),
			this, SLOT(renderComplete(QImage)));

	// Notify drawing area of resolution changes from the server
	connect(renderThread, SIGNAL(dpiChanged(double)),
			drawingArea, SLOT(setDPI(double)));
}

// Start the server
void GLEMainWindow::startServer()
{
	// Initialise the GS library
	initLibGS();

	// Remember to update the server as well if it's changed anywhere else
	serverThread = new GLEServerThread(this, settings->port());

	// Connect server signals
	connect(serverThread, SIGNAL(serverMessage(QString)),
			this, SLOT(updateServerStatus(QString)));

	// Notify QGLE that a new file has been opened using "gle -p"
	connect(serverThread, SIGNAL(gleMinusPRunned(QString)),
			this, SLOT(openFile(QString)));

	// Initialise the server
	serverThread->initialise();

	serverRunning = true;
}

// Stop the server
void GLEMainWindow::stopServer()
{
	// This should clean it all up in theory
	if(serverRunning){
		delete(serverThread);
		serverRunning = false;
	}
}

bool GLEMainWindow::tryGhostScriptLocation(const QString location, QString& error) {
	GSLibFunctions* fct = GSLibFunctions::getInstance();
	if (fct->loadLibrary(location, error) == 0) {
		GsLibVersionNumber = fct->getVersion();
		return true;
	} else {
		return false;
	}
}

// Initialise GS
void GLEMainWindow::initLibGS()
{
	QString error;
	bool found = false;
	GSLibFunctions* fct = GSLibFunctions::getInstance();
	// First try the location given in the current settings
	QString location = settings->getLibGSLocation();
	if (fct->loadLibrary(location, error) != 0) {
		// Next try the location proposed by GLE
		string gsloc = gleInterface->getGhostScriptLocation();
		location = QString::fromUtf8(gsloc.c_str());
		if (fct->loadLibrary(location, error) == 0) {
			found = true;
		}
	} else {
		found = true;
	}
	// If this does not work, consult the user
	if (!found) {
		SoftwareLocateDialogue dial(this, gleInterface, QGLE_SOFT_DIALOG_SEARCH_MANUAL | QGLE_SOFT_DIALOG_SEARCH_ABORT);
		dial.setWindowTitle("QGLE Error");
		dial.showGhostScriptError(error, location);
		dial.exec();
		// Don't forget to restore the console output
		restoreConsoleOutput();
		if (dial.isConfigModified()) {
			gleInterface->saveRCFile();
		}
	}
	// GSLibFunctions might discover the real location
	settings->setLibGSLocation(fct->libGSLocation());
	GsLibVersionNumber = fct->getVersion();
}

// Called when QGLE is closed
void GLEMainWindow::closeEvent(QCloseEvent *event)
{

	// Check whether to save changes
	if (drawingArea->isDirty())
	{
		int reply = QMessageBox::question(this, tr("Quit QGLE"),
				tr("Save changes to drawing?"),
				QMessageBox::Yes,
				QMessageBox::No,
				QMessageBox::Cancel);

		if (reply == QMessageBox::Yes)
			save();
		else if (reply == QMessageBox::No)
		{
			drawingArea->clearDirty();
		}
		else
		{
			event->ignore();
			return;
		}
	}

	// If the window size and position should be saved, update them
	if (settings->storeSize())
	{
		settings->setPosition(pos());
		settings->setSize(size());
		settings->setDrawingAreaSize(getScrollAreaSize());
		settings->setConsoleWindowAutoShowSize(consoleWindow->getAutoShowSize());
		if (currentMode == GLESettings::EditMode) {
			int sizes[2];
			sizes[0] = propertyEditor->height();
			sizes[1] = objectBlocksList->height();
			QByteArray winState((const char*)sizes, sizeof(int)*2);
			settings->setMainWindowState(winState);
		}
		QByteArray state = splitWindow->saveState();
		settings->setSplitterPosition(state);
	}
	// Store the settings
	settings->writeAll();

	// Delete all temporary files
	for (QHash<int,QString>::iterator it = tempFiles.begin(); it != tempFiles.end(); ++it)
	{
		QDir dir;
		QString file = it.value();
		dir.remove(file);
		QString fileNoExt = file.left(file.length()-4);
		#ifdef Q_OS_WIN32
			dir.remove(fileNoExt+tr(".tmp"));
		#else
			dir.remove(fileNoExt);
		#endif
	}

	event->accept();
}

void GLEMainWindow::updateWindowTitle()
{
	GLEFileInfo* currFile = getCurrentFile();
	bool isGLE = currFile->isGLE();
	bool has = isGLE || currFile->isEPS();
	QString title(APP_NAME);
	if (has) {
		title += tr(" - ") + currFile->primaryFile();
	}
	// Always include "[*]", it is treated special by Qt and only filled in when actually modified
	title += tr(" [*]");
	setWindowTitle(title);
}

void GLEMainWindow::updateEnableActions(bool enable)
{
	bool isGLE = getCurrentFile()->isGLE();
	bool has = isGLE || getCurrentFile()->isEPS();
	bool prevMode = (currentMode == GLESettings::PreviewMode);
	bool figure = (previewPageSize->currentIndex() == 0);
	enabledState = enable && has;
	editorAct->setEnabled(has);
	copyBitmapAct->setEnabled(has);
	if (copyPDFAct != NULL) copyPDFAct->setEnabled(has);
	if (copyEMFAct != NULL) copyEMFAct->setEnabled(has);
	saveAsAct->setEnabled(has && enable);
	saveAct->setEnabled(has && drawingArea->isDirty() && enable);
	exportAct->setEnabled(isGLE && enable);
	emulateVersion->setEnabled(isGLE && prevMode && enable);
	previewPageSize->setEnabled(isGLE && prevMode && enable);
	previewModeAct->setEnabled(has && enable);
	editModeAct->setEnabled(isGLE && figure && enable);
	zoomInAct->setEnabled(has && enable);
	zoomOutAct->setEnabled(has && enable);
	zoomAutoAct->setEnabled(has && enable);
	openAct->setEnabled(enable);
	newAct->setEnabled(enable);
	evaluatorAct->setEnabled(enable);
	ghostScriptLogWindowAct->setEnabled(enable);
	view3DAct->setEnabled(enable && gleInterface->getSurface()->z != NULL);
	pointerToolAct->setEnabled(enable);
	moveToolAct->setEnabled(enable);
	lineToolAct->setEnabled(enable);
	tanLineToolAct->setEnabled(enable);
	perpLineToolAct->setEnabled(enable);
	circleToolAct->setEnabled(enable);
	ellipseToolAct->setEnabled(enable);
	arcToolAct->setEnabled(enable);
	textToolAct->setEnabled(enable);
	bool reloadActEnable = has && enable && prevMode;
	if (!reloadActEnable) {
		if (has && prevMode) {
			reloadAct->setIcon(reloadIconRed);
		} else {
			reloadAct->setIcon(reloadIconBlack);
		}
	}
	reloadAct->setEnabled(reloadActEnable);
}

void GLEMainWindow::clearConsoleWindow()
{
	consoleWindow->clear();
}

void GLEMainWindow::restoreConsoleOutput()
{
	gleInterface->setOutputStream(consoleWindow->getOutput());
}

void GLEMainWindow::createActions()
{
	// Create the actions for each menu entry and connect them
	// to an appropriate slot

	// Action causing the current bitmap to be copied to the clipboard
	copyBitmapAct = new QAction(tr("&Copy as Bitmap"), this);
	copyBitmapAct->setStatusTip(tr("Copy current diagram as bitmap to clipboard"));
	connect(copyBitmapAct, SIGNAL(triggered()), this, SLOT(copyBitmap()));

	//! Action causing the figure to be copied to the clipboard as PDF
	copyPDFAct = NULL;
	#ifdef Q_WS_MAC
		copyPDFAct = new QAction(tr("&Copy as PDF"), this);
		copyPDFAct->setStatusTip(tr("Copy current diagram as PDF to clipboard"));
		connect(copyPDFAct, SIGNAL(triggered()), this, SLOT(copyPDF()));
	#endif

	//! Action causing the figure to be copied to the clipboard as EMF
	copyEMFAct = NULL;
	#ifdef Q_OS_WIN32
		copyEMFAct = new QAction(tr("&Copy as EMF"), this);
		copyEMFAct->setStatusTip(tr("Copy current diagram as EMF to clipboard"));
		connect(copyEMFAct, SIGNAL(triggered()), this, SLOT(copyEMF()));
	#endif

	// Quit the application
	quitAct = new QAction(tr("&Quit"), this);
	quitAct->setShortcut(Qt::Key_Q | Qt::ControlModifier);
	quitAct->setStatusTip(tr("Exit QGLE"));
	connect(quitAct, SIGNAL(triggered()),
			this, SLOT(close()));

	// Create a new file
	newAct = new QAction(QIcon(":images/new.png"), tr("&New"), this);
	newAct->setShortcut(Qt::Key_N | Qt::ControlModifier);
	newAct->setStatusTip(tr("Create a new file"));
	connect(newAct, SIGNAL(triggered()),
			this, SLOT(newFile()));

	// Open a specified file
	openAct = new QAction(QIcon(":images/open.png"), tr("&Open"), this);
	openAct->setShortcut(Qt::Key_O | Qt::ControlModifier);
	openAct->setStatusTip(tr("Open an existing file"));
	connect(openAct, SIGNAL(triggered()),
			this, SLOT(openFile()));

	// Open a specified file in a new window
	openInNewWindowAct = new QAction(QIcon(":images/open.png"), tr("Open in New &Window"), this);
	openInNewWindowAct->setStatusTip(tr("Open an existing file in a new window"));
	connect(openInNewWindowAct, SIGNAL(triggered()),
			this, SLOT(openFileInNewWindow()));

   browseAct = new QAction(tr("&Browse Script Folder"), this);
   browseAct->setStatusTip(tr("Browse the folder that contains the GLE script"));
	connect(browseAct, SIGNAL(triggered()),
			this, SLOT(browseScriptFolder()));

	// Save the file
	saveAct = new QAction(QIcon(":images/save.png"), tr("&Save"), this);
	saveAct->setShortcut(Qt::Key_S | Qt::ControlModifier);
	saveAct->setStatusTip(tr("Save changes"));
	saveAct->setEnabled(false);
	connect(saveAct, SIGNAL(triggered()),
			this, SLOT(save()));

	// Save the file under a new name
	saveAsAct = new QAction(tr("Save &As"), this);
	saveAsAct->setStatusTip(tr("Save under a new name"));
	saveAsAct->setEnabled(false);
	connect(saveAsAct, SIGNAL(triggered()),
			this, SLOT(saveAs()));

	exportAct = new QAction(tr("&Export"), this);
	exportAct->setShortcut(Qt::Key_E | Qt::ControlModifier);
	exportAct->setStatusTip(tr("Export file"));
	exportAct->setEnabled(false);
	connect(exportAct, SIGNAL(triggered()),
			this, SLOT(openExportDialogue()));

	printAct = new QAction(tr("&Print"), this);
	printAct->setStatusTip(tr("Print file"));
	printAct->setEnabled(false);
	connect(printAct, SIGNAL(triggered()),
			this, SLOT(openPrintDialogue()));

	// Show an about box
	aboutAct = new QAction(tr("&About"), this);
	aboutAct->setStatusTip(tr("Show information about QGLE"));
	connect(aboutAct, SIGNAL(triggered()),
			this, SLOT(about()));

	gleSiteAct = new QAction(tr("&Website"), this);
	gleSiteAct->setStatusTip(tr("Open GLE website"));
	connect(gleSiteAct, SIGNAL(triggered()), this, SLOT(showGLEWebsite()));

	gleManualAct = new QAction(tr("&Manual"), this);
	gleManualAct->setShortcut(Qt::Key_F1);
	gleManualAct->setStatusTip(tr("Open GLE manual"));
	connect(gleManualAct, SIGNAL(triggered()), this, SLOT(showGLEManual()));

	// Toggle the grid visibility
	gridAct = new QAction(QIcon(":images/grid.png"), tr("Grid"), this);
	gridAct->setStatusTip(tr("Toggle the grid on and off"));
	gridAct->setCheckable(true);
	gridAct->setChecked(false);
	connect(gridAct, SIGNAL(toggled(bool)),
			drawingArea, SLOT(gridToggle(bool)));

	// Toggle grid snap
	gridSnapAct = new QAction(QIcon(":images/grid_snap.png"), tr("Snap Grid"), this);
	gridSnapAct->setStatusTip(tr("Toggle grid snap on and off"));
	gridSnapAct->setCheckable(true);
	gridSnapAct->setChecked(false);
	connect(gridSnapAct, SIGNAL(toggled(bool)),
			drawingArea, SLOT(gridSnapToggle(bool)));

	// Toggle osnap
	osnapAct = new QAction(QIcon(":images/osnap.png"), tr("Snap to Objects"), this);
	osnapAct->setStatusTip(tr("Toggle OSNAP on and off"));
	osnapAct->setCheckable(true);
	osnapAct->setChecked(false);
	connect(osnapAct, SIGNAL(toggled(bool)),
			drawingArea, SLOT(osnapToggle(bool)));

	// Toggle orthoSnap
	orthoSnapAct = new QAction(QIcon(":images/orthosnap.png"), tr("Snap to axes"), this);
	orthoSnapAct->setStatusTip(tr("Toggle ORTHO on and off"));
	orthoSnapAct->setCheckable(true);
	orthoSnapAct->setChecked(false);
	connect(orthoSnapAct, SIGNAL(toggled(bool)),
			drawingArea, SLOT(orthoSnapToggle(bool)));

	// Toggle polar snap
	polarSnapAct = new QAction(QIcon(":images/polarsnap.png"), tr("Polar snap"), this);
	polarSnapAct->setStatusTip(tr("Toggle customisable snaps on and off"));
	polarSnapAct->setCheckable(true);
	polarSnapAct->setChecked(false);
	connect(polarSnapAct, SIGNAL(toggled(bool)),
			this, SLOT(polarSnapToggle(bool)));

	// Toggle polar track
	polarTrackAct = new QAction(QIcon(":images/polartrack.png"), tr("Polar tracking"), this);
	polarTrackAct->setStatusTip(tr("Toggle multiple customisable snaps on and off"));
	polarTrackAct->setCheckable(true);
	polarTrackAct->setChecked(false);
	connect(polarTrackAct, SIGNAL(toggled(bool)),
			this, SLOT(polarTrackToggle(bool)));

	// Toggle whether the server is running
	serverAct = new QAction(QIcon(":images/server.png"), tr("Server"), this);
	serverAct->setStatusTip(tr("Start or stop the server"));
	serverAct->setCheckable(true);
	serverAct->setChecked(false);
	connect(serverAct, SIGNAL(toggled(bool)),
			this, SLOT(serverToggle(bool)));

	// Switch to preview mode
	editPreviewGroup = new QActionGroup(this);
	previewModeAct = new QAction(QIcon(":images/preview_mode.png"), tr("Preview Mode"), this);
	previewModeAct->setStatusTip(tr("Switch to preview mode"));
	previewModeAct->setCheckable(true);
	previewModeAct->setChecked(true);
	editPreviewGroup->addAction(previewModeAct);
	previewModeAct->setEnabled(false);
	connect(previewModeAct, SIGNAL(toggled(bool)),
			this, SLOT(previewModeToggle(bool)));

	// Switch to edit mode
	editModeAct = new QAction(QIcon(":images/edit_mode.png"), tr("Edit Mode"), this);
	editModeAct->setStatusTip(tr("Switch to edit mode"));
	editModeAct->setCheckable(true);
	editPreviewGroup->addAction(editModeAct);
	editModeAct->setEnabled(false);
	connect(editModeAct, SIGNAL(toggled(bool)),
			this, SLOT(editModeToggle(bool)));

	// Open file in editor
	editorAct = new QAction(QIcon(":images/editor.png"), tr("Open in Editor"), this);
	editorAct->setStatusTip(tr("Open file in text editor"));
	editorAct->setCheckable(false);
	editorAct->setEnabled(false);
	connect(editorAct, SIGNAL(triggered()), this, SLOT(openInTextEditor()));

	// Prepare icons for reload action
	reloadIconBlack.addPixmap(QPixmap(":images/reload.png"), QIcon::Normal, QIcon::Off);
	reloadIconRed.addPixmap(QPixmap(":images/reload.png"), QIcon::Normal, QIcon::Off);
	reloadIconRed.addPixmap(QPixmap(":images/reload_red.png"), QIcon::Disabled, QIcon::Off);

	// Reload
	reloadAct = new QAction(reloadIconBlack, tr("Reload File"), this);
	reloadAct->setStatusTip(tr("Reload file"));
	reloadAct->setCheckable(false);
	reloadAct->setEnabled(false);
	connect(reloadAct, SIGNAL(triggered()), this, SLOT(reloadFile()));

	// Zoom In
	zoomInAct = new QAction(QIcon(":images/zoom_in.png"), tr("Zoom In (+)"), this);
	zoomInAct->setStatusTip(tr("Zoom in"));
	zoomInAct->setCheckable(false);
	zoomInAct->setEnabled(false);
	connect(zoomInAct, SIGNAL(triggered()),
			this, SLOT(zoomIn()));

	// Zoom Out
	zoomOutAct = new QAction(QIcon(":images/zoom_out.png"), tr("Zoom Out (-)"), this);
	zoomOutAct->setStatusTip(tr("Zoom out"));
	zoomOutAct->setCheckable(false);
	zoomOutAct->setEnabled(false);
	connect(zoomOutAct, SIGNAL(triggered()),
			this, SLOT(zoomOut()));

	// Zoom Out
	zoomAutoAct = new QAction(tr("Zoom Auto"), this);
	zoomAutoAct->setStatusTip(tr("Zoom automatically to fit window"));
	zoomAutoAct->setCheckable(false);
	zoomAutoAct->setEnabled(false);
	connect(zoomAutoAct, SIGNAL(triggered()), this, SLOT(zoomAuto()));

	// Tools

	// Pointer tool
	toolGroup = new QActionGroup(this);
	pointerToolAct = new QAction(QIcon(":images/pointer.png"), tr("Pointer Tool"), this);
	pointerToolAct->setStatusTip(tr("Selection Tool"));
	pointerToolAct->setCheckable(true);
	pointerToolAct->setChecked(true);
	toolGroup->addAction(pointerToolAct);
	connect(pointerToolAct, SIGNAL(toggled(bool)),
			this, SLOT(pointerToolToggle(bool)));

	// Line tool
	lineToolAct = new QAction(QIcon(":images/line.png"), tr("Line Tool"), this);
	lineToolAct->setStatusTip(tr("Line Tool"));
	lineToolAct->setCheckable(true);
	toolGroup->addAction(lineToolAct);
	connect(lineToolAct, SIGNAL(toggled(bool)),
			this, SLOT(lineToolToggle(bool)));

	// Tan Line tool
	tanLineToolAct = new QAction(QIcon(":images/tan_line.png"), tr("Tangent Tool"), this);
	tanLineToolAct->setStatusTip(tr("Tangent Tool"));
	tanLineToolAct->setCheckable(true);
	toolGroup->addAction(tanLineToolAct);
	connect(tanLineToolAct, SIGNAL(toggled(bool)),
			this, SLOT(tanLineToolToggle(bool)));

	// Perpendicular Line tool
	perpLineToolAct = new QAction(QIcon(":images/perp_line.png"), tr("Perpendicular Tool"), this);
	perpLineToolAct->setStatusTip(tr("Perpendicular Tool"));
	perpLineToolAct->setCheckable(true);
	toolGroup->addAction(perpLineToolAct);
	connect(perpLineToolAct, SIGNAL(toggled(bool)),
			this, SLOT(perpLineToolToggle(bool)));

	// Circle tool
	circleToolAct = new QAction(QIcon(":images/circle.png"), tr("Circle Tool"), this);
	circleToolAct->setStatusTip(tr("Circle Tool"));
	circleToolAct->setCheckable(true);
	toolGroup->addAction(circleToolAct);
	connect(circleToolAct, SIGNAL(toggled(bool)),
			this, SLOT(circleToolToggle(bool)));

	// Ellipse tool
	ellipseToolAct = new QAction(QIcon(":images/ellipse.png"), tr("Ellipse Tool"), this);
	ellipseToolAct->setStatusTip(tr("Ellipse Tool"));
	ellipseToolAct->setCheckable(true);
	toolGroup->addAction(ellipseToolAct);
	connect(ellipseToolAct, SIGNAL(toggled(bool)),
			this, SLOT(ellipseToolToggle(bool)));

	// Arc tool
	arcToolAct = new QAction(QIcon(":images/arc_3p.png"), tr("Arc Tool"), this);
	arcToolAct->setStatusTip(tr("Circular Arc Tool"));
	arcToolAct->setCheckable(true);
	toolGroup->addAction(arcToolAct);
	connect(arcToolAct, SIGNAL(toggled(bool)),
			this, SLOT(arcToolToggle(bool)));

	// Text tool
	textToolAct = new QAction(QIcon(":images/text.png"), tr("Text Tool"), this);
	textToolAct->setStatusTip(tr("Text Tool"));
	textToolAct->setCheckable(true);
	toolGroup->addAction(textToolAct);
	connect(textToolAct, SIGNAL(toggled(bool)),
			this, SLOT(textToolToggle(bool)));

	// move tool
	moveToolAct = new QAction(QIcon(":images/amove.png"), tr("Move Tool"), this);
	moveToolAct->setStatusTip(tr("Final Point Tool"));
	moveToolAct->setCheckable(true);
	toolGroup->addAction(moveToolAct);
	connect(moveToolAct, SIGNAL(toggled(bool)),
			this, SLOT(moveToolToggle(bool)));

	// Display the settings dialogue box
	settingsAct = new QAction(tr("&Options"), this);
	settingsAct->setStatusTip(tr("Configure QGLE"));
	connect(settingsAct, SIGNAL(triggered()),
			this, SLOT(openSettingsDialogue()));

	evaluatorAct = new QAction(tr("&Evaluator"), this);
	evaluatorAct->setStatusTip(tr("Open expression evaluator window"));
	connect(evaluatorAct, SIGNAL(triggered()),
			this, SLOT(openEvaluator()));

	// Open 3D view dialogue
	view3DAct = new QAction(tr("&3D Surface Plot"), this);
	view3DAct->setStatusTip(tr("Manipulate 3D surface plot"));
	connect(view3DAct, SIGNAL(triggered()),	this, SLOT(openView3DDialogue()));

	// Open the dialog displaying GhostScript's logs
	ghostScriptLogWindowAct = new QAction(tr("&GhostScript's Log"), this);
	ghostScriptLogWindowAct->setStatusTip(tr("Display GhostScript's Error Log"));
	connect(ghostScriptLogWindowAct, SIGNAL(triggered()), this, SLOT(openGhostScriptLogDialogue()));

	// Test the download function
	downloadAct = new QAction(tr("&Download"), this);
	downloadAct->setStatusTip(tr("Download GhostScript libgs.so"));
	connect(downloadAct, SIGNAL(triggered()),
			this, SLOT(openDownloadDialogue()));

	// Console window auto-show functionality
	consoleWindowHide = new QAction(tr("&Hide"), this);
	connect(consoleWindowHide, SIGNAL(triggered()),
			consoleWindow, SLOT(hideTriggered()));
	consoleWindow->setHideAction(consoleWindowHide);

	for (int i = 0; i < MaxRecentFiles; ++i) {
		recentFileActs[i] = new QAction(this);
		recentFileActs[i]->setVisible(false);
		connect(recentFileActs[i], SIGNAL(triggered()),
		this, SLOT(openRecentFile()));
	}
}

void GLEMainWindow::createDockWindows()
{
	int size1 = 0;
	int size2 = 0;
	const QByteArray& state = settings->mainWindowState();
	if (!state.isEmpty()) {
		const int* sizes = (const int*)state.constData();
		size1 = sizes[0];
		size2 = sizes[1];
	}

	propertyDock = new QDockWidget(tr("Properties"), this);
	propertyDock->setObjectName("GLEPropertyEditor");
	propertyDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::RightDockWidgetArea, propertyDock);
	propertyEditor = new GLEPropertyEditor(propertyDock, size1);
	propertyDock->setWidget(propertyEditor);

	objectBlocksDock = new QDockWidget(tr("Objects"), this);
	objectBlocksDock->setObjectName("GLEObjectBlocksList");
	objectBlocksDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::RightDockWidgetArea, objectBlocksDock);
	objectBlocksList = new GLEObjectBlocksList(objectBlocksDock, this, drawingArea, size2);
	objectBlocksDock->setWidget(objectBlocksList);
}

// Create the toolbars
void GLEMainWindow::createToolBars()
{

	fileToolBar = addToolBar(tr("File"));
	fileToolBar->setObjectName("fileToolBar");
	fileToolBar->addAction(newAct);
	fileToolBar->addAction(openAct);
	fileToolBar->addAction(saveAct);

	modeToolBar = addToolBar(tr("Mode"));
	modeToolBar->setObjectName("modeToolBar");
//	modeToolBar->addAction(serverAct);
	modeToolBar->addAction(previewModeAct);
	modeToolBar->addAction(editModeAct);

	viewToolBar = addToolBar(tr("View"));
	viewToolBar->setObjectName("viewToolBar");
	viewToolBar->addAction(reloadAct);
	viewToolBar->addAction(zoomInAct);
	viewToolBar->addAction(zoomOutAct);

	previewPageSize = new QComboBox(viewToolBar);
	previewPageSize->addItem(tr("Figure"));
	previewPageSize->addItem(tr("Portrait"));
	previewPageSize->addItem(tr("Landscape"));
	previewPageSize->setToolTip("Preview Page Size");
	viewToolBar->addWidget(previewPageSize);
	connect(previewPageSize, SIGNAL(activated(int)), this, SLOT(previewPageSizeChanged(int)));

	editorBut = new QToolButton(viewToolBar);
	editorBut->setDefaultAction(editorAct);
	editorBut->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(editorBut, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(createEditFilePopup(const QPoint&)));
	viewToolBar->addWidget(editorBut);

	compatToolBar = addToolBar(tr("Compatibility"));
	compatToolBar->setObjectName("compatToolBar");
	emulateVersion = new QComboBox(compatToolBar);
	emulateVersion->addItem(tr("4.2"));
	emulateVersion->addItem(tr("3.5"));
	emulateVersion->setToolTip("Emulate GLE Version");
	compatToolBar->addWidget(emulateVersion);
	connect(emulateVersion, SIGNAL(activated(int)), this, SLOT(emulateGLEVersionChanged(int)));

	// Initialise toolsToolBar and controlToolBar to 0
	toolsToolBar = 0;
	controlToolBar = 0;
}

void GLEMainWindow::emulateGLEVersionChanged(int)
{
	GLEFileInfo* currFile = getCurrentFile();
	settings->setEmulateGLEVersion(emulateVersion->currentIndex());
	if (currentMode == GLESettings::PreviewMode && !currFile->primaryFile().isEmpty()) {
		openFile(currFile->primaryFile());
	}
}

void GLEMainWindow::previewPageSizeChanged(int)
{
	GLEFileInfo* currFile = getCurrentFile();
	settings->setPreviewPageSize(previewPageSize->currentIndex());
	if (currentMode == GLESettings::PreviewMode && !currFile->primaryFile().isEmpty() && currFile->isGLE()) {
		editModeAct->setEnabled(previewPageSize->currentIndex() == 0);
		renderGLE(0.0, getScrollAreaSize());
	}
}

void GLEMainWindow::polarSnapToggle(bool state)
{
	drawingArea->polarSnapToggle(state);
	if (state == false)
		polarTrackAct->setChecked(false);
}

void GLEMainWindow::polarTrackToggle(bool state)
{
	drawingArea->polarTrackToggle(state);
	if (state == true)
		polarSnapAct->setChecked(true);
}

void GLEMainWindow::selectPointerTool()
{
	pointerToolAct->setChecked(true);
}

void GLEMainWindow::createEditModeToolBars()
{
	toolsToolBar = new QToolBar(tr("Tools"));
	toolsToolBar->setObjectName("toolsToolBar");
	toolsToolBar->setOrientation(Qt::Vertical);
	toolsToolBar->setAllowedAreas(Qt::LeftToolBarArea | Qt::RightToolBarArea);
	toolsToolBar->addAction(pointerToolAct);
	toolsToolBar->addAction(moveToolAct);
	toolsToolBar->addAction(lineToolAct);
	toolsToolBar->addAction(tanLineToolAct);
	toolsToolBar->addAction(perpLineToolAct);
	toolsToolBar->addAction(circleToolAct);
	toolsToolBar->addAction(ellipseToolAct);
	toolsToolBar->addAction(arcToolAct);
	toolsToolBar->addAction(textToolAct);
	addToolBar(Qt::LeftToolBarArea, toolsToolBar);

	controlToolBar = new QToolBar(tr("Drawing Control"));
	controlToolBar->setObjectName("controlToolBar");
	controlToolBar->setOrientation(Qt::Vertical);
	controlToolBar->setAllowedAreas(Qt::LeftToolBarArea | Qt::RightToolBarArea);
	controlToolBar->addAction(gridAct);
	controlToolBar->addAction(gridSnapAct);
	controlToolBar->addAction(osnapAct);
	controlToolBar->addAction(orthoSnapAct);
	controlToolBar->addAction(polarSnapAct);
	controlToolBar->addAction(polarTrackAct);
	addToolBar(Qt::LeftToolBarArea, controlToolBar);

}

void GLEMainWindow::enableEditModeToolBars(bool enable)
{
	if (!enable)
	{
		// Hide first, next gray out
		toolsToolBar->hide();
		controlToolBar->hide();
		propertyEditor->hide();
		propertyDock->hide();
		objectBlocksList->hide();
		objectBlocksDock->hide();
		emulateVersion->setCurrentIndex(min(emulateVersion->count()-1, settings->getEmulateGLEVersion()));
		previewPageSize->setCurrentIndex(settings->getPreviewPageSize());
	}
	toolsToolBar->setEnabled(enable);
	controlToolBar->setEnabled(enable);
	propertyEditor->setEnabled(enable);
	propertyDock->setEnabled(enable);
	objectBlocksList->setEnabled(enable);
	objectBlocksDock->setEnabled(enable);
	compatToolBar->setEnabled(!enable);
	emulateVersion->setEnabled(!enable);
	previewPageSize->setEnabled(!enable);
	reloadAct->setEnabled(!enable);
	if (enable)
	{
		// Enable first, then show
		toolsToolBar->show();
		controlToolBar->show();
		propertyEditor->show();
		propertyDock->show();
		objectBlocksList->show();
		objectBlocksDock->show();
		emulateVersion->setCurrentIndex(0);
		previewPageSize->setCurrentIndex(0);
	}
}

void GLEMainWindow::destroyEditModeToolBars()
{
	if (toolsToolBar)
	{
		removeToolBar(toolsToolBar);
		delete toolsToolBar;
		toolsToolBar = 0;
	}
	if (controlToolBar)
	{
		removeToolBar(controlToolBar);
		delete controlToolBar;
		controlToolBar = 0;
	}
}

//! Get non-current mode GLEFileInfo object
GLEFileInfo* GLEMainWindow::getNonCurrentModeFileInfo()
{
	int otherMode = (currentMode == GLESettings::PreviewMode ? GLESettings::EditMode : GLESettings::PreviewMode);
	return &currentFile[otherMode];
}

//! Set temp files correctly for GLE file
void GLEMainWindow::initTempFilesGLE()
{
	currentFile[GLESettings::PreviewMode].setEpsFile(tempFiles[PreviewModeOutput]);
	currentFile[GLESettings::EditMode].setEpsFile(tempFiles[EditModeOutput]);
}

//! Update currentFile on change PreviewMode <-> EditMode
int GLEMainWindow::updateFileInfoOnChangeMode(int mode)
{
	int oldMode = (mode == GLESettings::PreviewMode ? GLESettings::EditMode : GLESettings::PreviewMode);
	GLEFileInfo* oldFile = &currentFile[oldMode];
	GLEFileInfo* newFile = &currentFile[mode];
	int diff = 0;
	if (newFile->getLastDPI() != drawingArea->getDPI()) diff = 1; /* re-render EPS */
	if (newFile->gleFile() != oldFile->gleFile()) diff = 2; /* re-render GLE */
	newFile->copyFrom(oldFile);
	// Save DPI for next switch
	oldFile->setLastDPI(drawingArea->getDPI());
	currentMode = mode;
	return diff;
}

// SLOT: user clicked the preview mode button
void GLEMainWindow::previewModeToggle(bool state)
{
	// User switched mode on
	if (state)
	{
		// Switch to preview mode
		int diff = updateFileInfoOnChangeMode(GLESettings::PreviewMode);
		drawingArea->setEditMode(false);

		// Hide the tools toolbar
		enableEditModeToolBars(false);

		// Select no tool
		emit toolSelected(GLEDrawingArea::NoTool);

		// call renderGLEandObjects and then
		// Only process if new objects have been created
		// or if objects have been moved
		if (drawingArea->thereAreNewObjects())
		{
			updateGLE();
			if (settings->saveOnPreview())
			{
				save();
			}
			drawingArea->clearNewObjectsFlag();
			renderGLE();
		}
		else
		{
			if (diff == 0) refreshDisplay();
			else if (diff == 1) renderEPS();
			else renderGLE();
		}
	}
}


// SLOT: user clicked the edit mode button
void GLEMainWindow::editModeToggle(bool state)
{
	// User switched mode on
	if (state)
	{
		if (getCurrentFile()->gleFile().isEmpty())
		{
			previewModeAct->setChecked(true);
			return;
		}

		int diff = updateFileInfoOnChangeMode(GLESettings::EditMode);
		drawingArea->setEditMode(true);

		enableEditModeToolBars(true);
		pointerToolAct->setChecked(true);

		if (pointerToolAct->isChecked())
			emit toolSelected(GLEDrawingArea::PointerTool);
		else if (lineToolAct->isChecked())
			emit toolSelected(GLEDrawingArea::LineTool);
		else if (tanLineToolAct->isChecked())
			emit toolSelected(GLEDrawingArea::TanLineTool);
		else if (perpLineToolAct->isChecked())
			emit toolSelected(GLEDrawingArea::PerpLineTool);
		else if (circleToolAct->isChecked())
			emit toolSelected(GLEDrawingArea::CircleTool);
		else if (ellipseToolAct->isChecked())
			emit toolSelected(GLEDrawingArea::EllipseTool);
		else if (arcToolAct->isChecked())
			emit toolSelected(GLEDrawingArea::ArcTool);
		else if (textToolAct->isChecked())
			emit toolSelected(GLEDrawingArea::TextTool);
		else if (moveToolAct->isChecked())
			emit toolSelected(GLEDrawingArea::AMoveTool);

		// We should probably only emit this when there is
		// definitely an image, but edit mode shouldn't be
		// available if there isn't one
		if (diff == 0) refreshDisplay();
		else if (diff == 1) renderEPS();
		else renderGLE();
	}
}

// SLOT: user requested all the tools be disabled
void GLEMainWindow::noToolEnable()
{
	lineToolAct->setChecked(false);
	tanLineToolAct->setChecked(false);
	perpLineToolAct->setChecked(false);
	circleToolAct->setChecked(false);
	ellipseToolAct->setChecked(false);
	arcToolAct->setChecked(false);
	moveToolAct->setChecked(false);
	pointerToolAct->setChecked(false);
	textToolAct->setChecked(false);

	emit toolSelected(GLEDrawingArea::NoTool);
}


// SLOT: user clicked the pointer tool button
void GLEMainWindow::pointerToolToggle(bool state)
{
	// User switched mode on
	if (state)
	{
		// Notify drawing area
		emit toolSelected(GLEDrawingArea::PointerTool);
	}
}

// SLOT: user clicked the line tool button
void GLEMainWindow::lineToolToggle(bool state)
{
	// User switched mode on
	if (state)
	{
		// Notify drawing area
		emit toolSelected(GLEDrawingArea::LineTool);
	}
}

// SLOT: user clicked the tan line tool button
void GLEMainWindow::tanLineToolToggle(bool state)
{
	// User switched mode on
	if (state)
	{
		// Notify drawing area
		emit toolSelected(GLEDrawingArea::TanLineTool);
	}
}

// SLOT: user clicked the perp line tool button
void GLEMainWindow::perpLineToolToggle(bool state)
{
	// User switched mode on
	if (state)
	{
		// Notify drawing area
		emit toolSelected(GLEDrawingArea::PerpLineTool);
	}
}

// SLOT: user clicked the circle tool button
void GLEMainWindow::circleToolToggle(bool state)
{
	// User switched mode on
	if (state)
	{
		// Notify drawing area
		emit toolSelected(GLEDrawingArea::CircleTool);
	}
}

// SLOT: user clicked the ellipse tool button
void GLEMainWindow::ellipseToolToggle(bool state)
{
	// User switched mode on
	if (state)
	{
		// Notify drawing area
		emit toolSelected(GLEDrawingArea::EllipseTool);
	}
}

// SLOT: user clicked the arc tool button
void GLEMainWindow::arcToolToggle(bool state)
{
	// User switched mode on
	if (state)
	{
		// Notify drawing area
		emit toolSelected(GLEDrawingArea::ArcTool);
	}
}

// SLOT: user clicked the text tool button
void GLEMainWindow::textToolToggle(bool state)
{
	// User switched mode on
	if (state)
	{
		// Notify drawing area
		emit toolSelected(GLEDrawingArea::TextTool);
	}
}

// SLOT: user clicked the pointer tool button
void GLEMainWindow::moveToolToggle(bool state)
{
	// User switched mode on
	if (state)
	{
		// Notify drawing area
		emit toolSelected(GLEDrawingArea::AMoveTool);
	}
}

void GLEMainWindow::renderEPS()
{
	renderEPS(drawingArea->getDPI());
}

void GLEMainWindow::renderEPS(double dpi)
{
	renderEPS(getCurrentFile()->epsFile(), dpi, getScrollAreaSize());
}

// Render an EPS file from a given QString
void GLEMainWindow::renderEPS(QString outputFile, double dpi, const QSize& area)
{
	if (getCurrentFile()->isGLE())
	{
		GLEScript* script = getGLEScript();
		if (!script->getRecordedBytesBuffer(GLE_DEVICE_EPS)->empty()
			|| !script->getRecordedBytesBuffer(GLE_DEVICE_PDF)->empty())
		{
			updateEnableActions(false);
			renderThread->renderOutputFromMemoryToImage(script, outputFile, dpi, area);
			return;
		}
	}
	else
	{
		QFileInfo fi(outputFile);
		// If the file is readable, re-render it in the new scale
		if (fi.isReadable())
		{
			updateEnableActions(false);
			renderThread->renderEPSFromFileToImage(outputFile, dpi, area);
			return;
		}
	}
	renderComplete(QImage());
}

// Refresh GLEDrawing's display
void GLEMainWindow::refreshDisplay()
{
	emit imageChanged(getCurrentFile()->getImage());
}

// A signal from the server to indicate that rendering is complete
void GLEMainWindow::renderComplete(QImage image)
{
	statusBarMessage("");
	updateEnableActions(true);

	GLEFileInfo* currFile = getCurrentFile();
	// Save the image
	currFile->setImage(image);
	// Update the display
	emit imageChanged(image);

	updateWindowTitle();

	addFilesToMonitor();
	executeToDos();
}

void GLEMainWindow::addFilesToMonitor()
{
	GLEFileInfo* currFile = getCurrentFile();
	currFile->clearFilesToMonitor();
	GLEInterface* iface = getGLEInterface();
	vector<GLEFileLocation> infos = iface->getFileInfos();
	for (unsigned int i = 0; i < infos.size(); i++) {
		GLEFileLocation& info = infos[i];
		QString fullpath = QString::fromUtf8(info.getFullPath().c_str());
		currFile->addFileToMonitor(fullpath);
	}
}

//! Execute this after opening a file
void GLEMainWindow::executeToDoAfterFileOpen()
{
	// Restart the timer if appropriate
	updateFileMonitor(settings->monitorOpenFile());
	serverThread->sendMessageToGLEMinusP(consoleWindow->toPlainText(), true);
}

//! Execute this after rendering a GLE script
void GLEMainWindow::executeToDoAfterRenderGLE()
{
	consoleWindow->shouldAutoShow();
	// Now we should really check to see if it worked or not!
	if (consoleWindow->getOutput()->getExitCode() == 0)
	{
		if (currentMode != GLESettings::PreviewMode) {
			drawingArea->clearObjects();
			drawingArea->updateFromScript(getGLEScript());
			objectBlocksList->updateList();
		}
	}
	else
	{
		if (currentMode != GLESettings::PreviewMode) {
			drawingArea->clearObjects();
			objectBlocksList->updateList();
		}
	}
}

void GLEMainWindow::executeToDos()
{
	for (int i = todoList.size()-1; i >= 0; i--)
	{
		int todo = todoList.at(i);
		switch (todo)
		{
			case ToDoAfterRenderGLE:
				executeToDoAfterRenderGLE();
				break;
			case ToDoAfterFileOpen:
				executeToDoAfterFileOpen();
				break;
		}
	}
	todoList.clear();
}

QString GLEMainWindow::getGLEVersion() {
	return QString::fromUtf8(gleInterface->getGLEVersion().c_str());
}

QString GLEMainWindow::getGLEBuildDate() {
	return QString::fromUtf8(gleInterface->getGLEBuildDate().c_str());
}

const char* GLEMainWindow::getGLETop() {
	return gleInterface->getGLETop();
}

QString GLEMainWindow::getCompatibility() {
	return emulateVersion->currentText();
}

//! Return true if in full page mode
bool GLEMainWindow::isFullPage() {
	return previewPageSize->currentIndex() == 1 || previewPageSize->currentIndex() == 2;
}

//! Return true if in landscape mode
bool GLEMainWindow::isLandscape() {
	return previewPageSize->currentIndex() == 2;
}

void GLEMainWindow::renderGLE()
{
	renderGLE(drawingArea->getDPI(), getScrollAreaSize());
}

void GLEMainWindow::renderGLE(double dpi, const QSize& area)
{
	// Render a GLE file into a temporary eps and return the resulting image
	consoleWindow->clear();
	gleInterface->clearAllCmdLine();
	gleInterface->setMakeDrawObjects(currentMode != GLESettings::PreviewMode);
	if (currentMode == GLESettings::PreviewMode) {
		gleInterface->setCompatibilityMode(emulateVersion->currentText().toLatin1().constData());
		if (previewPageSize->currentIndex() != 0) {
			gleInterface->setCmdLineOption("fullpage");
		}
		if (previewPageSize->currentIndex() == 2) {
			gleInterface->setCmdLineOption("landscape");
		}
		// Make sure to re-render when switching to edit mode
		// Otherwise object blocks will be corrupt
		currentFile[GLESettings::EditMode].clearGleFile();
	} else {
		gleInterface->setCompatibilityMode("4.2");
	}
	if (settings->isRenderUsingCairo()) {
		gleInterface->setCmdLineOption("cairo");
	}
	updateEnableActions(false);
	todoList.append(ToDoAfterRenderGLE);
	renderThread->renderGLEToImage(getGLEScript(), getCurrentFile()->epsFile(), dpi, area);
}

// Commit changes to drawing to GLE
void GLEMainWindow::updateGLE()
{
	gleInterface->clearAllCmdLine();
	gleInterface->commitChangesGLE(getGLEScript());
	consoleWindow->shouldAutoShow();
}

//! Show console window if there is output
void GLEMainWindow::shouldAutoShowConsole()
{
	consoleWindow->shouldAutoShow();
}

// SLOT: display the about box
void GLEMainWindow::about()
{
	box = new AboutBox(this, getGLEInterface());
	box->setModal(true);
	box->show();
}

// SLOT: open the GLE website
void GLEMainWindow::showGLEWebsite() {
	QUrl url("http://glx.sourceforge.net/");
	QDesktopServices::openUrl(url);
}

// SLOT: show a given URL
void GLEMainWindow::showURL(const QUrl& url) {
	QDesktopServices::openUrl(url);
}

// Open a local file using the mime type application registered for it
void GLEMainWindow::showLocalFile(const QString& name) {
	QFileInfo file(name);
	if (!file.exists()) {
		QString msg = tr("File not found: '%1'").arg(name);
		QMessageBox::critical(this, APP_NAME, msg, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
		return;
	}
	if (QGLE::getFileNameExtension(name) == "pdf") {
		// Preview a PDF with the application set in .glerc as "pdfviewer"
		string pdfview = getGLEInterface()->getToolLocation("pdfviewer");
		if (pdfview != "") {
			QProcess pdfReader;
			pdfReader.startDetached(pdfview.c_str(), QStringList() << name);
			return;
		}
	}
	#ifdef Q_OS_WIN32
		// Why does Windows require three slashes for a file Url?
		QUrl url(tr("file:///") + name);
		QDesktopServices::openUrl(url);
	#else
		QUrl url(tr("file://") + name);
		QDesktopServices::openUrl(url);
	#endif
}

// SLOT: open the GLE manual
void GLEMainWindow::showGLEManual() {
	string name = getGLEInterface()->getManualLocation();
	showLocalFile(QString::fromUtf8(name.c_str()));
}

// SLOT: abort QGLE
void GLEMainWindow::abort() {
	exit(0);
}

// Create the menus and add the actions
void GLEMainWindow::createMenus()
{
	// File menu
	fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(newAct);
	fileMenu->addAction(openAct);
	fileMenu->addAction(openInNewWindowAct);
	editFileMenu = fileMenu->addMenu(tr("&Edit"));
	editFileMenu->setIcon(QIcon(":images/editor.png"));
	fileMenu->addAction(saveAct);
	fileMenu->addAction(saveAsAct);
	fileMenu->addAction(exportAct);
   fileMenu->addAction(browseAct);
	// fileMenu->addAction(printAct);

	separatorAct = fileMenu->addSeparator();
	for (int i = 0; i < MaxRecentFiles; i++)
		fileMenu->addAction(recentFileActs[i]);
	updateRecentFileActions();

	fileMenu->addSeparator();
	fileMenu->addAction(quitAct);

	connect(editFileMenu, SIGNAL(aboutToShow()),
	        this, SLOT(createEditFileMenu()));

	// Edit menu
	QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
	editMenu->addAction(copyBitmapAct);
	if (copyPDFAct != NULL) editMenu->addAction(copyPDFAct);
	if (copyEMFAct != NULL) editMenu->addAction(copyEMFAct);

	QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
	viewMenu->addAction(reloadAct);
	viewMenu->addAction(zoomInAct);
	viewMenu->addAction(zoomOutAct);
	viewMenu->addAction(zoomAutoAct);
	viewMenu->addAction(previewModeAct);
	viewMenu->addAction(editModeAct);

	// Tools menu
	toolsMenu = menuBar()->addMenu(tr("&Tools"));
	toolsMenu->addAction(settingsAct);
	toolsMenu->addAction(evaluatorAct);
	toolsMenu->addAction(view3DAct);
	toolsMenu->addAction(ghostScriptLogWindowAct);
	// toolsMenu->addAction(downloadAct);

	// Help menu
	helpMenu = menuBar()->addMenu(tr("&Help"));
	helpMenu->addAction(gleManualAct);
	helpMenu->addAction(gleSiteAct);
	helpMenu->addAction(aboutAct);
}

#define STATUS_BAR_LABEL_STYLE QFrame::NoFrame | QFrame::Plain

void GLEMainWindow::createStatusBar()
{
	// Create a status bar with an initial message
	statusBar()->showMessage(tr("Ready"));

	// Create a coordinate display (using GLE coordinates)
	mousePosition = new QLabel();
	mousePosition->setFrameStyle(STATUS_BAR_LABEL_STYLE);
	mousePosition->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

	dpiDisplay = new QLabel();
	dpiDisplay->setFrameStyle(STATUS_BAR_LABEL_STYLE);
	dpiDisplay->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

	// Create the snap button
/*	gridSnapButton = new QPushButton(tr("SNAP"));
	gridSnapButton->resize(0.4*gridSnapButton->size());
	gridSnapButton->setFlat(true);
	gridSnapButton->setCheckable(true);
	connect(gridSnapButton, SIGNAL(toggled(bool)),
			gridSnapAct, SLOT(setChecked(bool)));
	statusBar()->addPermanentWidget(gridSnapButton);*/

	// Set size and disable auto resize?
	statusBar()->addPermanentWidget(dpiDisplay);
	statusBar()->addPermanentWidget(mousePosition);
	updateMousePosition(QPointF(0.0,0.0));
	// Display the default resolution
	updateDPIDisplay(-1.0);
	connect(drawingArea, SIGNAL(mouseMoved(QPointF)),
			this, SLOT(updateMousePosition(QPointF)));
}

void GLEMainWindow::updateDPIDisplay(double newDPI)
{
	// Display the current resolution on the status bar
	if (newDPI < 0.0)
	{
		dpiDisplay->setVisible(false);
	}
	else
	{
		dpiDisplay->setVisible(true);
		dpiDisplay->setText(QString(tr("R:%1", "Resolution Indicator"))
				.arg(newDPI,8,'f',2));
		dpiDisplay->setFixedSize(100,24);
	}
}


// Update the coordinate display
void GLEMainWindow::updateMousePosition(QPointF gle)
{

	if (gle == QPointF(-1.0,-1.0))
		gle = lastMousePosition;
	else
		lastMousePosition = gle;

	double r, theta;
	switch (coordView)
	{
		case CoordinateOff:
			// Don't display anything
			mousePosition->setText("");
			break;
		case CoordinateCart:
			// Cartesian coordinates
			mousePosition->setText(QString("%1,%2").arg(gle.x(),5,'f',2).arg(gle.y(),5,'f',2));
			break;
		case CoordinatePolar:
			// Polar coordinates
			r = sqrt(pow(gle.x(),2)+pow(gle.y(),2));
			theta = QGLE::radiansToDegrees(atan2(gle.y(),gle.x()));
			mousePosition->setText(QString("%1<%2").arg(r,5,'f',2).arg(theta,5,'f',2));
			break;
		case CoordinateRelCart:
			// Cartesian coordinates relative to the last point used in drawing
			mousePosition->setText(QString("@%1,%2")
					.arg(gle.x()-lastPoint.x(),5,'f',2)
					.arg(gle.y()-lastPoint.y(),5,'f',2));
			break;
		case CoordinateRelPolar:
			// Polar coordinates relative to the last point used in drawing
			r = sqrt(pow(gle.x()-lastPoint.x(),2)+pow(gle.y()-lastPoint.y(),2));
			theta = QGLE::radiansToDegrees(atan2(gle.y()-lastPoint.y(),gle.x()-lastPoint.x()));
			mousePosition->setText(QString("@%1<%2").arg(r,5,'f',2).arg(theta,5,'f',2));
			break;
	}
	// Fix the size of the display
	mousePosition->setFixedSize(100,24);

}

// SLOT: Display a requested message on the status bar
void GLEMainWindow::updateServerStatus(QString message)
{
	statusBar()->showMessage(message);
}

// SLOT: Display a requested message on the status bar
void GLEMainWindow::statusBarMessage(QString msg)
{
	statusBar()->showMessage(msg);
}

//! Display an error message in the console
void GLEMainWindow::showConsoleError(QString message)
{
	consoleWindow->println(renderThread->getGhostScriptOutput());
	consoleWindow->println(message);
	consoleWindow->shouldAutoShow();
}

// SLOT: Open the Settings Dialogue
void GLEMainWindow::openSettingsDialogue()
{
	SettingsDialogue settingsDialogue(this);
	settingsDialogue.exec();
}

// SLOT: Display 3D viewer
void GLEMainWindow::openView3DDialogue()
{
	QGLE3DViewer* viewer = new QGLE3DViewer(this, gleInterface);
	viewer->show();
}

// SLOT: Display GhostScript's log dialogue box
void GLEMainWindow::openGhostScriptLogDialogue()
{
	QDialog gsLogDialog(this);
	gsLogDialog.setWindowTitle("GhostScript's Log");
	QVBoxLayout *layout = new QVBoxLayout();
	QLabel *stdoutlab = new QLabel(tr("Output:"));
	layout->addWidget(stdoutlab);
	QTextEdit* stdouttxt = new QTextEdit();
	stdouttxt->setReadOnly(true);
	stdouttxt->insertPlainText(renderThread->getGhostScriptOutput());
	layout->addWidget(stdouttxt);
	QPushButton *okButton = new QPushButton(tr("OK"));
	connect(okButton, SIGNAL(clicked()), &gsLogDialog, SLOT(close()));
	QHBoxLayout *buttonLayout = new QHBoxLayout();
	buttonLayout->addStretch(1);
	buttonLayout->addWidget(okButton);
	layout->addLayout(buttonLayout);
	gsLogDialog.setLayout(layout);
	QRect size = QApplication::desktop()->screenGeometry(&gsLogDialog);
	gsLogDialog.resize(size.width()/2, size.height()/2);
	gsLogDialog.exec();
}

void GLEMainWindow::openEvaluator()
{
	if (evaluatorDialog == NULL)
	{
		evaluatorDialog = new EvaluatorDialog(this);
		QRect size = QApplication::desktop()->screenGeometry(evaluatorDialog);
		evaluatorDialog->resize(size.width()/3, size.height()/2);
	}
	evaluatorDialog->show();
}

//! Display export dialogue box
void GLEMainWindow::openExportDialogue()
{
	if (getGLEScript() != NULL) {
		ExportDialogue exp(this);
		exp.exec();
	}
}

// SLOT: Display print dialogue box
void GLEMainWindow::openPrintDialogue()
{

}

void GLEMainWindow::openDownloadDialogue()
{
/*	QGLEDownloader* down = new QGLEDownloader(this);
	down->setModal(true);
	down->show();
	down->exec();*/
}

void GLEMainWindow::save()
{
	if (drawingArea->isDirty())
	{
		GLEFileInfo* currFile = getCurrentFile();
		if (currFile->hasFileName())
		{
			QFileInfo file;
			file.setFile(currFile->gleFile());
			if (file.isReadable() && file.isWritable())
			{
				save(currFile->gleFile());
				return;
			}
		}
		// If we don't have a file name and/or the file
		// isn't readable and writable, ask for a new file name.
		saveAs();
	}
}

void GLEMainWindow::saveAs()
{
	QString fileType;
	const char* fileExt;
	if (getCurrentFile()->isGLE()) {
		fileType = tr("GLE Files (*.gle)");
		fileExt = "gle";
	} else {
		fileType = tr("EPS Files (*.eps)");
		fileExt = "eps";
	}

	// Ask for a filename and save accordingly
	QString fileName = QFileDialog::getSaveFileName(this,
			tr("Choose a file name to save under"),
			settings->pwd(), fileType);

	if (fileName.isEmpty())
		return;

	// Add .gle or .eps if here isn't one already
	QGLE::ensureFileNameExtension(&fileName, fileExt);

	QFileInfo fname(fileName);
	settings->setPwd(fname.dir().path());

	save(fileName);
}

void GLEMainWindow::save(QString fileName)
{
	// Save to a given filename
	bool wasActive = false;

	// Stop the timer as it'll get confused by us writing out own file
	if (fileMonitorTimer->isActive())
	{
		wasActive = true;
		fileMonitorTimer->stop();
	}

	// Check if we are saving a GLE or EPS file
	GLEFileInfo* currFile = getCurrentFile();
	if (currFile->isGLE()) {
		if (currentMode == GLESettings::EditMode && drawingArea->thereAreNewObjects())
		{
			updateGLE();
		}
		gleInterface->saveGLEFile(getGLEScript(), fileName.toLatin1().constData());
	} else {
		// Copy EPS file to new name
		string error;
		string from = getCurrentFile()->primaryFile().toUtf8().constData();
		string to = fileName.toUtf8().constData();
		if (gleInterface->copyFile(from, to, &error) != 0) {
			QMessageBox::critical(this, APP_NAME, tr("Error: ")+QString::fromUtf8(error.c_str()),
				QMessageBox::Ok,
				QMessageBox::NoButton,
				QMessageBox::NoButton);
		}
	}

	// Clear the dirty flag
	drawingArea->clearDirty();
	// Check that currentFile points to the correct file
	if (currFile->isGLE()) {
		currFile->setGleFile(fileName);
	} else {
		currFile->setEpsFile(fileName);
	}
	updateWindowTitle();

	// Close the output and update the time stamp
	currFile->updateTimeStamp();

	// Add file to recent file list
	setCurrentFile(fileName);

	// Restart the timer if appropriate
	if (wasActive)
	{
		fileMonitorTimer->start();
	}
}

// SLOT: Open file in new window
void GLEMainWindow::openFileInNewWindow()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Select a file to open"),
	                   settings->pwd(), tr("GLE Files (*.gle);;EPS Files (*.eps)"));

	// If we now have a filename, open it
	if (!fileName.isEmpty())
	{
		QProcess qgle;
		qgle.startDetached(QGLE::GetExeName(), QStringList() << fileName);
	}
}

// SLOT: Browse the script folder
void GLEMainWindow::browseScriptFolder()
{
   QFileInfo fname(getCurrentFile()->primaryFile());
	showLocalFile(fname.dir().path());
}

// SLOT: Open an existing file
void GLEMainWindow::openFile(QString fileName, bool isOnStartup)
{
	bool isReload = (getCurrentGleFile() == fileName);

	// If changes have been made, ask what to do
	if (!isReload && drawingArea->isDirty())
	{
		int reply = QMessageBox::question(this, tr("Open New File"),
				tr("Save changes to drawing?"),
				QMessageBox::Yes,
				QMessageBox::No,
				QMessageBox::Cancel);

		if (reply == QMessageBox::Yes)
		{
			save();
		}
		else if (reply == QMessageBox::Cancel)
		{
			updateFileMonitor(settings->monitorOpenFile());
			return;
		}
	}

	// If we don't have a filename (i.e. if called as a slot rather than
	// as a function), ask what file to open
	if (fileName.isEmpty())
		fileName = QFileDialog::getOpenFileName(this,
				tr("Select a file to open"),
				settings->pwd(),
				tr("GLE Files (*.gle);;EPS Files (*.eps)"));

	// If we now have a filename, open it
	if (!fileName.isEmpty())
	{
		// In case we crash
		SetQGLEScriptNameBackup(fileName);
		// Check whether to autoscale and get the resolution
		bool autoScale = settings->autoScaleOnOpen() && !isReload;
		QSize scaleSize = isOnStartup ? settings->drawingAreaSize() : getScrollAreaSize();
		if (!isReload && !autoScale) {
			drawingArea->setDPI((double)settings->dpi());
		}
		double dpi = autoScale ? 0.0 : drawingArea->getDPI();

		// Get the file information
		QFileInfo fname(fileName);
		settings->setPwd(fname.dir().path());
		// Regular expressions to check whether it's an EPS file or a GLE file
		QRegExp rxeps(".*\\.eps$", Qt::CaseInsensitive);
		QRegExp rxgle(".*\\.gle$", Qt::CaseInsensitive);

		// Clear other mode file info
		getNonCurrentModeFileInfo()->clear();

		// Add file to recent file list
		setCurrentFile(fileName);

		// Restart file monitor and notify "GLE -p" process
		todoList.append(ToDoAfterFileOpen);

		// Stop the timer
		updateFileMonitor(false);

		// An EPS file, so open it.
		GLEFileInfo* currFile = getCurrentFile();
		if(rxeps.exactMatch(fileName))
		{
			currFile->clear();
			currFile->setEpsFile(fileName);
			getNonCurrentModeFileInfo()->setEpsFile(fileName);
			currFile->updateTimeStamp();
			resetDrawing();
			clearConsoleWindow();
			if (currentMode == GLESettings::PreviewMode) renderEPS(fileName, dpi, scaleSize);
			else previewModeAct->setChecked(true);
		}
		// A GLE file, so render it and open the resulting EPS file
		else if (rxgle.exactMatch(fileName))
		{
			currFile->clear();
			resetDrawing();
			clearConsoleWindow();
			gleScript = gleInterface->loadGLEFile(fileName.toLatin1().constData());
			if (!gleScript.isNull()) {
				currFile->setGleFile(fileName);
				currFile->updateTimeStamp();
				initTempFilesGLE();
				renderGLE(dpi, scaleSize);
			} else {
				renderComplete(QImage());
			}
		}
	}
}

// SLOT: Create a blank page
void GLEMainWindow::newFile()
{
	// Should we save changes?
	if (drawingArea->isDirty())
	{
		int reply = QMessageBox::question(this, tr("Create New File"),
				tr("Save changes to drawing?"),
				QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel);

		if (reply == QMessageBox::Yes)
			save();
		else if (reply == QMessageBox::Cancel)
			return;
	}

	// Ask the user what size drawing to create
	NewFileBox nfb;

	int dialogue_result = nfb.exec();

	if (dialogue_result == QDialog::Rejected)
		return;

	QString fileName;
	if (nfb.shouldSave())
	{
		fileName = QFileDialog::getSaveFileName(this, tr("Choose a file name to save under"), settings->pwd(), tr("GLE Files (*.gle)"));
		if (fileName.isEmpty())	return;

		// Add a .gle if there isn't one already
		QRegExp rxGLE(".*\\.[gG][lL][eE]");
		if(!rxGLE.exactMatch(fileName))	fileName += ".gle";
	}

	// Stop the timer
	updateFileMonitor(false);

	resetDrawing();
	newDiagramSize = nfb.size();

	// Clear the current drawing
	getNonCurrentModeFileInfo()->clear();
	GLEFileInfo* currFile = getCurrentFile();
	currFile->clear();
	currFile->setGleFile(tr("No Name"));
	currFile->setHasFileName(false);
	initTempFilesGLE();

	// Check whether to autoscale and get the resolution
	bool autoScale = settings->autoScaleOnOpen();
	if (!autoScale) drawingArea->setDPI((double) settings->dpi());
	double dpi = autoScale ? 0.0 : drawingArea->getDPI();

	emulateVersion->setCurrentIndex(0);
	previewPageSize->setCurrentIndex(0);
	settings->setEmulateGLEVersion(0);
	settings->setPreviewPageSize(0);

	// The file is very simple: just one line "size x y"
	// The "amove" works around a bug in GLE
	QString code = QString("size %1 %2\n").arg(QGLE::GLEToStr(newDiagramSize.width())).arg(QGLE::GLEToStr(newDiagramSize.height()));

	if (fileName == "")
	{
		gleScript = gleInterface->newGLEFile(code.toLatin1().constData(), tempFiles[NewGLE].toLatin1().constData());
		// Since we've added a "size" line, the file has (arguably) changed
		updateWindowTitle();
		drawingArea->setDirty();
	}
	else
	{
		gleScript = gleInterface->newGLEFile(code.toLatin1().constData(), fileName.toLatin1().constData());
		save(fileName);
	}

	// Restart file monitor and notify "GLE -p" process
	todoList.append(ToDoAfterFileOpen);

	// Render GLE script
	gleInterface->showGLEFile(getGLEScript());
	renderGLE(dpi, getScrollAreaSize());
}

void GLEMainWindow::openRecentFile()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (action)
		openFile(action->data().toString());
}

void GLEMainWindow::openRelatedFile()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (action)
	{
		openInTextEditor(action->data().toString());
	}
}

void GLEMainWindow::updateRecentFileActions()
{
	QSettings settings("gle", "qgle");
	QStringList files = settings.value("application/recentfiles").toStringList();

	int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);

	for (int i = 0; i < numRecentFiles; ++i) {
		QString text = tr("&%1 %2").arg(i + 1).arg(QFileInfo(files[i]).fileName());
		recentFileActs[i]->setText(text);
		recentFileActs[i]->setData(files[i]);
		recentFileActs[i]->setVisible(true);
	}
	for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
		recentFileActs[j]->setVisible(false);

	separatorAct->setVisible(numRecentFiles > 0);
}

void GLEMainWindow::createEditFileMenu()
{
	GLEInterface* iface = getGLEInterface();
	vector<GLEFileLocation> infos = iface->getFileInfos();
	editFileMenu->clear();
	GLEFileInfo* currFile = getCurrentFile();
	if (currFile->primaryFile() == "") {
		QAction* noRel = editFileMenu->addAction(tr("No files to edit"));
		noRel->setEnabled(false);
	} else {
		QFileInfo fh(currFile->primaryFile());
		QAction* mAct = editFileMenu->addAction(fh.fileName());
		QString status = tr("Open file '%1'").arg(fh.fileName());
		mAct->setStatusTip(status);
		connect(mAct, SIGNAL(triggered()), this, SLOT(openInTextEditor()));
		for (unsigned int i = 0; i < infos.size(); i++) {
			GLEFileLocation& info = infos[i];
			QString name = QString::fromUtf8(info.getName().c_str());
			QAction* fAct = editFileMenu->addAction(name);
			QString fullpath = QString::fromUtf8(info.getFullPath().c_str());
			fAct->setToolTip(fullpath);
			fAct->setData(fullpath);
			QString status = tr("Open related file '%1'").arg(name);
			fAct->setStatusTip(status);
			connect(fAct, SIGNAL(triggered()), this, SLOT(openRelatedFile()));
		}
	}
}

void GLEMainWindow::createEditFilePopup(const QPoint& pos) {
	GLEInterface* iface = getGLEInterface();
	vector<GLEFileLocation> infos = iface->getFileInfos();
	QMenu editFilePopup(this);
	if (infos.size() == 0) {
		QAction* noRel = editFilePopup.addAction(tr("No additional files to edit"));
		noRel->setStatusTip(tr("Open related files such as data files, include files, ..."));
		noRel->setEnabled(false);
	} else {
		for (unsigned int i = 0; i < infos.size(); i++) {
			GLEFileLocation& info = infos[i];
			QString name = QString::fromUtf8(info.getName().c_str());
			QAction* fAct = editFilePopup.addAction(name);
			QString fullpath = QString::fromUtf8(info.getFullPath().c_str());
			fAct->setToolTip(fullpath);
			fAct->setData(fullpath);
			QString status = tr("Open related file '%1'").arg(name);
			fAct->setStatusTip(status);
			connect(fAct, SIGNAL(triggered()), this, SLOT(openRelatedFile()));
		}
	}
	editFilePopup.exec(editorBut->mapToGlobal(pos));
}

void GLEMainWindow::setCurrentFile(const QString &fileName)
{
	QSettings settings("gle", "qgle");
	QStringList files = settings.value("application/recentfiles").toStringList();
	files.removeAll(fileName);
	files.prepend(fileName);
	while (files.size() > MaxRecentFiles) {
		files.removeLast();
	}
	settings.setValue("application/recentfiles", files);
	foreach (QWidget *widget, QApplication::topLevelWidgets()) {
		GLEMainWindow *mainWin = qobject_cast<GLEMainWindow *>(widget);
		if (mainWin)
			mainWin->updateRecentFileActions();
	}
}

void GLEMainWindow::copyBitmap() {
	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setImage(getCurrentFile()->getImage());
}

#ifdef Q_WS_MAC
bool copyPDFMac(const string& fname);
#endif

//! Action causing the current figure to be copied to the clipboard as PDF
void GLEMainWindow::copyPDF() {
	QString tempDir = QDir::tempPath();
	QRegExp rxEndSlash(".*[\\\\/]$");
	if (!rxEndSlash.exactMatch(tempDir))
		tempDir += QDir::separator();
	QString pdfname = tempDir + tr("qgle_copy.pdf");
	GLEInterface* iface = getGLEInterface();
	clearConsoleWindow();
	iface->clearAllCmdLine();
	iface->setMakeDrawObjects(false);
	iface->renderGLE(getGLEScript(), pdfname.toLatin1().constData(), GLE_DEVICE_PDF);
	iface->clearAllCmdLine();
	#ifndef Q_WS_MAC
		QFile file(pdfname);
		if (!file.open(QIODevice::ReadOnly)) {
			consoleWindow->println(tr("Can't open PDF: ")+pdfname);
			return;
		}
		QByteArray pdfData = file.readAll();
		file.close();
		QMimeData *mimeData = new QMimeData();
		mimeData->setData("x-pdf", pdfData);
		QClipboard *clipboard = QApplication::clipboard();
		clipboard->setMimeData(mimeData);
	#else
		copyPDFMac(string(pdfname.toUtf8().constData()));
	#endif
}

//! Action causing the current figure to be copied to the clipboard as EMF
void GLEMainWindow::copyEMF() {
	GLEInterface* iface = getGLEInterface();
	clearConsoleWindow();
	iface->clearAllCmdLine();
	iface->setMakeDrawObjects(false);
	iface->setCmdLineOption("copy");
	iface->renderGLE(getGLEScript(), "", GLE_DEVICE_EMF);
	iface->clearAllCmdLine();
}

void GLEMainWindow::resetDrawing()
{
	// Reset the drawing.
	drawingArea->clearDirty();
	drawingArea->clearObjects();
	drawingArea->clearNewObjectsFlag();
}

// SLOT: Reload File
void GLEMainWindow::reloadFile()
{
	openFile(getCurrentFile()->primaryFile());
}

// SLOT: Zoom In
void GLEMainWindow::zoomIn()
{
	double dpi = drawingArea->getDPI();
	dpi *= ZOOM_STEP;
	drawingArea->setDPI(dpi);
	zoom(dpi);
}

// SLOT: Zoom Out
void GLEMainWindow::zoomOut()
{
	double dpi = drawingArea->getDPI();
	dpi /= ZOOM_STEP;
	drawingArea->setDPI(dpi);
	zoom(dpi);
}

// SLOT: Zoom Automatically
void GLEMainWindow::zoomAuto()
{
	// dpi = 0.0 means auto-scale
	renderEPS(0.0);
}

// Zoom worker
void GLEMainWindow::zoom(double dpi)
{
	renderEPS(dpi);
}

void GLEMainWindow::openInTextEditor()
{

	QString file = getCurrentFile()->primaryFile();
	QFileInfo fh(file);
	if (!fh.isReadable())
	{
		int answer = QMessageBox::question(this, APP_NAME,
				tr("File does not exist, do you wish to save the current document?"),
				tr("Yes"),
				tr("Cancel"), 0, 1);
		if (answer == 0)
			save();
		else
			return;
	}
	file = getCurrentFile()->primaryFile();
	fh.setFile(file);
	if (!fh.isReadable())
	{
		QMessageBox::critical(this, APP_NAME, tr("Problem saving!"),
				QMessageBox::Ok,
				QMessageBox::NoButton,
				QMessageBox::NoButton);
		return;
	}
	openInTextEditor(file);
}

void GLEMainWindow::openInTextEditor(const QString& file)
{
	bool res;
	if (QGLE::isGraphicsExtension(file))
	{
		// GLE script may include bitmap graphics - don't open these with text editor
		showLocalFile(file);
		return;
	}
	QString editor = settings->editorLocation();
	if (editor.isEmpty())
	{
		QMessageBox::critical(this, APP_NAME, tr("Please select text editor in Options | Tools"),
				QMessageBox::Ok,
				QMessageBox::NoButton,
				QMessageBox::NoButton);
	}
	else
	{
		QProcess edit;
		QString filename = file;
#ifdef Q_OS_WIN32
		filename.replace('/', '\\');
#endif

#ifdef Q_WS_MAC
		// This checks whether we're on a Mac and if so, handles .app files appropriately
		QRegExp rxApp(".*\\.app");
		if (rxApp.exactMatch(editor))
		{
			res = edit.startDetached("open", QStringList() << "-a" << editor << filename);
		}
		else
#endif
		{
			res = edit.startDetached(editor, QStringList() << filename);
		}
// TEMPORARY FIX for problem with startDetached always failing
#ifndef Q_WS_MAC
		if (!res)
		{
			QMessageBox::critical(this, APP_NAME, tr("Failed to start text editor (configure it in Options | Tools)"),
				QMessageBox::Ok,
				QMessageBox::NoButton,
				QMessageBox::NoButton);
		}
#endif
	}
}

// Set a point upon which to base the relative coordinate system
void GLEMainWindow::setRelativeBasePoint(QPointF gle)
{
	lastPoint = gle;
}

// Get the size of the scroll area
QSize GLEMainWindow::getScrollAreaSize()
{
	return scrollArea->size();
}

// Get the current file name
QString GLEMainWindow::getCurrentGleFile()
{
	return getCurrentFile()->gleFile();
}

GLEInterface* GLEMainWindow::getGLEInterface()
{
	return gleInterface;
}

GLEScript* GLEMainWindow::getGLEScript()
{
	return gleScript.get();
}

// Enable or disable the save button
void GLEMainWindow::setSaveEnable(bool state)
{
	saveAct->setEnabled(state);
}

// Render Postscript code to QImage
void GLEMainWindow::renderPostscript(const char* ps, const GLERectangle& rect, double dpi, QImage* result)
{
	// qDebug() << "Rendering: " << rect.getWidth() << " x " << rect.getHeight();
	renderThread->startRender(rect, dpi);
	QString translate = QString("%1 %2 translate\n").arg(-rect.getXMin()*PS_POINTS_PER_INCH/CM_PER_INCH).arg(-rect.getYMin()*PS_POINTS_PER_INCH/CM_PER_INCH);
	renderThread->nextRender(translate.toLatin1().constData());
	renderThread->nextRender(gleInterface->getInitialPostScript());
	renderThread->nextRender(ps);
	renderThread->nextRender(gleInterface->getTerminatePostScript());
	renderThread->endRender(result);
}

