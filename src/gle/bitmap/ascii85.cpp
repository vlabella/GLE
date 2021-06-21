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
 * ASCII85 Encoding Support.
 *
 */

#include "../basicconf.h"
#include "../file_io.h"
#include "img2ps.h"

#define PS_MAXLINE 36

static char* Ascii85Encode(unsigned char* raw) {
	static char encoded[6];
	unsigned int word = (((raw[0]<<8)+raw[1])<<16) + (raw[2]<<8) + raw[3];
	if (word != 0L) {
		unsigned int q;
		unsigned int w1;

		q = word / (85L*85*85*85);	/* actually only a byte */
		encoded[0] = q + '!';

		word -= q * (85L*85*85*85); q = word / (85L*85*85);
		encoded[1] = q + '!';

		word -= q * (85L*85*85); q = word / (85*85);
		encoded[2] = q + '!';

		w1 = (unsigned int) ((word - q*(85L*85)) & 0xFFFF);
		encoded[3] = (w1 / 85) + '!';
		encoded[4] = (w1 % 85) + '!';
		encoded[5] = '\0';
	} else {
		encoded[0] = 'z', encoded[1] = '\0';
	}
	return encoded;
}

GLEASCII85ByteStream::GLEASCII85ByteStream(ostream* file) {
	m_File = file;
	m_BreakLength = 2*PS_MAXLINE;
	m_Count = 0;
}

int GLEASCII85ByteStream::sendByte(unsigned char code) {
	m_Buffer[m_Count++] = code;
	if (m_Count >= 4) {
		int n;
		unsigned char* p;
		for (n = m_Count, p = m_Buffer; n >= 4; n -= 4, p += 4) {
			char* cp;
			for (cp = Ascii85Encode(p); *cp; cp++) {
				m_File->put(*cp);
				if (--m_BreakLength == 0) {
					m_File->put('\n');
					m_BreakLength = 2*PS_MAXLINE;
				}
			}
		}
		for (int i = 0; i < n; i++) {
			m_Buffer[i] = p[i];
		}
		m_Count = n;
	}
	return GLE_IMAGE_ERROR_NONE;
}

int GLEASCII85ByteStream::term() {
	if (!isTerminated()) {
		if (m_Count > 0) {
			char* res;
			for (int i = 0; i < 3; i++) {
				m_Buffer[m_Count+i] = 0;
			}
			res = Ascii85Encode(m_Buffer);
			m_File->write(res[0] == 'z' ? "!!!!" : res, m_Count + 1);
		}
		*m_File << "~>" << endl;
	}
	return GLEByteStream::term();
}

GLEASCII85ByteStream::~GLEASCII85ByteStream() {
}
