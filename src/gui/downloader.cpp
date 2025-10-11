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
#include <QtNetwork>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QProgressDialog>
#include <QMessageBox>
#include <QLabel>
#include <QProgressBar>
#include <QListWidget>
#include <QNetworkAccessManager>
#include <QRegularExpression>

#include "downloader.h"

SimpleWizard::SimpleWizard(QWidget *parent)
        : QDialog(parent)
{
    cancelButton = new QPushButton(tr("Cancel"));
    backButton = new QPushButton(tr("< &Back"));
    nextButton = new QPushButton(tr("Next >"));
    finishButton = new QPushButton(tr("&Finish"));

    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect(backButton, SIGNAL(clicked()), this, SLOT(backButtonClicked()));
    connect(nextButton, SIGNAL(clicked()), this, SLOT(nextButtonClicked()));
//    connect(finishButton, SIGNAL(clicked()), this, SLOT(accept()));

    buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(backButton);
    buttonLayout->addWidget(nextButton);
    buttonLayout->addWidget(finishButton);

    mainLayout = new QVBoxLayout;
    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);
}

void SimpleWizard::setButtonEnabled(bool enable)
{
    if (history.size() == numPages)
	finishButton->setEnabled(enable);
    else
	nextButton->setEnabled(enable);
}

void SimpleWizard::setNumPages(int n)
{
    numPages = n;
    history.append(createPage(0));
    switchPage(0);
}

void SimpleWizard::backButtonClicked()
{
    nextButton->setEnabled(true);
    finishButton->setEnabled(true);

    QWidget *oldPage = history.takeLast();
    switchPage(oldPage);
    delete oldPage;
}

void SimpleWizard::nextButtonClicked()
{
    nextButton->setEnabled(true);
    finishButton->setEnabled(history.size() == numPages - 1);

    QWidget *oldPage = history.last();
    history.append(createPage(history.size()));
    switchPage(oldPage);
}

void SimpleWizard::switchPage(QWidget *oldPage)
{
    if (oldPage) {
	oldPage->hide();
	mainLayout->removeWidget(oldPage);
    }

    QWidget *newPage = history.last();
    mainLayout->insertWidget(0, newPage);
    newPage->show();
    newPage->setFocus();

    backButton->setEnabled(history.size() != 1);
    if (history.size() == numPages) {
	nextButton->setEnabled(false);
	finishButton->setDefault(true);
    } else {
	nextButton->setDefault(true);
	finishButton->setEnabled(false);
    }

    setWindowTitle(tr("Simple Wizard - Step %1 of %2")
		   .arg(history.size())
		   .arg(numPages));
}

QGLEDownloader::QGLEDownloader(QWidget *parent)
	: SimpleWizard(parent)
{
	setNumPages(3);
/*
	urlLineEdit = new QLineEdit("http://www.ietf.org/iesg/1rfc_index.txt");

	urlLabel = new QLabel(tr("&URL:"));
	urlLabel->setBuddy(urlLineEdit);
	statusLabel = new QLabel(tr("Please enter the URL of a file you want to "
				"download."));

	quitButton = new QPushButton(tr("Quit"));
	downloadButton = new QPushButton(tr("Download"));
	downloadButton->setDefault(true);

	progressDialog = new QProgressDialog(this);

	http = new QHttp(this);

	connect(urlLineEdit, SIGNAL(textChanged(const QString &)),
		this, SLOT(enableDownloadButton()));
	connect(http, SIGNAL(requestFinished(int, bool)),
		this, SLOT(httpRequestFinished(int, bool)));
	connect(http, SIGNAL(dataReadProgress(int, int)),
		this, SLOT(updateDataReadProgress(int, int)));
	connect(http, SIGNAL(responseHeaderReceived(const QHttpResponseHeader &)),
		this, SLOT(readResponseHeader(const QHttpResponseHeader &)));
	connect(progressDialog, SIGNAL(canceled()), this, SLOT(cancelDownload()));
	connect(downloadButton, SIGNAL(clicked()), this, SLOT(downloadFile()));
	connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));

	QHBoxLayout *topLayout = new QHBoxLayout;
	topLayout->addWidget(urlLabel);
	topLayout->addWidget(urlLineEdit);

	QHBoxLayout *buttonLayout = new QHBoxLayout;
	buttonLayout->addStretch(1);
	buttonLayout->addWidget(downloadButton);
	buttonLayout->addWidget(quitButton);

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addLayout(topLayout);
	mainLayout->addWidget(statusLabel);
	mainLayout->addLayout(buttonLayout);
	setLayout(mainLayout);

	setWindowTitle(tr("HTTP"));
	urlLineEdit->setFocus();
*/
}

