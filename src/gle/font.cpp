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

#include "all.h"
#include "tokens/Tokenizer.h"
#include "core.h"
#include "file_io.h"
#include "gprint.h"
#include "cutils.h"
#include "gle-interface/gle-interface.h"

int text_gprint(int *in,int ilen);
int fftext_block(uchar *s,double width,int justify);
void font_load(void) throw (ParserError);
void font_init(void);

int do_prim(uchar **in,int *out,int *lout);  /*  \frac{text}{text} */

#define true (!false)
#define false 0

#define dbg if ((gle_debug & 32)>0)


extern int gle_debug;
/* typedef char unsigned uchar; */

/*  FONT.DAT contains  names of fonts, and indexes for each font, and
	names of metric, vector and bitmap files
	This enables fonts to be referred to by number, as their index will
	not change */

#include "font.h"

class GLEFontUnicodeMap : public map<unsigned int, unsigned int> {
};

GLECoreFont* init_core_font(int n) {
	while ((unsigned int)n >= fnt.size()) {
		fnt.push_back(new GLECoreFont());
	}
	return fnt[n];
}

GLECoreFont* get_core_font(int n) {
	if (n < 0 || (unsigned int)n >= fnt.size()) {
		gprint("no font number: %d", n);
		return fnt[1];
	}
	return fnt[n];
}

GLECoreFont* get_core_font_ensure_loaded(int font) {
	if (fnt.size() == 0) {
		/* load font.dat */
		/* FIXME: move loading of font.dat to GLE's mandatory init code? */
		font_load();
	}
	if (font < 0 || (unsigned int)font >= fnt.size()) {
		gprint("no font number: %d", font);
		font = 1;
	}
	GLECoreFont* cfont = fnt[font];
	if (!cfont->metric_loaded) {
		/* load font metric */
		font_load_metric(font);
	}
	return cfont;
}

char *font_getname(int i) {
	GLECoreFont* cfont = get_core_font(i);
	return cfont->name;
}

void char_bbox(int ff, int cc, double *xx1, double *yy1, double *xx2, double *yy2) {
	GLECoreFont* cfont = get_core_font_ensure_loaded(ff);
	GLEFontCharData* cdata = cfont->getCharData(cc);
	if (cdata != 0) {
		*xx1 = cdata->x1;
		*yy1 = cdata->y1;
		*xx2 = cdata->x2;
		*yy2 = cdata->y2;
	} else {
		*xx1 = 0;
		*xx2 = 0;
		*yy1 = 0;
		*yy2 = 0;
	}
}

void font_get_chardata(struct char_data *cd, int ff, int cc) {
	GLECoreFont* cfont = get_core_font(ff);
	GLEFontCharData* cdata = cfont->getCharData(cc);
	if (cdata != 0) {
		cd->x1 = cdata->x1;
		cd->y1 = cdata->y1;
		cd->x2 = cdata->x2;
		cd->y2 = cdata->y2;
	} else {
		cd->x1 = 0;
		cd->y1 = 0;
		cd->x2 = 0;
		cd->y2 = 0;
	}
}

void font_load(void) throw(ParserError) {
	/* load font.dat */
	char inbuff[200];
	string fname = fontdir("font.dat");
	FILE *fptr = fopen(fname.c_str(), READ_BIN);
	if (fptr == NULL) {
		TokenizerPos pos;
		pos.setColumn(-1);
		stringstream err_str;
		err_str << "unable to open 'font.dat' file '" << fname << "': ";
		str_get_system_error(err_str);
		err_str << endl;
		err_str << "set GLE_TOP to the directory containing the file \"inittex.ini\" and the fonts";
		ParserError err_exp(err_str.str(), pos, NULL);
		throw err_exp;
	}
	/* font_load() */
	GLEInterface* iface = GLEGetInterfacePointer();
	TokenizerLanguage lang;
	lang.setSpaceTokens(" ,\t\r\n");
	lang.setSingleCharTokens("()");
	lang.setLineCommentTokens("!");
	StringTokenizer tokens(&lang);
	while(fgets(SC inbuff, 200, fptr) != NULL) {
		tokens.set_string(inbuff);
		if (tokens.has_more_tokens()) {
			GLEFont* font = new GLEFont();
			string name = tokens.next_token();
			// font zero is a dummy; font 1 is the first valid font
			int n = tokens.next_integer();
			font->setIndex(n);
			font->setName(name);
			GLECoreFont* cfont = init_core_font(n);
			mystrcpy(&cfont->name, (char*)name.c_str());
			const string& metric = tokens.next_token();
			mystrcpy(&cfont->file_metric, (char*)metric.c_str());
			const string& vec = tokens.next_token();
			mystrcpy(&cfont->file_vector, (char*)vec.c_str());
			const string& bitmap = tokens.next_token();
			mystrcpy(&cfont->file_bitmap, (char*)bitmap.c_str());
			if (tokens.is_next_token("%")) {
				const string& full = tokens.read_line();
				font->setFullName(full);
				iface->addFont(font);
			} else if (tokens.is_next_token("-")) {
				string type = tokens.next_token();
				tokens.ensure_next_token("(");
				string parent = tokens.next_token();
				tokens.ensure_next_token(")");
				GLEFont* pfont = iface->getFont(parent);
				if (pfont == NULL) {
					g_throw_parser_error("parent font '", parent.c_str(), "' not found");
				} else {
					iface->addSubFont(font);
					font->setParent(pfont);
					if (type == "B") pfont->setStyle(GLEFontStyleBold, font);
					else if (type == "I") pfont->setStyle(GLEFontStyleItalic, font);
					else if (type == "BI") pfont->setStyle(GLEFontStyleBoldItalic, font);
					else g_throw_parser_error("font style '", type.c_str(), "' not defined");
				}
			}
		}
	}
	fclose(fptr);
}

