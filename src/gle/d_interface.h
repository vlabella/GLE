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

#ifndef INCLUDE_D_INTERFACE
#define INCLUDE_D_INTERFACE

// all device files (d_*) should define these functions
// core.cpp calls them

class GLEBitmap;
class GLEPoint;

int setMaxPSVector(int newMax);

class GLEDevice {
protected:
	bool m_recording;
	GLEPoint m_boundingBox;
	double m_resolution;
public:
	GLEDevice();
	virtual ~GLEDevice();
	virtual void arc(dbl r,dbl t1,dbl t2,dbl cx,dbl cy) = 0;
	virtual void arcto(dbl x1,dbl y1,dbl x2,dbl y2,dbl rrr) = 0;
	virtual void beginclip(void)  = 0;
	virtual void bezier(dbl x1,dbl y1,dbl x2,dbl y2,dbl x3,dbl y3) = 0;
	virtual void box_fill(dbl x1, dbl y1, dbl x2, dbl y2) = 0;
	virtual void box_stroke(dbl x1, dbl y1, dbl x2, dbl y2, bool reverse) = 0;
	virtual void dochar(int font, int cc) = 0;
	virtual void resetfont() = 0;
	virtual void circle_fill(double zr) = 0;
	virtual void circle_stroke(double zr) = 0;
	virtual void clear(void) = 0;
	virtual void clip(void) = 0;
	virtual void closedev(void) = 0;
	virtual void closepath(void) = 0;
	virtual void dfont(char *c) = 0;
	virtual void ellipse_fill(double rx, double ry) = 0;
	virtual void ellipse_stroke(double rx, double ry) = 0;
	virtual void elliptical_arc(double rx,double ry,double t1,double t2,double cx,double cy) = 0;
	virtual void elliptical_narc(double rx,double ry,double t1,double t2,double cx,double cy) = 0;
	virtual void endclip(void)  = 0;
	virtual void fill(void) = 0;
	virtual void fill_ary(int nwk,double *wkx,double *wky) = 0;
	virtual void flush(void) = 0;
	// virtual void get_line_cap(int *i) = 0;
	virtual std::string get_type() = 0;
	virtual void line(double zx,double zy) = 0;
	virtual void line_ary(int nwk,double *wkx,double *wky) = 0;
	virtual void message(char *s) = 0;
	virtual void move(double zx,double zy) = 0;
	virtual void narc(dbl r,dbl t1,dbl t2,dbl cx,dbl cy) = 0;
	virtual void newpath(void) = 0;
	virtual void opendev(double width, double height, GLEFileLocation* outputfile, const string& inputfile) throw(ParserError) = 0;
	virtual void pscomment(char* ss) = 0;
	virtual void reverse(void)    /* reverse the order of stuff in the current path */ = 0;
	virtual void set_color(const GLERC<GLEColor>& color) = 0;
	virtual void set_fill(const GLERC<GLEColor>& fill) = 0;
	virtual void set_line_cap(int i) = 0;
	virtual void set_line_join(int i) = 0;
	virtual void set_line_miterlimit(double d) = 0;
	virtual void set_line_style(const char *s) = 0;
	virtual void set_line_styled(double dd) = 0;
	virtual void set_line_width(double w) = 0;
	virtual void set_matrix(double newmat[3][3]) = 0;
	virtual void set_path(int onoff) = 0;
	virtual void source(const char *s) = 0;
	virtual void stroke(void) = 0;
	virtual void set_color(void) = 0;
	virtual void set_fill(void) = 0;
	virtual void set_fill_method(int m);
	virtual void xdbox(double x1, double y1, double x2, double y2) = 0;
	virtual void devcmd(const char *s) = 0;
	virtual FILE* get_file_pointer(void) = 0;
	virtual int getDeviceType() = 0;
	virtual void bitmap(GLEBitmap* bitmap, GLEPoint* pos, GLEPoint* scale, int type);
	virtual void getRecordedBytes(string* output);
	void computeBoundingBox(double width, double height, int* int_bb_x, int* int_bb_y);
	void computeBoundingBox(double width, double height);
	inline GLEPoint* getBoundingBox() { return &m_boundingBox; }
	void setResolution(double resolution) { m_resolution = resolution; }
public:
	inline void setRecordingEnabled(bool rec) { m_recording = rec; }
	inline bool isRecordingEnabled() { return m_recording; }
};

