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
#include "../gprint.h"
#include "../file_io.h"
#include "../cutils.h"
#include "img2ps.h"

GLEBitmap::GLEBitmap() {
	m_Height = 0;
	m_Width = 0;
	m_Mode = GLE_BITMAP_NONE;
	m_Components = 1;
	m_ExtraComponents = 0;
	m_Colors = 0;
	m_Interlaced = 0;
	m_Palette = NULL;
	m_ASCII85 = 0;
	m_Compress = 0;
	m_Alpha = 0;
	m_Encoding = GLE_BITMAP_LZW;
	m_BitsPerComponent = 8;
}

GLEBitmap::~GLEBitmap() {
	close();
	if (m_Palette != NULL) {
		delete[] m_Palette;
	}
}

int GLEBitmap::open(const string& fname) {
	return 0;
}

int GLEBitmap::readHeader() {
	return 0;
}

int GLEBitmap::toPS(ostream* fp) {
	// Init image, set transformations, possibly load palette
	prepare(GLE_BITMAP_PREPARE_SCANLINE);
	// Variables
	const char* coder = "/ASCII85Decode filter";
	const char* encoder;
	int width = m_Width;
	int height = m_Height;
	int colors = getNbColors();
	int bits = getBitsPerComponent();
	if (getEncoding() == GLE_BITMAP_LZW) {
		encoder = "/LZWDecode";
	} else {
		encoder = "/DCTDecode";
	}
	*fp << "save 9 dict begin" << endl;
	*fp << "{/T currentfile" << coder << " def" << endl;
	if (isIndexed()) {
		*fp << "[/Indexed/DeviceRGB " << (colors-1) << " T " << (colors*3) << " string readstring pop]";
	} else if (isGrayScale()) {
		*fp << "/DeviceGray";
	} else {
		*fp << "/DeviceRGB";
	}
	*fp << " setcolorspace" << endl;
	*fp << "/F T" << encoder << " filter def" << endl;
	*fp << "<</ImageType 1/Width " << width << "/Height " << height << "/BitsPerComponent " << bits << endl;
	*fp << "/ImageMatrix[" << width << " 0 0 -" << height << " 0 " << height << "]/Decode" << endl;
	*fp << "[";
	int decodemax = isIndexed() ? (1 << bits)-1 : 1;
	*fp << "0 " << decodemax;
	int comps = getColorComponents();
	for (int j = 1; j < comps; j++) {
		*fp << " 0 " << decodemax;
	}
	*fp << "]/DataSource F>> image" << endl;
	*fp << "F closefile T closefile}" << endl;
	*fp << "exec" << endl;
	GLEASCII85ByteStream ascii85(fp);
	// Store palette for indexed image
	if (isIndexed()) {
		rgb* pal = getPalette();
		for (int i = 0; i < colors; i++) {
			ascii85.sendByte(pal[i].red);
			ascii85.sendByte(pal[i].green);
			ascii85.sendByte(pal[i].blue);
		}
	}
	// Encode image
	GLEByteStream* stream = NULL;
	if (getEncoding() == GLE_BITMAP_LZW) {
		GLELZWByteStream lzw(&ascii85);
		// Remove extra components?
		int extra = getExtraComponents();
		int color_alpha = getColorComponents();
		if (isAlpha()) {
			// Do not count alpha as extra component
			extra--;
			color_alpha++;
		}
		GLEComponentRemovalByteStream crem(&lzw, color_alpha, extra);
		if (extra != 0) {
			stream = &crem;
		} else {
			stream = &lzw;
		}
		// Remove alpha?
		GLEAlphaRemovalByteStream alpha(stream, color_alpha);
		if (isAlpha()) stream = &alpha;
		// Combine more than one pixel in each byte?
		GLEPixelCombineByteStream combine(stream, bits);
		if (bits < 8) stream = &combine;
		// Decode
		decode(stream);
		stream->term();
	} else {
		coded(&ascii85);
	}
	// End
	ascii85.term();
	*fp << "end restore" << endl;
	return 0;
}

int GLEBitmap::decode(GLEByteStream* output) {
	return GLE_IMAGE_ERROR_NOT_IMPL;
}

