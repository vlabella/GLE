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
#include "settings.h"
#include "settings_dialogue.h"
#include "dialogues.h"
#include "qgs.h"

// Constructor for the settings dialogue box
SettingsDialogue::SettingsDialogue(GLEMainWindow *parent)
	: QDialog(parent)
{
	mainWin = parent;
	// Create a new tab widget and add tabs for the various
	// categories of setting
	appTab = new ApplicationTab(this);
	servTab = new ServerTab(this);
	drawTab = new DrawingTab(this);
	toolTab = new ToolTab(this, parent);

	tabWidget = new QTabWidget;
	tabWidget->addTab(toolTab, tr("Tools"));
	tabWidget->addTab(appTab, tr("Application"));
	tabWidget->addTab(drawTab, tr("Drawing"));
	tabWidget->addTab(servTab, tr("Server"));

		// Add ok and cancel button
	QPushButton *okButton = new QPushButton(tr("OK"));
	QPushButton *cancelButton = new QPushButton(tr("Cancel"));
	okButton->setDefault(true);

	// Connect to slots: we need to handle what happens on accept
	// and reject
	connect(okButton, SIGNAL(clicked()), this, SLOT(okClicked()));
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

	// Layout the buttons
	QHBoxLayout *buttonLayout = new QHBoxLayout;
	buttonLayout->addStretch(1);
	buttonLayout->addWidget(okButton);
	buttonLayout->addWidget(cancelButton);

	// Layout the dialogue box as a whole
	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(tabWidget);
	mainLayout->addLayout(buttonLayout);
	setLayout(mainLayout);

	initApplicationTab();
	initServerTab();
	initDrawingTab();
	initToolTab();

	resize(mainWin->width()/2, mainWin->height()/2);

	// Set the window title
	setWindowTitle(tr("%1 Settings").arg(APP_NAME));
}

void SettingsDialogue::initApplicationTab()
{
	// Set up the tab based on the application's current settings
	appTab->setDPI(mainWin->settings->dpi());
	appTab->setStoreSize(mainWin->settings->storeSize());
	appTab->setStoreDir(mainWin->settings->storeDirectory());
	appTab->setSaveOnPreview(mainWin->settings->saveOnPreview());
	appTab->setMonitorOpenFile(mainWin->settings->monitorOpenFile());
	appTab->setMonitorAutoReload(mainWin->settings->monitorAutoReloadFile());
	appTab->setAutoScaleOnOpen(mainWin->settings->autoScaleOnOpen());
	appTab->setAskAboutObjects(mainWin->settings->askAboutObjects());
}

void SettingsDialogue::initServerTab()
{
	// Set up the tab based on the application's current settings
	servTab->setPort(mainWin->settings->port());
//	servTab->setAutoStart(mainWin->settings->autoStartServer());
}

void SettingsDialogue::initDrawingTab()
{
	// Set up the tab based on the application's current settings
	drawTab->setGrid(mainWin->settings->grid());
	drawTab->setEqualGrid(mainWin->settings->equalGrid());
	drawTab->setGridSnapOnStart(mainWin->settings->gridSnapOnStart());
	drawTab->setOrthoSnapOnStart(mainWin->settings->orthoSnapOnStart());
	drawTab->setOsnapOnStart(mainWin->settings->osnapOnStart());
	drawTab->setPolarSnapOnStart(mainWin->settings->polarSnapOnStart());
	drawTab->setPolarTrackOnStart(mainWin->settings->polarTrackOnStart());
	drawTab->setPolarSnapStartAngle(mainWin->settings->polarSnapStartAngle());
	drawTab->setPolarSnapIncAngle(mainWin->settings->polarSnapIncAngle());
}

void SettingsDialogue::initToolTab()
{
	// Set up the tab based on the application's current settings
	toolTab->setToolGLE(mainWin->getGLEVersion(), mainWin->getGLEBuildDate(), mainWin->getGLETop());
	toolTab->setToolGsLib(mainWin->settings->getLibGSLocation(), mainWin->getGsLibVersion());
	toolTab->setToolEditor(mainWin->settings->editorLocation());
	toolTab->setRenderUsingCairo(mainWin->settings->isRenderUsingCairo());
}

