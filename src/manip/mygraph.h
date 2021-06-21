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

int g_get_path(int *pathonoff);
int g_set_font_width(double v);
int draw_key(int nkd, double koffsetx, double koffsety, char *kpos, double khei, int knobox);
int font_get_encoding(int ff);
int my_char(int ff, int cc);
int g_get_just(int *j);
int g_set_just(int j);
int g_curve(int32 *pcode);
int g_devcmd(char *s);
int d_devcmd(char *s);
int g_get_line_join(int *jj);
int g_get_line_cap(int *jj);
int g_postscript(char *ss,double w,double h);
int g_set_font(int j);
int g_marker_def(char *name, char *subname);
int begin_tab(int *pln, int32 *pcode, int *cp);
int begin_graph(int *pln, int32 *pcode, int *cp);
int begin_key(int *pln, int32 *pcode, int *cp);
int begin_text(int *pln, int32 *pcode, int *cp, double w);
int g_set_line_styled(double x);
int g_text(char *s);
int g_jtext(int just);
int g_measure(const std::string& s, dbl *l, dbl *r, dbl *u, dbl *d);
int g_arc(dbl r,dbl t1,dbl t2,dbl cx,dbl cy);
int g_narc(dbl r,dbl t1,dbl t2,dbl cx,dbl cy);
int g_arcto(dbl x1,dbl y1,dbl x2,dbl y2,dbl rrr);
int g_beginclip(void);
int g_bezier(dbl x1,dbl y1,dbl x2,dbl y2,dbl x3,dbl y3);
int g_bitmap(char *bmap);
int g_box_fill(dbl x1, dbl y1, dbl x2, dbl y2);
int g_box_stroke(dbl x1, dbl y1, dbl x2, dbl y2);
int g_char(int font, int cc);
int g_circle_fill(double zr);
int g_circle_stroke(double zr);
int g_clear(void);
int g_clip(void);
int g_close(void);
int g_closepath(void);
int g_dbezier(dbl x1,dbl y1,dbl x2,dbl y2,dbl x3,dbl y3);
int g_dev(double x, double y,double *xd,double *yd);
int g_dfont(char *s);
int g_dline(double x, double y);
int g_dmove(double x, double y);
int g_dojust(dbl *x1, dbl *y1, dbl *x2, dbl *y2, int jj);
int g_dotjust(dbl *x1, dbl *y1, dbl l, dbl r, dbl u, dbl d, int jj);
int g_endclip(void);
int g_fill(void);
int g_fill_ary(int nwk,double **wkx,double **wky);
int g_flush(void);
int g_get_bounds(dbl *x1,dbl *y1,dbl *x2,dbl *y2);
int g_get_devsize(dbl *x,dbl *y);
int g_get_end(dbl *x,dbl *y);
int g_get_fill(int32 *f);
int g_get_line_style(char *s);
int g_get_line_styled(double *w);
int g_get_line_width(double *w);
int g_get_state(char *s);
int g_get_type(char *t);
int g_get_usersize(dbl *x,dbl *y);
int g_get_xy(double *x,double *y);
int g_grestore(void);
int g_gsave(void);
int g_init_bounds(void);
int g_line(double zx,double zy);
int g_line_ary(int nwk,double **wkx,double **wky);
int g_message(char *s);
int g_move(double zx,double zy);
int g_rmove(double zx,double zy);
int g_rline(double zx,double zy);
int g_newpath(void);
int g_open(dbl width,dbl height);
int g_rdev(double x, double y,double *xd,double *yd);
int g_reverse(void);
int g_rotate(double ar);
int g_rundev(double x, double y,double *xd,double *yd);
int g_scale(double sx,double sy);
int g_update_bounds(double x,double y);
int g_set_color(int32 l);
int g_set_end(dbl x,dbl y);
int g_set_line_cap(int i);
int g_set_line_join(int i);
int g_set_line_miterlimit(double d);
int g_set_line_style(const char *s);
int g_set_line_width(double w);
int g_set_matrix(double (*newmat)[3][3]);
int g_set_path(int onoff);
int g_set_state(char *s);
int g_set_xy(double x, double y);
int g_source(char *s);
int g_stroke(void);
int g_translate(double ztx,double zty);
int g_undev(double ux,double uy, double *x,double *y);
