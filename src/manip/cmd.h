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

#ifndef INC_CMD_H
#define INC_CMD_H

void do_command(char *cmd);
void passcmd(char *source,char *tk[],int *ntok);
void do_assign(char *t1, char *t2);
void evaluate(char *exp, double *v);
bool range_next(RANGE *r);
bool range_def(const char *sxx, RANGE *r);
char *range_int(char *s, int *v);
void swapint(int *a, int *b);
void cmd_load(char *fname, const char *range, int ntok);
void m_tokinit(const char *termset);
char *m_tokend(char *s);
void cmd_load_list(RANGE *r, char *inbuff);
void cmd_load_line(int x, int y, char *inbuff);
void load_str(int x, int y, char *s);
void cmd_save(char *fname, const char *range, const char *format, int ntok);
int strcpydecimal(char *dest, char *src, int wid, int dpoints);
int strcpywidth(char *dest, char *src, int wid);
void  set_outwidth(char *s);
char *unquote(char *s);
void cmd_copy(char *src, char *dest, char *ifexp, int ntok, int always);
void cmd_generate(char *patx, char *dest, int ntok);
bool gen_next(char *pat, double *v);
void cmd_polish(char *exp);
void cmd_eval(int x1, int y1, double *v);
void cmd_delete(char *range, char *ifexp, int ntok);
void cmd_sort(char *range, char *ifexp, int ntok);
void sort_shuffle(int i, int r1, int *pnt);
void cmd_insert(char *range);
void cmd_data(char *range);
bool swap_def(char *s, RANGE *r);
void cmd_swap(char *range);
void log_open(char *fname);
void log_write(char *s);
void log_close();
void add_dotman(char *s);
void at_open(char *fname);
bool at_read(char *s);
void strip_colon(char *s);
void cmd_parsum(char *range, char *dest);
void cmd_sum(char *range);
void rangestd(RANGE *rr, double mean, int32 numrow, double *variance, double *stddev);
void cmd_clear(char *range);
void cmd_fit(char *range);
void fitlsq(RANGE *rrr,int ndata, double *a, double *b, double *siga, double *sigb, double *chi2, double *q);

#endif
