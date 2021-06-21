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

/*--------------------------------------------------------------*/
/*	DecWindows Driver 1.0 , for GLE V3.0	         	*/
/*--------------------------------------------------------------*/
/*  	NOTE: MUST be run from a DECwindow (vt100 window, or curses) 	*/
/*								*/
/*      After 1.5 (float) beers the orphaned 3.3i gle_x code	*/
/*	compiled and was able to draw wallx.gle and adphas.gle 	*/
/*	using the gle 4.0.7 code. No core dump, yet!		*/
/*	Axel Rohde, Jan 13. 2005				*/
/*--------------------------------------------------------------*/

#include "all.h"
#include "core.h"
#include "d_interface.h"
#include "gprint.h"

#ifdef HAVE_X11

extern struct gmodel g;
extern int gle_debug;

/*---------------------------------------------------------------------------*/
/* 		XWindows stuff here 		*/
/*-----------------------------------------------*/

/* Usage of backingstore should be *Always* safe now.... */
#ifndef NOBACKINGSTORE
   #define BACKINGSTORE
#endif

#define FontName "-ADOBE-NEW CENTURY SCHOOLBOOK-MEDIUM-R-NORMAL--*-140-*-*-P-*"
#define WindowName "GLE Output"

X11GLEDevice::X11GLEDevice() : GLEDevice() {
	d_fillstyle = 1;
	gle_nspeed = 2;
	maxxsize = 0; maxysize = 0;
	window1W = 0; window1H = 0;
	window1 = 0;
	dpy = NULL;
	gc = NULL;
	gcf = NULL;
	screen = NULL;
	doesbackingstore = 0;
	savexsize = 0.0; saveysize = 0.0;
	i = 0; l = 0; j = 0; ix = 0; iy = 0;
	f = 0.0;
	xsizecm = 0.0; ysizecm = 0.0;
	d_scale = 0.0; d_xscale = 0.0; d_yscale = 0.0;
	d_graphmode = 0; d_fillstyle = 0; d_fillcolor = 0;
	d_lstyle = 0; d_lwidth = 0;
	d_maxy = 0; safnt = 0; npnts = 0;
	startx = 0; starty = 0;
}

X11GLEDevice::~X11GLEDevice() {
}

/******************** openDisplay ******************************************/

void X11GLEDevice::openDisplay() {
    dpy = XOpenDisplay(0);
    if (dpy == NULL){
	/*
        gle_abort("Display not opened!\n");
	gle_scr_end();
	*/
	perror("Unable to open Display!");
	exit(1);
    }
    screen = XDefaultScreenOfDisplay(dpy);
    doesbackingstore = DoesBackingStore(screen);
}

/***************** doInitialize **************************/
/* int color_table[10];	*/			/* a.r. 10 instead of 9 */

void X11GLEDevice::doInitialize() {
    int i;

    openDisplay();
    doCreateWindows();

    for (i=0;i<NUM_COLTABLE_ENTRIES; i++) {
	color_table[i] = doDefineColor(i);
    }

    doCreateGraphicsContext();

    XSync(dpy,False);

    doLoadFont();
    doWMHints();
    doMapWindows();
}

/******* doCreateWindows *********/
void X11GLEDevice::doCreateWindows() {
    int window1X = (XWidthOfScreen(screen)-window1W);
    int window1Y = 1;
    XSetWindowAttributes xswa;
    unsigned long CW;

    /* Create the window1 window   width and height are global variables*/

    xswa.event_mask = ExposureMask | ButtonPressMask | KeyPressMask
			| VisibilityChangeMask;

    xswa.background_pixel = doDefineColor(0);
#ifdef BACKINGSTORE
    if (doesbackingstore){
       xswa.backing_store = Always;
       CW = CWEventMask | CWBackPixel  | CWBackingStore;
    }
    else
       CW = CWEventMask | CWBackPixel;
#else
       CW = CWEventMask | CWBackPixel;
#endif

    window1 = XCreateWindow(dpy,
			    XRootWindowOfScreen(screen),
			    window1X, window1Y,
			    window1W, window1H,
			    0,
			    XDefaultDepthOfScreen(screen),
			    InputOutput,
			    XDefaultVisualOfScreen(screen),
			    CW,
			    &xswa);
}


/******** Create the graphics context *********/
void X11GLEDevice::doCreateGraphicsContext() {
    XGCValues xgcv;

    /* Create graphics context. */

    xgcv.background = doDefineColor(0);
    xgcv.foreground = doDefineColor(1);

    gc  = XCreateGC(dpy, window1, GCForeground | GCBackground, &xgcv);
    gcf = XCreateGC(dpy, window1, GCForeground | GCBackground, &xgcv);
}

void X11GLEDevice::setcolor(int i) {
    XGCValues xgcv;
    xgcv.foreground = color_table[i];
    XChangeGC(dpy, gc, GCForeground , &xgcv);
}

void X11GLEDevice::setfillcolor(int i) {
    XGCValues xgcv;
    xgcv.foreground = color_table[i];
    XChangeGC(dpy, gcf, GCForeground , &xgcv);
}

