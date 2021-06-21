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
#include "core.h"
#include "file_io.h"
#include "texinterface.h"
#include "cutils.h"
#include "gprint.h"
#include "bitmap/img2ps.h"

#ifdef HAVE_CAIRO

#include <cairo-ps.h>
#include <cairo-pdf.h>
#include <cairo-svg.h>

/************************************************************************************
 * Fonts in Cairo:
 *
 ************************************************************************************/

extern struct gmodel g;

char *font_getname(int i);

GLECairoDevice::GLECairoDevice(bool showerror):
	m_ShowError(showerror),
	m_width(0.0),
	m_height(0.0),
	m_surface(0),
	m_cr(0),
	m_FillMethod(GLE_FILL_METHOD_DEFAULT),
	m_currentFill(g_get_fill_clear())
{
}

GLECairoDevice::~GLECairoDevice() {
	for (unsigned int i = 0; i < m_surfacesToDelete.size(); ++i) {
		cairo_surface_destroy(m_surfacesToDelete[i]);
	}
}

void GLECairoDevice::arc(dbl r,dbl t1,dbl t2,dbl cx,dbl cy) {
	//double dx,dy;
	double x,y;
	g_get_xy(&x,&y);
	//polar_xy(r,t1,&dx,&dy);
	if (!g.inpath) {
		if (!g.xinline) cairo_new_path(m_cr);
	}
	cairo_arc(m_cr, cx, cy, r, t1*GLE_PI/180.0, t2*GLE_PI/180.0);
	g.xinline = true;
	if (!g.inpath) g_move(x,y);
}

void GLECairoDevice::arcto(dbl x1,dbl y1,dbl x2,dbl y2,dbl rrr) {
	if (g.xinline==false) move(g.curx,g.cury);
	cairo_curve_to(m_cr, x1, y1, x2, y2, x2, y2);
	g.xinline = true;
}

void GLECairoDevice::beginclip(void)  {
	cairo_save(m_cr);
}

void GLECairoDevice::bezier(dbl x1,dbl y1,dbl x2,dbl y2,dbl x3,dbl y3) {
	double x=g.curx,y=g.cury;
	if (g.inpath) {
		if (g.xinline==false) move(g.curx,g.cury);
		cairo_curve_to(m_cr, x1, y1, x2, y2, x3, y3);
	} else {
		g_flush();
		if (!g.xinline) cairo_move_to(m_cr, x, y);
		cairo_curve_to(m_cr, x1, y1, x2, y2, x3, y3);
	}
	g.xinline = true;
}

void GLECairoDevice::box_fill(dbl x1, dbl y1, dbl x2, dbl y2) {
	if (g.inpath) {
		xdbox(x1,y1,x2,y2);
	} else {
		g_flush();
		cairo_new_path(m_cr);
		GLERectangle rect(x1, y1, x2, y2);
		xdbox(x1,y1,x2,y2);
		ddfill(&rect);
		cairo_new_path(m_cr);
	}
}

void GLECairoDevice::box_stroke(dbl x1, dbl y1, dbl x2, dbl y2, bool reverse) {
	if (g.inpath) {
		if (reverse) {
			cairo_move_to(m_cr, x1, y1);
			cairo_line_to(m_cr, x1, y2);
			cairo_line_to(m_cr, x2, y2);
			cairo_line_to(m_cr, x2, y1);
			cairo_close_path(m_cr);
		} else {
			xdbox(x1,y1,x2,y2);
		}
	} else {
		g_flush();
		cairo_new_path(m_cr);
		xdbox(x1,y1,x2,y2);
		cairo_stroke(m_cr);
		//ps_nvec = 0;
	}
}

void GLECairoDevice::dochar(int font, int cc) {
	if (font_get_encoding(font) > 2) {
		my_char(font, cc);
	} else {
		g_throw_parser_error("PostScript fonts not supported with '-cairo'");
	}
}

void GLECairoDevice::resetfont() {
}

void GLECairoDevice::circle_fill(double zr) {
	double x=g.curx,y=g.cury;
	if (g.inpath) {
		cairo_arc(m_cr, x, y, zr, 0, 2*GLE_PI);
	} else {
		g_flush();
		cairo_new_path(m_cr);
		cairo_arc(m_cr, x, y, zr, 0, 2*GLE_PI);
		GLERectangle rect(x-zr, y-zr, x+zr, y+zr);
		ddfill(&rect);
		cairo_new_path(m_cr);
	}
}

void GLECairoDevice::circle_stroke(double zr) {
	double x,y;
	g_get_xy(&x,&y);
	if (g.inpath) {
		cairo_arc(m_cr, x, y, zr, 0, 2*GLE_PI);
	} else {
		g_flush();
		cairo_new_path(m_cr);
		cairo_arc(m_cr, x, y, zr, 0, 2*GLE_PI);
		cairo_close_path(m_cr);
		cairo_stroke(m_cr);
	}
}

void GLECairoDevice::clear(void) {
}

void GLECairoDevice::clip(void) {
	cairo_clip(m_cr);
}

void GLECairoDevice::closedev(void) {
    cairo_destroy(m_cr);
    cairo_surface_destroy(m_surface);
	if (g_verbosity() > 0) {
		string mainname;
		string extension(g_device_to_ext(getDeviceType()));
		GetMainNameExt(m_OutputName.getName(), extension.c_str(), mainname);
		cerr << "[" << mainname << "][" << extension << "]";
		g_set_console_output(false);
	}
}

