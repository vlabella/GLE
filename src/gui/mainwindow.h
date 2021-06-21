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

#ifndef __GLEMAINWINDOW_H
#define __GLEMAINWINDOW_H

#include <QtGui>
#include "qgs.h"
#include "serverthread.h"
#include "drawingobject.h"
#include "qgle_definitions.h"
#include "fileinfo.h"
#include "about.h"
#include "propertyeditor.h"
#include "objectblocks.h"

#include "../gle/gle-interface/gle-interface.h"

class GLESettings;
class ConsoleWindow;
class EvaluatorDialog;
class QDockWidget;

//! Class implementing Main QGLE Window
/*!
 * This class is responsible for controlling the graphical user
 * interface, setting up signals & slots, providing menu bars and so
 * on.  Like most of the classes in this project, it is a descendant of
 * QObject, so has access to internationalisation functions etc.
 */
class GLEMainWindow : public QMainWindow
{
	// The QT moc macro
	Q_OBJECT

public:
	//! Constructor
	/*!
	 * Responsible for generation of the Window and initialisation
	 * of children, signals & slots.
	 */
	GLEMainWindow(int argc, char *argv[]);

	//! Return the GLE version
	QString getGLEVersion();
	QString getGLEBuildDate();
	//! Return the GLE version
	const char* getGLETop();
	//! Return the current compatibility mode
	QString getCompatibility();
	//! Return true if in full page mode
	bool isFullPage();
	//! Return true if in landscape mode
	bool isLandscape();
	//! Return the GS version
	inline const QString& getGsLibVersion() { return GsLibVersionNumber; }
	//! Render Postscript code to QImage
	void renderPostscript(const char* ps, const GLERectangle& rect, double dpi, QImage* result);
	//! Render an EPS file from currentFile.epsFile()
	void renderEPS();
	//! Render an EPS file from currentFile.epsFile()
	void renderEPS(double dpi);
	//! Render an EPS file
	void renderEPS(QString epsFile, double dpi, const QSize& area);
	// Refresh GLEDrawing's display
	void refreshDisplay();
	//! Select the pointer tool from the edit mode toolbar
	void selectPointerTool();

protected:
	//! Called when QGLE is closing
	void closeEvent(QCloseEvent *event);
	//! Called when a MIME object is dragged onto window
	void dragEnterEvent(QDragEnterEvent *event);
	//! Called when a MIME object is dropped on the window
	void dropEvent(QDropEvent *event);
	//! Called when the user presses a key
	void keyPressEvent(QKeyEvent *event);
	//! Called when the user releases a key
	void keyReleaseEvent(QKeyEvent *event);

private slots:
	//! Display an About Box
	void about();
	//! Open the GLE website
	void showGLEWebsite();
	//! Open a given URL
	void showURL(const QUrl&);
	//! Open the GLE manual
	void showGLEManual();
	//! Abort QGLE
	void abort();
	//! Display a server status message
	void updateServerStatus(QString message);
	//! Display an error message in the console
	void showConsoleError(QString message);
	//! Start or stop the server
	void serverToggle(bool switchOn);
	//! Start the server
	void startServer();
	//! Stop the server
	void stopServer();
	//! Initialize render thread
	void initRenderThread();
	//! Display the settings dialogue box
	void openSettingsDialogue();
	//! Display GhostScript's log dialogue box
	void openGhostScriptLogDialogue();
	//! Display 3D viewer
	void openView3DDialogue();
	//! Display the expression evaluator dialogue box
	void openEvaluator();
	//! Display export dialogue box
	void openExportDialogue();
	//! Display print dialogue box
	void openPrintDialogue();
	//! Open download dialog box
	void openDownloadDialogue();
	//! Open file in new window
	void openFileInNewWindow();
	//! Browse the folder containing the current script
	void browseScriptFolder();
	//! Open an existing file
	void openFile(QString fileName = QString(), bool isOnStartup = false);
	//! Open a recently opened file
	void openRecentFile();
	//! Open a related file
	void openRelatedFile();
	//! Create "open related file" submenu
	void createEditFileMenu();
	//! Create "open in editor" popup submenu
	void createEditFilePopup(const QPoint& pos);
	//! Create a new file
	void newFile();
	//! Save the file
	void save();
	//! Save the file under a new name
	void saveAs();
	//! Save the file under the given name
	void save(QString fileName);
	//! Action causing the current bitmap to be copied to the clipboard
	void copyBitmap();
	//! Action causing the current figure to be copied to the clipboard as PDF
	void copyPDF();
	//! Action causing the current figure to be copied to the clipboard as EMF
	void copyEMF();
	//! Switch to preview mode
	void previewModeToggle(bool state);
	//! Switch to edit mode
	void editModeToggle(bool state);