int GLEBitmap::coded(GLEByteStream* output) {
	return GLE_IMAGE_ERROR_NOT_IMPL;
}

void GLEBitmap::loadImageData() {
}

int GLEBitmap::prepare(int mode) {
	return GLE_IMAGE_ERROR_NONE;
}

rgb* GLEBitmap::allocPalette(int ncolors) {
	if (m_Palette != NULL) {
		delete[] m_Palette;
	}
	m_Palette = new rgb[ncolors];
	return m_Palette;
}

int GLEBitmap::getScanlineSize() {
	int bytespc = getBitsPerComponent()/8;
	if (bytespc < 1) bytespc = 1;
	return getWidth()*getComponents()*bytespc;
}

int GLEBitmap::getColorComponents() {
	return getComponents() - getExtraComponents();
}

int GLEBitmap::getMaxBits() {
	if (isIndexed()) {
		if (m_Colors > 16) {
			/* More than 16 colors -> 8 bits (5,6,7 bits are not supported by postscript!) */
			return 8;
		} else if (m_Colors >= 5) {
			/* At most 16 colors */
			return 4;
		} else if (m_Colors >= 3) {
			/* At most 4 colors */
			return 2;
		} else {
			/* Monochrome bitmap */
			return 1;
		}
	} else {
		return 8;
	}
}

void GLEBitmap::checkGrayScalePalette() {
	rgb* pal = getPalette();
	if (getNbColors() == 256) {
		int nogray = 0;
		for (int i = 0; i < 256; i++) {
			if (pal[i].red != i || pal[i].green != i || pal[i].blue != i) {
				nogray = 1;
			}
		}
		if (nogray == 0) {
			setMode(GLE_BITMAP_GRAYSCALE);
			setBitsPerComponent(8);
		}
	} else if (getNbColors() == 2) {
		if (pal[0].red == 0   && pal[0].green == 0   && pal[0].blue == 0 &&
		    pal[1].red == 255 && pal[1].green == 255 && pal[1].blue == 255) {
			setMode(GLE_BITMAP_GRAYSCALE);
			setBitsPerComponent(1);
		}
	}
}

void GLEBitmap::printInfo(ostream& os) {
	os << getWidth();
	os << "x";
	os << getHeight();
	os << "x";
	os << getBitsPerComponent() * getComponents();
	switch (getMode()) {
		case GLE_BITMAP_GRAYSCALE:
			os << "-GRAY";
			break;
		case GLE_BITMAP_RGB:
			os << "-RGB";
			break;
		case GLE_BITMAP_INDEXED:
			os << "-PAL:" << getNbColors();
			break;
	}
}

void GLEBitmap::close() {
}

string GLEBitmap::getFName()
{
	return "";
}

GLEFileBitmap::GLEFileBitmap() {
	m_Palette = NULL;
}

GLEFileBitmap::~GLEFileBitmap() {
	close();
}

int GLEFileBitmap::open(const string& fname) {
	m_file.open(fname.c_str(), "rb");
	return m_file.isOpen() ? 1 : 0;
}

int GLEFileBitmap::read16BE() {
	int lh = fgetc(m_file.getFile());
	int ll = fgetc(m_file.getFile());
	return (lh << 8) | ll;
}

int GLEFileBitmap::read16LE() {
	int lh = fgetc(m_file.getFile());
	int ll = fgetc(m_file.getFile());
	return (ll << 8) | lh;
}

void GLEFileBitmap::close() {
	m_file.close();
}

string GLEFileBitmap::getFName()
{
	return m_file.getName();
}

GLEByteStream::GLEByteStream() {
	m_Terminated = 0;
}

GLEByteStream::~GLEByteStream() {
}

int GLEByteStream::send(GLEBYTE* bytes, GLEDWORD count) {
	for (unsigned int i = 0; i < count; i++) {
		sendByte(bytes[i]);
	}
	return GLE_IMAGE_ERROR_NONE;
}

int GLEByteStream::term() {
	m_Terminated = 1;
	return GLE_IMAGE_ERROR_NONE;
}

int GLEByteStream::endScanLine() {
	return GLE_IMAGE_ERROR_NONE;
}

GLERecordedByteStream::GLERecordedByteStream() {
}

GLERecordedByteStream::~GLERecordedByteStream() {
}