class PSGLEDevice : public GLEDevice {
protected:
	GLEFileLocation m_OutputName;
	ostringstream* m_OutputBuffer;
	ofstream* m_OutputFile;
	ostream* m_Out;
	bool m_IsEps;
	bool m_IsPageSize;
	int ps_nvec;
	int first_ellipse;
	vector<string> comments;
	FILE *psfile;
	int i,l,j;
	int m_FillMethod;
	GLERC<GLEColor> m_currentColor;
	GLERC<GLEColor> m_currentFill;
public:
	PSGLEDevice(bool eps);
	virtual ~PSGLEDevice();
	virtual void arc(dbl r,dbl t1,dbl t2,dbl cx,dbl cy);
	virtual void arcto(dbl x1,dbl y1,dbl x2,dbl y2,dbl rrr);
	virtual void beginclip(void) ;
	virtual void bezier(dbl x1,dbl y1,dbl x2,dbl y2,dbl x3,dbl y3);
	virtual void box_fill(dbl x1, dbl y1, dbl x2, dbl y2);
	virtual void box_stroke(dbl x1, dbl y1, dbl x2, dbl y2, bool reverse);
	virtual void dochar(int font, int cc);
	virtual void resetfont();
	virtual void circle_fill(double zr);
	virtual void circle_stroke(double zr);
	virtual void clear(void);
	virtual void clip(void);
	virtual void closedev(void);
	virtual void closepath(void);
	virtual void dfont(char *c);
	virtual void ellipse_fill(double rx, double ry);
	virtual void ellipse_stroke(double rx, double ry);
	virtual void elliptical_arc(double rx,double ry,double t1,double t2,double cx,double cy);
	virtual void elliptical_narc(double rx,double ry,double t1,double t2,double cx,double cy);
	virtual void endclip(void) ;
	virtual void fill(void);
	virtual void fill_ary(int nwk,double *wkx,double *wky);
	virtual void flush(void);
	// virtual void get_line_cap(int *i);
	virtual std::string get_type();
	virtual void line(double zx,double zy);
	virtual void line_ary(int nwk,double *wkx,double *wky);
	virtual void message(char *s);
	virtual void move(double zx,double zy);
	virtual void narc(dbl r,dbl t1,dbl t2,dbl cx,dbl cy);
	virtual void newpath(void);
	virtual void opendev(double width, double height, GLEFileLocation* outputfile, const string& inputfile) throw(ParserError);
	virtual void pscomment(char* ss);
	virtual void reverse(void)    /* reverse the order of stuff in the current path */;
	virtual void set_color(const GLERC<GLEColor>& color);
	virtual void set_fill(const GLERC<GLEColor>& fill);
	virtual void set_line_cap(int i);
	virtual void set_line_join(int i);
	virtual void set_line_miterlimit(double d);
	virtual void set_line_style(const char *s);
	virtual void set_line_styled(double dd);
	virtual void set_line_width(double w);
	virtual void set_matrix(double newmat[3][3]);
	virtual void set_path(int onoff);
	virtual void source(const char *s);
	virtual void stroke(void);
	virtual void set_color(void);
	virtual void set_fill(void);
	virtual void set_fill_method(int m);
	virtual void xdbox(double x1, double y1, double x2, double y2);
	virtual void devcmd(const char *s);
	virtual FILE* get_file_pointer(void);
	virtual int getDeviceType();
	virtual void bitmap(GLEBitmap* bitmap, GLEPoint* pos, GLEPoint* scale, int type);
	virtual void getRecordedBytes(string* output);
protected:
	void set_color_impl(const GLERC<GLEColor>& color);
	void ddfill(GLERectangle* bounds = NULL);
	void shadeGLE();
	void shadeBoundedIfThenElse1(GLERectangle* bounds, double step1);
	void shadeBoundedIfThenElse2(GLERectangle* bounds, double step2);
	void shadeBounded(GLERectangle* bounds);
	void shadePostScript();
	void shade(GLERectangle* bounds);
	void read_psfont(void);
	void displayGeometry(double width, double height, int *gsPixelWidth, int *gsPixelHeight, int *gsPixelRes);
	void psFileASCIILine(const char* prefix, int cnt, char ch, bool nl);
public:
	inline bool isEps() { return m_IsEps; }
	inline bool isOutputPageSize() { return m_IsPageSize; }
	inline void setOutputPageSize(bool out) { m_IsPageSize = out; }
	inline ostream& out() { return *m_Out; }
	void startRecording();
	void initialPS();
};

#ifdef HAVE_CAIRO

#include <cairo.h>

