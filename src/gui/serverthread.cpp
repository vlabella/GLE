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

#include <QtCore>
#include <QtNetwork>
#include <QtDebug>

#include "qgs.h"
#include "serverthread.h"
#include "drawingobject.h"
#include "math.h"
#define DIR_SEP "/"
#include "mainwindow.h"
#include "qgle_statics.h"
#include "../config.h"

#include <sstream>

using namespace std;

GLEServerThread::GLEServerThread(QObject *parent, quint16 port) : QThread(parent)
{
	// Just save the requested port
	this->port = port;
}

GLEServerThread::~GLEServerThread()
{
	// Use a locker to get access to shared variables,
	// set abort=true and wait for the server to die
	mutex.lock();
	abort = true;
	mutex.unlock();
	messageWaitCondition.wakeAll();
	wait();
}

void GLEServerThread::initialise()
{
	// Make sure abort is safe
	abort = false;
	// Start the processing
	start(LowPriority);
}

void GLEServerThread::run()
{
	// This is the bit that runs in a different thread

	// Set up some handy regexps
	QRegExp glefile(".*glefile: *\"([^\"]*)\".*");
	QRegExp rxdpi(".*dpi: *\"([^\"]*)\".*");
	QString rxdone = tr("*DONE*");

	// Create a new server and listen on the requested port
	QTcpServer server;
	if (!server.listen(QHostAddress::LocalHost, port))
	{
		emit serverMessage(tr("Unable to start the server"));
		return;
	}

	emit serverMessage(tr("Server listening"));

	// Loop forever
	forever
	{
		// Check whether we should quit
		mutex.lock();
		if (abort)
		{
			mutex.unlock();
			return;
		}
		mutex.unlock();


		// Wait for a new connection for 500ms
		if (server.waitForNewConnection(500))
		{
			// Get the connection
			QTcpSocket *clientConnection = server.nextPendingConnection();

			qint64 bytesread;
			char buffer[2000];
			QString gleCommand;

			// While we are connected
			bool done = false;
			while(!done && clientConnection->state() == QAbstractSocket::ConnectedState)
			{
				// Check whether we should quit
				mutex.lock();
				if (abort)
				{
					mutex.unlock();
					delete clientConnection;
					return;
				}
				mutex.unlock();

				// See whether there's anything to read
				if(clientConnection->waitForReadyRead(500))
				{
					// There is, so find out how much
					bytesread = clientConnection->bytesAvailable();
					// and read it
					clientConnection->read(buffer, bytesread);
					buffer[bytesread] = 0;
					// append it to our gleCommand buffer
					gleCommand += buffer;
				}

				if(gleCommand.indexOf(rxdone) != -1)
				{
					done = true;
				}
			}

			// Break it up into separate lines
			QStringList parts = gleCommand.split(QRegExp("[\\r\\n]+"));
			QString part, glefname;

			// Find the bits we're interested in
			foreach(part, parts)
			{
				if(rxdpi.exactMatch(part))
				{
					// do something with dpi?
				}
				if(glefile.exactMatch(part))
				{
					glefname = glefile.capturedTexts()[1];
				}
			}

			// Now make QGLE render this file
			emit gleMinusPRunned(glefname);

			// Send console output back to gle
			mutex.lock();
			done = false;
			while (!done)
			{
				if (abort)
				{
					mutex.unlock();
					delete clientConnection;
					return;
				}
				if (message.length() != 0)
				{
					done = true;
					clientConnection->write(message.toUtf8());
					clientConnection->waitForBytesWritten(-1);
					message = "";
				}
				else
				{
					messageWaitCondition.wait(&mutex);
				}
			}
			mutex.unlock();

			// Delete the connection and loop round to wait for a new one
			delete(clientConnection);
		}
	}
}

//! Send a message back to GLE
void GLEServerThread::sendMessageToGLEMinusP(const QString& msg, bool)
{
	mutex.lock();
	// Added space because empty message means no message yet in above code
	message = msg + " ";
	mutex.unlock();
	messageWaitCondition.wakeAll();
}

GLERenderThread::GLERenderThread(QObject *parent) : QThread(parent)
{
	abort = false;
	restart = false;
	interpreter = NULL;
}

GLERenderThread::~GLERenderThread()
{
	 mutex.lock();
	abort = true;
	condition.wakeOne();
	mutex.unlock();
	wait();
}

