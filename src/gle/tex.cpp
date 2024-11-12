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

#include "all.h"
#include "mem_limits.h"
#include "token.h"
#include "tokens/StringKeyHash.h"
#include "core.h"
#include "glearray.h"
#include "polish.h"
#include "pass.h"
#include "cutils.h"
#include "gprint.h"
#include "justify.h"
#include "file_io.h"
#include "texinterface.h"
#include "cmdline.h"
#include "config.h"

using namespace std;

// uncomment this if you want to store the
// tex font information to the decode.tex file.
// this will create the file decode.tex in your directory
//#define DO_TEX_DECODE
// there is no reason to read decode.tex except for
// debug purposes (I think).  At least GLE doesn't read
// decode.tex anywhere

/************************************************************************************
 * Explanation of the code:
 *
 * Main procedure for drawing text:
 *    void text_block(const string& s, double width, int justify, int innerjust)
 *
 * This calls:
 *    fftext_block
 *        text_tomacro
 *        text_topcode
 *           uses: do_prim
 *        text_wrapcode
 *           uses: set_glue
 *        text_draw
 *
 * text_tomacro
 *    -> expands macros (starting with "\")
 *    -> expands special characters (such as '^' and '_') unless they are escaped
 *
 * text_topcode
 *    -> convert result to op-codes
 *       -> does characters, ligatures, and kerning
 *       -> calls do_prim to handle primitive commands
 *
 * text_wrapcode
 *    -> introduce additional space (justify)
 *    -> handle newlines
 *
 * text_draw
 *    -> draw op-codes
 *
 * \accent{texcmr}{95}
 *    -> texcmr = font in which accent is defined
 *
 ************************************************************************************/

class TexArgStrs {
public:
	string str1;
	string str2;
	string str3;
	string str4;
public:
	void cmdParam1(uchar **in);
	void cmdParam2(uchar **in);
	void cmdParam3(uchar **in);
	void cmdParam4(uchar **in);
	void cmdParam12(uchar **in);
	void cmdParam4_swap34(uchar **in);
	inline const char* getCStr1() { return str1.c_str(); }
	inline const char* getCStr2() { return str2.c_str(); }
	inline const char* getCStr3() { return str3.c_str(); }
	inline const char* getCStr4() { return str4.c_str(); }
};

#define NORET (int) 0
double emtof(char *s);
double emtof(const string& s);
void font_reset_parskip(void);
void set_parskip(double v);
void set_lineskip(double v);
int ncat(char *a,char *b,int n);
void pp_fntchar(int f, int c,int *out, int *lout);
void cmd_token(uchar **in, uchar *cmdstr);
void set_base_size(void);
int find_primcmd(char *cmd);
void text_box(const string& s,double width,int *tbuff, int *rplen);
uchar* cmdParam(uchar **inp, uchar **pm, int* pmlen, int npm);
void text_tomacro(const string& in, uchar *out);
void texint(char *s, int *i);
void texint(const string& s, int *i);
void tex_presave(void);
void tex_preload(void);
void fsendstr(char *s, FILE *fout);
void text_gprint(int *in,int ilen);
void fftext_block(const string& s,double width,int justify);
void font_load(void);
void tex_init(void);
int g_font_fallback(int font);
void decode_utf8_basic(string& sc);

void do_prim(uchar **in, int *out, int *lout, TexArgStrs* params);

#define dbg if ((gle_debug & 1024)>0)
extern int gle_debug;
double text_endx,text_endy;

/*----------------------------------------------------------------------*/
/*              TeX Emulation routines                                  */
/*----------------------------------------------------------------------*/

#define FONTDEF extern
#include "font.h"
#include "tex.h"

double linegap;

/*----------------------------------------------------------------------*/

struct def_table_struct {
	struct def_table_struct *next;
	char *name;
	char *defn;
	int npm;
};
typedef struct def_table_struct deftable;

/*----------------------------------------------------------------------*/

bool tex_def(const char *name, const char *defn,int npm);
int tex_mathdef(const char *name, int defn);
int* tex_findmathdef(const char *s);
void tex_chardef(int c, const char *defn);
const char *tex_findchardef(int c);
deftable *tex_finddef(const char *s) ;

/*----------------------------------------------------------------------*/
/*              Global variables for TEX emulation                      */

int fontfam[16][4];
double fontfamsz[16][4];        /* 1=text,  2=script, 3=scriptscript */
int famdef = -1;        /* don't use unless/until it is defined */
char *cdeftable[256];   /* Character macros */
uchar chr_code[256];    /* Character codes 1..9  */
int chr_mathcode[256];  /* Character codes 1..9  */
int chr_init;           /* Flag to initialize chr variables */

#define dp if (dont_print==false)

int dont_print=0;
int gt_plen;
double gt_l,gt_r,gt_u,gt_d;

#define TEX_MAX_CHARS 100000

int gt_pbuff[TEX_MAX_CHARS];
uchar tbuff[TEX_MAX_CHARS];

int tofont[9] = {0,2,2,1,1,0,0,0,0};

int p_fnt;
double p_hei;
double grphei[10];
int grpfnt[10];
int p_ngrp;

double base_size;
int curstyle=6;

double stretch_factor=1;

double accent_x = 0.0;
double accent_y = 0.0;

IntStringHash m_Unicode;

union both {float f;int l;} bth;
#define outlong(v) *(out+((*lout)++)) = v
#define outfloat(v) bth.f = v; *(out+((*lout)++)) = bth.l
#define tolong(fff) ((bth.f = fff),bth.l )

float tofloat(int input) {
	union both {
		float f;
		int l;
	} bth;
	bth.l = input;
	return bth.f;
}

extern bool IS_INSTALL;
extern CmdLineObj g_CmdLine;

void set_stretch(double v) {
	stretch_factor = v;
}

void set_base_size() {
	g_get_hei(&base_size);
}

GLECoreFont* set_tex_font(int fnt) {
	p_fnt = g_font_fallback(fnt);
	return get_core_font_ensure_loaded(p_fnt);
}

void try_get_next_char(uchar **in, int *c1) {
	*c1 = 0;
	if (**in != 0) {
		/* char code */
		uchar code = chr_code[**in];
		if (code == 1 || code == 10) {
			*c1 = **in;
			(*in)++;
		}
	}
}

uchar try_get_next_two_chars(uchar **in, int *c1, int *c2) {
	if (**in == 0) {
		/* end of stream */
		return 0;
	}
	/* possibility that there is no second character */
	*c2 = 0;
	/* char code */
	*c1 = **in;
	uchar code = chr_code[**in];
	(*in)++;
	if (code == 6) {
		/* check for unicode character */
		if (str_ni_equals((const char*)*in, "UCHR{", 5)) {
			char *ptr;
			unsigned int unicode = strtol((const char*)(*in)+5, &ptr, 16);
			GLECoreFont* cfont = set_tex_font(p_fnt);
			int font_code = cfont->unicode_map(unicode);
			if (font_code != -1) {
				code = 1;
				*c1 = font_code;
				(*in) += 10;
			}
		}
	}
	if (code == 1 || code == 10) {
		/* normal character, also check second one */
		*c2 = **in;
		if (chr_code[**in] == 6) {
			/* check for unicode character */
			if (str_ni_equals((const char*)(*in)+1, "UCHR{", 5)) {
				char *ptr;
				unsigned int unicode = strtol((const char*)(*in)+6, &ptr, 16);
				GLECoreFont* cfont = set_tex_font(p_fnt);
				int font_code = cfont->unicode_map(unicode);
				if (font_code != -1) {
					*c2 = font_code;
				}
			}
		}
	}
	return code;
}