void GLECairoDevice::closepath(void) {
	cairo_close_path(m_cr);
}

void GLECairoDevice::dfont(char *c) {
}

void GLECairoDevice::ellipse_fill(double rx, double ry) {
	double x=g.curx,y=g.cury;
	if (g.inpath) {
		cairo_save (m_cr);
		cairo_translate (m_cr, x, y);
		cairo_scale (m_cr, rx, ry);
		cairo_arc (m_cr, 0, 0, 1, 0, 2*GLE_PI);
		cairo_restore (m_cr);
	} else {
		g_flush();
		cairo_new_path(m_cr);
		cairo_save (m_cr);
		cairo_translate (m_cr, x, y);
		cairo_scale (m_cr, rx, ry);
		cairo_arc (m_cr, 0, 0, 1, 0, 2*GLE_PI);
		cairo_restore (m_cr);
		GLERectangle rect(x-rx, y-ry, x+rx, y+ry);
		ddfill(&rect);
		cairo_new_path(m_cr);
	}
}

void GLECairoDevice::ellipse_stroke(double rx, double ry) {
	double x,y;
	g_get_xy(&x,&y);
	if (!g.inpath) {
		if (!g.xinline) cairo_new_path(m_cr);
	}
	cairo_save (m_cr);
	cairo_translate (m_cr, x, y);
	cairo_scale (m_cr, rx, ry);
	cairo_arc (m_cr, 0, 0, 1, 0, 2*GLE_PI);
	cairo_restore (m_cr);
	g.xinline = true;
	if (!g.inpath) g_move(x,y);
}

void GLECairoDevice::elliptical_arc(double rx,double ry,double t1,double t2,double cx,double cy) {
	double x,y;
	g_get_xy(&x,&y);
	if (!g.inpath) {
		if (!g.xinline) cairo_new_path(m_cr);
	}
	cairo_save (m_cr);
	cairo_translate (m_cr, cx, cy);
	cairo_scale (m_cr, rx, ry);
	cairo_arc (m_cr, 0, 0, 1, t1*GLE_PI/180.0, t2*GLE_PI/180.0);
	cairo_restore (m_cr);
	g.xinline = true;
	if (!g.inpath) g_move(x,y);
}

void GLECairoDevice::elliptical_narc(double rx,double ry,double t1,double t2,double cx,double cy) {
	double x,y;
	g_get_xy(&x,&y);
	if (!g.inpath) {
		if (!g.xinline) cairo_new_path(m_cr);
	}
	cairo_save (m_cr);
	cairo_translate (m_cr, cx, cy);
	cairo_scale (m_cr, rx, ry);
	cairo_arc_negative (m_cr, 0, 0, 1, t1*GLE_PI/180.0, t2*GLE_PI/180.0);
	cairo_restore (m_cr);
	g.xinline = true;
	if (!g.inpath) g_move(x,y);
}

void GLECairoDevice::endclip(void)  {
	g_flush();
	cairo_restore(m_cr);
	gmodel* state = new gmodel();
	g_get_state(state);
	g_set_state(state);
	delete state;
}

void GLECairoDevice::fill(void) {
	//cairo_save(cr);
	ddfill();
	//cairo_restore(cr);
}

void GLECairoDevice::ddfill(GLERectangle* bounds) {
	colortyp cur_fill;
	cur_fill.l = m_currentFill->getHexValueGLE();
	if (cur_fill.b[B_F] == 255) {
		return; /* clear fill, do nothing */
	} else if (cur_fill.b[B_F] == 2) {
		shade(bounds);
		return;
	}
	set_fill();			/*because color and fill are the same*/
	//cairo_fill(cr);
	cairo_fill_preserve(m_cr);
	set_color();
}

void GLECairoDevice::shadePattern(void) {
	colortyp cur_fill;
	cur_fill.l = m_currentFill->getHexValueGLE();
	int step1 = cur_fill.b[B_B];
	int step2 = cur_fill.b[B_G];
	int xstep = max(step1, step2);
	int ystep = max(step1, step2);
	cairo_save(m_cr);
	cairo_matrix_t matrix;
	cairo_get_matrix(m_cr, &matrix);
	cairo_surface_t *isurface = cairo_surface_create_similar(m_surface, CAIRO_CONTENT_COLOR_ALPHA, xstep, ystep);
	cairo_t *icr = cairo_create(isurface);
	GLERC<GLEColor> background(get_fill_background(m_currentFill.get()));
	if (!background->isTransparent()) {
		if (background->getHexValueGLE() == (unsigned int)GLE_COLOR_WHITE) {
			cairo_set_source_rgb(icr, 1.0, 1.0, 1.0);
		} else {
			cairo_set_source_rgb(icr, background->getRed(), background->getGreen(), background->getBlue());
		}
		cairo_rectangle(icr, -1, -1, (xstep+1), (ystep+1));
		cairo_fill(icr);
	}
	GLERC<GLEColor> foreground(get_fill_foreground(m_currentFill.get()));
	if (foreground->getHexValueGLE() == GLE_COLOR_BLACK) {
		cairo_set_source_rgb(icr, 0, 0, 0);
	} else {
		cairo_set_source_rgb(icr, foreground->getRed(), foreground->getGreen(), foreground->getBlue());
	}
	cairo_set_line_width(icr, cur_fill.b[B_R]);
	if (step1 > 0) {
		cairo_move_to(icr, 0, 0);
		cairo_line_to(icr, xstep, ystep);
		cairo_stroke(icr);
		if (step2 == 0) {
			cairo_move_to(icr, xstep/2, -ystep/2);
			cairo_line_to(icr, 3*xstep/2, ystep/2);
			cairo_stroke(icr);
			cairo_move_to(icr, -xstep/2, ystep/2);
			cairo_line_to(icr, xstep/2, 3*ystep/2);
			cairo_stroke(icr);
		}
	}
	if (step2 > 0) {
		cairo_move_to(icr, 0, ystep);
		cairo_line_to(icr, xstep, 0);
		cairo_stroke(icr);
		if (step1 == 0) {
			cairo_move_to(icr, -xstep/2, ystep/2);
			cairo_line_to(icr, xstep/2, -ystep/2);
			cairo_stroke(icr);
			cairo_move_to(icr, xstep/2, 3*ystep/2);
			cairo_line_to(icr, 3*xstep/2, ystep/2);
			cairo_stroke(icr);
		}
	}
	cairo_pattern_t *pattern = cairo_pattern_create_for_surface(isurface);
	cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REPEAT);
	cairo_matrix_init_scale(&matrix, 160.0, 160.0);
	cairo_pattern_set_matrix(pattern, &matrix);
	cairo_set_source(m_cr, pattern);
	cairo_fill(m_cr);
	cairo_restore(m_cr);
	cairo_pattern_destroy(pattern);
	cairo_destroy(icr);
	cairo_surface_destroy(isurface);
}

