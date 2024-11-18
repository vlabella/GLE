#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <conio.h>
#include <signal.h>
#include <float.h>
#define MANIP

#ifndef EMXOS2                  /* a.r. */
   #include <bios.h>
#else
   #include <sys/video.h>
   #include <sys/winmgr.h>
   #define textattr v_attrib
   int vx_top = 1, vx_bot = 25;
#endif

#define false 0
#define true (!false)


#ifndef EMXOS2                  /* a.r. */
   #define BCOLOR BLUE
   #define FCOLOR MAGENTA
   #define HBCOLOR h_bcolor
   #define HVCOLOR h_fcolor
   #define VCOLOR WHITE
#else

enum COLORS {
		BLACK,
		BLUE,
		GREEN,
		CYAN,
		RED,
		MAGENTA,
		BROWN,
		LIGHTGRAY,
		DARKGRAY,
		LIGHTBLUE,
		LIGHTGREEN,
		LIGHTCYAN,
		LIGHTRED,
		LIGHTMAGENTA,
		YELLOW,
		WHITE
};

#define BCOLOR B_BLUE
#define FCOLOR F_MAGENTA
#define HBCOLOR h_bcolor
#define HVCOLOR h_fcolor
#define VCOLOR B_WHITE

void insline(void)
{
	v_insline(1);
}
void delline(void)
{
	v_delline(1);
}

clrscr(void)
{
/*        printf("\033[2J\033[H");*/
	v_clear();
}
void normvideo(void)
{
	printf("\033[0m");
}
void highvideo(void)
{
	printf("\033[1m");
}
void lowvideo(void)
{
	printf("\033[0m");
}
void textbackground( unsigned char BgColor )
{   
	if (BgColor > 7) return;
	if      (BgColor == 6) BgColor = 3;            
	else if (BgColor == 3) BgColor = 6;
	else if (BgColor == 4) BgColor = 1;
	else if (BgColor == 1) BgColor = 4;
	printf("\033[%dm", BgColor + 40);
}

void textcolor( unsigned char FgColor )
{
	unsigned short *ScrAttr;
	ScrAttr=(unsigned short *) malloc(sizeof(unsigned short));
	*ScrAttr=0;
	if (FgColor > 128)
			{
		printf("\033[5m"); /* blink  */
		FgColor -= 128;
		}
	if ( FgColor > 7 && FgColor < 16)
			{
			 *ScrAttr = 1;
			 FgColor -= 8;
			}
		if      (FgColor == 6) FgColor = 3;            
		else if (FgColor == 3) FgColor = 6;
		else if (FgColor == 4) FgColor = 1;
			else if (FgColor == 1) FgColor = 4;
		   printf("\033[%d;%dm", *ScrAttr, FgColor + 30);
		   free(ScrAttr);
}

void gotoxy( int x, int y)
{
	v_gotoxy( x-1, y+vx_top-2 );
}
int wherex(void)
{
	int x, y;
	v_getxy(&x, &y);
	return x+1;
}
int wherey(void)
{
	int x, y;
	v_getxy(&x, &y);
	return y + 1;
}

struct text_info
	   {
		unsigned int winleft,   wintop;
		unsigned int winright,  winbottom;
		unsigned int attribute, normattr;
		unsigned int currmode; 
		unsigned int screenheight, screenwidth; 
		unsigned int curx,      cury; 
	   };

int gettextinfo(struct text_info *r)
{
		r->curx = wherex();
		r->cury = wherey();
		r->winleft = 1;
		r->winright = 80;
		r->wintop = vx_top;
		r->winbottom = vx_bot;
		r->attribute = v_getattr();
}
int gettext( int left, int top, int right, int bottom, void *destin)
{
 unsigned int length;

 length = 80 * (bottom - top + 1) * 2;
 VioReadCellStr( destin, &length, top-1, left-1, 0);
}
int puttext(int left, int top, int right, int bottom, void *source)
{
 unsigned int length;

 length = 80 * (bottom - top + 1) * 2;
 VioWrtCellStr(  source, &length, top-1, left-1, 0);
}
void window(int left, int top, int right, int bottom)
{
	vx_top = top;
	vx_bot = bottom;
}
#endif /* EMXOS2 */

extern int h_bcolor;
extern int h_fcolor;
extern int noscreenio;

int gle_abort(char *s);
int graphmode(void);
int hpgl_initstr(void);

scr_refresh()
{
}
int scr_getch()
{
}
void exitcode(int i,int j);
void exitcode_c(int i,int j);
void exitcodef(int i,int j);
scr_init()
{
	signal(SIGFPE,exitcodef);
#ifndef DJ                                /* a.r. */
	signal(SIGABRT,exitcode);
#endif
	signal(SIGILL,exitcode);
	signal(SIGINT,exitcode_c);
	signal(SIGSEGV,exitcode);
#ifndef EMXOS2
	signal(SIGTERM,exitcode);
#endif        
#ifdef EMXOS2                           /* a.r. */
	v_init();
	v_attrib(F_YELLOW | B_BLUE | INTENSITY);
#endif
}
int d_tidyup(void);

#if (defined MANIP || defined SURFACE)
	/* nothing */