void text_topcode(uchar *in, int *out, int *lout) {
	int skip_space = false;
	float w;
	uchar c;
	int c1, c2;
	GLECoreFont* cfont;
	outlong(8);     /* set font size */
	outfloat(p_hei);
	TexArgStrs params;
	while ((c = try_get_next_two_chars(&in, &c1, &c2)) != 0) {
	  switch (c) {
	    case 10:
	    case 1: /* Normal character */
		/* if next char is normal, then check for ligature and kern */
norm_again:
		w = 0;
		cfont = set_tex_font(p_fnt);
		if (c2 != 0) {
			if (!g_CmdLine.hasOption(GLE_OPT_NO_LIGATURES)) {
				if (cfont->char_lig(&c1, c2)) {
					try_get_next_char(&in, &c2);
					goto norm_again;
				}
			}
			cfont->char_kern(c1, c2, &w);
			// cout << "kern(" << c1 << "," << c2 << ") = " << w << endl;
		}
		outlong(1);
		outlong(c1 | p_fnt*1024);
		dbg gprint("==char width %d %f %f \n", c1, cfont->getCharDataThrow(c1)->wx, w);
		outfloat((cfont->getCharDataThrow(c1)->wx + w)*p_hei);
		skip_space = false;
		break;
	    case 2: /* Single space */
		if (skip_space) break;
		skip_space = true;
		outlong(2);
		cfont = set_tex_font(p_fnt);
		outfloat(cfont->info.space*p_hei);
		outfloat(cfont->info.space_stretch*p_hei*10*stretch_factor);
		outfloat(cfont->info.space_shrink*p_hei*10);
		break;
	    case 3: /* Tab (for tabular) */
		break;
	    case 4: /* 8Tab (verbatim) */
		break;
	    case 5: /* \\ End of line */
		skip_space = false;
		outlong(5);
		outlong(0);             /* space for x,y to be put */
		outlong(0);
		break;
	    case 6: /* \ Primitive Command (macros already done) */
		skip_space = false;
		do_prim(&in,out,lout,&params);
		break;
	    case 7: /* { begin group */
		skip_space = false;
		grphei[++p_ngrp] = p_hei;
		grpfnt[p_ngrp] = p_fnt;
		break;
	    case 8: /* } end group */
		skip_space = false;
		if (p_ngrp<1) {
			gprint("%s\n",in);
			gprint("Too many end group brackets \n");
			return;
		}
		p_hei = grphei[p_ngrp];
		p_fnt = grpfnt[p_ngrp--];
		font_load_metric(p_fnt);

		outlong(8); outfloat(p_hei);
		break;
	    case 9: /* $^_ Macro (Done in macro expansion) */
		skip_space = false;
		break;
	    case 11: /* flag for end of paragraph */
			skip_space = false;
		outlong(10);
		outlong(0);     /* space for x,y to be put */
		outlong(0);
		break;
	    default:
		gprint("error, not valid character \n");
	  }
	}
}

uchar* cmdParam(uchar **inp, uchar **pm, int* pmlen, int npm) {
	int gcnt=0,i;
	uchar* save_inp = *inp; /* need this to look ahead but not move ahead */
	uchar* in = *inp;
	gcnt = 0;
	for (i=0;i<npm;i++) {
		pm[i] = in;
		pmlen[i] = 0;
		if (chr_code[*in]==7) { /* begin group */
			pm[i] = ++in;
			for (;*in!=0;in++) {
				if (chr_code[*in]==7) gcnt++;
				if (chr_code[*in]==8) {
					if (gcnt==0) break;
					gcnt--;
				}
			}
			pmlen[i] = in - pm[i];
			in++;
		} else {
			if (chr_code[*in]==6) { /* backslash look for non-alpha */
				pm[i] = ++in;
				if (isalpha(*pm[i])) {
					for (;*in!=0;in++) {
						if (!isalpha(*in)) {
							break;
						}
					}
					pmlen[i] = in - pm[i];
				} else {
					pm[i] = in;
					pmlen[i] = 1;
					in++;
				}
			} else {
				pm[i] = in;
				pmlen[i] = 1;
				in++;
			}
		}
	}
	*inp = (uchar*)in;
	return save_inp;
}

// used by topcode()
void text_box(const string& s,double width,int *tbuff, int *rplen) {
	int plen=0;
	char *workbuff;

	workbuff = (char*) myalloc(1000);
	if (s.length() == 0) return;
	if (chr_init == false) tex_init();
	text_tomacro(s,UC workbuff);
	plen = 0;
	if (width==0) width = 400;
	text_topcode(UC workbuff,tbuff,&plen);
	text_wrapcode(tbuff,plen,width);
	*rplen = plen;
	myfree(workbuff);
}

void topcode(const string& s, int slen, double width, int **pbuff, int *plen, double *l,double *r,double *u,double *d) {
	*pbuff = (int*) myalloc(1000);
	g_init_bounds();
	string sc = s.substr(0, slen);
	text_box(sc,width,*pbuff,plen);
	g_get_bounds(l,d,r,u);
	if (*l > *r) {*l=0; *r=0; *u=0; *d=0;}
}

#define p_sethei(hh) pp_sethei(hh,out,lout)
#define p_hfill(hh) pp_hfill(hh,out,lout)
#define p_move(x,y) pp_move(x,y,out,lout)
#define p_fntchar(ff,cc) pp_fntchar(ff,cc,out,lout)
#define p_mathchar(m) pp_mathchar(m,out,lout)
#define p_pcode(pbuff,plen) pp_pcode(pbuff,plen,out,lout)

void pp_move(double x, double y, int *out,int *lout) {
	outlong(4);
	outfloat(x);
	outfloat(y);
}

void pp_sethei(double h, int *out,int *lout) {
	outlong(8);
	outfloat(h);
	p_hei = h;
}

void pp_hfill(double h, int *out,int *lout) {
	outlong(2);
	outfloat(0.0);
	outfloat(h*p_hei);
	outfloat(h*p_hei);
}

void char_bbox_user(int p_fnt,int ix, double *x1,double *y1,double *x2,double *y2) {
	char_bbox(p_fnt,ix,x1,y1,x2,y2);
	*x1 *= p_hei;
	*x2 *= p_hei;
	*y1 *= p_hei;
	*y2 *= p_hei;
}

void pp_mathchar(int m, int *out, int *lout) {
	int mchar,mfam,mtyp;
	int ix;
	double x1,y1,x2,y2,reqhi,yc;
	double oldhei;
	oldhei = p_hei;
	mchar = m & 0xff;
	mfam = (m & 0xf00) / 0x100;
	mtyp = (m & 0xf000) / 0x1000;
	if (mtyp == 7  && famdef>=0) mfam = famdef;
	if (mtyp == 7) mtyp = 0;
	ix = 'b'; /* center on letter b */
	char_bbox_user(p_fnt,ix,&x1,&y1,&x2,&y2);
	reqhi = y2/2;
	p_sethei(fontfamsz[mfam][tofont[curstyle]] * p_hei);
	char_bbox_user(fontfam[mfam][tofont[curstyle]],mchar,&x1,&y1,&x2,&y2);
	yc = (y2-y1)/2;
	if (mtyp==1) pp_move(0,reqhi+yc-y2,out,lout);
	p_fntchar(fontfam[mfam][tofont[curstyle]],mchar);
	if (mtyp==1) pp_move(0,-(reqhi+yc-y2),out,lout);
	p_sethei(oldhei);
}

void mathchar_bbox(int m, double* x1, double* y1, double* x2, double* y2, double *cw) {
	int mchar,mfam,mtyp;
	mchar = m & 0xff;
	mfam = (m & 0xf00) / 0x100;
	mtyp = (m & 0xf000) / 0x1000;
	if (mtyp == 7  && famdef >= 0) mfam = famdef;
	if (mtyp == 7) mtyp = 0;
	int font = fontfam[mfam][tofont[curstyle]];
	char_bbox(font, mchar, x1, y1, x2, y2);
	*cw = fnt[font]->getCharDataThrow(mchar)->wx;
}

void pp_fntchar(int ff, int ch, int *out,int *lout) {
	ff = g_font_fallback(ff);
	outlong(1);
	if (ch == 0) ch = 254;
	outlong(ch | ff*1024);
	GLECoreFont* cfont = get_core_font_ensure_loaded(ff);
	outfloat(cfont->getCharDataThrow(ch)->wx * p_hei);
}

void pp_pcode(int *pbuff, int plen, int *out,int *lout) {
	int i;
	out += *lout;
	for (i=0;i<plen;i++) *out++ = *pbuff++;
	*lout = *lout + plen;
}

void p_unichar(const string& str, int *out, int *lout) {
	// cout << "unichar(" << str << ")" << endl;
	char *ptr;
 	string code;
	unsigned int unicode = strtol(str.c_str(), &ptr, 16);
	int res = m_Unicode.try_get(unicode, &code);
	if (res != 0) {
		char* workbuff = (char*)myalloc(1000);
		text_tomacro(code, UC workbuff);
		text_topcode(UC workbuff, out, lout);
		myfree(workbuff);
	} else {
		int i = 0;
		int myfnt = g_font_fallback(31);
		double savehei = p_hei;
		p_sethei(0.4*savehei);
		pp_move(0.0, 0.4*savehei, out, lout);
		GLECoreFont* cfont = get_core_font_ensure_loaded(myfnt);
		double crxpos = 0.0;
		while (str[i] != 0) {
			int ch = str[i];
			double wid = cfont->getCharDataThrow(ch)->wx * p_hei;
			switch (i) {
				case 0:
				case 1:
				case 3:
					break;
				case 2:
					pp_move(-crxpos, -0.4*savehei, out, lout);
					break;
			}
			pp_fntchar(myfnt, ch, out, lout);
			crxpos += wid;
			i++;
		}
		p_sethei(savehei);
	}
}

void TexArgStrs::cmdParam1(uchar **in) {
	uchar *s[2];
	int pmlen[2];
	cmdParam(in, s, pmlen, 1);
	str1.assign((char*)s[0], pmlen[0]);
}

void TexArgStrs::cmdParam12(uchar **in) {
	uchar *s[2];
	int pmlen[2];
	cmdParam(in, s, pmlen, 1);
	str2.assign((char*)s[0],pmlen[0]);
}