void SettingsDialogue::okClicked()
{
	// Copy the settings from all the tabs to the application's settings
	mainWin->settings->setStoreSize(appTab->storeSize());
	mainWin->settings->setStoreDirectory(appTab->storeDir());
	mainWin->settings->setSaveOnPreview(appTab->saveOnPreview());
	mainWin->settings->setMonitorOpenFile(appTab->monitorOpenFile());
	mainWin->settings->setMonitorAutoReloadFile(appTab->monitorAutoReload());
	mainWin->settings->setAskAboutObjects(appTab->askAboutObjects());

	if (mainWin->settings->autoScaleOnOpen() != appTab->autoScaleOnOpen())
	{
		mainWin->settings->setAutoScaleOnOpen(appTab->autoScaleOnOpen());
		// not required (?):
		// emit autoScaleChanged(appTab->autoScaleOnOpen());
	}
	if (mainWin->settings->dpi() != appTab->dpi())
	{
		mainWin->settings->setDPI(appTab->dpi());
		emit defaultResolutionChanged((double) appTab->dpi());
	}

	mainWin->settings->setPort(servTab->port());
//	mainWin->settings->setAutoStartServer(servTab->autoStart());

	mainWin->settings->setGrid(drawTab->grid());
	mainWin->settings->setEqualGrid(drawTab->equalGrid());
	mainWin->settings->setGridSnapOnStart(drawTab->gridSnapOnStart());
	mainWin->settings->setOrthoSnapOnStart(drawTab->orthoSnapOnStart());
	mainWin->settings->setOsnapOnStart(drawTab->osnapOnStart());
	mainWin->settings->setPolarSnapOnStart(drawTab->polarSnapOnStart());
	mainWin->settings->setPolarTrackOnStart(drawTab->polarTrackOnStart());
	mainWin->settings->setPolarSnapStartAngle(drawTab->polarSnapStartAngle());
	mainWin->settings->setPolarSnapIncAngle(drawTab->polarSnapIncAngle());

	mainWin->settings->setLibGSLocation(toolTab->getToolGsLib());
	mainWin->settings->setEditorLocation(toolTab->getToolEditor());
	mainWin->settings->setRenderUsingCairo(toolTab->isRenderUsingCairo());

	// Close
	accept();
}


// Constructor for the application settings tab
ApplicationTab::ApplicationTab(QWidget *parent)
	: QWidget(parent)
{
	QGroupBox *settingsGroup = new QGroupBox(tr("Settings"));

	saveSizeBox = new QCheckBox(tr("Save window size and position on exit"));
	saveDirBox = new QCheckBox(tr("Remember working directory"));
	saveOnPreviewBox = new QCheckBox(tr("Auto save on preview"));
	monitorOpenFileBox = new QCheckBox(tr("Monitor file for changes"));
	monitorAutoReloadBox = new QCheckBox(tr("Reload without asking"));
	autoScaleOnOpenBox = new QCheckBox(tr("Auto scale when opening diagrams"));
	askAboutObjectsBox = new QCheckBox(tr("Automatically keep objects when reloading"));

	QLabel *resolutionLabel = new QLabel(tr("Default display resolution (dpi):"));
	resolutionBox = new QSpinBox();
	resolutionBox->setMinimum(0);
	resolutionBox->setMaximum(500);
	resolutionBox->setSingleStep(10);

	QVBoxLayout *settingsLayout = new QVBoxLayout;
	settingsLayout->addWidget(saveSizeBox);
	settingsLayout->addWidget(saveDirBox);
	settingsLayout->addWidget(saveOnPreviewBox);
	settingsLayout->addWidget(autoScaleOnOpenBox);
	settingsLayout->addWidget(monitorOpenFileBox);
	settingsLayout->addWidget(monitorAutoReloadBox);
	// has become obsolete:
	// settingsLayout->addWidget(askAboutObjectsBox);
	settingsLayout->addWidget(resolutionLabel);
	settingsLayout->addWidget(resolutionBox);
	settingsGroup->setLayout(settingsLayout);

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(settingsGroup);
	mainLayout->addStretch(1);
	setLayout(mainLayout);

	connect(monitorOpenFileBox, SIGNAL(stateChanged(int)),
		this, SLOT(fileMonitorUpdated(int)));
}

void ApplicationTab::setDPI(int new_dpi)
{
	resolutionBox->setValue(new_dpi);
}

int ApplicationTab::dpi()
{
	return(resolutionBox->value());
}

void ApplicationTab::setAutoScaleOnOpen(bool new_autoscale)
{
	autoScaleOnOpenBox->setChecked(new_autoscale);
}

bool ApplicationTab::autoScaleOnOpen()
{
	if (autoScaleOnOpenBox->checkState() == Qt::Checked)
		return(true);
	else
		return(false);
}