	//! Switch to pointer tool
	void pointerToolToggle(bool state);
	//! Switch to line tool
	void lineToolToggle(bool state);
	void textToolToggle(bool state);
	void tanLineToolToggle(bool state);
	void perpLineToolToggle(bool state);
	//! Switch to circle tool
	void circleToolToggle(bool state);
	//! Switch to ellipse tool
	void ellipseToolToggle(bool state);
	//! Switch to move tool
	void moveToolToggle(bool state);
	//! Switch to arc tool
	void arcToolToggle(bool state);
	//! Switch off all tools
	void noToolEnable();
	//! Reload file
	void reloadFile();
	//! Zoom in
	void zoomIn();
	//! Zoom out
	void zoomOut();
	//! Zoom automatically
	void zoomAuto();
	//! Zoom worker function (called by zoomIn() and zoomOut())
	void zoom(double dpi);
	//! Open current file in text editor
	void openInTextEditor();
	//! Get the current file's data (name, .eps, modification time, ...)
	inline GLEFileInfo* getCurrentFile() { return &currentFile[currentMode]; }
	//! Get non-current mode GLEFileInfo object
	GLEFileInfo* getNonCurrentModeFileInfo();
	//! Set temp files correctly for GLE file
	void initTempFilesGLE();
	//! Update currentFile on change PreviewMode <-> EditMode
	int updateFileInfoOnChangeMode(int mode);
public slots:
	//! Update the coordinate display
	void updateMousePosition(QPointF gle = QPointF(-1.0,-1.0));
	//! Process newly rendered image (from QGLE)
	void renderComplete(QImage image);
	//! Set the base point for relative coordinates
	void setRelativeBasePoint(QPointF gle);
	//! Display a status bar message
	void statusBarMessage(QString msg);
	//! Start or stop the file monitor
	void updateFileMonitor(bool state);
	//! Check whether the file has changed
	void checkForFileUpdates();
	//! Enable or disable the save menu option
	void setSaveEnable(bool state);
	//! Update the status bar display of resolution
	void updateDPIDisplay(double newDPI);
	//! Polar snap toggle
	void polarSnapToggle(bool state);
	//! Polar track toggle
	void polarTrackToggle(bool state);
	//! Emulate GLE version changed
	void emulateGLEVersionChanged(int);
	//! Preview page size changed
	void previewPageSizeChanged(int);
signals:
	//! A new tool has been selected
	void toolSelected(int newTool);
	//! The image has changed, so update the drawing
	void imageChanged(QImage newImage);
	//! There's a new EPS file to be rendered
	void newEPS(QString file, double dpi, bool autoscale);
private:
	//! Create the actions for use in menus and toolbars.
	void createActions();
	//! Create the menus.
	void createMenus();
	//! Create a simple status bar at the bottom of the screen.
	void createStatusBar();
	//! Create the tool bars
	void createToolBars();
	//! Create the dock windows
	void createDockWindows();
	//! Add the edit mode toolbars
	void createEditModeToolBars();
	//! Destroy the edit mode toolbars
	void destroyEditModeToolBars();
	//! Show/hide and enable/disable edit mode toolbars
	void enableEditModeToolBars(bool enable);
	//! Create a temporary file
	QString createTempFile(QString ext);