void X11GLEDevice::setfillstyle(int i) {
#define BM_WIDTH	16
#define BM_HEIGHT	16
Pixmap bm;
//unsigned
const unsigned char bm_bits[][32] = {
{  0x00, 0x80, 0x00, 0x40, 0x00, 0x20, 0x00, 0x10, 0x00, 0x08, 0x00, 0x04,
   0x00, 0x02, 0x00, 0x01, 0x80, 0x00, 0x40, 0x00, 0x20, 0x00, 0x10, 0x00,
   0x08, 0x00, 0x04, 0x00, 0x02, 0x00, 0x01, 0x00},  /* SHADE    */
{  0x11, 0x11, 0x88, 0x88, 0x44, 0x44, 0x22, 0x22, 0x11, 0x11, 0x88, 0x88,
   0x44, 0x44, 0x22, 0x22, 0x11, 0x11, 0x88, 0x88, 0x44, 0x44, 0x22, 0x22,
   0x11, 0x11, 0x88, 0x88, 0x44, 0x44, 0x22, 0x22},  /* SHADE  1 */
{  0x01, 0x01, 0x80, 0x80, 0x40, 0x40, 0x20, 0x20, 0x10, 0x10, 0x08, 0x08,
   0x04, 0x04, 0x02, 0x02, 0x01, 0x01, 0x80, 0x80, 0x40, 0x40, 0x20, 0x20,
   0x10, 0x10, 0x08, 0x08, 0x04, 0x04, 0x02, 0x02},  /* SHADE  2 */
{  0x33, 0x33, 0x99, 0x99, 0xcc, 0xcc, 0x66, 0x66, 0x33, 0x33, 0x99, 0x99,
   0xcc, 0xcc, 0x66, 0x66, 0x33, 0x33, 0x99, 0x99, 0xcc, 0xcc, 0x66, 0x66,
   0x33, 0x33, 0x99, 0x99, 0xcc, 0xcc, 0x66, 0x66},  /* SHADE  3 */
{  0x07, 0x07, 0x83, 0x83, 0xc1, 0xc1, 0xe0, 0xe0, 0x70, 0x70, 0x38, 0x38,
   0x1c, 0x1c, 0x0e, 0x0e, 0x07, 0x07, 0x83, 0x83, 0xc1, 0xc1, 0xe0, 0xe0,
   0x70, 0x70, 0x38, 0x38, 0x1c, 0x1c, 0x0e, 0x0e},  /* SHADE  4 */
{  0x0f, 0x0f, 0x87, 0x87, 0xc3, 0xc3, 0xe1, 0xe1, 0xf0, 0xf0, 0x78, 0x78,
   0x3c, 0x3c, 0x1e, 0x1e, 0x0f, 0x0f, 0x87, 0x87, 0xc3, 0xc3, 0xe1, 0xe1,
   0xf0, 0xf0, 0x78, 0x78, 0x3c, 0x3c, 0x1e, 0x1e},  /* SHADE  5 */
{  0x01, 0x80, 0x02, 0x40, 0x04, 0x20, 0x08, 0x10, 0x10, 0x08, 0x20, 0x04,
   0x40, 0x02, 0x80, 0x01, 0x80, 0x01, 0x40, 0x02, 0x20, 0x04, 0x10, 0x08,
   0x08, 0x10, 0x04, 0x20, 0x02, 0x40, 0x01, 0x80},  /* GRID     */
{  0x01, 0x01, 0x82, 0x82, 0x44, 0x44, 0x28, 0x28, 0x10, 0x10, 0x28, 0x28,
   0x44, 0x44, 0x82, 0x82, 0x01, 0x01, 0x82, 0x82, 0x44, 0x44, 0x28, 0x28,
   0x10, 0x10, 0x28, 0x28, 0x44, 0x44, 0x82, 0x82},  /* GRID   1 */
{  0x01, 0x01, 0x82, 0x82, 0x44, 0x44, 0x28, 0x28, 0x10, 0x10, 0x28, 0x28,
   0x44, 0x44, 0x82, 0x82, 0x01, 0x01, 0x82, 0x82, 0x44, 0x44, 0x28, 0x28,
   0x10, 0x10, 0x28, 0x28, 0x44, 0x44, 0x82, 0x82},  /* GRID   2 */
{  0x83, 0x83, 0xc6, 0xc6, 0x6c, 0x6c, 0x38, 0x38, 0x38, 0x38, 0x6c, 0x6c,
   0xc6, 0xc6, 0x83, 0x83, 0x83, 0x83, 0xc6, 0xc6, 0x6c, 0x6c, 0x38, 0x38,
   0x38, 0x38, 0x6c, 0x6c, 0xc6, 0xc6, 0x83, 0x83},  /* GRID   3 */
{  0x03, 0xc0, 0x06, 0x60, 0x0c, 0x30, 0x18, 0x18, 0x30, 0x0c, 0x60, 0x06,
   0xc0, 0x03, 0x80, 0x01, 0xc0, 0x03, 0x60, 0x06, 0x30, 0x0c, 0x18, 0x18,
   0x0c, 0x30, 0x06, 0x60, 0x03, 0xc0, 0x01, 0x80},  /* GRID   4 */
{  0x03, 0xc0, 0x07, 0xe0, 0x0e, 0x70, 0x1c, 0x38, 0x38, 0x1c, 0x70, 0x0e,
   0xe0, 0x07, 0xc0, 0x03, 0xc0, 0x03, 0xe0, 0x07, 0x70, 0x0e, 0x38, 0x1c,
   0x1c, 0x38, 0x0e, 0x70, 0x07, 0xe0, 0x03, 0xc0}   /* GRID   5 */
};

    bm = XCreateBitmapFromData(dpy,window1, (char*)bm_bits[i], BM_WIDTH, BM_HEIGHT);
    XSetStipple(dpy, gcf, bm);
    XSetFillStyle(dpy, gcf, FillStippled);
    XFreePixmap(dpy, bm);
}


/******* Load the font for text writing ******/
void X11GLEDevice::doLoadFont() {
    Font font;

    font = XLoadFont(dpy, FontName);
    XSetFont(dpy, gc, font);
}

               /******* Create color ************************/
int X11GLEDevice::doDefineColor(int i) {
    static Visual *vis;
    /* change NUM_COLTABLE_ENTRIES, when you add colors */
    const char *colors[] = {
	"white",	/* 0 */
	"black",	/* 1 */
	"red",		/* 2 */
	"green",	/* 3 */
	"blue",		/* 4 */
	"yellow",	/* 5 */
	"magenta",	/* 6 */
	"cyan",		/* 7 */
	"light grey",	/* 8 */
	"dim grey",	/* 9 */		/* a.r. instead of black */
	"grey5",	/* 10 */
	"grey10",	/* 11 */
	"grey20",	/* 12 */
	"grey30",	/* 13 */
	"grey40",	/* 14 */
	"grey50",	/* 15 */
	"grey60",	/* 16 */
	"grey70",	/* 17 */
	"grey80",	/* 18 */
	"grey90",	/* 19 */
	"grey100",	/* 20 */	/* dupl to white, for simplicity */
    "orange",	/* 21 */	/*  255 165 0 	*/
    "brown",	/* 22 */	/*  165 42 42	*/
    "violet",	/* 23 */	/*  238 130 238	*/
    "pink",		/* 24 */	/*  255 192 203	*/
    "dark red",	/* 25 */	/*  139 0 0	*/
    "dark blue",	/* 26 */	/*  0 0 139	*/
    "dark green",	/* 27 */	/*  0 139 0	*/
    "tan",		/* 28 */	/*  210 180 140 */
    "tan4",		/* 29 */	/*  139  90 43	*/
    "khaki",	/* 30 */	/*  240 230 140	*/
    "khaki4",	/* 31 */	/*  139 134 78	*/
    "pink4",	/* 32 */	/*  255 192 203	*/
    "darkcyan",	/* 33 */	/*  0   139 139	*/
	};

    XColor exact_color,screen_color;	/* a.r. added TrueColor */
    vis = XDefaultVisualOfScreen(screen);
    if (
            vis->c_class == PseudoColor
        ||  vis->c_class == DirectColor
        ||  vis->c_class == TrueColor
       )
        if (XAllocNamedColor(dpy, XDefaultColormapOfScreen(screen),
            colors[i], &screen_color, &exact_color))
                return screen_color.pixel;
        else
	        gprint("Color not allocated! {%s}\n",colors[i]);
    else
        switch (i) {
            case 0:		return XWhitePixelOfScreen(screen); break;
	    case 1:		return XBlackPixelOfScreen(screen); break;
            default:		return XBlackPixelOfScreen(screen); break;
        }
    return 0;
}

