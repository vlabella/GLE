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

#ifndef _GLESETTINGS_H
#define _GLESETTINGS_H

#include <QtGui>
#include "qgle_definitions.h"

class QSettings;

//! Class definition for settings object
class GLESettings : public QObject
{
	Q_OBJECT
public:

	enum Modes
	{
		PreviewMode,
		EditMode
	};
	//! Constructor
	GLESettings(QObject *parent);

	//! Read all the settings from the OS
	void readAll();
	//! Write all the settings to the OS
	void writeAll();

	// Application configuration
	//! Should we store the application size on exit?
	inline const bool& storeSize() { return storeAppSize; }
	inline void setStoreSize(const bool& new_storeSize) { storeAppSize = new_storeSize; }

	//! Should we store the last used directory on exit?
	inline const bool& storeDirectory() { return storeDirectoryOnExit; }
	inline void setStoreDirectory(const bool& new_storeDir) { storeDirectoryOnExit = new_storeDir; }

	//! Should we autosave when switching to preview mode?
	inline const bool& saveOnPreview() { return autoSaveOnPreview; }
	inline void setSaveOnPreview(const bool& new_save) { autoSaveOnPreview = new_save; }

	//! Should we monitor the open file for changes?
	inline const bool& monitorOpenFile() { return monitorFile; }
	inline void setMonitorOpenFile(const bool& new_monitorFile) { monitorFile = new_monitorFile; emit monitorFileChanged(monitorFile); }

	//! Should we auto reload if file changed?
	inline const bool& monitorAutoReloadFile() { return monitorAutoReload; }
	inline void setMonitorAutoReloadFile(const bool& new_monitorAutoReload) { monitorAutoReload = new_monitorAutoReload; }

	//! Should we autoscale when opening new diagrams
	inline const bool& autoScaleOnOpen() { return autoScaleWhenOpening; }
	inline void setAutoScaleOnOpen(const bool& new_scale) { autoScaleWhenOpening = new_scale; }

	//! Should we store the last used directory on exit?
	inline const QString& pwd() { return currentWorkingDirectory; }
	inline void setPwd(const QString& new_wd) { currentWorkingDirectory = new_wd; }

	//! Where is ghostscript's library?
	inline const QString& getLibGSLocation() { return libGSLocation; }
	inline void setLibGSLocation(const QString& str) { libGSLocation = str; }

	//! Where is my favorite editor
	inline const QString& editorLocation() { return editorExecutable; }
	inline void setEditorLocation(const QString& str) { editorExecutable = str; }

	//! What was the window position last time it was saved?
	inline const QPoint& position() { return windowPosition; }
	inline void setPosition(const QPoint& pos) { windowPosition = pos; }

	//! What was the window's size last time it was saved?
	inline const QSize& size() { return windowSize; }
	inline void setSize(const QSize& new_size) { windowSize = new_size; }

	//! What was the window's size last time it was saved?
	inline const QSize& drawingAreaSize() { return m_DrawingAreaSize; }
	inline void setDrawingAreaSize(const QSize& new_size) { m_DrawingAreaSize = new_size; }

	//! What is the DEFAULT (not LAST) image resolution?
	inline const int& dpi() { return defaultResolution; }
	inline void setDPI(const int& new_dpi) { defaultResolution = new_dpi; }

	// Server Configuration
	//! What port number should we listen on?
	inline const int& port() { return serverPort; }
	inline void setPort(const int& new_port) { serverPort = new_port; emit portChanged(serverPort); }

	//! Should we start the server when the application starts?
	inline const bool& autoStartServer() { return serverAutoStart; }
	inline void setAutoStartServer(const bool& new_autoStart) { serverAutoStart = new_autoStart; }

	// Drawing Configuration
	//! Grid Spacings
	inline const QPointF& grid() { return gridSpacing; }
	inline void setGrid(const QPointF& newGrid) { gridSpacing = newGrid ; emit gridChanged(gridSpacing); }

	inline const double& polarSnapStartAngle() { return polarSnapStartingAngle; }
	inline void setPolarSnapStartAngle(double newAngle) { polarSnapStartingAngle = newAngle ; emit polarSnapStartAngleChanged(polarSnapStartingAngle); }

	inline const double& polarSnapIncAngle() { return polarSnapIncrementAngle; }
	inline void setPolarSnapIncAngle(double newAngle) { polarSnapIncrementAngle = newAngle ; emit polarSnapIncAngleChanged(polarSnapIncrementAngle); }

	//! Should we start in OSNAP mode?
	inline const bool& osnapOnStart() { return startOSNAP; }
	inline void setOsnapOnStart(const bool& new_startosnap) { startOSNAP = new_startosnap; }

	//! Should we start in orthosnap mode?
	inline const bool& orthoSnapOnStart() { return startORTHOSNAP; }
	inline void setOrthoSnapOnStart(const bool& new_startorthosnap) { startORTHOSNAP = new_startorthosnap; }