class GLECairoDevice : public GLEDevice {
protected:
	GLEFileLocation m_OutputName;
	bool m_ShowError;
	double m_width;
	double m_height;
	cairo_surface_t *m_surface;
	cairo_t *m_cr;
	int m_FillMethod;
	GLERC<GLEColor> m_currentColor;
	GLERC<GLEColor> m_currentFill;
	std::vector<char> m_recorded;
	StringVoidPtrHash m_bitmapCache;
	std::vector<cairo_surface_t*> m_surfacesToDelete;
public:
	GLECairoDevice(bool showerror);
	virtual ~GLECairoDevice();
	virtual void arc(dbl r,dbl t1,dbl t2,dbl cx,dbl cy);
	virtual void arcto(dbl x1,dbl y1,dbl x2,dbl y2,dbl rrr);
	virtual void beginclip(void) ;
	virtual void bezier(dbl x1,dbl y1,dbl x2,dbl y2,dbl x3,dbl y3);
	virtual void box_fill(dbl x1, dbl y1, dbl x2, dbl y2);
	virtual void box_stroke(dbl x1, dbl y1, dbl x2, dbl y2, bool reverse);
	virtual void dochar(int font, int cc);
	virtual void resetfont();
	virtual void circle_fill(double zr);
	virtual void circle_stroke(double zr);
	virtual void clear(void);
	virtual void clip(void);
	virtual void closedev(void);
	virtual void closepath(void);
	virtual void dfont(char *c);
	virtual void ellipse_fill(double rx, double ry);
	virtual void ellipse_stroke(double rx, double ry);
	virtual void elliptical_arc(double rx,double ry,double t1,double t2,double cx,double cy);
	virtual void elliptical_narc(double rx,double ry,double t1,double t2,double cx,double cy);
	virtual void endclip(void) ;
	virtual void fill(void);
	virtual void fill_ary(int nwk,double *wkx,double *wky);
	virtual void flush(void);
	virtual std::string get_type();
	virtual void line(double zx,double zy);
	virtual void line_ary(int nwk,double *wkx,double *wky);
	virtual void message(char *s);
	virtual void move(double zx,double zy);
	virtual void narc(dbl r,dbl t1,dbl t2,dbl cx,dbl cy);
	virtual void newpath(void);
	virtual void opendev(double width, double height, GLEFileLocation* outputfile, const string& inputfile) throw(ParserError);
	virtual void pscomment(char* ss);
	virtual void reverse(void)    /* reverse the order of stuff in the current path */;
	virtual void set_color(const GLERC<GLEColor>& color);
	virtual void set_fill(const GLERC<GLEColor>& fill);
	virtual void set_fill_method(int m);
	virtual void set_line_cap(int i);
	virtual void set_line_join(int i);
	virtual void set_line_miterlimit(double d);
	virtual void set_line_style(const char *s);
	virtual void set_line_styled(double dd);
	virtual void set_line_width(double w);
	virtual void set_matrix(double newmat[3][3]);
	virtual void set_path(int onoff);
	virtual void source(const char *s);
	virtual void stroke(void);
	virtual void set_color(void);
	virtual void set_fill(void);
	virtual void xdbox(double x1, double y1, double x2, double y2);
	virtual void devcmd(const char *s);
	virtual FILE* get_file_pointer(void);
	virtual int getDeviceType();
	virtual void bitmap(GLEBitmap* bitmap, GLEPoint* pos, GLEPoint* scale, int type);
	virtual void getRecordedBytes(string* output);
	void recordData(const unsigned char *data, unsigned int length);
protected:
	void set_color_impl(const GLERC<GLEColor>& color);
	void ddfill(GLERectangle* bounds = NULL);
	void shadePattern();
	void shadeGLE();
	void shadeBoundedIfThenElse1(GLERectangle* bounds, double p, double step1);
	void shadeBoundedIfThenElse2(GLERectangle* bounds, double p, double step2);
	void shadeBounded(GLERectangle* bounds);
	void shade(GLERectangle* bounds);
	void clearRecordedData();
private:
	cairo_surface_t* bitmapCreateSurface(GLEBitmap* bitmap);
};

class GLECairoDevicePDF : public GLECairoDevice {
public:
	GLECairoDevicePDF(bool showerror);
	virtual ~GLECairoDevicePDF();
	virtual void opendev(double width, double height, GLEFileLocation* outputfile, const string& inputfile) throw(ParserError);
	virtual int getDeviceType();
};

class GLECairoDeviceEPS : public GLECairoDevice {
public:
	GLECairoDeviceEPS(bool showerror);
	virtual ~GLECairoDeviceEPS();
	virtual void opendev(double width, double height, GLEFileLocation* outputfile, const string& inputfile) throw(ParserError);
	virtual int getDeviceType();
	virtual void getRecordedBytes(string* output);
};

