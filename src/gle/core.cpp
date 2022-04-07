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

#define FONTDEF extern

#include "all.h"
#include "file_io.h"
#include "bitmap/img2ps.h"
#include "core.h"
#include "d_interface.h"
#include "justify.h"
#include "gprint.h"
#include "var.h"
#include "sub.h"
#include "cutils.h"
#include "keyword.h"
#include "run.h"
#include "color.h"
#include "texinterface.h"
#include "tokens/stokenizer.h"
#include "gle-interface/gle-interface.h"
#include "font.h"
#include "cmdline.h"
#include "config.h"
#include "glearray.h"
#include "polish.h"

void g_graph_init();
void dis_mat(char *s,double m[3][3]);

int mystrncmp(char *a, char *b, int n);
gmodel g;
double tmpimg[3][3];
double tmpimg2[3][3];
double font_lwidth;

static int ngsave;
static gmodel *gsave[100];

int g_compatibility = GLE_COMPAT_MOST_RECENT;

/*---------------------------------------------------------------------------*/

#define dbg if ((gle_debug & 32)>0)
extern int gle_debug;

/*-----------------------------------------------*/
/* The global variables that CORE keeps track of */
/*-----------------------------------------------*/

void test_unit(void);
void clean_surface();
void g_arrowsize_actual(GLEArrowProps* arrow, double* lwd, bool sz_az);
void g_arrowsize_transform(GLEArrowProps* arrow, double lwd, bool sz_az);
void g_set_color_to_device();
void g_set_fill_to_device();

int gunit=false;

GLERectangle g_UserBoxDev;
GLECore g_core;

unsigned int coreleft() {
	return 4000000;
}

unsigned int farcoreleft() {
	return 1200000;
}

GLECore::GLECore()
{
}

GLECore::~GLECore() {
}

void GLECore::reset() {
	m_isComputingLength = false;
	m_totalLength = 0.0;
	m_showNoteAboutFallback = true;
}

bool GLECore::isComputingLength() const {
	return m_isComputingLength;
}

void GLECore::setComputingLength(bool computingLength) {
	m_isComputingLength = computingLength;
}

double GLECore::getTotalLength() const {
	return m_totalLength;
}

void GLECore::setTotalLength(double length) {
	m_totalLength = length;
}

void GLECore::addToLength(double value) {
	m_totalLength += value;
}

bool GLECore::isShowNoteAboutFallback() const {
	return m_showNoteAboutFallback;
}

void GLECore::setShowNoteAboutFallback(bool value) {
	m_showNoteAboutFallback = value;
}

GLEWithoutUpdates::GLEWithoutUpdates() {
	m_core = g_get_core();
	m_isComputingLength = m_core->isComputingLength();
	m_core->setComputingLength(false);
}

GLEWithoutUpdates::~GLEWithoutUpdates() {
	m_core->setComputingLength(m_isComputingLength);
}

GLECore* g_get_core() {
	return &g_core;
}

// Called only once g_clear is called for each script.
void g_init() {
	clean_surface();
	g.devtype = GLE_DEVICE_NONE;
	g.dev = NULL;
	g.console_output = false;
	g.needs_newline = true;
	g_graph_init();
}

void g_get_version(string* version) {
	*version = GLEVN;
	#ifdef GLE_SNAPSHOT
		*version += GLE_SNAPSHOT;
	#endif
}

void g_get_build_date(string* date) {
	unsigned long epoch;
	char *c_time_string;
	date->resize(0);
#if defined(DEBIAN_EPOCH)
	// convert seconds since 1970 into date string
	const time_t build = DEBIAN_EPOCH;
	struct tm *tm = gmtime(&build);
	char buf[20];
	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm);
	string bdate(buf);
#else
  #if defined(__TIME__) && defined(__DATE__)
	string bdate(__DATE__);
	bdate += " "; bdate += __TIME__;
	str_replace_all(bdate, "  ", " ");
  #endif
#endif
	*date = bdate;
}

string g_get_version() {
	string version;
	g_get_version(&version);
	return version;
}

string g_get_version_nosnapshot() {
	return string(GLEVN);
}

const char* g_device_to_ext(int device) {
	switch (device) {
		case GLE_DEVICE_EPS:
		case GLE_DEVICE_CAIRO_EPS:
			return ".eps";
		case GLE_DEVICE_PS:
		case GLE_DEVICE_CAIRO_PS:
			return ".ps";
		case GLE_DEVICE_PDF:
		case GLE_DEVICE_CAIRO_PDF:
			return ".pdf";
		case GLE_DEVICE_SVG:
		case GLE_DEVICE_CAIRO_SVG:
			return ".svg";
		case GLE_DEVICE_JPEG:
			return ".jpg";
		case GLE_DEVICE_PNG:
			return ".png";
		default:
			return "";
	}
}

GLEDevice* g_select_device(int device) {
	g.devtype = device;
	if (g.dev != NULL) {
		delete g.dev;
		g.dev = NULL;
	}
	switch (device) {
		case GLE_DEVICE_PS:
			g.dev = new PSGLEDevice(false);
			break;
		case GLE_DEVICE_EPS:
			g.dev = new PSGLEDevice(true);
			break;
		case GLE_DEVICE_DUMMY:
			g.dev = new GLEDummyDevice(false);
			break;
#ifdef HAVE_CAIRO
        case GLE_DEVICE_CAIRO_PDF:
            g.dev = new GLECairoDevicePDF(false);
            break;
        case GLE_DEVICE_CAIRO_EPS:
            g.dev = new GLECairoDeviceEPS(false);
            break;
        case GLE_DEVICE_CAIRO_SVG:
            g.dev = new GLECairoDeviceSVG(false);
            break;
#ifdef __WIN32__
        case GLE_DEVICE_EMF:
            g.dev = new GLECairoDeviceEMF(false);
            break;
#endif
#endif
#ifdef HAVE_X11
		case GLE_DEVICE_X11:
			g.dev = new X11GLEDevice();
			break;
#endif
	}
	return g.dev;
}

GLEDevice* g_set_dummy_device() {
	g_flush();
	GLEDevice* previous = g.dev;
	g.dev = new GLEDummyDevice(false);
	g.devtype = GLE_DEVICE_DUMMY;
	return previous;
}

void g_restore_device(GLEDevice* device) {
	if (device != NULL) {
		g_flush();
		delete g.dev;
		g.dev = device;
		g.devtype = device->getDeviceType();
	}
}

GLEDevice* g_get_device_ptr() {
	return g.dev;
}

int g_get_device() {
	return g.devtype;
}

bool g_is_dummy_device() {
	return g.devtype == GLE_DEVICE_DUMMY;
}

void g_devcmd(const char *s) {
	g.dev->devcmd(s);
}

void g_resetfont() {
	g.dev->resetfont();
}

void g_dfont(const std::string& s) {
	g.dev->dfont((char*)s.c_str());
}

GLERC<GLEColor> g_get_fill_clear() {
	GLERC<GLEColor> color(new GLEColor());
	color->setTransparent(true);
	return color;
}

GLERC<GLEColor> g_get_color_hex(int hexValue) {
	GLERC<GLEColor> color(new GLEColor());
	color->setHexValue(hexValue);
	return color;
}

void g_hint(char *s) {
	gprint("%s\n",s);
}

void g_message(const string& s) {
	g_message(s.c_str());
}

void g_message(const char *s) {
	GLEInterface* iface = GLEGetInterfacePointer();
	if (!g.console_output) {
		g.console_output = true;
		if (g.needs_newline) iface->getOutput()->println();
	}
	iface->getOutput()->println(s);
}

bool g_has_console_output() {
	return g.console_output;
}

void g_set_console_output(bool set) {
	g.console_output = set;
}

bool g_reset_message() {
	bool res = g.console_output;
	if (g.console_output) {
		cerr << endl;
	}
	g.console_output = false;
	return res;
}

void g_message_first_newline(bool newline) {
	g.needs_newline = newline;
}

void g_get_end(dbl *x,dbl *y) {
	*x = tex_xend();
	*y = tex_yend();
}

void g_set_end(dbl x,dbl y) {
	gprint("What is setting end \n");
}

void g_get_usersize(dbl *x,dbl *y) {
	*x = g.userwidth;
	*y = g.userheight;
}

void g_get_pagesize(dbl *x,dbl *y) {
	*x = g.pagewidth;
	*y = g.pageheight;
}

void g_get_pagesize(dbl *x, dbl *y, int *type) {
	*x = g.pagewidth;
	*y = g.pageheight;
	*type = g.papersize;
}

void g_get_bounds(dbl *x1,dbl *y1,dbl *x2,dbl *y2) {
	*x1 = g.xmin;
	*y1 = g.ymin;
	*x2 = g.xmax;
	*y2 = g.ymax;
}

void g_init_bounds() {
	g.xmin = 1e30;
	g.ymin = 1e30;
	g.xmax = -1e30;
	g.ymax = -1e30;
}

std::string g_get_type() {
	return g.dev->get_type();
}

void g_set_path(int onoff) {
	if (static_cast<bool>(onoff)==g.inpath) return;
	g_flush();
	if (static_cast<bool>(onoff) == true) {
		g.inpath=true;
		g.npath = 0;
		g.xinline=false;
	} else {
		g.inpath = false;
		g.xinline = false;
	}
	g.dev->set_path(onoff);
}

void g_get_path(int *onoff) {
	*onoff = g.inpath;
}

void g_newpath() {
	g.npath = 0;
	g.dev->newpath();
}

void g_set_size(double width, double height, bool box) {
	g.userwidth  = width;
	g.userheight = height;
	g.hasbox = box;
}

bool g_has_size() {
	return g.userwidth > 0 && g.userheight > 0;
}

void g_set_pagesize(double width, double height) {
	g.pagewidth = width;
	g.pageheight = height;
	g.papersize = GLE_PAPER_UNKNOWN;
}

void g_set_margins(double top, double bottom, double left, double right) {
	g.topmargin = top; g.bottommargin = bottom;
	g.leftmargin = left; g.rightmargin = right;
}

int g_papersize_type(const string& papersize) {
	if (papersize == "a0paper") {
		return GLE_PAPER_A0;
	} else if (papersize == "a1paper") {
		return GLE_PAPER_A1;
	} else if (papersize == "a2paper") {
		return GLE_PAPER_A2;
	} else if (papersize == "a3paper") {
		return GLE_PAPER_A3;
	} else if (papersize == "a4paper") {
		return GLE_PAPER_A4;
	} else if (papersize == "letterpaper") {
		return GLE_PAPER_LETTER;
	} else {
		return GLE_PAPER_UNKNOWN;
	}
}

/*
4A0	1682 � 2378	?	?	?	?
2A0	1189 � 1682	?	?	?	?
A0	841 � 1189	B0	1000 � 1414	C0	917 � 1297
A1	594 � 841 	B1	707 � 1000	C1	648 � 917
A2	420 � 594 	B2	500 � 707 	C2	458 � 648
A3	297 � 420 	B3	353 � 500 	C3	324 � 458
A4	210 � 297	B4	250 � 353	C4	229 � 324
A5	148 � 210 	B5	176 � 250 	C5	162 � 229
A6	105 � 148 	B6	125 � 176 	C6	114 � 162
A7	74 � 105 	B7	88 � 125 	C7	81 � 114
A8	52 � 74 	B8	62 � 88 	C8	57 � 81
A9	37 � 52 	B9	44 � 62 	C9	40 � 57
A10	26 � 37 	B10	31 � 44 	C10	28 � 40
*/

void g_set_pagesize(int type) {
	g.papersize = type;
	switch (type) {
		case GLE_PAPER_A0:
			g.pagewidth = 84.1;
			g.pageheight = 118.9; break;
		case GLE_PAPER_A1:
			g.pagewidth = 59.4;
			g.pageheight = 84.1; break;
		case GLE_PAPER_A2:
			g.pagewidth = 42.0;
			g.pageheight = 59.4; break;
		case GLE_PAPER_A3:
			g.pagewidth = 29.7;
			g.pageheight = 42.0; break;
		case GLE_PAPER_A4:
			g.pagewidth = 21.0;
			g.pageheight = 29.7; break;
		case GLE_PAPER_LETTER:
			g.pagewidth = 21.6;
			g.pageheight = 27.9; break;
	}
}

void g_set_pagesize(const string& papersize) throw (ParserError) {
	SpaceStringTokenizer tokens(papersize.c_str());
	const string& token = tokens.next_token();
	int type = g_papersize_type(token);
	if (type != GLE_PAPER_UNKNOWN) {
		g_set_pagesize(type);
	} else {
		tokens.pushback_token();
		g.pagewidth = tokens.next_double();
		g.pageheight = tokens.next_double();
		g.papersize = GLE_PAPER_UNKNOWN;
	}
}

void g_set_margins(const string& margins) throw (ParserError) {
	SpaceStringTokenizer tokens(margins.c_str());
	g.topmargin = tokens.next_double();
	g.bottommargin = tokens.next_double();
	g.leftmargin = tokens.next_double();
	g.rightmargin = tokens.next_double();
}

void g_set_rotate_fullpage(bool rotfp) {
	g.rotate = rotfp;
}

bool g_is_rotate_fullpage() {
	return g.rotate;
}

void g_set_fullpage(bool fullpage) {
	g.fullpage = fullpage;
}

bool g_is_fullpage() {
	return g.fullpage;
}

void g_set_landscape(bool landscape) {
	g.landscape = landscape;
}

bool g_is_landscape() {
	return g.landscape;
}

double g_draw_width() {
	if (g_is_landscape()) {
		return g.pageheight - g.leftmargin - g.rightmargin;
	} else {
		return g.pagewidth - g.leftmargin - g.rightmargin;
	}
}

double g_draw_height() {
	if (g_is_landscape()) {
		return g.pagewidth - g.topmargin - g.bottommargin;
	} else {
		return g.pageheight - g.topmargin - g.bottommargin;
	}
}

void g_open(GLEFileLocation* outputfile, const string& inputfile) {
	if (g.isopen) return;
	g.isopen = true;
	g_reset_message();
	bool centerfig = false;
	double scalefac = 1.0;
	if (g.userwidth < 0 || g.userheight < 0) {
		// No size command given: full page + all available space
		g.userwidth = g_draw_width();
		g.userheight = g_draw_height();
		g.fullpage = true;
	} else {
		// Center figure if full page mode
		centerfig = true;
		if (g.fullpage) {
			// how much area is outside the page in landscape / portrait mode?
			double area_horiz_norm = max(0.0, g.userwidth  - g.pagewidth)  * g.userheight;
			double area_vert_norm =  max(0.0, g.userheight - g.pageheight) * g.userwidth;
			double area_horiz_land = max(0.0, g.userheight - g.pagewidth)  * g.userwidth;
			double area_vert_land =  max(0.0, g.userwidth  - g.pageheight) * g.userheight;
			if (area_horiz_land + area_vert_land < area_horiz_norm + area_vert_norm) {
				g_set_landscape(true);
			}
			double userWidth = g.userwidth;
			double userHeight = g.userheight;
			if (g_is_landscape()) {
				userWidth = g.userheight;
				userHeight = g.userwidth;
			}
			if (userWidth > g.pagewidth) {
				scalefac = g.pagewidth / userWidth;
			}
			if (userHeight > g.pageheight) {
				scalefac = min(scalefac, g.pageheight / userHeight);
			}
			// cout << endl << "scale = " << scalefac << endl;
		}
	}
	if (g.fullpage) {
		if (g_get_compatibility() <= GLE_COMPAT_35) {
			if (!g_is_rotate_fullpage()) {
				// No real fullpage mode in 3.5
				g.dev->opendev(g.pagewidth, g.pageheight, outputfile, inputfile);
				g_on_open();
				// This is rather arbitrary, but this is how it was in 3.x
				g_translate(1.5, 1.01);
				if (g.userwidth > g.userheight) {
					// Rotate page in landscape mode (not just the figure)
					g_move(0.0,0.0);
					g_rotate(90.0);
					g_translate(0.0,-g.userheight);
					g_move(0.0,0.0);
				}
			} else {
				// Rotate page into landscape mode
				g.dev->opendev(g.pageheight, g.pagewidth, outputfile, inputfile);
				g_on_open();
				if (g.userwidth > g.userheight) {
					// Rotates cancel out
					g_translate(1.01, -1.5+g.pagewidth-g.userheight);
				} else {
					g_translate(0, g.pagewidth);
					g_rotate(-90.0);
					g_translate(1.5, 1.01);
				}
			}
		} else {
			if (!g_is_rotate_fullpage()) {
				g.dev->opendev(g.pagewidth, g.pageheight, outputfile, inputfile);
				g_on_open();
				if (g_is_landscape()) {
					// Note: landscape only rotates the figure, not the full page
					// -> this is how it was intended (don't change this)
					g_translate(g.pagewidth, 0);
					g_rotate(90.0);
				}
			} else {
				g.dev->opendev(g.pageheight, g.pagewidth, outputfile, inputfile);
				g_on_open();
				if (!g_is_landscape()) {
					// This rotation together with landscape cancels out
					g_translate(0, g.pagewidth);
					g_rotate(-90.0);
				}
			}
			if (centerfig) {
				double left = g.leftmargin + (g_draw_width() - g.userwidth)/2;
				double bottom = g.bottommargin + (g_draw_height() - g.userheight)/2;
				g_translate(left, bottom);
			} else {
				g_translate(g.leftmargin, g.bottommargin);
			}
		}
	} else {
		g.dev->opendev(g.userwidth, g.userheight, outputfile, inputfile);
		g_on_open();
	}
	g_UserBoxDev.setDimensions(0.0, 0.0, g.userwidth, g.userheight);
	g_dev(&g_UserBoxDev);
	if (g.hasbox) {
		g_box_stroke(0.0, 0.0, g.userwidth, g.userheight, false);
	}
}