void TexArgStrs::cmdParam2(uchar **in) {
	uchar *s[3];
	int pmlen[3];
	cmdParam(in, s, pmlen, 2);
	str1.assign((char*)s[0],pmlen[0]);
	str2.assign((char*)s[1],pmlen[1]);
}

void TexArgStrs::cmdParam3(uchar **in) {
	uchar *s[4];
	int pmlen[4];
	cmdParam(in, s, pmlen, 3);
	str1.assign((char*)s[0],pmlen[0]);
	str2.assign((char*)s[1],pmlen[1]);
	str3.assign((char*)s[2],pmlen[2]);
}

void TexArgStrs::cmdParam4(uchar **in) {
	uchar *s[5];
	int pmlen[5];
	cmdParam(in, s, pmlen, 4);
	str1.assign((char*)s[0],pmlen[0]);
	str2.assign((char*)s[1],pmlen[1]);
	str3.assign((char*)s[2],pmlen[2]);
	str4.assign((char*)s[3],pmlen[3]);
}

void TexArgStrs::cmdParam4_swap34(uchar **in) {
	uchar *s[5];
	int pmlen[5];
	cmdParam(in, s, pmlen, 4);
	str1.assign((char*)s[0],pmlen[0]);
	str2.assign((char*)s[1],pmlen[1]);
	str4.assign((char*)s[2],pmlen[2]);
	str3.assign((char*)s[3],pmlen[3]);
}

void tex_get_char_code(uchar** in, int* code) {
	// parse the string "{number}"
	// and make in point to after the "}"
	string res;
	while (*(*in) != '}' && *(*in) != 0) {
		res += *(*in);
		(*in)++;
	}
	if (*(*in) == '}') {
		(*in)++;
	}
	texint((char*)(res.c_str()+1), code);
}

void tex_draw_accent(uchar **in, TexArgStrs* params, int *out, int *lout) {
	double lef,dep,wid,hei;
	double wid2,hei2,lef2,dep2,h=0,cwid,cwid2;
	int ix,ix2;
	int savefnt = p_fnt;
	int newfnt = pass_font(params->getCStr1());
	texint(params->str2,&ix);
	int *m = NULL;
	if (params->str3[0] != 0 && params->str3[1] != 0) {
		if (str_i_equals(params->str3, "CHAR")) {
			tex_get_char_code(in, &ix2);
		} else {
			m = tex_findmathdef(params->getCStr3());
			if (m == NULL) {
				gprint("Can't put accent on '%s'", params->getCStr3());
			} else {
				if (*(*in) == ' ') (*in)++;
			}
		}
	} else {
		ix2 = params->str3[0];
	}
	// cout << "'" << params->str1 << "' '" << params->str2 << "' '" << params->str3 << "'" << endl;
	// cout << *in << endl;
	char_bbox(newfnt, ix, &lef, &dep, &wid, &hei);
	cwid = p_hei * fnt[newfnt]->getCharDataThrow(ix)->wx;
	if (m == NULL) {
		char_bbox(p_fnt, ix2, &lef2, &dep2, &wid2, &hei2);
		cwid2 = p_hei * fnt[p_fnt]->getCharDataThrow(ix2)->wx;
	} else {
		mathchar_bbox(*m, &lef2, &dep2, &wid2, &hei2, &cwid2);
		cwid2 *= p_hei;
	}
	// Print regular character
	wid *= p_hei;  wid2 *= p_hei; hei *= p_hei; hei2 *= p_hei;
	lef *= p_hei;  dep *= p_hei;
	lef2 *= p_hei; dep2 *= p_hei;
	if (hei2>p_hei*(3.6/8.0)) h = hei2-p_hei*(3.6/8.0);
	if (m == NULL) {
		p_fntchar(p_fnt,ix2);
	} else {
		p_mathchar(*m);
	}
	// Print accent on top of it
	p_move(-cwid2+lef2+wid2/2-wid/2+accent_x,h+accent_y);  /* cwid2/2 - cwid/2 */
	p_fntchar(newfnt,ix);        /* cwid2/2 + cwid/2 */
	p_move(-cwid+cwid2-lef2-wid2/2+wid/2-accent_x,-h-accent_y);
	set_tex_font(savefnt);
}

void tex_draw_accent_cmb(uchar **in, TexArgStrs* params, int *out, int *lout) {
	if (params->str4.length() != 0 && params->str3.length() != 0) {
		if (params->str3.length() == 1) {
			int accent;
			int ch = params->str3[0];
			texint(params->str4, &accent);
			GLECoreFont* cfont = set_tex_font(p_fnt);
			FontCompositeInfo* info = cfont->get_composite_char(ch, accent);
			if (info != NULL) {
				double w1 = cfont->getCharDataThrow(info->c1)->wx * p_hei;
				double w2 = cfont->getCharDataThrow(info->c2)->wx * p_hei;
				double dx1 = info->dx1 * p_hei;
				double dx2 = info->dx2 * p_hei;
				double dy1 = info->dy1 * p_hei;
				double dy2 = info->dy2 * p_hei;
				p_move(dx1, dy1);
				p_fntchar(p_fnt, info->c1);
				p_move(dx2-dx1-w1, dy2-dy1);
				p_fntchar(p_fnt, info->c2);
				p_move(w1-w2-dx2, -dy2);
			} else {
				if (ch == 'i') params->str3[0] = 0x10;
				if (ch == 'j') params->str3[0] = 0x11;
				tex_draw_accent(in, params, out, lout);
			}
		} else {
			tex_draw_accent(in, params, out, lout);
		}
	}
}

int select_font_encoding(int font, int encoding, const char* defaultFont) {
	GLECoreFont* crfont = get_core_font_ensure_loaded(font);
	if (crfont->info.encoding == encoding) {
		return font;
	} else {
		return pass_font(defaultFont);
	}
}