/******** do WMHints *************/
void X11GLEDevice::doWMHints() {
    XSizeHints xsh;

    /* Define the size and name of the window1 window */

    xsh.x = XWidthOfScreen(screen)-window1W;
    xsh.y = 1;
    xsh.width  = xsh.min_width  = xsh.max_width  = window1W;
    xsh.height = xsh.min_height = xsh.max_height = window1H;
    xsh.flags = PSize | PMinSize | PMaxSize | PPosition | USPosition;

    XSetNormalHints(dpy, window1, &xsh);

    XStoreName(dpy, window1, WindowName);
}

/******** doMapWindows ***********/
void X11GLEDevice::doMapWindows() {
	set_expose();
	XMapWindow(dpy, window1);
}

double X11GLEDevice::getmaxx(void) {
	return XHeightOfScreen( screen );			/* a.r. */
}

double X11GLEDevice::getmaxy(void) {
	return (XHeightOfScreen( screen ) * 0.95 );		/* a.r. */
	/* how do I query the height of a titlebar from the Window Manager ? */
}

int X11GLEDevice::getmaxcolor(void) {
	return 10;				/* a.r. 10 instead of 9 */
}
/*---------------------------------------------------------------------------*/

#define DASHED_LINE 2
#define SOLID_LINE 1
#define BLACKANDWHITE 1
#define false 0
#define true (!false)
#define dbg if ((gle_debug & 64)>0)

#define sx(v) ( (int) ((v) * d_xscale))
#define sy(v) ( d_maxy - ((int) ((v) * d_yscale)))
#define rx(v) ( (int) ((v) * d_xscale))
#define ry(v) ( d_maxy - ((int) ((v) * d_yscale)))

void X11GLEDevice::dxy(double x, double y, int *dx, int *dy) {
	static double fx,fy;
	g_dev(x,y,&fx,&fy);
	*dx = sx(fx);
	*dy = sy(fy);
}

void X11GLEDevice::rxy(double x, double y, int *dx, int *dy) {
	static double fx,fy,zx,zy;
	g_dev(x,y,&fx,&fy);
	g_dev(0.0,0.0,&zx,&zy);
	*dx = (int) ( (fx-zx) * d_xscale);
	*dy = (int) ( (fy-zy) * d_yscale);
}

/* short for XFillPolygon() */
void X11GLEDevice::dxy_short(double x, double y, short *dx, short *dy) {
	static double fx,fy;
	g_dev(x,y,&fx,&fy);
	*dx = sx(fx);
	*dy = sy(fy);
}

void X11GLEDevice::rxy_short(double x, double y, short *dx, short *dy) {
	static double fx,fy,zx,zy;
	g_dev(x,y,&fx,&fy);
	g_dev(0.0,0.0,&zx,&zy);
	*dx = (int) ( (fx-zx) * d_xscale);
	*dy = (int) ( (fy-zy) * d_yscale);
}

void X11GLEDevice::devcmd(const char *s) {
}
/*---------------------------------------------------------------------------*/
void X11GLEDevice::dfont(char *c)
{
	/* only used for the DFONT driver which builds fonts */
}
void X11GLEDevice::resetfont()
{
}
/*---------------------------------------------------------------------------*/
static char lastline[80];
void X11GLEDevice::message(char *s)
{
	/* w_message(s); 4.0 */
	printf("%s\n",s);
}
/*---------------------------------------------------------------------------*/
void X11GLEDevice::source(const char *s)
{
	dbg fprintf(stderr,"drawing %s ....",s);
}
/*---------------------------------------------------------------------------*/
std::string X11GLEDevice::get_type()
{
	return "INTERACTIVE, X, DECWINDOWS, XWINDOWS";
}
/*---------------------------------------------------------------------------*/
void X11GLEDevice::set_path(int onoff)
{}
/*---------------------------------------------------------------------------*/
void X11GLEDevice::newpath()
{
	path_newpath();
}
/*---------------------------------------------------------------------------*/
void X11GLEDevice::opendev(double width, double height, GLEFileLocation* outputfile, const string& inputfile) throw(ParserError)
{
	gle_nspeed = 2; /* text mode = slow and fast */
	xsizecm = 16;
	ysizecm = 16*0.95;
	/* Get largest rectangle we can fit on the screen */
	d_scale = xsizecm / width;
	f = ysizecm / height;
	if (f<d_scale) d_scale = f;
	if (dpy==NULL)
		openDisplay();
	d_xscale = d_scale * (getmaxx()) / xsizecm; /* Device Scale X, Device Scale y */
	d_yscale = d_scale * (getmaxy()) / ysizecm;
    window1W = (int) (width*d_xscale);
    window1H = (int) (height*d_yscale);
	d_maxy = window1H;
	if (dpy==NULL || savexsize != width || saveysize != height) {
		if (dpy != NULL) {
			if (window1 != 0) {
				XUnmapWindow(dpy, window1);
				XDestroyWindow(dpy, window1);
			}
			XCloseDisplay(dpy);
		}
		openDisplay();
		doInitialize();
			set_expose();
		XRaiseWindow(dpy, window1);
		wait_expose();
	} else {
		set_expose();
		XRaiseWindow(dpy, window1);
#ifndef BACKINGSTORE						/* a.r. */
		wait_expose();
#else
		if (!doesbackingstore)
			wait_expose();
#endif
	}
	XClearWindow(dpy, window1);
	XSync(dpy,False);
	savexsize = width;
	saveysize = height;
}
/*---------------------------------------------------------------------------*/

void X11GLEDevice::closedev() {
        int emask;
        XEvent ereturn;
        Window rr, cr;
        int rx, ry, cx, cy;
        unsigned int retmask;

        g_flush();
        lastline[0] = 0;
        XSync(dpy,False);
        XQueryPointer(dpy, window1, &rr, &cr, &rx, &ry, &cx, &cy, &retmask);
/*      if ( cx >= 0 &&  cx <= window1W && cy >= 0 &&  cy <= window1H ) {
            fner("Picture completed, press any key or click mouse in the graphics window        ");
            gle_scr_refresh();
*/
            fprintf(stderr, "\nDrawing completed, press CTRL-c on commandline to exit...\n");
            emask = NoEventMask;
            XWindowEvent(dpy, window1, emask, &ereturn);
/*      } else {
            fner("Picture completed, press RETURN to continue                    (Press any key)");
            gle_scr_refresh();

            text_inkey();
        }

        XLowerWindow(dpy, window1);
        XSync(dpy,True);
*/
}