	//! Should we start in grid snap mode?
	inline const bool& gridSnapOnStart() { return startGRIDSNAP; }
	inline void setGridSnapOnStart(const bool& new_startgridsnap) { startGRIDSNAP = new_startgridsnap; }

	//! Should we start in polar snap mode?
	inline const bool& polarSnapOnStart() { return startPOLARSNAP; }
	inline void setPolarSnapOnStart(const bool& new_startpolar) { startPOLARSNAP = new_startpolar; }

	//! Should we start in polar track mode?
	inline const bool& polarTrackOnStart() { return startPOLARTRACK; }
	inline void setPolarTrackOnStart(const bool& new_startpolartrack) { startPOLARTRACK = new_startpolartrack; }

	//! Should the grid spacing be equal
	inline const bool& equalGrid() { return equalGridSpacing; }
	inline void setEqualGrid(const bool& newEqualGrid) { equalGridSpacing = newEqualGrid; }

	//! Should the user be asked whether to keep objects on reload
	inline const bool& askAboutObjects() { return askWhetherToKeepObjects; }
	inline void setAskAboutObjects(const bool& newAsk) { askWhetherToKeepObjects = newAsk; }

	inline const QByteArray& splitterPosition() { return currentSplitterPos; }
	inline void setSplitterPosition(const QByteArray& pos) { currentSplitterPos = pos; }

	inline void setConsoleWindowAutoShowSize(int height) { consoleWindowAutoShowSize = height; }
	inline int getConsoleWindowAutoShowSize() { return consoleWindowAutoShowSize; }

	inline const QByteArray& mainWindowState() { return currentMainWindowState; }
	inline void setMainWindowState(const QByteArray& state) { currentMainWindowState = state; }

	//! Should emulate a particular GLE version
	inline int getEmulateGLEVersion() { return emulateGLEVersion; }
	inline void setEmulateGLEVersion(int emul) { emulateGLEVersion = emul; }

	//! Should export to a particular format
	inline int getExportFormat() { return exportFormat; }
	inline void setExportFormat(int expf) { exportFormat = expf; }

	//! Should export to a particular page size
	inline int getExportPageSize() { return exportPageSize; }
	inline void setExportPageSize(int exps) { exportPageSize = exps; }

	//! Should preview in a particular page size
	inline int getPreviewPageSize() { return previewPageSize; }
	inline void setPreviewPageSize(int prevs) { previewPageSize = prevs; }

	//! Should open exported figure in previewer
	inline bool isOpenExportedFigure() { return openExportedFigure; }
	inline void setOpenExportedFigure(bool value) { openExportedFigure = value; }

	inline bool isExportGrayScale() { return exportGrayScale; }
	inline void setExportGrayScale(bool value) { exportGrayScale = value; }

	inline bool isExportTransparent() { return exportTransparent; }
	inline void setExportTransparent(bool value) { exportTransparent = value; }

	inline int getExportBitmapResolution() { return exportBitmapResolution; }
	inline void setExportBitmapResolution(int value) { exportBitmapResolution = value; }

	inline int getExportVectorResolution() { return exportVectorResolution; }
	inline void setExportVectorResolution(int value) { exportVectorResolution = value; }

	//! Should render using Cairo backend
	inline bool isRenderUsingCairo() { return renderUsingCairo; }
	inline void setRenderUsingCairo(bool value) { renderUsingCairo = value; }

signals:
	void gridChanged(QPointF grid);
	void portChanged(int newPort);
	void monitorFileChanged(bool mf);
	void polarSnapStartAngleChanged(double newAngle);
	void polarSnapIncAngleChanged(double newAngle);

private:
	QSettings *settingStore;
	QString libGSLocation;
	QString gleExecutable;
	QString editorExecutable;
	int imageDPI;
	QPoint windowPosition;
	QSize windowSize;
	QSize m_DrawingAreaSize;
	bool storeAppSize;
	int serverPort;
	bool serverAutoStart;
	QPointF gridSpacing;
	int defaultResolution;
	bool equalGridSpacing;
	bool autoSaveOnPreview;
	bool autoScaleWhenOpening;
	bool monitorFile;
	bool monitorAutoReload;
	bool askWhetherToKeepObjects;
	int exportFormat;
	int exportPageSize;
	int previewPageSize;
	bool openExportedFigure;
	bool exportGrayScale;
	bool exportTransparent;
	int exportBitmapResolution;
	int exportVectorResolution;
	bool renderUsingCairo;

	bool startOSNAP;
	bool startORTHOSNAP;
	bool startPOLARSNAP;
	bool startPOLARTRACK;
	bool startGRIDSNAP;
	double polarSnapStartingAngle;
	double polarSnapIncrementAngle;

	bool storeDirectoryOnExit;
	int consoleWindowAutoShowSize;
	int emulateGLEVersion;

	QString currentWorkingDirectory;
	QByteArray currentSplitterPos;
	QByteArray currentMainWindowState;
};


#endif