void g_get_userbox_undev(GLERectangle* rect) {
	rect->copy(&g_UserBoxDev);
	g_undev(rect);
}

void g_close() {
	g.isopen = false;
	g_flush();
	g.dev->closedev();
}

void g_pscomment(char* ss) {
	if(g.isopen) {
		gprint("Can't call PSCOMMENT before SIZE command.  Ignoring\n");
	} else {
		g.dev->pscomment(ss);
	}
}

void g_psbbtweak(void) {
}

bool g_is_bbtweak() {
	// return g.bbtweak;
	return true;
}

void g_set_line_cap(int i) {
	/*  lcap, 0= butt, 1=round, 2=projecting square */
	if (i<0 || i>2) {
		gprint("Invalid line cap, {%d}, valid numbers are \n",i);
		gprint("	0= butt, 1=round, 2=projecting square \n");
	}
	g.dev->set_line_cap(i);
	g.lcap = i;
}

void g_set_line_join(int i) {
	if (i<0 || i>2) {
		gprint("Invalid line join, {%d}, valid numbers are \n",i);
		gprint("	0= mitre, 1=round, 2=bevel \n");
	}
	g.dev->set_line_join(i);
	g.ljoin = i;
}

void g_get_line_join(int *i) {
	*i = g.ljoin;
}

void g_get_line_cap(int *i) {
	*i = g.lcap;
}

void g_set_line_miterlimit(double d) {
	g.dev->set_line_miterlimit(d);
	g.miterlimit = d;
}

void g_get_line_width(double *w) {
	*w = g.lwidth;
}

void g_set_line_width(double w) {
	if (w < 0) return;
	g.dev->set_line_width(w);
	g.lwidth = w;
}

void g_set_font_width(double w) {
	font_lwidth = w;
}

void g_get_line_styled(double *w) {
	*w = g.lstyled;
}

void g_set_line_styled(double w) {
	if (w==0) return;
	g.dev->set_line_styled(w);
	g.lstyled = w;
}

void g_set_line_style(const char *s) {
	g.dev->set_line_style(s);
	strncpy(g.lstyle,s,8);
}

void g_get_line_style(char *s) {
	strncpy(s, (char *)&g.lstyle, 8);
}

double g_get_angle_deg() {
	if (fabs(g.image[0][0]) <= 1e-6) {
		return g.image[1][0] > 0.0 ? 90.0 : -90.0;
	} else {
		return myatan2(g.image[1][0], g.image[0][0])*180/GLE_PI;
	}
}

void g_get_scale(double* sx, double* sy) {
	double x0, y0, x1, y1;
	g_dev(0.0, 0.0, &x0, &y0);
	g_dev(1.0, 1.0, &x1, &y1);
	*sx = (x1-x0)/PS_POINTS_PER_INCH*CM_PER_INCH;
	*sy = (y1-y0)/PS_POINTS_PER_INCH*CM_PER_INCH;
}

double g_get_avg_scale() {
	double sx, sy;
	g_get_scale(&sx, &sy);
	return (sx + sy)/2.0;
}

void g_dev(double x, double y, double *xd, double *yd) {
	if (static_cast<bool>(gunit)==true) {
		*xd = x; *yd = y;
	} else {
		*xd = g.image[0][0]*x + g.image[0][1]*y + g.image[0][2];
		*yd = g.image[1][0]*x + g.image[1][1]*y + g.image[1][2];
	}
}

void g_dev(GLEPoint* pt) {
	g_dev(pt->m_X, pt->m_Y, &pt->m_X, &pt->m_Y);
}

void g_dev_rel(GLEPoint* pt) {
	pt->m_X = g.image[0][0]*pt->m_X + g.image[0][1]*pt->m_Y;
	pt->m_Y = g.image[1][0]*pt->m_X + g.image[1][1]*pt->m_Y;
}

void g_dev(GLERectangle* rect) {
	double x1, x2, y1, y2;
	g_dev(rect->getXMin(), rect->getYMin(), &x1, &y1);
	g_dev(rect->getXMax(), rect->getYMax(), &x2, &y2);
	rect->setDimensions(x1, y1, x2, y2);
	rect->normalize();
}

void g_undev(double ux, double uy, double *x, double *y, gmodel* model) {
	static double xx,yy,cdiv,xd,yd;
	if (static_cast<bool>(gunit) == true) {
		*x = ux; *y = uy;
	} else {
		cdiv = ( model->image[0][1]*model->image[1][0] - model->image[0][0]*model->image[1][1] );
		if (cdiv == 0) {
			gprint("Image matrix FLAT, a 1D world, giving up \n");
			return;
		}
		xd = ux - model->image[0][2];
		yd = uy - model->image[1][2];
		xx = -xd*model->image[1][1] + yd*model->image[0][1];
		*x = xx/cdiv;
		yy = xd*model->image[1][0]-yd*model->image[0][0];
		*y = yy/cdiv;
	}
}

void g_undev(double ux, double uy, double *x, double *y) {
	g_undev(ux, uy, x, y, &g);
}

void g_undev(GLERectangle* rect, gmodel* g) {
	double x1, x2, y1, y2;
	g_undev(rect->getXMin(), rect->getYMin(), &x1, &y1, g);
	g_undev(rect->getXMax(), rect->getYMax(), &x2, &y2, g);
	rect->setDimensions(x1, y1, x2, y2);
	rect->normalize();
}

void g_undev(GLERectangle* rect) {
	double x1, x2, y1, y2;
	g_undev(rect->getXMin(), rect->getYMin(), &x1, &y1);
	g_undev(rect->getXMax(), rect->getYMax(), &x2, &y2);
	rect->setDimensions(x1, y1, x2, y2);
	rect->normalize();
}

void g_rundev(double x, double y,double *xd,double *yd) {
	static double zx,zy;
	g_undev(0,0,&zx,&zy);
	g_undev(x,y,xd,yd);
	*xd -= zx;
	*yd -= zy;
}

void g_rdev(double x, double y,double *xd,double *yd) {
	static double zx,zy;
	g_dev(0,0,&zx,&zy);
	g_dev(x,y,xd,yd);
	*xd -= zx;
	*yd -= zy;
}

void g_fill() {
	g.dev->fill();
}

void g_fill_ary(int nwk,double *wkx,double *wky) {
	g.dev->fill_ary(nwk,wkx,wky);
}

void g_line_ary(int nwk,double *wkx,double *wky) {
	g.dev->line_ary(nwk,wkx,wky);
}

void g_stroke() {
	g.dev->stroke();
}

void g_clip() {
	/* Find the intersection of the current path with the clipping path */
	/* and thus define a new clipping path */
	g.dev->clip();
}

void g_set_matrix(double newmat[3][3], gmodel* old_g, gmodel* new_g) {
	double x1, y1, x2, y2, x3, y3, x4, y4;
	double a1, b1, a2, b2, a3, b3, a4, b4;
	bool modified = false;
	bool has_box = g_has_box(old_g);
	// Check if transformation matrix is different from old one
	if (memcmp(newmat, old_g->image, GLE_TRNS_MATRIX_SIZE) != 0) {
		if (has_box) {
			g_dev(old_g->xmin, old_g->ymin, &x1, &y1);
			g_dev(old_g->xmax, old_g->ymin, &x2, &y2);
			g_dev(old_g->xmax, old_g->ymax, &x3, &y3);
			g_dev(old_g->xmin, old_g->ymax, &x4, &y4);
			#ifdef CORE_MATRIX_DEBUG
			cout << "c1 : " << old_g->xmin << ", " << old_g->ymin << endl;
			cout << "c2 : " << old_g->xmax << ", " << old_g->ymin << endl;
			cout << "c3 : " << old_g->xmax << ", " << old_g->ymax << endl;
			cout << "c4 : " << old_g->xmin << ", " << old_g->ymax << endl;
			#endif
		}
		new_g->dev->set_matrix(newmat);
		if (newmat != new_g->image) memcpy(&new_g->image, newmat, GLE_TRNS_MATRIX_SIZE);
		modified = true;
	}
	#ifdef CORE_MATRIX_DEBUG
	cout << "modified: " << modified << " has box: " << has_box << " min " << old_g->xmin << " max " << old_g->xmax << endl;
	#endif
	if (has_box) {
		if (modified) {
			g_undev(x1, y1, &a1, &b1, new_g);
			g_undev(x2, y2, &a2, &b2, new_g);
			g_undev(x3, y3, &a3, &b3, new_g);
			g_undev(x4, y4, &a4, &b4, new_g);
			g_set_bounds(a1, b1, new_g);
			g_set_bounds(a2, b2, new_g);
			g_set_bounds(a3, b3, new_g);
			g_set_bounds(a4, b4, new_g);
			#ifdef CORE_MATRIX_DEBUG
			cout << "c1 : " << a1 << ", " << b1 << endl;
			cout << "c2 : " << a2 << ", " << b2 << endl;
			cout << "c3 : " << a3 << ", " << b3 << endl;
			cout << "c4 : " << a4 << ", " << b4 << endl;
			#endif
		} else {
			g_set_bounds(old_g->xmin, old_g->ymin, new_g);
			g_set_bounds(old_g->xmax, old_g->ymax, new_g);
			#ifdef CORE_MATRIX_DEBUG
			cout << "update : " << old_g->xmin << ", " << old_g->ymin << endl;
			cout << "update : " << old_g->xmax << ", " << old_g->ymax << endl;
			cout << "yields : " << new_g->xmin << ", " << new_g->ymin << endl;
			cout << "yields : " << new_g->xmax << ", " << new_g->ymax << endl;
			#endif
		}
	}
}

void g_set_matrix(double newmat[3][3]) {
	g_set_matrix(newmat, &g, &g);
}

void g_clear_matrix() {
	/* Clear transformation matrix */
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			g.image[i][j] = 0.0;
		}
		g.image[i][i] = 1.0;
	}
}

void mat_mult(double a[3][3],double b[3][3]) {
	static double c[3][3],tot;
	int y,xb,x;
	for (y=0;y<3;y++) {
	  for (xb=0;xb<3;xb++) {
		tot = 0;
		for (x=0;x<3;x++) tot += a[x][y] * b[xb][x];
		c[xb][y] = tot;
	  }
	}
	memcpy(a,&c,GLE_TRNS_MATRIX_SIZE);	/* maybe sizeof(*a) would be better? */
}

void g_rotate(double ar) {
	static double r[3][3],unx,uny,cx,cy;
	if (ar == 0.0) {
		/* Do not have to do anything if arrow is zero */
		return;
	}
	ar = GLE_PI*ar/180;
	r[0][0] = cos(ar);
	r[0][1] = -sin(ar);
	r[1][0] = sin(ar);
	r[1][1] = cos(ar);
	r[2][2] = 1;
	g_dev(g.curx,g.cury,&cx,&cy);
	g_rundev(-cx,-cy,&unx,&uny);
	g_translate(unx,uny);
	memcpy(tmpimg,g.image,GLE_TRNS_MATRIX_SIZE);
	mat_mult(tmpimg,r);
	g_set_matrix(tmpimg);
	g_rundev(cx,cy,&unx,&uny);
	g_translate(unx,uny);	/* not shore about this ORIGIN stuff */
	test_unit();
}

void gg_unrotate(void);
void gg_rerotate(void);

void g_scale(double sx,double sy) {
	/* The idea is to rotate or scale about the CURRENT POINT */
	static double r[3][3],unx,uny,cx,cy;
	r[0][0] = sx;
	r[1][1] = sy;
	r[2][2] = 1;

	gg_unrotate();
	g_dev(g.curx,g.cury,&cx,&cy);
	g_rundev(-cx,-cy,&unx,&uny);
	g_translate(unx,uny);
	memcpy(tmpimg,g.image,GLE_TRNS_MATRIX_SIZE);
	mat_mult(tmpimg,r);
	g_set_matrix(tmpimg);
	g_rundev(cx,cy,&unx,&uny);
	g_translate(unx,uny);	/* not shore about this ORIGIN stuff */
	gg_rerotate();
	test_unit();
}

void g_shear(double sx,double sy) {
	/* The idea is to rotate or scale about the CURRENT POINT */
	static double r[3][3],unx,uny,cx,cy;
	r[0][0] = 1;
	r[1][0] = sy;
	r[1][1] = 1;
	r[0][1] = sx;
	r[2][2] = 1;

	gg_unrotate();
	g_dev(g.curx,g.cury,&cx,&cy);
	g_rundev(-cx,-cy,&unx,&uny);
	g_translate(unx,uny);
	memcpy(tmpimg,g.image,GLE_TRNS_MATRIX_SIZE);
	mat_mult(tmpimg,r);
	g_set_matrix(tmpimg);
	g_rundev(cx,cy,&unx,&uny);
	g_translate(unx,uny);	/* not shore about this ORIGIN stuff */
	gg_rerotate();
	test_unit();
}

static double ggra;
void gg_rerotate() {
	g_rotate(ggra);
}

void gg_unrotate() {
	double ox,oy,x1,y0,dx,dy,tt;
	g_dev(0.0,0.0,&ox,&oy);
	g_dev(1.0,0.0,&x1,&y0);
	dx = x1-ox;
	dy = y0-oy;
	tt = myatan2(dy,dx);
	ggra = tt * 180.0 / GLE_PI;
	g_rotate(-ggra);
}

void dis_mat(char *s,double m[3][3]) {
	gprint("\n Matrix {%s} \n",s);
	for (int i = 0; i < 3; i++) {
		gprint("	%f %f %f \n",m[0][i],m[1][i],m[2][i]);
	}
}

void g_translate(double ztx,double zty) {
	static double tx,ty,r[3][3];
	g_rdev(ztx,zty,&tx,&ty);

	r[0][0] = 1;
	r[1][1] = 1;
	r[2][2] = 1;
	r[0][2] = tx;
	r[1][2] = ty;
	memcpy(tmpimg,g.image,GLE_TRNS_MATRIX_SIZE);
	mat_mult(tmpimg,r);
	g_set_matrix(tmpimg);
	test_unit();
}

void g_move(const GLEPoint& pt) {
	g_move(pt.getX(), pt.getY());
}

void g_move(double zx, double zy) {
	if (static_cast<bool>(g.xinline)==true) g_flush();
	g.dev->move(zx,zy);
	g.curx = zx;
	g.cury = zy;
	g.closex = zx;
	g.closey = zy;
}

void g_set_pos(double zx,double zy) {
	g.curx = zx;
	g.cury = zy;
	g_update_bounds(zx,zy);
}

void g_set_pos(const GLEPoint& p) {
	g.curx = p.getX();
	g.cury = p.getY();
	g_update_bounds(p.getX(), p.getY());
}

void g_rset_pos(double x, double y) {
	g.curx += x;
	g.cury += y;
	g_update_bounds(g.curx,g.cury);
}

void g_rmove(double zx,double zy) {
	g_move(g.curx+zx,g.cury+zy);
}

void g_rline(double zx,double zy) {
	g_line(g.curx+zx,g.cury+zy);
}

void g_reverse() {
	g.dev->reverse();
}

void g_closepath() {
	if (g.inpath) g.dev->closepath();
	else g_line(g.closex,g.closey);
	g.curx = g.closex;
	g.cury = g.closey;
	if (!g.inpath) g_flush();
}

void g_line(const GLEPoint& p) {
	g_line(p.getX(), p.getY());
}

void g_line(double zx, double zy) {
	GLEPoint origin;
	g_get_xy(&origin);
	g.dev->line(zx,zy);
	if (g.xinline == false) {
		g.xinline = true;
		g_update_bounds(g.curx, g.cury);
	}
	g.curx = zx;
	g.cury = zy;
	g_update_bounds(zx, zy);
	GLECore* core = g_get_core();
	if (core->isComputingLength()) {
		GLEPoint endPoint(zx, zy);
		core->addToLength(origin.distance(endPoint));
	}
}

void g_update_bounds(double x, double y) {
	if (x < g.xmin) g.xmin = x;
	if (x > g.xmax) g.xmax = x;
	if (y < g.ymin) g.ymin = y;
	if (y > g.ymax) g.ymax = y;
	//g_check_bounds("after g_set_bounds");
}

void g_set_bounds(double x, double y, gmodel* model) {
	if (x < model->xmin) model->xmin = x;
	if (x > model->xmax) model->xmax = x;
	if (y < model->ymin) model->ymin = y;
	if (y > model->ymax) model->ymax = y;
}

bool g_has_box(gmodel* model) {
	return model->xmin <= model->xmax && model->ymin <= model->ymax;
}

void g_update_bounds_box(GLERectangle* box) {
	// cout << "update " << g.xmin << " " << g.xmax << " " << g.ymin << " " << g.ymax << endl;
	if (g_has_box(&g)) {
		if (g.xmin < box->getXMin()) box->setXMin(g.xmin);
		if (g.xmax > box->getXMax()) box->setXMax(g.xmax);
		if (g.ymin < box->getYMin()) box->setYMin(g.ymin);
		if (g.ymax > box->getYMax()) box->setYMax(g.ymax);
	}
}