void GLECairoDevice::shadeGLE() {
	colortyp cur_fill;
	cur_fill.l = m_currentFill->getHexValueGLE();
	double step1 = cur_fill.b[B_B]/160.0;
	double step2 = cur_fill.b[B_G]/160.0;
	if (step1 > 0) {
		for (double x = -40.0; x < 40.0; x += step1) {
			cairo_move_to(m_cr, x, 0.0);
			cairo_line_to(m_cr, 40.0+x, 40);
			cairo_stroke(m_cr);
		}
	}
	if (step2 > 0) {
		for (double x = 0.0; x < 80; x += step2) {
			cairo_move_to(m_cr, x, 0.0);
			cairo_line_to(m_cr, x-40.0, 40);
			cairo_stroke(m_cr);
		}
	}
}

void GLECairoDevice::shadeBoundedIfThenElse1(GLERectangle* bounds, double p, double step1) {
	// if x1+p*s > y1 then
	if (bounds->getXMax()+p*step1 > bounds->getYMax()) {
		// aline y1-p*s y1
		cairo_line_to(m_cr, bounds->getYMax()-p*step1, bounds->getYMax());
	} else {
		// aline x1 x1+p*s
		cairo_line_to(m_cr, bounds->getXMax(), bounds->getXMax()+p*step1);
	}
	cairo_stroke(m_cr);
}

void GLECairoDevice::shadeBoundedIfThenElse2(GLERectangle* bounds, double p, double step2) {
	// if p*s-y1 > x0 then
	if (p*step2-bounds->getYMax() > bounds->getXMin()) {
		// aline p*s-y1 y1
		cairo_line_to(m_cr, p*step2-bounds->getYMax(), bounds->getYMax());
	} else {
		// aline x0 p*s-x0
		cairo_line_to(m_cr, bounds->getXMin(), p*step2-bounds->getXMin());
	}
	cairo_stroke(m_cr);
}

void GLECairoDevice::shadeBounded(GLERectangle* bounds) {
	colortyp cur_fill;
	cur_fill.l = m_currentFill->getHexValueGLE();
	double step1 = cur_fill.b[B_B]/160.0;
	double step2 = cur_fill.b[B_G]/160.0;
	cairo_set_line_cap(m_cr, CAIRO_LINE_CAP_SQUARE);
	if (step1 > 0) {
		int p0 = (int)ceil((bounds->getYMax()-bounds->getXMin())/step1-1e-6);
		if (bounds->getXMin() + p0*step1 > bounds->getYMax()) p0--;
		int p1 = (int)floor((bounds->getYMin()-bounds->getXMin())/step1+1e-6);
		if (bounds->getXMin() + p1*step1 < bounds->getYMin()) p1++;
		int p2 = (int)floor((bounds->getYMin()-bounds->getXMax())/step1+1e-6);
		if (bounds->getXMax() + p2*step1 < bounds->getYMin()) p2++;
		for (int p = p0; p > p1; p--) {
			// amove x0 x0+p*s
			cairo_move_to(m_cr, bounds->getXMin(), bounds->getXMin()+p*step1);
			shadeBoundedIfThenElse1(bounds, p, step1);
		}
		for (int p = p1; p >= p2; p--) {
			// amove y0-p*s y0
			cairo_move_to(m_cr, bounds->getYMin()-p*step1, bounds->getYMin());
			shadeBoundedIfThenElse1(bounds, p, step1);
		}
	}
	if (step2 > 0) {
		int p0 = (int)ceil((bounds->getYMax()+bounds->getXMax())/step2-1e-6);
		if (p0*step2 - bounds->getXMin() > bounds->getYMax()) p0--;
		int p1 = (int)floor((bounds->getYMin()+bounds->getXMax())/step2+1e-6);
		if (p1*step2 - bounds->getXMax() < bounds->getYMin()) p1++;
		int p2 = (int)floor((bounds->getYMin()+bounds->getXMin())/step2+1e-6);
		if (p2*step2 - bounds->getXMax() < bounds->getYMin()) p2++;
		for (int p = p0; p > p1; p--) {
			// amove x0 x0+p*s
			cairo_move_to(m_cr, bounds->getXMax(), p*step2-bounds->getXMax());
			shadeBoundedIfThenElse2(bounds, p, step2);
		}
		for (int p = p1; p >= p2; p--) {
			// amove y0-p*s y0
			cairo_move_to(m_cr, p*step2-bounds->getYMin(), bounds->getYMin());
			shadeBoundedIfThenElse2(bounds, p, step2);
		}
	}
}