QWidget *QGLEDownloader::createPage(int index)
{
    switch (index) {
    case 0:
	firstPage = new QGLEDownloaderMirror(this);
	return firstPage;
    case 1:
	secondPage = new QGLEDownloaderDownload(this);
	return secondPage;
    case 2:
	thirdPage = new QGLEDownloaderInstall(this);
	return thirdPage;
    }
    return 0;
}

void QGLEDownloader::downloadFile()
{
	QUrl url(urlLineEdit->text());
	QFileInfo fileInfo(url.path());
	QString fileName = fileInfo.fileName();

	if (QFile::exists(fileName)) {
	QMessageBox::information(this, tr("HTTP"),
				 tr("There already exists a file called %1 in "
					"the current directory.")
				 .arg(fileName));
	return;
	}

	file = new QFile(fileName);
	if (!file->open(QIODevice::WriteOnly)) {
	QMessageBox::information(this, tr("HTTP"),
				 tr("Unable to save the file %1: %2.")
				 .arg(fileName).arg(file->errorString()));
	delete file;
	file = 0;
	return;
	}

	http->setHost(url.host(), url.port() != -1 ? url.port() : 80);
	if (!url.userName().isEmpty())
	http->setUser(url.userName(), url.password());

	httpRequestAborted = false;
	httpGetId = http->get(url.path(), file);

	progressDialog->setWindowTitle(tr("HTTP"));
	progressDialog->setLabelText(tr("Downloading %1.").arg(fileName));
	downloadButton->setEnabled(false);
}

void QGLEDownloader::cancelDownload()
{
	statusLabel->setText(tr("Download canceled."));
	httpRequestAborted = true;
	http->abort();
	downloadButton->setEnabled(true);
}

void QGLEDownloader::httpRequestFinished(int requestId, bool error)
{
	if (httpRequestAborted) {
	if (file) {
		file->close();
		file->remove();
		delete file;
		file = 0;
	}

	progressDialog->hide();
	return;
	}

	if (requestId != httpGetId)
	return;

	progressDialog->hide();
	file->close();

	if (error) {
	file->remove();
	QMessageBox::information(this, tr("HTTP"),
				 tr("Download failed: %1.")
				 .arg(http->errorString()));
	} else {
	QString fileName = QFileInfo(QUrl(urlLineEdit->text()).path()).fileName();
	statusLabel->setText(tr("Downloaded %1 to current directory.").arg(fileName));
	}

	downloadButton->setEnabled(true);
	delete file;
	file = 0;
}

void QGLEDownloader::readResponseHeader(const QHttpResponseHeader &responseHeader)
{
	if (responseHeader.statusCode() != 200) {
	QMessageBox::information(this, tr("HTTP"),
				 tr("Download failed: %1.")
				 .arg(responseHeader.reasonPhrase()));
	httpRequestAborted = true;
	progressDialog->hide();
	http->abort();
	return;
	}
}

void QGLEDownloader::updateDataReadProgress(int bytesRead, int totalBytes)
{
	if (httpRequestAborted)
	return;

	progressDialog->setMaximum(totalBytes);
	progressDialog->setValue(bytesRead);
}

void QGLEDownloader::accept() {
}

QGLEDownloaderMirror::QGLEDownloaderMirror(QGLEDownloader *wizard) : QWidget(wizard) {
	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(new QLabel(tr("Download")));
	layout->addWidget(downloadName = new QLineEdit());
	downloadName->setReadOnly(true);
	layout->addWidget(new QLabel(tr("Select Mirror")));
	layout->addWidget(mirrorList = new QListWidget());
	setLayout(layout);

	// old qt 4
	#ifdef QT4
	http = new QHttp(this);
	file = new QBuffer(this);
	file->open(QBuffer::ReadWrite);

	connect(http, SIGNAL(requestFinished(int, bool)), this, SLOT(httpRequestFinished(int, bool)));
	connect(http, SIGNAL(responseHeaderReceived(const QHttpResponseHeader &)), this, SLOT(readResponseHeader(const QHttpResponseHeader &)));

	mirrorList->addItem(tr("Downloading mirror list"));
	mirrorList->setEnabled(false);

	QUrl url(tr("http://prdownloads.sourceforge.net/glx/gle_4.0.11_src.zip?download"));
	http->setHost(url.host(), url.port() != -1 ? url.port() : 80);
	if (!url.userName().isEmpty()) {
		http->setUser(url.userName(), url.password());
	}
	#endif
	// new qt 5
	QNetworkAccessManager *manager = new QNetworkAccessManager(this);
	connect(manager, &QNetworkAccessManager::finished, this, SLOT(httpRequestFinished(int, bool)));
	mirrorList->addItem(tr("Downloading mirror list"));
	mirrorList->setEnabled(false);
	manager->get(QNetworkRequest(QUrl(tr("http://prdownloads.sourceforge.net/glx/gle_4.0.11_src.zip?download"))));

	httpGetId = manager->get(url.path(), file);
}