void g_restore_bounds(double xmin, double xmax, double ymin, double ymax) {
	g.xmin = xmin;
	g.xmax = xmax;
	g.ymin = ymin;
	g.ymax = ymax;
}

void g_get_bounds(GLERectangle* rect) {
	rect->setXMin(g.xmin);
	rect->setYMin(g.ymin);
	rect->setXMax(g.xmax);
	rect->setYMax(g.ymax);
}

void g_set_bounds(GLERectangle* rect) {
	g.xmin = rect->getXMin();
	g.ymin = rect->getYMin();
	g.xmax = rect->getXMax();
	g.ymax = rect->getYMax();
}

void g_update_bounds(GLERectangle* rect) {
	g_update_bounds(rect->getXMin(), rect->getYMin());
	g_update_bounds(rect->getXMax(), rect->getYMax());
}

void g_debug_bounds(const char* out) {
	cout << out << ": bounds: (" << g.xmin << ", " << g.ymin << ") - (" << g.xmax << ", " << g.ymax << ")" << endl;
}

void g_check_bounds(const char* after) {
	if (g.xmin == -1e30 || g.xmax == 1e30 || g.ymin == -1e30 || g.ymax == 1e30) {
		ostringstream ostr;
		ostr << "bounds error: " << after << endl;
		ostr << "yields : " << g.xmin << ", " << g.ymin << endl;
		ostr << "yields : " << g.xmax << ", " << g.ymax;
		g_throw_parser_error(ostr.str().c_str());
	}
}

void test_unit() {
	int i,j;
	gunit = true;
	for (i=0;i<3;i++) for (j=0;j<3;j++) if (i!=j) if(g.image[i][j]!=0.0) gunit = false;
	for (i=0;i<3;i++) if (g.image[i][i]!=1.0) gunit = false;
}

void tex_clear(void);

void g_clear() {
	/* Reset all params */
	// cout << "calling g_clear()" << endl;
	ngsave = 0;
	g.fontn = 0.0;
	g.fontsz = 0.0;		   /* up to here for font caching */
	g.color = color_or_fill_from_int(GLE_COLOR_BLACK);
	g.fill = g_get_fill_clear();
	g.lwidth = 0.0;
	g.lstyled = 0.0;
	g.curx = 0.0;
	g.cury = 0.0;
	g.endx = 0.0;
	g.endy = 0.0;
	g.miterlimit = 0.0;
	g.lcap = 0;
	g.ljoin = 0;
	g.just = 0;
	g.xinline = 0;
	g.npath = 0;		     /* up to here for STATE */
	g.inpath = 0;
	g.xmin = 0.0;
	g.xmax = 0.0;
	g.ymin = 0.0;
	g.ymax = 0.0;		    /* bounds in USER coordinates */
	g.startx = 0.0;
	g.starty = 0.0;
	g.closex = 0.0;
	g.closey = 0.0;			/* for closepath */
	g.arrowsize = 0.0;
	g.arrowangle = 0.0;
	g.arrowstyle = GLE_ARRSTY_FILLED;
	g.arrowtip = GLE_ARRTIP_SHARP;
	g.pdfimgformat = PDF_IMG_COMPR_ZIP;
	g.userheight = -1.0;
	g.userwidth = -1.0;
	g.pagewidth = -1.0;
	g.pageheight = -1.0;
	g.papersize = GLE_PAPER_UNKNOWN;
	g.topmargin = 0.0; g.bottommargin = 0.0;
	g.leftmargin = 0.0; g.rightmargin = 0.0;
	g.fullpage = false;
	g.landscape = false;
	g.rotate = false;
	g.hasbox = false;
	g.isopen = false;
	g.texlabels = false;
	/* Reset line style */
	for (unsigned int i = 0; i < sizeof(g.lstyle); i++) {
		g.lstyle[i] = 0;
	}
	g.lstyle[0] = '1';
	/* Clear transformation matrix */
	g_clear_matrix();
	/* set other constants */
	g_set_fconst(GLEC_ATITLEDIST, 0.5);
	g_set_fconst(GLEC_ALABELDIST, 0.5);
	g_compatibility_settings();
	g_get_core()->reset();
}

void g_compatibility_settings() {
	if (g_get_compatibility() <= GLE_COMPAT_35) {
		g_set_fconst(GLEC_TITLESCALE, 1.5);
		g_set_fconst(GLEC_ATITLESCALE, 1.3);
		g_set_fconst(GLEC_ALABELSCALE, 1.0);
		g_set_fconst(GLEC_TICKSSCALE, 0.2);
		g.arrowstyle = GLE_ARRSTY_OLD35;
	} else {
		// Make sure GLEC_ATITLESCALE = 1
		g_set_fconst(GLEC_TITLESCALE, 1.16);
		g_set_fconst(GLEC_ATITLESCALE, 1.0);
		g_set_fconst(GLEC_ALABELSCALE, 0.8);
		g_set_fconst(GLEC_TICKSSCALE, 0.3);
	}
}

void g_on_open() {
	/* Clear device dependent stuff */
	g.dev->clear();
	g_resetfont();
	tex_clear();
	g_set_just(JUST_LEFT);
	g_set_line_styled(0.04);
	g_set_line_style("1");
	g_set_line_width(0.02);
	g_set_color(GLE_COLOR_BLACK);
	g_set_fill(GLE_FILL_CLEAR);
	g_set_font(1);           /* Load and set default font */
	g_set_font_width(-1);    /* Load and set default font */
	if (g_get_compatibility() <= GLE_COMPAT_35) {
		g_set_hei(1);
	} else {
		// Same default height as TeX fonts, don't change this.
		g_set_hei(0.3633);
	}
	g_move(0.0,0.0);
	test_unit();
}

void g_restore_defaults() {
	/* FIXME - improve code sharing with above methods */
	/*         after 4.0.12 is out */
	g.curx = 0.0;
	g.cury = 0.0;
	g.endx = 0.0;
	g.endy = 0.0;
	g.arrowsize = 0.0;
	g.arrowangle = 0.0;
	g.arrowstyle = GLE_ARRSTY_FILLED;
	g.arrowtip = GLE_ARRTIP_SHARP;
	if (g_get_compatibility() <= GLE_COMPAT_35) {
		g_set_fconst(GLEC_TITLESCALE, 1.5);
		g_set_fconst(GLEC_ATITLESCALE, 1.3);
		g_set_fconst(GLEC_ALABELSCALE, 1.0);
		g_set_fconst(GLEC_TICKSSCALE, 0.2);
		g.arrowstyle = GLE_ARRSTY_OLD35;
	} else {
		g_set_fconst(GLEC_TITLESCALE, 1.16);
		g_set_fconst(GLEC_ATITLESCALE, 1.0);
		g_set_fconst(GLEC_ALABELSCALE, 0.8);
		g_set_fconst(GLEC_TICKSSCALE, 0.3);
	}
	g_set_fconst(GLEC_ATITLEDIST, 0.5);
	g_set_fconst(GLEC_ALABELDIST, 0.5);
	g_set_just(JUST_LEFT);
	g_set_line_styled(0.04);
	g_set_line_style("1");
	g_set_line_width(0.02);
	g_set_color(GLE_COLOR_BLACK);
	g_set_fill(GLE_FILL_CLEAR);
	g_set_font(1);           /* Load and set default font */
	g_set_font_width(-1);    /* Load and set default font */
	if (g_get_compatibility() <= GLE_COMPAT_35) {
		g_set_hei(1);
	} else {
		// Same default height as TeX fonts, don't change this.
		g_set_hei(0.3633);
	}
	g_move(0.0,0.0);
	test_unit();
}

void g_get_xy(double *x,double *y) {
	*x = g.curx;
	*y = g.cury;
}

GLEPoint g_get_xy() {
	GLEPoint result;
	g_get_xy(&result);
	return result;
}

void g_get_xy(GLEPoint* pt) {
	pt->setX(g.curx);
	pt->setY(g.cury);
}

void g_set_xy(double x, double y) {
	g_move(x,y);
}

void g_flush() {
	if (g.inpath) return;
	g.dev->flush();
	g.xinline = false;
}

void g_arcto(dbl x1,dbl y1,dbl x2,dbl y2,dbl rrr) {
	g.dev->arcto(x1,y1,x2,y2,rrr);
	g.curx = x2;
	g.cury = y2;
	g.xinline = true;
	/* should do MASSES of calc to find bounds of arc  */
	g_update_bounds(x1,y1);
	g_update_bounds(x2,y2);
}

void g_dojust(dbl *x1, dbl *y1, dbl *x2, dbl *y2, int jj) {
	double d;
	int jx,jy;
	jx = (jj & 0xf0) / 16;
	jy = jj & 0x0f;
	d = jx * (*x2-*x1)/2;
	*x1 -= d;
	*x2 -= d;
	d = jy * (*y2-*y1)/2;
	*y1 -= d;
	*y2 -= d;
}

void g_dotjust(dbl *x1, dbl *y1, dbl l, dbl r, dbl u, dbl d, int jj) {
	static int jx,jy,t;
	static double ddd;
	jx = (jj & 0xf0) / 16;
	jy = jj & 0x0f;
	t = (jj & 0xf00) / 256 ;
	ddd = jx * (-l+r)/2;
	*x1 = *x1 + -l - ddd;
	ddd = jy * (u-d)/2;
	if (t==0) *y1 = *y1 - d - ddd;
}

void g_box_stroke(GLERectangle* rect, bool reverse) {
	g_box_stroke(rect->getXMin(), rect->getYMin(), rect->getXMax(), rect->getYMax(), reverse);
}

void g_box_stroke(dbl x1, dbl y1, dbl x2, dbl y2, bool reverse) {
	double x,y;
	g_get_xy(&x,&y);
	g.dev->box_stroke(x1,y1,x2,y2,reverse);
	g_update_bounds(x1,y1);
	g_update_bounds(x2,y2);
	g_move(x,y);
}

void g_box_fill(GLERectangle* rect) {
	g_box_fill(rect->getXMin(), rect->getYMin(), rect->getXMax(), rect->getYMax());
}

void g_box_fill(dbl x1, dbl y1, dbl x2, dbl y2) {
	double x,y;
	g_get_xy(&x,&y);
	g.dev->box_fill(x1,y1,x2,y2);
	g_update_bounds(x1,y1);
	g_update_bounds(x2,y2);
	g_move(x,y);
}

enum arrow_direction{CW,CCW};

void g_circle_stroke(dbl zr) {
	GLEPoint orig(g_get_xy());
	g.dev->circle_stroke(zr);
	g_update_bounds(g.curx-zr,g.cury-zr);
	g_update_bounds(g.curx+zr,g.cury+zr);
	GLECore* core = g_get_core();
	if (core->isComputingLength()) {
		GLECircleArc curve(orig, zr, 0.0, 2.0 * GLE_PI);
		core->addToLength(curve.getDist(curve.getT0(), curve.getT1()));
	}
}

void g_circle_fill(dbl zr) {
	g.dev->circle_fill(zr);
	g_update_bounds(g.curx-zr,g.cury-zr);
	g_update_bounds(g.curx+zr,g.cury+zr);
}

void g_update_arc_bounds_for_arrow_heads(GLECurvedArrowHead* head_start,
		                                 GLECurvedArrowHead* head_end,
		                                 double* t1,
		                                 double* t2)
{
	if (head_start->getStyle() != GLE_ARRSTY_SIMPLE) {
		if (head_start->isEnabled()) *t1 = head_start->getParamValueEnd()*180/GLE_PI;
		if (head_end->isEnabled()) *t2 = head_end->getParamValueEnd()*180/GLE_PI;
	}
}

double g_arc_normalized_angle2(double a1, double a2) {
	if (a2 < a1) {
		// Correctly normalize a2
		return a2 + 360 * ceil((a1 - a2)/360.0);
	} else {
		return a2;
	}
}

void g_arc(double r, double t1, double t2, double cx, double cy, int arrow) {
	g_flush();
	GLEPoint orig(cx, cy);
	GLECore* core = g_get_core();
	if (core->isComputingLength()) {
		GLECircleArc circle(orig, r, t1*GLE_PI/180, t2*GLE_PI/180);
		core->addToLength(fabs(circle.getDist(circle.getT0(), circle.getT1())));
	}
	GLEWithoutUpdates noUpdates;
	if (arrow != 0) {
		GLECircleArc circle(orig, r, t1*GLE_PI/180, t2*GLE_PI/180);
		GLECurvedArrowHead head_start(&circle);
		GLECurvedArrowHead head_end(&circle);
		if (arrow == 1 || arrow == 3) g_init_arrow_head(&head_start, true);
		if (arrow == 2 || arrow == 3) g_init_arrow_head(&head_end, false);
		g_update_arc_bounds_for_arrow_heads(&head_start, &head_end, &t1, &t2);
		g.dev->arc(r,t1,t2,cx,cy);
		head_start.computeAndDraw();
		head_end.computeAndDraw();
	} else {
		g.dev->arc(r,t1,t2,cx,cy);
	}
	g.curx = cx; g.cury = cy;
}

void g_narc(double r, double t1, double t2, double cx, double cy, int arrow) {
	g_flush();
	GLEPoint orig(cx, cy);
	GLECore* core = g_get_core();
	if (core->isComputingLength()) {
		GLECircleArc circle(orig, r, t1*GLE_PI/180, t2*GLE_PI/180);
		core->addToLength(fabs(circle.getDist(circle.getT0(), circle.getT1())));
	}
	GLEWithoutUpdates noUpdates;
	if (arrow != 0) {
		GLECircleArc circle(orig, r, t2*GLE_PI/180, t1*GLE_PI/180);
		GLECurvedArrowHead head_start(&circle);
		GLECurvedArrowHead head_end(&circle);
		if (arrow == 1 || arrow == 3) g_init_arrow_head(&head_start, false);
		if (arrow == 2 || arrow == 3) g_init_arrow_head(&head_end, true);
		g_update_arc_bounds_for_arrow_heads(&head_start, &head_end, &t1, &t2);
		g.dev->narc(r,t1,t2,cx,cy);
		head_start.computeAndDraw();
		head_end.computeAndDraw();
	} else {
		g.dev->narc(r,t1,t2,cx,cy);
	}
	g.curx = cx; g.cury = cy;
}

void g_ellipse_stroke(dbl rx, dbl ry) {
	GLEPoint orig(g_get_xy());
	g.dev->ellipse_stroke(rx,ry);
	g_update_bounds(g.curx-rx,g.cury-ry);
	g_update_bounds(g.curx+rx,g.cury+ry);
	GLECore* core = g_get_core();
	if (core->isComputingLength()) {
		GLEEllipseArc curve(orig, rx, ry, 0.0, 2.0 * GLE_PI);
		core->addToLength(curve.getDist(curve.getT0(), curve.getT1()));
	}
}

void g_ellipse_fill(dbl rx, dbl ry) {
	g.dev->ellipse_fill(rx,ry);
	g_update_bounds(g.curx-rx,g.cury-ry);
	g_update_bounds(g.curx+rx,g.cury+ry);
}

void g_elliptical_arc(double rx, double ry, double t1, double t2, double cx, double cy, int arrow) {
	g_flush();
	GLEPoint orig(cx, cy);
	GLECore* core = g_get_core();
	if (core->isComputingLength()) {
		GLEEllipseArc arc(orig, rx, ry, t1*GLE_PI/180, t2*GLE_PI/180);
		core->addToLength(fabs(arc.getDist(arc.getT0(), arc.getT1())));
	}
	GLEWithoutUpdates noUpdates;
	if (arrow != 0) {
		GLEEllipseArc ellipse(orig, rx, ry, t1*GLE_PI/180, t2*GLE_PI/180);
		GLECurvedArrowHead head_start(&ellipse);
		GLECurvedArrowHead head_end(&ellipse);
		if (arrow == 1 || arrow == 3) g_init_arrow_head(&head_start, true);
		if (arrow == 2 || arrow == 3) g_init_arrow_head(&head_end, false);
		g_update_arc_bounds_for_arrow_heads(&head_start, &head_end, &t1, &t2);
		g.dev->elliptical_arc(rx, ry, t1, t2, cx, cy);
		head_start.computeAndDraw();
		head_end.computeAndDraw();
	} else {
		g.dev->elliptical_arc(rx, ry, t1, t2, cx, cy);
	}
	g.curx = cx; g.cury = cy;
}

void g_elliptical_narc(double rx, double ry, double t1, double t2, double cx, double cy, int arrow) {
	g_flush();
	GLEPoint orig(cx, cy);
	GLECore* core = g_get_core();
	if (core->isComputingLength()) {
		GLEEllipseArc arc(orig, rx, ry, t1*GLE_PI/180, t2*GLE_PI/180);
		core->addToLength(fabs(arc.getDist(arc.getT0(), arc.getT1())));
	}
	GLEWithoutUpdates noUpdates;
	if (arrow != 0) {
		GLEEllipseArc ellipse(orig, rx, ry, t2*GLE_PI/180, t1*GLE_PI/180);
		GLECurvedArrowHead head_start(&ellipse);
		GLECurvedArrowHead head_end(&ellipse);
		if (arrow == 1 || arrow == 3) g_init_arrow_head(&head_start, false);
		if (arrow == 2 || arrow == 3) g_init_arrow_head(&head_end, true);
		g_update_arc_bounds_for_arrow_heads(&head_start, &head_end, &t1, &t2);
		g.dev->elliptical_narc(rx, ry, t1, t2, cx, cy);
		head_start.computeAndDraw();
		head_end.computeAndDraw();
	} else {
		g.dev->elliptical_narc(rx, ry, t1, t2, cx, cy);
	}
	g.curx = cx; g.cury = cy;
}