/*---------------------------------------------------------------------------*/
void X11GLEDevice::set_line_cap(int i)
{
   /*  lcap, 0= butt, 1=round, 2=projecting square */
/* if X11 would do the same as Postscript......
   XGCValues xgcv;

   xgcv.cap_style = i+1;
   XChangeGC(dpy, gc,  GCCapStyle , &xgcv);
   XChangeGC(dpy, gcf, GCCapStyle , &xgcv);
*/
}
/*---------------------------------------------------------------------------*/
void X11GLEDevice::set_line_join(int i)
{
   /* 0= mitre, 1=round, 2=bevel */
/* if X11 would do the same as Postscript......
   XGCValues xgcv;

   xgcv.join_style = i;
   XChangeGC(dpy, gc,  GCJoinStyle, &xgcv);
   XChangeGC(dpy, gcf, GCJoinStyle, &xgcv);
*/
}
/*---------------------------------------------------------------------------*/
void X11GLEDevice::set_line_miterlimit(double d)
{
	i++;
}
/*---------------------------------------------------------------------------*/
void X11GLEDevice::set_line_width(double w)
{
	int xa,xb;
	XGCValues xgcv;
	rxy(w,w,&xa,&xb);
	rxy(w,w,&xa,&xb);
	xa = abs(xa);
    	xgcv.line_width = xa;
    	XChangeGC(dpy, gc, GCLineWidth , &xgcv);
}
/*---------------------------------------------------------------------------*/
void X11GLEDevice::set_line_styled(double dd)
{}
void X11GLEDevice::set_line_style(const char *s)
{
	XGCValues xgcv;
    const char *defline[] = {"", "", "12", "41", "14", "92", "1282", "9229", "4114", "54"};
	int i, dashoff=0;
	char dashlist[64];

	if (strlen(s)==1)  s = defline[*s-'0'];
	if (strcmp(s,"")==0) {
	    xgcv.line_style  = LineSolid;
	    XChangeGC(dpy, gc, GCLineStyle , &xgcv);
	} else {
	    xgcv.line_style  = LineDoubleDash;
	    XChangeGC(dpy, gc, GCLineStyle , &xgcv);
	    for (i=0; *s!=0; s++,i++)
		dashlist[i] = *s-'0' ? *s-'0' : 1;
	    XSetDashes(dpy, gc, dashoff, dashlist, i);
	}
}
/*---------------------------------------------------------------------------*/
int in_font;
void X11GLEDevice::fill()
{
	if (in_font)
		path_stroke();
	else
		path_fill();
}
/*---------------------------------------------------------------------------*/
void X11GLEDevice::fill_ary(int nwk,double *wkx,double *wky)
{
/*	fprintf(psfile,"%g %g moveto \n",wkx[0],wky[0]);
	for (i=1;i<nwk;i++)
		fprintf(psfile,"%g %g l \n",wkx[i],wky[i]);
*/
}
void X11GLEDevice::line_ary(int nwk,double *wkx,double *wky)
{
	int i;
	dxy( wkx[0], wky[0], &ix, &iy);
/*	moveto(ix,iy); */
	for (i=1;i<nwk;i++) {
		dxy( wkx[i], wky[i], &ix, &iy);
	}
}
/*---------------------------------------------------------------------------*/
void X11GLEDevice::stroke()
{
	path_stroke();
}
/*---------------------------------------------------------------------------*/
void X11GLEDevice::clip()
{
}
/*---------------------------------------------------------------------------*/
void X11GLEDevice::set_matrix(double newmat[3][3])
{
}
/*---------------------------------------------------------------------------*/
void X11GLEDevice::move(double zx,double zy)
{
}
/*---------------------------------------------------------------------------*/
void X11GLEDevice::reverse() 	/* reverse the order of stuff in the current path */
{
}
/*---------------------------------------------------------------------------*/
void X11GLEDevice::closepath()
{
	if (g.inpath==true) {
		path_close();
		return;
	}
	g_line(g.closex,g.closey);
}
/*---------------------------------------------------------------------------*/
void X11GLEDevice::line(double zx,double zy)
{
	static int ux,uy;
	dxy(g.curx,g.cury,&ux,&uy);
	dxy(zx,zy,&ix,&iy);
	if (g.inpath==true) {
		if (!g.xinline) path_move(ux,uy);
		path_line(ix,iy);
		return;
	}
    	XDrawLine(dpy,window1,gc,ux,uy,ix,iy);
}
/*---------------------------------------------------------------------------*/
void X11GLEDevice::clear()
{
}
/*---------------------------------------------------------------------------*/
void X11GLEDevice::flush()
{
}
/*---------------------------------------------------------------------------*/
void polar_xy(double r, double angle, double *dx, double *dy);
void xy_polar(double dx,double dy,double *radius,double *angle);