void do_prim(uchar **in, int *out, int *lout, TexArgStrs* params) {
	int ci;
	int ix;
	uchar cmdstr[20];
	double lef,wid,hei,dep,savehei;
	int *pbuff=0;
	int plen;
	uchar *pmu[10];
	int pmlen[10];
	int *m,i,k,n,npm;

	k = 0;

	cmd_token(in,cmdstr);   /* finds command name and parameters */
	ci = find_primcmd((char*)cmdstr);

	if (ci==0) { /* then maybe it's a mathchar */
		m = tex_findmathdef((char*)cmdstr);
		if (m!=0) {
			p_mathchar(*m);
		} else {
			gprint("Unrecognised control sequence {%s} \n",cmdstr);
		}
		return;
	}

	switch (ci) {
	  case tp_sup: /* \superscript{exp} */
	  /*printf("l=%g r=%g u=%g d=%g\n",gt_l,gt_r,gt_u,gt_d);*/
		cmdParam(in,pmu,pmlen,1);
		savehei = p_hei;
		p_hei = p_hei * .7;
		topcode((char*)pmu[0],pmlen[0],0.0,&pbuff,&plen,&lef,&wid,&hei,&dep);
		/*g_measure(pm.s[0],&lef,&wid,&hei,&dep);
		fftext_block(UC pm.s[0],0.0,0);
		g_get_bounds(&lef,&wid,&hei,&dep);*/

		p_move(0,0.8*p_hei);
		p_pcode(pbuff,plen);
		/*printf("this P=[%s]\n",pm.s[0]);*/
		p_move(0,-0.8*p_hei);
		find_primcmd((char*)cmdstr);

		/* do a look ahead to see if there is a superscript */
		/* but keep in pointing to current spot */
		*in = cmdParam(in,pmu,pmlen,1);
		/* check to see if subscript is next */
		if(strncmp((char*)pmu[0],"sub ",4)==0) {
			/* move back to starting point */
			/*p_move(-wid,0);
			printf("subscript next P=[%s]\n",pm.s[0]);
			*/
		}
		myfree(pbuff);
		p_sethei(savehei);
		break;
	  case tp_sub: /* \subscript{exp} */
	  /*printf("l=%g r=%g u=%g d=%g\n",gt_l,gt_r,gt_u,gt_d);*/
		cmdParam(in,pmu,pmlen,1);
		savehei = p_hei;
		p_hei = p_hei * .7;
		topcode((char*)pmu[0],pmlen[0],0.0,&pbuff,&plen,&lef,&wid,&hei,&dep);
		/*printf("lef=%g wid=%g hei=%g dep=%g\n",lef,wid,hei,dep);*/
		p_move(0.0,-0.3*p_hei);
		p_pcode(pbuff,plen);
		/*printf("this B=[%s]\n",pm.s[0]);*/

		p_move(0,0.3*p_hei);
		/* do a look ahead to see if there is a superscript */
		/* but keep in pointing to current spot */
		*in = cmdParam(in,pmu,pmlen,1);
		/* check to see if superscript is next */
		if(strncmp((char*)pmu[0],"sup ",4)==0) {
			/* move back to starting point */
			/*p_move(-wid,0);*/
			/*printf("supescript next B=[%s]\n",pm.s[0]);*/
		}

		myfree(pbuff);
		p_sethei(savehei);
		break;
	  case tp_sethei: /* \sethei{exp} */
		params->cmdParam1(in);
		p_sethei(emtof(params->str1));
		break;
	  case tp_hfill: /* \sethei{exp} */
		p_hfill(10.0);
		break;
	  case tp_char:
		params->cmdParam1(in);
		texint(params->str1,&ix);
		p_fntchar(p_fnt,ix);
		break;
	  case tp_chardef:  /* \chardef{a}{xxxxx} */
		params->cmdParam2(in);
		tex_chardef(params->str1[0],params->getCStr2());
		break;
	  case tp_ssfont:
		k++;
	  case tp_sfont:
		k++;
	  case tp_tfont:  /* \tfont{0}{cmr10}{.5}  */
		params->cmdParam3(in);
		i = atoi(params->getCStr1());  if (i>15) i = 1;
		fontfam[i][k] = pass_font(params->getCStr2());
		fontfamsz[i][k] = emtof(params->str3);
		break;
	  case tp_fontenc:
		params->cmdParam2(in);
		set_tex_font(select_font_encoding(p_fnt, atoi(params->getCStr1()), params->getCStr2()));
		break;
	  case tp_acccmb:
  		params->cmdParam4_swap34(in);
		tex_draw_accent_cmb(in, params, out, lout);
	  	break;
	  case tp_accent:  /* accent{texcmr}{123}{a} */
		params->cmdParam3(in);
		tex_draw_accent(in, params, out, lout);
		break;
	  case tp_accentxy:
		params->cmdParam2(in);
		accent_x = emtof(params->str1);
		accent_y = emtof(params->str2);
		break;
	  case tp_unicode:
		params->cmdParam2(in);
		texint(params->str1,&ix);
		m_Unicode.add_item(ix, params->str2);
		break;
	  case tp_uchr:
		params->cmdParam1(in);
		p_unichar(params->str1, out, lout);
		break;
	  case tp_def:
		params->cmdParam1(in); /* finds everything up to the #1#2 */
		npm = 0;
		while (**in == '#') {
			(*in)++;
			n = (*(*in)++) - '0';
			if (n>0 && n<9) if (npm<n) npm=n;
		}
		params->cmdParam12(in);
		// cout << "def: '" << params->getCStr1() << "' -> '" << params->getCStr2() << "'" << endl;
		tex_def(params->getCStr1(),params->getCStr2(),npm);
		break;
	  case tp_mathchardef:  /* /mathchardef */
		params->cmdParam2(in);
		texint(params->str2,&ix);
		tex_mathdef(params->getCStr1()+1,ix);
		break;
	  case tp_movexy:
		params->cmdParam2(in);
		p_move(emtof(params->str1),emtof(params->str2));
		break;
	  case tp_rule:
		params->cmdParam2(in);
		outlong(6);
		outfloat(emtof(params->str1));
		outfloat(emtof(params->str2));
		break;
	  case tp_mathchar:
		params->cmdParam1(in);
		texint(params->str1,&ix);
		p_mathchar(ix);
		break;
	  case tp_mathcode:
		params->cmdParam2(in);
		texint(params->str2,&ix);
		chr_mathcode[(unsigned char)params->str1[0]] = ix;
		break;
	  case tp_delcode:
		params->cmdParam2(in);
		texint(params->str2,&ix);
		chr_mathcode[(unsigned char)params->str1[0]] = ix;
		break;
	  case tp_setfont:
		params->cmdParam1(in);
		set_tex_font(pass_font(params->getCStr1()));
		break;
	  case tp_presave:
		gprint("Saving definitions\n");
		tex_presave();
		break;
	  case tp_newline:
		outlong(5);
		outlong(0);             /* space for x,y to be put */
		outlong(0);
		break;
	  case tp_parskip:
		params->cmdParam1(in);
		set_parskip(emtof(params->str1));
		break;
	  case tp_setstretch:
		params->cmdParam1(in);
		set_stretch(emtof(params->str1));
		break;
	  case tp_lineskip:
		params->cmdParam1(in);
		set_lineskip(emtof(params->str1));
		break;
	  case tp_linegap:
		params->cmdParam1(in);
		linegap = emtof(params->str1);
		break;
	  case tp_tex:
		params->cmdParam1(in);
		outlong(11);
		outlong(TeXInterface::getInstance()->createObj(params->getCStr1(), p_hei));
	  	break;
	  case tp_frac:
	  case tp_delimiter:
	  case tp_left:
	  case tp_right:
	  case tp_defbegin:
	  case tp_nolimits:
	  case tp_overbrace:
	  case tp_overline:
	  case tp_underbrace:
	  case tp_underline:
	    gprint("A valid GLE-TEX primitive which isn't implemented yet %d \n",ci);
		break;
	  default:
	    gprint("An invalid GLE-TEX primitive %d \n",ci);
	    break;
	}
}

double emtof(char *s) {       /* same as ATOF but if it sees EM then it multiplies by FONT HEI */
	if (strstr(s,"sp")!=NULL) {
		GLECoreFont* cfont = set_tex_font(p_fnt);
		return atof(s) * cfont->info.space*p_hei;
	}
	if (strstr(s,"em")!=NULL) {
		return atof(s)*p_hei*0.75;
	}
	return atof(s);
}

double emtof(const string& s) {
	if (str_i_str(s, "sp") != -1) {
		GLECoreFont* cfont = set_tex_font(p_fnt);
		return atof(s.c_str()) * cfont->info.space*p_hei;
	}
	if (str_i_str(s, "em") != -1) {
		return atof(s.c_str())*p_hei*0.75;
	}
	return atof(s.c_str());
}

void texint(char *s, int *i) {
	int j;
	if (*s=='$') {
		sscanf(s+1,"%x",&j);
		*i = j;
	} else {
		*i = atoi(s);
	}
}

void texint(const string& s, int *i) {
	if (s[0] == '$') {
		sscanf(s.c_str()+1, "%x", i);
	} else {
		*i = atoi(s.c_str());
	}
}

/*
	Character Codes

	a..z            1       normal character
	space           2       single space
	<TAB>,&         3       Tab (as in tabular)
	<8TAB>          4       TAB (as in verbatim)
	<LF>,\\         5       End of line
	\               6       Macro or command
	{               7       Begin group
	}               8       end group
	$_^             9       Macro call
			10      null
	255             11      end of paragraph (crcr)
	end of command, = first non alpha character
*/

/*      Bounding Box ??

	1=char  font+char,x
	2=move  x,stret,shrink                  throw away after cr
	3=MOVE  x,0,0                           glue which has been set
	4=MOVE  x,y                             solid move
	5=newline,x,y
	6=rule  x,y
	10=color color
	8=fontsz   fontsz
	9=font   i
	15=null
*/

#define infloat(fff) ((bth.l = fff),bth.f)