void ApplicationTab::setStoreSize(bool new_storeSize)
{
	if (new_storeSize)
		saveSizeBox->setCheckState(Qt::Checked);
	else
		saveSizeBox->setCheckState(Qt::Unchecked);

}

bool ApplicationTab::storeSize()
{
	if (saveSizeBox->checkState() == Qt::Checked)
		return(true);
	else
		return(false);
}

void ApplicationTab::setStoreDir(bool new_storeDir)
{
	if (new_storeDir)
		saveDirBox->setCheckState(Qt::Checked);
	else
		saveDirBox->setCheckState(Qt::Unchecked);

}

bool ApplicationTab::storeDir()
{
	if (saveDirBox->checkState() == Qt::Checked)
		return(true);
	else
		return(false);
}

void ApplicationTab::setSaveOnPreview(bool new_save)
{
	if (new_save)
		saveOnPreviewBox->setCheckState(Qt::Checked);
	else
		saveOnPreviewBox->setCheckState(Qt::Unchecked);

}

bool ApplicationTab::saveOnPreview()
{
	if (saveOnPreviewBox->checkState() == Qt::Checked)
		return(true);
	else
		return(false);
}

void ApplicationTab::setMonitorOpenFile(bool new_save)
{
	if (new_save)
		monitorOpenFileBox->setCheckState(Qt::Checked);
	else
		monitorOpenFileBox->setCheckState(Qt::Unchecked);

}

bool ApplicationTab::monitorOpenFile()
{
	if (monitorOpenFileBox->checkState() == Qt::Checked)
		return(true);
	else
		return(false);
}

void ApplicationTab::setMonitorAutoReload(bool new_save)
{
	if (new_save)
		monitorAutoReloadBox->setCheckState(Qt::Checked);
	else
		monitorAutoReloadBox->setCheckState(Qt::Unchecked);
}

void ApplicationTab::fileMonitorUpdated(int state)
{
	monitorAutoReloadBox->setEnabled(state == Qt::Checked);
}

bool ApplicationTab::monitorAutoReload()
{
	if (monitorAutoReloadBox->checkState() == Qt::Checked)
		return(true);
	else
		return(false);
}

void ApplicationTab::setAskAboutObjects(bool newAsk)
{
	if (newAsk)
		askAboutObjectsBox->setCheckState(Qt::Unchecked);
	else
		askAboutObjectsBox->setCheckState(Qt::Checked);
}

bool ApplicationTab::askAboutObjects()
{
	if (askAboutObjectsBox->checkState() == Qt::Unchecked)
		return(true);
	else
		return(false);
}

// Constructor for the server settings tab
ServerTab::ServerTab(QWidget *parent)
	: QWidget(parent)
{
	QLabel *portNumLabel = new QLabel(tr("Server port number:"));
	portNumBox = new QSpinBox();
	portNumBox->setMaximum(65535);
	portNumBox->setMinimum(1);
	portNumBox->setSingleStep(1);
	// This needs to change to read from the settings!
	portNumBox->setValue(6667);

	QVBoxLayout *all = new QVBoxLayout();
	QGridLayout *mainLayout = new QGridLayout();
	mainLayout->addWidget(portNumLabel,0,0,1,1,Qt::AlignLeft);
	mainLayout->addWidget(portNumBox,0,1,1,1,Qt::AlignLeft);
	all->addLayout(mainLayout);
	all->addStretch(1);
	setLayout(all);
}

void ServerTab::setPort(int new_port)
{
	portNumBox->setValue(new_port);
}

int ServerTab::port()
{
	return(portNumBox->value());
}

void ServerTab::setAutoStart(bool new_autoStart)
{
	if (new_autoStart)
		autoStartBox->setCheckState(Qt::Checked);
	else
		autoStartBox->setCheckState(Qt::Unchecked);
}

bool ServerTab::autoStart()
{
	if (autoStartBox->checkState() == Qt::Checked)
		return(true);
	else
		return(false);
}


