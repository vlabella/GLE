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

#ifndef _DIALOGUES_H
#define _DIALOGUES_H

#include <QDialog>
#include <QTabWidget>
#include "mainwindow.h"
#include "consolewindow.h"

class EvaluatorDialog : public QDialog
{
	Q_OBJECT
public:
	EvaluatorDialog(GLEMainWindow *parent);
private slots:
	void eval();
	void clearResults();
private:
	GLEMainWindow *mainWin;
	ConsoleWindow* results;
	QLineEdit* field;
};

//! Dialogue box used to export to different formats
class ExportDialogue : public QDialog
{
	Q_OBJECT
public:
	ExportDialogue(GLEMainWindow *parent);

private slots:
	void exportFormatChanged(int idx);
	void exportFormatChangedImpl(int idx, bool updateAll);
	int entryIDtoDevice(int id);
	void resolutionChanged(int value);
	void exportClicked();
	void exportAsClicked();

private:
	void updateResolution();
	void performExport(const QString& file);
	void addFormat(const QString& name, int gleFormatID);
	bool isBitmap();
	int device;
	QList<int> fmtToDevice;
	GLEMainWindow *mainWin;
	QListWidget* list;
	QCheckBox* transp;
	QCheckBox* grayScale;
	QCheckBox* openResult;
	QLineEdit* fname;
	QSpinBox* resolution;
	QLabel* resolutionLabel;
	QLabel* sizeLabel;
	QComboBox* format;
	bool previousWasBitmap;
};

class SoftwareLocateDialogue;

class SoftwareLocateThread : public QThread {
	Q_OBJECT
protected:
	SoftwareLocateDialogue* parentWindow;
	QString rootDir;
public:
	SoftwareLocateThread(SoftwareLocateDialogue* parent, const QString& root);
	~SoftwareLocateThread();
	void run();
signals:
	void threadDone();
};

class SoftwareLocateOutput : public QObject, public GLEOutputStream {
	Q_OBJECT
public:
	SoftwareLocateOutput();
	virtual ~SoftwareLocateOutput();
	virtual void println(const char* str);
	virtual void printflush(const char* str);
signals:
	void print(const QString str);
};

#define QGLE_SOFT_DIALOG_SEARCH_MANUAL 1
#define QGLE_SOFT_DIALOG_SEARCH_ABORT  2
#define QGLE_SOFT_DIALOG_SEARCH_CANCEL 4

//! Dialogue box used to locate the GhostScript library
class SoftwareLocateDialogue : public QDialog
{
	Q_OBJECT
public:
	SoftwareLocateDialogue(GLEMainWindow *parent, GLEInterface* iface, int buttons);
	~SoftwareLocateDialogue();
	void showGhostScriptError(QString error, QString loc);
	void disableAll();
	void enableOK(bool enable);
	inline GLEMainWindow* getMainWindow() { return mainWin; }
	inline bool isCancel() { return cancel; }
	inline bool hasFound() { return found; }
	inline bool isConfigModified() { return modified; }
	inline GLEInterface* getGLEInterface() { return gleInterface; }
	inline void setRootDir(const QString& dir) { rootDir = dir; }
private slots:
	void locateManual();
public slots:
	void searchAuto();
	void cancelClicked();
	void setRootClicked();
	void print(const QString str);
	void threadDone();
private:
	void tryGhostScriptLocation(QString loc);
	bool found, modified;
	GLEInterface* gleInterface;
	QPushButton* rootButton;
	QPushButton* searchButton;
	QPushButton* locButton;
	QPushButton* abortButton;
	QPushButton* okButton;
	QTextBrowser* browser;
	SoftwareLocateOutput output;
	GLEMainWindow *mainWin;
	QString rootDir;
	bool cancel;
};

class CrashRecoverDialogue : public QDialog
{
	Q_OBJECT
public:
	CrashRecoverDialogue(const QString& file);
	QString createReport();
	void saveEmail();
	void foundFile(QStringList* list, const QDir& dir, const QString& file);
	void performRequest(QString url, QString script);
public slots:
	void viewReport();
	void sendReport();
	void cancelReport();
	void requestFinished(int id, bool error);
protected:
	int requestID, closeID;
//	QHttp* http;
	QBuffer *buffer;
	QFileInfo gleFileName;
	QString crashLog;
    QStringList crashLogLines;
	QTextEdit* description;
	QLineEdit* email;
	QCheckBox* includeScript;
	QPushButton* sendButton;
	QPushButton* cancelButton;
};

#endif