//! Render GLE script
void GLERenderThread::renderGLEToImage(GLEScript* script, const QString& outFile, double dpi, const QSize& area)
{
	QMutexLocker locker(&mutex);
	todo = ToDoRenderGLE;
	gleScript = script;
	outFileName = outFile;
	outFileDPI = dpi;
	outFileArea = area;
	if (!isRunning()) {
		start();
	} else {
		restart = true;
		condition.wakeOne();
	}
}

void GLERenderThread::renderOutputFromMemoryToImage(GLEScript* script, const QString& epsFile, double dpi, const QSize& area)
{
	QMutexLocker locker(&mutex);
#ifdef HAVE_POPPLER
	string* bytesPDF = script->getRecordedBytesBuffer(GLE_DEVICE_PDF);
	string* bytesEPS = script->getRecordedBytesBuffer(GLE_DEVICE_EPS);
	if (!bytesPDF->empty()) {
		todo = ToDoRenderPDFFromMemory;
	} else if (!bytesEPS->empty()) {
		todo = ToDoRenderEPSFromMemory;
	} else {
		QImage emptyImage;
		emit renderComplete(emptyImage);
		return;
	}
#else
	todo = ToDoRenderEPSFromMemory;
#endif
	gleScript = script;
	outFileName = epsFile;
	outFileDPI = dpi;
	outFileArea = area;
	if (!isRunning()) {
		start();
	} else {
		restart = true;
		condition.wakeOne();
	}
}

//! Convert an EPS file to an image (dpi == 0 -> autoscale)
void GLERenderThread::renderEPSFromFileToImage(const QString& fname, double dpi, const QSize& area)
{
	QMutexLocker locker(&mutex);
	todo = ToDoRenderEPSFromFile;
	outFileName = fname;
	outFileDPI = dpi;
	outFileArea = area;
	if (!isRunning()) {
		start();
	} else {
		restart = true;
		condition.wakeOne();
	}
}

void GLERenderThread::run()
{
	qRegisterMetaType<QImage>("QImage");
	forever
	{
		mutex.lock();
		if (abort)
		{
			mutex.unlock();
			return;
		}
		int myToDo = todo;
		QString myOutFileName = outFileName;
		double myOutFileDPI = outFileDPI;
		QSize myOutFileArea = outFileArea;
		GLERC<GLEScript> myGLEScript;
		if (myToDo == ToDoRenderGLE || myToDo == ToDoRenderEPSFromMemory || myToDo == ToDoRenderPDFFromMemory) {
			// script is NULL if render EPS
			myGLEScript = gleScript;
		}
		mutex.unlock();

		// do processing
		if (myToDo == ToDoRenderGLE) {
			emit serverMessage(tr("Rendering GLE file"));
			GLEInterface* iface = myGLEScript->getGLEInterface();
			if (myOutFileDPI != 0.0) {
				std::ostringstream resolutionString;
				resolutionString << (int)floor(myOutFileDPI + 0.5);
				iface->setCmdLineOptionString("resolution", resolutionString.str().c_str());
			}
			bool renderPoppler = false;
			int device = GLE_DEVICE_EPS;
			#ifdef HAVE_POPPLER
				if (iface->hasCmdLineOptionString("cairo")) {
					renderPoppler = true;
					device = GLE_DEVICE_PDF;
				}
			#endif
			iface->renderGLE(myGLEScript.get(), myOutFileName.toLatin1().constData(), device, true);
			if (iface->getOutput()->getExitCode() == 0) {
				if (renderPoppler) {
					myToDo = ToDoRenderPDFFromMemory;
				} else {
					myToDo = ToDoRenderEPSFromMemory;
				}
			} else {
				QImage emptyImage;
				emit renderComplete(emptyImage);
			}
		}
		if (myToDo == ToDoRenderEPSFromMemory) {
			renderEPSToImageInternalFromMemory(myGLEScript.get(), myOutFileDPI, myOutFileArea);
		}
		if (myToDo == ToDoRenderPDFFromMemory) {
			renderPDFToImageInternalFromMemory(myGLEScript.get(), myOutFileDPI, myOutFileArea);
		}
		if (myToDo == ToDoRenderEPSFromFile) {
			renderEPSToImageInternalFromFile(myOutFileName, myOutFileDPI, myOutFileArea);
		}
		mutex.lock();
		if (!restart)
		{
			 condition.wait(&mutex);
		}
		restart = false;
		mutex.unlock();
	}
}

