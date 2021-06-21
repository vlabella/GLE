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

#include <signal.h>

#include "vaxconio.h"
#define dbg if (1==1)

#define NORET (int) 0
extern int noscreenio;

void delay(int i) {
	sleep(i);
}

int vx_top=1,vx_bot=24;

void textattr() {
}

bool abort_key() {
	return false;
}

bool kbhit() {
	return false;
}

void scr_gets(char *x) {
	getstr(x);
}

int clreol() {
	if (noscreenio) return NORET;
	clrtoeol();
	return 0;
}

int cputs(const char *line) {
	int x;
	int y;
	if (noscreenio) return NORET;
	getyx(stdscr,y,x);
	mvaddstr(y,x,line);
	return 0;
}

void delline() {
	int x;
	int y;
	getyx(stdscr,y,x);
	move(22,1);
	clrtobot();
	move(y,1);
	deleteln();
	move(y,x);
}

int gotoxy(int x, int y) {
	if (noscreenio) return NORET;
	if (y==25) y=24;
	if (y<1) y=1;
	move(y+vx_top-2,x);
	return 0;
}

void insline() {
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

void putch(int char_val) {
	int x;
	int y;
	getyx(stdscr,y,x);
	mvaddch(y,x,char_val);
}

void scr_refresh() {
	if (!noscreenio) refresh();
}

int scr_getch() {
	if (noscreenio)
           return getc(stdin);
	else
           return getch();
}

void ctrlhandler() {
	fprintf(stderr,"ctrlhandler!\n");
	exit(1);
}

void trap() {
	echo();
	nl();
	nocbreak();
	endwin();
	exit(1);
}

void manip_scr_init(char * dummy) {
	static int doneinit = false;
#if (defined ultrix || defined aix)
	signal(SIGINT,trap);
#endif
	if (doneinit) { printf("init called twice \n"); exit(0);}
	doneinit = true;
	initscr();
	scrollok(stdscr,true);
#ifdef __UNIX__
	if (signal(SIGINT, SIG_IGN) == SIG_ERR) {
		fprintf(stderr,"Unable to disable ctrl-c\n");
	}
	noecho();
	nonl();
	cbreak();
	clear();
// The AIX R6000 goes looney if you set keypad to be true
#ifndef NOKEYPAD
	keypad(stdscr,TRUE);
#endif
#endif
}

int scr_end(void) {
	if (noscreenio) return NORET;
	echo();
	nl();
	nocbreak();
	endwin();
	return 0;
}

void textbackground(int color_num) {
}

void textcolor(int colornum) {
}

int gettextinfo(struct text_info *r) {
	int x;
	int y;
	if (noscreenio) return NORET;
	getyx(stdscr,y,x);
	r->curx = x;
	r->cury = y;
	r->wintop = vx_top;
	return 0;
}

void screen_save() {
}

int screen_restore() {
	if (noscreenio) return NORET;
	scr_norm();
	clrscr();
	gotoxy(1,1);
	cputs("\n");
	return 0;
}

int wyerr;

int w_message(char *s) {
	wyerr++;
	if (noscreenio) return NORET;
	scr_savexy();
	gotoxy(1,wyerr);
	clreol();
	cputs(s);
	scr_restorexy();
	return 0;
}

int window(int left,int top, int right, int bottom) {
	if (left==1 && top==1 && bottom==25) {
		if (noscreenio) return NORET;
	}
	vx_top = top;
	vx_bot = bottom;
	wyerr = 0;
	return 0;
}

int clrscr() {
	if (noscreenio) return NORET;
	if (vx_top==1 && vx_bot==25) {
		clearok(stdscr,TRUE);
		clear();
		refresh();
		clearok(stdscr,FALSE);
		return NORET;
	}
	clear();
	return 0;
}

void scr_dots(int i) {
}

int scr_left(int i) {
	int y,x;
	if (i<=0) return NORET;
	getyx(stdscr,y,x);
	move(y,x-i);
	return 0;
}

int scr_right(int i) {
	int y,x;
	if (i<=0) return NORET;
	getyx(stdscr,y,x);
	move(y,x+i);
	return 0;
}

int vx_topsave,vx_botsave;
int savex,savey;

int scr_savexy() {
	if (noscreenio) return NORET;
	getyx(stdscr,savey,savex);
	vx_topsave = vx_top;
	vx_botsave = vx_bot;
	return 0;
}

int scr_restorexy() {
	if (noscreenio) return NORET;
	move(savey,savex);
	vx_top = vx_topsave;
	vx_bot = vx_botsave;
	return 0;
}

int scr_norm() {
#ifndef NOATTRIB
	attrset(A_NORMAL);
#endif
	return 0;
}

int scr_inv() {
#ifndef NOATTRIB
	attrset(A_BOLD);
#endif
	return 0;
}

int scr_grey() {
#ifndef NOATTRIB
	attrset(A_REVERSE);
#endif
	return 0;
}

int scr_isblackwhite() {
	return true;
}

void scr_menubg() {
	scr_norm();
}

void scr_menuval() {
	scr_inv();
}

void scr_menuhi() {
	scr_grey();
}

int vax_edt(char *s) {
	return -1;
}


