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
#include "edt.h"

#ifdef NOKEYPAD
#include "nofnkeys.h"
#endif

int do_fnkey(void);
int do_gold(void);
int escape_seq(void);
int fner(char *s);
int scr_getch(void);
const char *function_bar()
{
  static int ab;
  if (ab==0) ab=1;
  ab = 3-ab;
  if (ab==1) {
	#ifndef LINUXFKEY /* a.r.: only F1-F10 on AT keyboards */
	return "F11-Help F12-Save F13-Load F14-Saveas F9-Graphmenu F10-Drawit ^x=Exit";
	#else
	return "F1-Help F2-Save F3-Load F4-Saveas F9-Graphmenu F10-Drawit ^x=Exit";
	#endif
      } else {
	return "^F 1 =Help, ^F 2 = Save, ^F 3 = Load,      ^F 9 Graphmenu ^F 0 Drawit";
      }
}

struct escape_struct {const char *str; int val;};

struct escape_struct gold[] = {
		{ "v", 	epaste     },
		{ "R",	esearch    },
		{ "S",	eundeleol  },
		{ NULL, 0          } };

#ifndef LINUXFKEY
struct escape_struct eseq[] = {
	{ "4~", eselect },
	{ "1~",	esearch },
	{ "2~",	epaste },
	{ "3~",	ecut },
	{ "4~",	eselect },
	{ "5~",	epageup },
	{ "6~",	epagedown },
	{ "A",	eup },
	{ "B",	edown },
	{ "C",	eright },
	{ "D",	eleft },
	{ "20~", egraphmenu },
	{ "21~", edrawit },
	{ "23~", ehelp },
	{ "24~", esave },
	{ "25~", eload },
	{ "26~", esaveas },
	{ "P",	egold },
	{ "Q",	ehelp },
	{ "R",	efindnext },
	{ "S",	edeleol },
	{ "n",	eselect },
	{ "v",	ecut },
	{ "l",	edelright },
	{ NULL,	0} };
#else /* a.r.: X11 escape sequences for F-keys were added */
struct escape_struct eseq[] = {
	{ "2~",	epaste },
	{ "5~",	epageup },
	{ "6~",	epagedown },
	{ "A",	eup },
	{ "B",	edown },
	{ "C",	eright },
	{ "D",	eleft },
	{ "20~", egraphmenu },
	{ "21~", edrawit },
	{ "23~", ehelp },
	{ "11~", ehelp },
	{ "24~", esave },
	{ "12~", esave },
	{ "25~", eload },
	{ "13~", eload },
	{ "26~", esaveas },
	{ "14~", esaveas },
#ifndef MANIP
	{ "15~", eshowerror },
#endif
	{ "P",	egold },
	{ "Q",	ehelp },
	{ "R",	efindnext },
	{ "S",	edeleol },
	{ "n",	eselect },
	{ "v",	ecut },
	{ "l",	edelright },
	{ "3~",	edelright },
	{ "1~",	ebol },
	{ "4~",	eeol },
	{ NULL,	0 } };
