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

int fner(char *s);
int scr_manip_refresh(void);
int scr_getch(void);

char *function_bar() {
	return "F11-Help  F12-Save  F13-Load  F14-Save as        F9-Graph Form  F10-Drawit";
}

struct escape_struct {char *str; int val;};

struct escape_struct gold[] = {
		"v", 	epaste,
		"R",	esearch,
		"S",	eundeleol,
		NULL,	};

struct escape_struct eseq[] = {
		"4~", 	eselect,
		"1~",	esearch,
		"2~",	epaste,
		"3~",	ecut,
		"4~",	eselect,
		"5~",	epageup,
		"6~",	epagedown,
		"A",	eup,
		"B",	edown,
		"C",	eright,
		"D",	eleft,
		"20~",	egraphmenu,
		"21~",	edrawit,
		"23~",	ehelp,
		"24~",	esave,
		"25~",	eload,
		"26~",	esaveas,
		"P",	egold,
		"Q",	ehelp,
		"R",	efindnext,
		"S",	edeleol,
		"n",	eselect,
		"v",	ecut,
		"l",	edelright,
		NULL,	};

int gold_fn[] = {
	edrawit,ehelp,esave,eload,esaveas,eshowerror,0,0,0,egraphmenu,edrawit
};
struct keymatch {int m; int val;};
/* Normal key and ^ commands  commands */
struct keymatch kmatch2[] = {
	13, ereturn,
	3, equit,
	4, eword,
	5, eedt,
	6, efast,
	7, edrawit,
	18, eshowerror,
	8, ehelp,
	20, etrace,
	12, efindnext,
	21, eundelline,
	25, edelline,
	26, eescape,
	27, eescape,
	127, edelete,
	0,0
};
/* Control K commands */
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
	0,0
};
/* Control Q commands */
struct keymatch kmatch4[] = {
	'f', esearch,
	'c', eendoffile,
	'r', etopoffile,
	0,0
};

text_inkey()
{
	int cc,i,c1,c2;

	scr_manip_refresh();

loop1:	cc = tt_inkey();
	c2 = (cc & 0xff);
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
		if (c2==egold) {
			c2 = tt_inkey();
			if (c2==27) {
				tt_inkey();
				return escape_seq_gold();
			} else if (isdigit(c2)) {
				return gold_fn[c2-'0'];
			}
		}
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
escape_seq()
{
	int cc,i;
	unsigned char esq[10];
	char *s;

	s = &esq[0];
	*s++ = cc = tt_inkey();
	while (cc<65) *s++ = cc = tt_inkey();
	*s++ = 0;
	for (i=0;eseq[i].str!=NULL;i++)
		if (strcmp(eseq[i].str,esq)==0) break;
	if (eseq[i].str!=NULL)
		return eseq[i].val;
	else
		return 0;

}
escape_seq_gold()
{
	int cc,i;
	unsigned char esq[10];
	char *s;

	s = &esq[0];
	*s++ = cc = tt_inkey();
	while (cc<65) *s++ = cc = tt_inkey();
	*s++ = 0;
	for (i=0;gold[i].str!=NULL;i++)
		if (strcmp(gold[i].str,esq)==0) break;
	if (gold[i].str!=NULL)
		return gold[i].val;
	else
		return 0;

}


#include <descrip.h>
#include <iodef.h>
int tt_chan;
short tt_iosb[4];
int tt_inkey();
key_open()
{
	int st;
	static $DESCRIPTOR(tt_desc,"tt");
	st = SYS$ASSIGN(&tt_desc,&tt_chan,0,0);
}
int tt_inkey()
{
	int read_mask,st;
	unsigned char inbuff[9];
	static int key_isopen;

	if (!key_isopen) {key_open(); key_isopen=true;}
	read_mask = IO$_READLBLK | IO$M_NOFILTR | IO$M_NOECHO;
	st = SYS$QIOW(0,tt_chan,read_mask,tt_iosb,0,0,inbuff,1,0,0,0,0);
	if ((st & 1) != 1) LIB$SIGNAL(st);
	if ((tt_iosb[0] & 1) != 1) LIB$SIGNAL(tt_iosb[0]);
	return inbuff[0];
}