void GLECairoDevice::shade(GLERectangle* bounds) {
	if (m_FillMethod == GLE_FILL_METHOD_GLE ||
	   (m_FillMethod == GLE_FILL_METHOD_DEFAULT && bounds != NULL)) {
		cairo_save(m_cr);
		GLERC<GLEColor> background(get_fill_background(m_currentFill.get()));
		if (!background->isTransparent()) {
			cairo_set_source_rgb(m_cr, background->getRed(), background->getGreen(), background->getBlue());
			cairo_fill_preserve(m_cr);
		}
		// Implemented by using path as clip and then painting strokes over the clip
		cairo_clip(m_cr);
		cairo_new_path(m_cr);
		GLERC<GLEColor> foreground(get_fill_foreground(m_currentFill.get()));
		cairo_set_source_rgb(m_cr, foreground->getRed(), foreground->getGreen(), foreground->getBlue());
		colortyp cur_fill;
		cur_fill.l = m_currentFill->getHexValueGLE();
		cairo_set_line_width(m_cr, (double)(cur_fill.b[B_R]/160.0));
		if (m_FillMethod == GLE_FILL_METHOD_DEFAULT && bounds != NULL) {
			shadeBounded(bounds);
		} else {
			shadeGLE();
		}
		cairo_restore(m_cr);
	} else {
		shadePattern();
	}
}

void GLECairoDevice::fill_ary(int nwk,double *wkx,double *wky) {
	cout << "fill_ary not yet implemented" << endl;
}

void GLECairoDevice::flush(void) {
	if (g.inpath) return;
	if (g.xinline) {
		cairo_stroke(m_cr);
		//ps_nvec = 0;
	}
}

std::string GLECairoDevice::get_type() {
	std::vector<std::string> temp(g_create_device_string());
	temp.push_back("FILLPATH");
	temp.push_back("CAIRO");
	return str_join(temp);
}

void GLECairoDevice::line(double zx,double zy) {
	if (g.xinline==false) {
		move(g.curx,g.cury);
	}
	cairo_line_to(m_cr, zx, zy);
}

void GLECairoDevice::line_ary(int nwk,double *wkx,double *wky) {
	cout << "line_ary not yet implemented" << endl;
}

void GLECairoDevice::message(char *s) {
	printf("%s\n",s);
}

void GLECairoDevice::move(double zx,double zy) {
	if (g.inpath) {
		cairo_move_to(m_cr, zx, zy);
	} else {
		//ps_nvec++;
		cairo_new_path(m_cr);
		cairo_move_to(m_cr, zx, zy);
	}
}

void GLECairoDevice::narc(dbl r,dbl t1,dbl t2,dbl cx,dbl cy) {
	//double dx,dy;
	double x,y;
	g_get_xy(&x,&y);
	//polar_xy(r,t1,&dx,&dy);
	if (!g.inpath) {
		if (!g.xinline) cairo_new_path(m_cr);
	}
	cairo_arc_negative(m_cr, cx, cy, r, t1*GLE_PI/180.0, t2*GLE_PI/180.0);
	g.xinline = true;
	if (!g.inpath) g_move(x,y);
}

void GLECairoDevice::newpath(void) {
	cairo_new_path(m_cr);
}

void GLECairoDevice::opendev(double width, double height, GLEFileLocation* outputfile, const string& inputfile) throw(ParserError) {
}

void GLECairoDevice::pscomment(char* ss) {
	cout << "pscomment not yet implemented" << endl;
}

void GLECairoDevice::reverse(void)    /* reverse the order of stuff in the current path */ {
	cout << "reverse not yet implemented" << endl;
}

void GLECairoDevice::set_color_impl(const GLERC<GLEColor>& color) {
	if (color->hasAlpha()) {
		cairo_set_source_rgba(m_cr, color->getRed(), color->getGreen(), color->getBlue(), color->getAlpha());
	} else {
		cairo_set_source_rgb(m_cr, color->getRed(), color->getGreen(), color->getBlue());
	}
}

void GLECairoDevice::set_color() {
	set_color_impl(m_currentColor);
}

void GLECairoDevice::set_color(const GLERC<GLEColor>& color) {
	g_flush();
	m_currentColor = color;
	set_color();
}

void GLECairoDevice::set_fill(const GLERC<GLEColor>& fill) {
	m_currentFill = fill;
}

void GLECairoDevice::set_fill_method(int m) {
	m_FillMethod = m;
}

void GLECairoDevice::set_line_cap(int i) {
	/*  lcap, 0= butt, 1=round, 2=projecting square */
	if (!g.inpath) g_flush();
	cairo_set_line_cap(m_cr, (cairo_line_cap_t)i);
}

void GLECairoDevice::set_line_join(int i) {
	/* 0=miter, 1=round, 2=bevel */
	if (!g.inpath) g_flush();
	cairo_set_line_join(m_cr, (cairo_line_join_t)i);
}