void font_file_vector(int ff, char *s) {
	if (fnt.size() == 0) {
		/* load font.dat */
		font_load();
	}
	GLECoreFont* cfont = get_core_font(ff);
	strcpy(s, cfont->file_vector);
}

void font_replace_vector(int ff) {
	if (fnt.size() == 0) {
		/* load font.dat */
		font_load();
	}
	GLECoreFont* cfont = get_core_font(ff);
	myfree(cfont->file_vector);
	cfont->file_vector = sdup(fnt[17]->file_vector);
}

int font_get_encoding(int ff) {
	GLECoreFont* cfont = get_core_font_ensure_loaded(ff);
	return cfont->info.encoding;
}

int check_has_font(const std::string& name) {
	if (fnt.size() == 0) {
		/* load font.dat */
		font_load();
	}
	int ff = 0;
	// font zero is a dummy!
	for (unsigned int i = 1; i < fnt.size(); i++) {
		if (fnt[i]->name != NULL && str_i_equals(name, fnt[i]->name)) {
			ff = i;
			break;
		}
	}
	if (ff == 0) {
		/* don't know about font */
		return 0;
	}
	GLECoreFont* cfont = fnt[ff];
	if (cfont->metric_loaded) {
		return 1;
	}
	if (cfont->error) {
		return 0;
	}
	string fname = fontdir(cfont->file_metric);
	if (GLEFileExists(fname)) {
		font_load_metric(ff);
	} else {
		cfont->error = true;
	}
	return cfont->error ? 0 : 1;
}

void font_load_metric(int ff) {
	if (ff == 0) {
		gprint("There is no zero font, error loading font \n");
		return;
	}
	if (fnt.size() == 0) {
		/* load font.dat */
		font_load();
	}
	GLECoreFont* cfont = get_core_font(ff);
	if (cfont->metric_loaded) {
		/* font metric file already loaded */
		return;
	}
	/* don't do multiple tries */
	cfont->metric_loaded = true;
	/* try to open font metric file */
	string fname = fontdir(cfont->file_metric);
	GLEFileIO fmt;
	fmt.open(fname.c_str(), READ_BIN);
	if (!fmt.isOpen()) {
		cfont->error = true;
		ostringstream err;
		err << "font metric file not found: '" << fname << "'; spacing will be incorrect";
		g_message(err.str().c_str());
		myfree(cfont->file_metric);
		cfont->file_metric = sdup(fnt[1]->file_metric);
		fname = fontdir(cfont->file_metric);
		fmt.open(fname.c_str(), READ_BIN);
		if (!fmt.isOpen()) {
			gprint("can't open metric file: '%s'\n", fname.c_str());
			return;
		}
	}
	/* reads in font data */
	fmt.fread(&cfont->info, sizeof(GLEFontTable), 1);
	dbg printf("Encoding %d  slant %f,  box %f %f %f %f \n",
	           cfont->info.encoding,cfont->info.slant,cfont->info.fx1,
	           cfont->info.fy1,cfont->info.fx2,cfont->info.fy2);
	/* read number of characters */
	int nb_chars;
	fmt.fread(&nb_chars, sizeof(int), 1);
	/* read unicode map */
	int uni_map_size;
	fmt.fread(&uni_map_size, sizeof(int), 1);
	if (uni_map_size != 0) {
		unsigned int* encoding_unicode = new unsigned int[uni_map_size];
		unsigned int* encoding_code = new unsigned int[uni_map_size];
		fmt.fread(encoding_unicode, sizeof(unsigned int), uni_map_size);
		fmt.fread(encoding_code, sizeof(unsigned int), uni_map_size);
		for (int i = 0; i < uni_map_size; i++) {
			if (encoding_code[i] < (unsigned int)nb_chars) {
				(*cfont->unimap)[encoding_unicode[i]] = encoding_code[i];
			}
		}
		delete[] encoding_unicode;
		delete[] encoding_code;
	}
	/* read character data */
	for (int i = 0; i < nb_chars; i++) {
		GLEFontCharData* cdata = cfont->addCharData();
		if (fmt.fgetc() == 1) {
			fmt.fread(&cdata->wx, sizeof(float), 1);
			fmt.fread(&cdata->wy, sizeof(float), 1);
			fmt.fread(&cdata->x1, sizeof(float), 1);
			fmt.fread(&cdata->y1, sizeof(float), 1);
			fmt.fread(&cdata->x2, sizeof(float), 1);
			fmt.fread(&cdata->y2, sizeof(float), 1);
			unsigned int ksize;
			fmt.fread(&ksize, sizeof(unsigned int), 1);
			if (ksize > 0) {
				cdata->Kern.resize(ksize);
				fmt.fread(&cdata->Kern[0], sizeof(GLEFontKernInfo), ksize);
			}
			unsigned int lsize;
			fmt.fread(&lsize, sizeof(unsigned int), 1);
			if (lsize > 0) {
				cdata->Lig.resize(lsize);
				fmt.fread(&cdata->Lig[0], sizeof(GLEFontLigatureInfo), lsize);
			}
		}
	}
	/* add some dummy characters */
	for (int i = nb_chars; i <= 256; i++) {
		/* some routines in tex.cpp assume fonts have at least this number of characters */
		cfont->addCharData();
	}
	/* Also do the composites to support accents in GLE (new 070506) */
	int char_code = 0;
	fmt.fread(&char_code, sizeof(int), 1);
	while (char_code != 0) {
		int accent_idx;
		fmt.fread(&accent_idx, sizeof(int), 1);
		int hash_key = (char_code << 7) | accent_idx;
		FontCompositeInfo* info = new FontCompositeInfo();
		cfont->composites.add_item(hash_key, info);
		fmt.fread(&info->c1,  sizeof(int), 1);
		fmt.fread(&info->dx1, sizeof(double), 1);
		fmt.fread(&info->dy1, sizeof(double), 1);
		fmt.fread(&info->c2,  sizeof(int), 1);
		fmt.fread(&info->dx2, sizeof(double), 1);
		fmt.fread(&info->dy2, sizeof(double), 1);
		/* another character in the file? */
		fmt.fread(&char_code, sizeof(int), 1);
	}
	fmt.close();
}