	//! A box to show the about message
	AboutBox *box;

	//! Remember the last mouse position
	QPointF lastMousePosition;

	//! Check whether GLE works
	bool checkGLE();

	//! Render an GLE file from currentFile.gleFile()
	void renderGLE();
	//! Render an GLE file
	void renderGLE(double dpi, const QSize& area);
	//! Commit changes to drawing to GLE
	void updateGLE();

	//! Initialise GS
	void initLibGS();
	//! Sometimes one has to ask anyway before reload (auto reload not possible)
	bool askAnywayBeforeReload();

	void setCurrentFile(const QString &fileName);
	void updateRecentFileActions();

	//! Execute this after rendering a GLE script
	void executeToDoAfterRenderGLE();
	//! Execute this after opening a file
	void executeToDoAfterFileOpen();

	//! Menu/toolbar member variables: file menu
	QMenu *fileMenu;
	//! Menu/toolbar member variables: tools menu
	QMenu *toolsMenu;
	//! Menu/toolbar member variables: help menu
	QMenu *helpMenu;

	//! Open related files menu (submenu of fileMenu)
	QMenu *editFileMenu;

	//! What form should the coordinate display take?
	int coordView;

	//! Should we restore the mode after rendering
	bool saveMode;

	//! Reload icons
	QIcon reloadIconBlack;
	QIcon reloadIconRed;

	//! Action causing the application to quit
	QAction *quitAct;
	//! Action causing the current bitmap to be copied to the clipboard
	QAction *copyBitmapAct;
	//! Action causing the figure to be copied to the clipboard as PDF
	QAction *copyPDFAct;
	//! Action causing the figure to be copied to the clipboard as EMF
	QAction *copyEMFAct;
	//! Linked to the slot: about()
	QAction *aboutAct;
	//! Open GLE website
	QAction* gleSiteAct;
	//! Open GLE manual
	QAction* gleManualAct;
	//! Toggle the grid on and off
	QAction *gridAct;
	//! Toggle grid snap on and off
	QAction *gridSnapAct;
	//! Toggle the status of the server on and off
	QAction *serverAct;
	//! Toggle orthosnap
	QAction *orthoSnapAct;
	//! Toggle osnap
	QAction *osnapAct;
	//! Toggle polar snap
	QAction *polarSnapAct;
	//! Toggle polar track
	QAction *polarTrackAct;
	//! Display the settings dialogue box
	QAction *settingsAct;
	//! Display expression evaluator
	QAction *evaluatorAct;
	//! View 3D part of script
	QAction *view3DAct;
	//! Display GhostScript's log window
	QAction *ghostScriptLogWindowAct;
	//! Open an existing file
	QAction *openInNewWindowAct;
	//! Open an existing file
	QAction *openAct;
	//! Browse the script folder
	QAction *browseAct;
	//! Save the file under a new name
	QAction *saveAsAct;
	//! Save the file
	QAction *saveAct;
	//! Create a new file
	QAction *newAct;
	//! Export the file
	QAction *exportAct;
	//! Print the file
	QAction *printAct;
	//! Open file in text editor
	QAction *editorAct;
	//! Go to preview mode
	QAction *previewModeAct;
	//! Go to edit mode
	QAction *editModeAct;
	//! Reload Action
	QAction *reloadAct;
	//! Zoom In Action
	QAction *zoomInAct;
	//! Zoom Out Action
	QAction *zoomOutAct;
	//! Zoom Automatically Action
	QAction *zoomAutoAct;
	//! Download action
	QAction *downloadAct;
	//! Console window auto-show action
	QAction *consoleWindowHide;
	//! The GLE version to emulate
	QComboBox *emulateVersion;
	//! The page size to use during preview
	QComboBox *previewPageSize;
	//! Used by the recent file list
	QAction *separatorAct;

	//! Group for edit / preview mode buttongs
	QActionGroup* editPreviewGroup;
	//! Group editing tools
	QActionGroup* toolGroup;

