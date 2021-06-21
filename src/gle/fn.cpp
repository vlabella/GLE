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
#include "cutils.h"
#include "fn.h"

/*-------------------------------------------------------------------*/
/*      Find the function called *cp, and return it's number         */
/*-------------------------------------------------------------------*/

/* You can add a function, you MUST place it in alphabetical order   */
/* and give it the next unused index number			                 */

/* ret = Type of returned value,  1=number, 2=string                 */
/* np = number of parameters expected                                 */
/* Parameters p0...p4  ==  1=number, 2=string, 0=none (e.g. height()).*/

// current max is 105 ==> FN_FACTORIAL

#define NKEYS (sizeof keywfn / sizeof(struct keyw))
struct keyw { const char *word; int index; int ret,np,p[5]; } keywfn[] = {
	{ " ",           0,             1,0, { 0,0,0,0,0 } },
	{ "+",           1,             1,0, { 0,0,0,0,0 } },
	{ "-",           2,             1,0, { 0,0,0,0,0 } },
	{ "ABS",         3,             1,1, { 1,0,0,0,0 } },
	{ "ACOS",        53,            1,1, { 1,0,0,0,0 } },
	{ "ACOSH",       68,            1,1, { 1,0,0,0,0 } },
	{ "ACOT",        56,            1,1, { 1,0,0,0,0 } },
	{ "ACOTH",       71,            1,1, { 1,0,0,0,0 } },
	{ "ACSC",        58,            1,1, { 1,0,0,0,0 } },
	{ "ACSCH",       73,            1,1, { 1,0,0,0,0 } },
	{ "ARG",         FN_ARG,        1,1, { 1,0,0,0,0 } },
	{ "ARG$",        FN_ARGS,       2,1, { 1,0,0,0,0 } },
	{ "ASEC",        57,            1,1, { 1,0,0,0,0 } },
	{ "ASECH",       72,            1,1, { 1,0,0,0,0 } },
	{ "ASIN",        54,            1,1, { 1,0,0,0,0 } },
	{ "ASINH",       69,            1,1, { 1,0,0,0,0 } },
	{ "ASSOCIATED_LAGUERRE",    FN_ASSOCIATED_LAGUERRE,   1,3, { 1,1,1,0,0 } },
	{ "ASSOCIATED_LEGENDRE",    FN_ASSOCIATED_LEGENDRE,   1,3, { 1,1,1,0,0 } },
	{ "ATAN",        4,             1,1, { 1,0,0,0,0 } },
	{ "ATAN2",       FN_ATAN2,      1,2, { 1,1,0,0,0 } },
	{ "ATANH",       70,            1,1, { 1,0,0,0,0 } },
	{ "ATN",         4,             1,1, { 1,0,0,0,0 } }, // for backward coompatability same as ATAN
	{ "BESSEL_FIRST",    FN_BESSEL_FIRST,   1,2, { 1,1,0,0,0 } },
	{ "BESSEL_SECOND",   FN_BESSEL_SECOND,  1,2, { 1,1,0,0,0 } },
	{ "CHR$",        52,            2,1, { 1,0,0,0,0 } },
	{ "COS",         5,             1,1, { 1,0,0,0,0 } },
	{ "COSH",        62,            1,1, { 1,0,0,0,0 } },
	{ "COT",         59,            1,1, { 1,0,0,0,0 } },
	{ "COTH",        65,            1,1, { 1,0,0,0,0 } },
	{ "CSC",         61,            1,1, { 1,0,0,0,0 } },
	{ "CSCH",        67,            1,1, { 1,0,0,0,0 } },
	{ "CVTCOLOR",    49,            1,1, { 2,0,0,0,0 } },
	{ "CVTFONT",     50,            1,1, { 2,0,0,0,0 } },
	{ "CVTGRAY",     45,            1,1, { 1,0,0,0,0 } },
	{ "CVTINT",      46,            1,1, { 1,0,0,0,0 } },
	{ "CVTMARKER",   48,            1,1, { 2,0,0,0,0 } },
	{ "CVTRGB",      47,            1,3, { 1,1,1,0,0 } },
	{ "DATAXVALUE",  FN_DATAXVALUE, 1,2, { 2,1,0,0,0 } },
	{ "DATAYVALUE",  FN_DATAYVALUE, 1,2, { 2,1,0,0,0 } },
	{ "DATE$",       6,             2,0, { 0,0,0,0,0 } },
	{ "DEVICE$",     51,            2,0, { 0,0,0,0,0 } },
	{ "DOUBLE_FACTORIAL",   FN_DOUBLE_FACTORIAL,  1,1, { 1,0,0,0,0 } },
	{ "EOF",         55,            1,1, { 1,0,0,0,0 } },
	{ "EVAL",        FN_EVAL,       1,1, { 2,0,0,0,0 } },
	{ "EXP",         7,             1,1, { 1,0,0,0,0 } },
	{ "FACTORIAL",   FN_FACTORIAL,  1,1, { 1,0,0,0,0 } },
	{ "FEOF",        55,            1,1, { 1,0,0,0,0 } },
	{ "FILE$",       FN_FILE,       2,0, { 0,0,0,0,0 } },
	{ "FIX",         8,             1,1, { 1,0,0,0,0 } },
	{ "FONT",        FN_FONT,       1,1, { 2,0,0,0,0 } },
	{ "FORMAT$",     79,            2,2, { 1,2,0,0,0 } },
	{ "GETENV",      FN_GETENV,     2,1, { 2,0,0,0,0 } },
	{ "HEIGHT",      9,             1,1, { 0,0,0,0,0 } },
	{ "HERMITE",     FN_HERMITE,    1,2, { 1,1,0,0,0 } },
	{ "INT",         10,            1,1, { 1,0,0,0,0 } },
	{ "ISNAME",      FN_ISNAME,     1,1, { 2,0,0,0,0 } },
    { "JUSTIFY",     FN_JUSTIFY,    1,1, { 2,0,0,0,0 } },
    { "LAGUERRE",    FN_LAGUERRE,   1,2, { 1,1,0,0,0 } },
	{ "LEFT$",       11,            2,2, { 2,1,0,0,0 } },
	{ "LEGENDRE",    FN_LEGENDRE,   1,2, { 1,1,0,0,0 } },
	{ "LEN",         12,            1,1, { 2,0,0,0,0 } },
	{ "LOG",         13,            1,1, { 1,0,0,0,0 } },
	{ "LOG10",       14,            1,1, { 1,0,0,0,0 } },
	{ "MAX",         FN_MAX,        1,2, { 1,1,0,0,0 } },
	{ "MIN",         FN_MIN,        1,2, { 1,1,0,0,0 } },
	{ "NARGS",       FN_NARGS,      1,0, { 0,0,0,0,0 } },
	{ "NDATA",       FN_NDATA,      1,1, { 2,0,0,0,0 } },
	{ "NDATASETS",   FN_NDATASETS,  1,0, { 0,0,0,0,0 } },
	{ "NOT",         15,            1,1, { 1,0,0,0,0 } },
	{ "NUM$",        16,            2,1, { 1,0,0,0,0 } },
	{ "NUM1$",       17,            2,1, { 1,0,0,0,0 } },
	{ "PAGEHEIGHT",  18,            1,0, { 0,0,0,0,0 } },
	{ "PAGEWIDTH",   19,            1,0, { 0,0,0,0,0 } },
	{ "PATH$",       FN_PATH,       2,0, { 0,0,0,0,0 } },
	{ "POINTX",      77,            1,1, { 2,0,0,0,0 } },
	{ "POINTY",      78,            1,1, { 2,0,0,0,0 } },
	{ "POS",         20,            1,3, { 2,2,1,0,0 } },
	{ "PTX",         77,            1,1, { 2,0,0,0,0 } },
	{ "PTY",         78,            1,1, { 2,0,0,0,0 } },
	{ "RGB",         47,            1,3, { 1,1,1,0,0 } },
	{ "RGB255",      80,            1,3, { 1,1,1,0,0 } },
	{ "RGBA",        FN_RGBA,       1,4, { 1,1,1,1,0 } },
	{ "RGBA255",     FN_RGBA255,    1,4, { 1,1,1,1,0 } },
	{ "RIGHT$",      21,            2,2, { 2,1,0,0,0 } },
	{ "RND",         22,            1,1, { 1,0,0,0,0 } },
	{ "SDIV",        FN_SDIV,       1,2, { 1,1,0,0,0 } },
	{ "SEC",         60,            1,1, { 1,0,0,0,0 } },
	{ "SECH",        66,            1,1, { 1,0,0,0,0 } },
	{ "SEG$",        23,            2,3, { 2,1,1,0,0 } },
	{ "SGN",         24,            1,1, { 1,0,0,0,0 } },
	{ "SIN",         25,            1,1, { 1,0,0,0,0 } },
	{ "SINH",        63,            1,1, { 1,0,0,0,0 } },
	{ "SPHERICAL_HARMONIC",    FN_SPHERICAL_HARMONIC,   1,4, { 1,1,1,1,0 } },
	{ "SQR",         26,            1,1, { 1,0,0,0,0 } },
	{ "SQRT",        27,            1,1, { 1,0,0,0,0 } },
	{ "TAN",         28,            1,1, { 1,0,0,0,0 } },
	{ "TANH",        64,            1,1, { 1,0,0,0,0 } },
	{ "TDEPTH",      29,            1,1, { 2,0,0,0,0 } },
	{ "THEIGHT",     30,            1,1, { 2,0,0,0,0 } },
	{ "TIME$",       31,            2,0, { 0,0,0,0,0 } },
	{ "TODEG",       74,            1,1, { 1,0,0,0,0 } },
	{ "TORAD",       75,            1,1, { 1,0,0,0,0 } },
	{ "TWIDTH",      32,            1,1, { 2,0,0,0,0 } },
	{ "VAL",         33,            1,1, { 2,0,0,0,0 } },
	{ "WIDTH",       34,            1,1, { 0,0,0,0,0 } },
	{ "XBAR",        FN_XBAR,       1,2, { 1,1,0,0,0 } },
	{ "XEND",        35,            1,0, { 0,0,0,0,0 } },
	{ "XG",          36,            1,1, { 1,0,0,0,0 } },
	{ "XG3D",        FN_XG3D,       1,3, { 1,1,1,0,0 } },
	{ "XGRAPH",      36,            1,1, { 1,0,0,0,0 } },
	{ "XMAX",        37,            1,1, { 0,0,0,0,0 } },
	{ "XMIN",        38,            1,1, { 0,0,0,0,0 } },
	{ "XPOS",        39,            1,0, { 0,0,0,0,0 } },
	{ "XY2ANGLE",    FN_XY2ANGLE,   1,2, { 1,1,0,0,0 } },
	{ "YEND",        40,            1,0, { 0,0,0,0,0 } },
	{ "YG",          41,            1,1, { 1,0,0,0,0 } },
	{ "YG3D",        FN_YG3D,       1,3, { 1,1,1,0,0 } },
	{ "YGRAPH",      41,            1,1, { 1,0,0,0,0 } },
	{ "YMAX",        42,            1,1, { 0,0,0,0,0 } },
	{ "YMIN",        43,            1,1, { 0,0,0,0,0 } },
	{ "YPOS",        44,            1,0, { 0,0,0,0,0 } }
};

int binsearch(char *word, struct keyw tab[], int n);

void find_un(char *cp, int *idx,int *ret,int *np,int **plist)
{
	int i;
	i = binsearch(cp,keywfn,NKEYS);
	*idx = keywfn[i].index;
	*ret = keywfn[i].ret;
	*np =  keywfn[i].np;
	*plist = &keywfn[i].p[0];
}

/*------------------------------------------------------------------*/
/* Simple binary search 					    */
/*------------------------------------------------------------------*/
int binsearch(char *word, struct keyw tab[], int n)
{
	int cond,low,high,mid;
	low = 0;
	high = n-1;
	while (low <= high) {
		mid = (low+high) / 2;
		if ((cond = str_i_cmp(word,tab[mid].word)) < 0)
			high = mid - 1;
		else if (cond > 0)
			low = mid + 1;
		else
			return mid;
	}
	return 0;
}