void text_wrapcode(int *in,int ilen,double width) {
	double cx=0,cy=0,p_hei,ax=0,y,cdep=0,chei=0;
	int i,c,crFont,si,skline,saveii;
	double totstretch=0,totshrink=0,ls=0,gap=0,last_y,last_x,lastdep,last_stret,last_shrink;
	int *pcr=0,last_space=0;
	double setlen;
	bool eat_glue = false;
	dbg text_gprint(in,ilen);
	ls = 0;
	last_x = 0;
	gap = 0;
	last_y = 0;
	lastdep = 0;
	last_stret = 0;
	last_shrink = 0;
	GLECoreFont* cfont;
	dbg gprint("==wrap pcode, ilen = %d \n",ilen);
	dbg gprint("wrap pcode ilen=%d \n",ilen);
	p_hei = 1;
	si = 0;
	for (i=0;i<ilen;i++) {
	  switch (*(in+i)) {
	    case 1: /* char     font+char,wx    */
		eat_glue = false;
		crFont = g_font_fallback((*(in+ ++i)) / 1024);
		cfont = get_core_font_ensure_loaded(crFont);
		c = *(in+i) & 0x3ff;
		{
			GLEFontCharData* charData = cfont->getCharDataThrow(c);
			if (cdep > cy + p_hei * charData->y1)
				cdep = cy + p_hei * charData->y1;
			if (chei < cy + p_hei * charData->y2)
				chei = cy + p_hei * charData->y2;
		}
		/* gprint("chei=%f, cdep=%f \n",chei,cdep); */
		cx += tofloat(*(in+ ++i));
		ax = cx;
		if (cx>width) {
		  if (last_space>si) {
			dbg gprint("Call SET_GLUE  from %d, to %d \n",si,last_space);
			set_glue(in+si,last_space-si,last_x,width,last_stret,last_shrink,&setlen);
			i = last_space;
			*(in+i++) = 4;
			*(in+i++) = tolong(-setlen);
			if (pcr!=NULL) { /* put in last line feed now */
				y = last_y-ls;
				if ((y+chei+gap)>lastdep)
					y = lastdep-chei-gap;
				cy = y;
				*pcr = tolong(y);
			}
			font_get_lineskip(&ls,&gap);
			pcr = (in+i++);   /* place to put line feed */
			*(in+i) = 20;           /* null */
			last_stret = 0;
			last_shrink = 0;
			totstretch = 0;
			totshrink = 0;
			lastdep = cdep;
			last_y = cy;
			cx = 0;
			cy = 0;
			si = i;
			eat_glue = true;
		  }
		}
		break;
	    case 2: /* move     x,stretch,shrink */
		last_space = i;
		last_x = ax;
		last_y = cy;
		last_stret = totstretch;
		last_shrink = totshrink;
		if (eat_glue) {*(in+i)=3; *(in+ ++i)=tolong(0);i+=2;break;}
		cx += tofloat(*(in+ ++i));
		totstretch += tofloat(*(in+ ++i));
		totshrink += tofloat(*(in+ ++i));
		dbg gprint("total stretch %f, shrink %f \n",totstretch,totshrink);
		break;
	    case 3: /* move     x,0,0   SOLID   */
		cx += tofloat(*(in+ ++i));
		i += 2;
		ax = cx;
		eat_glue = false;
		break;
	    case 4: /* move     x,y     SOLID   */
		eat_glue = false;
		cx += tofloat(*(in+ ++i));
		cy += tofloat(*(in+ ++i));
		ax = cx;
		break;
	    case 5: /* Newline  x,y     (0,0 at moment) */
	    case 10:
		if (*(in+i)==5) skline = true; else skline = false;
		*(in+i) = 0;
/*              last_space = i;
		last_x = ax;
		last_y = cy;
		last_stret = totstretch;
		last_shrink = totshrink;
*/
		  if (last_space<=si || ax==cx) {
			last_x = ax;
			last_y = cy;
			last_stret = totstretch;
			last_shrink = totshrink;
			last_space = i;
		  }
			dbg gprint("Call SET_GLUE  from %d, to %d \n",si,last_space);
			set_glue(in+si,last_space-si,last_x,width,last_stret,last_shrink,&setlen);
			saveii = i;
			i = last_space;
			while (i < saveii) *(in+i++) = 20; /* nop */
			*(in+i++) = 4;
			*(in+i++) = tolong(-setlen);
			if (pcr!=NULL) { /* put in last line feed now */
				y = last_y-ls;
				if ((y+chei+gap)>lastdep)
					y = lastdep-chei-gap;
				cy = y;
				*pcr = tolong(y);
			}
			if (skline)
				font_get_lineskip(&ls,&gap);
			else    font_get_parskip(&ls,&gap);
			pcr = (in+i);   /* place to put line feed */
			last_stret = 0;
			last_shrink = 0;
			totstretch = 0;
			totshrink = 0;
			lastdep = cdep;
			last_y = cy;
			cx = 0;
			cy = 0;
			si = i+1;
			eat_glue = true;
			break;
/*              eat_glue = true;
		i += 2;
		break; */
	    case 11: /* TeX */
		{
			TeXInterface* iface = TeXInterface::getInstance();
			TeXHashObject* hobj = iface->getHashObject(*(in+ ++i));
			cx += hobj->getWidth();
		}
	    	break;
	    case 6: /* rule     x,y             */
		i += 2;
		eat_glue = false;
		break;
	    case 7: /* color    color           */
		g_set_color((int) tofloat(*(in+ ++i)));
		break;
	    case 8: /* fontsz   sz              */
		p_hei = tofloat(*(in+ ++i));
		g_set_hei(p_hei);
		break;
	    case 9: /* font     p_fnt   */
	    crFont = g_font_fallback(*(in+ ++i));
		font_load_metric(crFont);
		break;
	    case 20: /*  nop  */
		break;
	    default:
		gprint("dud pcode in wrap pcode %d   i=%d \n",*(in+i),i);
		break;
	  }
	}

	if (last_space==0) last_space = ilen;
	dbg gprint("Exiting call to SET_GLUE  from %d, to %d \n",si,last_space);
	set_glue(in+si,last_space-si,last_x,width,last_stret,last_shrink,&setlen);
	if (pcr!=NULL) { /* put in last line feed now */
		y = last_y-ls;
		if ((y+chei+gap)>lastdep)
			y = lastdep-chei-gap;
		cy = y;
		*pcr = tolong(y);
	}
	dbg text_gprint(in,ilen);
}

void set_glue(int *in,int ilen,double actual,double width,double stretch,double	shrink,double *setlen) {
	double mst=0,msh=0;
	float s1,s2,x;
	int i=0;

	dbg gprint("===set glue \n");
	dbg text_gprint(in,ilen);
	dbg gprint("set glue ilen=%d actual=%f, width=%f, stretch=%f shrink=%f \n"
			,ilen,actual,width,stretch,shrink);
/*      if (actual<0) get_natural(in,ilen,&actual); */
	if (actual<width) {
		if (stretch>0.0000001) mst = (width-actual)/stretch;
		msh = 0;
		if (mst>1) mst=0;
	} else {
		mst = 0;
		if (shrink>0) msh = (actual-width)/shrink;
		if (msh>1) msh=0;
	}
	*setlen = actual+stretch*mst+shrink*msh;
	dbg gprint("SETTing glue to  %f  %f  actual %f, setto %f\n",mst,msh,actual,*setlen);

	for (i=0;i<ilen;i++) {
	  switch (*(in+i)) {
	    case 1: /* char     font+char,wx    */
		i += 2;
		break;
	    case 2: /* move     x,stretch,shrink */
		x = tofloat(*(in+i+1));
		s1 = tofloat(*(in+i+2));
		s2 = tofloat(*(in+i+3));
		*(in+i) = 3;
		*(in+i+1) = tolong(x + s1*mst+s2*msh);
		i += 3;
		break;
	    case 3: /* move     x,0,0   SOLID   */
		i += 3;
		break;
	    case 4: /* move     x,y     SOLID   */
		i += 2;
		break;
	    case 5: /* Newline  x,y     (0,0 at moment) */
		i += 2;
		break;
	    case 6: /* rule     x,y             */
		i += 2;
		break;
	    case 7: /* color    color           */
		i += 1;
		break;
	    case 8: /* fontsz   sz              */
		i += 1;
		break;
	    case 9: /* font     p_fnt   */
		i += 1;
		break;
	    case 10: /* Newparagraph x,y        (0,0 at moment) */
		i += 2;
		break;
	    case 11: /* TeX */
	    	i += 1;
		break;
	    case 20: /*  nop  */
		break;
	    default:
		gprint("dud (in set glue) pcode in text pcode %d i=%d\n",*(in+i),i);
		break;
	  }
	}
	dbg printf("=== Result after setting \n");
	dbg text_gprint(in,ilen);
	dbg printf("===+++++ END OF SET GLUE  =============== \n");
}

void text_draw(int *in,int ilen) {
	double cx,cy,p_hei,x,y;
	int i,c,crFont;
	GLECoreFont* cfont;

	dbg gprint("---TEXT DRAW, ilen = %d \n",ilen);
	dbg text_gprint(in,ilen);
	cx = 0;
	cy = 0;
	dp g_get_xy(&cx,&cy);
	dbg printf("Current x y, %g %g \n",cx,cy);
	p_hei = 1;

	for (i=0;i<ilen;i++) {
	  switch (*(in+i)) {
	    case 1: /* char     font+char,wx    */
	    crFont = g_font_fallback((*(in+ ++i)) / 1024);
		cfont = get_core_font_ensure_loaded(crFont);
		c = *(in+i) & 0x3ff;
		{
			GLEFontCharData* cdata = cfont->getCharDataThrow(c);
			g_update_bounds(cx+p_hei*(cdata->x1), cy+p_hei*(cdata->y1));
			g_update_bounds(cx+p_hei*(cdata->x2), cy+p_hei*(cdata->y2));
		}
		dp {
			g_move(cx,cy);
			g_char(crFont,c);
		}
		cx += tofloat(*(in+ ++i));
		break;
	    case 2: /* move     x,stretch,shrink */
		cx += tofloat(*(in+ ++i));
		i += 2;         /* glue is already set */
		break;
	    case 3: /* move     x,0,0   SOLID   */
		cx += tofloat(*(in+ ++i));
		i += 2;
		break;
	    case 4: /* move     x,y     SOLID   */
		cx += tofloat(*(in+ ++i));
		cy += tofloat(*(in+ ++i));
		break;
	    case 5: /* Newline  x,y     (turned into a move) */
		i += 2;
		break;
	    case 6: /* rule     x,y             */
		x = tofloat(*(in+ ++i));
		y = tofloat(*(in+ ++i));
		g_update_bounds(cx,cy);
		g_update_bounds(cx+x,cy+y);
		if (x>0) g_box_fill(cx,cy,cx+x,cy+y);
		break;
	    case 7: /* color    color           */
/*              dp g_set_color(tofloat(*(in+ ++i)));*/
		break;
	    case 8: /* fontsz   sz              */
		p_hei = tofloat(*(in+ ++i));
		g_set_hei(p_hei);
		break;
	    case 9: /* font     p_fnt   */
	    crFont = g_font_fallback(*(in+ ++i));
		font_load_metric(crFont);
		break;
	    case 10: /* Newline  x,y    (turned into a move) */
		i += 2;
		break;
	    case 11: /* TeX */
		{
			TeXObjectInfo info;
			info.setPosition(cx, cy);
			info.setJustify(JUST_LEFT);
			if (dont_print) info.setFlag(TEX_OBJ_INF_DONT_PRINT);
			TeXInterface* iface = TeXInterface::getInstance();
			TeXHashObject* hobj = iface->getHashObject(*(in+ ++i));
			iface->drawObj(hobj, info);
			cx += hobj->getWidth();
		}
	    case 20: /*  nop  */
		break;
	    case 0:
		dbg gprint("zero");
		break;
	    default:
		gprint("dud3 pcode in text pcode %d %d \n",*(in+i),i);
		break;
	  }
	}
	text_endx = cx;
	text_endy = cy;
	dbg gprint("---TEXT DRAW, DONE. %g %g \n",cx,cy);
}

