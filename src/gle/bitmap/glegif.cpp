/************************************************************************
 *                                                                      *
 * GLE - Graphics Layout Engine <http://glx.sourceforge.net/>          *
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
#include "glegif.h"
#include <cstring>

/*
 * GIF
 */

GLEGIF::GLEGIF () : GLEFileBitmap() {
	m_ImageOffs = 0;
}

GLEGIF::~GLEGIF () {
}

int GLEGIF::readHeader() {
	GIFHEADER hdr;
	if (hdr.get(m_file.getFile())) {
		return GLE_IMAGE_ERROR_INTERN;
	}
	if (! hdr.isvalid()) {
		return GLE_IMAGE_ERROR_TYPE;
	}
	GIFSCDESC scd;
	if (scd.get(this)) {
		return GLE_IMAGE_ERROR_INTERN;
	}
	rgb* pal = allocPalette(256);
	if (scd.isgct()) {
		// Skip global color table - should remember position?
		m_Colors = scd.ncolors();
		for (int i = 0; i < m_Colors; i++) {
			pal[i].red = m_file.fgetc();
			pal[i].green = m_file.fgetc();
			pal[i].blue = m_file.fgetc();
		}
	}
	int type;
	while ((type = m_file.fgetc()) > 0) {
		// printf("GIF Block Ox%X\n", type);
		if (type == 0x2C) {
			// image descriptor
			headerImage();
			break;
		} else if (type == 0x21) {
			// extension
			headerExtension();
		} else if (type == 0x3B) {
			// trailer
			return GLE_IMAGE_ERROR_DATA;
		} else {
			return GLE_IMAGE_ERROR_DATA;
		}
	}
	return GLE_IMAGE_ERROR_NONE;
}

int GLEGIF::headerImage() {
	GIFIMDESC imd;
	if (imd.get(this) == 0) {
		return GLE_IMAGE_ERROR_DATA;
	}
	if (imd.islct()) {
		rgb* pal = getPalette();
		m_Colors = imd.ncolors();
		for (int i = 0; i < m_Colors; i++) {
			pal[i].red = m_file.fgetc();
			pal[i].green = m_file.fgetc();
			pal[i].blue = m_file.fgetc();
		}
	}
	m_ImageOffs = ftell(m_file.getFile());
	updateImageType();
	m_Width = imd.getWidth();
	m_Height = imd.getHeight();
	return GLE_IMAGE_ERROR_NONE;
}

void GLEGIF::updateImageType() {
	if (getNbColors() == 0) {
		setMode(GLE_BITMAP_GRAYSCALE);
	} else {
		setMode(GLE_BITMAP_INDEXED);
		setBitsPerComponent(getMaxBits());
		checkGrayScalePalette();
	}
}

int GLEGIF::headerExtension() {
	int label = m_file.fgetc();
	switch (label) {
		case 0xFE : // comment extension
			headerCOMExt();
			break;
		case 0xF9 : // control extension
		case 0x01 : // text extension
		case 0xFF : // application extension
			skipBlocks();
			break;
		default :   // unknown or invalid
			return 0;
	}
	return 1;
}

void GLEGIF::skipBlocks() {
	int nbytes;
	while ((nbytes = m_file.fgetc()) > 0) {
		fseek(m_file.getFile(), nbytes, SEEK_CUR);
	}
}

void GLEGIF::headerCOMExt() {
	skipBlocks();
/*	int nbytes;
	while ((nbytes = fgetc(m_In)) > 0) {
		char* comment = new char[nbytes+1];
		fread(comment, nbytes, 1, m_In);
		comment[nbytes] = 0;
		printf("Comment: '%s'\n", comment);
		delete comment;
	}*/
}

int GLEGIF::decode(GLEByteStream* out) {
	fseek(m_file.getFile(), m_ImageOffs, SEEK_SET);
	GLEGIFDecoder decoder(this, out);
	int result = decoder.decode(m_file.getFile());
	return result;
}

int GIFHEADER::get(FILE *f) {
	return (fread(sig, 6, 1, f) == 1) ? 0 : -1;
}

int GIFHEADER::isvalid(void) {
	if (strncmp( sig, "GIF", 3 ) != 0 ) {
		return 0;
	}
	if ((strncmp( ver, "87a", 3 ) != 0) && (strncmp( ver, "89a", 3 ) != 0)) {
		return 0;
	}
	return 1;
}

GIFSCDESC::GIFSCDESC() {
	scwidth = scheight = 0;
	flags = bgclr = pixasp = 0;
}

int GIFSCDESC::get(GLEGIF *gif) {
	scwidth = gif->read16LE();
	scheight = gif->read16LE();
	flags = gif->read8();
	bgclr = gif->read8();
	pixasp = gif->read8();
	return 0;
}

int GIFIMDESC::ncolors() {
	if (islct()) {
		return 1 << ((flags & imdLCTSIZE) + 1);
	} else {
		return 0;
	}
}

int GIFIMDESC::get(GLEGIF* gif) {
	xleft = gif->read16LE();     // x origin
	ytop = gif->read16LE();	     // y origin
	imwidth = gif->read16LE();   // image width
	imheight = gif->read16LE();  // image height
	flags = gif->read8();	     // various flags
	return 1;
}