void g_arrowcurve(double x, double y, int arrow, double a1, double a2, double d1, double d2) {
	if (d1 == 0.0 && d2 == 0.0) {
		g_arrowline(x, y, arrow, true);
		return;
	}
	double cx, cy, x1, y1, x2, y2;
	polar_xy(d1, a1, &x1, &y1);
	polar_xy(d2, a2, &x2, &y2);
	g_get_xy(&cx, &cy);
	GLEBezier bezier(cx, cy, cx+x1, cy+y1, x+x2, y+y2, x, y);
	GLECore* core = g_get_core();
	if (core->isComputingLength()) {
		core->addToLength(bezier.getDist(0.0, 1.0));
	}
	GLEWithoutUpdates noUpdates;
	if (arrow != 0) {
		GLECurvedArrowHead head_start(&bezier);
		GLECurvedArrowHead head_end(&bezier);
		if (arrow == 1 || arrow == 3) g_init_arrow_head(&head_start, true);
		if (arrow == 2 || arrow == 3) g_init_arrow_head(&head_end, false);
		GLEBezier todraw(bezier);
		if (head_start.getStyle() != GLE_ARRSTY_SIMPLE) {
			if (arrow == 1) todraw.cutFromParamValue(head_start.getParamValueEnd());
			if (arrow == 2) todraw.cutAtParamValue(head_end.getParamValueEnd());
			if (arrow == 3) {
				todraw.cutAtParamValue(head_end.getParamValueEnd());
				double t0 = todraw.distToParamValue(0.0, head_start.getArrowCurveDist()*0.75);
				todraw.cutFromParamValue(t0);
			}

		}
		todraw.draw();
		head_start.computeAndDraw();
		head_end.computeAndDraw();
	} else {
		bezier.draw();
	}
}

void g_bezier(dbl x1, dbl y1, dbl x2, dbl y2, dbl x3, dbl y3) {
	GLEPoint origin;
	g_get_xy(&origin);
	g.dev->bezier(x1, y1, x2, y2, x3, y3);
	if (g.xinline == false) {
		g.xinline = true;
		g_update_bounds(g.curx, g.cury);
	}
	g.curx = x3;
	g.cury = y3;
	g_update_bounds(x3,y3);
	GLECore* core = g_get_core();
	if (core->isComputingLength()) {
		GLEBezier bezierCurve(origin.getX(), origin.getY(), x1, y1, x2, y2, x3, y3);
		core->addToLength(bezierCurve.getDist(0.0, 1.0));
	}
}

void g_bezier(const GLEPoint& p1, const GLEPoint& p2, const GLEPoint& p3) {
	g_bezier(p1.getX(), p1.getY(), p2.getX(), p2.getY(), p3.getX(), p3.getY());
}

void g_dmove(double x, double y) {
	double ux,uy;
	g_undev(x,y,&ux,&uy);
	g.dev->move(ux,uy);
	g.curx = ux;
	g.cury = uy;
}

void g_dline(double x, double y) {
	double ux,uy;
	g_undev(x,y,&ux,&uy);
	g.dev->line(ux,uy);
	g.curx = ux;
	g.cury = uy;
}

void g_dbezier(dbl x1,dbl y1,dbl x2,dbl y2,dbl x3,dbl y3) {
	double a1,b1,a2,b2,a3,b3;
	g_undev(x1,y1,&a1,&b1);
	g_undev(x2,y2,&a2,&b2);
	g_undev(x3,y3,&a3,&b3);
	g_bezier(a1,b1,a2,b2,a3,b3);
	g.curx = a3;
	g.cury = b3;
}

int g_hash_string_to_color(const string& str, colortyp* c) {
	int err = 0;
	const char* color_str = str.c_str();
	c->b[B_F] = (unsigned char)1;
	c->b[B_R] = (unsigned char)gle_pass_hex(color_str, 1, 2, &err);
	c->b[B_G] = (unsigned char)gle_pass_hex(color_str, 3, 2, &err);
	c->b[B_B] = (unsigned char)gle_pass_hex(color_str, 5, 2, &err);
	return err;
}

void g_set_fill(int fill) {
	g_set_fill(color_or_fill_from_int(fill));
}

void g_set_fill(GLEColor* fill) {
	// Note: g_set_fill resets "pattern", so "set pattern" should come after "set fill"
	if (fill == 0) {
		g.fill = g_get_fill_clear();
	} else {
		g.fill = fill->clone();
	}
	g_set_fill_to_device();
}

void g_set_fill(const GLERC<GLEColor>& fill) {
	g_set_fill(fill.get());
}

void g_set_background(const GLERC<GLEColor>& color) {
	update_color_fill_background(g.fill.get(), color.get());
	g_set_fill_to_device();
}

void g_set_fill_pattern(const GLERC<GLEColor>& pattern) {
	if (pattern->isFill() && pattern->getFill()->getFillType() == GLE_FILL_TYPE_PATTERN) {
		update_color_fill_pattern(g.fill.get(), static_cast<GLEPatternFill*>(pattern->getFill()));
		g_set_fill_to_device();
	} else {
		g_throw_parser_error("expected fill pattern");
	}
}

GLERC<GLEColor> g_modify_color_for_device(const GLERC<GLEColor>& color) {
	GLERC<GLEColor> result(color);
    CmdLineObj* cmdLine = GLEGetInterfacePointer()->getCmdLine();
    if (cmdLine->hasOption(GLE_OPT_INVERSE)) {
    	unsigned int hexValue = color->getHexValueGLE();
    	if (hexValue == (unsigned int)GLE_COLOR_WHITE && color->getAlpha() == 1.0) {
    		result = new GLEColor(0.0, 0.0, 0.0);
    	}
    	if (hexValue == (unsigned int)GLE_COLOR_BLACK && color->getAlpha() == 1.0) {
    		result = new GLEColor(1.0, 1.0, 1.0);
    	}
    }
    if (cmdLine->hasOption(GLE_OPT_NO_COLOR)) {
    	double gray = color->getGray();
    	result = new GLEColor(gray, gray, gray);
    }
	return result;
}

void g_set_color_to_device() {
	g.dev->set_color(g_modify_color_for_device(g.color));
}

void g_set_fill_to_device() {
	g.dev->set_fill(g_modify_color_for_device(g.fill));
}

void g_set_color(GLEColor* color) {
	g.color = color->clone();
	g_set_color_to_device();
}

void g_set_color(const GLERC<GLEColor>& color) {
	if (!color.isNull()) {
		g_set_color(color.get());
	}
}

void g_set_color(int l) {
	if (l==0) return;
	g.color->setHexValueGLE(l);
	g_set_color_to_device();
}

GLERC<GLEColor> g_get_color() {
	return g.color->clone();
}

void g_set_fill_method(const char* meth) {
	if (str_i_equals(meth, "DEFAULT")) g.dev->set_fill_method(GLE_FILL_METHOD_DEFAULT);
	else if (str_i_equals(meth, "GLE")) g.dev->set_fill_method(GLE_FILL_METHOD_GLE);
	else g.dev->set_fill_method(GLE_FILL_METHOD_POSTSCRIPT);
}

bool g_is_filled(void) {
	return !g.fill->isTransparent();
}

GLERC<GLEColor> g_get_fill() {
	return g.fill->clone();
}

void g_beginclip() {
	g.dev->beginclip();
}

void g_endclip() {
	g.dev->endclip();
}

void g_gsave() {
	ngsave++;
	if (ngsave>=99) {
		gprint("Over 99 GSAVE's, probably a loop in your code\n");
		return;
	}
	gsave[ngsave] = new gmodel();
	g_get_state(gsave[ngsave]);
	g_init_bounds();
}

void g_grestore() {
	static double a,b;
	g_flush();
	if (ngsave==0) {
		gprint("Attempt to GRESTORE at top of stack\n");
		if (gle_debug>0) a = a/b;
		return;
	}
	g_set_state(gsave[ngsave]);
	delete gsave[ngsave];
	ngsave--;
}

void g_get_state(gmodel *s) {
	*s = g;
	s->fill = g.fill->clone();
	s->color = g.color->clone();
}

void g_set_state(gmodel* s) {
	g_set_matrix(s->image, &g, s);
	g = *s;
	g_set_color_to_device();
	g_set_fill_to_device();
	g.dev->set_line_width(g.lwidth);
	g.dev->set_line_style(g.lstyle);
	g.dev->set_line_styled(g.lstyled);
	test_unit();
}

void g_set_partial_state(gmodel* s) {
	g_set_color(s->color);
	g_set_fill(s->fill);
	g_set_line_width(s->lwidth);
	g_set_line_style(s->lstyle);
	g_set_line_styled(s->lstyled);
}

	/*		 12,4,-.5,-.5,0.35,	original dot */
struct mark_struct { int ff; int cc; double rx; double ry; double scl; double x1; double x2; double y1; double y2; };
struct mark_struct minf[61];
extern char *mrk_name[];
extern char *mrk_fname[];

/* struct mark_struct { int ff, int cc, double rx, double ry, double scl;}; */
extern char *mark_name[];
extern char *mark_sub[];
extern int mark_subp[];
extern int nmark;
extern int nmrk;

void g_marker(int i, double sz) {
	g_marker2(i,sz,1.0);
}

void g_marker2(int i, double sz, double dval) throw(ParserError) {
	static double cx,cy,h,scale;
	static double x1,y1,x2,y2;
	if (i<0) {
		GLERC<GLEArrayImpl> stk(new GLEArrayImpl());
		++i;
		i = -i;
		if (mark_subp[i] == -1) {
			GLESub* sub = sub_find(mark_sub[i]);
			mark_subp[i] = sub != NULL ? sub->getIndex() : -1;
			if (mark_subp[i] == -1)  {
				stringstream err;
				err << "subroutine '" << mark_sub[i] << "', which defines marker '" << mark_name[i] << "' not found";
				g_throw_parser_error(err.str());
			} else if (sub->getNbParam() != 2) {
				stringstream err;
				err << "subroutine '" << mark_sub[i] << "', which defines marker '" << mark_name[i] << "' should take two parameters (size and data), not " << sub->getNbParam();
				g_throw_parser_error(err.str());
			}
		}
		setEvalStack(stk.get(), 0, sz);
		setEvalStack(stk.get(), 1, dval);
		g_get_xy(&cx,&cy);
		getGLERunInstance()->sub_call(sub_get(mark_subp[i]), stk.get());
		g_move(cx,cy);
		return;
	}
	if (i<1 || i>nmrk) {gprint("Invalid marker number %d \n",i); return;}
	g_get_xy(&cx,&cy);
	g_get_hei(&h);
	i--;
	scale = minf[i].scl*sz;
	g_set_hei(scale);
	if (minf[i].ff == 0) {
		minf[i].ff = g_font_fallback(pass_font(mrk_fname[i]));
		char_bbox(minf[i].ff,minf[i].cc,&x1,&y1,&x2,&y2);
		minf[i].x1 = x1; minf[i].x2 = x2;
		minf[i].y1 = y1; minf[i].y2 = y2;
	} else if (minf[i].ff == -1) {
		minf[i].ff = g_font_fallback(pass_font(mrk_fname[i]));
		char_bbox(minf[i].ff,minf[i].cc,&x1,&y1,&x2,&y2);
		minf[i].ry = (minf[i].ry + -y1-(y2-y1)/2.0);
		minf[i].rx = (minf[i].rx + -x1-(x2-x1)/2.0);
		minf[i].x1 = x1; minf[i].x2 = x2;
		minf[i].y1 = y1; minf[i].y2 = y2;
	}
	double xpos = cx+minf[i].rx*scale;
	double ypos = cy+minf[i].ry*scale;
	g_move(xpos, ypos);
	g_char(minf[i].ff,minf[i].cc);
	g_update_bounds(xpos+minf[i].x1*scale, ypos+minf[i].y1*scale);
	g_update_bounds(xpos+minf[i].x2*scale, ypos+minf[i].y2*scale);
	g_move(cx,cy);
	g_set_hei(h);
}

void g_defmarker(const char *mname, const char *font, int ccc, double dx, double dy, double sz, int autodx) {
	int i;
	for (i=0; i<nmrk; i++) {
		if (str_i_equals(mname,mrk_name[i])) {
			myfree(mrk_name[i]);
			myfree(mrk_fname[i]);
			nmrk--;
			break;
		}
	}
	nmrk++;
	if (nmrk>61-1) {
		gprint("Too many markers defined \n");
		return;
	}
	mrk_name[i] = sdup(mname);
	mrk_fname[i] = sdup(font);
	minf[i].ff = 0;
	if (autodx) minf[i].ff = -1;
	minf[i].cc = ccc;
	minf[i].rx = dx;
	minf[i].ry = dy;
	minf[i].scl = sz;
	minf[i].x1 = 0.0;
	minf[i].x2 = 0.0;
	minf[i].y1 = 0.0;
	minf[i].y2 = 0.0;
}

void g_marker_def(char *mname,char *subname) {
	int i;
	for (i=0; i<nmark; i++) {
		if (str_i_equals(mname,mark_name[i])) {
			myfree(mark_name[i]);
			myfree(mark_sub[i]);
			nmark--;
			break;
		}
	}
	nmark++;
	mark_name[i] = sdup(mname);
	mark_sub[i] = sdup(subname);
	mark_subp[i] = -1;
}

void g_char(int font, int cc) {
	g.dev->dochar(font,cc);
}

void g_text(const string& s) {
	string str(s);
	text_block(str, 0.0, g.just);
}

void g_set_just(int jj) {
	g.just = jj;
}

void g_set_font(int jj) {
	if (jj==0) return;
	font_load_metric(jj);
	g.fontn = jj;
}

void g_set_hei(double h) {
	if (h <= 0.0) {
		cerr << "font size zero or negative: " << h << endl;
		return;
	}
	g.fontsz = h;
}

void g_get_just(int *jj) {
	*jj = g.just;
}

void g_get_font(int *jj) {
	*jj = (int) g.fontn;
}

void g_get_hei(double *h) {
	*h = g.fontsz;
}

bool g_parse_ps_boundingbox(const string& line, int* bx1, int* by1, int* bx2, int* by2)
{
	if (str_ni_equals(line.c_str(), "%%BoundingBox:", 14) && str_i_str(line, "(atend)") == -1) {
		char_separator sep(" :\t");
		tokenizer<char_separator> tokens(line, sep);
		if (tokens.has_more()) tokens.next_token();
		if (tokens.has_more()) {
			*bx1 = atoi(tokens.next_token().c_str());
		}
		if (tokens.has_more()) {
			*by1 = atoi(tokens.next_token().c_str());
		}
		if (tokens.has_more()) {
			*bx2 = atoi(tokens.next_token().c_str());
		}
		if (tokens.has_more()) {
			*by2 = atoi(tokens.next_token().c_str());
		}
		return true;
	}
	return false;
}

void g_postscript(char *fname, double wx, double wy) throw(ParserError) {
	/*
	fname ... the filename
	wx .... the users desired width  in GLE units
	wy .... the users desired height in GLE units
	*/
	int bx1 = 0, by1 = 0, bx2 = 0, by2 = 0;
	double cx, cy;
	/* Open PostScript file */
	ifstream input;
	validate_open_input_stream(input, fname);
	while (input.good()) {
		string line;
		getline(input, line);
		if (g_parse_ps_boundingbox(line, &bx1, &by1, &bx2, &by2))
		{
			// do not get confused by second bounding box
			break;
		}
	}
	/* compute size if only height or width is given */
	bx2 -= bx1;
	by2 -= by1;
	if (bx2 == 0 || by2 == 0) {
		gprint("Invalid bounding box in EPS file\n");
		return;
	}
	if (fabs(wy) < 1e-18) {
		if (fabs(wx) < 1e-18) {
			// if with and hight is zero, then use default size
			wx = bx2/PS_POINTS_PER_INCH*CM_PER_INCH;
			wy = by2/PS_POINTS_PER_INCH*CM_PER_INCH;
		} else {
			wy = wx*by2/bx2;
		}
	} else if (fabs(wx) < 1e-18) {
		wx = wy*bx2/by2;
	}
	/* non-Postscript device? */
	std::string inbuff(g_get_type());
	if (str_i_str(inbuff, "POSTSCRIPT") == 0) {
		input.close();
		g_get_xy(&cx,&cy);
		g_box_stroke(cx,cy,cx+wx,cy+wy);
		return;
	}
	/* Output PostScript */
	GLERectangle save_box;
	g_get_bounds(&save_box);
	g_devcmd("/GLESTATE save def\n");
	g_devcmd("gsave\n");
	g_devcmd("/a4small {} def /legal {} def\n");
	g_devcmd("/letter {} def /note {} def /copypage {} def\n");
	g_devcmd("/erasepage {} def /showpage {} def\n");
	g_gsave();
	g_get_xy(&cx,&cy);
	g_translate(cx, cy);
	g_set_pos(0, 0);
	g_scale(wx/bx2, wy/by2);
	g_translate(-bx1, -by1);
	g_devcmd("0 setgray 0 setlinecap 1 setlinewidth 0 setlinejoin\n");
	g_devcmd("10 setmiterlimit [] 0 setdash newpath\n");
	/* Copy Poscrscript file over */
	string begin_doc("%%BeginDocument: ");
	begin_doc += fname;
	begin_doc += "\n";
	g_devcmd(begin_doc.c_str());
	input.seekg(0, ios_base::beg);
	while (input.good()) {
		string line;
		getline(input, line);
		if (!str_ni_equals(line.c_str(), "%%BoundingBox:", strlen("%%BoundingBox:")) &&
		    !str_ni_equals(line.c_str(), "%%HiResBoundingBox:", strlen("%%HiResBoundingBox:")) &&
		    !str_ni_equals(line.c_str(), "%%EOF", strlen("%%EOF"))) {
			str_trim_right(line);
			line += "\n";
			g_devcmd(line.c_str());
		}
	}
	input.close();
	g_devcmd("%%EndDocument\n");
	/* Done copy */
	g_devcmd("grestore GLESTATE restore\n");
	g_grestore();
	/* Adjust bounds */
	g_set_bounds(&save_box);
	g_update_bounds(cx, cy);
	g_update_bounds(cx+wx, cy+wy);
}