void GLECairoDevice::set_line_miterlimit(double d) {
	if (!g.inpath) g_flush();
	cairo_set_miter_limit(m_cr, d);
}

void GLECairoDevice::set_line_style(const char *s) {
	/* should deal with [] for solid lines */
	static const char *defline[] = {"","","12","41","14","92","1282",
	                                "9229","4114","54","73","7337","6261","2514"};
	if (!g.inpath) g_flush();
	if (strlen(s) == 1) {
		s = defline[*s-'0'];
	}
	int nb_dashes = strlen(s);
	double *dashes = new double[nb_dashes];
	for (int i = 0; i < nb_dashes; i++) {
		dashes[i] = (s[i]-'0')*g.lstyled;
	}
	cairo_set_dash(m_cr, dashes, nb_dashes, 0);
	delete[] dashes;
}

void GLECairoDevice::set_line_styled(double dd) {
	// no need to implement: set_line_style gets this from context
	// FIXME: rather: should call set_line_style again?
}

void GLECairoDevice::set_line_width(double w) {
	if (w == 0) w = 0.02;
	if (w < .0002) w = 0;
	if (!g.inpath) g_flush();
	cairo_set_line_width(m_cr, w);
}

void GLECairoDevice::set_matrix(double newmat[3][3]) {
	cairo_matrix_t matrix;
	matrix.xx = newmat[0][0];
	matrix.xy = newmat[0][1];
	matrix.yx = - newmat[1][0];
	matrix.yy = - newmat[1][1];
	matrix.x0 = newmat[0][2];
	double offset = 0.0;
	if (!g_is_fullpage()) {
		offset = 2.0 * CM_PER_INCH / 72;
	}
	matrix.y0 = (m_height + offset) * 72 / CM_PER_INCH - newmat[1][2];
	cairo_set_matrix(m_cr, &matrix);
}

void GLECairoDevice::set_path(int onoff) {
	//cout << "set_path not yet implemented" << endl;
}

void GLECairoDevice::source(const char *s) {
	//cout << "source not yet implemented" << endl;
}

void GLECairoDevice::stroke(void) {
	cairo_stroke_preserve(m_cr);
}

void GLECairoDevice::set_fill(void) {
	set_color_impl(m_currentFill);
}

void GLECairoDevice::xdbox(double x1, double y1, double x2, double y2) {
	//cout << "xdbox" << endl;
	//cairo_rectangle_t(cr, x1, y1, x2 - x1, y2 - y1);
	cairo_move_to(m_cr, x1, y1);
	cairo_line_to(m_cr, x2, y1);
	cairo_line_to(m_cr, x2, y2);
	cairo_line_to(m_cr, x1, y2);
	cairo_close_path(m_cr);
}

void GLECairoDevice::devcmd(const char *s) {
	cout << "devcmd not yet implemented" << endl;
}

FILE* GLECairoDevice::get_file_pointer(void) {
	return NULL;
}

int GLECairoDevice::getDeviceType() {
	return GLE_DEVICE_NONE;
}

void delete_gle_recorded_byte_stream(void* todelete) {
	GLERecordedByteStream* stream = (GLERecordedByteStream*)todelete;
	delete stream;
}

class GLECairoImageByteStream: public GLEByteStream {
public:
	GLECairoImageByteStream(cairo_surface_t *image) {
		m_buffer = cairo_image_surface_get_data(image);
		m_stride = cairo_image_surface_get_stride(image);
		m_pos = 0;
		m_scanLine = 0;
	}

	virtual int sendByte(GLEBYTE byte) {
		m_buffer[m_pos++] = byte;
		return GLE_IMAGE_ERROR_NONE;
	}

	virtual int endScanLine() {
		m_scanLine++;
		m_pos = m_scanLine * m_stride;
		return GLE_IMAGE_ERROR_NONE;
	}

protected:
	unsigned char* m_buffer;
	int m_stride;
	int m_pos;
	int m_scanLine;
};

cairo_surface_t* GLECairoDevice::bitmapCreateSurface(GLEBitmap* bitmap) {
	/* Set options */
	bitmap->setCompress(0.0);
	bitmap->setASCII85(1);
	/* Convert bitmap to postscript */
	bitmap->prepare(GLE_BITMAP_PREPARE_SCANLINE);
	cairo_format_t imageFormat = CAIRO_FORMAT_RGB24;
	if (bitmap->isAlpha()) {
		imageFormat = CAIRO_FORMAT_ARGB32;
	}
	if (bitmap->isGrayScale()) {
		imageFormat = CAIRO_FORMAT_A8;
		if (bitmap->getBitsPerComponent() == 1) {
			imageFormat = CAIRO_FORMAT_A1;
		}
	}
	cairo_surface_t* image = cairo_image_surface_create(imageFormat, bitmap->getWidth(), bitmap->getHeight());
	GLECairoImageByteStream stream(image);
	GLEByteStream* toDecode = &stream;
	GLERGBATo32BitByteStream rgbTo32Bit(toDecode, bitmap->isAlpha());
	if (bitmap->isIndexed() || bitmap->getColorComponents() >= 3) {
		toDecode = &rgbTo32Bit;
	}
	int extra = bitmap->getExtraComponents();
	int color_alpha = bitmap->getColorComponents();
	if (bitmap->isAlpha()) {
		// Do not count alpha as extra component
		extra--;
		color_alpha++;
	}
	GLEComponentRemovalByteStream crem(toDecode, color_alpha, extra);
	if (extra != 0) {
		toDecode = &crem;
	}
	GLEPNegateByteStream negated(&stream);
	if (bitmap->isGrayScale()) {
		toDecode = &negated;
	}
	GLEIndexedToRGBByteStream indexedToRGB(toDecode, bitmap->getPalette());
	if (bitmap->isIndexed()) {
		toDecode = &indexedToRGB;
	}
	GLEBitsTo32BitByteStream combine(toDecode);
	if (bitmap->isGrayScale() && bitmap->getBitsPerComponent() == 1) {
		toDecode = &combine;
	}
	bitmap->decode(toDecode);
	cairo_surface_mark_dirty(image);
	if (bitmap->getEncoding() == GLE_BITMAP_JPEG) {
		GLERecordedByteStream* stream = new GLERecordedByteStream();
		bitmap->coded(stream);
		cairo_status_t status = cairo_surface_set_mime_data(image,
															CAIRO_MIME_TYPE_JPEG,
															stream->getBytes(),
															stream->getNbBytes(),
															delete_gle_recorded_byte_stream,
															(void*)stream);
		CUtilsAssert(status == CAIRO_STATUS_SUCCESS);
	}
	return image;
}