#ifdef OLDARC /* draws weird things */
void X11GLEDevice::arcto(dbl x1,dbl y1,dbl x2,dbl y2,dbl rrr)
{
	double x0,y0,r1,a1,r2,a2,r3,a3,a4,r5,ssx,ssy,ex,ey;
	double bx1,by1,bx2,by2,dist,neg;
	g_get_xy(&x0,&y0);
	xy_polar(x1-x0,y1-y0,&r1,&a1);
	xy_polar(x2-x1,y2-y1,&r2,&a2);
	neg = 1;
	a4 = (180-a2+a1);
	a3 = a2 + (a4/2);
	if ((a4/2)>90 && (a4/2)<180 ) neg = -1;
	if ((a4/2)<0 && (a4/2)>-90 ) neg = -1;
	r3 = neg*rrr/(tan((GLE_PI/180)*a4/2));
	dbg gprint("rrr %g a4/2 %g t=%g a2=%g a1=%g r1=%g r2=%g r3=%g \n",rrr,a4/2,tan(a4/2),a2,a1,r1,r2,r3);
	polar_xy(-r3,a1,&ssx,&ssy); ssx += x1; ssy += y1;
	polar_xy(r3,a2,&ex,&ey); ex += x1; ey += y1;
	g_line(ssx,ssy);
	dist = sqrt((ex-ssx) * (ex-ssx) + (ey-ssy)*(ey-ssy));
	polar_xy(r1+ dist/2.5-r3,a1,&bx1,&by1); bx1 += x0; by1 += y0;
	polar_xy(-r2+ -dist/2.5+r3,a2,&bx2,&by2); bx2 += x2; by2 += y2;
	g_bezier(bx1,by1,bx2,by2,ex,ey);
	g_line(x2,y2);
}
/*---------------------------------------------------------------------------*/
void X11GLEDevice::arc(dbl r, dbl t1, dbl t2, dbl cx, dbl cy)
{
	static int ixr,iyr,icx,icy,a1,a2;
	double ux,uy,z;
	dxy(cx,cy,&icx,&icy);
	rxy(r,r,&ixr,&iyr);
	if (t1>t2) {z = t1; t1 = t2; t2 = z;}
	a1 = t1*64; a2 = t2*64 - a1;
    	XDrawArc(dpy,window1,gc,icx-ixr,icy-iyr,ixr*2,iyr*2,a1,a2);
}
/*---------------------------------------------------------------------------*/
void X11GLEDevice::narc(dbl r, dbl t1, dbl t2, dbl cx, dbl cy)
{
	static int ixr,iyr,icx,icy,a1,a2;
	double ux,uy,z;
	dxy(cx,cy,&icx,&icy);
	rxy(r,r,&ixr,&iyr);
	if (t1>t2) {z = t1; t1 = t2; t2 = z;}
	a1 = t1*64; a2 = t2*64 - a1;
    	XDrawArc(dpy,window1,gc,icx-ixr,icy-iyr,ixr*2,iyr*2,a1,a2);
}
/*---------------------------------------------------------------------------*/
#else /* arc code from easydev.c */
#define CSTEP (360/6)
void df_arcto(dbl x1,dbl y1,dbl x2,dbl y2,dbl rrr)
{
	double x0,y0,r1,a1,r2,a2,r3,a4,sx,sy,ex,ey;
	double bx1,by1,bx2,by2,dist,neg;
	g_get_xy(&x0,&y0);
	xy_polar(x1-x0,y1-y0,&r1,&a1);
	xy_polar(x2-x1,y2-y1,&r2,&a2);
	neg = 1;
	a4 = (180-a2+a1);
	if ((a4/2)>90 && (a4/2)<180 ) neg = -1;
	if ((a4/2)<0 && (a4/2)>-90 ) neg = -1;
	r3 = neg*rrr/(tan((GLE_PI/180)*a4/2));
	polar_xy(-r3,a1,&sx,&sy); sx += x1; sy += y1;
	polar_xy(r3,a2,&ex,&ey); ex += x1; ey += y1;
	g_line(sx,sy);
	dist = sqrt((ex-sx)*(ex-sx) + (ey-sy)*(ey-sy));
	polar_xy(r1+ dist/2.5-r3,a1,&bx1,&by1); bx1 += x0; by1 += y0;
	polar_xy(-r2+ -dist/2.5+r3,a2,&bx2,&by2); bx2 += x2; by2 += y2;
	g_bezier(bx1,by1,bx2,by2,ex,ey);
	g_line(x2,y2);
}
void xdf_barc(double r,dbl t1,dbl t2,dbl cx,dbl cy)
{
	double rx1,ry1,rx2,ry2,d,dx1,dy1,dx2,dy2;

	polar_xy(r,t1,&rx1,&ry1);
	polar_xy(r,t2,&rx2,&ry2);
	d = sqrt( (rx2-rx1)*(rx2-rx1) + (ry2-ry1)*(ry2-ry1));
	polar_xy(d/3,t1+90,&dx1,&dy1);
	polar_xy(d/3,t2-90,&dx2,&dy2);
	if (g.inpath) {
		g_line(rx1+cx,ry1+cy);
		g_bezier(rx1+cx+dx1,ry1+cy+dy1
			,rx2+cx+dx2,ry2+cy+dy2,rx2+cx,ry2+cy);
	} else {
		g_move(rx1+cx,ry1+cy);
		g_bezier(rx1+cx+dx1,ry1+cy+dy1
			,rx2+cx+dx2,ry2+cy+dy2,rx2+cx,ry2+cy);
		g_move(cx,cy);
	}
}

void df_arc(dbl r,dbl t1,dbl t2,dbl cx,dbl cy)
{
	/* circle from t1 to t2, lets use 6 bezier's for a circle */
	double stz;
	int nst,i;

	for (;t2<t1;)
	      t2 = t2 + 360;

	nst = (int) (floor((t2-t1)/CSTEP)+1);
	stz = (t2-t1) / nst;
	for (i=1;i<=nst;i++)
		xdf_barc(r,t1+stz*(i-1),t1+stz*i,cx,cy);
}

void X11GLEDevice::arcto(dbl x1,dbl y1,dbl x2,dbl y2,dbl rrr)
{
	df_arcto(x1,y1,x2,y2,rrr);
}
/*---------------------------------------------------------------------------*/
void X11GLEDevice::arc(dbl r,dbl t1,dbl t2,dbl cx,dbl cy)
{
	df_arc(r,t1,t2,cx,cy);
}
/*---------------------------------------------------------------------------*/
void X11GLEDevice::narc(dbl r,dbl t1,dbl t2,dbl cx,dbl cy)
{
	/* swap t1 and t2 */
	df_arc(r,t2,t1,cx,cy);
}
/*---------------------------------------------------------------------------*/
#endif /* OLDARC */
/*---------------------------------------------------------------------------*/

