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

#define BEGINDEF

#include "all.h"
#include "mem_limits.h"
#include "gle-interface/gle-interface.h"
#include "glearray.h"
#include "token.h"
#include "begin.h"
#include "gprint.h"
#include "cutils.h"
#include "polish.h"

void replace_exp(char* exp);
void replace_exp(string& exp);

double token_next_double(int i) throw(ParserError) {
	char* tok = tk[i];
	if (!is_float(tok)) {
		stringstream err;
		err << "floating point number expected, but found: '" << tok << "'";
		g_throw_parser_error(err.str());
	}
	return atof(tok);
}

int begin_token(int **pcode,int *cp,int *pln,char *srclin,TOKENS tk,int *ntk,char *outbuff) {
	g_set_error_line(*pln);
	(*pcode) = gpcode[(*pln)++];
	if ((*pcode)[1] != 5 || (*pcode)[2] == 0) {
		(*pln)--;
		return false;
	}
	strcpy(srclin,(char *) ((*pcode)+3));
	replace_exp(srclin);
	for (int i = 0; i < TOKEN_LENGTH; i++) {
		strcpy(tk[i], " ");
	}
	token(srclin,tk,ntk,outbuff);
	return true;
}

int begin_token(GLESourceLine& sline, char *srclin, TOKENS tk, int *ntk, char *outbuff, bool replaceExpr) {
	g_set_error_line(sline.getGlobalLineNo());
	strcpy(srclin, sline.getCodeCStr());
	if (replaceExpr) {
		replace_exp(srclin);
	}
	for (int i = 0; i < TOKEN_LENGTH; i++) {
		strcpy(tk[i], " ");
	}
	token(srclin,tk,ntk,outbuff);
	return true;
}

bool begin_line(int *pln, string& srclin) {
	g_set_error_line(*pln);
	int *pcode = gpcode[(*pln)++];
	if (pcode[1] != 5 || pcode[2] == 0) {
		(*pln)--;
		return false;
	}
	srclin = (char *)(pcode+3);
	replace_exp(srclin);
	return true;
}

bool get_block_line(int pln, string& srclin) {
	g_set_error_line(pln);
	int *pcode = gpcode[pln];
	if (pcode[1] != 5 || pcode[2] == 0) {
		srclin = "";
		return false;
	}
	srclin = (char *)(pcode+3);
	replace_exp(srclin);
	return true;
}

bool begin_line_norep(int *pln, string& srclin) {
	int *pcode = gpcode[(*pln)++];
	if (pcode[1] != 5 || pcode[2] == 0) {
		(*pln)--;
		return false;
	}
	srclin = (char *)(pcode+3);
	return true;
}

void begin_init() {
	strcpy(space_str," \0");
	for (int i = 0; i < TOKEN_LENGTH; i++) {
		strcpy(tk[i], " ");
	}
}

int begin_next_line(int *pcode, int *cp) {
	return true;
}

// replaces \expr{some_exp} by the result of evaluating some_exp
void replace_exp(char* exp) {
	char *ptr = str_i_str(exp, "\\EXPR{");
	while (ptr != NULL) {
		int depth = 0;
		int first = ptr-exp;
		int pos = first+6; // 6 = strlen("\\EXPR{")
		char ch = exp[pos];
		string to_eval = "", eval_res;
		while (ch != 0 && (ch != '}' || depth > 0)) {
			if (ch == '{') depth++;
			else if (ch == '}') depth--;
			if (ch != 0 && (ch != '}' || depth > 0)) {
				to_eval += ch;
				ch = exp[++pos];
			}
		}
		polish_eval_string(to_eval.c_str(), &eval_res, true);
		string at_end = exp+pos+1;
		exp[first] = 0;
		strcat(exp, eval_res.c_str());
		strcat(exp, at_end.c_str());
		ptr = str_i_str(exp, "\\EXPR{");
	}
}

// replaces \expr{some_exp} by the result of evaluating some_exp
void replace_exp(string& exp) {
	int ptr = str_i_str(exp, "\\EXPR{");
	while (ptr != -1) {
		int depth = 0;
		int pos = ptr+6; // 6 = strlen("\\EXPR{")
		char ch = exp[pos];
		int len = exp.length();
		string to_eval = "", eval_res;
		while (pos < len && (ch != '}' || depth > 0)) {
			if (ch == '{') depth++;
			else if (ch == '}') depth--;
			if (ch != 0 && (ch != '}' || depth > 0)) {
				to_eval += ch; pos++;
				ch = pos < len ? exp[pos] : 0;
			}
		}
		polish_eval_string(to_eval.c_str(), &eval_res, true);
		exp.erase(ptr, pos-ptr+1);
		exp.insert(ptr, eval_res);
		ptr = str_i_str(exp, "\\EXPR{");
	}
}
