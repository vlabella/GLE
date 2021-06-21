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

#ifdef HAVE_LIBPNG

#include <png.h>


GLEPNG::GLEPNG() {
	m_PNGPtr = NULL;
	m_InfoPtr = NULL;
	m_EndInfo = NULL;
}

GLEPNG::~GLEPNG() {
	png_destroy_read_struct(&m_PNGPtr, &m_InfoPtr, &m_EndInfo);
}

#define GLEPNG_SIG_SIZE 8

int GLEPNG::readHeader() {
	GLEBYTE sig[GLEPNG_SIG_SIZE];
	m_file.fread(sig, 1, GLEPNG_SIG_SIZE);
	if (png_sig_cmp(sig, 0, GLEPNG_SIG_SIZE)) {
		setError("invalid PNG file");
		return GLE_IMAGE_ERROR_TYPE;
	}
	m_PNGPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, (png_voidp)NULL, NULL, NULL);
	if (!m_PNGPtr) {
		return GLE_IMAGE_ERROR_INTERN;
	}
	m_InfoPtr = png_create_info_struct(m_PNGPtr);
	if (!m_InfoPtr) {
		png_destroy_read_struct(&m_PNGPtr, (png_infopp)NULL, (png_infopp)NULL);
		return GLE_IMAGE_ERROR_INTERN;
	}
	m_EndInfo = png_create_info_struct(m_PNGPtr);
	if (!m_EndInfo) {
		png_destroy_read_struct(&m_PNGPtr, &m_InfoPtr, (png_infopp)NULL);
		return GLE_IMAGE_ERROR_INTERN;
	}
	png_init_io(m_PNGPtr, m_file.getFile());
	png_set_sig_bytes(m_PNGPtr, GLEPNG_SIG_SIZE);
	png_read_info(m_PNGPtr, m_InfoPtr);
	// Get most important parts from header
	m_Width = png_get_image_width(m_PNGPtr, m_InfoPtr);
	m_Height = png_get_image_height(m_PNGPtr, m_InfoPtr);
	m_BitsPerComponent = png_get_bit_depth(m_PNGPtr, m_InfoPtr);
	// Check if interlaced
	int interlace_type = png_get_interlace_type(m_PNGPtr, m_InfoPtr);
	if (interlace_type != PNG_INTERLACE_NONE) {
		setError("interlaced PNGs not yet supported");
		return GLE_IMAGE_ERROR_DATA;
	}
	return GLE_IMAGE_ERROR_NONE;
}

int GLEPNG::prepare(int mode) {
	int color_type = png_get_color_type(m_PNGPtr, m_InfoPtr);
	if (color_type == PNG_COLOR_TYPE_PALETTE) {
		/* Palette image */
		int num_palette;
		png_colorp palette;
		png_get_PLTE(m_PNGPtr, m_InfoPtr, &palette, &num_palette);
		rgb* pal = allocPalette(num_palette);
		for (int i = 0; i < num_palette; i++) {
			pal[i].red = palette[i].red;
			pal[i].green = palette[i].green;
			pal[i].blue = palette[i].blue;
		}
		setNbColors(num_palette);
		setMode(GLE_BITMAP_INDEXED);
		setComponents(1);
		if (m_BitsPerComponent < 8) {
			png_set_packing(m_PNGPtr);
		}
		checkGrayScalePalette();
	} else if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
		/* Grayscale image */
		setMode(GLE_BITMAP_GRAYSCALE);
		setComponents(1);
	} else {
		/* RGB image */
		setMode(GLE_BITMAP_RGB);
		setComponents(3);
	}
	if (color_type & PNG_COLOR_MASK_ALPHA) {
		setAlpha(true);
		setComponents(getComponents() + 1);
		setExtraComponents(1);
	}
	return GLE_IMAGE_ERROR_NONE;
}

int GLEPNG::decode(GLEByteStream* output) {
	int size = getScanlineSize();
	GLEBYTE* scanline = new GLEBYTE[size];
	for (int i = 0; i < getHeight(); i++) {
		png_read_row(m_PNGPtr, scanline, NULL);
		output->send(scanline, size);
		output->endScanLine();
	}
	delete[] scanline;
	png_read_end(m_PNGPtr, m_EndInfo);
	return GLE_IMAGE_ERROR_NONE;
}

#endif
