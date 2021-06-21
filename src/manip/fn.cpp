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
#include "eval.h"

/*------------------------------------------------------------------*/
/*      Find the function callped *cp, and return it's number       */
/*------------------------------------------------------------------*/

/* You can add a function, you MUST place it in alphabetical order  */
/* and give it the next unused index number			    */

/* ret = Type of returned value,  1=number, 2=string */
/* np = number of parameters expected */
/* Parameters p0...p4  ==  1=number, 2=string, 0=none (e.g. height()) .  */

#define NKEYS (sizeof keywfn / sizeof(struct keyw))
struct keyw { const char *word; int index; int ret,np,p[5]; } keywfn[] = {
		/*		 r  np p1p2p3p4p5 */
		 { " ",0	   ,1 ,0 ,{ 0,0,0,0,0 } }
		,{ "+",f_plus	   ,1 ,0 ,{ 0,0,0,0,0 } }
		,{ "-",f_minus	   ,1 ,0 ,{ 0,0,0,0,0 } }
		,{ "ABS",f_abs	   ,1 ,1 ,{ 1,0,0,0,0 } }
		,{ "ATN",f_atn	   ,1 ,1 ,{ 1,0,0,0,0 } }
		,{ "CELL",f_cell   ,1 ,2 ,{ 1,1,0,0,0 } }
		,{ "COS",f_cos	   ,1 ,1 ,{ 1,0,0,0,0 } }
		,{ "DATE$",f_date  ,2 ,0 ,{ 0,0,0,0,0 } }
		,{ "EXP",f_exp	   ,1 ,1 ,{ 1,0,0,0,0 } }
		,{ "FIX",f_fix	   ,1 ,1 ,{ 1,0,0,0,0 } }
		,{ "INT",f_int	   ,1 ,1 ,{ 1,0,0,0,0 } }
		,{ "LEFT$",f_left  ,2 ,2 ,{ 2,1,0,0,0 } }
		,{ "LEN",f_len	   ,1 ,1 ,{ 2,0,0,0,0 } }
		,{ "LOG",f_log	   ,1 ,1 ,{ 1,0,0,0,0 } }
		,{ "LOG10",f_log10 ,1 ,1 ,{ 1,0,0,0,0 } }
		,{ "MISS",f_miss   ,1 ,2 ,{ 1,1,0,0,0 } }
		,{ "NOT",f_not	   ,1 ,1 ,{ 1,0,0,0,0 } }
		,{ "NUM$",f_num	   ,2 ,1 ,{ 1,0,0,0,0 } }
		,{ "NUM1$",f_num1  ,2 ,1 ,{ 1,0,0,0,0 } }
		,{ "POS",f_pos	   ,1 ,2 ,{ 2,1,0,0,0 } }
		,{ "RIGHT$",f_right,2 ,2 ,{ 2,1,0,0,0 } }
		,{ "RND",f_rnd	   ,1 ,1 ,{ 1,0,0,0,0 } }
		,{ "SEG$",f_seg	   ,2 ,3 ,{ 2,1,1,0,0 } }
		,{ "SGN",f_sgn	   ,1 ,1 ,{ 1,0,0,0,0 } }
		,{ "SIN",f_sin	   ,1 ,1 ,{ 1,0,0,0,0 } }
		,{ "SQR",f_sqr	   ,1 ,1 ,{ 1,0,0,0,0 } }
		,{ "SQRT",f_sqrt   ,1 ,1 ,{ 1,0,0,0,0 } }
		,{ "TAN",f_tan	   ,1 ,1 ,{ 1,0,0,0,0 } }
		,{ "TIME$",f_time  ,2 ,0 ,{ 0,0,0,0,0 } }
		,{ "VAL",f_val	   ,1 ,1 ,{ 2,0,0,0,0 } }
} ;

int binsearch(char *word, struct keyw tab[], int n);

void find_un(char *cp, int *idx,int *ret,int *np,int **plist) {
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

int binsearch(char *word, struct keyw tab[], int n) {
	int cond,low,high,mid;
	low = 0;
	high = n-1;
	while (low <= high) {
		mid = (low+high) / 2;
		if ((cond = strcmp(word,tab[mid].word)) < 0)
			high = mid - 1;
		else if (cond > 0)
			low = mid + 1;
		else
			return mid;
	}
	return 0;
}