int g_bitmap_string_to_type(const std::string& stype) {
	return g_bitmap_string_to_type(stype.c_str());
}

int g_bitmap_string_to_type(const char* stype) {
	if (str_i_equals(stype, "tiff")) {
		return BITMAP_TYPE_TIFF;
	} else if (str_i_equals(stype, "tif")) {
		return BITMAP_TYPE_TIFF;
	} else if (str_i_equals(stype, "gif")) {
		return BITMAP_TYPE_GIF;
	} else if (str_i_equals(stype, "png")) {
		return BITMAP_TYPE_PNG;
	} else if (str_i_equals(stype, "jpg")) {
		return BITMAP_TYPE_JPEG;
	} else if (str_i_equals(stype, "jpeg")) {
		return BITMAP_TYPE_JPEG;
	} else {
		return BITMAP_TYPE_UNK;
	}
}

void g_update_bitmap_type(const string& fname, int* type) throw(ParserError) {
	if (*type == 0) {
		string ext;
		GetExtension(fname, ext);
		*type = g_bitmap_string_to_type(ext.c_str());
		if (*type == BITMAP_TYPE_UNK) {
			g_throw_parser_error("unsupported bitmap type: '", ext.c_str(), "'");
		}
	}
}

void g_bitmap_type_to_string(int type, string &typestr) {
	switch (type) {
		case BITMAP_TYPE_TIFF:
			typestr = "TIFF"; break;
		case BITMAP_TYPE_GIF:
			typestr = "GIF"; break;
		case BITMAP_TYPE_PNG:
			typestr = "PNG"; break;
		case BITMAP_TYPE_JPEG:
			typestr = "JPEG"; break;
	}
}

int g_device_to_bitmap_type(int device) {
	switch (device) {
		case GLE_DEVICE_PNG:
			return BITMAP_TYPE_PNG;
		case GLE_DEVICE_JPEG:
			return BITMAP_TYPE_JPEG;
		default:
			return BITMAP_TYPE_UNK;
	}
}

bool g_bitmap_supports_type(int type) {
	switch (type) {
		case BITMAP_TYPE_TIFF:
			#ifdef HAVE_LIBTIFF
				return true;
			#else
				return false;
			#endif
		case BITMAP_TYPE_GIF:
			return true;
		case BITMAP_TYPE_PNG:
			#ifdef HAVE_LIBPNG
				return true;
			#else
				return false;
			#endif
		case BITMAP_TYPE_JPEG:
			#ifdef HAVE_LIBJPEG
				return true;
			#else
				return false;
			#endif
		default:
			return false;
	}
}

void g_bitmap_add_supported_type(int type, ostream& out, int* i) {
	if (g_bitmap_supports_type(type)) {
		string str;
		if ((*i) != 0) out << ", ";
		g_bitmap_type_to_string(type, str);
		out << str;
		(*i)++;
	}
}

string g_bitmap_supported_types() {
	int i = 0;
	stringstream out;
	g_bitmap_add_supported_type(BITMAP_TYPE_JPEG, out, &i);
	g_bitmap_add_supported_type(BITMAP_TYPE_PNG,  out, &i);
	g_bitmap_add_supported_type(BITMAP_TYPE_TIFF, out, &i);
	g_bitmap_add_supported_type(BITMAP_TYPE_GIF,  out, &i);
	if (i == 0) out << "None";
	return out.str();
}

GLEBitmap* g_bitmap_type_to_object(int type) {
	switch (type) {
		case BITMAP_TYPE_TIFF:
			#ifdef HAVE_LIBTIFF
				return new GLETIFF();
			#else
				return NULL;
			#endif
		case BITMAP_TYPE_GIF:
			return new GLEGIF();
		case BITMAP_TYPE_PNG:
			#ifdef HAVE_LIBPNG
				return new GLEPNG();
			#else
				return NULL;
			#endif
		case BITMAP_TYPE_JPEG:
			return new GLEJPEG();
		default:
			return NULL;
	}
}

void g_bitmap(string& fname, double wx, double wy, int type) throw(ParserError) {
	fname = GLEExpandEnvironmentVariables(fname);
	validate_file_name(fname, true);
	g_update_bitmap_type(fname, &type);
	if (type == 0) {
		// Unknown bitmap type
		return;
	}
	/* Get bitmap type string */
	string typestr;
	g_bitmap_type_to_string(type, typestr);
	/* Get bitmap writer object */
	GLEBitmap* bitmap = g_bitmap_type_to_object(type);
	if (bitmap == NULL) {
		g_throw_parser_error("support for ", typestr.c_str(), " bitmaps not enabled");
	}
	/* Open file */
	if (bitmap->open(fname) == 0) {
		g_throw_parser_error("can't open bitmap file: '", fname.c_str(), "'");
	}
	g_bitmap(bitmap, wx, wy, type);
	bitmap->close();
	delete bitmap;
}

void g_bitmap(GLEBitmap* bitmap, double wx, double wy, int type) throw(ParserError) {
	/* Read header */
	int result = bitmap->readHeader();
	if (result != GLE_IMAGE_ERROR_NONE) {
		stringstream str;
		str << "error reading bitmap header '" << bitmap->getFName() << "': ";
		if (bitmap->getError() == "") str << "unknown";
		else str << bitmap->getError();
		g_throw_parser_error(str.str());
	}
	/* Get current point */
	double cx, cy;
	g_get_xy(&cx,&cy);
	/* Update scale */
	if (wx == 0.0 || wy == 0.0) {
		double bw = bitmap->getWidth();
		double bh = bitmap->getHeight();
		if (wx == 0.0 && bh != 0.0) {
			wx = wy*bw/bh;
		}
		if (wy == 0.0 && bw != 0.0) {
			wy = wx*bh/bw;
		}
	}
	/* Actually draw the bitmap */
	GLEPoint pos(cx, cy);
	GLEPoint scale(wx, wy);
	g.dev->bitmap(bitmap, &pos, &scale, type);
	/* Output info */
	if (!g_is_dummy_device() && type != BITMAP_TYPE_USER && g_verbosity() >= 2) {
		cerr << "{" << bitmap->getFName() << "-";
		bitmap->printInfo(cerr);
		cerr << "}";
	}
	/* Adjust bounds */
	g_update_bounds(cx, cy);
	g_update_bounds(cx+wx, cy+wy);
}

void g_bitmap_info(string& fname, int xvar, int yvar, int type)  throw(ParserError) {
	fname = GLEExpandEnvironmentVariables(fname);
	validate_file_name(fname, true);
	g_update_bitmap_type(fname, &type);
	if (type == 0) {
		// Unknown bitmap type
		return;
	}
	/* Get bitmap type string */
	string typestr;
	g_bitmap_type_to_string(type, typestr);
	/* Get bitmap writer object */
	GLEBitmap* bitmap = g_bitmap_type_to_object(type);
	if (bitmap == NULL) {
		g_throw_parser_error("support for ", typestr.c_str(), " bitmaps not enabled");
	}
	/* Open file */
	if (bitmap->open(fname) == 0) {
		g_throw_parser_error("can't open bitmap file: '", fname.c_str(), "'");
	}
	/* Read header */
	int result = bitmap->readHeader();
	if (result != GLE_IMAGE_ERROR_NONE) {
		stringstream str;
		str << "error reading bitmap header '" << bitmap->getFName() << "': ";
		if (bitmap->getError() == "") str << "unknown";
		else str << bitmap->getError();
		g_throw_parser_error(str.str());
	}
	var_set(xvar, (double)bitmap->getWidth());
	var_set(yvar, (double)bitmap->getHeight());
	bitmap->close();
	delete bitmap;
}

void g_arrowpoints(double cx, double cy, double dx, double dy, GLEArrowPoints* pts);

void g_arrowline(double x2, double y2, int flag, int can_fillpath) throw(ParserError) {
	double x1,y1;
	GLECore* core = g_get_core();
	if (core->isComputingLength()) {
		core->addToLength(g_get_xy().distance(GLEPoint(x2, y2)));
	}
	GLEWithoutUpdates noUpdates;
	if ((flag & 3) == 0) {
		g_line(x2,y2);
		return;
	}
	g_get_xy(&x1,&y1);
	if (can_fillpath && g.arrowstyle < GLE_ARRSTY_SUB) {
		g_psarrow(x1, y1, x2, y2, flag);
	} else {
		if ((flag & 1) != 0) g_arrow(x2-x1, y2-y1, can_fillpath);
		g_line(x2, y2);
		if ((flag & 2) != 0) g_arrow(x1-x2, y1-y2, can_fillpath);
	}
}

int g_arrow_style() {
	return g.arrowstyle;
}

int g_arrow_tip() {
	return g.arrowtip;
}

void g_arrowpoints(GLEPoint& pt, double dx, double dy, GLEArrowProps* arrow, double lwd, GLEArrowPoints* pts) {
	double radius, angle, dx1, dy1;
	double arrow_angle = arrow->angle*GLE_PI/180;
	xy_polar(dx, dy, &radius, &angle);
	if (arrow->tip == GLE_ARRTIP_SHARP && arrow->style != GLE_ARRSTY_OLD35) {
		double dist = lwd/(2*sin(arrow_angle));
		polar_xy(dist, angle, &dx1, &dy1);
	} else {
		dx1 = dy1 = 0.0;
	}
	pts->xt = pt.getX() + dx1;
	pts->yt = pt.getY() + dy1;
	polar_xy(arrow->size, angle+arrow->angle, &dx, &dy);
	pts->xa = pt.getX() + dx1 + dx;
	pts->ya = pt.getY() + dy1 + dy;
	polar_xy(arrow->size, angle-arrow->angle, &dx, &dy);
	pts->xb = pt.getX() + dx1 + dx;
	pts->yb = pt.getY() + dy1 + dy;
	// How far should the line stick into the arrow?
	if (arrow->style == GLE_ARRSTY_OLD35 || arrow->tip == GLE_ARRTIP_SHARP) {
		double dist = 1.1*lwd/(2*tan(arrow_angle));
		polar_xy(dist, angle, &dx1, &dy1);
		pts->xl = pt.getX() + dx1;
		pts->yl = pt.getY() + dy1;
	} else {
		pts->xl = pt.getX();
		pts->yl = pt.getY();
	}
}

void g_arrowpoints(double cx, double cy, double dx, double dy, GLEArrowPoints* pts) {
	double lwd;
	GLEArrowProps arrow;
	GLEPoint pt(cx, cy);
	g_arrowsize_actual(&arrow, &lwd, true);
	g_arrowpoints(pt, dx, dy, &arrow, lwd, pts);
}

void g_arrowsize(GLEArrowProps* arrow) {
	double width;
	double arrow_len = g.arrowsize;
	double arrow_angle = g.arrowangle;
	arrow->tip = g.arrowtip;
	arrow->style = g.arrowstyle;
	g_get_line_width(&width);
	if (width == 0) width = 0.02;
	if (arrow_angle <= 0.0) {
		if (arrow->style == GLE_ARRSTY_OLD35) {
			arrow_angle = 10;
		} else {
			arrow_angle = 15;
		}
		if (width > 0.1) arrow_angle = 20;
		if (width > 0.3) arrow_angle = 30;
	}
	if (arrow_len <= 0.0) {
		double ang_rad = arrow_angle*GLE_PI/180;
		if (arrow->style == GLE_ARRSTY_OLD35) {
			g_get_hei(&arrow_len);
			arrow_len = arrow_len/2 * cos(ang_rad);
			if (arrow_len*tan(ang_rad) < width/1.5) {
				arrow_len = width / (1.5*tan(ang_rad));
			}
		} else {
			arrow_len = 0.2;
			double fac = (20*width+2.5)/(20*width+1);
			if (arrow_len*tan(ang_rad) < width*fac) {
				arrow_len = width*fac / tan(ang_rad);
			}
			if (arrow->style == GLE_ARRSTY_EMPTY || arrow->style == GLE_ARRSTY_FILLED) {
				arrow->size += width/2;
			}
		}
	}
	//cout << "size: " << arrow_len << endl;
	arrow->size = arrow_len;
	arrow->angle = arrow_angle;
}

void g_arrowsize_actual(GLEArrowProps* arrow, double* lwd, bool sz_az) {
	g_get_line_width(lwd);
	g_arrowsize(arrow);
	g_arrowsize_transform(arrow, *lwd, sz_az);
}

void g_arrowsize_transform(GLEArrowProps* arrow, double lwd, bool sz_az) {
	double ang_rad = arrow->angle*GLE_PI/180;
	if (arrow->style != GLE_ARRSTY_OLD35) {
		if (arrow->style == GLE_ARRSTY_EMPTY || arrow->style == GLE_ARRSTY_FILLED) {
			arrow->size -= lwd/2;
		}
		if (arrow->tip == GLE_ARRTIP_SHARP) {
			double dist = lwd/(2*sin(ang_rad));
			arrow->size -= dist;
		}
		if (arrow->size < lwd*0.1) {
			arrow->size = lwd*0.1;
		}
	}
	if (sz_az) {
		arrow->size /= cos(ang_rad);
	}
}

void g_init_arrow_head(GLECurvedArrowHead* head, bool startend) {
	double lwidth;
	GLEArrowProps arrow;
	g_arrowsize_actual(&arrow, &lwidth, false);
	head->setLineWidth(lwidth);
	head->setSharp(arrow.tip == GLE_ARRTIP_SHARP);
	head->setArrowAngleSizeSharp(arrow.style, arrow.size, arrow.angle);
	head->setStartEnd(startend);
}

void g_init_arrow_head_from_properties(GLECurvedArrowHead* head, GLEPropertyStore* props, double fac, bool startend) {
	GLEArrowProps arrow;
	double lwd = props->getRealProperty(GLEDOPropertyLineWidth) * fac;
	arrow.size = props->getRealProperty(GLEDOPropertyArrowSize) * fac;
	arrow.angle = props->getRealProperty(GLEDOPropertyArrowAngle);
	arrow.tip = (GLEArrowTip)props->getIntProperty(GLEDOPropertyArrowTip);
	arrow.style = GLE_ARRSTY_FILLED;
	g_arrowsize_transform(&arrow, lwd, false);
	head->setLineWidth(lwd);
	head->setSharp(arrow.tip == GLE_ARRTIP_SHARP);
	head->setArrowAngleSizeSharp(arrow.style, arrow.size, arrow.angle);
	head->setStartEnd(startend);
}

double GLEArcNormalizedAngle2(double a1, double a2) {
	return g_arc_normalized_angle2(a1, a2);
}

void GLEArcUpdateCurvedArrowHeads(GLECurvedArrowHead* head_start,
		                          GLECurvedArrowHead* head_end,
		                          double* t1,
		                          double* t2,
		                          GLEPropertyStore* props,
		                          double fac,
		                          int arrow)
{
	if (arrow == 1 || arrow == 3) g_init_arrow_head_from_properties(head_start, props, fac, true);
	if (arrow == 2 || arrow == 3) g_init_arrow_head_from_properties(head_end, props, fac, false);
	g_update_arc_bounds_for_arrow_heads(head_start, head_end, t1, t2);
}

void GLEGetArrowPoints(GLEPoint& pt, double dx, double dy, GLEPropertyStore* props, double fac, GLEArrowPoints* pts) {
	GLEArrowProps arrow;
	double lwd = props->getRealProperty(GLEDOPropertyLineWidth) * fac;
	arrow.size = props->getRealProperty(GLEDOPropertyArrowSize) * fac;
	arrow.angle = props->getRealProperty(GLEDOPropertyArrowAngle);
	arrow.tip = (GLEArrowTip)props->getIntProperty(GLEDOPropertyArrowTip);
	arrow.style = GLE_ARRSTY_FILLED;
	g_arrowsize_transform(&arrow, lwd, true);
	g_arrowpoints(pt, dx, dy, &arrow, lwd, pts);
}

void GLEGetArrowPointsNoProps(GLEPoint& pt, double dx, double dy, double lwd, double size, double angle, GLEArrowPoints* pts) {
	GLEArrowProps arrow;
	arrow.size = size;
	arrow.angle = angle;
	arrow.tip = GLE_ARRTIP_SHARP;
	arrow.style = GLE_ARRSTY_FILLED;
	g_arrowsize_transform(&arrow, lwd, true);
	g_arrowpoints(pt, dx, dy, &arrow, lwd, pts);
}

