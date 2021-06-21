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

/*----------------------------------------------------------------------*/
/* This routine prints a char from one of my fonts, it has cacheing 	*/
/* for the char data, which could be extended to bitmap cacheing 	*/
/*----------------------------------------------------------------------*/

#include "all.h"
#include "core.h"
#include "color.h"
extern struct gmodel g;
#include "gprint.h"
#include "file_io.h"

#ifdef BIGINDIAN
#define BYTE0 1
#define BYTE1 0
#else
#define BYTE0 0
#define BYTE1 1
#endif

void font_file_vector(int ff,char *filename);
int draw_char_pcode(char *s);
void my_load_font(int ff);
int char_plen(char *s);
void get_char_pcode(int ff, int cc, char **pp);
double frx(char **s);


unsigned char my_name[80];
char *my_code[80];
int my_ref[80];
char *my_buff;
int my_font[80];
int my_pnt[257];
int my_curfont;
extern double font_lwidth;

void my_char(int ff, int cc)
{
	char *pp;
	GLEWithoutUpdates noUpdates;
	get_char_pcode(ff, cc, &pp);
	draw_char_pcode(pp);
}

void get_char_pcode(int ff, int cc, char **pp)
{
	int i,plen,mi,minref;

/* y:	Is char in font cache, if so draw it, inc ref */
	/*  should use *memchr(s,c,n)  */
	for (i=1;i<80;i++) {
		if (my_name[i]==cc) {
			if (my_font[i]==ff) {
				my_ref[i]++;
				*pp = my_code[i];
				return;
			}
		}
	}

/* x:	Is that font currently loaded, if so put in cache at least used, goto y */
	minref = 30000;
	mi = 0;
	if (my_curfont!=ff) my_load_font(ff);
	/* Find least used cache character */
	for (i=1;i<80;i++)  {
		if (my_ref[i]<minref) {minref=my_ref[i]; mi = i;}
	}
	if (mi==0) mi=1;
	plen = char_plen(my_buff+my_pnt[cc]);
	if (my_code[mi]==0) my_code[mi] = (char*) myallocz(plen+1);
	else		{
	/*	my_code[mi] = realloc(my_code[mi],plen+1); */
		myfree(my_code[mi]);
		my_code[mi] = (char*) myalloc((plen+1));
	}
	if (my_code[mi]==0) gprint("Memory allocation failure, in myfont.c \n");
	memcpy(my_code[mi],my_buff+my_pnt[cc],plen+1);
	*pp = my_code[mi];
	my_name[mi] = cc;
	my_ref[mi] = 1;
	my_font[mi] = ff;
}

void my_load_font(int ff)
{
	char vector_file[60];
	font_file_vector(ff,vector_file);
	string fontfname = fontdir(vector_file);
	GLEFileIO fin;
	fin.open(fontfname.c_str(), READ_BIN);
	if (!fin.isOpen()) {
		ostringstream err;
		err << "font vector file not found: '" << fontfname << "'; using texcmr instead";
		g_message(err.str().c_str());
		/* Replace font by texcmr instead */
		font_replace_vector(ff);
		font_file_vector(ff, vector_file);
		fontfname = fontdir(vector_file);
		fin.open(fontfname.c_str(), READ_BIN);
		if (!fin.isOpen()) {
			gle_abort("Font vector texcmr.fve not found\n");
		}
	}
	fin.fread(my_pnt, sizeof(int), 256);
	/* gprint("Size of rest of font %d \n",my_pnt[0]); */
	if (my_buff==0) my_buff = (char*) myallocz(10 + my_pnt[0]);
	else 		{
		myfree(my_buff);
		my_buff = (char*) myallocz(10 + my_pnt[0]);
	}
	if (my_buff==0) gprint("Memory allocation failure MY_BUFF , in myfont.c \n");
	fin.fread(my_buff, 1, my_pnt[0]);
	fin.close();
	my_curfont = ff;
}

int frxi(char **s);

