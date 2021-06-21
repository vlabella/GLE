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

#include <Carbon/Carbon.h>

#include <string>
#include <fstream>

using namespace std;

bool copyPDFMac(const string& fname) {
	CFMutableDataRef data = CFDataCreateMutable(kCFAllocatorDefault, 0);
    if (data != NULL) {
		char buffer[10000];
		ifstream file;
		file.open(fname.c_str());
		if (file.is_open()) {
			while (file.good()) {
				file.read(buffer, 9000);
				CFDataAppendBytes(data, (const UInt8*)buffer, file.gcount());
			}
			file.close();
			// copy PDF to clipboard
			PasteboardRef clipboard = NULL;
			PasteboardCreate(kPasteboardClipboard, &clipboard);
			PasteboardClear(clipboard);
			PasteboardPutItemFlavor(clipboard, (PasteboardItemID) 1, kUTTypePDF, data, kPasteboardFlavorNoFlags);
			CFRelease(clipboard);
			CFRelease(data);
			return true;
		}
	}
	return false;
}
