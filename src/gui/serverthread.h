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

#ifndef __GLESERVERTHREAD_H
#define __GLESERVERTHREAD_H

#include <QtGui>
#include <QtNetwork>
#include "qgle_definitions.h"
#include "../gle/gle-interface/gle-interface.h"
#include "fileinfo.h"
#include "qgs.h"
//#include "psfunctions.h"

class GLERectangle;
class GLEDrawingObject;

//! TCP Server Thread definition
/*!
 * This class contains the code to run a TCP server
 * and process the result of connections to that server.
 * It also performs the calls to GS and should send a QImage
 * to the drawingArea (via a queued signal) to make the on-screen
 * image update
 */
class GLEServerThread : public QThread
{
	Q_OBJECT

public:
	//! Constructor
	/*! As a thread, it should have a parent.  The calling function
	 * can specify a port to listen on
	 */
	GLEServerThread(QObject *parent, quint16 port = DEFAULT_PORT);
	~GLEServerThread();

	//! This starts the thread: run()
	void initialise();

	//! Send a message back to GLE
	void sendMessageToGLEMinusP(const QString& msg, bool last);

signals:
	//! This signal keeps the user notified of what's going on.
	void serverMessage(QString msg);
	//! This signal notifies QGLE that a new file has been opended using "gle -p"
	void gleMinusPRunned(QString file);

protected:
	//! This is the main processing part of the thread.
	/*!
	 * Starts a TCPServer and when the server receives a new connection it
	 * gets the data from GLE
	 */
	void run();

private:
	//! Set true to quit
	bool abort;
	//! A mutex for variable protection
	QMutex mutex;
	//! The port to listen on
	quint16 port;
	QString message;
	QWaitCondition messageWaitCondition;
};

class GLERenderThread : public QThread
{
	Q_OBJECT
public:
	GLERenderThread(QObject *parent);
	~GLERenderThread();

	enum {
		ToDoRenderEPSFromFile,
		ToDoRenderEPSFromMemory,
		ToDoRenderPDFFromMemory,
		ToDoRenderGLE
	};

	//! Render GLE script
	void renderGLEToImage(GLEScript* script, const QString& outFile, double dpi, const QSize& area);

	//! Convert an EPS file to an image (dpi == 0 -> autoscale)
	void renderOutputFromMemoryToImage(GLEScript* script, const QString& epsFile, double dpi, const QSize& area);
	void renderEPSFromFileToImage(const QString& fname, double dpi, const QSize& area);

	//! Return GhostScript's output stream
	QString& getGhostScriptOutput();

	//! Render arbitrary Postscript data
	void startRender(const GLERectangle& rect, double dpi);
	void nextRender(const char* postscriptCode);
	void endRender(QImage* result);

signals:
	//! This signal keeps the user notified of what's going on.
	void serverMessage(QString msg);
	//! This signal notifies the user when an error occurs.
	void serverError(QString msg);
	//! This signal notifies the world that the dpi has changed
	void dpiChanged(double new_dpi);
	//! This signal notifies QGLE that internal image processing is complete
	void renderComplete(QImage image);

protected:
	//! This is the main processing part of the thread.
	void run();

	//! Convert an EPS file to an image (dpi == 0 -> autoscale)
	void renderEPSToImageInternalFromFile(const QString& fname, double dpi, const QSize& area);

	void renderEPSToImageInternalFromMemory(GLEScript* script, double dpi, const QSize& area);

	void renderPDFToImageInternalFromMemory(GLEScript* script, double dpi, const QSize& area);

	GSInterpreterLib* setupInterpreter(const QSize& area, double b1, double b2, double width, double height, double& dpi, bool& newDPI);
	void renderEPSDone(GSInterpreterLib* interpreter, double dpi, bool newDPI);

protected:
	//! Set true to quit
	bool abort, restart;
	//! A mutex for variable protection
	QMutex mutex;
	QWaitCondition condition;
	//! What should I do?
	int todo;
	//! Render EPS arguments
	QString outFileName;
	double outFileDPI;
	QSize outFileArea;
	//! Render GLE arguments
	GLEScript* gleScript;
	//! Streams from GhostScript
	QString GhostScriptOutput;
	GSInterpreterLib *interpreter;
};

#endif