#endif
int gold_fn[] = {
	edrawit,ehelp,esave,eload,esaveas,eshowerror,0,0,0,egraphmenu,edrawit
};
struct keymatch {int m; int val;};
/* Normal key and ^ commands  commands */
#ifndef LINUXFKEY /* a.r. */
struct keymatch kmatch2[] = {
	{ 13, ereturn },
	{ 3, equit },
	{ 4, eword },
	{ 5, eedt },
	{ 6, efast },
	{ 7, edrawit },
	{ 8, edelete },
	{ 18, eshowerror },
	{ 8, ehelp },
	{ 20, etrace },
	{ 12, efindnext },
	{ 21, eundelline },
	{ 24, eescape },
	{ 25, edelline },
	{ 26, eescape },
	{ 27, eescape },
	{ 127, edelete },
	{ 0,0 }
};
#else
struct keymatch kmatch2[] = {
	{ 13, ereturn },
	{ 3, equit },
/*	23, eword,	eword does not exist! */
	{ 1, ebol }, /* a.r. bol and eol like EMACS and Joe */
	{ 5, eeol },
	{ 4, edelright, /*  delete the char at the cursor-position */
	{ 6, efast },
	{ 7, edrawit },
	{ 8, edelete },
	{ 18, eshowerror },
	{ 8, ehelp },
	{ 20, etrace },
	{ 12, efindnext },
	{ 21, eundelline },
	{ 24, eescape },
	{ 25, edelline },
	{ 26, eescape },
	{ 27, eescape },
	{ 127, edelete },
	{ 0,0 }
};
#endif
struct keymatch kmatchx[] = {
	{ KEY_DOWN, edown },
	{ KEY_UP, eup },
	{ KEY_F0+4, edeleol },
	{ KEY_LEFT, eleft },
	{ KEY_RIGHT, eright },
	{ KEY_NPAGE, epagedown },
	{ KEY_PPAGE, epageup },
	{ 0,0 }
};
/* Control K commands */
#ifndef LINUXFKEY	/* a.r. */
struct keymatch kmatch3[] = {
	{ 'b', eselect },
	{ 'v', emove },
	{ 'k', emark },
	{ 'c', ecopy },
	{ 'y', ecut },
	{ 'u', epaste },
	{ 'p', epaste },
	{ 'r', eblockread },
	{ 'w', eblockwrite },
	{ 'm', egraphmenu },
	{ 'l', eload },
	{ 'd', edrawit },
	{ 's', esave },
	{ 'x', equit },
	{ 0,0 }
};
#else
struct keymatch kmatch3[] = {
	'b', eselect,
	'v', emove,
	'k', emark,
	'c', ecopy,
	'y', ecut,
	'u', epaste,
	'p', epaste,
	'r', eblockread,
	'w', eblockwrite,
	'm', egraphmenu,
	'l', eload,
	'd', edrawit,
	's', esave,
	'x', equit,
	'f', esearch, /* like Joe's Own Editor */
	0,0
};
#endif

/* Control Q commands */
struct keymatch kmatch4[] = {
	{ 'f', esearch },
	{ 'c', eendoffile },
	{ 'r', etopoffile },
	{ 0,0 }
};
extern int noscreenio;
int tt_inkey()
{
	int i;
	if (noscreenio) return getc(stdin);
	else {
		i = getch();
	        /* printw("{%d} ",i); */
		return i;
	}
}

int text_inkey()
{
	int cc,i,c2;

	scr_refresh();

loop1:	cc = tt_inkey();
	c2 = cc;
	if (c2==6) return do_fnkey();
	if (c2==KEY_F(1)) return do_gold();
	if (cc>KEY_BREAK) {
	    for (i=0;kmatchx[i].m!=0;i++)
		if (kmatchx[i].m==c2) return kmatchx[i].val;
	}
	switch(c2) {
	  default:
	    for (i=0;kmatch2[i].m!=0;i++)
		if (kmatch2[i].m==c2) return kmatch2[i].val;
	    break;
	  case 27:
		c2 = tt_inkey(); /* throw away next char (unless escape) */
		if (c2==27) return eescape;
	  case -101:
	  case -113:
		c2 = escape_seq();
		if (c2==egold) 	return do_gold();
		return c2;
		break;
	  case 17:
	    fner("^Q  F=Find string,  R=Top of file");
	    cc = tt_inkey();
	    c2 = (cc & 0xff);
	    if (c2<32) c2 = c2 + 'a' - 1;
	    c2 = tolower(c2);
	    for (i=0;kmatch4[i].m!=0;i++)
		if (kmatch4[i].m==c2) return kmatch4[i].val;
	    fner("Unrecognized Quick movement command");
	    goto loop1;
	  case 11:
	    fner("^K  B=begin block,  P=Paste,  (use KP6 for Cut),  K=End block");
	    cc = tt_inkey();
	    c2 = (cc & 0xff);
	    if (c2<32) c2 = c2 + 'a' - 1;
	    c2 = tolower(c2);
	    for (i=0;kmatch3[i].m!=0;i++)
		if (kmatch3[i].m==c2) return kmatch3[i].val;
	    fner("Unrecognized block command");
	    goto loop1;
	}
	return c2;
}
int escape_seq(void)
{
	int cc,i;
	unsigned char esq[10];
	unsigned char *s;

	s = &esq[0];
	*s++ = cc = tt_inkey();
	while (cc<65) *s++ = cc = tt_inkey();
	*s++ = 0;
	for (i=0;eseq[i].str!=NULL;i++)
		if (strcmp(eseq[i].str,(char*)esq)==0) break;
	if (eseq[i].str!=NULL)
		return eseq[i].val;
	else
		return 0;

}
int escape_seq_gold()
{
	int cc,i;
	unsigned char esq[10];
	unsigned char *s;

	s = &esq[0];
	*s++ = cc = tt_inkey();
	while (cc<65) *s++ = cc = tt_inkey();
	*s++ = 0;
	for (i=0;gold[i].str!=NULL;i++)
		if (strcmp(gold[i].str,(char*)esq)==0) break;
	if (gold[i].str!=NULL)
		return gold[i].val;
	else
		return 0;

}
int do_fnkey(void)
{
  int c2;
  fner("1=Help 2=Save 3=Load 4=Saveas 9=Graph_menu     0=Drawit");
  c2 = tt_inkey();
  if  (isdigit(c2)) {
       	return gold_fn[c2-'0'];
  }
  return -1;
}
int do_gold(void)
{
	int c2;
	c2 = tt_inkey();
	if (c2==KEY_F(4)) return eundeleol;
	if (c2==27) {
		tt_inkey();
		return escape_seq_gold();
	} else if (isdigit(c2)) {
		return gold_fn[c2-'0'];
	}
	return -1;
}