void X11GLEDevice::box_fill(dbl x1, dbl y1, dbl x2, dbl y2)
{
#ifdef OLDARC
	static int ix1,iy1,ix2,iy2;
	static int ii;
	dxy(x1,y1,&ix1,&iy1);
	dxy(x2,y2,&ix2,&iy2);
	if (ix1>ix2) { ii = ix1; ix1 = ix2; ix2 = ii; }
	if (iy1>iy2) { ii = iy1; iy1 = iy2; iy2 = ii; }
    	XFillRectangle(dpy,window1,gcf,ix1,iy1,ix2-ix1,iy2-iy1);
#else
	XPoint point[4];
	int n = 0;

        dxy_short(x1,y1,&point[n].x,&point[n].y); n++;
        dxy_short(x2,y1,&point[n].x,&point[n].y); n++;
        dxy_short(x2,y2,&point[n].x,&point[n].y); n++;
        dxy_short(x1,y2,&point[n].x,&point[n].y);

   	XFillPolygon(dpy,window1,gcf,point,4,Convex,CoordModeOrigin);
#endif
}
void X11GLEDevice::box_stroke(dbl x1, dbl y1, dbl x2, dbl y2, bool reverse)
{
   	g_move(x1,y1);
        g_line(x2,y1);
        g_line(x2,y2);
        g_line(x1,y2);
        g_line(x1,y1);
}
/*---------------------------------------------------------------------------*/
void X11GLEDevice::circle_stroke(double zr)
{
#ifdef OLDARC
	static int ixr,iyr;
	int ux,uy;
	rxy(zr,zr,&ixr,&iyr);
	ixr =(fabs(ixr)+fabs(iyr))/2;
	dxy(g.curx,g.cury,&ux,&uy);
    	XDrawArc(dpy,window1,gc,ux-ixr,uy-ixr,ixr*2,ixr*2,0,64*360);
#else
	arc(zr,0,360,g.curx,g.cury);
#endif
}
void X11GLEDevice::circle_fill(double zr)
{
#ifdef OLDARC
	static int ixr,iyr;
	int rr;
	int ux,uy;
	rxy(zr,zr,&ixr,&iyr);
	rr = (fabs(ixr)+fabs(iyr))/2;
	dxy(g.curx,g.cury,&ux,&uy);
    	XFillArc(dpy,window1,gcf,ux-rr,uy-rr,rr*2,rr*2,0,64*360);
#else
	if (!g.inpath){
		g_set_path(true);
		g_newpath();
		g_arc(zr,0,360,g.curx,g.cury,0);
		g_closepath();
		g_fill();
		g_set_path(false);
	}
	else
		g_arc(zr,0,360,g.curx,g.cury,0);
#endif
}
/*---------------------------------------------------------------------------*/
void X11GLEDevice::bezier(dbl x1,dbl y1,dbl x2,dbl y2,dbl x3,dbl y3)
{
	double ax,bx,cx,ay,by,cy,dist;
	double xxx,yyy,i,t,nstep,x0,y0;
	g_get_xy(&x0,&y0);
	dist = fabs(x3-x0) + fabs(y3-y0);
	nstep = 12;
	if (dist<1) nstep = 7;
	if (dist<.5) nstep = 3;
	if (dist<.1) {
		g_line(x3,y3);
		return;
	}
	cx = (x1-x0)*3;
	bx = (x2-x1)*3-cx;
	ax = x3-x0-cx-bx;
	cy = (y1-y0)*3;
	by = (y2-y1)*3-cy;
	ay = y3-y0-cy-by;
	for (i=0;i<=nstep;i++) {
		t = i/nstep;
		xxx = ax*pow(t,3.0) + bx*t*t + cx*t + x0;
		yyy = ay*pow(t,3.0) + by*t*t + cy*t + y0;
		g_line(xxx,yyy);
	}
}
/*---------------------------------------------------------------------------*/
void X11GLEDevice::set_color(const GLERC<GLEColor>& color)			/* colors slightly changed a.r. */
{
	int f = color->getHexValueGLE();
	int i;
	colortyp  cc;
	cc.l = f;
	i = 1;

     /* process grey stuff first */
     if (cc.b[B_R] == cc.b[B_G] && cc.b[B_G] == cc.b[B_B]) {
        if (cc.b[B_R] <   25) 	i = 1;	/* black */
        if (cc.b[B_R] >=  25) 	i = 11;
        if (cc.b[B_R] >=  50) 	i = 12;
        if (cc.b[B_R] >=  75) 	i = 13;
        if (cc.b[B_R] >= 100) 	i = 14;
        if (cc.b[B_R] >= 125) 	i = 15;
        if (cc.b[B_R] >= 150) 	i = 16;
        if (cc.b[B_R] >= 175) 	i = 17;
        if (cc.b[B_R] >= 200) 	i = 18;
        if (cc.b[B_R] >= 225) 	i = 19;
        if (cc.b[B_R] >= 250)	i = 0;	/* white */
     }
     else {
	if (cc.b[B_R]>=10 && cc.b[B_G]>=10 && cc.b[B_B]>=10) 	i = 9;
	if (cc.b[B_R]>60  && cc.b[B_G]>60  && cc.b[B_B]>60)  	i = 8;
	if (cc.b[B_R]>80) 					i = 25;
	if (cc.b[B_R]>175)					i = 2;
	if (cc.b[B_B]>80) 					i = 26;
      	if (cc.b[B_B]>175) 					i = 4;
	if (cc.b[B_G]>80) 					i = 27;
	if (cc.b[B_G]>175) 					i = 3;
	if (cc.b[B_R]>100 && cc.b[B_G]>100) 			i = 5;
	if (cc.b[B_G]>100 && cc.b[B_B]>100) 			i = 7;
	if (cc.b[B_R]>30  && cc.b[B_B]>100) 			i = 6;
	if (cc.b[B_R]>100 && cc.b[B_G]>100 && cc.b[B_B]>100) 	i = 8;
	if (cc.b[B_R]<10  && cc.b[B_G]<10  && cc.b[B_B]<10) 	i = 1;
	if (cc.b[B_R]>250 && cc.b[B_G]>250 && cc.b[B_B]>250) 	i = 0;
        /* violet */
        if (cc.b[B_R]>230 && cc.b[B_G]>110 && cc.b[B_B]>230) 	i = 23;
        /* orange: */
	if (cc.b[B_R]>245 && cc.b[B_G]>150
                          && cc.b[B_G]<180 && cc.b[B_B]<10) 	i = 21;
        /* brown */
	if (cc.b[B_R]>150 && cc.b[B_G]>35 && cc.b[B_B]>35
         && cc.b[B_R]<180 && cc.b[B_G]<50 && cc.b[B_B]<50) 	i = 22;
        /* pink */
	if (cc.b[B_R]>250 && cc.b[B_G]>175 && cc.b[B_B]>185
                          && cc.b[B_G]<210 && cc.b[B_B]<225) 	i = 24;
        /* darkpink */
	if (cc.b[B_R]>129 && cc.b[B_G]>89 && cc.b[B_B]>98
         && cc.b[B_R]<149  && cc.b[B_G]<100  && cc.b[B_B]<118 )	i = 32;
        /* darkcyan */
	if (              cc.b[B_G]>120 && cc.b[B_B]>120
         && cc.b[B_R]<25  && cc.b[B_G]<160  && cc.b[B_B]<160 ) 	i = 33;
        /* khaki */
	if (cc.b[B_R]>225 && cc.b[B_G]>215 && cc.b[B_B]>120
                          && cc.b[B_G]<245 && cc.b[B_B]<160 ) 	i = 30;
        /* darkkhaki */
	if (cc.b[B_R]>120 && cc.b[B_G]>119 && cc.b[B_B]>50
         && cc.b[B_R]<160  && cc.b[B_G]<150  && cc.b[B_B]<100 ) i = 31;
        /* tan */
	if (cc.b[B_R]>190 && cc.b[B_G]>160 && cc.b[B_B]>120
         && cc.b[B_R]<230  && cc.b[B_G]<200  && cc.b[B_B]<160 ) i = 28;
        /* tan4 */
	if (cc.b[B_R]>129 && cc.b[B_G]>70 && cc.b[B_B]>23
         && cc.b[B_R]<169  && cc.b[B_G]<110  && cc.b[B_B]<63 ) 	i = 29;
        /* magenta */
        if (cc.b[B_R]>238 && cc.b[B_G]<20 && cc.b[B_B]>238) 	i = 6;
     }
     setcolor(i);
}

