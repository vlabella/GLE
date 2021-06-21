/************************************************************************
 *                                                                      *
 * GLE - Graphics Layout Engine <http://www.gle-graphics.org/>          *
 *                                                                      *
 * Modified BSD License                                                 *
 *                                                                      *
 * Copyright (C) 2009 GLE.                                              *
 *                                                                      *
 * Redistribution and use in source and binary forms, with or without   *
 * modification, are permitted provided that the following conditions   *
 * are met:                                                             *
 *                                                                      *
 *    1. Redistributions of source code must retain the above copyright *
 * notice, this list of conditions and the following disclaimer.        *
 *                                                                      *
 *    2. Redistributions in binary form must reproduce the above        *
 * copyright notice, this list of conditions and the following          *
 * disclaimer in the documentation and/or other materials provided with *
 * the distribution.                                                    *
 *                                                                      *
 *    3. The name of the author may not be used to endorse or promote   *
 * products derived from this software without specific prior written   *
 * permission.                                                          *
 *                                                                      *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR   *
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED       *
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE   *
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY       *
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL   *
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE    *
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS        *
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER *
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR      *
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN  *
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                        *
 *                                                                      *
 ************************************************************************/

/*
 * 2004 Jan Struyf
 *
 */

#include "../basicconf.h"
#include "../file_io.h"
#include "img2ps.h"

/*
 * JPEG
 */

#define M_SOF0  0xC0
#define M_SOF1  0xC1
#define M_SOF2  0xC2
#define M_SOF3  0xC3
#define M_SOF5  0xC5
#define M_SOF6  0xC6
#define M_SOF7  0xC7
#define M_SOF9  0xC9
#define M_SOF10 0xCA
#define M_SOF11 0xCB
#define M_SOF13 0xCD
#define M_SOF14 0xCE
#define M_SOF15 0xCF
#define M_SOI   0xD8
#define M_EOI   0xD9
#define M_SOS   0xDA
#define M_EXIF  0xE1
#define M_COM   0xFE
#define M_TEM   0x01
#define M_RST0  0xD0
#define M_RST7  0xD7

GLEJPEG::GLEJPEG() : GLEFileBitmap() {
	m_Encoding = GLE_BITMAP_JPEG;
}

GLEJPEG::~GLEJPEG() {
}

int GLEJPEG::readImageSize() {
	m_BitsPerComponent = m_file.fgetc();
	m_Height = read16BE();
	m_Width = read16BE();
	m_Components = m_file.fgetc();
	if (m_Components == 1) {
		setMode(GLE_BITMAP_GRAYSCALE);
	} else {
		setMode(GLE_BITMAP_RGB);
	}
	return GLE_IMAGE_ERROR_NONE;
}


int GLEJPEG::checkJPG() {
	if (m_BitsPerComponent != 8) {
		stringstream str;
		str << "unsupported number of bits/component: " << m_BitsPerComponent << " <> 8";
		setError(str.str());
		return GLE_IMAGE_ERROR_DATA;
	}
	if (m_Components != 1 && m_Components != 3 && m_Components != 4) {
		stringstream str;
		str << "unsupported number of components: " << m_BitsPerComponent << " (should be 1, 3, or 4)";
		setError(str.str());
		return GLE_IMAGE_ERROR_DATA;
	}
	return GLE_IMAGE_ERROR_NONE;
}

int GLEJPEG::readHeader() {
	char markstr[20];
	while (!m_file.feof()) {
		/* Skip padding at start of section */
		int marker = 0;
		int nbpadding = 0;
		while (true) {
			marker = m_file.fgetc();
			if (m_file.feof()) {
				setError("SOF marker not found");
				return GLE_IMAGE_ERROR_DATA;
			}
			if (marker != 0xff) {
				break;
			} else {
				nbpadding++;
			}
		}
		if (nbpadding == 0) {
			sprintf(markstr, "0x%X", marker);
			setError(string("no 0xFF before marker: ") +markstr);
			return GLE_IMAGE_ERROR_DATA;
		}
		/* Just before section */
		if (marker == M_SOI || marker == M_EOI || marker == M_TEM ||
		    (marker >= M_RST0 && marker <= M_RST7)) {
			continue;
		}
		/* Start of scan found */
		if (marker == M_SOS) {
			setError("start of scan comes before SOF marker");
			return GLE_IMAGE_ERROR_DATA;
		}
		/* Read section length */
		int crpos = m_file.ftell();
		int size = read16BE();
		if (size < 2) {
			sprintf(markstr, "0x%X", marker);
			setError(string("size error for block with marker: ") +markstr);
			return GLE_IMAGE_ERROR_DATA;
		}
		/* Size info? */
		switch (marker) {
			case M_SOF0:
			case M_SOF1:
			case M_SOF2:
			case M_SOF3:
			case M_SOF5:
			case M_SOF6:
			case M_SOF7:
			case M_SOF9:
			case M_SOF10:
			case M_SOF11:
			case M_SOF13:
			case M_SOF14:
			case M_SOF15:
 				readImageSize();
				return checkJPG();
		}
		/* Skip till after section */
		fseek(m_file.getFile(), crpos+size, SEEK_SET);
	}
	setError("SOF marker not found");
	return GLE_IMAGE_ERROR_DATA;
}

int GLEJPEG::coded(GLEByteStream* output) {
	fseek(m_file.getFile(), 0, SEEK_SET);
	while (true) {
		int nextByte = m_file.fgetc();
		if (nextByte == EOF) {
			break;
		} else {
			output->sendByte((GLEBYTE)nextByte);
		}
	}
	return GLE_IMAGE_ERROR_NONE;
}
