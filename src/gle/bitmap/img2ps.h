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

#ifndef INC_IMG2PS_H
#define INC_IMG2PS_H

/*
 * 2004 Jan Struyf
 *
 * Still busy implementing - import of JPEG, GIF and TIFF should work to some extend
 * If you encounter problems, please contact the GLE list
 */

#ifdef HAVE_LIBTIFF
	#include <tiffio.h>
#endif

#ifdef HAVE_LIBPNG
	#include <png.h>
#endif

#define GLE_BITMAP_INDEXED    1
#define GLE_BITMAP_GRAYSCALE  2
#define GLE_BITMAP_RGB        3

#define GLE_IMAGE_ERROR_NONE      0
#define GLE_IMAGE_ERROR_DATA      1
#define GLE_IMAGE_ERROR_FILE      2
#define GLE_IMAGE_ERROR_TYPE      3
#define GLE_IMAGE_ERROR_INTERN    4
#define GLE_IMAGE_ERROR_NOT_IMPL  10

#define GLE_BITMAP_NONE 0
#define GLE_BITMAP_LZW  1
#define GLE_BITMAP_JPEG 2

#define GLE_BITMAP_PREPARE_SCANLINE 0

#define GLE_BITMAP_MAX_COMPONENTS 3

#define	CVT16TO8(x) (((x) * 255) / ((1L<<16)-1))

typedef unsigned char GLEBYTE;
typedef unsigned int GLEDWORD;

typedef struct {
	unsigned char red;
	unsigned char green;
	unsigned char blue;
} rgb;

class GLEByteStream;

class GLEBitmap {
protected:
	string m_Error;
	int  m_Height;
	int  m_Width;
	char m_Mode;
	int  m_Components;
	int  m_ExtraComponents;
	int  m_Colors;
	char m_Encoding;
	char m_Interlaced;
	char m_Alpha;
	int  m_BitsPerComponent;
	rgb* m_Palette;
	int m_ASCII85;
	double m_Compress;
public:
	GLEBitmap();
	virtual ~GLEBitmap();
	inline int getWidth() { return m_Width; }
	inline int getHeight() { return m_Height; }
	inline int getNbColors() { return m_Colors; }
	inline void setNbColors(int colors) { m_Colors = colors; }
	inline int getComponents() { return m_Components; }
	inline void setComponents(int components) { m_Components = components; }
	inline void setMode(char mode) { m_Mode = mode; }
	inline char getMode() { return m_Mode; }
	inline char isIndexed() { return m_Mode == GLE_BITMAP_INDEXED; }
	inline char isGrayScale() { return m_Mode == GLE_BITMAP_GRAYSCALE; }
	inline char isInterlaced() { return m_Interlaced; }
	inline void setInterlaced(char interlaced) { m_Interlaced = interlaced; }
	inline char isAlpha() { return m_Alpha; }
	inline void setAlpha(char alpha) { m_Alpha = alpha; }
	inline char getEncoding() { return m_Encoding; }
	inline void setEncoding(char encoding) { m_Encoding = encoding; }
	inline int getBitsPerComponent() { return m_BitsPerComponent; }
	inline void setBitsPerComponent(int bits) { m_BitsPerComponent = bits; }
	inline int isASCII85() { return m_ASCII85; }
	inline void setASCII85(int val) { m_ASCII85 = val; }
	inline double getCompress() { return m_Compress; }
	inline void setCompress(double val) { m_Compress = val; }
	inline rgb* getPalette() { return m_Palette; }
	inline int getExtraComponents() { return m_ExtraComponents; }
	inline void setExtraComponents(int extra) { m_ExtraComponents = extra; }
	inline const string& getError() { return m_Error; }
	inline void setError(const string& err) { m_Error = err; }
	inline void setError(const char* err) { m_Error = err; }
	rgb* allocPalette(int ncolors);
	int getScanlineSize();
	int getColorComponents();
	int getMaxBits();
	void checkGrayScalePalette();
	virtual int open(const string& fname);
	virtual int readHeader();
	virtual int toPS(ostream* fp);
	virtual void loadImageData();
	virtual int prepare(int mode);
	virtual int decode(GLEByteStream* output);
	virtual int coded(GLEByteStream* output);
	virtual void close();
	virtual string getFName();
	void printInfo(ostream& os);
};

