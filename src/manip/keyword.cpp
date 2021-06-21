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
#include "keyword.h"

/*------------------------------------------------------------------*/
/*      Find the KEY WORD *cp, and return it's number       */
/*------------------------------------------------------------------*/

/* You can add a function, you MUST place it in alphabetical order  */


#define NKEYS (sizeof mkeywfn / sizeof(struct mkeyw))
struct mkeyw { const char *word; int index;  } mkeywfn[] = {
	  { "!",	k_comment }
	, { "AT",	k_at }
	, { "BLANK",	k_clear }
	, { "CALL",	k_call }
	, { "CLEAR",	k_clear }
	, { "CLOSE",	k_close }
	, { "COPY",	k_copy }
	, { "DATA",	k_data }
	, { "DEL",	k_delete }
	, { "DELETE",	k_delete }
	, { "ELSE",	k_else }
	, { "EXIT",	k_exit }
	, { "FIT", 	k_fit }
	, { "GEN",	k_generate }
	, { "GENERATE",	k_generate }
	, { "GOTO",	k_goto }
	, { "HELP",	k_help }
	, { "IF",	k_if }
	, { "INPUT",	k_input }
	, { "INSERT",	k_insert }
	, { "LET",	k_let }
	, { "LIST",	k_list }
	, { "LOAD",	k_load }
	, { "LOG",	k_logging }
	, { "LOGG",	k_logging }
	, { "LOGGING",	k_logging }
	, { "MOVE",	k_move }
	, { "NEW", 	k_new }
	, { "NEXT",	k_next }
	, { "PARSUM",	k_parsum }
	, { "PRINT",	k_print }
	, { "PROP",	k_prop }
	, { "PROPAGATE",k_prop }
	, { "QUIT",	k_quit }
	, { "SAVE",	k_save }
	, { "SET",	k_set }
	, { "SHELL",	k_shell }
	, { "SORT",	k_sort }
	, { "SUM",	k_sum }
	, { "SWAP",	k_swap }
};

int binsearchk(char *word, struct mkeyw tab[], int n);

void cmd_name(int idx, char **cp) {
	static char *kp;
	static char fail[]="Keyword not found";
	if (kp==NULL) kp = (char*)myallocz(80);
	for (unsigned int i=0;i<NKEYS;i++) {
		if (mkeywfn[i].index==idx) {
			strcpy(kp,mkeywfn[i].word);
			*cp = kp;
			return;
		}
	}
	*cp = &fail[0];
}

void find_mkey(char *cp, int *idx) {
	int i;
	i = binsearchk(cp,mkeywfn,NKEYS);
	if (i==-1) { *idx = 0; return;}
	*idx = mkeywfn[i].index;
}

/*------------------------------------------------------------------*/
/* Simple binary search 					    */
/*------------------------------------------------------------------*/

int binsearchk(char *word, struct mkeyw tab[], int n) {
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
	return -1;
}

/*-------------------------------------------------------------------------*/
/* This is for the tex primitives */

struct mkeyw tkeywfn[] = {
{ 0,0} };

#define NTKEYS (sizeof tkeywfn / sizeof(struct mkeyw))
int find_primcmd(char *cp) {
	int i;
	i = binsearchk(cp,tkeywfn,NTKEYS);
	if (i==-1) return 0;
	return tkeywfn[i].index;
}