GLEFontCharData::GLEFontCharData() {
	wx = 0.0;
	wy = 0.0;
	x1 = 0.0;
	y1 = 0.0;
	x2 = 0.0;
	y2 = 0.0;
}

GLEFontCharData::~GLEFontCharData() {
}

GLECoreFont::GLECoreFont() {
	name = NULL;
	full_name = NULL;
	file_metric = NULL;
	file_vector = NULL;
	file_bitmap = NULL;
	metric_loaded = false;
	error = false;
	unimap = new GLEFontUnicodeMap();
}

GLECoreFont::~GLECoreFont() {
}

int GLECoreFont::char_lig(int *c1, int c2) {
	GLEFontCharData* ch_data = getCharData(*c1);
	if (ch_data != 0) {
		for (unsigned int i = 0; i < ch_data->Lig.size(); i++) {
			if (ch_data->Lig[i].NextChar == (unsigned int)c2) {
				*c1 = ch_data->Lig[i].RepChar;
				return *c1;
			}
		}
	}
	return 0;
}

void GLECoreFont::char_kern(int c1, int c2, float *w) {
	GLEFontCharData* ch_data = getCharData(c1);
	if (ch_data != 0) {
		for (unsigned int i = 0; i < ch_data->Kern.size(); i++) {
			if (ch_data->Kern[i].CharCode == (unsigned int)c2) {
				*w = ch_data->Kern[i].X;
				return;
			}
		}
	}
	*w = 0;
}

FontCompositeInfo* GLECoreFont::get_composite_char(int ch, int accent) {
	int hash_key = (ch << 7) | accent;
	return composites.try_get(hash_key);
}

int GLECoreFont::unicode_map(unsigned int ucode) {
	GLEFontUnicodeMap::iterator i = unimap->find(ucode);
	if (i == unimap->end()) {
		return -1;
	} else {
		return i->second;
	}
}

GLEFontCharData* GLECoreFont::getCharData(int cc) {
	if (cc >= 0 && (unsigned int)cc < cdata.size()) {
		return cdata[cc];
	} else {
		return 0;
	}
}

GLEFontCharData* GLECoreFont::addCharData() {
	cdata.push_back(new GLEFontCharData());
	return cdata[cdata.size() - 1];
}

GLEFontCharData* GLECoreFont::getCharDataThrow(int cc) {
	GLEFontCharData* result = getCharData(cc);
	if (result == 0) {
		std::ostringstream msg;
		msg << "font '" << name << "' does not contain a character with id = " << cc;
		g_throw_parser_error(msg.str());
	}
	return result;
}
