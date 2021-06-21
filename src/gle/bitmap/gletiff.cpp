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

#ifdef HAVE_LIBTIFF

#include "tiffio.h"
#include "img2ps.h"

/*
 * TIFF
 */

GLETIFF::GLETIFF() : GLEBitmap() {
}

GLETIFF::~GLETIFF() {
}

int GLETIFF::open(const string& fname) {
	m_fname = fname;
	/* leave as "r" for WINDOWS VL */
	m_Tiff = TIFFOpen(fname.c_str(), "r");
	if (m_Tiff == NULL) {
		return 0;
	}
	return 1;
}

int GLETIFF::readHeader() {
	uint16 samplesperpixel, bitspersample, planarconfiguration;
	uint16 photometric, extrasamples, *sampleinfo;
	/* Get fields */
	TIFFGetField(m_Tiff, TIFFTAG_IMAGEWIDTH, &m_Width);
	TIFFGetField(m_Tiff, TIFFTAG_IMAGELENGTH, &m_Height);
	TIFFGetFieldDefaulted(m_Tiff, TIFFTAG_BITSPERSAMPLE, &bitspersample);
	TIFFGetFieldDefaulted(m_Tiff, TIFFTAG_SAMPLESPERPIXEL, &samplesperpixel);
	TIFFGetFieldDefaulted(m_Tiff, TIFFTAG_PLANARCONFIG, &planarconfiguration);
	TIFFGetField(m_Tiff, TIFFTAG_COMPRESSION, &m_TIFFCompress);
	TIFFGetFieldDefaulted(m_Tiff, TIFFTAG_EXTRASAMPLES, &extrasamples, &sampleinfo);
	/* Set fields */
	setComponents(samplesperpixel);
	setBitsPerComponent(bitspersample);
	/* Detect alpha channel */
	if (extrasamples == 1) {
		if (sampleinfo[0] == EXTRASAMPLE_ASSOCALPHA || sampleinfo[0] == EXTRASAMPLE_UNSPECIFIED) {
			setAlpha(1);
		}
		setExtraComponents(1);
	} else if (extrasamples != 0) {
		printf("\nTIFF: Unsupported number of extra samples: %d\n", extrasamples);
	}
	/* Detect photometric */
	if (!TIFFGetField(m_Tiff, TIFFTAG_PHOTOMETRIC, &photometric)) {
		switch (samplesperpixel - extrasamples) {
			case 1:
				if (isCCITTCompression()) photometric = PHOTOMETRIC_MINISWHITE;
				else photometric = PHOTOMETRIC_MINISBLACK;
				break;
			case 3:
				photometric = PHOTOMETRIC_RGB;
				break;
		}
	}
	/* Detect mode based on photometric */
	switch (photometric) {
		case PHOTOMETRIC_MINISWHITE:
		case PHOTOMETRIC_MINISBLACK:
			/* Should invert if MINISWHITE? */
			setMode(GLE_BITMAP_GRAYSCALE);
			break;
		case PHOTOMETRIC_RGB:
			setMode(GLE_BITMAP_RGB);
			break;
		case PHOTOMETRIC_PALETTE:
			setMode(GLE_BITMAP_INDEXED);
			setNbColors(1 << bitspersample);
			break;
/*
		case PHOTOMETRIC_MASK:
		case PHOTOMETRIC_SEPARATED:
		case PHOTOMETRIC_YCBCR:
		case PHOTOMETRIC_CIELAB:
		case PHOTOMETRIC_ICCLAB:
		case PHOTOMETRIC_ITULAB:
		case PHOTOMETRIC_LOGL:
		case PHOTOMETRIC_LOGLUV:
*/
		default:
			printf("\nTIFF: Unsupported photometric: %d\n", photometric);
			return GLE_IMAGE_ERROR_NOT_IMPL;
	}
	/* Determine organization */
	int tiled_image = TIFFIsTiled(m_Tiff);
	if (tiled_image) {
		printf("\nTIFF: Tiled images not yet supported\n");
		return GLE_IMAGE_ERROR_NOT_IMPL;
	}
	/* Determine planar configuration */
	if (planarconfiguration != PLANARCONFIG_CONTIG) {
		printf("\nTIFF: Only planar images supported\n");
		return GLE_IMAGE_ERROR_NOT_IMPL;
	}
	return GLE_IMAGE_ERROR_NONE;
}

int GLETIFF::prepare(int mode) {
	/* Load palette */
	if (isIndexed()) {
		uint16 *rmap, *gmap, *bmap;
		if (!TIFFGetField(m_Tiff, TIFFTAG_COLORMAP, &rmap, &gmap, &bmap)) {
			printf("\nTIFF: Indexed image without palette\n");
			return GLE_IMAGE_ERROR_DATA;
		}
		/* Check if palette is 8 or 16 bit */
		int is8bit = 1;
		for (int i = 0; i < getNbColors(); i++) {
			if (rmap[i] >= 256 || gmap[i] >= 256 || bmap[i] >= 256) {
				is8bit = 0;
			}
		}
		/* Convert palette */
		rgb* pal = allocPalette(getNbColors());
		for (int i = 0; i < getNbColors(); i++) {
			if (is8bit == 1) {
				pal[i].red = rmap[i];
				pal[i].green = gmap[i];
				pal[i].blue = bmap[i];
			} else {
				pal[i].red = CVT16TO8(rmap[i]);
				pal[i].green = CVT16TO8(gmap[i]);
				pal[i].blue = CVT16TO8(bmap[i]);
			}
		}
	}
	return GLE_IMAGE_ERROR_NONE;
}

int GLETIFF::decode(GLEByteStream* output) {
	int size = TIFFScanlineSize(m_Tiff);
	GLEBYTE* scanline = (GLEBYTE*)_TIFFmalloc(size);
	for (int i = 0; i < getHeight(); i++) {
		TIFFReadScanline(m_Tiff, scanline, i);
		output->send(scanline, size);
		output->endScanLine();
	}
	_TIFFfree(scanline);
	return GLE_IMAGE_ERROR_NONE;
}

void GLETIFF::close() {
	if (m_Tiff != NULL) {
		TIFFClose(m_Tiff);
		m_Tiff = NULL;
	}
}

int GLETIFF::isCCITTCompression() {
	return (m_TIFFCompress == COMPRESSION_CCITTFAX3 ||
		m_TIFFCompress == COMPRESSION_CCITTFAX4 ||
		m_TIFFCompress == COMPRESSION_CCITTRLE ||
		m_TIFFCompress == COMPRESSION_CCITTRLEW);
}

string GLETIFF::getFName()
{
	return m_fname;
}

#endif

