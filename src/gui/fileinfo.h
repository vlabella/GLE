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

#ifndef _GLE_FILE_INFO_H
#define _GLE_FILE_INFO_H

#include <QtGui>

class GLEFileData
{
public:
	QString fname;
	QDateTime lastModified;
};

//! Class used to describe the open file
class GLEFileInfo
{
private:
	//! The GLE file name
	QString gleFileName;
	//! The EPS file name
	QString epsFileName;
	//! The rendered image
	QImage image;
	//! The previous DPI value
	double lastDPI;
	//! The time stamp
	QDateTime lastModified;
	//! Flag noting whether we have a file name
	bool hasFileNameFlag;
	//! Additional files to monitor
	QList<GLEFileData> filesToMonitor;

public:
	//! Constructor: initialise variables
	GLEFileInfo();

	//! Return the GLE file name
	inline const QString& gleFile() { return gleFileName; }
	//! Set the GLE file name
	inline void setGleFile(const QString& newGLEFile) { gleFileName = newGLEFile; setHasFileName(true);}
	//! Clear the GLE file name
	inline void clearGleFile() { gleFileName = ""; }
	//! Return the EPS file name
	inline const QString& epsFile() { return epsFileName; }
	//! Set the EPS file name
	inline void setEpsFile(const QString& newEPSFile) { epsFileName = newEPSFile; }
	//! Get the rendered base image
	inline const QImage& getImage() { return image; }
	//! Set the rendered base image
	void setImage(const QImage& newImage);
	//! What was the previous DPI value
	inline double getLastDPI() { return lastDPI; }
	//! Set whether this is a temporary file
	inline void setLastDPI(double dpi) { lastDPI = dpi; }
	//! Set whether we have a file name
	inline void setHasFileName(const bool& newFlag) { hasFileNameFlag = newFlag; }
	//! Do we have a file name?
	inline const bool& hasFileName() { return hasFileNameFlag; }
	//! Update the time stamp
	void updateTimeStamp();
	//! Has the file changed externally?
	bool hasChanged();
	//! Is the primary file a GLE file?
	bool isGLE();
	//! Is the primary file an EPS file?
	bool isEPS();
	//! What's the file name of the primary file?
	QString primaryFile();
	//! Clear the object
	void clear();
	//! Copy relevant information from other GLEFileInfo object
	void copyFrom(GLEFileInfo* other);
	void clearFilesToMonitor();
	void addFileToMonitor(const QString& str);
};

#endif