double tex_xend(void) {
	return text_endx;
}

double tex_yend(void) {
	return text_endy;
}

void text_gprint(int *in,int ilen) {
	int i,c,crFont;
	for (i=0;i<ilen;i++) printf("%x ",*(in+i));
	printf("\n");
	printf("# ");
	double x = 0.0;
	for (i=0;i<ilen;i++) {
	  switch (*(in+i)) {
	    case 1: /* char     font+char,wx    */
	    crFont = g_font_fallback((*(in+ ++i)) / 1024);
		font_load_metric(crFont);
		c = *(in+i) & 0x3ff;
		x = tofloat(*(in+ ++i));
		printf("%c[%3.3f]",c,x);
	/*      printf("%c{%d %3.3f} ",c,p_fnt,tofloat(*(in+ ++i)));  */
		break;
	    case 2: /* move     x,stretch,shrink */
		printf("[sp %3.3f %3.3f %3.3f] \n# ",tofloat(*(in+1+i))
				,tofloat(*(in+2+i)),tofloat(*(in+3+i)));
		i += 3;
		break;
	    case 3: /* move     x,0,0   SOLID   */
		printf("(3 %3.3f %3.3f %3.3f) \n# ",tofloat(*(in+1+i))
				,tofloat(*(in+2+i)),tofloat(*(in+3+i)));
		i += 3;
		break;
	    case 4: /* move     x,y     SOLID   */
		printf("(4 %3.3f %3.3f) \n# ",tofloat(*(in+1+i))
				,tofloat(*(in+2+i)));
		i += 2;
		break;
	    case 5: /* Newline  x,y     (turned into a move) */
		i += 2;
		printf("5 \n# ");
		break;
	    case 6: /* rule     x,y             */
		printf("(rule %3.3f %3.3f) \n# ",tofloat(*(in+1+i))
				,tofloat(*(in+2+i)));
		i += 2;
		break;
	    case 7: /* color    color           */
		printf("(color %x) \n# ",(*(in+ ++i)));
		break;
	    case 8: /* fontsz   sz              */
		printf("(p_hei %3.3f) \n# ",tofloat(*(in+ ++i)));
		break;
	    case 9: /* font     p_fnt   */
		printf("(font %d) \n",(*(in+ ++i)));
		break;
	    case 10: /* Newline  x,y    (turned into a move) */
		i += 2;
		printf("\n10(paragraph)\n# ");
		break;
	    case 20: /*  nop  */
		printf("NOP ");
		break;
	    default:
		printf("(err=%4x pos=%d)\n ",*(in+i),i);
		break;
	  }
	}
	printf("\n");
}

double lineskip1,parskip1;

void font_reset_parskip() {
	lineskip1 = 1.0;
	parskip1 = 2.5;
}

void set_parskip(double v) {
	parskip1 = v;
}

void set_lineskip(double v) {
	lineskip1 = v;
}

void font_get_lineskip(double *ls,double *gap) {
	*ls = p_hei * lineskip1;
	*gap = *ls * .1 + linegap;
}

void font_get_parskip(double *ls,double *gap) {
	*ls = p_hei * parskip1;
	*gap = *ls * .1;
}

#define get_exps(ss) polish(ss,(char *) pcode,plen,&etype)
#define tok(n)  tk[n]

const char* get_font_name(int idx) {
	GLECoreFont* cfont = get_core_font(idx);
	return cfont->name;
}

int get_nb_fonts() {
	return fnt.size();
}

void text_block(const string& s,double width,int justify) {
	text_block(s, width, justify, 0);
}

void text_block(const string& s, double width, int justify, int innerjust) {
	double ox,oy,x,y,ll,rr,uu,dd;
	double a,b,c,d;
	set_base_size();
	g_get_bounds(&a,&b,&c,&d);
	g_init_bounds();
	dont_print = true;
	fftext_block(s,width,justify);
	dont_print = false;
	g_get_bounds(&ll,&dd,&rr,&uu);
	if (ll > rr) {ll=0; rr=0; uu=0; dd=0;}
	g_get_xy(&ox,&oy);
	x = ox; y = oy;
	g_dotjust(&x,&y,ll,rr,uu,dd,justify);
	g_move(x,y);
	g_init_bounds();
	if (a<=c) {
		g_update_bounds(a,b);
		g_update_bounds(c,d);
	}
	g_get_bounds(&a,&b,&c,&d);
	text_draw(gt_pbuff,gt_plen);
	g_get_bounds(&a,&b,&c,&d);
	g_move(ox,oy);
}

/* Searches CMD and replaces #n's with parameters */
char *tex_replace(char *cmd,char *pm[],int pmlen[],int npm) {
	char *r,*s,*o;
	int n;

/*      printf("replace {%s} \n",cmd);
	for (i=0;i<npm;i++) {
		printf("pm[%d] = {%s} %d \n",i,pm[i],pmlen[i]);
	}
*/
	if (strchr(cmd,'#')==0) return sdup(cmd);
	r = (char*) myalloc(1000);
	o = r;
	for (s=cmd; *s!=0; s++) {
		if (*s=='#') {
			n = *(++s) - '0';
			if (n>0 && n<=npm) {
				strncpy(o,pm[n-1],pmlen[n-1]);
				o += pmlen[n-1];
			}
		} else *o++ = *s;
	}
	*o++ = 0;
	return r;
}

/*----------------------------------------------------------------------*/
/*              tex_chardef                                             */
/*----------------------------------------------------------------------*/

void tex_chardef(int c, const char *defn) {
	if (c < 0 || c > 255) return;
	if (cdeftable[c] != NULL) myfree(cdeftable[c]);
	cdeftable[c] = sdup(defn);
}

const char* tex_findchardef(int c) {
	if (c < 0 || c > 255) return "";
	else return cdeftable[c];
}

/*----------------------------------------------------------------------*/
/*              Hashing table code for \def                             */
/*----------------------------------------------------------------------*/

#define HASHSIZE 101

static deftable  *def_hashtab[HASHSIZE];

unsigned hash_str(const char *s) {
	unsigned hashval;

	for (hashval=0; *s != 0; s++)
		hashval = *s + 31*hashval;
	return hashval % HASHSIZE ;
}

deftable *tex_finddef(const char *s) {
	deftable  *np;

	for (np = def_hashtab[hash_str(s)]; np != NULL; np = np->next)
		if (strcmp(s, np->name)==0) {
			return np;
		}
	return NULL;
}

bool tex_def(const char *name, const char *defn, int npm) {
	deftable  *np;
	unsigned hashval;
	// cout << "define '" << name << "' -> '" << defn << "'" << endl;
	if ((np = tex_finddef(name)) == NULL) { /* not found */
		np = (deftable  *) myalloc(sizeof(*np));
		if ((np == NULL) || (np->name = sdup(name)) == NULL)
			return false;
		hashval = hash_str(name);
		np->next = def_hashtab[hashval];
		def_hashtab[hashval] = np;
		np->npm = npm;
		if ((np->defn = sdup(defn)) == NULL) return false;
	} else {
		myfree(np->defn);
		if ((np->defn = sdup(defn)) == NULL) return false;
	}
	return true;
}

/*----------------------------------------------------------------------*/
/*              Hashing table code for \mathchardef                     */
/*----------------------------------------------------------------------*/

struct mdef_table_struct {
	struct mdef_table_struct  *next;
	char *name;
	int defn;
};
typedef struct mdef_table_struct mdeftable;

static mdeftable  *mdef_hashtab[HASHSIZE];