void g_set_arrow_size(double size) {
	g.arrowsize = size;
}

void g_set_arrow_angle(double angle) {
	g.arrowangle = angle;
}

void g_set_arrow_style(int shape) {
	g.arrowstyle = shape;
}

void g_set_arrow_style(const char* shape) throw(ParserError) {
	if (str_i_equals(shape, "SIMPLE")) {
		g_set_arrow_style(GLE_ARRSTY_SIMPLE);
	} else if (str_i_equals(shape, "FILLED")) {
		g_set_arrow_style(GLE_ARRSTY_FILLED);
	} else if (str_i_equals(shape, "EMPTY")) {
		g_set_arrow_style(GLE_ARRSTY_EMPTY);
	} else {
		string name = "ARROW_";
		name += shape;
		str_to_uppercase(name);
		GLESub* sub = sub_find(name.c_str());
		if (sub == NULL || sub->getIndex() == -1) {
			g_throw_parser_error("subroutine defining arrow style '", name.c_str(), "' not defined");
		}
		g_set_arrow_style(GLE_ARRSTY_SUB + sub->getIndex());
	}
}

void g_set_arrow_tip(int tip) {
	g.arrowtip = tip;
}

void g_set_arrow_tip(const char* tip) throw (ParserError) {
	if (str_i_equals(tip, "SHARP")) {
		g_set_arrow_tip(GLE_ARRTIP_SHARP);
	} else if (str_i_equals(tip, "ROUND")) {
		g_set_arrow_tip(GLE_ARRTIP_ROUND);
	} else {
		g_throw_parser_error("unsupported arrow tip style '", tip, "'");
	}
}

void g_arrow(double dx, double dy, int can_fillpath) throw(ParserError) {
	int old_join;
	char old_lstyle[15];
	g_get_line_style(old_lstyle);
	if (!(old_lstyle[0] == '1' && old_lstyle[1] == 0)) {
		g_set_line_style("1");
	}
	int desired_join = g.arrowtip == GLE_ARRTIP_ROUND ? 1 : 0;
	g_get_line_join(&old_join);
	if (old_join != desired_join) {
		g_set_line_join(desired_join);
	}
	double x1, y1;
	g_get_xy(&x1, &y1);
	if (g.arrowstyle >= GLE_ARRSTY_SUB) {
		double radius, angle, lwd;
		xy_polar(dx, dy, &radius, &angle);
		GLEArrowProps arrow;
		g_arrowsize_actual(&arrow, &lwd, true);
		double args[3];
		args[0] = angle;
		args[1] = arrow.angle;
		args[2] = arrow.size;
		call_sub_byid(arrow.style-GLE_ARRSTY_SUB, args, 3, "(used for defining arrow style)");
	} else {
		GLEArrowPoints pts;
		g_arrowpoints(x1, y1, dx, dy, &pts);
		g_set_path(true);
		g_newpath();
		g_move(pts.xa, pts.ya);
		g_line(pts.xt, pts.yt);
		g_line(pts.xb, pts.yb);
		if (g.arrowstyle != GLE_ARRSTY_SIMPLE) {
			g_closepath();
			GLERC<GLEColor> curr_color(g_get_color());
			GLERC<GLEColor> curr_fill(g_get_fill());
			if (g.arrowstyle == GLE_ARRSTY_EMPTY) g_set_fill(GLE_COLOR_WHITE);
			else g_set_fill(curr_color);
			g_fill();
			g_set_fill(curr_fill);
		}
		if (g.arrowstyle != GLE_ARRSTY_OLD35) {
			g_stroke();
		}
		g_set_path(false);
	}
	if (old_join != desired_join) {
		g_set_line_join(old_join);
	}
	if (!(old_lstyle[0] == '1' && old_lstyle[1] == 0)) {
		g_set_line_style(old_lstyle);
	}
	g_move(x1, y1);
}

void g_psarrow(double x1, double y1, double x2, double y2, int flag) {
	double dx = x2-x1;
	double dy = y2-y1;
	int old_join;
	char old_lstyle[15];
	GLEArrowPoints pts1, pts2;
	g_arrowpoints(x1, y1,  dx,  dy, &pts1);
	g_arrowpoints(x2, y2, -dx, -dy, &pts2);
	g_get_line_style(old_lstyle);
	int desired_join = g.arrowtip == GLE_ARRTIP_ROUND ? 1 : 0;
	g_get_line_join(&old_join);
	if (old_join != desired_join) {
		g_set_line_join(desired_join);
	}
	/* draw line */
	if ((flag & 1)>0) g_move(pts1.xl, pts1.yl);
	else g_move(x1, y1);
	if ((flag & 2)>0) g_line(pts2.xl, pts2.yl);
	else g_line(x2, y2);
	/* draw arrow heads */
	if (!(old_lstyle[0] == '1' && old_lstyle[1] == 0)) {
		g_set_line_style("1");
	}
	g_set_path(true);
	g_newpath();
	if ((flag & 1)>0) {
		g_move(pts1.xa, pts1.ya);
		g_line(pts1.xt, pts1.yt);
		g_line(pts1.xb, pts1.yb);
		if (g.arrowstyle != GLE_ARRSTY_SIMPLE) g_closepath();
	}
	if ((flag & 2)>0) {
		g_move(pts2.xa, pts2.ya);
		g_line(pts2.xt, pts2.yt);
		g_line(pts2.xb, pts2.yb);
		if (g.arrowstyle != GLE_ARRSTY_SIMPLE) g_closepath();
	}
	if (g.arrowstyle != GLE_ARRSTY_SIMPLE) {
		GLERC<GLEColor> cur_color(g_get_color());
		GLERC<GLEColor> cur_fill(g_get_fill());
		if (g.arrowstyle == GLE_ARRSTY_EMPTY) g_set_fill(GLE_COLOR_WHITE);
		else g_set_fill(cur_color);
		g_fill();
		g_set_fill(cur_fill);
	}
	if (g.arrowstyle != GLE_ARRSTY_OLD35) {
		g_stroke();
	}
	g_set_path(false);
	if (old_join != desired_join) {
		g_set_line_join(old_join);
	}
	if (!(old_lstyle[0] == '1' && old_lstyle[1] == 0)) {
		g_set_line_style(old_lstyle);
	}
	g_move(x2, y2);
}

void g_set_pdf_image_format(const char* format) {
	if (str_i_equals(format, "AUTO")) {
		g.pdfimgformat = PDF_IMG_COMPR_AUTO;
	} else if (str_i_equals(format, "ZIP")) {
		g.pdfimgformat = PDF_IMG_COMPR_ZIP;
	} else if (str_i_equals(format, "JPEG")) {
		g.pdfimgformat = PDF_IMG_COMPR_JPEG;
	} else if (str_i_equals(format, "PS")) {
		g.pdfimgformat = PDF_IMG_COMPR_PS;
	}
}

void g_set_tex_scale(const char* ss) {
	TeXInterface* iface = TeXInterface::getInstance();
	if (str_i_equals(ss, "NONE")) {
		iface->setScaleMode(TEX_SCALE_MODE_NONE);
	} else if (str_i_equals(ss, "FIXED")) {
		iface->setScaleMode(TEX_SCALE_MODE_FIXED);
	} else if (str_i_equals(ss, "SCALE")) {
		iface->setScaleMode(TEX_SCALE_MODE_SCALE);
	}
}

void g_set_tex_labels(bool onoff) {
	g.texlabels = onoff;
}

bool g_get_tex_labels() {
	return g.texlabels;
}

int g_get_pdf_image_format() {
	return g.pdfimgformat;
}

int g_get_compatibility() {
	return g_compatibility;
}

int g_parse_compatibility(const string& compat) throw (ParserError) {
	TokenizerLanguage lang;
	lang.setSpaceTokens(" ");
	lang.setSingleCharTokens(".");
	StringTokenizer tokens(&lang, true);
	string compat_noquote = compat;
	str_remove_quote(compat_noquote);
	tokens.set_string(compat_noquote);
	int minor = 0;
	int micro = 0;
	int major = tokens.next_integer();
	if (tokens.has_more_tokens()) {
		tokens.ensure_next_token(".");
		minor = tokens.next_integer();
	}
	if (tokens.has_more_tokens()) {
		tokens.ensure_next_token(".");
		micro = tokens.next_integer();
	}
	int value = (major << 16) | (minor << 8) | micro;
	if (value > GLE_COMPAT_MOST_RECENT) {
		stringstream err;
		int cr_major = (GLE_COMPAT_MOST_RECENT >> 16) & 0xFF;
		int cr_minor = (GLE_COMPAT_MOST_RECENT >> 8) & 0xFF;
		int cr_micro = GLE_COMPAT_MOST_RECENT & 0xFF;
		err << "can't set compatibility beyond " << cr_major << "." << cr_minor << "." << cr_micro;
		throw tokens.error(err.str());
	}
	return value;
}

int g_set_compatibility(const string& compat) throw (ParserError) {
	int value = g_parse_compatibility(compat);
	g_set_compatibility(value);
	return value;
}

void g_set_compatibility(int compat) {
	g_compatibility = compat;
}

void g_set_iconst(int i, int value) {
	g.intconst[i] = value;
}

void g_set_fconst(int i, double value) {
	g.floatconst[i] = value;
}

int g_get_iconst(int i) {
	return g.intconst[i];
}

double g_get_fconst(int i) {
	return g.floatconst[i];
}

GLEDevice::GLEDevice():
	m_recording(false),
	m_resolution(300.0)
{
}

GLEDevice::~GLEDevice() {
}

void GLEDevice::bitmap(GLEBitmap* bitmap, GLEPoint* pos, GLEPoint* scale, int type) {
}

void GLEDevice::getRecordedBytes(string* output) {
	*output = "";
}

void GLEDevice::set_fill_method(int m) {
}

void GLEDevice::computeBoundingBox(double width, double height) {
	if (g_is_fullpage()) {
		m_boundingBox.setX(72*width/CM_PER_INCH);
		m_boundingBox.setY(72*height/CM_PER_INCH);
	} else {
		// Make bounding box a little larger (bounding box tweak)
		m_boundingBox.setX(72*width/CM_PER_INCH+2);
		m_boundingBox.setY(72*height/CM_PER_INCH+2);
	}
}

void GLEDevice::computeBoundingBox(double width, double height, int* int_bb_x, int* int_bb_y) {
	computeBoundingBox(width, height);
	if (g_is_fullpage()) {
		// Just round the bounding box to make sure papersize detection works in GhostView
		*int_bb_x = (int)floor(m_boundingBox.getX()+0.5);
		*int_bb_y = (int)floor(m_boundingBox.getY()+0.5);
	} else {
		// Make sure approximate bounding box is large enough for ImageMagick and GhostView
		*int_bb_x = (int)ceil(m_boundingBox.getX()+1e-6);
		*int_bb_y = (int)ceil(m_boundingBox.getY()+1e-6);
	}
}

GLEMeasureBox::GLEMeasureBox() : GLERectangle() {
}

GLEMeasureBox::~GLEMeasureBox() {
}

void GLEMeasureBox::measureStart() {
	g_get_bounds(&m_XMin, &m_YMin, &m_XMax, &m_YMax);
	g_init_bounds();
}

void GLEMeasureBox::measureEnd() {
	double x1, y1, x2, y2;
	g_get_bounds(&x1, &y1, &x2, &y2);
	if (m_XMin <= m_XMax && m_YMin <= m_YMax) {
		g_update_bounds(m_XMin, m_YMin);
		g_update_bounds(m_XMax, m_YMax);
	}
	m_XMin = x1; m_YMin = y1;
	m_XMax = x2; m_YMax = y2;
}

void GLEMeasureBox::measureEndIgnore() {
	double x1, y1, x2, y2;
	g_get_bounds(&x1, &y1, &x2, &y2);
	g_restore_bounds(m_XMin, m_XMax, m_YMin, m_YMax);
	m_XMin = x1; m_YMin = y1;
	m_XMax = x2; m_YMax = y2;
}

GLEPolynomial::GLEPolynomial(double* coefs, int degree) {
	m_Coefs = coefs;
	m_Degree = degree;
}

GLEPolynomial::~GLEPolynomial() {
}

void GLEPolynomial::print() {
	int d = degree();
	cout << "Polynomial: ";
	for (int i = d; i >= 0; i--) {
		double c = a(i);
		if (c >= 0.0 && i != d) { cout << "+"; }
		cout << c;
		if (i != 0) { cout << "*x^" << i; }
	}
	cout << endl;
}

void GLEPolynomial::horner(double v) {
	int d = degree();
	for (int i = d-1; i >= 0; i--) {
		set(i, a(i) + a(i+1)*v);
	}
	for (int i = 0; i < d; i++) {
		set(i, a(i+1));
	}
	setDegree(d-1);
}

double GLEPolynomial::evalPoly(double t) {
	double r = 0.0;
	for (int i = degree(); i >= 0; i--) {
		r *= t;
		r += a(i);
	}
	return r;
}

double GLEPolynomial::evalDPoly(double t) {
	double r = 0.0;
	for (int i = degree(); i > 0; i--) {
		r *= t;
		r += i * a(i);
	}
	return r;
}

double GLEPolynomial::newtonRaphson(double t, double prec) {
	while (true) {
		double v = evalPoly(t);
		if (fabs(v) < 1e-9) break;
		t = t - evalPoly(t)/evalDPoly(t);
	}
	return t;
}

GLERange::GLERange() {
	m_Min = GLE_INF; m_Max = -GLE_INF;
}

GLERange::~GLERange() {
}

void GLERange::setMinMax(double min, double max) {
	setMin(min);
	setMax(max);
}

void GLERange::copy(GLERange* other) {
	m_Min = other->getMin();
	m_Max = other->getMax();
}

void GLERange::copyHas(GLERangeSet* other) {
	if (other->hasMin()) m_Min = other->getMin();
	if (other->hasMax()) m_Max = other->getMax();
}

void GLERange::initRange() {
	setMin(GLE_INF);
	setMax(-GLE_INF);
}

bool GLERange::isMinValid() const {
	return !gle_isinf(m_Min);
}

bool GLERange::isMaxValid() const {
	return !gle_isinf(m_Max);
}

void GLERange::updateRange(double value) {
	if (value < m_Min) m_Min = value;
	if (value > m_Max) m_Max = value;
}

void GLERange::clip(double* value) {
	if (*value < m_Min) *value = m_Min;
	if (*value > m_Max) *value = m_Max;
}

bool GLERange::contains(double value) {
	return value >= m_Min && value <= m_Max;
}

ostream& GLERange::printRange(ostream& out) const {
	out << "min = ";
	if (isMinValid()) out << m_Min; else out << "?";
	out << " max = ";
	if (isMaxValid()) out << m_Max; else out << "?";
	return out;
}

GLERangeSet::GLERangeSet() : GLERange() {
	m_MinSet = m_MaxSet = false;
}

GLERangeSet::~GLERangeSet() {
}

void GLERangeSet::setMinSet(double min) {
	setMin(min);
	m_MinSet = true;
}

void GLERangeSet::setMaxSet(double max) {
	setMax(max);
	m_MaxSet = true;
}

void GLERangeSet::setMinMaxSet(double min, double max) {
	setMinSet(min);
	setMaxSet(max);
}

void GLERangeSet::resetSet() {
	m_MinSet = m_MaxSet = false;
}

void GLERangeSet::setMinIfNotSet(double min) {
	if (!hasMin()) setMin(min);
}

void GLERangeSet::setMaxIfNotSet(double max) {
	if (!hasMax()) setMax(max);
}

void GLERangeSet::copyIfNotSet(GLERange* other) {
	if (!hasMin()) setMin(other->getMin());
	if (!hasMax()) setMax(other->getMax());
}

void GLERangeSet::initRangeIfNotSet() {
	if (!hasMin()) setMin(GLE_INF);
	if (!hasMax()) setMax(-GLE_INF);
}

void GLERangeSet::copySet(GLERangeSet* other) {
	GLERange::copy(other);
	m_MinSet = other->hasMin();
	m_MaxSet = other->hasMax();
}

GLERectangle::GLERectangle() {
	m_XMin = m_YMin = m_XMax = m_YMax = 0.0;
}

GLERectangle::GLERectangle(double xmin, double ymin, double xmax, double ymax) {
	m_XMin = xmin; m_YMin = ymin;
	m_XMax = xmax; m_YMax = ymax;
}

GLERectangle::GLERectangle(GLERectangle* other) {
	m_XMin = other->m_XMin; m_YMin = other->m_YMin;
	m_XMax = other->m_XMax;	m_YMax = other->m_YMax;
}

GLERectangle::~GLERectangle() {
}

void GLERectangle::reset() {
	m_XMin = m_YMin = m_XMax = m_YMax = 0.0;
}

void GLERectangle::normalize() {
	if (m_XMin > m_XMax) {
		double a = m_XMin;
		m_XMin = m_XMax;
		m_XMax = a;
	}
	if (m_YMin > m_YMax) {
		double a = m_YMin;
		m_YMin = m_YMax;
		m_YMax = a;
	}
}

void GLERectangle::copy(GLERectangle* other) {
	m_XMin = other->m_XMin; m_YMin = other->m_YMin;
	m_XMax = other->m_XMax; m_YMax = other->m_YMax;
}