	//! Pointer tool
	QAction *pointerToolAct;
	QAction *lineToolAct;
	QAction *textToolAct;
	QAction *tanLineToolAct;
	QAction *perpLineToolAct;
	QAction *circleToolAct;
	QAction *ellipseToolAct;
	QAction *arcToolAct;
	QAction *moveToolAct;

	//! Coordinate Display
	QLabel *mousePosition;
	//! Resolution Display
	QLabel *dpiDisplay;
	//! Grid snap button
	QPushButton *gridSnapButton;
	//! Open file in text editor
	QToolButton *editorBut;

	//! Server thread
	GLEServerThread *serverThread;
	//! Render thread
	GLERenderThread *renderThread;

	//! Current file information
	GLEFileInfo currentFile[2];

	//! Scroll Area containing the drawing widget
	QScrollArea *scrollArea;

	//! Size of (new) diagram
	QSizeF newDiagramSize;


	//! Is the server running?
	bool serverRunning;
	//! Was it running in preview mode?
	bool serverInPreviewMode;

	//! What mode are we currently in?
	int currentMode;

	//! What was the last mode
	int oldMode;

	//! Are we mid-render?
	bool renderInProgress;

	//! ToDo list options
	enum
	{
		ToDoAfterRenderGLE,
		ToDoAfterFileOpen
	};

	//! Enumeration of the various coordinate display options
	enum
	{
		CoordinateOff,
		CoordinateCart,
		CoordinatePolar,
		CoordinateRelCart,
		CoordinateRelPolar
	};

	//! Enumeration for a hash of temporary files
	enum
	{
		EditModeOutput,
		PreviewModeOutput,
		NewGLE
	};
	//! Hash of temporary files
	QHash<int,QString> tempFiles;

	//! The last point clicked (for relative coordinates)
	QPointF lastPoint;

	//! File toolbar
	QToolBar *fileToolBar;
	//! Control toolbar
	QToolBar *controlToolBar;
	//! Mode toolbar
	QToolBar *modeToolBar;
	//! Tools toolbar
	QToolBar *toolsToolBar;
	//! View toolbar
	QToolBar *viewToolBar;
	//! Compatibility toolbar
	QToolBar *compatToolBar;

	//! Property editor
	GLEPropertyEditor *propertyEditor;
	QDockWidget *propertyDock;

	//! Object blocks list
	GLEObjectBlocksList *objectBlocksList;
	QDockWidget *objectBlocksDock;

	QSplitter *splitWindow;
	ConsoleWindow* consoleWindow;
	EvaluatorDialog* evaluatorDialog;

	QString GsLibVersionNumber;

	//! Timer for checking for file changes
	QTimer *fileMonitorTimer;
	int fileMonitorCount;

	// Pointer to our GLE interface
	GLEInterface *gleInterface;

	QList<int> todoList;

	GLERC<GLEScript> gleScript;

	enum { MaxRecentFiles = 5 };
	QAction *recentFileActs[MaxRecentFiles];

	bool enabledState;
public:
	//! The drawing area
	GLEDrawingArea *drawingArea;

	//! Settings for application
	GLESettings *settings;

	//! Remove all drawn objects
	void resetDrawing();
	//! Return the size of the drawing area
	QSize getScrollAreaSize();
	//! Return the name of the current file
	QString getCurrentGleFile();

	GLEInterface* getGLEInterface();

	GLEScript* getGLEScript();

	//! Open given file in text editor
	void openInTextEditor(const QString& file);

	//! Open a local file using the mime type application registered for it
	void showLocalFile(const QString& name);

	bool tryGhostScriptLocation(const QString location, QString& error);

	void updateWindowTitle();

	void updateEnableActions(bool enable);

	void executeToDos();
	void addFilesToMonitor();

	//! Show console window if there is output
	void shouldAutoShowConsole();
	void clearConsoleWindow();
	void restoreConsoleOutput();
};

#endif
