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

delay(int i)
{}
int vx_top=1,vx_bot=24;
textattr()
{}
kbhit()
{}
scr_gets(char *x)
{
	getstr(x);
}
clreol()
{
	clrtoeol();
}
cputs(const char *line)
{
	int x;
	int y;
	getyx(stdscr,y,x);
	mvaddstr(y,x,line);
}
delline()
{
	int x;
	int y;
	getyx(stdscr,y,x);
	move(22,1);
	clrtobot();
	move(y,1);
	deleteln();
	move(y,x);
}
gotoxy(int x, int y)
{
	if (y==25) y=24;
	move(y+vx_top-2,x);
}
insline()
{
	int x;
	int y;
	getyx(stdscr,y,x);
	move(y,x);
	insertln();
	move(y,x);
	move(22,1);
	clrtobot();
	move(y,x);

}
putch(int char_val)
{
	int x;
	int y;
	getyx(stdscr,y,x);
	mvaddch(y,x,char_val);
}
int scr_manip_refresh()
{
	manip_refresh();
}
int scr_getch()
{
	return getch();
}
manip_scr_init()
{
	initscr();
	scrollok(stdscr,true);
}
scr_end()
{
	endwin();
}
textbackground(int color_num)
{}
textcolor(int colornum)
{}
gettextinfo(struct text_info *r)
{
	int x;
	int y;
	getyx(stdscr,y,x);
	r->curx = x;
	r->cury = y;
	r->wintop = vx_top;
}
screen_save()
{}
screen_restore()
{
	scr_norm();
	clrscr();
	gotoxy(1,1);
	cputs("\n");
}
int wyerr;
w_message(char *s)
{
	wyerr++;
	scr_savexy();
	gotoxy(1,wyerr);
	clreol();
	cputs(s);
	scr_restorexy();
}
window(int left,int top, int right, int bottom)
{
	if (left==1 && top==1 && bottom==25) {
		printf("\x1b[%d;%dr",1,24);
	}
	vx_top = top;
	vx_bot = bottom;
	wyerr = 0;
}
clrscr()
{
	if (vx_top==1 && vx_bot==25) {
		clearok(stdscr,TRUE);
		clear();
		manip_refresh();
		clearok(stdscr,FALSE);
		return;
	}
	clear();
}
scr_dots(int i)
{
}
scr_left(int i)
{
	int y,x;
	if (i<=0) return;
	getyx(stdscr,y,x);
	move(y,x-i);
}
scr_right(int i)
{
	int y,x;
	if (i<=0) return;
	getyx(stdscr,y,x);
	move(y,x+i);
}
int vx_topsave,vx_botsave;
int savex,savey;
scr_savexy()
{
	getyx(stdscr,savey,savex);
	vx_topsave = vx_top;
	vx_botsave = vx_bot;

}
scr_restorexy()
{
	move(savey,savex);
	vx_top = vx_topsave;
	vx_bot = vx_botsave;
}

scr_norm()  /* yellow on blue */
{
	clrattr(_BOLD | _REVERSE);
}
scr_inv()   /* black on white */
{
	clrattr(_REVERSE);
	setattr(_BOLD);
}
scr_grey()  /* black on grey */
{
	clrattr(_BOLD);
	setattr(_REVERSE);
}
scr_isblackwhite()
{
	return true;
}
scr_menubg()
{
	scr_norm();
}
scr_menuval()
{
	scr_inv();
}
scr_menuhi()
{
	scr_grey();
}

#include <descrip.h>
vax_edt(char *s) 	/* call the vax EDT editor */
{
	$DESCRIPTOR(sdesc,"");
	sdesc.dsc$a_pointer = s;
	sdesc.dsc$w_length = strlen(s);
	edt$edit(&sdesc,&sdesc);
}