void X11GLEDevice::set_fill(const GLERC<GLEColor>& fill)			/* colors slightly changed a.r. */
{
	int f = fill->getHexValueGLE();
	int i, j;
	colortyp  cc;
	cc.l = f;
	i = 1;
   	j = 0;
        if (cc.b[B_F] == 1)     /* colours */
        {
             /* process grey stuff first */
             if (cc.b[B_R] == cc.b[B_G] && cc.b[B_G] == cc.b[B_B]) {
                if (cc.b[B_R] <   25) 	i = 1;	/* black */
                if (cc.b[B_R] >=  25) 	i = 11;
                if (cc.b[B_R] >=  50) 	i = 12;
                if (cc.b[B_R] >=  75) 	i = 13;
                if (cc.b[B_R] >= 100) 	i = 14;
                if (cc.b[B_R] >= 125) 	i = 15;
                if (cc.b[B_R] >= 150) 	i = 16;
                if (cc.b[B_R] >= 175) 	i = 17;
                if (cc.b[B_R] >= 200) 	i = 18;
                if (cc.b[B_R] >= 225) 	i = 19;
                if (cc.b[B_R] >= 250)	i = 0;	/* white */
             }
             else {
                if (cc.b[B_R]>=10 && cc.b[B_G]>=10 && cc.b[B_B]>=10) 	i = 9;
                if (cc.b[B_R]>60  && cc.b[B_G]>60  && cc.b[B_B]>60)  	i = 8;
	        if (cc.b[B_R]>60) 					i = 25;
	        if (cc.b[B_R]>175)					i = 2;
	        if (cc.b[B_B]>60) 					i = 26;
      	        if (cc.b[B_B]>175) 					i = 4;
	        if (cc.b[B_G]>60) 					i = 27;
	        if (cc.b[B_G]>175) 					i = 3;
                if (cc.b[B_R]>100 && cc.b[B_G]>100) 			i = 5;
                if (cc.b[B_G]>100 && cc.b[B_B]>100) 			i = 7;
                if (cc.b[B_R]>30  && cc.b[B_B]>100) 			i = 6;
                if (cc.b[B_R]>100 && cc.b[B_G]>100 && cc.b[B_B]>100) 	i = 8;
                if (cc.b[B_R]<10  && cc.b[B_G]<10  && cc.b[B_B]< 10) 	i = 1;
                if (cc.b[B_R]>250 && cc.b[B_G]>250 && cc.b[B_B]>250) 	i = 0;
                /* violet */
                if (cc.b[B_R]>230 && cc.b[B_G]>110 && cc.b[B_B]>230) 	i = 23;
                /* orange: */
	        if (cc.b[B_R]>245 && cc.b[B_G]>150
                                  && cc.b[B_G]<180 && cc.b[B_B]<10) 	i = 21;
                /* brown */
	        if (cc.b[B_R]>150 && cc.b[B_G]>35 && cc.b[B_B]>35
                 && cc.b[B_R]<180 && cc.b[B_G]<50 && cc.b[B_B]<50) 	i = 22;
                /* pink */
	        if (cc.b[B_R]>250 && cc.b[B_G]>175 && cc.b[B_B]>185
                                  && cc.b[B_G]<210 && cc.b[B_B]<225) 	i = 24;
                /* darkpink */
	        if (cc.b[B_R]>129 && cc.b[B_G]>89 && cc.b[B_B]>98
                 && cc.b[B_R]<149  && cc.b[B_G]<100  && cc.b[B_B]<118 )	i = 32;
                /* darkcyan */
	        if (              cc.b[B_G]>120 && cc.b[B_B]>120
                 && cc.b[B_R]<25  && cc.b[B_G]<160  && cc.b[B_B]<160 ) 	i = 33;
                /* khaki */
	        if (cc.b[B_R]>225 && cc.b[B_G]>215 && cc.b[B_B]>120
                                  && cc.b[B_G]<245 && cc.b[B_B]<160 ) 	i = 30;
                /* darkkhaki */
	        if (cc.b[B_R]>120 && cc.b[B_G]>119 && cc.b[B_B]>50
                 && cc.b[B_R]<160  && cc.b[B_G]<150  && cc.b[B_B]<100 ) i = 31;
                /* tan */
	        if (cc.b[B_R]>190 && cc.b[B_G]>160 && cc.b[B_B]>120
                 && cc.b[B_R]<230  && cc.b[B_G]<200  && cc.b[B_B]<160 ) i = 28;
                /* tan4 */
	        if (cc.b[B_R]>129 && cc.b[B_G]>70 && cc.b[B_B]>23
                 && cc.b[B_R]<169  && cc.b[B_G]<110  && cc.b[B_B]<63 ) 	i = 29;
                /* magenta */
                if (cc.b[B_R]>238 && cc.b[B_G]<20 && cc.b[B_B]>238) 	i = 6;
            }
            XSetFillStyle(dpy, gcf, FillSolid);
            d_fillcolor = i;
            setfillcolor(i);
        }
        if (cc.b[B_F] == 2)     /* grids and shades */
        {
            if (cc.b[B_R]==0x00 && cc.b[B_G]==0x00 && cc.b[B_B]==0x20) j=0; /* SHADE  */
            if (cc.b[B_R]==0x04 && cc.b[B_G]==0x00 && cc.b[B_B]==0x0C) j=1; /* SHADE1 */
            if (cc.b[B_R]==0x00 && cc.b[B_G]==0x00 && cc.b[B_B]==0x10) j=2; /* SHADE2 */
            if (cc.b[B_R]==0x05 && cc.b[B_G]==0x00 && cc.b[B_B]==0x20) j=3; /* SHADE3 */
            if (cc.b[B_R]==0x10 && cc.b[B_G]==0x00 && cc.b[B_B]==0x40) j=4; /* SHADE4 */
            if (cc.b[B_R]==0x20 && cc.b[B_G]==0x00 && cc.b[B_B]==0x60) j=5; /* SHADE5 */
            if (cc.b[B_R]==0x00 && cc.b[B_G]==0x20 && cc.b[B_B]==0x20) j=6; /* GRID   */
            if (cc.b[B_R]==0x04 && cc.b[B_G]==0x0f && cc.b[B_B]==0x0f) j=7; /* GRID1  */
            if (cc.b[B_R]==0x00 && cc.b[B_G]==0x10 && cc.b[B_B]==0x10) j=8; /* GRID2  */
            if (cc.b[B_R]==0x05 && cc.b[B_G]==0x20 && cc.b[B_B]==0x20) j=9; /* GRID3  */
            if (cc.b[B_R]==0x10 && cc.b[B_G]==0x40 && cc.b[B_B]==0x40) j=10;/* GRID4  */
            if (cc.b[B_R]==0x20 && cc.b[B_G]==0x60 && cc.b[B_B]==0x60) j=11;/* GRID5  */
            setfillcolor(1); /* black */
            setfillstyle(j);
        }
        else
        {
            XSetFillStyle(dpy, gcf, FillSolid);
            setfillcolor(d_fillcolor);
        }
}

