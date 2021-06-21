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

#include "settings.h"
#include <QSettings>

// Constructor: create the settings object
GLESettings::GLESettings(QObject *parent) : QObject(parent),
consoleWindowAutoShowSize(0)
{
	settingStore = new QSettings("gle", "qgle", this);
}

void GLESettings::readAll()
{
	// Read the application settings
	setPosition(settingStore->value("application/position", QPoint(200,200)).toPoint());
	setSize(settingStore->value("application/size", QSize(400,400)).toSize());
	setMainWindowState(settingStore->value("application/mainstate").toByteArray());
	setDrawingAreaSize(settingStore->value("application/drawingsize", QSize(400,400)).toSize());
	setStoreSize(settingStore->value("application/storeSize", true).toBool());
	setStoreDirectory(settingStore->value("application/storeDirectory", true).toBool());
	setSaveOnPreview(settingStore->value("application/saveOnPreview", false).toBool());
	setAskAboutObjects(settingStore->value("application/askAboutKeepingObjects", true).toBool());
	setAutoScaleOnOpen(settingStore->value("application/autoScaleOnOpen", true).toBool());
	setLibGSLocation(settingStore->value("application/libGSLocation", QString()).toString());
	setEditorLocation(settingStore->value("application/editorLocation", QString("")).toString());
	setDPI(settingStore->value("application/resolution", 100).toInt());
	setMonitorOpenFile(settingStore->value("application/monitorOpenFile", true).toBool());
	setMonitorAutoReloadFile(settingStore->value("application/monitorAutoReload", true).toBool());
	setSplitterPosition(settingStore->value("application/splitterSizes").toByteArray());
	setConsoleWindowAutoShowSize(settingStore->value("application/consoleAutoShowSize", 0).toInt());
	setEmulateGLEVersion(settingStore->value("application/emulateGLEVersion", 0).toInt());
	setExportFormat(settingStore->value("application/exportFormat", 0).toInt());
	setExportPageSize(settingStore->value("application/exportPageSize", 0).toInt());
	setPreviewPageSize(settingStore->value("application/previewPageSize", 0).toInt());
	setOpenExportedFigure(settingStore->value("application/openExportedFigure", true).toBool());
	setExportGrayScale(settingStore->value("application/exportGrayScale", false).toBool());
	setExportTransparent(settingStore->value("application/exportTransparent", false).toBool());
	setExportBitmapResolution(settingStore->value("application/exportBitmapResolution", 150).toInt());
	setExportVectorResolution(settingStore->value("application/exportVectorResolution", 600).toInt());
	setRenderUsingCairo(settingStore->value("application/renderUsingCairo", false).toBool());

	if (storeDirectory())
		setPwd(settingStore->value("application/workingDirectory", "").toString());

	// Read the server settings
	setPort(settingStore->value("server/portNumber", DEFAULT_PORT).toInt());
	setAutoStartServer(settingStore->value("server/autoStart", true).toBool());

	// Read the drawing settings
	setGrid(QPointF(settingStore->value("drawing/gridX", 1.0).toDouble(),
	                settingStore->value("drawing/gridY", 1.0).toDouble()));
	setEqualGrid(settingStore->value("drawing/equalGrid", false).toBool());
	setPolarSnapStartAngle(settingStore->value("drawing/polarSnapStartAngle", 0.0).toDouble());
	setPolarSnapIncAngle(settingStore->value("drawing/polarSnapIncAngle", 30.0).toDouble());
	setOsnapOnStart(settingStore->value("drawing/osnapOnStart", false).toBool());
	setOrthoSnapOnStart(settingStore->value("drawing/orthoSnapOnStart", false).toBool());
	setPolarSnapOnStart(settingStore->value("drawing/polarSnapOnStart", false).toBool());
	setPolarTrackOnStart(settingStore->value("drawing/polarTrackOnStart", false).toBool());
	setGridSnapOnStart(settingStore->value("drawing/gridSnapOnStart", false).toBool());
}

void GLESettings::writeAll()
{
	// Store the application settings
	settingStore->setValue("application/position", position());
	settingStore->setValue("application/size", size());
	settingStore->setValue("application/mainstate", mainWindowState());
	settingStore->setValue("application/drawingsize", drawingAreaSize());
	settingStore->setValue("application/storeSize", storeSize());
	settingStore->setValue("application/storeDirectory", storeDirectory());
	settingStore->setValue("application/saveOnPreview", saveOnPreview());
	settingStore->setValue("application/autoScaleOnOpen", autoScaleOnOpen());
	settingStore->setValue("application/libGSLocation", getLibGSLocation());
	settingStore->setValue("application/editorLocation", editorLocation());
	settingStore->setValue("application/resolution", dpi());
	settingStore->setValue("application/monitorOpenFile", monitorOpenFile());
	settingStore->setValue("application/monitorAutoReload", monitorAutoReloadFile());
	settingStore->setValue("application/askAboutKeepingObjects", askAboutObjects());
	settingStore->setValue("application/splitterSizes", splitterPosition());
	settingStore->setValue("application/consoleAutoShowSize", getConsoleWindowAutoShowSize());
	settingStore->setValue("application/emulateGLEVersion", getEmulateGLEVersion());
	settingStore->setValue("application/exportFormat", getExportFormat());
	settingStore->setValue("application/exportPageSize", getExportPageSize());
	settingStore->setValue("application/previewPageSize", getPreviewPageSize());
	settingStore->setValue("application/openExportedFigure", isOpenExportedFigure());
	settingStore->setValue("application/exportGrayScale", isExportGrayScale());
	settingStore->setValue("application/exportTransparent", isExportTransparent());
	settingStore->setValue("application/exportBitmapResolution", getExportBitmapResolution());
	settingStore->setValue("application/exportVectorResolution", getExportVectorResolution());
	settingStore->setValue("application/renderUsingCairo", isRenderUsingCairo());

	if (storeDirectory())
		settingStore->setValue("application/workingDirectory", pwd());

	// Store the server settings
	settingStore->setValue("server/portNumber", port());
	settingStore->setValue("server/autoStart", autoStartServer());

	// Store the drawing settings
	settingStore->setValue("drawing/gridX", grid().x());
	settingStore->setValue("drawing/gridY", grid().y());
	settingStore->setValue("drawing/equalGrid", equalGrid());

	settingStore->setValue("drawing/polarSnapStartAngle", polarSnapStartAngle());
	settingStore->setValue("drawing/polarSnapIncAngle", polarSnapIncAngle());
	settingStore->setValue("drawing/osnapOnStart", osnapOnStart());
	settingStore->setValue("drawing/orthoSnapOnStart", orthoSnapOnStart());
	settingStore->setValue("drawing/polarSnapOnStart", polarSnapOnStart());
	settingStore->setValue("drawing/polarTrackOnStart", polarSnapOnStart());
	settingStore->setValue("drawing/gridSnapOnStart", gridSnapOnStart());
}

