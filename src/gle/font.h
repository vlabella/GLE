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

struct char_data {
	float wx, wy, x1, y1, x2, y2;
};

class FontCompositeInfo {
public:
	int c1, c2;
	double dx1, dy1;
	double dx2, dy2;
};

typedef struct {
	unsigned int CharCode;
	float X;
	float Y;
} GLEFontKernInfo;

typedef struct {
	unsigned int NextChar;
	unsigned int RepChar;
} GLEFontLigatureInfo;

typedef struct {
	int encoding; /* 1 = postscript text, 2 = postscrip symbol, 3 = TEX text, 4 = TEX symbol, 5 = GLE markers */
	float space, space_stretch, space_shrink;
	float scale, slant, uposition, uthickness;
	float fx1, fy1, fx2, fy2, caphei, xhei, descender, ascender;
} GLEFontTable;

class GLEFontCharData {
public:
	vector<GLEFontKernInfo> Kern;
	vector<GLEFontLigatureInfo> Lig;
	float wx, wy, x1, y1, x2, y2;
public:
	GLEFontCharData();
	~GLEFontCharData();
};

class GLEFontUnicodeMap;

class GLECoreFont {
public:
	char *name;
	char *full_name;
	char *file_metric;
	char *file_vector;
	char *file_bitmap;
	bool metric_loaded;
	bool error;
	GLEFontTable info;
	GLEFontUnicodeMap* unimap;
	IntKeyHash<FontCompositeInfo*> composites;
public:
	GLECoreFont();
	~GLECoreFont();
	int char_lig(int *c1, int c2);
	void char_kern(int c1, int c2, float *w);
	FontCompositeInfo* get_composite_char(int ch, int accent);
	int unicode_map(unsigned int ucode);
	GLEFontCharData* getCharData(int cc);
	GLEFontCharData* getCharDataThrow(int cc);
	GLEFontCharData* addCharData();

private:
	std::vector<GLEFontCharData*> cdata;
};

GLECoreFont* get_core_font(int font);
GLECoreFont* get_core_font_ensure_loaded(int font);

#ifndef FONTDEF
	#define FONTDEF
#endif

FONTDEF vector<GLECoreFont*> fnt;
