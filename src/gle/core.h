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

#ifndef INC_CORE_H
#define INC_CORE_H

#include "color.h"
#include "gle-interface/gle-interface.h"
#include "d_interface.h"

#define GLE_PAPER_UNKNOWN  0
#define GLE_PAPER_A0       1
#define GLE_PAPER_A1       2
#define GLE_PAPER_A2       3
#define GLE_PAPER_A3       4
#define GLE_PAPER_A4       5
#define GLE_PAPER_LETTER   6

#define PDF_IMG_COMPR_AUTO 0
#define PDF_IMG_COMPR_ZIP  1
#define PDF_IMG_COMPR_JPEG 2
#define PDF_IMG_COMPR_PS   3

#define GLE_COMPAT_35          0x030500
#define GLE_COMPAT_MOST_RECENT 0x040200

#define GLEC_MAX_INT       1
#define GLEC_COMPATIBILITY 0

#define GLEC_MAX_DOUBLE    6
#define GLEC_TITLESCALE    0
#define GLEC_ATITLESCALE   1
#define GLEC_ALABELSCALE   2
#define GLEC_TICKSSCALE    3
#define GLEC_ATITLEDIST    4
#define GLEC_ALABELDIST    5

#define GLE_ARRSTY_SIMPLE 0
#define GLE_ARRSTY_FILLED 1
#define GLE_ARRSTY_EMPTY  2
#define GLE_ARRSTY_OLD35  3
#define GLE_ARRSTY_SUB    10

#define GLE_ARRTIP_ROUND 0
#define GLE_ARRTIP_SHARP 1

class GLEBitmap;
class GLEPcodeList;

class GLEArrowProps {
public:
	int style;
	int tip;
	double size;
	double angle;
};

struct gmodel {
	double image[3][3];
	double fontn,fontsz;	/* up to here for font caching */
	GLERC<GLEColor> color;
	GLERC<GLEColor> fill;
	double lwidth,lstyled,curx,cury;
	double endx,endy;
	double miterlimit;
	int lcap,ljoin;
	int just,xinline,npath; /* up to here for STATE */
	bool inpath;
	char lstyle [9];
	/* 18*8, 6*int, 2*int */
	double xmin,xmax,ymin,ymax;	 /* bounds in USER coordinates */
	double startx,starty;
	double closex,closey;	     /* for closepath */
	double arrowsize, arrowangle;
	double userwidth,userheight; /* The user req size */
	double pagewidth,pageheight;
	double topmargin, bottommargin;
	double leftmargin, rightmargin;
	int arrowstyle;
	int arrowtip;
	bool texlabels;
	bool fullpage;
	bool rotate;
	bool landscape;
	int papersize;
	bool hasbox;
	bool isopen;
	int devtype;
	GLEDevice* dev;
	bool console_output;
	bool needs_newline;
	int pdfimgformat;
	int intconst[GLEC_MAX_INT];
	double floatconst[GLEC_MAX_DOUBLE];
};

class GLESaveRestore {
protected:
	gmodel* model;
public:
	GLESaveRestore();
	~GLESaveRestore();
	void save();
	void restore();
};

class GLEMeasureBox : public GLERectangle {
public:
	GLEMeasureBox();
	~GLEMeasureBox();
	void measureStart();
	void measureEnd();
	void measureEndIgnore();
};

void g_init_arrow_head(GLECurvedArrowHead* head, bool startend);

class GLEPolynomial {
protected:
	double* m_Coefs;
	int m_Degree;
public:
	GLEPolynomial(double* coefs, int degree);
	~GLEPolynomial();
	double evalPoly(double t);
	double evalDPoly(double t);
	double newtonRaphson(double t, double prec = 1e-9);
	void horner(double v);
	void print();
	inline void setDegree(int deg) { m_Degree = deg; }
	inline int degree() { return m_Degree; }
	inline double a(int i) { return m_Coefs[i]; }
	inline void set(int i, double v) { m_Coefs[i] = v; }
};