class GLECairoDeviceSVG : public GLECairoDevice {
public:
	GLECairoDeviceSVG(bool showerror);
	virtual ~GLECairoDeviceSVG();
	virtual void opendev(double width, double height, GLEFileLocation* outputfile, const string& inputfile) throw(ParserError);
	virtual int getDeviceType();
};

#ifdef __WIN32__

class GLECairoDeviceEMFWinInfo;

class GLECairoDeviceEMF : public GLECairoDevice {
protected:
	GLECairoDeviceEMFWinInfo* m_WinInfo;
	double m_DPI;
	bool m_CopyClipboard;
public:
	GLECairoDeviceEMF(bool showerror);
	virtual ~GLECairoDeviceEMF();
	virtual void set_matrix(double newmat[3][3]);
	virtual void opendev(double width, double height, GLEFileLocation* outputfile, const string& inputfile) throw(ParserError);
	virtual int getDeviceType();
	virtual void closedev(void);
	inline void setDPI(double d) { m_DPI = d; }
	inline void setCopyClipboard(bool c) { m_CopyClipboard = true; }
};

#endif

#endif

class GLEDummyDevice : public GLEDevice {
protected:
	bool m_ShowError;
public:
	GLEDummyDevice(bool showerror);
	virtual ~GLEDummyDevice();
	virtual void arc(dbl r,dbl t1,dbl t2,dbl cx,dbl cy);
	virtual void arcto(dbl x1,dbl y1,dbl x2,dbl y2,dbl rrr);
	virtual void beginclip(void) ;
	virtual void bezier(dbl x1,dbl y1,dbl x2,dbl y2,dbl x3,dbl y3);
	virtual void box_fill(dbl x1, dbl y1, dbl x2, dbl y2);
	virtual void box_stroke(dbl x1, dbl y1, dbl x2, dbl y2, bool reverse);
	virtual void dochar(int font, int cc);
	virtual void resetfont();
	virtual void circle_fill(double zr);
	virtual void circle_stroke(double zr);
	virtual void clear(void);
	virtual void clip(void);
	virtual void closedev(void);
	virtual void closepath(void);
	virtual void dfont(char *c);
	virtual void ellipse_fill(double rx, double ry);
	virtual void ellipse_stroke(double rx, double ry);
	virtual void elliptical_arc(double rx,double ry,double t1,double t2,double cx,double cy);
	virtual void elliptical_narc(double rx,double ry,double t1,double t2,double cx,double cy);
	virtual void endclip(void) ;
	virtual void fill(void);
	virtual void fill_ary(int nwk,double *wkx,double *wky);
	virtual void flush(void);
	virtual std::string get_type();
	virtual void line(double zx,double zy);
	virtual void line_ary(int nwk,double *wkx,double *wky);
	virtual void message(char *s);
	virtual void move(double zx,double zy);
	virtual void narc(dbl r,dbl t1,dbl t2,dbl cx,dbl cy);
	virtual void newpath(void);
	virtual void opendev(double width, double height, GLEFileLocation* outputfile, const string& inputfile) throw(ParserError);
	virtual void pscomment(char* ss);
	virtual void reverse(void)    /* reverse the order of stuff in the current path */;
	virtual void set_color(const GLERC<GLEColor>& color);
	virtual void set_fill(const GLERC<GLEColor>& fill);
	virtual void set_line_cap(int i);
	virtual void set_line_join(int i);
	virtual void set_line_miterlimit(double d);
	virtual void set_line_style(const char *s);
	virtual void set_line_styled(double dd);
	virtual void set_line_width(double w);
	virtual void set_matrix(double newmat[3][3]);
	virtual void set_path(int onoff);
	virtual void source(const char *s);
	virtual void stroke(void);
	virtual void set_color(void);
	virtual void set_fill(void);
	virtual void xdbox(double x1, double y1, double x2, double y2);
	virtual void devcmd(const char *s);
	virtual FILE* get_file_pointer(void);
	virtual int getDeviceType();
};

#ifdef HAVE_X11

#ifdef VMS
 #include <decw$include/Xlib.h>
 #include <decw$include/Xutil.h>
#else
 #ifdef aix
  #define NeedFunctionPrototypes 0
 #endif
 #include <X11/Xlib.h>
 #include <X11/Xutil.h>
#endif

#define NUM_COLTABLE_ENTRIES 34
#define X11_PATH_LENGTH 500

