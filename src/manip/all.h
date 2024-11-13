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

#ifdef HAVE_CONFIG_H
	#include "../config.h"
#else
	#include "../config_noauto.h"
#endif

#ifdef INT32
typedef int int32;
typedef unsigned int uint32;
#else
typedef long int32;
typedef unsigned long uint32;
#endif

#include <stdio.h>
#if HAVE_SYS_TYPES_H
	#include <sys/types.h>
#endif
#if HAVE_SYS_STAT_H
	#include <sys/stat.h>
#endif
#if STDC_HEADERS
	#include <stdlib.h>
	#include <stddef.h>
#else
	#if HAVE_STDLIB_H
		#include <stdlib.h>
	#endif
#endif
#if HAVE_STRING_H
	#if !STDC_HEADERS && HAVE_MEMORY_H
		#include <memory.h>
	#endif
	#include <string.h>
#endif
#if HAVE_STRINGS_H
	#include <strings.h>
#endif
#if HAVE_UNISTD_H
	#include <unistd.h>
#endif

#include <ctype.h>
#include <math.h>

#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#if defined(__unix__) || defined(__APPLE__)
	#include <stdarg.h>
#endif

#if defined(__OS2__) && defined(__EMX__)
	#include <stdarg.h>
	#undef wprintf
	#define wprintf printf
#endif

struct s_range {int col,row,c1,r1,c2,r2,colfirst; };
typedef struct s_range RANGE;

#include <string>
#include <string>
#include <vector>
#include <fstream>

#ifdef _WIN32
#include <direct.h>
#endif

// using namespace std;  should not reside in header file

#ifdef HAVE_NCURSES_H
	#include <ncurses.h>
#endif
#ifdef HAVE_CURSES_H
	#include <curses.h>
#endif

#include "manip.h"
#include "cell.h"
#include "cmd.h"
#include "filemenu.h"
#include "mjl.h"
#include "eval.h"
#include "mscreen.h"
#include "../gle/cutils.h"

/*
#if defined(__OS2__) && defined(__EMX__)
#define delline v_delline
#define clreol  v_clreol
#define putch   v_putc
#define cputs   v_puts
#endif
*/

#define far
#define farcalloc calloc
#define DASHCHAR '.'

#define gprint printf
#define myfree free

struct GLEMemoryCell;

char *range_int(char *s, int *v);
void find_mkey(char *cp, int *idx);
extern int gle_debug;
typedef char (*(*TOKENS)[500]);
typedef unsigned char uchar;
typedef double dbl;
char *unquote(char *s);
std::string gledir(const char *s);
int set_missing(int x, int y);
double vcell(int x, int y);
void var_find_rc(int *idx, int *var, int *nd, int c);
void var_getstr(int v, char *s);
int alloc_temp(int n);
void gle_abort(const char *s);
void d_tidyup(void);
int text_inkey(void);
void ncpy(char *d, const char *s, int n);
void token_space(void);
char *sdup(const char *s);
/* void myfree(void *p); */
void myfrees(void *p, char *s);
void *myallocn(int32 nitems,int32 size);
void *myalloc(int32 size);
void *myallocz(int32 size);
int showpcode(int32 *p);
void add_fn(char *pcode, int *plen, int i);
void add_i(char *pcode,int *plen,int32 i);
void add_f(char *pcode,int *plen,double f);
void add_pcode(char *pcode,int *plen,char *fcode,int *flen);
void add_string(char *pcode, int *plen, char *s);
void add_strvar(char *pcode,int *plen,int i);
void add_var(char *pcode,int *plen,int i);
char *eval_str(int32 *pcode,int *plen);
char *find_non_space(char *cp);
char *find_term(char *cp);
char *un_quote(char *ct);
void find_un(char *cp, int *idx,int *ret,int *np,int **plist);
bool gle_isnumber(char *s);
char manip_lastchar(const char *s, char c);
void mystrcpy(char **d, const char *s);
void polish(char *expr,char *pcode,int *plen,int *rtype);
int scheck(int v);
int spop(int v);
int spush(int v);
void stack_op(char *pcode, int *plen, int stk[] ,int stkp[], int *nstk,  int i, int p);
void token(char *lin,char *tok[],int *ntok,char *outbuff);
void var_add(const char *name, int *idx, int *type);        /* Add a variable to the list */
void var_find(const char *name, int *idx, int *type);       /* Find a variable in the list */
void var_findadd(const char *name, int *idx, int *type);    /* Add a variable to the list */
void var_get(int jj, double *v);
void var_nlocal(int *l);
void var_set(int jj, double v);
void var_set(int jj, GLEMemoryCell* value);
void var_setstr(int jj, char *s);
void xy_polar(double dx,double dy,double *radius,double *angle);

void sub_clear(void);
int sub_def(char *s);
void sub_call(int idx,double *pval,char **pstr,int *npm);
int sub_find(char *s,int *idx,int *zret, int *np, int **plist);
void sub_get_startend(int idx, int *ss, int *ee);
void sub_param(int idx,char *s);
void sub_set_return(double d);
void sub_set_startend(int idx, int ss, int ee);

void show_cell(int x, int y);
void show_ifcell(int x, int y);
char *scell(int x, int y);
void fner_clear(void);
void show_cellwide(int x, int y);
int read_command(int *cmd,char *ans,const char *ques);
void do_arrow(int k);
void hi_cell(int x,int y);
void fix_cur(void);
void gotocell(int x, int y);
int read_str(int *cmd, char *s);
int32 coreleft(void);
void manip_refresh();
const char *function_bar();
void scr_refresh();
