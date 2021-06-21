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
#include "justify.h"
#include "core.h"
#include "gprint.h"
#include "mem_limits.h"
#include "token.h"
#include "cutils.h"

#define BEGINDEF extern
#include "begin.h"

#define true (!false)
#define false 0
#define dbg if ((gle_debug & 32)>0)

extern int gle_debug;

void g_textfindend(const string& s, double *x, double *y);
void tab_line(const string& line, stringstream& tab_buf, double swid, const vector<int>& delta);
void tab_line_delta(const string& line, stringstream& tab_buf, vector<int>& delta);

void begin_tab(int *pln, int *pcode, int *cp) {
	vector<int> delta;
	int cjust, save_fnt;
 	double zzz, save_hei, base_owidth;
	(*pln)++;
	string line;
	stringstream tab_buf;
	g_get_font(&save_fnt);
	g_get_hei(&save_hei);
	g_get_just(&cjust);
	g_textfindend("o",&base_owidth,&zzz);
	int save_pln = *pln;
	while (begin_line_norep(pln, line)) {
		tab_line_delta(line, tab_buf, delta);
	}
	*pln = save_pln;
	while (begin_line_norep(pln, line)) {
		tab_line(line, tab_buf, base_owidth, delta);
	}
	// cout << "tab buf '" << tab_buf.str() << "'" << endl;
	g_set_font(save_fnt);
	g_set_hei(save_hei);
	text_block(tab_buf.str(),0.0,cjust);
}

void tab_line_delta(const string& line, stringstream& tab_buf, vector<int>& delta) {
	string::size_type len = line.length();
	string::size_type pos = 0, c = 0;
	while (pos < line.length()) {
		if (line[pos] == 9) {
			c = (c/8)*8+8; pos++;
		} else if (line[pos] == ' ')  {
	  		c++; pos++;
		} else {
			string::size_type savec = c;
			while (delta.size() <= savec) {
				delta.push_back(0);
			}
			int delta_cnt = 0;
			while (pos < len && line[pos] != 9 &&
			       !(pos < len-1 && isspace(line[pos]) && isspace(line[pos+1]))) {
			       if (pos < len-1 && line[pos] == '\\') {
				       	int ch = line[pos+1];
				       	if (gle_isalphanum(ch)) {
						delta_cnt++; pos++; c++;
						while (pos < len && gle_isalphanum(line[pos])) {
							pos++; c++;
							delta_cnt++;
						}
						if (pos < len && line[pos] == '{') {
							int old_pos = pos;
							pos = str_skip_brackets(line, pos, '{', '}');
							delta_cnt += pos - old_pos + 1;
							c += pos - old_pos + 1;
						}
					} else {
						if (strchr("{}_$", ch) != NULL) {
							delta_cnt++;
						} else {
							delta_cnt += 2;
						}
						pos++; c++;
					}
			       } else {
			       		pos++; c++;
			       }
			}
			// cout << "delta_cnt = " << delta_cnt << endl;
			if (delta[savec] < delta_cnt) delta[savec] = delta_cnt;
		}
	}
}

void tab_line(const string& line, stringstream& tab_buf, double swid, const vector<int>& delta) {
	int len = line.length();
	bool is_modified = false;
	int pos = 0, c = 0, diff = 0;
	// cout << "line = '" << line << "'" << endl;
	while (pos < (int)line.length()) {
		if (line[pos] == 9) {
			c = (c/8)*8+8; pos++;
		} else if (line[pos] == ' ')  {
	  		c++; pos++;
		} else {
			int savec = c;
			string next_line;
			while (pos < len && line[pos] != 9 &&
			       !(pos < len-1 && isspace(line[pos]) && isspace(line[pos+1]))) {
				next_line += line[pos++]; c++;
			}
			double br,bd;
			replace_exp(next_line);
			// cout << "'" << next_line << "': " << savec << endl;
			g_textfindend(next_line,&br,&bd);
			int real_c = savec - diff;
			tab_buf << "\\movexy{" << (swid*real_c) << "}{0}";
			tab_buf << next_line;
			tab_buf << "\\movexy{" << (-br-swid*real_c) << "}{0}";
			is_modified = true;
			diff += savec < (int)delta.size() ? delta[savec] : 0;
			diff ++;
			// cout << "savec = " << savec << " delta = " << delta[savec] << " diff = " << diff << endl;
		}
	}
	if (!is_modified) tab_buf << "\\movexy{0}{0}";
	tab_buf << endl;
}