void GLECairoDevice::bitmap(GLEBitmap* bitmap, GLEPoint* pos, GLEPoint* scale, int type) {
	/* Store current box */
	GLERectangle save_box;
	g_get_bounds(&save_box);
	/* Generate header in postrscript output */
	g_gsave();
	/* Get current position	*/
	g_scale(scale->getX() / bitmap->getWidth(), scale->getY() / bitmap->getHeight());
	g_translate(pos->getX(), pos->getY());
	/* Flip image so that it is not upside down */
	cairo_matrix_t mirror, current, result;
	cairo_matrix_init(&mirror, 1, 0, 0, -1, 0, bitmap->getHeight());
	cairo_get_matrix(m_cr, &current);
	cairo_matrix_multiply(&result, &mirror, &current);
	cairo_set_matrix(m_cr, &result);
	/* Create image */
	bool inCache = false;
	cairo_surface_t* image = 0;
	const std::string& name(bitmap->getFName());
	if (name.empty()) {
		image = bitmapCreateSurface(bitmap);
	} else {
		inCache = true;
		image = (cairo_surface_t*)m_bitmapCache.try_get(name);
		if (image == 0) {
			image = bitmapCreateSurface(bitmap);
			m_bitmapCache.add_item(name, image);
			m_surfacesToDelete.push_back(image);
		}
	}
	cairo_set_source_surface(m_cr, image, 0, 0);
	cairo_paint(m_cr);
	if (!inCache) {
		cairo_surface_destroy(image);
	}
	/* Footer */
	g_grestore();
	g_set_bounds(&save_box);
	bitmap->close();
}

void GLECairoDevice::getRecordedBytes(string* output) {
	if (!m_recorded.empty()) {
		*output = std::string(&m_recorded[0], m_recorded.size());
	} else {
		output->clear();
	}
}

void GLECairoDevice::recordData(const unsigned char *data, unsigned int length) {
	m_recorded.reserve(m_recorded.size() + length);
	for (unsigned int i = 0; i < length; ++i) {
		m_recorded.push_back((char)data[i]);
	}
}

void GLECairoDevice::clearRecordedData() {
	m_recorded.clear();
}

cairo_status_t gle_cairo_device_write(void* closure, const unsigned char *data, unsigned int length) {
	GLECairoDevice* cairoDevice = (GLECairoDevice*)closure;
	cairoDevice->recordData(data, length);
	return CAIRO_STATUS_SUCCESS;
}

GLECairoDeviceSVG::GLECairoDeviceSVG(bool showerror) : GLECairoDevice(showerror) {
}

GLECairoDeviceSVG::~GLECairoDeviceSVG() {
}

void GLECairoDeviceSVG::opendev(double width, double height, GLEFileLocation* outputfile, const string& inputfile) throw(ParserError) {
	m_width = width;
	m_height = height;
	m_OutputName.copy(outputfile);
	m_OutputName.addExtension(g_device_to_ext(getDeviceType()));
	m_surface = cairo_svg_surface_create(m_OutputName.getFullPath().c_str(), 72*width/CM_PER_INCH+2, 72*height/CM_PER_INCH+2);
	cairo_surface_set_fallback_resolution(m_surface, m_resolution, m_resolution);
	m_cr = cairo_create(m_surface);
	computeBoundingBox(width, height);
	g_scale(PS_POINTS_PER_INCH/CM_PER_INCH, PS_POINTS_PER_INCH/CM_PER_INCH);
	if (!g_is_fullpage()) {
		g_translate(1.0*CM_PER_INCH/72, 1.0*CM_PER_INCH/72);
	}
}

int GLECairoDeviceSVG::getDeviceType() {
	return GLE_DEVICE_CAIRO_SVG;
}

GLECairoDevicePDF::GLECairoDevicePDF(bool showerror) : GLECairoDevice(showerror) {
}

GLECairoDevicePDF::~GLECairoDevicePDF() {
}