// Constructor for the drawing settings tab
DrawingTab::DrawingTab(QWidget *parent)
	: QWidget(parent)
{
	QGroupBox *gridSpacingGroup = new QGroupBox(tr("Grid Spacing"));
	QGroupBox *snapControlGroup = new QGroupBox(tr("Snap Control"));

	QLabel *xLabel = new QLabel(tr("X:"));
	xBox = new QDoubleSpinBox();
	xBox->setMaximum(100.0);
	xBox->setMinimum(0.005);
	xBox->setDecimals(3);
	xBox->setSingleStep(0.1);

	QLabel *yLabel = new QLabel(tr("Y:"));
	yBox = new QDoubleSpinBox();
	yBox->setMaximum(100.0);
	yBox->setMinimum(0.005);
	yBox->setDecimals(3);
	yBox->setSingleStep(0.1);


	equalSpacing = new QCheckBox("Equal spacing");
	connect(equalSpacing, SIGNAL(toggled(bool)),
			this, SLOT(equalSpacingToggled(bool)));


	QGridLayout *gridSpacingLayout = new QGridLayout;
	gridSpacingLayout->addWidget(xLabel,0,0,1,1,Qt::AlignLeft);
	gridSpacingLayout->addWidget(xBox,0,1,1,1,Qt::AlignLeft);
	gridSpacingLayout->addWidget(yLabel,1,0,1,1,Qt::AlignLeft);
	gridSpacingLayout->addWidget(yBox,1,1,1,1,Qt::AlignLeft);
	gridSpacingLayout->addWidget(equalSpacing,2,0,1,5,Qt::AlignLeft);
	gridSpacingGroup->setLayout(gridSpacingLayout);

	QGridLayout *snapLayout = new QGridLayout;

	gridSnapOnStartBox = new QCheckBox(tr("Start with grid snap enabled"));
	osnapOnStartBox = new QCheckBox(tr("Start with object snap enabled"));
	orthoSnapOnStartBox = new QCheckBox(tr("Start with orthogonal snap enabled"));
	polarSnapOnStartBox = new QCheckBox(tr("Start with polar snap enabled"));
	polarTrackOnStartBox = new QCheckBox(tr("Start with polar tracking enabled"));
	QLabel *startLabel = new QLabel(tr("Polar snap start angle:"));
	QLabel *incLabel = new QLabel(tr("Polar snap increment angle:"));
	polarSnapStartAngleBox = new QDoubleSpinBox();
	polarSnapStartAngleBox->setMaximum(360.0);
	polarSnapStartAngleBox->setMinimum(0.0);
	polarSnapStartAngleBox->setSingleStep(5.0);
	polarSnapIncAngleBox = new QDoubleSpinBox();
	polarSnapIncAngleBox->setMaximum(360.0);
	polarSnapIncAngleBox->setMinimum(0.0);
	polarSnapIncAngleBox->setSingleStep(5.0);

	snapLayout->addWidget(osnapOnStartBox,0,0,1,5,Qt::AlignLeft);
	snapLayout->addWidget(polarSnapOnStartBox,1,0,1,5,Qt::AlignLeft);
	snapLayout->addWidget(polarTrackOnStartBox,2,0,1,5,Qt::AlignLeft);
	snapLayout->addWidget(orthoSnapOnStartBox,3,0,1,5,Qt::AlignLeft);
	snapLayout->addWidget(gridSnapOnStartBox,4,0,1,5,Qt::AlignLeft);
	snapLayout->addWidget(startLabel,5,0,1,1,Qt::AlignLeft);
	snapLayout->addWidget(polarSnapStartAngleBox,5,1,1,1,Qt::AlignLeft);
	snapLayout->addWidget(incLabel,6,0,1,1,Qt::AlignLeft);
	snapLayout->addWidget(polarSnapIncAngleBox,6,1,1,1,Qt::AlignLeft);
	snapControlGroup->setLayout(snapLayout);

	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(gridSpacingGroup);
	layout->addWidget(snapControlGroup);
	setLayout(layout);
}

void DrawingTab::equalSpacingToggled(bool state)
{
	yBox->setValue(xBox->value());
	yBox->setEnabled(!state);
}

void DrawingTab::setGrid(QPointF newGrid)
{
	xBox->setValue(newGrid.x());
	yBox->setValue(newGrid.y());
}

void DrawingTab::setEqualGrid(bool state)
{
	equalSpacing->setChecked(state);
}

bool DrawingTab::equalGrid()
{
	return(equalSpacing->isChecked());
}

QPointF DrawingTab::grid()
{
	if (equalSpacing->checkState() == Qt::Checked)
		return(QPointF(xBox->value(),xBox->value()));
	else
		return(QPointF(xBox->value(),yBox->value()));
}


//! Set polar snap on start
void DrawingTab::setPolarSnapOnStart(bool state)
{
	if (state)
		polarSnapOnStartBox->setChecked(Qt::Checked);
	else
		polarSnapOnStartBox->setChecked(Qt::Unchecked);
}
//! Get polar snap on start
bool DrawingTab::polarSnapOnStart()
{
	if (polarSnapOnStartBox->isChecked())
		return(true);
	else
		return(false);
}