int GLERecordedByteStream::sendByte(GLEBYTE byte) {
	m_bytes.push_back(byte);
	return GLE_IMAGE_ERROR_NONE;
}

unsigned long GLERecordedByteStream::getNbBytes() const {
	return (unsigned long)m_bytes.size();
}

GLEBYTE* GLERecordedByteStream::getBytes() {
	if (m_bytes.empty()) {
		return 0;
	} else {
		return &m_bytes[0];
	}
}

GLEPipedByteStream::GLEPipedByteStream(GLEByteStream* pipe): GLEByteStream() {
	m_Pipe = pipe;
}

GLEPipedByteStream::~GLEPipedByteStream() {
}

int GLEPipedByteStream::endScanLine() {
	m_Pipe->endScanLine();
	return GLEByteStream::endScanLine();
}

int GLEPipedByteStream::term() {
	m_Pipe->term();
	return GLEByteStream::term();
}

GLEPNegateByteStream::GLEPNegateByteStream(GLEByteStream* pipe):
	GLEPipedByteStream(pipe)
{
}

GLEPNegateByteStream::~GLEPNegateByteStream() {
}

int GLEPNegateByteStream::sendByte(GLEBYTE byte) {
	m_Pipe->sendByte(~byte);
	return GLE_IMAGE_ERROR_NONE;
}

GLEIndexedToRGBByteStream::GLEIndexedToRGBByteStream(GLEByteStream* pipe, rgb* palette):
	GLEPipedByteStream(pipe),
	m_palette(palette)
{
}

GLEIndexedToRGBByteStream::~GLEIndexedToRGBByteStream() {
}

int GLEIndexedToRGBByteStream::sendByte(GLEBYTE byte) {
	rgb values(m_palette[byte]);
	m_Pipe->sendByte(values.red);
	m_Pipe->sendByte(values.green);
	m_Pipe->sendByte(values.blue);
	return GLE_IMAGE_ERROR_NONE;
}

GLERGBATo32BitByteStream::GLERGBATo32BitByteStream(GLEByteStream* pipe, bool isAlpha):
	GLEPipedByteStream(pipe),
	m_index(0),
	m_nbComponents(isAlpha ? 4 : 3)
{
}

GLERGBATo32BitByteStream::~GLERGBATo32BitByteStream() {
}

int GLERGBATo32BitByteStream::sendByte(GLEBYTE byte) {
	m_components[m_index++] = byte;
	if (m_index == m_nbComponents) {
		unsigned int value = ((unsigned int)m_components[0]) << 16
                             | ((unsigned int)m_components[1]) << 8
                             | ((unsigned int)m_components[2]);
		if (m_nbComponents == 4) {
			value |= ((unsigned int)m_components[3]) << 24;
		}
		const char* valueAsBytes = (const char*)&value;
		for (unsigned int i = 0; i < sizeof(unsigned int); ++i) {
			m_Pipe->sendByte(valueAsBytes[i]);
		}
		m_index = 0;
	}
	return GLE_IMAGE_ERROR_NONE;
}

int GLERGBATo32BitByteStream::endScanLine() {
	m_index = 0;
	return GLEPipedByteStream::endScanLine();
}

GLEPixelCombineByteStream::GLEPixelCombineByteStream(GLEByteStream* pipe, int bpc) : GLEPipedByteStream(pipe) {
	m_BitsPerComponent = bpc;
	m_BitsLeft = 8;
	m_Combined = 0;
}

GLEPixelCombineByteStream::~GLEPixelCombineByteStream() {
}

int GLEPixelCombineByteStream::flushBufferByte() {
	m_Pipe->sendByte(m_Combined);
	m_BitsLeft = 8;
	m_Combined = 0;
	return GLE_IMAGE_ERROR_NONE;
}

int GLEPixelCombineByteStream::sendByte(GLEBYTE byte) {
	if (m_BitsLeft >= m_BitsPerComponent) {
		/* Still fits in */
		m_Combined |= byte << (m_BitsLeft - m_BitsPerComponent);
		m_BitsLeft -= m_BitsPerComponent;
	} else {
		/* Does not fit in */
		int left = m_BitsPerComponent - m_BitsLeft;
		m_Combined |= byte >> left;
		flushBufferByte();
		m_Combined |= byte << (m_BitsLeft - left);
		m_BitsLeft -= left;
	}
	if (m_BitsLeft == 0) flushBufferByte();
	return GLE_IMAGE_ERROR_NONE;
}