class GLECore {
public:
	GLECore();
	~GLECore();

	void reset();
	bool isComputingLength() const;
	void setComputingLength(bool computingLength);
	double getTotalLength() const;
	void setTotalLength(double length);
	void addToLength(double value);
	bool isShowNoteAboutFallback() const;
	void setShowNoteAboutFallback(bool value);

private:
	bool m_isComputingLength;
	double m_totalLength;
	bool m_showNoteAboutFallback;
};

class GLEWithoutUpdates {
public:
	GLEWithoutUpdates();
	~GLEWithoutUpdates();

private:
	GLECore* m_core;
	bool m_isComputingLength;
};

unsigned int coreleft();
unsigned int farcoreleft();

#define GLE_FILL_METHOD_DEFAULT 0
#define GLE_FILL_METHOD_GLE 1
#define GLE_FILL_METHOD_POSTSCRIPT 2

#define SIZEOFSTATE sizeof(gmodel)

void g_init();
void g_get_version(string* version);
void g_get_build_date(string* date);
string g_get_version();
string g_get_version_nosnapshot();
void g_open(GLEFileLocation* outputfile, const string& inputfile);
void g_get_font(int *f);
//void g_close(void);
void g_defmarker(const char *mname, const char *font, int ccc, double dx, double dy, double sz, int autodx);
void g_get_hei(double *h);
//void g_gsave(void);
void g_hint(char *s);
void g_marker(int i, double sz);
void g_marker2(int i, double sz, double dval) throw (ParserError);
void g_set_hei(double h);
bool gclip(double *x1,double *y1, double *x2, double *y2,double xmin, double ymin, double xmax, double ymax);
void g_pscomment(char* ss);
void g_psbbtweak(void);
void g_message(const string& s);
void g_message_first_newline(bool newline);
bool g_has_console_output();
void g_set_console_output(bool set);
void g_get_usersize(double *x,double *y);
void g_get_userbox_undev(GLERectangle* rect);
GLECore* g_get_core();

//
// -- no need to ifdef these
// they will simply print warning message if libraries are not linked
//
void g_bitmap(string& fname, double wx, double wy, int type) throw(ParserError);
void g_bitmap_info(string& fname, int xvar, int yvar, int type) throw(ParserError);
int g_bitmap_string_to_type(const std::string& stype);
int g_bitmap_string_to_type(const char* stype);
string g_bitmap_supported_types();

void g_ellipse_stroke(double rx, double ry);
void g_ellipse_fill(double rx, double ry);
void g_elliptical_arc(double rx, double ry, double t1, double t2, double cx, double cy, int arrow);
void g_elliptical_narc(double rx, double ry, double t1, double t2, double cx, double cy, int arrow);

void g_arrowsize(GLEArrowProps* arrow);
void g_arrowline(double x2, double y2, int flag, int can_fillpath) throw(ParserError);

void g_arrowpoints(double cx,double cy,double dx,double dy, double *ax1,
                   double *ay1,double *ax2,double *ay2, double *nnx, double *nny);
void g_psarrow(double x1, double y1, double x2, double y2, int flag);

double g_arc_normalized_angle2(double a1, double a2);
void g_arc(double r, double t1, double t2, double cx, double cy, int arrow);
void g_narc(double r, double t1, double t2, double cx, double cy, int arrow);

bool g_is_bbtweak();
double g_get_angle_deg();
void g_get_scale(double* sx, double* sy);
double g_get_avg_scale();

const char* g_device_to_ext(int device);
GLEDevice* g_select_device(int device);
GLEDevice* g_get_device_ptr();
GLEDevice* g_set_dummy_device();
void g_restore_device(GLEDevice* device);
int g_get_device();
bool g_is_dummy_device();
void g_resetfont();