int* tex_findmathdef(const char *s) {
	mdeftable* np;
	for (np = mdef_hashtab[hash_str(s)]; np != NULL; np = np->next)
		if (strcmp(s, np->name)==0) {
			return &np->defn;
		}
	return NULL;
}

int tex_mathdef(const char *name, int defn) {
	mdeftable  *np;
	int *d;
	unsigned hashval;

	if ((d = tex_findmathdef(name)) == NULL ) { /* not found */
		np = (mdeftable  *) myalloc(sizeof(*np));
		if ((np == NULL) || (np->name = sdup(name)) == NULL)
			return 0;
		hashval = hash_str(name);
		np->next = mdef_hashtab[hashval];
		mdef_hashtab[hashval] = np;
		np->defn = defn;
	} else {
		*d = defn;
	}
	return true;
}

void tex_init() {
	for (int i = 0; i<256; i++) chr_code[i]=10;     /* other */
	for (int i = 65; i<91; i++) chr_code[i]=1;      /* alpha */
	for (int i = 97; i<123; i++) chr_code[i]=1;
	for (int i = 0; i < HASHSIZE; i++) mdef_hashtab[i] = NULL;
	chr_code[0] = 2; /* maybe should be 0,  this is only tested in lig*/
	chr_code[(unsigned int)' '] = 2;
	chr_code[9] = 2;
	chr_code[(unsigned int)'\n'] = 2;
	chr_code[(unsigned int)'\\'] = 6;
	chr_code[(unsigned int)'{'] = 7;
	chr_code[(unsigned int)'}'] = 8;
	chr_code[255] = 11;     /* flag for end of paragraph, unless verbatim */
	chr_init = true;
	tex_preload();
	tex_def(" ","\\movexy{1sp}{}",0);
	tex_def("\\","\\newline{}",0);
	tex_def("{","\\char{123}",0);
	tex_def("}","\\char{125}",0);
	tex_def("_","\\char{95}",0);
	tex_def("^","\\acccmb{texcmr}{94}{4}",0);
	tex_def("$","\\char{36}",0);
}

void tex_clear() {
	/* clear CHARDEF table before each redraw */
	tex_term();
	tex_chardef('^',"\\sup ");
	tex_chardef('_',"\\sub ");
}

void tex_term() {
	for (int c = 0; c < 256; c++) {
		if (cdeftable[c] != NULL) {
			myfree(cdeftable[c]);
			cdeftable[c] = NULL;
		}
	}
}

void g_measure(const string& s, double *l, double *r, double *u, double *d) {
	double sa,sb,sc,sd;
	g_get_bounds(&sa,&sb,&sc,&sd);
	set_base_size();
	g_init_bounds();
	dont_print = true;
	fftext_block(s,0.0,0);
	dont_print = false;
	g_get_bounds(l,d,r,u);
	if (*l > *r) {*l=0; *r=0; *u=0; *d=0;}
	gt_l = *l;
	gt_r = *r;
	gt_u = *u;
	gt_d = *d;
	g_init_bounds();
	if (sa>sc) return;
	g_update_bounds(sa,sb);
	g_update_bounds(sc,sd);
}

void g_textfindend(const string& s, double *cx, double *cy) {
	double sa,sb,sc,sd;
	set_base_size();
	g_get_bounds(&sa,&sb,&sc,&sd);
	dont_print = true;
	fftext_block(s,0.0,0);
	*cx = text_endx;
	*cy = text_endy;
	dont_print = false;
	g_init_bounds();
	if (sa>sc) return;
	g_update_bounds(sa,sb);
	g_update_bounds(sc,sd);
}

void g_jtext(int just) {
	double ox,oy,x,y;
	g_get_xy(&ox,&oy);
	x = ox; y = oy;
	g_dotjust(&x,&y,gt_l,gt_r,gt_u,gt_d,just);
	g_move(x,y);
	text_draw(gt_pbuff,gt_plen);
	g_move(ox,oy);
}

void text_def(uchar *s) {
	gt_plen = 0;
	if (chr_init==false) tex_init();
	text_topcode(s,gt_pbuff,&gt_plen);
}

void fftext_block(const string& s,double width,int justify) {
	g_get_font(&p_fnt);
	font_load_metric(p_fnt);
	g_get_hei(&p_hei);

	font_reset_parskip();

	gt_plen = 0;
	if (s.length() == 0) {
		/* gprint("TEXT_BLOCK, Passed empty string \n"); */
		return;
	}
	if (chr_init == false) {
		tex_init();
	}
	string sc = s;
	// flag end of paragraph with 255 (octal 377)
	decode_utf8_notex(sc);
	str_replace_all(sc, "\n\n", "\n\377");
	text_tomacro(sc, tbuff);
	gt_plen = 0;
	if (width == 0.0) {
		width = 400;
		chr_code[(unsigned int)'\n'] = 5;
	} else {
		chr_code[(unsigned int)'\n'] = 2;
	}
	text_topcode(tbuff,gt_pbuff,&gt_plen);
	text_wrapcode(gt_pbuff,gt_plen,width);
	text_draw(gt_pbuff,gt_plen);

	g_set_font(p_fnt);
	g_set_hei(p_hei);
}

void cmd_token(uchar **in, uchar *cmdstr) {
	int i = 0;
	if ( (!isalpha(**in)) && (**in != 0)) {
		if ((*in)[0] == '\'' && (*in)[1] == '\'') {
			// special case for two subsequent single quotes
			*cmdstr++ = *(*in)++;
			*cmdstr++ = *(*in)++;
		} else {
			*cmdstr++ = *(*in)++;
		}
	} else {
		for (; chr_code[**in]==1 && **in != 0 && i<20;(*in)++,i++)  {
			*cmdstr++ = **in;
		}
	}
	*cmdstr = 0;
	cmdstr -= 1;
	if (chr_code[*cmdstr]==1) {
		// if last character is alpha, then eat all subsequent space
		// (chr_code alpha = 1, space = 2)
		for (;(**in != 0) && (chr_code[**in]==2);) (*in)++;
	}
}

void  text_tomacro(const string& in, uchar *out) {
	/* find /cmdname  or defined characters */
	uchar macroname[30];
	uchar *s,*dfn,*r,*saves;
	int dlen;
	int nrep;
	deftable  *np;
	uchar* pmu[10];
	int pmlen[10];
	nrep = 0;
	strcpy(SC out,SC in.c_str());
	// Replace macros
	for (s=out; *s != 0;s++)  {
		// cout << "current: '" << s << "'" << endl;
		if (nrep>300) gle_abort("Loop in text macros\n");
		if (chr_code[*s]==6) {        /* backslash, beginning of macro? */
			saves = s;
			s++;
			cmd_token(&s,macroname);
			// cout << "macro: " << macroname << endl;
			np = tex_finddef((char*)macroname);
			if (np != NULL) {
				// cout << "found!" << endl;
				nrep++;
				dfn = UC np->defn;
				dbg printf("Found macro {%s} = {%s} \n",macroname,dfn);
				cmdParam(&s,pmu,pmlen,np->npm);
				dlen = s-saves;
				r = UC tex_replace(SC dfn,(char**)pmu,pmlen,np->npm);
				s = saves;
				memmove(SC s+strlen(SC r),SC s+dlen,strlen(SC s)+1);
				strncpy(SC s,SC r,strlen(SC r));
				myfree(r);
				s--;
			}
			s = saves;
			if (strcmp((char*)macroname, "tex")==0) {
				// do not expand macros inside \tex{} expression
				s = UC str_skip_brackets((char*)s, '{', '}');
			}
			if (strcmp((char*)macroname, "unicode")==0) {
				// do not expand macros inside \unicode{}{} expression
				s = UC str_skip_brackets((char*)s, '{', '}');
				if (s[0] == '}') s++;
				s = UC str_skip_brackets((char*)s, '{', '}');
			}
			if (strcmp((char*)macroname, "def")==0) {
				// make sure not to expand macros in macro name
				// otherwise, during a second pass, the name will be replaced by the value
				// e.g., \def\a{b} will turn into: \defb{b{}
				s = UC str_find_char((const char*)s, '{');
			}
		}
		if (cdeftable[*s]!=0) {
			dbg printf("Found char definition %d  {%s} \n",*s,s);
			nrep++;
			dfn = UC tex_findchardef(*s);
			memmove(s+strlen(SC dfn)-1,s,strlen(SC s)+1);
			strncpy(SC s,SC dfn,strlen(SC dfn));
			s--;
		}
	}
	// cout << "result of macros: '" << out << "'" << endl;
}