void QGLEDownloaderMirror::httpRequestFinished(int requestId, bool error)
{
	if (requestId != httpGetId) {
		return;
	}
	if (error) {
		QMessageBox::information(this, tr("Mirror List"),
		                         tr("Failed to download mirror list: %1.").arg(http->errorString()));
	} else {
		/*
	<td><a href="http://www.heanet.ie"><img alt="heanet logo" border="0" src="http://images.sourceforge.net/images/prdownloads/heanet_100x34.gif" /></a></td>
		<td>Dublin, Ireland</td>
		<td>Europe</td>
		<td><a href="/glx/gle_4.0.11_src.zip?use_mirror=heanet"><b>Download</b></a></td>
	</tr> <tr >
		*/
		mirrorList->clear();
		// old qt5
		// QRegExp rxurl(".*\\<a href\\=\"http\\:\\/\\/([^\"\\/]+).*");
		// QRegExp rxname(".*\\<td\\>([^\\<]+)\\<\\/td\\>.*");
		// QRegExp rxmirr(".*\\?use_mirror\\=([^\"]+)\".*");
		// QString line, item, url, toadd;
		// file->reset();
		// while(!file->atEnd())
		// {
		// 	QByteArray data = file->readLine();
		// 	QString line = QString::fromUtf8(data.constData());
		// 	if (rxmirr.exactMatch(line)) {
		// 		toadd = url;
		// 		if (item != "") {
		// 			toadd += " ("; toadd += item; toadd += ")";
		// 		}
		// 		mirrorList->addItem(toadd);
		// 	} else if (rxurl.exactMatch(line)) {
		// 		url = rxurl.capturedTexts()[1];
		// 		item = "";
		// 	} else if (rxname.exactMatch(line)) {
		// 		if (item != "") item += ", ";
		// 		item += rxname.capturedTexts()[1];
		// 	}
		// }

		QRegularExpression rxurl(".*<a href=\"http://([^\"/]+).*");
		QRegularExpression rxname(".*<td>([^<]+)</td>.*");
		QRegularExpression rxmirr(".*\\?use_mirror=([^\"]+)\".*");

		QString line, item, url, toadd;
		file->reset();

		while (!file->atEnd()) {
		    QByteArray data = file->readLine();
		    QString line = QString::fromUtf8(data.constData());

		    QRegularExpressionMatch matchMirr = rxmirr.match(line);
		    QRegularExpressionMatch matchUrl = rxurl.match(line);
		    QRegularExpressionMatch matchName = rxname.match(line);

		    if (matchMirr.hasMatch()) {
		        toadd = url;
		        if (!item.isEmpty()) {
		            toadd += " (" + item + ")";
		        }
		        mirrorList->addItem(toadd);
		    } else if (matchUrl.hasMatch()) {
		        url = matchUrl.captured(1);
		        item.clear();
		    } else if (matchName.hasMatch()) {
		        if (!item.isEmpty()) item += ", ";
		        item += matchName.captured(1);
		    }
		}
		file->close();
		mirrorList->setEnabled(true);
	}
}

void QGLEDownloaderMirror::readResponseHeader(const QHttpResponseHeader &responseHeader)
{
	if (responseHeader.statusCode() != 200) {
		QMessageBox::information(this, tr("Mirror List"),
		                         tr("Failed to download mirror list: %1.").arg(responseHeader.reasonPhrase()));
		http->abort();
	}
}

QGLEDownloaderDownload::QGLEDownloaderDownload(QGLEDownloader *wizard) : QWidget(wizard) {
	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(new QLabel(tr("Downloading")));
	layout->addWidget(new QProgressBar());
	layout->addWidget(new QLabel(tr("Select Installation Directory")));
	layout->addStretch(1);
	setLayout(layout);
}

QGLEDownloaderInstall::QGLEDownloaderInstall(QGLEDownloader *wizard) : QWidget(wizard) {
	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(new QLabel(tr("Installing")));
	layout->addWidget(new QProgressBar());
	layout->addWidget(new QListWidget());
	setLayout(layout);
}