void GLECairoDevicePDF::opendev(double width, double height, GLEFileLocation* outputfile, const string& inputfile) throw(ParserError) {
	clearRecordedData();
	m_width = width;
	m_height = height;
	m_OutputName.copy(outputfile);
	m_OutputName.addExtension(g_device_to_ext(getDeviceType()));
	if (isRecordingEnabled()) {
		m_surface = cairo_pdf_surface_create_for_stream(gle_cairo_device_write, (void*)this, 72*width/CM_PER_INCH+2, 72*height/CM_PER_INCH+2);
	} else {
		m_surface = cairo_pdf_surface_create(m_OutputName.getFullPath().c_str(), 72*width/CM_PER_INCH+2, 72*height/CM_PER_INCH+2);
	}
	cairo_surface_set_fallback_resolution(m_surface, m_resolution, m_resolution);
	m_cr = cairo_create(m_surface);
	computeBoundingBox(width, height);
	g_scale(PS_POINTS_PER_INCH/CM_PER_INCH, PS_POINTS_PER_INCH/CM_PER_INCH);
	if (!g_is_fullpage()) {
		g_translate(1.0*CM_PER_INCH/72, 1.0*CM_PER_INCH/72);
	}
}

int GLECairoDevicePDF::getDeviceType() {
	return GLE_DEVICE_CAIRO_PDF;
}

GLECairoDeviceEPS::GLECairoDeviceEPS(bool showerror) : GLECairoDevice(showerror) {
}

GLECairoDeviceEPS::~GLECairoDeviceEPS() {
}

void GLECairoDeviceEPS::opendev(double width, double height, GLEFileLocation* outputfile, const string& inputfile) throw(ParserError) {
	clearRecordedData();
	m_width = width;
	m_height = height;
	m_OutputName.copy(outputfile);
	m_OutputName.addExtension(g_device_to_ext(getDeviceType()));
	if (isRecordingEnabled()) {
		m_surface = cairo_ps_surface_create_for_stream(gle_cairo_device_write, (void*)this, 72*width/CM_PER_INCH+2, 72*height/CM_PER_INCH+2);
	} else {
		m_surface = cairo_ps_surface_create(m_OutputName.getFullPath().c_str(), 72*width/CM_PER_INCH+2, 72*height/CM_PER_INCH+2);
	}
	cairo_surface_set_fallback_resolution(m_surface, m_resolution, m_resolution);
	cairo_ps_surface_set_eps(m_surface, true);
	int int_bb_x = 0, int_bb_y = 0;
	computeBoundingBox(width, height, &int_bb_x, &int_bb_y);
	std::ostringstream bbLoRes;
	std::ostringstream bbHiRes;
	bbLoRes << "%%BoundingBox: 0 0 " << int_bb_x << " " << int_bb_y;
	bbHiRes << "%%HiResBoundingBox: 0 0 " << m_boundingBox.getX() << " " << m_boundingBox.getY();
	cairo_ps_surface_dsc_comment(m_surface, bbLoRes.str().c_str());
	cairo_ps_surface_dsc_comment(m_surface, bbHiRes.str().c_str());
	m_cr = cairo_create(m_surface);
	g_scale(PS_POINTS_PER_INCH/CM_PER_INCH, PS_POINTS_PER_INCH/CM_PER_INCH);
	if (!g_is_fullpage()) {
		g_translate(1.0*CM_PER_INCH/72, 1.0*CM_PER_INCH/72);
	}
}

void GLECairoDeviceEPS::getRecordedBytes(string* output) {
	// work-around for bug in Cairo 1.10.x
	int int_bb_x = 0, int_bb_y = 0;
	computeBoundingBox(m_width, m_height, &int_bb_x, &int_bb_y);
	std::ostringstream bbLoRes;
	std::ostringstream bbHiRes;
	bbLoRes << "%%BoundingBox: 0 0 " << int_bb_x << " " << int_bb_y;
	bbHiRes << "%%HiResBoundingBox: 0 0 " << m_boundingBox.getX() << " " << m_boundingBox.getY();
	// do conversion
	std::stringstream inp;
	std::ostringstream outp;
	inp.write(&m_recorded[0], m_recorded.size());
	while (inp.good()) {
		string line;
		getline(inp, line);
		if (str_starts_with(line, "%%BoundingBox:")) {
			outp << bbLoRes.str() << std::endl;
		} else if (str_starts_with(line, "%%HiResBoundingBox:")) {
			outp << bbHiRes.str() << std::endl;
		} else {
			outp << line << std::endl;
		}
	}
	*output = outp.str();
}

int GLECairoDeviceEPS::getDeviceType() {
	return GLE_DEVICE_CAIRO_EPS;
}

#ifdef __WIN32__

#include <cairo-win32.h>

class GLECairoDeviceEMFWinInfo {
public:
	HDC hdc;
	bool shouldClose;
	GLECairoDeviceEMFWinInfo();
	~GLECairoDeviceEMFWinInfo();
};

GLECairoDeviceEMFWinInfo::GLECairoDeviceEMFWinInfo() :
	shouldClose(false)
{
}

GLECairoDeviceEMFWinInfo::~GLECairoDeviceEMFWinInfo()
{
	if (shouldClose) {
		HENHMETAFILE meta = CloseEnhMetaFile(hdc);
		DeleteEnhMetaFile(meta);
	}
}

GLECairoDeviceEMF::GLECairoDeviceEMF(bool showerror) : GLECairoDevice(showerror) {
	m_WinInfo = new GLECairoDeviceEMFWinInfo();
	m_DPI = 1000;
	m_CopyClipboard = false;
}

GLECairoDeviceEMF::~GLECairoDeviceEMF() {
	delete m_WinInfo;
}