void g_arrow(double dx, double dy, int can_fillpath) throw(ParserError);
void g_arrowcurve(double x, double y, int arrow, double a1, double a2, double d1, double d2);
bool g_has_size();
void g_set_size(double width, double height, bool box);
int  g_papersize_type(const string& papersize);
void g_set_pagesize(int type);
void g_set_pagesize(double width, double height);
void g_set_margins(double top, double bottom, double left, double right);
void g_set_fullpage(bool fullpage);
void g_set_rotate_fullpage(bool rotfp);
bool g_is_rotate_fullpage();
void g_on_open();
bool g_is_fullpage();
void g_set_landscape(bool landscape);
bool g_is_landscape();
void g_set_arrow_size(double size);
void g_set_arrow_angle(double angle);
void g_set_pdf_image_format(const char* format);
int g_get_pdf_image_format();

int g_get_compatibility();
void g_set_compatibility(int compat);
int g_set_compatibility(const string& compat) throw (ParserError);
int g_parse_compatibility(const string& compat) throw (ParserError);
void g_compatibility_settings();

void g_set_iconst(int i, int value);
void g_set_fconst(int i, double value);
int g_get_iconst(int i);
double g_get_fconst(int i);

void g_update_bounds_box(GLERectangle* box);
void g_get_bounds(GLERectangle* rect);
void g_set_bounds(GLERectangle* rect);
void g_update_bounds(GLERectangle* rect);
void g_restore_bounds(double xmin, double xmax, double ymin, double ymax);
void g_set_bounds(double x, double y, gmodel* model);
void g_debug_bounds(const char* out);
void g_check_bounds(const char* after);
bool g_has_box(gmodel* model);
void g_set_state(gmodel *s);
void g_get_state(gmodel *s);
void g_rset_pos(double x, double y);

void g_undev(double ux, double uy, double *x, double *y, gmodel* model);
void g_dev(GLEPoint* pt);
void g_dev_rel(GLEPoint* pt);
void g_dev(GLERectangle* rect);
void g_undev(GLERectangle* rect);
void g_undev(GLERectangle* rect, gmodel* g);
void g_set_arrow_style(const char* shape) throw (ParserError);
void g_set_arrow_tip(const char* tip) throw (ParserError);

void g_bitmap(GLEBitmap*, double wx, double wy, int type) throw(ParserError);

void g_set_tex_scale(const char* ss);
void g_set_tex_labels(bool onoff);
bool g_get_tex_labels();

void g_restore_defaults();

int g_arrow_style();
int g_arrow_tip();

GLEPoint g_get_xy();
void g_get_xy(GLEPoint* pt);
void g_move(const GLEPoint& pt);
void g_clear_matrix();
void g_set_color(const GLERC<GLEColor>& color);
void g_set_fill(int fill);
void g_set_fill(const GLERC<GLEColor>& fill);
bool g_is_filled(void);
void g_set_fill_pattern(const GLERC<GLEColor>& pattern);
void g_set_partial_state(gmodel* s);

unsigned char float_to_color_comp(double value);
unsigned char float_to_color_comp_255(double value);

int g_hash_string_to_color(const string& str, colortyp* c);
int g_verbosity();

