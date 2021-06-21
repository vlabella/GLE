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

#ifndef INCLUDE_GLEPRO
#define INCLUDE_GLEPRO

#include "tokens/Tokenizer.h"

class GLEColor;
class GLEString;

#define SC (char *)
#define UC (unsigned char *)

#define GLE_TRNS_MATRIX_SIZE 3*3*sizeof(double)

typedef double dbl;

typedef unsigned char uchar;

//int do_pcode(int *srclin, int *pcode, int plen, int *pend);
char *eval_str(int *pcode,int *plen);
string gledir(const char *filename);
const char *gle_top(void);
char *sdup(const char *s);
double graph_xgraph(double v);
double graph_ygraph(double v);
double myatan2(double y, double x);
double tex_xend(void);
double tex_yend(void);
void df_arc(dbl r,dbl t1,dbl t2,dbl cx,dbl cy);
void df_arcto(dbl x1, dbl y1, dbl x2, dbl y2, dbl r);
int df_box_fill(dbl x1, dbl y1, dbl x2, dbl y2);
int df_box_stroke(dbl x1, dbl y1, dbl x2, dbl y2);
int df_circle_fill(dbl r);
int df_circle_stroke(dbl r);
int df_narc(dbl r,dbl t1,dbl t2,dbl cx,dbl cy);
int mem_total(void);
int mem_worst(void);
GLERC<GLEColor> pass_color_var(const std::string& token) throw(ParserError);
int pass_font(const std::string& token);
int pass_justify(const std::string& token);
int pass_marker(char *s) throw(ParserError);
int testfree(int i);
int text_printf(int *in,int ilen);
int w_message(char *s);
void init_memory(void);
void *myalloc(int size);
void *myallocn(int nitems,int size);
void *myallocz(int size);
void add_svga(void);
void alloc_temp(int n);
void big_close(void);
void box_clip(double *x1, double *y1, double xmin, double ymin, double xmax, double ymax);
void char_bbox(int ff,int cc,double *xx1, double *yy1, double *xx2, double *yy2);
void cmd_name(int idx, char **cp);
void d_tidyup(void);
void debug_polish(int *pcode,int *zcp);
void dr_init(void);
void f_init(void);
void find_mkey(string cp, int *idx);
void find_un(char *cp, int *idx,int *ret,int *np,int **plist);
void font_get_lineskip(double *ls,double *gap);
void font_get_parskip(double *ls,double *gap);
void font_init(void);
void font_load_metric(int ff);
void font_replace_vector(int ff);
void freefont(int i);
void g_shear(double sx,double sy);
void gle_include(char *s);
void gprint_do(char *output);
void gr_nomiss(int i);
void gr_thrownomiss(void);
void graph_freedata(void);
void myfree(void *p);
void myfrees(void *p, const char *s);
void mystrcpy(char **d, const char *s);
void ncpy(char *d, const char *s, int n);
void polar_xy(double r, double angle, double *dx, double *dy);
void polar_xy(double rx, double ry, double angle, double *dx, double *dy);
void scheck(int v);
void set_glue(int *in,int ilen,double actual,double width,double stretch,double shrink,double *setlen);
void setdstr(char **s, const char *in);
void setdstr(char **s, const std::string& in);
void setsstr(char **s, const char *in);
void showpcode(int *p);
void spop(int v);
void spush(int v);
void sub_clear(bool undef);
void sub_get_startend(int idx, int *ss, int *ee);
void sub_set_return(double d);
void sub_set_return_str(GLEString* s);
void subscript();
void text_block(const string& s,double width,int justify);
void text_block(const string& s,double width,int justify, int innerjust);
void text_draw(int *in,int ilen);
void text_tomacro(const string& in, uchar *out);
void text_topcode(uchar *in, int *out, int *lout)      /*  passed a paragraph  */;
void text_wrapcode(int *in,int ilen,double width);
void wprintf_do(char *s);
void xy_polar(double dx,double dy,double *radius,double *angle);
void reset_new_error(bool val);
int get_nb_errors();
void inc_nb_errors();
const char* get_font_name(int idx);
int get_nb_fonts();
void pass_file_name(const char* name, string& file);
int get_nb_extra_args();
const string& get_extra_arg(int i);
void gle_abort(const char *s);
void do_wait_for_enter();
void do_wait_for_enter_exit(int exitcode);
void validate_open_input_stream(ifstream& input, const string& fname) throw(ParserError);
FILE* validate_fopen(const string& fname, const char *mode, bool isread) throw(ParserError);
void validate_file_name(const string& fname, bool isread) throw(ParserError);
string fontdir(const char *s);
int check_has_font(const std::string& name);
int get_marker_string(const string& marker, IThrowsError* error);
void gle_set_constants();

#endif