int char_plen(char *s) {
	char *savelen;
	savelen = s;
	while (*s!=15) {
	  switch (*s++) {
	    case 1:
	    case 2:
	    case 9:
	    	frxi(&s); frxi(&s);
	    	break;
	    case 3:
	    	frxi(&s); frxi(&s);
	    	frxi(&s); frxi(&s);
	    	frxi(&s); frxi(&s);
	    	break;
	    case 4:
	    case 5:
	    case 6:
	    case 7:
	    case 8:
	    	break;
	    case 10:
	    	frxi(&s);
	    	break;
	    case 0: /* char does not exist */
	    	goto abort;
	    default:
	    	gprint("Error in mychar pcode %d \n",*s++);
	    	goto abort;
	  }
	}
abort:
	return s-savelen;
}

int draw_char_pcode(char *s)
{
	static double cx,cy,ox,oy,x1,y1,x2,y2;
	char *savelen;
	double old_lwidth;
	int old_path, old_join;

	g_get_path(&old_path);
	GLERC<GLEColor> old_color(g_get_color());
	GLERC<GLEColor> old_fill(g_get_fill());
	g_set_fill(old_color);
	g_get_line_width(&old_lwidth);
	g_set_line_width(font_lwidth);
	g_get_line_join(&old_join);
	g_set_line_join(1);	/* use rounded lines to avoid ucky peeks */

	g_get_xy(&ox,&oy);
	savelen = s;
	if (!old_path) {
		g_set_path(true);
		g_newpath();
	}
	while (*s!=15) {
	  switch (*s++) {
	    case 1:
	    	cx = ox + frx(&s); cy = oy + frx(&s);
	    	g_move(cx,cy);
	    	break;
	    case 2:
	    	cx = cx + frx(&s); cy = cy + frx(&s);
	    	g_line(cx,cy);
	    	break;
	    case 3:
	    	cx = cx + frx(&s); cy = cy + frx(&s);
	    	x1 = cx; y1 = cy;
			cx = cx + frx(&s); cy = cy + frx(&s);
			x2 = cx; y2 = cy;
			cx = cx + frx(&s); cy = cy + frx(&s);
			g_bezier(x1,y1,x2,y2,cx,cy);
			break;
	    case 4:
	    	g_closepath();
	    	break;
	    case 5:
	    	if (!old_path) g_fill();
	    	break;
	    case 6:
	    	g_stroke();
	    	break;
	    case 7:
	    	g_gsave();
	    	g_set_fill(GLE_COLOR_WHITE);
	    	g_fill();
	    	g_grestore();
	    	break;
	    case 8:
	    	g_set_line_width(frx(&s));
	    	break;
	    case 9:
	    	cx = ox + frx(&s); cy = oy + frx(&s);
	    	g_set_pos(cx,cy);
	    	break;
	    case 10:
	    	g_circle_stroke(frx(&s));
	    	break;
	    case 0:         /* no such char in this font */
	    	goto abort;
	    default:
	    	gprint("Error in mychar pcode %d \n",*s++);
	    	goto abort;
	  }
	}
abort:	if (!old_path) g_set_path(old_path);
	g_set_line_join(old_join);
	g_set_line_width(old_lwidth);
	g_set_color(old_color);
	g_set_fill(old_fill);
	return s-savelen;
}

double frx(char **s)
{
	static union {char a[2];short b;} both;
	static int i;

	if (g.fontsz==0) {
		gprint("Font size is zero ***\n");
		g.fontsz = 1;
	}
	i = *(*s)++;
	if (i==127) {
		both.a[0] = (*(*s)++);
		both.a[1] = (*(*s)++);
		if (1==2) printf("bothb %d \n",both.b);
		return (g.fontsz*both.b)/1000.0;
	} else {
	        if (i>127) i = -(256-i);
		if (1==2) printf("ii %d \n",i);
		return (g.fontsz*i)/1000.0;
	}
}
int frxi(char **s)
{
	static union {char a[2];short b;} both;
	static int i;

	i = *(*s)++;
	if (i==127) {
		both.a[0] = (*(*s)++);
		both.a[1] = (*(*s)++);
		if (1==2) printf("both %d \n",both.b);
		return (both.b);
	} else {
	        if (i>127) i = -(256-i);
		if (1==2) printf("i %d \n",i);
		return i;
	}
}