QString& GLERenderThread::getGhostScriptOutput()
{
	return GhostScriptOutput;
}

GSInterpreterLib* GLERenderThread::setupInterpreter(const QSize& area, double b1, double b2, double width, double height, double& dpi, bool& newDPI) {
	// QString gsError = QString("%1 %2 wd %3 %4 dpi %5").arg(b1).arg(b2).arg(width).arg(height).arg(dpi);
	// qDebug() << "*** Origin: " << gsError;

	// Auto scale new files to size of drawing area
	if (dpi == 0.0) {
		dpi = QGLE::computeAutoScaleDPIFromPts(area, 5, width, height);
		newDPI = true;
	}
	// Create the interpreter object and set a few parameters
	GSInterpreterLib *interpreter = new GSInterpreterLib();
	interpreter->setOrigin(b1, b2);
	interpreter->setMedia("a4");
	interpreter->setMagnify(1);
	interpreter->setOrientation(0);
	interpreter->setDPI(dpi);
	int img_wd = GLEBBoxToPixels(dpi, width);
	int img_hi = GLEBBoxToPixels(dpi, height);
	interpreter->setSize(img_wd, img_hi);
	interpreter->setAABits(4,4);
	return interpreter;
}

void GLERenderThread::renderEPSDone(GSInterpreterLib* interpreter, double dpi, bool newDPI) {
	QString gsError;
	GhostScriptOutput = interpreter->getOutpuStreamText();
	if (interpreter->hasError()) {
		const GSError& e = interpreter->getError();
		gsError = QString("Ghostscript error: %1 %2").arg(e.getCode()).arg(e.getName());
		qDebug() << "Error: " << gsError;
	}

	QImage image(interpreter->getImage());
	delete interpreter;

	// Delete interpreter should go before different signals
	// otherwise recursive calls to this function may cause multiple instantiations of
	// libgs, which is not allowed and results in crashes
	if (gsError != "") {
		emit serverError(gsError);
	}
	if (newDPI) {
		emit dpiChanged(dpi);
	}
	emit renderComplete(image);
}

//! Convert an EPS file to an image (dpi == 0 -> autoscale)
void GLERenderThread::renderEPSToImageInternalFromFile(const QString& fname, double dpi, const QSize& area)
{
	// Check that the file can be opened
	QFile file(fname);
	if (!file.open( QIODevice::ReadOnly ))
	{
		QImage emptyImage;
		emit renderComplete(emptyImage);
		return;
	}

	// Rendering EPS has started
	emit serverMessage(tr("Rendering PostScript"));

	// Scan through file for BoundingBox and set width and height
	double b1 = 0.0;
	double b2 = 0.0;
	double b3 = 0.0;
	double b4 = 0.0;
	double width = 0.0;
	double height = 0.0;

	QRegExp rxbb("^%%BoundingBox:\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)\\s*$");
	QRegExp rxbbh("^%%HiResBoundingBox:\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)\\s*$");

	QTextStream in(&file);
	while (!in.atEnd())
	{
		QString line = in.readLine();
		if (rxbb.exactMatch(line))
		{
			b1 = (double)rxbb.capturedTexts()[1].toInt();
			b2 = (double)rxbb.capturedTexts()[2].toInt();
			b3 = (double)rxbb.capturedTexts()[3].toInt();
			b4 = (double)rxbb.capturedTexts()[4].toInt();
			width = b3-b1+1;
			height = b4-b2+1;
			if (!in.atEnd())
			{
				// GLE writes a HiResBoundingBox right after the BoundingBox
				line = in.readLine();
				if (rxbbh.exactMatch(line))
				{
					b1 = rxbb.capturedTexts()[1].toDouble();
					b2 = rxbb.capturedTexts()[2].toDouble();
					b3 = rxbb.capturedTexts()[3].toDouble();
					b4 = rxbb.capturedTexts()[4].toDouble();
					width = b3-b1;
					height = b4-b2;
				}
			}
			break;
		}
	}

	bool newDPI = false;
	// this class has this defined as a member so no need to redfine - causes error
	GSInterpreterLib* interpreter = setupInterpreter(area, b1, b2, width, height, dpi, newDPI);

	//interpreter = setupInterpreter(area, b1, b2, width, height, dpi, newDPI);
	// Run the interpreter
	file.reset();
	interpreter->run(file);
	file.close();

	renderEPSDone(interpreter, dpi, newDPI);
}