//! Set polar snap on start
void DrawingTab::setPolarTrackOnStart(bool state)
{
	if (state)
		polarTrackOnStartBox->setChecked(Qt::Checked);
	else
		polarTrackOnStartBox->setChecked(Qt::Unchecked);
}
//! Get polar snap on start
bool DrawingTab::polarTrackOnStart()
{
	if (polarTrackOnStartBox->isChecked())
		return(true);
	else
		return(false);
}

//! Set osnap on start
void DrawingTab::setOsnapOnStart(bool state)
{
	if (state)
		osnapOnStartBox->setChecked(Qt::Checked);
	else
		osnapOnStartBox->setChecked(Qt::Unchecked);
}
//! Get osnap on start
bool DrawingTab::osnapOnStart()
{
	if (osnapOnStartBox->isChecked())
		return(true);
	else
		return(false);
}

//! Set ortho snap on start
void DrawingTab::setOrthoSnapOnStart(bool state)
{
	if (state)
		orthoSnapOnStartBox->setChecked(Qt::Checked);
	else
		orthoSnapOnStartBox->setChecked(Qt::Unchecked);
}
//! Get ortho snap on start
bool DrawingTab::orthoSnapOnStart()
{
	if (orthoSnapOnStartBox->isChecked())
		return(true);
	else
		return(false);
}

//! Set grid snap on start
void DrawingTab::setGridSnapOnStart(bool state)
{
	if (state)
		gridSnapOnStartBox->setChecked(Qt::Checked);
	else
		gridSnapOnStartBox->setChecked(Qt::Unchecked);
}
//! Get grid snap on start
bool DrawingTab::gridSnapOnStart()
{
	if (gridSnapOnStartBox->isChecked())
		return(true);
	else
		return(false);
}

//! Set polar snap start angle
void DrawingTab::setPolarSnapStartAngle(double angle)
{
	polarSnapStartAngleBox->setValue(angle);
}
//! Get polar snap start angle
double DrawingTab::polarSnapStartAngle()
{
	return(polarSnapStartAngleBox->value());
}

//! Set polar snap Inc angle
void DrawingTab::setPolarSnapIncAngle(double angle)
{
	polarSnapIncAngleBox->setValue(angle);
}
//! Get polar snap Inc angle
double DrawingTab::polarSnapIncAngle()
{
	return(polarSnapIncAngleBox->value());
}

ToolTab::ToolTab(QWidget *parent, GLEMainWindow *main) : QWidget(parent)
{
	mainWin = main;
	ToolGsLib = new QLineEdit();
	ToolEditor = new QLineEdit();
	GLEVersion = new QLabel();
	GLEBuildDate = new QLabel();
	GLETop = new QLineEdit();
	GsLibVersion = new QLabel();
	renderUsingCairo = new QCheckBox(tr("Render with the Cairo graphics library"));
#ifndef HAVE_CAIRO
	renderUsingCairo->setEnabled(false);
#endif

	QGridLayout *gle = new QGridLayout();
	gle->addWidget(GLETop,0,0);
	GLETop->setReadOnly(true);
	QPushButton *top_expl = new QPushButton(tr("Explore"));
	gle->addWidget(top_expl,0,1);
	connect(top_expl, SIGNAL(clicked()), this, SLOT(exploreGLETop()));
	gle->setColumnStretch(0, 100);

	QGridLayout *gs = new QGridLayout();
	gs->addWidget(ToolGsLib,0,0);
	QPushButton *gs_browse = new QPushButton(tr("Browse"));
	gs->addWidget(gs_browse,0,1);
	connect(gs_browse, SIGNAL(clicked()), this, SLOT(browseToolGsLib()));
	gs->setColumnStretch(0, 100);

	QGridLayout *editor = new QGridLayout();
	editor->addWidget(ToolEditor,0,0);
	QPushButton *edit_browse = new QPushButton(tr("Browse"));
	editor->addWidget(edit_browse,0,1);
	connect(edit_browse, SIGNAL(clicked()), this, SLOT(browseToolEditor()));
	editor->setColumnStretch(0, 100);

	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(GLEVersion);
	layout->addWidget(GLEBuildDate);
	layout->addWidget(new QLabel(tr("GLE_TOP:")));
	layout->addLayout(gle);
	layout->addWidget(GsLibVersion);
	layout->addLayout(gs);
	layout->addWidget(new QLabel(tr("Text editor")));
	layout->addLayout(editor);
	layout->addWidget(renderUsingCairo);
	layout->addStretch(1);

	QHBoxLayout *configLayo = new QHBoxLayout();
	configLayo->addWidget(new QLabel(tr("Edit GLE's configuration:")));
	QPushButton *globalConf = new QPushButton(tr("Global"));
	connect(globalConf, SIGNAL(clicked()), this, SLOT(editGlobalConfig()));
	configLayo->addWidget(globalConf);
	QPushButton *userConf = new QPushButton(tr("User"));
	if (mainWin->getGLEInterface()->getUserConfigLocation() == "") {
		userConf->setEnabled(false);
	} else {
		connect(userConf, SIGNAL(clicked()), this, SLOT(editUserConfig()));
	}
	configLayo->addWidget(userConf);
	configLayo->addStretch(1);
	QPushButton *findDeps = new QPushButton(tr("Auto Find Software"));
	connect(findDeps, SIGNAL(clicked()), this, SLOT(findSoftwareDependencies()));
	configLayo->addWidget(findDeps);
	layout->addLayout(configLayo);

	setLayout(layout);
}