struct X11Pnt {int type,x,y;} ;

class X11GLEDevice : public GLEDevice {
protected:
	int color_table[NUM_COLTABLE_ENTRIES];
	int maxxsize;
	int maxysize;
	int window1W;
	int window1H;
	Display *dpy;
	Window window1;
	GC gc,gcf;
	Screen *screen;
	int doesbackingstore;
	double savexsize,saveysize;
	int gle_nspeed; /* text mode = slow and fast */
	int i,l,j,ix,iy;
	double f;
	double xsizecm,ysizecm;
	double d_scale, d_xscale, d_yscale;
	int d_graphmode;
	int d_fillstyle,d_fillcolor;
	int d_lstyle,d_lwidth;
	int d_maxy;
	int safnt;
	struct X11Pnt pnts[X11_PATH_LENGTH];
	int npnts;
	int startx,starty;
public:
	X11GLEDevice();
	virtual ~X11GLEDevice();
	virtual void arc(dbl r,dbl t1,dbl t2,dbl cx,dbl cy);
	virtual void arcto(dbl x1,dbl y1,dbl x2,dbl y2,dbl rrr);
	virtual void beginclip(void) ;
	virtual void bezier(dbl x1,dbl y1,dbl x2,dbl y2,dbl x3,dbl y3);
	virtual void box_fill(dbl x1, dbl y1, dbl x2, dbl y2);
	virtual void box_stroke(dbl x1, dbl y1, dbl x2, dbl y2, bool reverse);
	virtual void dochar(int font, int cc);
	virtual void resetfont();
	virtual void circle_fill(double zr);
	virtual void circle_stroke(double zr);
	virtual void clear(void);
	virtual void clip(void);
	virtual void closedev(void);
	virtual void closepath(void);
	virtual void dfont(char *c);
	virtual void ellipse_fill(double rx, double ry);
	virtual void ellipse_stroke(double rx, double ry);
	virtual void elliptical_arc(double rx,double ry,double t1,double t2,double cx,double cy);
	virtual void elliptical_narc(double rx,double ry,double t1,double t2,double cx,double cy);
	virtual void endclip(void) ;
	virtual void fill(void);
	virtual void fill_ary(int nwk,double *wkx,double *wky);
	virtual void flush(void);
	// virtual void get_line_cap(int *i);
	virtual std::string get_type();
	virtual void line(double zx,double zy);
	virtual void line_ary(int nwk,double *wkx,double *wky);
	virtual void message(char *s);
	virtual void move(double zx,double zy);
	virtual void narc(dbl r,dbl t1,dbl t2,dbl cx,dbl cy);
	virtual void newpath(void);
	virtual void opendev(double width, double height, GLEFileLocation* outputfile, const string& inputfile) throw(ParserError);
	virtual void pscomment(char* ss);
	virtual void reverse(void)    /* reverse the order of stuff in the current path */;
	virtual void set_color(const GLERC<GLEColor>& color);
	virtual void set_fill(const GLERC<GLEColor>& fill);
	virtual void set_line_cap(int i);
	virtual void set_line_join(int i);
	virtual void set_line_miterlimit(double d);
	virtual void set_line_style(const char *s);
	virtual void set_line_styled(double dd);
	virtual void set_line_width(double w);
	virtual void set_matrix(double newmat[3][3]);
	virtual void set_path(int onoff);
	virtual void source(const char *s);
	virtual void stroke(void);
	virtual void set_color(void);
	virtual void set_fill(void);
	virtual void xdbox(double x1, double y1, double x2, double y2);
	virtual void devcmd(const char *s);
	virtual FILE* get_file_pointer(void);
	virtual int getDeviceType();
protected:
	void setcolor(int);
	void setfillcolor(int);
	void setfillstyle(int);
	double getmaxx();
	double getmaxy();
	int getmaxcolor();
	void doInitialize();
	int  doDefineColor(int n);
	void doCreateWindows();
	void doCreateGraphicsContext();
	void doLoadFont();
	void doExpose();
	void doWMHints();
	void doMapWindows();
	void openDisplay();
	void path_move(int x, int y);
	void path_line(int x, int y);
	void path_close(void);
	void path_fill(void);
	void path_newpath(void);
	void path_stroke(void);
	int set_expose(void);
	int wait_expose(void);
	void dxy(double x, double y, int *dx, int *dy);
	void rxy(double x, double y, int *dx, int *dy);
	void dxy_short(double x, double y, short *dx, short *dy);
	void rxy_short(double x, double y, short *dx, short *dy);
};

#endif
#endif