#else
extern int abort_flag;
#endif
void exitcode_c(int i, int j)
{
	static char etxt[60];
	printf("Abort key\n");
#if (defined MANIP || defined SURFACE)
	/* nothing */
#else
	abort_flag = true;
#endif
	signal(SIGINT,exitcode_c);
}
abort_key(void)
{
#ifndef EMXOS2                          /* a.r. */
	int c;
 #if (defined MANIP || defined SURFACE)
	/* nothing */
 #else
	if (abort_flag) return true;
 #endif
	if (kbhit()) {
		c = bioskey(1);
		c = c & 0xff;
		if (c==27) 
		       {
			getch();
		       #if (defined MANIP || defined SURFACE)
			/* nothing */
		       #else
			abort_flag = true;
		       #endif
			return true;
		       }
	}
#else
	#ifndef MANIP
	peekMsg();
	#endif
#endif
	return false;
}

void exitcode(int i, int j)
{
	static char etxt[60];
	d_tidyup();
	sprintf(etxt,"Exit handler called %d %d \n",i,j);
	gle_abort(etxt);
}
void exitcodef(int i, int j)
{
	d_tidyup();
	printf("Floating point error %d %d \n",i,j);
#if ( ! ( defined DJ  ||  defined EMXOS2 ))                   /* a.r. */
	if (j==FPE_ZERODIVIDE) gle_abort("Divide by zero \n");
	if (j==FPE_INTDIV0) gle_abort("Interger Divide by zero \n");
	if (j==FPE_OVERFLOW) gle_abort("Numeric overflow \n");
#endif
	gle_abort("Floating point Exit handler called");
}
scr_end()
{
#ifdef DJ                                       /* a.r. */
	normvideo();
	ScreenClear();
#elif EMXOS2                                    /* a.r. */
	normvideo();
	v_attrib(F_WHITE | B_BLACK);
	clrscr();
#else
#endif
}
struct text_info r;
void *screensave;
screen_save()
{
	if (noscreenio) return;
	gettextinfo(&r);
	screensave = malloc(25*80*2);
	if (screensave==NULL) return;
	gettext(1,1,80,25,screensave);
}
screen_restore()
{
	if (noscreenio) return;
	if (screensave==NULL) return;
	puttext(1,1,80,25,screensave);
	free(screensave);
	screensave = NULL;
	textattr(r.attribute);
	window(r.winleft,r.wintop,r.winright,r.winbottom);
	gotoxy(r.curx,r.cury);
}
scr_savexy()
{
	if (noscreenio) return;
	gettextinfo(&r);
}
scr_left(int i)
{
	if (noscreenio) return;
	gotoxy(wherex()-i,wherey());
}
scr_right(int i)
{
	if (noscreenio) return;
	gotoxy(wherex()+i,wherey());
}
scr_restorexy()
{
	if (noscreenio) return;
	textattr(r.attribute);
#if (defined EMXOS2 && defined MANIP)

#else
	window(r.winleft,r.wintop,r.winright,r.winbottom);
#endif
	gotoxy(r.curx,r.cury);
}
scr_gets(char *x)
{
	gets(x);
}

extern int scr_blackwhite;
scr_norm()  /* yellow on blue */
{
		if (noscreenio) return;
#ifndef EMXOS2                                          /* a.r. */ 
		if (scr_blackwhite) {
				textcolor(WHITE); textbackground(BLACK);
		} else {
				textcolor(YELLOW); textbackground(BLUE);
		}
#else
		if (scr_blackwhite) {
				v_attrib(F_WHITE | B_BLACK | INTENSITY);
		} else {
				v_attrib(F_YELLOW | B_BLUE |INTENSITY);
				textbackground(BLUE);
		}        
#endif
}
int scr_grey(void);
scr_menuhi()
{
	if (noscreenio) return;
	scr_grey();
}
scr_menuval()
{
		if (noscreenio) return;
#ifndef EMXOS2                                                  /* a.r. */
		textcolor(WHITE); textbackground(BLUE);
#else
		v_attrib(F_WHITE | B_BLUE | INTENSITY);
		textbackground(BLUE);
#endif
}
scr_menubg()
{
		if (noscreenio) return;
#ifndef EMXOS2                                                  /* a.r. */
		textcolor(YELLOW); textbackground(BLUE);
#else
		v_attrib(F_YELLOW | B_BLUE | INTENSITY);
		textbackground(BLUE);
#endif
}
scr_inv()   /* black on white */
{
		if (noscreenio) return;
#ifndef EMXOS2                                                  /* a.r. */
		textcolor(WHITE); textbackground(BLACK);
#else
		v_attrib(F_WHITE | B_BLACK | INTENSITY);
		textbackground(BLACK);
#endif
}
scr_grey()  /* black on grey */
{
		if (noscreenio) return;
#ifndef EMXOS2                                                  /* a.r. */
		textcolor(BLACK); textbackground(LIGHTGRAY);
#else
		v_attrib(B_WHITE | F_BLACK);
		textbackground(WHITE);
#endif
}
scr_isblackwhite()
{
	char *s;
	s = getenv("CGLE_BLACKWHITE");
	if (s!=NULL) return true;
	else return false;
}
vax_edt(char *s)
{
}
#if ( ! ( defined DJ || defined EMXOS2 ) )                 /* a.r. */
char *getsymbol(char *s)
{
	static char ss[100];
	ss[0] = 0;
	if ( getenv(s) != NULL) strcpy(ss,getenv(s));
	return ss;
}
#endif