void GLERenderThread::renderEPSToImageInternalFromMemory(GLEScript* script, double dpi, const QSize& area) {
	// Rendering EPS has started
	emit serverMessage(tr("Rendering PostScript"));

	// Render EPS from memory
	GLEPoint bb(script->getBoundingBox());
	GLEPoint origin(script->getBoundingBoxOrigin());
	bool newDPI = false;
	GSInterpreterLib* interpreter = setupInterpreter(area, origin.getX(), origin.getY(), bb.getX(), bb.getY(), dpi, newDPI);
	string* bytes = script->getRecordedBytesBuffer(GLE_DEVICE_EPS);
	interpreter->run(bytes);
	renderEPSDone(interpreter, dpi, newDPI);
}

void gle_write_data_vector(void* closure, char* data, int length) {
	std::vector<unsigned char>* vec = (std::vector<unsigned char>*)closure;
	vec->reserve(vec->size() + length);
	for (int i = 0; i < length; ++i) {
		vec->push_back((unsigned char)data[i]);
	}
}

#ifdef HAVE_POPPLER
void GLERenderThread::renderPDFToImageInternalFromMemory(GLEScript* script, double dpi, const QSize& area) {
	// Auto scale new files to size of drawing area
	bool newDPI = false;
	if (dpi == 0.0) {
		GLEPoint bb(script->getBoundingBox());
		dpi = QGLE::computeAutoScaleDPIFromPts(area, 5, bb.getX(), bb.getY());
		newDPI = true;
	}
	string* bytes = script->getRecordedBytesBuffer(GLE_DEVICE_PDF);
	std::vector<unsigned char> imageData;
	GLEInterface* iface = script->getGLEInterface();
	iface->convertPDFToImage((char*)bytes->c_str(),
			                 bytes->size(),
			                 dpi,
			                 GLE_DEVICE_PNG,
			                 0,
			                 gle_write_data_vector,
			                 &imageData);
	QImage image;
	image.loadFromData(&imageData[0], imageData.size(), "PNG");
	if (newDPI) {
		emit dpiChanged(dpi);
	}
	emit renderComplete(image);
}
#else
void GLERenderThread::renderPDFToImageInternalFromMemory(GLEScript* /* script */, double /* dpi */, const QSize& /* area */) {
}
#endif

void GLERenderThread::startRender(const GLERectangle& rect, double dpi)
{
	// Create the interpreter object and set a few parameters
	interpreter = new GSInterpreterLib();
	interpreter->setOrigin(0, 0);
	interpreter->setMedia("a4");
	interpreter->setMagnify(1);
	interpreter->setOrientation(0);
	interpreter->setDPI(dpi);
	interpreter->setAlpha(true);
	double width  = 72*rect.getWidth()/CM_PER_INCH+2;
	double height = 72*rect.getHeight()/CM_PER_INCH+2;
	int img_wd = GLEBBoxToPixels(dpi, width);
	int img_hi = GLEBBoxToPixels(dpi, height);
	interpreter->setSize(img_wd, img_hi);
	interpreter->setAABits(4,4);
	// Run the interpreter
	interpreter->startRender();
}

void GLERenderThread::nextRender(const char* postscriptCode)
{
	interpreter->nextRender(postscriptCode);
}

void GLERenderThread::endRender(QImage* result)
{
	// End render and copy image
	interpreter->endRender();
	*result = interpreter->getImage();
	// Copy Ghostscript's output stream
	QString gsError;
	GhostScriptOutput = interpreter->getOutpuStreamText();
	if (interpreter->hasError()) {
		const GSError& e = interpreter->getError();
		gsError = QString("Ghostscript error: %1 %2").arg(e.getCode()).arg(e.getName());
		qDebug() << "Error: " << gsError;
	}
	// Clean up
	delete interpreter;
	interpreter = NULL;
	// Emit signals
	if (gsError != "")
	{
		emit serverError(gsError);
	}
}
