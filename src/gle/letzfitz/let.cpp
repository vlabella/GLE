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
 * Trying to add surface support back into GLE
 * Jan Struyf 2005
 */

#include "../all.h"
#include "../tokens/Tokenizer.h"
#include "../core.h"
#include "../file_io.h"
#include "../texinterface.h"
#include "../cutils.h"
#include "../gprint.h"
#include "../cmdline.h"
#include "../config.h"
#include "../mem_limits.h"
#include "../token.h"
#include "../glearray.h"
#include "../polish.h"
#include "../var.h"
#include "../gprint.h"
#include "../keyword.h"
#include "../run.h"

#define BEGINDEF extern
#include "../begin.h"

double get_next_exp(TOKENS tk, int ntk, int *curtok);
void get_next_exp_file(TOKENS tk, int ntok, int *curtok, string* res);

#define kw(ss) if (ct <= ntk && str_i_equals(tk[ct],ss))
#define next_file_eval(s) get_next_exp_file(tk,ntk,&ct,&s)
#define next_str(s) ct+=1;s=tk[ct]

void get_from_to_step(TOKENS tk, int ntok, int *curtok, double* from, double* to, double* step) throw(ParserError) {
	(*curtok) = (*curtok) + 1;
	if ((*curtok) >= ntok) {
		return;
	}
	if (!str_i_equals(tk[(*curtok)], "FROM")) {
		g_throw_parser_error("expecting 'from' in letz block");
	}
	*from = get_next_exp(tk, ntok, curtok);
	(*curtok) = (*curtok) + 1;
	if ((*curtok) >= ntok) {
		return;
	}
	if (!str_i_equals(tk[(*curtok)], "TO")) {
		g_throw_parser_error("expecting 'to' in letz block");
	}
	*to = get_next_exp(tk, ntok, curtok);
	(*curtok) = (*curtok) + 1;
	if ((*curtok) >= ntok) {
		return;
	}
	if (!str_i_equals(tk[(*curtok)], "STEP")) {
		g_throw_parser_error("expecting 'step' in letz block");
	}
	if (*from >= *to) {
		ostringstream err;
		err << "from value (" << *from << ") should be strictly smaller than to value (" << *to << ") in letz block";
		g_throw_parser_error(err.str());
	}
	*step = get_next_exp(tk, ntok, curtok);
	if (*step <= 0.0) {
		ostringstream err;
		err << "step value (" << *from << ") should be strictly positive in letz block";
		g_throw_parser_error(err.str());
	}
}

void begin_letz(int *pln, GLEPcodeList* pclist, int *pcode, int *cp) throw(ParserError) {
	// Variables
	double xmin = 10, xmax = 10, xstep = 1;
	double ymin = 10, ymax = 10, ystep = 1;
	string equation, data_file;
	// Start with pcode from the next line
	(*pln)++;
	begin_init();
	while (true) {
		int st = begin_token(&pcode,cp,pln,srclin,tk,&ntk,outbuff);
		if (!st) {
			/* exit loop */
			break;
		}
		int ct = 1;
		kw("DATA") {
			next_file_eval(data_file);
		} else kw("Z") {
			ct++;
			next_str(equation);
		} else kw("X") {
			get_from_to_step(tk, ntk, &ct, &xmin, &xmax, &xstep);
		} else kw("Y") {
			get_from_to_step(tk, ntk, &ct, &ymin, &ymax, &ystep);
		} else if (ct <= ntk) {
			stringstream err;
			err << "illegal keyword in letz block: '" << tk[ct] << "'";
			g_throw_parser_error(err.str());
		}
	}
	// Create data file (if modified)
/*
	cout << "Wriging to: " << data_file << endl;
	cout << "Equation:   " << equation << endl;
	cout << "X from " << xmin << " to " << xmax << " step " << xstep << endl;
	cout << "Y from " << ymin << " to " << ymax << " step " << ystep << endl;
*/
	int xvar, yvar, vtype = 1;
	var_findadd("X", &xvar, &vtype);
	var_findadd("Y", &yvar, &vtype);
	token_space();
	/* Polish our equation */
	int rtype = 1;
	GLEPcodeList pc_list;
	GLEPcode evalPcode(&pc_list);
	get_global_polish()->polish(equation.c_str(), evalPcode, &rtype);
	if (get_nb_errors()!=0) {
		return;
	}
	FILE* fp = validate_fopen(data_file, "wb", false);
	int nx = (int)((xmax-xmin)/xstep + 1);
	int ny = (int)((ymax-ymin)/ystep + 1);
	fprintf(fp,"! nx %d ny %d xmin %g xmax %g ymin %g ymax %g \n",nx,ny,xmin,xmax,ymin,ymax);
	GLERC<GLEArrayImpl> stk(new GLEArrayImpl());
	for (double y=ymin, yi=0; yi<ny ; yi++, y+=ystep) {
		for (double x=xmin, xi=0; xi<nx; xi++, x+=xstep) {
			var_set(xvar, x);
			var_set(yvar, y);
			int cp = 0;
			double value = evalDouble(stk.get(), pclist, (int*)&evalPcode[0], &cp);
			fprintf(fp,"%g ",value);
		}
		fprintf(fp,"\n");
	}
	fclose(fp);
}
