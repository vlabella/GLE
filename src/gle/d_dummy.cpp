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

/*
 * 2004 Jan Struyf
 *
 */

#include "all.h"
#include "core.h"
#include "file_io.h"
#include "texinterface.h"
#include "cutils.h"
#include "gprint.h"

GLEDummyDevice::GLEDummyDevice(bool showerror) {
	m_ShowError = showerror;
}

GLEDummyDevice::~GLEDummyDevice() {
}

void GLEDummyDevice::arc(dbl r,dbl t1,dbl t2,dbl cx,dbl cy) {
}

void GLEDummyDevice::arcto(dbl x1,dbl y1,dbl x2,dbl y2,dbl rrr) {
}

void GLEDummyDevice::beginclip(void)  {
}

void GLEDummyDevice::bezier(dbl x1,dbl y1,dbl x2,dbl y2,dbl x3,dbl y3) {
}

void GLEDummyDevice::box_fill(dbl x1, dbl y1, dbl x2, dbl y2) {
}

void GLEDummyDevice::box_stroke(dbl x1, dbl y1, dbl x2, dbl y2, bool reverse) {
}

void GLEDummyDevice::dochar(int font, int cc) {
}

void GLEDummyDevice::resetfont() {
}

void GLEDummyDevice::circle_fill(double zr) {
}

void GLEDummyDevice::circle_stroke(double zr) {
}

void GLEDummyDevice::clear(void) {
}

void GLEDummyDevice::clip(void) {
}

void GLEDummyDevice::closedev(void) {
}

void GLEDummyDevice::closepath(void) {
}

void GLEDummyDevice::dfont(char *c) {
}

void GLEDummyDevice::ellipse_fill(double rx, double ry) {
}

void GLEDummyDevice::ellipse_stroke(double rx, double ry) {
}

void GLEDummyDevice::elliptical_arc(double rx,double ry,double t1,double t2,double cx,double cy) {
}

void GLEDummyDevice::elliptical_narc(double rx,double ry,double t1,double t2,double cx,double cy) {
}

void GLEDummyDevice::endclip(void)  {
}

void GLEDummyDevice::fill(void) {
}

void GLEDummyDevice::fill_ary(int nwk,double *wkx,double *wky) {
}

void GLEDummyDevice::flush(void) {
}

std::string GLEDummyDevice::get_type() {
	return "DUMMY";
}

void GLEDummyDevice::line(double zx,double zy) {
}

void GLEDummyDevice::line_ary(int nwk,double *wkx,double *wky) {
}

void GLEDummyDevice::message(char *s) {
}

void GLEDummyDevice::move(double zx,double zy) {
}

void GLEDummyDevice::narc(dbl r,dbl t1,dbl t2,dbl cx,dbl cy) {
}

void GLEDummyDevice::newpath(void) {
}

void GLEDummyDevice::opendev(double width, double height, GLEFileLocation* outputfile, const string& inputfile) throw(ParserError) {
	g_scale(PS_POINTS_PER_INCH/CM_PER_INCH, PS_POINTS_PER_INCH/CM_PER_INCH);
	g_translate(1.0*CM_PER_INCH/72, 1.0*CM_PER_INCH/72);
}

void GLEDummyDevice::pscomment(char* ss) {
}

void GLEDummyDevice::reverse(void)    /* reverse the order of stuff in the current path */ {
}

void GLEDummyDevice::set_color(const GLERC<GLEColor>& color) {
}

void GLEDummyDevice::set_fill(const GLERC<GLEColor>& fill) {
}

void GLEDummyDevice::set_line_cap(int i) {
}

void GLEDummyDevice::set_line_join(int i) {
}

void GLEDummyDevice::set_line_miterlimit(double d) {
}

void GLEDummyDevice::set_line_style(const char *s) {
}

void GLEDummyDevice::set_line_styled(double dd) {
}

void GLEDummyDevice::set_line_width(double w) {
}

void GLEDummyDevice::set_matrix(double newmat[3][3]) {
}

void GLEDummyDevice::set_path(int onoff) {
}

void GLEDummyDevice::source(const char *s) {
}

void GLEDummyDevice::stroke(void) {
}

void GLEDummyDevice::set_color(void) {
}

void GLEDummyDevice::set_fill(void) {
}

void GLEDummyDevice::xdbox(double x1, double y1, double x2, double y2) {
}

void GLEDummyDevice::devcmd(const char *s) {
}

FILE* GLEDummyDevice::get_file_pointer(void) {
	return NULL;
}

int GLEDummyDevice::getDeviceType() {
	return GLE_DEVICE_DUMMY;
}