class GLEFileBitmap : public GLEBitmap {
protected:
	GLEFileIO m_file;
public:
	GLEFileBitmap();
	virtual ~GLEFileBitmap();
	virtual int open(const string& fname);
	virtual void close();
	virtual string getFName();
	int read16BE();
	int read16LE();
	inline int read8() { return fgetc(m_file.getFile()); }
};

class GLEJPEG : public GLEFileBitmap {
public:
	GLEJPEG();
	virtual ~GLEJPEG();
	virtual int readHeader();
	virtual int coded(GLEByteStream* output);
	int checkJPG();
	int readImageSize();
};

class GLEGIFDecoder;

class GLEGIF : public GLEFileBitmap {
protected:
	long m_ImageOffs;
public:
	GLEGIF();
	virtual ~GLEGIF();
	virtual int readHeader();
	virtual int decode(GLEByteStream* output);
	void updateImageType();
	int headerExtension();
	int headerImage();
	void headerCOMExt();
	void skipBlocks();
};

#ifdef HAVE_LIBTIFF
class GLETIFF : public GLEBitmap {
protected:
	TIFF* m_Tiff;
	uint16 m_TIFFCompress;
	string m_fname;
public:
	GLETIFF();
	virtual ~GLETIFF();
	virtual int open(const string& fname);
	virtual int readHeader();
	virtual int prepare(int mode);
	virtual int decode(GLEByteStream* output);
	virtual void close();
	virtual string getFName();
	int isCCITTCompression();
	uint16 getTIFFCompression() { return m_TIFFCompress; }
};
#endif

#ifdef HAVE_LIBPNG
class GLEPNG : public GLEFileBitmap {
protected:
	png_structp m_PNGPtr;
	png_infop m_InfoPtr;
	png_infop m_EndInfo;
public:
	GLEPNG();
	virtual ~GLEPNG();
	virtual int readHeader();
	virtual int prepare(int mode);
	virtual int decode(GLEByteStream* output);
};
#endif

class GLEByteStream {
protected:
	char m_Terminated;
public:
	GLEByteStream();
	virtual ~GLEByteStream();
	virtual int send(GLEBYTE* bytes, GLEDWORD count);
	virtual int sendByte(GLEBYTE byte) = 0;
	virtual int endScanLine();
	virtual int term();
	inline int isTerminated() { return m_Terminated; }
};

class GLERecordedByteStream : public GLEByteStream {
protected:
	std::vector<GLEBYTE> m_bytes;
public:
	GLERecordedByteStream();
	virtual ~GLERecordedByteStream();
	virtual int sendByte(GLEBYTE byte);
	unsigned long getNbBytes() const;
	GLEBYTE* getBytes();
};

class GLEPipedByteStream : public GLEByteStream {
protected:
	GLEByteStream* m_Pipe;
public:
	GLEPipedByteStream(GLEByteStream* pipe);
	virtual ~GLEPipedByteStream();
	virtual int endScanLine();
	virtual int term();
};

class GLEPNegateByteStream : public GLEPipedByteStream {
public:
	GLEPNegateByteStream(GLEByteStream* pipe);
	virtual ~GLEPNegateByteStream();
	virtual int sendByte(GLEBYTE byte);
};

class GLEIndexedToRGBByteStream : public GLEPipedByteStream {
protected:
	rgb* m_palette;
public:
	GLEIndexedToRGBByteStream(GLEByteStream* pipe, rgb* palette);
	virtual ~GLEIndexedToRGBByteStream();
	virtual int sendByte(GLEBYTE byte);
};

class GLERGBATo32BitByteStream : public GLEPipedByteStream {
protected:
	GLEBYTE m_components[4];
	int m_index;
	int m_nbComponents;
public:
	GLERGBATo32BitByteStream(GLEByteStream* pipe, bool isAlpha);
	virtual ~GLERGBATo32BitByteStream();
	virtual int sendByte(GLEBYTE byte);
	virtual int endScanLine();
};