/*---------------------------------------------------------------------------*/

void X11GLEDevice::beginclip() {
}

void X11GLEDevice::endclip() {
}

struct X11_char_data {float wx,wy,x1,y1,x2,y2; };

void X11GLEDevice::dochar(int font, int cc) {
	in_font = true;
	if (safnt==0) safnt = pass_font("PLSR");
	if (font_get_encoding(font)>2) {
		my_char(font,cc);
		in_font = false;
		return;
	}
	my_char(safnt,cc);
	in_font = false;
}


int X11GLEDevice::wait_expose(void) {
	/* Wait till he presses key or clicks mouse */
	XEvent ereturn;
	int emask;

/*	for (;;) {
		XNextEvent(dpy,&ereturn);
		wprintf("Event %d  %d %d %d\n",ereturn.type,Expose,ButtonPress,
			VisibilityNotify);
		gle_scr_refresh();
	}
*/
/*	 XSync(dpy,True);	*/
    	emask = ExposureMask | ButtonPressMask | KeyPressMask;
	XWindowEvent(dpy, window1, emask, &ereturn);
	return 0;
}

int X11GLEDevice::set_expose(void) {
   int emask;

   emask = ExposureMask | ButtonPressMask | KeyPressMask
			| VisibilityChangeMask;

/* #define X11R5 true for X11R5 and R6 */
#define X11R5 true
#ifdef X11R5
	XSelectInput(dpy, window1, emask);
#else
	XSelectInput(dpy, window1, emask, &ereturn);
#endif
    return 0;
}

enum {P_MOVE,P_LINE,P_BEZIER};

void X11GLEDevice::path_move(int x, int y) {
	startx = x; starty = y;
	pnts[npnts].type = P_MOVE;
	pnts[npnts].x = x;
	pnts[npnts++].y = y;
}

void X11GLEDevice::path_line(int x, int y) {
	pnts[npnts].type = P_LINE;
	pnts[npnts].x = x;
	pnts[npnts++].y = y;
}

void X11GLEDevice::path_close(void) {
	pnts[npnts].type = P_LINE;
	pnts[npnts].x = startx;
	pnts[npnts++].y = starty;
}

void X11GLEDevice::path_fill(void) {
	XPoint pts[X11_PATH_LENGTH];
	int i,npts;

	for (i=0; i<npnts; i++) {
		if (pnts[i].type==P_LINE) {
			npts = 0;
			pts[npts].x = pnts[i].x;
			pts[npts++].y = pnts[i++].y;
			for (;pnts[i].type==P_LINE && i<npnts;i++) {
				pts[npts].x = pnts[i].x;
				pts[npts++].y = pnts[i].y;
			}
		   	XFillPolygon(dpy,window1,gcf,pts,npts,Complex, CoordModeOrigin);
		}
	}
}

void X11GLEDevice::path_newpath(void) {
	npnts = 0;
}

void X11GLEDevice::path_stroke(void) {
	for (i=1; i<npnts; i++) {
		if (pnts[i].type==P_LINE) {
		    	XDrawLine(dpy,window1,gc,
				pnts[i-1].x,pnts[i-1].y,
				pnts[i].x,pnts[i].y);
		}
	}
}

/* new gle 4.x stuff */

void X11GLEDevice::ellipse_fill(double rx, double ry) {
}

void X11GLEDevice::ellipse_stroke(double rx, double ry) {
}

void X11GLEDevice::elliptical_arc(double rx,double ry,double t1,double t2,double cx,double cy) {
}

void X11GLEDevice::elliptical_narc(double rx,double ry,double t1,double t2,double cx,double cy) {
}

/*
void X11GLEDevice::get_line_cap(int *i)
{
}
*/

void X11GLEDevice::pscomment(char* ss) {
}

void X11GLEDevice::set_color() {
	/*
	if (BLACKANDWHITE) {
	 	fprintf(psfile,"%g setgray \n",((g_cur_color.b[B_R]*3.0/255.0
		+g_cur_color.b[B_G]*2.0/255.0+g_cur_color.b[B_B]/255.0) / 6));
	} else
	 	fprintf(psfile,"%g %g %g setrgbcolor \n",g_cur_color.b[B_R]/255.0
		,g_cur_color.b[B_G]/255.0,g_cur_color.b[B_B]/255.0);
	*/
}

void X11GLEDevice::set_fill() {
	/*
	if (BLACKANDWHITE) {
	 	fprintf(psfile,"%g setgray \n",((g_cur_fill.b[B_R]*3.0/255.0
		+g_cur_fill.b[B_G]*2.0/255.0+g_cur_fill.b[B_B]/255.0) / 6));
	} else
	 	fprintf(psfile,"%g %g %g setrgbcolor \n",g_cur_fill.b[B_R]/255.0
		,g_cur_fill.b[B_G]/255.0,g_cur_fill.b[B_B]/255.0);
	*/
}

void X11GLEDevice::xdbox(double x1, double y1, double x2, double y2) {
 /*
   fprintf(psfile," %g %g moveto %g %g l %g %g l %g %g l closepath \n",
	x1,y1,x2,y1,x2,y2,x1,y2);
 */
}


FILE* X11GLEDevice::get_file_pointer(void) {
	return (FILE*) NULL;
}

int X11GLEDevice::getDeviceType() {
	return GLE_DEVICE_X11;
}

#endif
