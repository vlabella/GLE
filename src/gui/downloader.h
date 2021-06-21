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

#ifndef QGLEDOWNLOADER_H
#define QGLEDOWNLOADER_H

#include <QDialog>
#include <QList>

class QFile;
class QHttp;
class QHttpResponseHeader;
class QLabel;
class QLineEdit;
class QProgressDialog;
class QPushButton;
class QHBoxLayout;
class QVBoxLayout;
class QListWidget;
class QBuffer;

class SimpleWizard : public QDialog
{
    Q_OBJECT

public:
    SimpleWizard(QWidget *parent = 0);

    void setButtonEnabled(bool enable);

protected:
    virtual QWidget *createPage(int index) = 0;
    void setNumPages(int n);

private slots:
    void backButtonClicked();
    void nextButtonClicked();

private:
    void switchPage(QWidget *oldPage);

    QList<QWidget *> history;
    int numPages;
    QPushButton *cancelButton;
    QPushButton *backButton;
    QPushButton *nextButton;
    QPushButton *finishButton;
    QHBoxLayout *buttonLayout;
    QVBoxLayout *mainLayout;
};

class QGLEDownloader;

class QGLEDownloaderMirror : public QWidget
{
    Q_OBJECT

public:
    QGLEDownloaderMirror(QGLEDownloader *wizard);

private slots:
	void httpRequestFinished(int requestId, bool error);
	void readResponseHeader(const QHttpResponseHeader &responseHeader);

private:
	QLineEdit* downloadName;
	QListWidget* mirrorList;
	QHttp *http;
	QBuffer *file;
	int httpGetId;

    friend class QGLEDownloader;
};

class QGLEDownloaderDownload : public QWidget
{
    Q_OBJECT

public:
    QGLEDownloaderDownload(QGLEDownloader *wizard);

    friend class QGLEDownloader;
};

class QGLEDownloaderInstall : public QWidget
{
    Q_OBJECT

public:
    QGLEDownloaderInstall(QGLEDownloader *wizard);

    friend class QGLEDownloader;
};

class QGLEDownloader : public SimpleWizard
{
	Q_OBJECT

public:
	QGLEDownloader(QWidget *parent = 0);

public:
	QWidget *createPage(int index);

private slots:
	void accept();
	void downloadFile();
	void cancelDownload();
	void httpRequestFinished(int requestId, bool error);
	void readResponseHeader(const QHttpResponseHeader &responseHeader);
	void updateDataReadProgress(int bytesRead, int totalBytes);
//	void enableDownloadButton();

private:
	QLabel *statusLabel;
	QLabel *urlLabel;
	QLineEdit *urlLineEdit;
	QProgressDialog *progressDialog;
	QPushButton *quitButton;
	QPushButton *downloadButton;

	QHttp *http;
	QFile *file;
	int httpGetId;
	bool httpRequestAborted;

	QGLEDownloaderMirror *firstPage;
	QGLEDownloaderDownload *secondPage;
	QGLEDownloaderInstall *thirdPage;

	friend class QGLEDownloaderMirror;
	friend class QGLEDownloaderDownload;
	friend class QGLEDownloaderInstall;
};

#endif