GLEGIFDecoder::GLEGIFDecoder(GLEGIF* gif, GLEByteStream* out) {
	m_Gif = gif;
	m_Output = out;
	m_First = new GLEDWORD[LZW_MAXVAL];
	m_Last = new GLEBYTE[LZW_MAXVAL];
	m_Buffer = new GLEBYTE[LZW_MAXVAL];
	m_ScanLine = new GLEBYTE[gif->getWidth()];
}

GLEGIFDecoder::~GLEGIFDecoder() {
	delete[] m_First;
	delete[] m_Last;
	delete[] m_Buffer;
	delete[] m_ScanLine;
}

void GLEGIFDecoder::clearTable() {
	int maxi = 1 << m_RootCodeSize;
	m_Expected = maxi + 2;
	m_Old = LZW_MAXVAL;
	m_CodeSize = m_RootCodeSize + 1;
	m_Mask = (1 << m_CodeSize) - 1;
	for (int i = 0; i < maxi; i++) {
		m_First[i] = LZW_MAXVAL;
		m_Last[i] = i & 0xFF;
	}
	m_TopBuffer = m_Buffer;
}

int GLEGIFDecoder::deInterlace(int nrows) {
	static int gif_delta[]  = { 8, 8, 4, 2 };
	static int gif_origin[] = { 0, 4, 2, 1 };
	m_CrLine += gif_delta[m_CrPass];
	if (m_CrLine >= nrows) {
		m_CrPass++;
		m_CrLine = gif_origin[m_CrPass];
	}
	return m_CrLine;
}

void GLEGIFDecoder::storeBytes(int bytes, GLEBYTE* source) {
	int wd = (int)m_Gif->getWidth();
	int i = bytes - 1;
	while (i >= 0) {
		int j = i-wd+m_CrPos+1;
		if (j < 0) j = 0;
		while (i >= j) m_ScanLine[m_CrPos++] = source[i--];
		if (m_CrPos >= wd) {
			m_CrPos = 0;
			if (isInterlaced()) {
				// reader->put(scanLine, Deinterlace(descr->gfHeight));
				printf("HELP, can't handle interlaced gifs\n");
			} else {
				m_Output->send(m_ScanLine, wd);
				m_Output->endScanLine();
			}
		}
	}
}

int GLEGIFDecoder::decode(FILE* input) {
	int need = 0;
	GLEDWORD bits = 0, code = 0, codeFeched;
	GLEBYTE *chPos, m_FirstCodeOut = 0, charBuff[256];
	// Read data block header
	m_RootCodeSize = fgetc(input);
	GLEDWORD CLEAR = 1 << m_RootCodeSize;
	GLEDWORD EOI = CLEAR + 1;
	clearTable();
	// Read image data
	m_CrPass = m_CrPos = 0;
	m_CrLine = isInterlaced() ? -8 : 0;
	GLEDWORD buffCount = fgetc(input);
	if (buffCount == 0) return GLE_IMAGE_ERROR_DATA;
	while(buffCount > 0) {
		if ((GLEDWORD)fread(charBuff, 1, buffCount, input) != buffCount) return GLE_IMAGE_ERROR_FILE;
		for(chPos = charBuff; buffCount-- > 0; chPos++) {
			need += (int) *chPos << bits;
			bits += 8;
			while (bits >= m_CodeSize) {
				code = need & m_Mask;
				need >>= m_CodeSize;
				bits -= m_CodeSize;
				if (code > m_Expected) return GLE_IMAGE_ERROR_DATA; // Neither LZW nor RunLength
				if (code == EOI) return GLE_IMAGE_ERROR_NONE;
				if (code == CLEAR) {
					clearTable();
					continue;
				}
				if (m_Old == LZW_MAXVAL) {  // m_First code after clear table
					storeBytes(1, &m_Last[code]);
					m_FirstCodeOut = m_Last[code];
					m_Old = code;
					continue;
				}
				codeFeched = code;
				if (code == m_Expected) {
					*m_TopBuffer++ = m_FirstCodeOut;
					code = m_Old;
				}
				while (code > CLEAR) {
					*m_TopBuffer++ = m_Last[code];
					code = m_First[code];
				}
				*m_TopBuffer++ = m_FirstCodeOut = m_Last[code];
				m_First[m_Expected] = m_Old;
				m_Last[m_Expected] = m_FirstCodeOut;
				if (m_Expected < LZW_MAXVAL) m_Expected++;
				if (((m_Expected & m_Mask) == 0) && (m_Expected < LZW_MAXVAL)) {
					m_CodeSize++;
					m_Mask += m_Expected;
				}
				m_Old = codeFeched;
				storeBytes(m_TopBuffer - m_Buffer, m_Buffer);
				m_TopBuffer = m_Buffer;
			}
		}
		buffCount = fgetc(input);
		if (buffCount == 0) return GLE_IMAGE_ERROR_DATA;
	}
	return GLE_IMAGE_ERROR_NONE;
}