void GLECairoDeviceEMF::set_matrix(double newmat[3][3]) {
    cairo_matrix_t matrix;
    matrix.xx = newmat[0][0];
    matrix.xy = newmat[0][1];
    matrix.yx = - newmat[1][0];
    matrix.yy = - newmat[1][1];
    matrix.x0 = newmat[0][2];
	double offset = 0.0;
	if (!g_is_fullpage()) {
		offset = 2.0 * CM_PER_INCH / 72;
	}
    matrix.y0 = (m_height + offset) * m_DPI/CM_PER_INCH - newmat[1][2];
    cairo_set_matrix(m_cr, &matrix);
}

void GLECairoDeviceEMF::opendev(double width, double height, GLEFileLocation* outputfile, const string& inputfile) throw(ParserError) {
	m_width = width;
	m_height = height;
	m_OutputName.copy(outputfile);
	m_OutputName.addExtension("emf");
	double myWidth = width + 2.0/PS_POINTS_PER_INCH*CM_PER_INCH;
	double myHeight = height + 2.0/PS_POINTS_PER_INCH*CM_PER_INCH;
	HDC hdcRef = GetDC(NULL);
	int iWidthMM = GetDeviceCaps(hdcRef, HORZSIZE);    // iWidthMM is the display width in millimeters.
	int iHeightMM = GetDeviceCaps(hdcRef, VERTSIZE);   // iHeightMM is the display height in millimeters.
	int iWidthPels = GetDeviceCaps(hdcRef, HORZRES);   // iWidthPels is the display width in pixels.
	int iHeightPels = GetDeviceCaps(hdcRef, VERTRES);  // iHeightPels is the display height in pixels.
	RECT dim;
	dim.left = 0;
	dim.top = 0;
	dim.right = (int)floor(1000.0 * myWidth + 0.5);
	dim.bottom = (int)floor(1000.0 * myHeight + 0.5);
	// cout << "dim.right = " << dim.right << endl;
	// cout << "dim.bottom = " << dim.bottom << endl;
	if (m_CopyClipboard) {
		m_WinInfo->hdc = CreateEnhMetaFile(hdcRef, NULL, &dim, NULL);
	} else {
		m_WinInfo->hdc = CreateEnhMetaFile(hdcRef, m_OutputName.getFullPath().c_str(), &dim, NULL);
	}
	ReleaseDC(NULL, hdcRef);
	if (m_WinInfo->hdc != NULL) {
		m_WinInfo->shouldClose = true;
	} else {
		g_throw_parser_error("error creating EMF file: '", m_OutputName.getFullPath().c_str(), "'");
	}
	// cout << "Width = " << iWidthMM << " mm = " << iWidthPels << " pixels" << endl;
	// cout << "Height = " << iHeightMM << " mm = " << iHeightPels << " pixels" << endl;
	int viewportWidth = (int)floor(10.0 * myWidth * iWidthPels / iWidthMM + 0.5);
	int viewportHeight = (int)floor(10.0 * myHeight * iHeightPels / iHeightMM + 0.5);
	// cout << "Viewport width in pixels = " << viewportWidth << endl;
	// cout << "Viewport height in pixels = " << viewportHeight << endl;
	int windowWidth = (int)floor(myWidth * m_DPI / CM_PER_INCH + 0.5);
	int windowHeight = (int)floor(myHeight * m_DPI / CM_PER_INCH + 0.5);
	// cout << "Window width in pixels = " << windowWidth << endl;
	// cout << "Window height in pixels = " << windowHeight << endl;
	SetMapMode(m_WinInfo->hdc, MM_ANISOTROPIC);
	SetWindowOrgEx(m_WinInfo->hdc, 0, 0, NULL);
	SetWindowExtEx(m_WinInfo->hdc, windowWidth, windowHeight, NULL);
	SetViewportExtEx(m_WinInfo->hdc, viewportWidth, viewportHeight, NULL);
	m_surface = cairo_win32_printing_surface_create(m_WinInfo->hdc);
	cairo_surface_set_fallback_resolution(m_surface, m_resolution, m_resolution);
	m_cr = cairo_create(m_surface);
	computeBoundingBox(width, height);
	g_scale(m_DPI/CM_PER_INCH, m_DPI/CM_PER_INCH);
	if (!g_is_fullpage()) {
		g_translate(1.0*CM_PER_INCH/72, 1.0*CM_PER_INCH/72);
	}
}

BOOL PutEnhMetafileOnClipboard(HWND hWnd, HENHMETAFILE hEMF) {
	BOOL bResult = false;
	HENHMETAFILE hEMF2 = CopyEnhMetaFile(hEMF, NULL);
	if (hEMF2 != NULL) {
		if (OpenClipboard(hWnd)) {
			if (EmptyClipboard()) {
				HANDLE hRes = (HENHMETAFILE)SetClipboardData(CF_ENHMETAFILE, hEMF2);
				bResult = (hRes != NULL);
			}
			CloseClipboard();
		}
	}
	return bResult;
}

void GLECairoDeviceEMF::closedev(void) {
	GLECairoDevice::closedev();
	HENHMETAFILE meta = CloseEnhMetaFile(m_WinInfo->hdc);
	if (meta == NULL) {
		return;
	}
	if (m_CopyClipboard) {
		PutEnhMetafileOnClipboard(NULL, meta);
	}
	DeleteEnhMetaFile(meta);
	m_WinInfo->shouldClose = false;
}

int GLECairoDeviceEMF::getDeviceType() {
	return GLE_DEVICE_EMF;
}

#endif

#endif