void tex_presave() {
	int i;
	deftable *dt;
	GLEFileIO fout;
	mdeftable *mdt;
	/* Save all defined features possible */
	string fname = gledir("inittex.ini");

	fout.open(fname.c_str(), "wb");
	if (!fout.isOpen()) gprint("Could not create inittex.ini file \n");
	fout.fwrite(fontfam, sizeof(int), 16*4);
	fout.fwrite(fontfamsz, sizeof(double), 16*4);
	fout.fwrite(chr_mathcode, sizeof(char), 256);
	for (i=0;i<HASHSIZE;i++) {
	  for (dt = def_hashtab[i]; dt != NULL; dt = dt->next) {
		fout.fwrite(&i, sizeof(i), 1);
		fout.fwrite(&dt->npm, sizeof(i), 1);
		fout.fsendstr(dt->name);
		fout.fsendstr(dt->defn);
	  }
	}
	i = 0x0fff; fout.fwrite(&i, sizeof(i), 1);
	for (i=0;i<HASHSIZE;i++) {
	  for (mdt = mdef_hashtab[i]; mdt != NULL; mdt = mdt->next) {
		fout.fwrite(&i, sizeof(i), 1);
		fout.fwrite(&mdt->defn, sizeof(i), 1);
		fout.fsendstr(mdt->name);
	  }
	}
	i = 0x0fff; fout.fwrite(&i, sizeof(i), 1);
	for (i=0;i<256;i++) fout.fsendstr(cdeftable[i]);
	/* write unicode definitions */
	for (IntStringHash::const_iterator it = m_Unicode.begin(); it != m_Unicode.end(); it++ ) {
		int key = it->first;
		const string& data = it->second;
		int len = data.size();
		fout.fwrite(&key, sizeof(int), 1);
		fout.fwrite(&len, sizeof(int), 1);
		fout.fwrite(data.c_str(), sizeof(char),len);
	}
	i = 0;
	fout.fwrite(&i, sizeof(int), 1);
	fout.close();
}

void fgetvstr(char** s, GLEFileIO* fmt) {
	int i = fmt->fgetc();
	if (i == 0) return;
	if (*s != NULL) myfree(*s);
	*s = (char*) myalloc(i+1);
	fmt->fread(*s, 1, i);
	*(*s+i) = 0;
}

void tex_preload() {
	int i,j;
	GLEFileIO fout;
	char str1[80],str2[80];
	/* reload all defined features */
#ifdef DO_TEX_DECODE
	FILE* fhv;
	fhv = fopen("decode.tex","w");
#endif
	string fname = gledir("inittex.ini");
	fout.open(fname.c_str(), "rb");
	if (!fout.isOpen()) {
		if (!IS_INSTALL) gprint("Could not open inittex.ini file \n");
		return;
	}
	fout.fread(fontfam, sizeof(int), 16*4);
	fout.fread(fontfamsz, sizeof(double), 16*4);
	fout.fread(chr_mathcode, sizeof(char), 256);
#ifdef DO_TEX_DECODE
	for(k=0;k<=16;k++){
		fprintf(fhv,"*fontfam[%d]=%s  fontfamsz[%d]=%s\n",k,fontfam[k],k,fontfamsz[k]);
	}
	fprintf(fhv,"chr_mathcode=%s\n",chr_mathcode);
#endif
	for (; fout.fread(&i, sizeof(i), 1), i != 0x0fff;) {
		fout.fread(&j, sizeof(j), 1);
		fout.fgetcstr(str1);
		fout.fgetcstr(str2);
		tex_def(str1,str2,j);
#ifdef DO_TEX_DECODE
		fprintf(fhv,"str1=%s  str2=%s j=%d\n",str1,str2,j);
#endif
	}
	i=0;
	for (; fout.fread(&i, sizeof(i), 1), i != 0x0fff;) {
		fout.fread(&j, sizeof(j), 1);
		fout.fgetcstr(str1);
		tex_mathdef(str1,j);
#ifdef DO_TEX_DECODE
		fprintf(fhv,"str1=%s  i=%d\n",str1,i);
#endif
	}
	i=0;
	for (i=0;i<256;i++){
		fgetvstr(&cdeftable[i], &fout);
#ifdef DO_TEX_DECODE
		fprintf(fhv,"cdeftable[%d]=%s\n",i,cdeftable[i]);
#endif
	}
	/* load unicode definitions */
	m_Unicode.clear();
	int key;
	fout.fread(&key, sizeof(int), 1);
	char* read_ptr = NULL;
	int read_ptr_len = 0;
	while (key != 0) {
		int len;
		fout.fread(&len, sizeof(int), 1);
		if (len >= read_ptr_len) {
			read_ptr_len = 2*read_ptr_len + len + 1;
			read_ptr = (char*)realloc(read_ptr, read_ptr_len);
		}
		fout.fread(read_ptr, sizeof(char), len);
		read_ptr[len] = 0;
		m_Unicode.add_item(key, read_ptr);
		fout.fread(&key, sizeof(int), 1);
	}
	if (read_ptr != NULL) free(read_ptr);
	fout.close();
#ifdef DO_TEX_DECODE
	fclose(fhv);
#endif
}

/*
    0 0000  1 0001  2 0010  3 0011  4 0100  5 0101  6 0110
    7 0111  8 1000  9 1001  a 1010  b 1011  c 1100  d 1101
    e 1110  f 1111

    Char. number range  |        UTF-8 octet sequence
       (hexadecimal)    |              (binary)
    --------------------+---------------------------------------------
    0000 0000-0000 007F | 0xxxxxxx
    0000 0080-0000 07FF | 110xxxxx 10xxxxxx
    0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx
    0001 0000-0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
*/

int decode_utf8_byte(const string& sc, int len, int pos) {
	if (pos >= len) return -1;
	unsigned char ch = sc[pos];
	if ((ch & 0xC0) != 0x80) return -1;
	ch &= 0x3F;
	return ch;
}

void decode_utf8_remove(string& sc, int* len, int pos, int nb) {
	if (pos+nb <= *len) {
		sc.erase(pos, nb);
		(*len) = (*len) - nb;
	}
}

void decode_utf8_add_unicode(int unicode, string& sc, int* len, int pos, int nb) {
	string code;
	char hexcode[10];
	sprintf(hexcode, "%.4X", unicode);
	code = "\\uchr{";
	code += hexcode;
	code += "}";
	decode_utf8_remove(sc, len, pos, nb+1);
	sc.insert(pos, code);
	(*len) += code.length();
}

void decode_utf8_notex(string& sc) {
	int prev = 0;
	int ps = str_i_str(sc, 0, "\\TEX{");
	if (ps != -1) {
		string buffer;
		while (ps != -1) {
			int end_ps = str_skip_brackets(sc, ps, '{', '}') + 1;
			string first_part(sc, prev, ps - prev);
			decode_utf8_basic(first_part);
			buffer += first_part;
			buffer += string(sc, ps, end_ps - ps);
			ps = str_i_str(sc, end_ps, "\\TEX{");
			prev = end_ps;
		}
		if (prev+1 <= (int)sc.length()) {
			string first_part(sc, prev);
			decode_utf8_basic(first_part);
			buffer += first_part;
		}
		sc = buffer;
	} else {
		decode_utf8_basic(sc);
	}
}

void decode_utf8_basic(string& sc) {
	int i = 0;
	int len = sc.length();
	while (i < len) {
		unsigned char ch = sc[i];
		if ((ch & 0x80) != 0) {
			/* highest bit is one - 3 possibilities */
			if ((ch & 0xE0) == 0xC0) {
				/* 110x -> two byte unicode */
				ch &= 0x1F;
				int val = decode_utf8_byte(sc, len, i+1);
				if (val == -1) {
					sc[i] = '?';
				} else {
					int unicode = ch;
					unicode = unicode << 6;
					unicode += val;
					decode_utf8_add_unicode(unicode, sc, &len, i, 1);
				}
				i++;
			} else if ((ch & 0xF0) == 0xE0) {
				/* 1110x -> three byte unicode */
				ch &= 0x0F;
				int val1 = decode_utf8_byte(sc, len, i+1);
				int val2 = decode_utf8_byte(sc, len, i+2);
				if (val1 == -1 || val2 == -1) {
					sc[i] = '?';
				} else {
					int unicode = ch;
					unicode = unicode << 6;
					unicode += val1;
					unicode = unicode << 6;
					unicode += val2;
					decode_utf8_add_unicode(unicode, sc, &len, i, 2);
				}
				i += 2;
			} else if ((ch & 0xF8) == 0xF0) {
				/* 1111x -> four byte unicode */
				ch &= 0x07;
				int val1 = decode_utf8_byte(sc, len, i+1);
				int val2 = decode_utf8_byte(sc, len, i+2);
				int val3 = decode_utf8_byte(sc, len, i+3);
				if (val1 == -1 || val2 == -1 || val3 == -1) {
					sc[i] = '?';
				} else {
					int unicode = ch;
					unicode = unicode << 6;
					unicode += val1;
					unicode = unicode << 6;
					unicode += val2;
					unicode = unicode << 6;
					unicode += val3;
					decode_utf8_add_unicode(unicode, sc, &len, i, 3);
				}
				i += 3;
			} else {
				sc[i] = '?';
				i++;
			}
		} else {
			/* regular ascii 7 bit character */
			i++;
		}
	}
}