int font_get_encoding(int ff);
void d_devcmd(char *s);
void g_arcto(dbl x1,dbl y1,dbl x2,dbl y2,dbl rrr);
void g_beginclip(void);
void g_bezier(dbl x1,dbl y1,dbl x2,dbl y2,dbl x3,dbl y3);
void g_bitmap(char *bmap);
void g_box_fill(GLERectangle* rect);
void g_box_fill(dbl x1, dbl y1, dbl x2, dbl y2);
void g_box_stroke(dbl x1, dbl y1, dbl x2, dbl y2, bool reverse = false);
void g_box_stroke(GLERectangle* rect, bool reverse = false);
void g_char(int font, int cc);
void g_circle_fill(double zr);
void g_circle_stroke(double zr);
void g_clear(void);
void g_clip(void);
void g_close(void);
void g_closepath(void);
void g_curve(GLEPcodeList* pclist, int *pcode);
void g_dbezier(dbl x1,dbl y1,dbl x2,dbl y2,dbl x3,dbl y3);
void g_dev(double x, double y,double *xd,double *yd);
void g_devcmd(const char *s);
void g_dfont(const std::string& s);
void g_dline(double x, double y);
void g_dmove(double x, double y);
void g_dojust(dbl *x1, dbl *y1, dbl *x2, dbl *y2, int jj);
void g_dotjust(dbl *x1, dbl *y1, dbl l, dbl r, dbl u, dbl d, int jj);
void g_endclip(void);
void g_fill(void);
void g_fill_ary(int nwk,double *wkx,double *wky);
void g_flush(void);
void g_get_bounds(dbl *x1,dbl *y1,dbl *x2,dbl *y2);
GLERC<GLEColor> g_get_color();
GLERC<GLEColor> g_get_fill_clear();
GLERC<GLEColor> g_get_color_hex(int hexValue);
void g_get_devsize(dbl *x,dbl *y);
void g_get_end(dbl *x,dbl *y);
GLERC<GLEColor> g_get_fill();
void g_get_just(int *j);
void g_get_line_cap(int *jj);
void g_get_line_join(int *jj);
void g_get_line_style(char *s);
void g_get_line_styled(double *w);
void g_get_line_width(double *w);
void g_get_path(int *pathonoff);
std::string g_get_type();
void g_get_usersize(dbl *x,dbl *y);
void g_get_pagesize(dbl *x,dbl *y,int *type);
void g_get_pagesize(dbl *x,dbl *y);
void g_get_xy(double *x,double *y);
void g_grestore(void);
void g_gsave(void);
void g_init_bounds(void);
void g_jtext(int just);
void g_line(const GLEPoint& p);
void g_line(double zx,double zy);
void g_line_ary(int nwk,double *wkx,double *wky);
void g_marker_def(char *name, char *subname);
// void g_measure(const char *s, dbl *l, dbl *r, dbl *u, dbl *d);
void g_measure(const string& s, dbl *l, dbl *r, dbl *u, dbl *d);
void g_message(const char *s);
bool g_reset_message();
void g_move(double zx,double zy);
void g_set_pos(double zx,double zy);
void g_newpath(void);
void g_postscript(char *ss,double w,double h) throw (ParserError);
bool g_parse_ps_boundingbox(const string& line, int* bx1, int* by1, int* bx2, int* by2);
void g_rdev(double x, double y,double *xd,double *yd);
void g_reverse(void);
void g_rline(double zx,double zy);
void g_rmove(double zx,double zy);
void g_rotate(double ar);
void g_rundev(double x, double y,double *xd,double *yd);
void g_scale(double sx,double sy);
void g_update_bounds(double x,double y);
void g_set_color(int l);
void g_set_background(const GLERC<GLEColor>& color);
void g_set_end(dbl x,dbl y);
void g_set_hei(double h);
void g_set_fill_method(const char* meth);
void g_set_font(int j);
void g_set_font_width(double v);
void g_set_just(int j);
void g_set_line_cap(int i);
void g_set_line_join(int i);
void g_set_line_miterlimit(double d);
void g_set_line_style(const char *s);
void g_set_line_styled(double x);
void g_set_line_width(double w);
void g_set_matrix(double newmat[3][3]);
void g_set_path(int onoff);
void g_set_xy(double x, double y);
void g_stroke(void);
void g_text(const string& s);
void g_translate(double ztx,double zty);
void g_undev(double ux,double uy, double *x,double *y);
void my_char(int ff, int cc);
int g_device_to_bitmap_type(int device);
bool g_bitmap_supports_type(int type);
int g_font_fallback(int font);
std::vector<std::string> g_create_device_string();

#endif
