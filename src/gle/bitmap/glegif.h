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

#ifndef INC_GIF_H
#define INC_GIF_H

class GIFHEADER {
protected:
	char sig[3];              // "GIF"
	char ver[3];              // "87a" or "89a"
public:
	int isvalid(void);
	int get(FILE *f);
};

#define scdGCT       0x80    // global color table
#define scdGCTCRES   0x70    // color res : nbits-1
#define scdGCTSORT   0x08    // sort flag
#define scdGCTSIZE   0x07    // color cnt as 2^(sss+1)

class GIFSCDESC {                 // Screen Descriptor
protected:
	unsigned short scwidth;   // width in pixels
	unsigned short scheight;  // height in pixels
	unsigned char  flags;	  // various flags
	unsigned char  bgclr;	  // background color
	unsigned char  pixasp;    // pixel aspect ratio
public:
	GIFSCDESC();
	int get(GLEGIF *gif);
	inline int isgct() { return (flags & scdGCT) ? 1 : 0; }
	inline int issorted() { return (flags & scdGCTSORT) ? 1 : 0; }
// bits per pixel
	inline int depth() { return ((flags & scdGCTCRES) >> 4) + 1; }
// # entries in GCT
	inline int ncolors() { return 1 << ((flags & scdGCTSIZE) + 1); }
};

#define imdLCT       0x80    // local color table
#define imdINTRLACE  0x40    // interlace flag
#define imdLCTSORT   0x20    // sort flag
#define imdRESV      0x18    // reserved
#define imdLCTSIZE   0x07    // color cnt as 2^(sss+1)

class GIFIMDESC {                 // Image Descriptor
protected:
	unsigned char  id;	  // 0x2C
	unsigned short xleft;	  // x origin
	unsigned short ytop;	  // y origin
	unsigned short imwidth;   // image width
	unsigned short imheight;  // image height
	unsigned char  flags;	  // various flags
public:
  	int get(GLEGIF* gif);
	inline int islct() { return (flags & imdLCT) ? 1 : 0; }
	inline int issorted() { return (flags & imdLCTSORT) ? 1 : 0; }
	inline int isinterlaced() { return (flags & imdINTRLACE) ? 1 : 0; }
	inline int getWidth() { return imwidth; }
	inline int getHeight() { return imheight; }
	int ncolors();
};


#define LZW_MAXVAL  4096


class PGImageReader;

class GLEGIFDecoder {
public:
	GLEGIF* m_Gif;
	GLEByteStream* m_Output;
	int m_CrLine, m_CrPos, m_CrPass;
	GLEBYTE *m_Last, *m_TopBuffer, *m_Buffer, *m_ScanLine;
	GLEDWORD *m_First, m_RootCodeSize, m_CodeSize, m_Expected, m_Mask, m_Old;
public:
	GLEGIFDecoder(GLEGIF* gif, GLEByteStream* out);
	virtual ~GLEGIFDecoder();
	void clearTable();
	int deInterlace(int nbrows);
	int decode(FILE* input);
	void storeBytes(int bytes, GLEBYTE* source);
	inline int isInterlaced() { return m_Gif->isInterlaced(); }
	inline int getWidth() { return m_Gif->getWidth(); }
};

#endif
