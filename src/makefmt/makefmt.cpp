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

#include <fstream>
#include <map>

#include "../gle/basicconf.h"
#include "../gle/tokens/stokenizer.h"
#include "../gle/file_io.h"
#include "parseAFM.h"


int debugit = 0;
#define dbg if (debugit)
void writefmt(const char* fmtname, const char* encname, FontInfo *fi);

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
	bool used;
public:
	GLEFontCharData();
	~GLEFontCharData();
};

GLEFontCharData::GLEFontCharData() {
	used = false;
	wx = 0.0;
	wy = 0.0;
	x1 = 0.0;
	y1 = 0.0;
	x2 = 0.0;
	y2 = 0.0;
}

GLEFontCharData::~GLEFontCharData() {
}

vector<string> accents;

void init_accents() {
	accents.push_back("caron");
	accents.push_back("cedilla");
	accents.push_back("acute");
	accents.push_back("dieresis");
	accents.push_back("circumflex");
	accents.push_back("grave");
	accents.push_back("ring");
	accents.push_back("tilde");
}

int find_accent(const string& accent) {
	for (vector<string>::size_type i = 0; i < accents.size(); i++) {
		if (accent == accents[i]) {
			return i;
		}
	}
	return -1;
}

int main(int argc, char *argv[]) {
	char filename[80];
	char fmtname[80];
	init_accents();
	/* encoding file given? */
	int argpos = 1;
	const char* encname = NULL;
	if (strcmp(argv[argpos], "-e") == 0) {
		encname = argv[argpos+1];
		argpos += 2;
	}
	/* output file given? */
	if (strcmp(argv[argpos], "-o") == 0) {
		strcpy(filename, argv[argpos+2]);
		strcpy(fmtname, argv[argpos+1]);
	} else {
		strcpy(filename, argv[argpos]);
		char* s = strchr(filename,'.');
		if (s!=NULL) *s=0;
		strcpy(fmtname,filename);
		if (strchr(filename,':')!=NULL) strcpy(fmtname,strchr(filename,':')+1);
		strcat(fmtname,".fmt");
		strcat(filename,".afm");
	}
	printf("[%s]==>[%s]\n",filename,fmtname);
	if (!filename[0]) {
		printf ("*** ERROR: can't open. filename is missing.\n");
		return 0;
	}
	FILE* fp = fopen(filename, READ_BIN);
	if (fp == NULL) {
		printf ("*** ERROR: can't find: %s\n", filename );
		perror("");
		exit(1);
	}
	FontInfo *fi;
	switch (parseFile(fp, &fi, P_ALL)) {
		case parseError:
			printf("*** ERROR: problem in parsing the AFM File.\n");
			exit(1);
		case ok:
			fclose(fp);
			writefmt(fmtname, encname, fi);
			break;
		case earlyEOF:
			printf("The AFM File ended prematurely.\n");
			exit(1);
			break;
		default:
			break;
	}
	return 0;
}

unsigned int font_str_to_code(map<string, unsigned int>& charcodes, const char* name) {
	string my_name = name;
	map<string, unsigned int>::iterator it = charcodes.find(my_name);
	if (it == charcodes.end()) {
		printf("error: can't find code point for '%s'\n", name);
		exit(-1);
	}
	return it->second;
}

