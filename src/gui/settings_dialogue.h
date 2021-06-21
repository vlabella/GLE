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

#ifndef _SETTINGS_DIALOGUE_H
#define _SETTINGS_DIALOGUE_H

#include <QDialog>
#include <QTabWidget>
#include "mainwindow.h"
#include "../config.h"

//! Tab used to configure application settings
class ApplicationTab : public QWidget
{
	Q_OBJECT

public:
	ApplicationTab(QWidget *parent = 0);

	//! Set the DEFAULT resolution
	void setDPI(int new_dpi);
	//! Get the DEFAULT resolution
	int dpi();
	//! Should we store the application size/position?
	void setStoreSize(bool new_storeSize);
	//! Should we store the application size/position?
	bool storeSize();

	//! Should we store the application working directory?
	bool storeDir();
	//! Should we store the application working directory?
	void setStoreDir(bool new_storeDir);

	//! Should we auto save on preview
	bool saveOnPreview();
	//! Should we auto save on preview
	void setSaveOnPreview(bool new_save);

	//! Should we monitor the open file for changes
	bool monitorOpenFile();
	//! Should we monitor the open file for changes
	void setMonitorOpenFile(bool new_mof);

	//! Should we auto-reload if file changed
	bool monitorAutoReload();
	//! Should we auto-reload if file changed
	void setMonitorAutoReload(bool new_mar);

	//! Should we ask about keeping objects?
	bool askAboutObjects();
	//! Should we ask about keeping objects?
	void setAskAboutObjects(bool newAsk);

	//! Should we auto scale the diagram when opening
	bool autoScaleOnOpen();
	//! Should we auto scale the diagram when opening
	void setAutoScaleOnOpen(bool new_autoscale);

public slots:
	void fileMonitorUpdated(int state);

private:
	QSpinBox *resolutionBox;
	QCheckBox *saveSizeBox;
	QCheckBox *saveDirBox;
	QCheckBox *saveOnPreviewBox;
	QCheckBox *autoScaleOnOpenBox;
	QCheckBox *monitorOpenFileBox;
	QCheckBox *monitorAutoReloadBox;
	QCheckBox *askAboutObjectsBox;
};


//! Tab used to configure server settings
class ServerTab : public QWidget
{
	Q_OBJECT

public:
	ServerTab(QWidget *parent = 0);
	//! Set the port number on which to listen
	void setPort(int new_port);
	//! Get the port number on which to listen
	int port();
	//! Set whether to start the server in preview mode
	void setAutoStart(bool new_autoStart);
	//! Get whether to start the server in preview mode
	bool autoStart();

private:
	QSpinBox *portNumBox;
	QCheckBox *autoStartBox;
};


//! Tab used to configure drawing settings
class DrawingTab : public QWidget
{
	Q_OBJECT

public:
	DrawingTab(QWidget *parent = 0);
	//! Set the grid spacing
	void setGrid(QPointF newGrid);
	//! Get the grid spacing
	QPointF grid();
	//! Set whether grid spacing is equal
	void setEqualGrid(bool state);
	//! Set whether grid spacing is equal
	bool equalGrid();

	//! Set polar snap on start
	void setPolarSnapOnStart(bool state);
	//! Get polar snap on start
	bool polarSnapOnStart();

	//! Set polar track on start
	void setPolarTrackOnStart(bool state);
	//! Get polar track on start
	bool polarTrackOnStart();

	//! Set osnap on start
	void setOsnapOnStart(bool state);
	//! Get osnap on start
	bool osnapOnStart();

	//! Set ortho snap on start
	void setOrthoSnapOnStart(bool state);
	//! Get ortho snap on start
	bool orthoSnapOnStart();

	//! Set grid snap on start
	void setGridSnapOnStart(bool state);
	//! Get grid snap on start
	bool gridSnapOnStart();

	//! Set polar snap start angle
	void setPolarSnapStartAngle(double angle);
	//! Get polar snap start angle
	double polarSnapStartAngle();

	//! Set polar snap Inc angle
	void setPolarSnapIncAngle(double angle);
	//! Get polar snap Inc angle
	double polarSnapIncAngle();

private slots:
	//! Toggle whether the X & Y spacing should be equal
	void equalSpacingToggled(bool state);

private:
	QDoubleSpinBox *xBox;
	QDoubleSpinBox *yBox;
	QCheckBox *equalSpacing;
	QCheckBox *osnapOnStartBox;
	QCheckBox *polarSnapOnStartBox;
	QCheckBox *polarTrackOnStartBox;
	QCheckBox *orthoSnapOnStartBox;
	QCheckBox *gridSnapOnStartBox;
	QDoubleSpinBox *polarSnapStartAngleBox;
	QDoubleSpinBox *polarSnapIncAngleBox;
};

//! Tab used to configure drawing settings
class ToolTab : public QWidget
{
	Q_OBJECT

public:
	ToolTab(QWidget *parent, GLEMainWindow* main);

	void setToolGLE(const QString& vers, const QString& date, const QString& top);
	void setToolGsLib(const QString& value, const QString& vers);
	void setToolEditor(const QString& value);

	inline QString getToolGsLib() { return ToolGsLib->text(); }
	inline QString getToolEditor() { return ToolEditor->text(); }

	//! Render using Cairo library
	bool isRenderUsingCairo();
	void setRenderUsingCairo(bool value);

private slots:
	void exploreGLETop();
	void browseToolGsLib();
	void browseToolEditor();
	void findSoftwareDependencies();
	void editGlobalConfig();
	void editUserConfig();

	private:
	QLineEdit* GLETop;
	QLineEdit* ToolGsLib;
	QLineEdit* ToolEditor;

	QLabel* GLEVersion;
	QLabel* GLEBuildDate;
	QLabel* GsLibVersion;
	GLEMainWindow *mainWin;
	QCheckBox *renderUsingCairo;
};

//! Dialogue box used to configure settings
class SettingsDialogue : public QDialog
{
	Q_OBJECT

public:
	SettingsDialogue(GLEMainWindow *parent);

signals:
	void defaultResolutionChanged(double newdpi);

private slots:
	void okClicked();

private:
	QTabWidget *tabWidget;

	//! Initialise the Application Tab
	void initApplicationTab();
	//! Initialise the Server Tab
	void initServerTab();
	//! Initialise the Drawing Tab
	void initDrawingTab();
	//! Initialise the Tool Tab
	void initToolTab();

	// The tabs
	ApplicationTab *appTab;
	ServerTab *servTab;
	DrawingTab *drawTab;
	ToolTab *toolTab;

	// The parent
	GLEMainWindow *mainWin;
};

#endif