void ToolTab::exploreGLETop() {
	mainWin->showLocalFile(mainWin->getGLETop());
}

void ToolTab::browseToolGsLib() {
	QString dir = QGLE::GetDirName(ToolGsLib->text());
	if (dir.isEmpty()) dir = "/";
	// don't use stock dialog; otherwise it's impossible to select a framework on MacOS/X
	QFileDialog dialog(this, tr("Locate the GhostScript library"), dir, QGLE::libraryFilter());
	dialog.setFileMode(QFileDialog::AnyFile);
	if (dialog.exec()) {
		QStringList locs = dialog.selectedFiles();
		if (locs.size() > 0 && locs[0] != "") {
			ToolGsLib->setText(locs[0]);
		}
	}
}

void ToolTab::browseToolEditor() {
	QString dir = QGLE::GetDirName(ToolEditor->text());
	if (dir.isEmpty()) dir = "/";
	QString location = QFileDialog::getOpenFileName(this, tr("Locate the executable of your text editor"), dir,
		QGLE::executableFilter());
	if (!location.isEmpty()) ToolEditor->setText(location);
}

void ToolTab::setToolGLE(const QString& vers, const QString& date, const QString& top)
{
	GLEVersion->setText(QString("GLE version: ") + vers);
	GLEBuildDate->setText(QString("GLE build date: ") + date);
	GLETop->setText(top);
}

void ToolTab::setToolGsLib(const QString& value, const QString& vers)
{
	ToolGsLib->setText(value);
	GsLibVersion->setText(QString("GhostScript DLL (v") + vers + QString(")"));
}

void ToolTab::setToolEditor(const QString& value)
{
	ToolEditor->setText(value);
}

void ToolTab::findSoftwareDependencies()
{
	SoftwareLocateDialogue dial(mainWin, mainWin->getGLEInterface(), QGLE_SOFT_DIALOG_SEARCH_CANCEL | QGLE_SOFT_DIALOG_SEARCH_MANUAL);
	dial.setWindowTitle("GLE - Finding Dependencies");
	dial.print(tr("\nClick 'Search Automatically' to make GLE search common locations for software tools, such as LaTeX and GhostScript.\n"));
	dial.exec();
	// Don't forget to restore the console output
	mainWin->restoreConsoleOutput();
	if (!dial.isCancel()) {
		GSLibFunctions* fct = GSLibFunctions::getInstance();
		setToolGsLib(fct->libGSLocation(), mainWin->getGsLibVersion());
		if (dial.isConfigModified()) {
			mainWin->getGLEInterface()->saveRCFile();
		}
	}
}

void ToolTab::editGlobalConfig()
{
	QString name = QString::fromUtf8(mainWin->getGLEInterface()->getGLETop());
	name += QDir::separator();
	name += tr("glerc");
	mainWin->openInTextEditor(name);
}

void ToolTab::editUserConfig()
{
	QString name = QString::fromUtf8(mainWin->getGLEInterface()->getUserConfigLocation().c_str());
	mainWin->openInTextEditor(name);
}

bool ToolTab::isRenderUsingCairo()
{
	return renderUsingCairo->checkState() == Qt::Checked;
}

void ToolTab::setRenderUsingCairo(bool value)
{
	renderUsingCairo->setCheckState(value ? Qt::Checked : Qt::Unchecked);
}