void GLERectangle::copy(GLEPoint* point) {
	m_XMin = point->getX(); m_YMin = point->getY();
	m_XMax = point->getX(); m_YMax = point->getY();
}

void GLERectangle::getDimensions(double* xmin, double* ymin, double* xmax, double* ymax) {
	*xmin = m_XMin;	*ymin = m_YMin;
	*xmax = m_XMax;	*ymax = m_YMax;
}

void GLERectangle::setDimensions(double xmin, double ymin, double xmax, double ymax) {
	m_XMin = xmin; m_YMin = ymin;
	m_XMax = xmax; m_YMax = ymax;
}

void GLERectangle::translate(double x, double y) {
	m_XMin += x; m_YMin += y;
	m_XMax += x; m_YMax += y;
}

void GLERectangle::translate(GLEPoint* p) {
	m_XMin += p->getX(); m_YMin += p->getY();
	m_XMax += p->getX(); m_YMax += p->getY();
}

void GLERectangle::scale(double s) {
	m_XMin *= s; m_YMin *= s;
	m_XMax *= s; m_YMax *= s;
}

void GLERectangle::subtractXFrom(double x) {
	m_XMin = x - m_XMin;
	m_XMax = x - m_XMax;
}

void GLERectangle::subtractYFrom(double y) {
	m_YMin = y - m_YMin;
	m_YMax = y - m_YMax;
}

void GLERectangle::grow(double d) {
	m_XMin -= d; m_XMax += d;
	m_YMin -= d; m_YMax += d;
}

bool GLERectangle::contains(double x, double y) {
	return x >= m_XMin && x <= m_XMax && y >= m_YMin && y <= m_YMax;
}

void GLERectangle::initRange() {
	m_XMin = m_YMin =  GLE_INF;
	m_XMax = m_YMax = -GLE_INF;
}

void GLERectangle::updateRange(double x, double y) {
	if (x < m_XMin) m_XMin = x;
	if (y < m_YMin) m_YMin = y;
	if (x > m_XMax) m_XMax = x;
	if (y > m_YMax) m_YMax = y;
}

void GLERectangle::updateRange(GLEPoint* pt) {
	if (pt->getX() < m_XMin) m_XMin = pt->getX();
	if (pt->getY() < m_YMin) m_YMin = pt->getY();
	if (pt->getX() > m_XMax) m_XMax = pt->getX();
	if (pt->getY() > m_YMax) m_YMax = pt->getY();
}

void GLERectangle::addToRangeX(GLERange* range) {
	if (m_XMin <= m_XMax) {
		range->updateRange(m_XMin);
		range->updateRange(m_XMax);
	}
}

void GLERectangle::addToRangeY(GLERange* range) {
	if (m_YMin <= m_YMax) {
		range->updateRange(m_YMin);
		range->updateRange(m_YMax);
	}
}

ostream& GLERectangle::print(ostream& out) const {
	out << "(" << m_XMin << "," << m_YMin << ") x (" << m_XMax << "," << m_YMax << ")";
	return out;
}

void GLERectangle::toPoint(GLEJustify just, GLEPoint* point) {
	if (just == GLEJustifyVert) {
		/* vertical box */
		if (fabs(m_YMax - point->getY()) < fabs(m_YMin - point->getY())) point->setY(m_YMax);
		else point->setY(m_YMin);
		return;
	}
	if (just == GLEJustifyHorz) {
		/* horizontal box */
		if (fabs(m_XMax - point->getX()) < fabs(m_XMin - point->getX())) point->setX(m_XMax);
		else point->setX(m_XMin);
		return;
	}
	/* jx and jy value 0..2 */
	int jx = (just & 0xf0) >> 4;
	int jy = just & 0x0f;
	double dx = jx * (m_XMax-m_XMin)/2;
	point->setX(m_XMin + dx);
	double dy = jy * (m_YMax-m_YMin)/2;
	point->setY(m_YMin + dy);
}

GLEPoint::GLEPoint(double x, double y) {
	m_X = x;
	m_Y = y;
}

GLEPoint::GLEPoint() {
	m_X = 0.0;
	m_Y = 0.0;
}

GLEPoint::GLEPoint(const GLEPoint& p) {
	m_X = p.m_X;
	m_Y = p.m_Y;
}

GLEPoint::~GLEPoint() {
}

void GLEPoint::add(double s, const GLEPoint& p) {
	m_X += s*p.m_X;
	m_Y += s*p.m_Y;
}

double GLEPoint::distance(const GLEPoint& p) const {
	double dx = m_X - p.m_X;
	double dy = m_Y - p.m_Y;
	return sqrt(dx*dx + dy*dy);
}

double GLEPoint::norm() const {
	return sqrt(m_X*m_X + m_Y*m_Y);
}

double GLEPoint::normSq() const {
	return m_X*m_X + m_Y*m_Y;
}

void GLEPoint::normalize() {
	double n = norm();
	m_X /= n; m_Y /= n;
}

bool GLEPoint::approx(double x, double y) {
	return fabs(x - m_X) < 1e-6 && fabs(y - m_Y) < 1e-6;
}

void GLEPoint::swap(GLEPoint& other) {
	double backup_x = m_X;
	double backup_y = m_Y;
	m_X = other.m_X;
	m_Y = other.m_Y;
	other.m_X = backup_x;
	other.m_Y = backup_y;
}

ostream& GLEPoint::write(ostream& os) const {
	os << m_X << ":" << m_Y;
	return os;
}

GLELineSegment::GLELineSegment(const GLEPoint& p1, const GLEPoint& p2):
	m_p1(p1),
	m_p2(p2)
{
}

GLELineSegment::GLELineSegment(double x1, double y1, double x2, double y2):
	m_p1(x1, y1),
	m_p2(x2, y2)
{
}

GLEPoint3D::GLEPoint3D() {
	m_C[0] = m_C[1] = m_C[2] = 0.0;
}

GLEPoint3D::GLEPoint3D(double x, double y, double z) {
	m_C[0] = x;
	m_C[1] = y;
	m_C[2] = z;
}

GLEPoint3D::GLEPoint3D(const GLEPoint3D& p) {
	m_C[0] = p.m_C[0];
	m_C[1] = p.m_C[1];
	m_C[2] = p.m_C[2];
}

void GLEPoint3D::dotScalar(double s) {
	m_C[0] *= s;
	m_C[1] *= s;
	m_C[2] *= s;
}

void GLEPoint3D::add(const GLEPoint3D& p) {
	m_C[0] += p.m_C[0];
	m_C[1] += p.m_C[1];
	m_C[2] += p.m_C[2];
}

void GLEPoint3D::subtract(const GLEPoint3D& p) {
	m_C[0] -= p.m_C[0];
	m_C[1] -= p.m_C[1];
	m_C[2] -= p.m_C[2];
}

void GLEPoint3D::addScalar(double s1, double s2, const GLEPoint3D& p) {
	m_C[0] = s1*m_C[0] + s2*p.m_C[0];
	m_C[1] = s1*m_C[1] + s2*p.m_C[1];
	m_C[2] = s1*m_C[2] + s2*p.m_C[2];
}

double GLEPoint3D::norm() const {
	return sqrt(m_C[0]*m_C[0]+m_C[1]*m_C[1]+m_C[2]*m_C[2]);
}

void GLEPoint3D::normalize() {
	double n = norm();
	m_C[0] /= n; m_C[1] /= n; m_C[2] /= n;
}

/**
 * Operator ortho3DUnit
 * Description : Compute a unit vector that is orthogonal to this vector and p.
 * Parameters  : p - Other vector.
 *               r - Result vector.
 */
void GLEPoint3D::ortho3DUnit(const GLEPoint3D& p, GLEPoint3D* r) {
	double ax = m_C[0];
	double ay = m_C[1];
	double az = m_C[2];
	double bx = p.m_C[0];
	double by = p.m_C[1];
	double bz = p.m_C[2];
	double cx = ay*bz - by*az;
	double cy = bx*az - ax*bz;
	double cz = ax*by - bx*ay;
	double div = sqrt(cx*cx+cy*cy+cz*cz);
	r->set(cx/div, cy/div, cz/div);
}

ostream& GLEPoint3D::write(ostream& os) const {
	os << m_C[0] << ", " << m_C[1] << ", " << m_C[2];
	return os;
}

GLEMatrix::GLEMatrix(int rows, int cols) {
	m_Rows = rows;
	m_Cols = cols;
	m_C = new double[m_Rows*m_Cols];
}

GLEMatrix::GLEMatrix(const GLEMatrix& p) {
	m_Rows = p.m_Rows;
	m_Cols = p.m_Cols;
	int size = m_Rows*m_Cols;
	m_C = new double[size];
	for (int i = 0; i < size; i++) {
		m_C[i] = p.m_C[i];
	}
}

GLEMatrix::~GLEMatrix() {
	delete[] m_C;
}

/**
 * Operator dot
 * Description : Compute the dot product with a given vector.
 * Parameters  : p - The given vector
 *               r - The result vector
 */
void GLEMatrix::dot(const GLEPoint3D& p, GLEPoint3D* r) const {
	int idx = 0;
	for (int row = 0; row < 3; row++) {
		double result = 0.0;
		for (int col = 0; col < 3; col++) {
			result += m_C[idx++]*p.m_C[col];
		}
		r->m_C[row] = result;
	}

}

/**
 * Method setVertVector
 * Description : Copy vector to matrix row.
 * Parameters  : row       - Destination row.
 *               col       - Destination column.
 *               p         - Vector to copy.
 */
void GLEMatrix::setVertVector(int row, int col, const GLEPoint3D& p) {
	int idx = row*m_Cols+col;
	for (int pos = 0; pos < 3; pos++) {
		m_C[idx] = p.m_C[pos];
		idx += m_Cols;
	}
}

ostream& GLEMatrix::write(ostream& os) const {
	int offs = 0;
	for (int i = 0; i < m_Rows; i++) {
		for (int j = 0; j < m_Cols; j++) {
			if (j != 0) os << ", ";
			os << m_C[offs++];
		}
		os << endl;
	}
	return os;
}

GLEProjection::GLEProjection() {
}

void GLEProjection::zoom(double factor) {
	GLEPoint3D dir(m_Eye);
	dir.subtract(m_Reference);
	dir.dotScalar(factor);
	m_Eye.add(dir);
}

void GLEProjection::rotate(double angle, bool horiz) {
	GLEPoint3D dir(m_Eye);
	dir.subtract(m_Reference);
	double len = dir.norm();
	double v_cos = cos(angle*GLE_PI/180);
	double v_sin = sin(angle*GLE_PI/180);
	GLEPoint3D n_eye, n_vvec;
	if (horiz) {
		n_eye.set(len*v_sin, 0.0, len*v_cos);
		n_vvec.set(0.0, v_cos, -v_sin);
	} else {
		n_eye.set(0.0, len*v_sin, len*v_cos);
		n_vvec.set(0.0, 1.0, 0.0);
	}
	GLEMatrix axis_rotate(3, 3);
	invToReference(&axis_rotate);
	axis_rotate.dot(n_eye, &m_Eye);
	m_Eye.add(m_Reference);
	axis_rotate.dot(n_vvec, &m_VVector);
}

/**
 * Inspector invToReference
 * Explanation : Returns the inverse viewing transformation
 *               Z-axis parallel to eye-ref.
 *               X-axis orthogonal to vvec and z-axis.
 *               Y-axis orthogonal to previous two axis.
 */
void GLEProjection::invToReference(GLEMatrix* mtrx) {
		GLEPoint3D z_axis(m_Eye);
		z_axis.subtract(m_Reference);
		z_axis.normalize();
		GLEPoint3D x_axis, y_axis;
		m_VVector.ortho3DUnit(z_axis, &x_axis);
		z_axis.ortho3DUnit(x_axis, &y_axis);
		mtrx->setVertVector(0,0,x_axis);
		mtrx->setVertVector(0,1,y_axis);
		mtrx->setVertVector(0,2,z_axis);
}

void GLEProjection::reference(const GLEPoint3D& ref) {
	GLEPoint3D delta;
	GLEMatrix axis_rotate(3, 3);
	invToReference(&axis_rotate);
	axis_rotate.dot(ref, &delta);
	m_Reference.add(delta);
}

void GLEProjection::adjustV(double angle) {
	GLEMatrix axis_rotate(3, 3);
	invToReference(&axis_rotate);
	double v_cos = cos(angle*GLE_PI/180);
	double v_sin = sin(angle*GLE_PI/180);
	GLEPoint3D y_vect(v_sin, v_cos, 0.0);
	axis_rotate.dot(y_vect, &m_VVector);
}

GLELinearEquation::GLELinearEquation() {
	m_A = 1.0;
	m_B = 0.0;
}

GLELinearEquation::~GLELinearEquation() {
}

void GLELinearEquation::fit(double x0, double y0, double x1, double y1) {
	double det = x0 - x1;
	if (det != 0.0) {
		m_A = (y0 - y1) / det;
		m_B = (x0*y1 - x1*y0) / det;
	}
}

GLECurve::GLECurve() {
}

GLECurve::~GLECurve() {
}

double GLECurve::getT0() {
	return 0.0;
}

double GLECurve::getT1() {
	return 1.0;
}

double GLECurve::computeDistRecursive(double t1, GLEPoint& p1, double t2, GLEPoint& p2) {
	GLEPoint pm, pm1, pm2;
	/* Check for trivial case */
	if (t1 == t2) {
		return 0.0;
	}
	/* Rough approximation */
	double tm = (t1 + t2) / 2.0;
	getC(tm, pm);
	double dist1 = p1.distance(pm) + p2.distance(pm);
	/* Finer approximation */
	getC((t1+tm)/2.0, pm1);
	getC((t2+tm)/2.0, pm2);
	double dist2 = p1.distance(pm1) + pm1.distance(pm) + pm2.distance(pm) + p2.distance(pm2);
	if (fabs(dist1-dist2)/(t2-t1) < 1e-9) {
		return dist2;
	} else {
		return computeDistRecursive(t1, p1, tm, pm) +
		       computeDistRecursive(tm, pm, t2, p2);
	}
}

double GLECurve::getDist(double t1, double t2) {
	GLEPoint p1, p2;
	getC(t1, p1);
	getC(t2, p2);
	if (t2 < t1) {
		return -1.0*computeDistRecursive(t2, p2, t1, p1);
	} else {
		return computeDistRecursive(t1, p1, t2, p2);
	}
}

double GLECurve::distToParamValue(double t1, double dist) {
	GLEPoint T;
	getCp(t1, T);
	double dist_corr = dist * 1.05; /* add 5% */
	double t = t1 + dist_corr / T.norm();
	return distToParamValue(t1, dist, t);
}

class GLECurveDistToParamValue {
protected:
	GLECurve* m_Curve;
	double m_Dist, m_CrDist, m_BestErr, m_BestParam;
public:
	GLECurveDistToParamValue(GLECurve* curve);
	~GLECurveDistToParamValue();
	void update(double dist, double param);
	void distToParamValueRecursive(double t1, GLEPoint& p1, double t2, GLEPoint& p2);
	double distToParamValue(double t1, double dist, double t2);
};

GLECurveDistToParamValue::GLECurveDistToParamValue(GLECurve* curve) {
	m_Curve = curve;
}

GLECurveDistToParamValue::~GLECurveDistToParamValue() {
}

void GLECurveDistToParamValue::update(double dist, double param) {
	m_CrDist += dist;
	double err = fabs(m_Dist - m_CrDist);
	if (err < m_BestErr) {
		m_BestErr = err;
		m_BestParam = param;
	}
}

void GLECurveDistToParamValue::distToParamValueRecursive(double t1, GLEPoint& p1, double t2, GLEPoint& p2) {
	GLEPoint pm, pm1, pm2;
	/* Check for trivial case */
	if (t1 == t2) {
		return;
	}
	/* Rough approximation */
	double tm = (t1 + t2) / 2.0;
	m_Curve->getC(tm, pm);
	double dist1 = p1.distance(pm) + p2.distance(pm);
	/* Finer approximation */
	m_Curve->getC((t1+tm)/2.0, pm1);
	m_Curve->getC((t2+tm)/2.0, pm2);
	double dist2 = p1.distance(pm1) + pm1.distance(pm) + pm2.distance(pm) + p2.distance(pm2);
	if (fabs(dist1-dist2)/(t2-t1) < 1e-9) {
		update(p1.distance(pm1), (t1+tm)/2.0);
		update(pm1.distance(pm), tm);
		update(pm2.distance(pm), (t2+tm)/2.0);
		update(p2.distance(pm2), t2);
	} else {
		distToParamValueRecursive(t1, p1, tm, pm);
		distToParamValueRecursive(tm, pm, t2, p2);
	}
}

double GLECurveDistToParamValue::distToParamValue(double t1, double dist, double t2) {
	GLEPoint p1, p2;
	m_Curve->getC(t1, p1);
	m_Curve->getC(t2, p2);
	m_Dist = dist;
	m_BestErr = 1e16;
	m_BestParam = t2;
	m_CrDist = 0.0;
	distToParamValueRecursive(t1, p1, t2, p2);
	return m_BestParam;
}

