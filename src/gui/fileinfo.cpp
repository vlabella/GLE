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

#include "fileinfo.h"
#include "qgle_statics.h"

/**********************************************************************
 * A class that holds various bits of information about the open file *
 **********************************************************************/

// Clear the file info object on creation
GLEFileInfo::GLEFileInfo()
{
	clear();
}

void GLEFileInfo::setImage(const QImage& newImage)
{
	image = newImage;
}

// Clear the fileinfo object by setting everything to null
void GLEFileInfo::clear()
{
	gleFileName = "";
	epsFileName = "";
	image = QImage();
	lastDPI = -1;
	hasFileNameFlag = false;
}

// Copy relevant information from other GLEFileInfo object
void GLEFileInfo::copyFrom(GLEFileInfo* other)
{
	// *DO NOT* copy image and .eps file name!
	gleFileName = other->gleFileName;
	hasFileNameFlag = other->hasFileNameFlag;
	lastModified = other->lastModified;
}

// Read the time stamp from the file and save it.
void GLEFileInfo::updateTimeStamp()
{
	QFileInfo fi;
	QString mainFile = primaryFile();
	if (mainFile.isEmpty())
	{
		lastModified = QDateTime();
		return;
	}

	fi.setFile(mainFile);

	lastModified = fi.lastModified();
}

// Return the filename of the main file (i.e. the GLE
// file if we have one and the EPS file otherwise)
QString GLEFileInfo::primaryFile()
{
	if (isGLE())
		return(gleFileName);
	else if (isEPS())
		return(epsFileName);
	else
		return QString();
}

// Is it a GLE file?
bool GLEFileInfo::isGLE()
{
	return(!gleFileName.isEmpty());
}

// Is it an EPS file?
bool GLEFileInfo::isEPS()
{
	if(isGLE())
		return false;

	return(!epsFileName.isEmpty());
}

// Check whether the file has changed on disk
// by looking at the time stamp
bool GLEFileInfo::hasChanged()
{
	QFileInfo fi;
	QString mainFile(primaryFile());
	if (mainFile.isEmpty())	{
		return false;
	}
	fi.setFile(mainFile);
	if (!fi.isReadable()) {
		return false;
	}
	QDateTime modifiedTime(fi.lastModified());
	if (modifiedTime > lastModified) {
		return true;
	} else {
		for (int i = 0; i < filesToMonitor.size(); ++i) {
			GLEFileData data(filesToMonitor.at(i));
			fi.setFile(data.fname);
			if (fi.isReadable() && fi.lastModified() > data.lastModified) {
				return true;
			}
		}
		return false;
	}
}

void GLEFileInfo::clearFilesToMonitor()
{
	filesToMonitor.clear();
}

void GLEFileInfo::addFileToMonitor(const QString& str)
{
	QFileInfo fi;
	GLEFileData data;
	fi.setFile(str);
	data.fname = str;
	data.lastModified = fi.lastModified();
	filesToMonitor.append(data);
}