void writefmt(const char* fmtname, const char* encname, FontInfo *fi) {
	/* create output file */
	GLEFileIO fmt;
	fmt.open(fmtname, WRITE_BIN);
	if (!fmt.isOpen()) {
		perror ("Can't open output file");
		exit(1);
	}
	GLEFontTable fnt = {0};
	fnt.encoding = 1;
	if (strcmp(fi->gfi->encodingScheme,"TEXENCODING") == 0) {
		fnt.encoding = 3;
	}
	if (strcmp(fi->gfi->encodingScheme,"AdobeStandardEncoding") == 0) {
		fnt.encoding = 1;
	}
	if (strcmp(fi->gfi->encodingScheme,"GLEMARK") == 0) {
		fnt.encoding = 5;
	}
	fnt.scale = 1000;
	float scl = 0.001;
	fnt.slant = fi->gfi->italicAngle;
	fnt.uposition = fi->gfi->underlinePosition*scl;
	fnt.uthickness =  fi->gfi->underlineThickness*scl;
	fnt.fx1 = fi->gfi->fontBBox.llx*scl;
	fnt.fy1 = fi->gfi->fontBBox.lly*scl;
	fnt.fx2 = fi->gfi->fontBBox.urx*scl;
	fnt.fy2 = fi->gfi->fontBBox.ury*scl;
	fnt.caphei = fi->gfi->capHeight*scl;
	fnt.xhei = fi->gfi->xHeight*scl;
	fnt.descender = fi->gfi->descender*scl;
	fnt.ascender = fi->gfi->ascender*scl;
	/* look for space character */
	for (int i = 0; i < fi->numOfChars; i++) {
		if (fi->cmi[i].code == 32) {
			fnt.space = fi->cmi[i].wx*scl;
			fnt.space_stretch = fnt.space*0.5;
			fnt.space_shrink = fnt.space*0.3;
			break;
		}
	}
	/* write out global font information */
	fmt.fwrite(&fnt, sizeof(GLEFontTable), 1);
	/* read encoding table */
	map<string, unsigned int> charcodes;
	vector<unsigned int> encoding_unicode;
	vector<unsigned int> encoding_code;
	int nb_chars = 256;
	if (encname != NULL) {
		ifstream strm;
		strm.open(encname);
		if (!strm.is_open()) {
			printf("error: can't open '%s'\n", encname);
			exit(-1);
		}
		char_separator sep(" ,;");
		tokenizer<char_separator> tokens(sep);
		while (strm.good()) {
			string line;
			getline(strm, line);
			/* remove comments */
			string::size_type pos = line.find('!');
			if (pos != string::npos) {
				line = line.substr(0, pos);
			}
			/* tokenize line */
			tokens.set_input(line);
			if (tokens.has_more()) {
				string unicode = tokens.next_token();
				if (unicode.length() < 1 || unicode[0] != '$') {
					printf("error: illegal unicode at line '%s'\n", line.c_str());
				}
				char *ptr = NULL;
				unsigned int unicode_pt = (unsigned int)strtol(unicode.c_str()+1, &ptr, 16);
				if (*ptr != 0) {
					printf("error: illegal unicode at line '%s'\n", line.c_str());
				}
				if (!tokens.has_more()) {
					printf("error: too few tokens at line '%s'\n", line.c_str());
				}
				string code = tokens.next_token();
				unsigned int code_pt = (unsigned int)strtol(code.c_str(), &ptr, 10);
				if (*ptr != 0) {
					printf("error: illegal code at line '%s'\n", line.c_str());
				}
				if (!tokens.has_more()) {
					printf("error: too few tokens at line '%s'\n", line.c_str());
				}
				string name = tokens.next_token();
				// cout << "unicode: '" << unicode_pt << "'" << endl;
				// cout << "code: '" << code_pt << "'" << endl;
				// cout << "name: '" << name << "'" << endl;
				charcodes[name] = code_pt;
				encoding_unicode.push_back(unicode_pt);
				encoding_code.push_back(code_pt);
				if (code_pt+1 > (unsigned int)nb_chars) {
					/* update number of characters in font */
					nb_chars  = code_pt + 1;
				}
			}
		}
		strm.close();
	}
	/* create character metrics data */
	vector<GLEFontCharData*> chardata;
	for (int i = 0; i < nb_chars; i++) {
		chardata.push_back(new GLEFontCharData());
	}
	/* make code points for all characters */
	for (int i = 0; i < fi->numOfChars; i++) {
		CharMetricInfo *ci = &fi->cmi[i];
		if (ci->code != -1) {
			unsigned int code_pt = (unsigned int)ci->code;
			charcodes[ci->name] = code_pt;
		}
	}
	/* read in all characters */
	for (int i = 0; i < fi->numOfChars; i++) {
		CharMetricInfo *ci = &fi->cmi[i];
		unsigned int code_pt = 0;
		if (ci->code != -1) {
			code_pt = (unsigned int)ci->code;
		} else {
			string my_name = ci->name;
			map<string, unsigned int>::iterator it = charcodes.find(my_name);
			if (it == charcodes.end()) {
				/* no code point for this character in encoding */
				continue;
			} else {
				code_pt = it->second;
			}
		}
		/* set character metrics */
		GLEFontCharData* cdata = chardata[code_pt];
		cdata->used = true;
		cdata->wx = ci->wx*scl;
		cdata->wy = ci->wy*scl;
		cdata->x1 = ci->charBBox.llx*scl;
		cdata->x2 = ci->charBBox.urx*scl;
		cdata->y1 = ci->charBBox.lly*scl;
		cdata->y2 = ci->charBBox.ury*scl;
		/* store ligatures */
		for (Ligature *node = ci->ligs; node != NULL; node = node->next) {
			GLEFontLigatureInfo ldata;
			ldata.NextChar = font_str_to_code(charcodes, node->succ);
			ldata.RepChar = font_str_to_code(charcodes, node->lig);
			cdata->Lig.push_back(ldata);
			chardata[ldata.NextChar]->used = true;
			chardata[ldata.RepChar]->used = true;
		}
	}
	/* read kerning data */
	for (int i = 0; i < fi->numOfPairs; i++) {
		PairKernData *pkd = fi->pkd;
		unsigned int c1 = font_str_to_code(charcodes, pkd[i].name1);
		unsigned int c2 = font_str_to_code(charcodes, pkd[i].name2);
		GLEFontKernInfo kinfo;
		kinfo.CharCode = c2;
		kinfo.X = pkd[i].xamt*scl;
		kinfo.Y = pkd[i].yamt*scl;
		chardata[c1]->used = true;
		chardata[c2]->used = true;
		chardata[c1]->Kern.push_back(kinfo);
	}
	/* write out number of characters */
	fmt.fwrite(&nb_chars, sizeof(int), 1);
	/* write unicode table */
	int size = encoding_code.size();
	fmt.fwrite(&size, sizeof(int), 1);
	if (size > 0) {
		fmt.fwrite(&encoding_unicode[0], sizeof(unsigned int), size);
		fmt.fwrite(&encoding_code[0], sizeof(unsigned int), size);
	}
	/* write out character data */
	for (int i = 0; i < nb_chars; i++) {
		GLEFontCharData* cdata = chardata[i];
		if (cdata->used) {
			fmt.fputc(1);
			fmt.fwrite(&cdata->wx, sizeof(float), 1);
			fmt.fwrite(&cdata->wy, sizeof(float), 1);
			fmt.fwrite(&cdata->x1, sizeof(float), 1);
			fmt.fwrite(&cdata->y1, sizeof(float), 1);
			fmt.fwrite(&cdata->x2, sizeof(float), 1);
			fmt.fwrite(&cdata->y2, sizeof(float), 1);
			unsigned int ksize = cdata->Kern.size();
			fmt.fwrite(&ksize, sizeof(unsigned int), 1);
			if (ksize > 0) fmt.fwrite(&cdata->Kern[0], sizeof(GLEFontKernInfo), ksize);
			unsigned int lsize = cdata->Lig.size();
			fmt.fwrite(&lsize, sizeof(unsigned int), 1);
			if (lsize > 0) fmt.fwrite(&cdata->Lig[0], sizeof(GLEFontLigatureInfo), lsize);
		} else {
			fmt.fputc(0);
		}
	}
	/* Also do the composites to support accents in GLE (new 070506) */
	for (int i = 0; i < fi->numOfComps; i++) {
		CompCharData *ccd = &(fi->ccd[i]);
		string name = ccd->ccName;
		if (name.length() > 1) {
			int char_code = name[0];
			string accent_code = name.substr(1);
			int accent_idx = find_accent(accent_code);
			if (accent_idx != -1) {
				if (ccd->numOfPieces != 2) {
					cout << ">>> incorrect number of pieces: '" << name << "': " << ccd->numOfPieces << endl;
				} else {
					fmt.fwrite(&char_code, sizeof(int), 1);
					fmt.fwrite(&accent_idx, sizeof(int), 1);
					for (int j = 0; j < ccd->numOfPieces; j++) {
						Pcc* piece = &ccd->pieces[j];
						int piece_code = font_str_to_code(charcodes, piece->pccName);
						if (piece_code == 0) {
							cout << ">>> can't find piece: '" << piece->pccName << "'" << endl;
						}
						fmt.fwrite(&piece_code, sizeof(int), 1);
						float dx = piece->deltax * scl;
						float dy = piece->deltay * scl;
						fmt.fwrite(&dx, sizeof(float), 1);
						fmt.fwrite(&dy, sizeof(float), 1);
					}
				}
			} else {
				cout << ">>> unknown accent: '" << accent_code << "'" << endl;
			}
		} else {
			cout << ">>> composite too short: '" << name << "'" << endl;
		}
	}
	int term = 0;
	fmt.fwrite(&term, sizeof(int), 1);
	fmt.close();
}