double GLECurve::distToParamValue(double t1, double dist, double t2) {
	GLEPoint T, p0, p;
	getC(t1, p0);
	double dist_abs = fabs(dist);
	double dist_corr = dist_abs * 1.05; /* add 5% */
	/* Newton-Raphson on line distance between c(t1) and c(t) */
	/* Make this distance 5% longer than desired, just to be sure */
	double froot = 0.0;
	do {
		getC(t2, p);
		getCp(t2, T);
		p.add(-1.0, p0);
		froot = p.norm();
		double ft = froot - dist_corr;
		double fpt = 1/froot * (p.getX()*T.getX() + p.getY()*T.getY());
		t2 = t2 - ft/fpt;
	} while (fabs(froot - dist_corr)/dist_corr > 1e-4);
	/* Line distance should be upper bound on true distance */
	/* Now do iterative refinement to obtain true distance */
	GLECurveDistToParamValue dtp(this);
	double res = dtp.distToParamValue(t1, dist_abs, t2);
	return res;
}

double GLECurve::getDistp(double t) {
	GLEPoint T;
	getCp(t, T);
	return T.norm();
}

GLECurveT0T1::GLECurveT0T1() {
}

GLECurveT0T1::~GLECurveT0T1() {
}

double GLECurveT0T1::getT0() {
	return m_T0;
}

double GLECurveT0T1::getT1() {
	return m_T1;
}

GLECircleArc::GLECircleArc(const GLEPoint& orig, double r, double t0, double t1) {
	m_Orig.set(orig);
	m_T0 = t0; m_T1 = t1; m_R = r;
}

GLECircleArc::~GLECircleArc() {
}

void GLECircleArc::getC(double t, GLEPoint& p) {
	p.setX(m_Orig.getX() + m_R * cos(t));
	p.setY(m_Orig.getY() + m_R * sin(t));
}

void GLECircleArc::getCp(double t, GLEPoint& p) {
	p.setX(-sin(t) * m_R);
	p.setY(cos(t) * m_R);
}

void GLECircleArc::getCpp(double t, GLEPoint& p) {
	p.setX(-cos(t) * m_R);
	p.setY(-sin(t) * m_R);
}


double GLECircleArc::getDist(double t1, double t2) {
	return m_R * (t2 - t1);
}

double GLECircleArc::distToParamValue(double t1, double dist, double t2) {
	return distToParamValue(t1, dist);
}

double GLECircleArc::distToParamValue(double t1, double dist) {
	return t1 + dist/m_R;
}

GLEEllipseArc::GLEEllipseArc(const GLEPoint& orig, double rx, double ry, double t0, double t1) {
	m_Orig.set(orig);
	m_T0 = t0; m_T1 = t1;
	m_Rx = rx; m_Ry = ry;
}

GLEEllipseArc::~GLEEllipseArc() {
}

void GLEEllipseArc::getC(double t, GLEPoint& p) {
	p.setX(m_Orig.getX() + m_Rx * cos(t));
	p.setY(m_Orig.getY() + m_Ry * sin(t));
}

void GLEEllipseArc::getCp(double t, GLEPoint& p) {
	p.setX(-sin(t) * m_Rx);
	p.setY(cos(t) * m_Ry);
}

void GLEEllipseArc::getCpp(double t, GLEPoint& p) {
	p.setX(-cos(t) * m_Rx);
	p.setY(-sin(t) * m_Ry);
}

/*
    x(t) = axt^3 + bxt^2 + cxt + x0

        x1 = x0 + cx / 3
        x2 = x1 + (cx + bx) / 3
        x3 = x0 + cx + bx + ax

    y(t) = ayt^3 + byt^2 + cyt + y0

        y1 = y0 + cy / 3
        y2 = y1 + (cy + by) / 3
        y3 = y0 + cy + by + ay

        cx = 3 (x1 - x0)
        bx = 3 (x2 - x1) - cx
        ax = x3 - x0 - cx - bx

        cy = 3 (y1 - y0)
        by = 3 (y2 - y1) - cy
        ay = y3 - y0 - cy - by
*/

GLEBezier::GLEBezier() {
}

GLEBezier::GLEBezier(double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3) : m_P0(x0, y0), m_P1(x1, y1), m_P2(x2, y2), m_P3(x3, y3) {
	updateEquation();
}

GLEBezier::GLEBezier(const GLEBezier& other) : m_P0(other.getP0()), m_P1(other.getP1()), m_P2(other.getP2()), m_P3(other.getP3()) {
	updateEquation();
}

GLEBezier::~GLEBezier() {
}

void GLEBezier::updateEquation() {
	m_Cx = 3*(m_P1.getX() - m_P0.getX());
	m_Bx = 3*(m_P2.getX() - m_P1.getX()) - m_Cx;
	m_Ax = m_P3.getX() - m_P0.getX() - m_Cx - m_Bx;
	m_Cy = 3*(m_P1.getY() - m_P0.getY());
	m_By = 3*(m_P2.getY() - m_P1.getY()) - m_Cy;
	m_Ay = m_P3.getY() - m_P0.getY() - m_Cy - m_By;
}

void GLEBezier::getC(double t, GLEPoint& p) {
	p.setX(((m_Ax*t + m_Bx)*t + m_Cx)*t + m_P0.getX());
	p.setY(((m_Ay*t + m_By)*t + m_Cy)*t + m_P0.getY());
}

void GLEBezier::getCp(double t, GLEPoint& p) {
	p.setX((3.0*m_Ax*t + 2.0*m_Bx)*t + m_Cx);
	p.setY((3.0*m_Ay*t + 2.0*m_By)*t + m_Cy);
}

void GLEBezier::getCpp(double t, GLEPoint& p) {
	p.setX(6.0*m_Ax*t + 2.0*m_Bx);
	p.setY(6.0*m_Ay*t + 2.0*m_By);
}

void GLEBezier::draw() {
	g_set_pos(getP0());
	g_bezier(getP1(), getP2(), getP3());
}

void GLEBezier::cutAtParamValue(double t) {
	double ot = 1.0 - t;
	GLEPoint p_1_1(m_P2);
	p_1_1.dotScalar(t);
	p_1_1.addScalar(m_P1, ot);
	GLEPoint p_2_1(m_P3);
	p_2_1.dotScalar(t);
	p_2_1.addScalar(m_P2, ot);
	GLEPoint p_1_2(p_2_1);
	p_1_2.dotScalar(t);
	p_1_2.addScalar(p_1_1, ot);
	m_P1.dotScalar(t);
	m_P1.addScalar(m_P0, ot);
	m_P2.set(m_P1);
	m_P2.dotScalar(ot);
	m_P2.addScalar(p_1_1, t);
	m_P3.set(m_P2);
	m_P3.dotScalar(ot);
	m_P3.addScalar(p_1_2, t);
	updateEquation();
}

void GLEBezier::cutFromParamValue(double t) {
	double ot = 1.0 - t;
	GLEPoint p_1_1(m_P2);
	p_1_1.dotScalar(t);
	p_1_1.addScalar(m_P1, ot);
	GLEPoint p_0_1(m_P1);
	p_0_1.dotScalar(t);
	p_0_1.addScalar(m_P0, ot);
	GLEPoint p_0_2(p_1_1);
	p_0_2.dotScalar(t);
	p_0_2.addScalar(p_0_1, ot);
	m_P2.dotScalar(ot);
	m_P2.addScalar(m_P3, t);
	m_P1.set(m_P2);
	m_P1.dotScalar(t);
	m_P1.addScalar(p_1_1, ot);
	m_P0.set(m_P1);
	m_P0.dotScalar(t);
	m_P0.addScalar(p_0_2, ot);
	updateEquation();
}

void GLEBezier::throughPoint(GLEPoint& p, GLEPoint& dir1, GLEPoint& dir2) {
	double poly[6];
	double a = p.getX() - m_P0.getX();
	double b = 3*(m_P0.getX()-m_P3.getX());
	double c = 2*(m_P3.getX()-m_P0.getX());
	double d = dir1.getX();
	double e = dir2.getX() - 2*dir1.getX();
	double f = dir1.getX() - dir2.getX();
	double g = p.getY() - m_P0.getY();
	double h = 3*(m_P0.getY()-m_P3.getY());
	double i = 2*(m_P3.getY()-m_P0.getY());
	double j = dir1.getY();
	double k = dir2.getY() - 2*dir1.getY();
	double l = dir1.getY() - dir2.getY();
	poly[5] = c*l - i*f;
	poly[4] = c*k + b*l - i*e - h*f;
	poly[3] = c*j + b*k - i*d - h*e;
	poly[2] = b*j + a*l - h*d - g*f;
	poly[1] = a*k - g*e;
	poly[0] = a*j - g*d;
	GLEPolynomial polynomial(poly, 5);
	polynomial.horner(1);
	// polynomial.print();
	double t = polynomial.newtonRaphson(0.5);
	double scale = (a + b*t*t + c*t*t*t)/(d + e*t + f*t*t)/(3*t);
	m_P1.set(m_P0);
	m_P1.add(scale, dir1);
	m_P2.set(m_P3);
	m_P2.add(scale, dir2);
}

GLECurvedArrowHead::GLECurvedArrowHead(GLECurve* curve) {
	m_Curve = curve;
	m_T0 = m_TM = m_T1 = m_ArrAlpha = m_ArrSize = m_LWidth = 0.0;
	m_Sharp = m_Enable = false;
	m_ArrStyle = GLE_ARRSTY_FILLED;
}

GLECurvedArrowHead::~GLECurvedArrowHead() {
}

void GLECurvedArrowHead::setArrowAngleSize(int style, double size, double angle) {
	m_ArrStyle = style;
	m_ArrSize = size;
	m_ArrAlpha = angle*GLE_PI/180.0;
}

void GLECurvedArrowHead::setArrowAngleSizeSharp(int style, double size, double angle) {
	m_ArrStyle = style;
	m_ArrAlpha = angle*GLE_PI/180.0;
	if (style != GLE_ARRSTY_FILLED) setSharp(false);
	if (isSharp() && style == GLE_ARRSTY_FILLED) {
		m_ArrSize = size + m_LWidth*(1+1/sin(m_ArrAlpha))/2;
	} else {
		m_ArrSize = size;
	}
	if (style == GLE_ARRSTY_OLD35) {
		setSharp(true);
		m_ArrStyle = GLE_ARRSTY_FILLED;
	}
}

void GLECurvedArrowHead::getA(double t, double pm, GLEPoint& p) {
	GLEPoint N, T;
	// get curve tangent
	m_Curve->getCp(t, T);
	T.normalize();
	// compute curve normal
	N.setXY(T.getY(), -T.getX());
	// multiply by height
	double h = pm * m_Curve->getDist(m_T0, t) * tan(m_ArrAlpha);
	N.dotScalar(h);
	// add to point on curve
	m_Curve->getC(t, p);
	p.add(N);
}

void GLECurvedArrowHead::getAp(double t, double pm1, double pm2, GLEPoint& p) {
	GLEPoint T, Tp, N, Np, c;
	m_Curve->getC(t, c);
	m_Curve->getCp(t, T);
	m_Curve->getCpp(t, Tp);
	/* the normal vector */
	N.setXY(T.getY(), -T.getX());
	N.normalize();
	/* derivative of the normal vector */
	double norm_sq = T.normSq();
	double norm = sqrt(norm_sq);
	double dnorm = (T.getX()*Tp.getX() + T.getY()*Tp.getY())/norm;
	Np.setX((Tp.getY()*norm - T.getY()*dnorm)/norm_sq);
	Np.setY((T.getX()*dnorm - Tp.getX()*norm)/norm_sq);
	/* result = T +- (h'*N + h*N') */
	double h = m_Curve->getDist(m_T0, t) * tan(m_ArrAlpha);
	double hp = m_Curve->getDistp(t) * tan(m_ArrAlpha);
	N.dotScalar(hp);
	Np.dotScalar(h);
	N.add(Np);
	N.dotScalar(pm1);
	/* compute result in p */
	p.set(T);
	p.add(N);
	p.normalize();
	p.dotScalar(pm2);
}

void GLECurvedArrowHead::computeArrowHead() {
	// compute P0 and P3 for both sides
	GLEPoint r1, r2, p;
	getA(m_T1, +1, m_Side1.P0());
	m_Curve->getC(m_T0, m_Side1.P3());
	getA(m_TM, +1, p);
	getAp(m_T1, +1, -1, r1);
	getAp(m_T0, +1, +1, r2);
	m_Side1.throughPoint(p, r1, r2);
	m_Curve->getC(m_T0, m_Side2.P0());
	getA(m_T1, -1, m_Side2.P3());
	getA(m_TM, -1, p);
	getAp(m_T0, -1, +1, r1);
	getAp(m_T1, -1, -1, r2);
	m_Side2.throughPoint(p, r1, r2);
}

void GLECurvedArrowHead::draw() {
	char old_lstyle[15];
	int old_join;
	double x, y;
	g_get_xy(&x, &y);
	g_get_line_style(old_lstyle);
	if (!(old_lstyle[0] == '1' && old_lstyle[1] == 0)) {
		g_set_line_style("1");
	}
	g_get_line_join(&old_join);
	if (old_join != 1) {
		g_set_line_join(1);
	}
	g_set_path(true);
	g_newpath();
	m_Side1.draw();
	m_Side2.draw();
	if (getStyle() != GLE_ARRSTY_SIMPLE) {
		g_closepath();
		GLERC<GLEColor> cur_color(g_get_color());
		GLERC<GLEColor> cur_full(g_get_fill());
		if (getStyle() == GLE_ARRSTY_EMPTY) g_set_fill(GLE_COLOR_WHITE);
		else g_set_fill(cur_color);
		g_fill();
		g_set_fill(cur_full);
	}
	if (!isSharp()) {
		g_stroke();
	}
	g_set_path(false);
	g_move(x, y);
	if (old_join != 1) {
		g_set_line_join(old_join);
	}
	if (!(old_lstyle[0] == '1' && old_lstyle[1] == 0)) {
		g_set_line_style(old_lstyle);
	}
}

double GLECurvedArrowHead::getArrowCurveDist() {
	return m_ArrSize;
}

void GLECurvedArrowHead::setStartEnd(bool dir) {
	setEnabled(true);
	double dist = getArrowCurveDist();
	if (dir) {
		m_T0 = m_Curve->getT0();
		m_T1 = m_Curve->distToParamValue(m_T0, dist);
		m_TM = m_Curve->distToParamValue(m_T0, dist/2.0, (m_T0 + m_T1)/2.0);
	} else {
		m_T0 = m_Curve->getT1();
		m_T1 = m_Curve->distToParamValue(m_T0, -dist);
		m_TM = m_Curve->distToParamValue(m_T0, -dist/2.0, (m_T0 + m_T1)/2.0);
	}
}

double GLECurvedArrowHead::getParamValueEnd() {
	return (m_T1 + m_TM) / 2;
}

void GLECurvedArrowHead::drawDirection(bool dir) {
	setStartEnd(dir);
	computeArrowHead();
	draw();
}

void GLECurvedArrowHead::computeAndDraw() {
	if (isEnabled()) {
		computeArrowHead();
		draw();
	}
}

int GLEBBoxToPixels(double dpi, double bbox) {
	// this has been tested with bbox.gle in gle-testsuite/plain
	return (int)floor((double)dpi/PS_POINTS_PER_INCH*bbox+1);
}

GLESaveRestore::GLESaveRestore() {
	model = NULL;
}

GLESaveRestore::~GLESaveRestore() {
	if (model != NULL) {
		delete model;
	}
}

void GLESaveRestore::save() {
	if (model == NULL) model = new gmodel();
	g_get_state(model);
}

void GLESaveRestore::restore() {
	g_set_state(model);
}

int g_font_fallback(int font) {
	GLECoreFont* coreFont = get_core_font_ensure_loaded(font);
	if (coreFont->info.encoding <= 2) {
		CmdLineObj* cmdLine = GLEGetInterfacePointer()->getCmdLine();
		if (cmdLine->hasOption(GLE_OPT_CAIRO)) {
			GLECore* core = g_get_core();
			if (core->isShowNoteAboutFallback()) {
				core->setShowNoteAboutFallback(false);
				g_message(">> PostScript fonts not supported with '-cairo'; using 'texcmr' instead");
			}
			return 17;
		}
	}
	return font;
}

std::vector<std::string> g_create_device_string() {
	CmdLineObj* cmdLine = GLEGetInterfacePointer()->getCmdLine();
	CmdLineArgSet* device = (CmdLineArgSet*)cmdLine->getOption(GLE_OPT_DEVICE)->getArg(0);
	std::vector<std::string> result(device->getValues());
	if (cmdLine->hasOption(GLE_OPT_LANDSCAPE)) {
		result.push_back("LANDSCAPE");
	}
	if (cmdLine->hasOption(GLE_OPT_FULL_PAGE)) {
		result.push_back("FULLPAGE");
	}
	if (cmdLine->hasOption(GLE_OPT_TEX) || cmdLine->hasOption(GLE_OPT_CREATE_INC)) {
		result.push_back("TEX");
	}
	if (cmdLine->hasOption(GLE_OPT_NO_COLOR)) {
		result.push_back("GRAYSCALE");
	}
	if (cmdLine->hasOption(GLE_OPT_TRANSPARENT)) {
		result.push_back("TRANSPARENT");
	}
	if (cmdLine->hasOption(GLE_OPT_NO_LIGATURES)) {
		result.push_back("NOLIGATURES");
	}
	if (cmdLine->hasOption(GLE_OPT_SAFEMODE)) {
		result.push_back("SAFE");
	}
	return strs_to_uppercase(result);
}