int GLEPixelCombineByteStream::endScanLine() {
	if (m_BitsLeft != 8) flushBufferByte();
	return GLEPipedByteStream::endScanLine();
}

int GLEPixelCombineByteStream::term() {
	if (m_BitsLeft != 8) flushBufferByte();
	return GLEPipedByteStream::term();
}

GLEBitsTo32BitByteStream::GLEBitsTo32BitByteStream(GLEByteStream* pipe) : GLEPipedByteStream(pipe) {
	m_bitsLeft = 32;
	m_combined = 0;
}

GLEBitsTo32BitByteStream::~GLEBitsTo32BitByteStream() {
}

int GLEBitsTo32BitByteStream::flushBufferByte() {
	const char* valueAsBytes = (const char*)&m_combined;
	for (unsigned int i = 0; i < sizeof(unsigned int); ++i) {
		m_Pipe->sendByte(valueAsBytes[i]);
	}
	m_bitsLeft = 32;
	m_combined = 0;
	return GLE_IMAGE_ERROR_NONE;
}

int GLEBitsTo32BitByteStream::sendByte(GLEBYTE byte) {
	if (m_bitsLeft > 0) {
		m_combined |= ((unsigned int)byte) << (32 - m_bitsLeft);
		m_bitsLeft--;
	}
	if (m_bitsLeft == 0) {
		flushBufferByte();
	}
	return GLE_IMAGE_ERROR_NONE;
}

int GLEBitsTo32BitByteStream::endScanLine() {
	if (m_bitsLeft != 32) flushBufferByte();
	return GLEPipedByteStream::endScanLine();
}

int GLEBitsTo32BitByteStream::term() {
	if (m_bitsLeft != 32) flushBufferByte();
	return GLEPipedByteStream::term();
}

GLEComponentRemovalByteStream::GLEComponentRemovalByteStream(GLEByteStream* pipe, int main, int remove) : GLEPipedByteStream(pipe) {
	m_Index = 0;
	m_Removed = 0;
	m_Main = main;
	m_Total = main+remove;
}

GLEComponentRemovalByteStream::~GLEComponentRemovalByteStream() {
}

int GLEComponentRemovalByteStream::sendByte(GLEBYTE byte) {
	if (m_Index < m_Main) {
		m_Pipe->sendByte(byte);
	} else {
		m_Removed++;
	}
	m_Index++;
	if (m_Index >= m_Total) {
		m_Index = 0;
	}
	return GLE_IMAGE_ERROR_NONE;
}

int GLEComponentRemovalByteStream::endScanLine() {
	m_Index = 0;
	return GLEPipedByteStream::endScanLine();
}

GLEAlphaRemovalByteStream::GLEAlphaRemovalByteStream(GLEByteStream* pipe, int components) : GLEPipedByteStream(pipe) {
	m_Components = components-1;
	m_Index = 0;
	// Do not support images with more channels
	if (m_Components > GLE_BITMAP_MAX_COMPONENTS) {
		m_Components = GLE_BITMAP_MAX_COMPONENTS;
	}
}

GLEAlphaRemovalByteStream::~GLEAlphaRemovalByteStream() {
}

int GLEAlphaRemovalByteStream::sendByte(GLEBYTE byte) {
	if (m_Index >= m_Components) {
		unsigned int adjust = 255-byte;
		for (int i = 0; i < m_Components; i++) {
			unsigned int output = (unsigned int)m_Buffer[i] + adjust;
			if (output >= 0xFF) m_Pipe->sendByte(0xFF);
			else m_Pipe->sendByte((GLEBYTE)output);
		}
		m_Index = 0;
	} else {
		m_Buffer[m_Index++] = byte;
	}
	return GLE_IMAGE_ERROR_NONE;
}

int GLEAlphaRemovalByteStream::endScanLine() {
	m_Index = 0;
	return GLEPipedByteStream::endScanLine();
}

IpolData::IpolData() {
}

IpolData::~IpolData() {
}