class GLEPixelCombineByteStream : public GLEPipedByteStream {
protected:
	GLEBYTE m_Combined;
	int m_BitsPerComponent;
	int m_BitsLeft;
public:
	GLEPixelCombineByteStream(GLEByteStream* pipe, int bpc);
	virtual ~GLEPixelCombineByteStream();
	virtual int sendByte(GLEBYTE byte);
	virtual int endScanLine();
	virtual int term();
private:
	int flushBufferByte();
};

class GLEBitsTo32BitByteStream : public GLEPipedByteStream {
protected:
	unsigned int m_combined;
	int m_bitsLeft;
public:
	GLEBitsTo32BitByteStream(GLEByteStream* pipe);
	virtual ~GLEBitsTo32BitByteStream();
	virtual int sendByte(GLEBYTE byte);
	virtual int endScanLine();
	virtual int term();
private:
	int flushBufferByte();
};

class GLEAlphaRemovalByteStream : public GLEPipedByteStream {
protected:
	GLEBYTE m_Buffer[GLE_BITMAP_MAX_COMPONENTS];
	int m_Components;
	int m_Index;
public:
	GLEAlphaRemovalByteStream(GLEByteStream* pipe, int components);
	virtual ~GLEAlphaRemovalByteStream();
	virtual int sendByte(GLEBYTE byte);
	virtual int endScanLine();
};

class GLEComponentRemovalByteStream : public GLEPipedByteStream {
protected:
	int m_Index, m_Main, m_Total, m_Removed;
public:
	GLEComponentRemovalByteStream(GLEByteStream* pipe, int main, int remove);
	virtual ~GLEComponentRemovalByteStream();
	virtual int sendByte(GLEBYTE byte);
	virtual int endScanLine();
};

class GLEASCII85ByteStream : public GLEByteStream {
private:
	ostream* m_File;
	unsigned char m_Buffer[10];
	int m_Count;
	int m_BreakLength;
public:
	GLEASCII85ByteStream(ostream* file);
	virtual ~GLEASCII85ByteStream();
	virtual int sendByte(GLEBYTE byte);
	virtual int term();
	inline void decBreakLength(int cnt) { m_BreakLength -= cnt; }
};

class GLELZWByteStream : public GLEPipedByteStream {
private:
	GLEBYTE* m_Data;           /* compression scheme private data */
	GLEBYTE* m_RawData;        /* raw data buffer */
	GLEDWORD m_RawDatasize;    /* # of bytes in raw data buffer */
	GLEBYTE* m_RawCP;          /* current spot in raw buffer */
	GLEDWORD m_RawCC;          /* bytes unread from raw buffer */
	GLEBYTE  m_State;
public:
	GLELZWByteStream(GLEByteStream* pipe);
	virtual ~GLELZWByteStream();
	virtual int send(GLEBYTE* bytes, GLEDWORD count);
	virtual int sendByte(GLEBYTE byte);
	virtual int term();
private:
	int setupEncode();
	int preEncode();
	int encode(GLEBYTE* bp, GLEDWORD cc);
	int postEncode();
	int flushData();
	int init();
	void cleanUp();
};

enum IpolType
{
	IPOL_TYPE_BICUBIC,
	IPOL_TYPE_NEAREST
};

class IpolData {
protected:
	int m_Width;
	int m_Height;
public:
	IpolData();
	virtual ~IpolData();
	inline int getWidth() { return m_Width; }
	inline int getHeight() { return m_Height; }
	virtual double getValue(int x, int y) = 0;
};

class IpolDoubleMatrix: public IpolData {
protected:
	double* m_Data;
public:
	IpolDoubleMatrix(double* data, int wd, int hi);
	virtual ~IpolDoubleMatrix();
	virtual double getValue(int x, int y);
};

class Ipol {
public:
	Ipol();
	virtual ~Ipol();
	virtual double ipol(double xp, double yp) = 0;
};

class BicubicIpol: public Ipol {
protected:
	IpolData* m_Data;
public:
	BicubicIpol(IpolData* data);
	virtual double ipol(double xp, double yp);
	double R(double x);
};

class NearestIpol: public Ipol {
protected:
	IpolData* m_Data;
public:
	NearestIpol(IpolData* data);
	virtual double ipol(double xp, double yp);
};

void GLEBitmapSetPalette(GLEBYTE* pal, int offs, double red, double green, double blue);

GLEBYTE* GLEBitmapCreateColorPalette(int nc);

#endif