IpolDoubleMatrix::IpolDoubleMatrix(double* data, int wd, int hi) {
	m_Width = wd;
	m_Height = hi;
	m_Data = data;
}

IpolDoubleMatrix::~IpolDoubleMatrix() {
}

double IpolDoubleMatrix::getValue(int x, int y) {
	if (x < 0) x = 0;
	if (x >= m_Width) x = m_Width - 1;
	if (y < 0) y = 0;
	if (y >= m_Height) y = m_Height - 1;
	return m_Data[y * m_Width + x];
}

Ipol::Ipol()
{
}

Ipol::~Ipol()
{
}

BicubicIpol::BicubicIpol(IpolData* data) {
	m_Data = data;
}

double BicubicIpol::R(double x) {
	double v;
	double sum = 0.0;
	v = x + 2;
	if (v > 0) sum += v*v*v;
	v = x + 1;
	if (v > 0) sum -= 4*v*v*v;
	if (x > 0) sum += 6*x*x*x;
	v = x - 1;
	if (v > 0) sum -= 4*v*v*v;
	return sum/6.0;
}

double BicubicIpol::ipol(double xp, double yp) {
	double x = xp * m_Data->getWidth();
	double y = yp * m_Data->getHeight();
	int i = (int)floor(x);
	int j = (int)floor(y);
	double dx = x - i;
	double dy = y - j;
	double value = 0;
	for (int m = -1; m <= 2; m++) {
		int xo = i + m;
		double rx = R(m-dx);
		for (int n = -1; n <= 2; n++) {
			int yo = j + n;
			value += m_Data->getValue(xo, yo) * rx * R(dy-n);
		}
	}
	return value;
}

NearestIpol::NearestIpol(IpolData* data):
	m_Data(data)
{
}

double NearestIpol::ipol(double xp, double yp) {
	return m_Data->getValue(gle_round_int(xp * m_Data->getWidth()), gle_round_int(yp * m_Data->getHeight()));
}

void GLEBitmapSetPalette(GLEBYTE* pal, int offs, double red, double green, double blue) {
	int i_red = (int)floor(red*255+0.5);
	int i_green = (int)floor(green*255+0.5);
	int i_blue = (int)floor(blue*255+0.5);
	if (i_red > 255) i_red = 255;
	if (i_green > 255) i_green = 255;
	if (i_blue > 255) i_blue = 255;
	if (i_red < 0) i_red = 0;
	if (i_green < 0) i_green = 0;
	if (i_blue < 0) i_blue = 0;
	pal[offs*3] = i_red;
	pal[offs*3+1] = i_green;
	pal[offs*3+2] = i_blue;
}

GLEBYTE* GLEBitmapCreateColorPalette(int nc) {
	int offset = 1;
	int nintervals = 6;
	int divisor = 3;
	int num = nc-offset;
	int n = (num / (nintervals*divisor))*divisor;
	nc = n*nintervals+offset;
	GLEBYTE* result = new GLEBYTE[nc*3];
	double ninv = 1.0/n;
	int n3 = n/3;
	int n23 = 2*n3;
	double third = n3*ninv;
	double twothirds = n23*ninv;
	for (int i = 0; i < n3; ++i) {
		double ininv = i*ninv;
		GLEBitmapSetPalette(result, i, ininv, 0.0, ininv);
		GLEBitmapSetPalette(result, n3+i, third, 0.0, third+ininv);
		GLEBitmapSetPalette(result, n23+i, third-ininv, 0.0, twothirds+ininv);
	}
	for (int i = 0; i < n; ++i) {
		double ininv = i*ninv;
		double ininv1 = 1.0-ininv;
		GLEBitmapSetPalette(result, n+i, 0.0, ininv, 1.0);
		GLEBitmapSetPalette(result, 2*n+i, 0.0, 1.0, ininv1);
		GLEBitmapSetPalette(result, 3*n+i, ininv, 1.0, 0.0);
		GLEBitmapSetPalette(result, 4*n+i, 1.0, ininv1, 0.0);
		GLEBitmapSetPalette(result, 5*n+i, 1.0, ininv, ininv);
	}
	GLEBitmapSetPalette(result, 6*n, 1.0, 1.0, 1.0);
	return result;
}
