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
#include "file_io.h"
#include "gle-interface/gle-interface.h"
#include "core.h"

typedef double dbl;

//#ifndef unix
#include <stdarg.h>
//#endif
#include "gprint.h"
#include "cutils.h"
char *line(int i);
#define false 0
#define true (!false)

int this_line = -1;
int g_error_col = -1;
int last_line = -1;
extern int trace_on;
bool new_error = true;
extern int ngerror;
void gprint_send(const char* ss);
void gprint_send(const string& output);
void check_new_error();

extern GLEGlobalSource* g_Source;

#if defined(__UNIX__) || defined(__MAC__)
	void gprint(const char* arg_list, ...)
#else
	void gprint(va_list arg_list, ...)
#endif
/* Prints an error message */
{
	va_list arg_ptr;
	char *format;
	char output[1024];
	va_start(arg_ptr, arg_list);
	format = (char*)arg_list;
	vsprintf(output, format, arg_ptr);
	output[1023] = 0;
	check_new_error();
	gprint_send(output);
	g_set_error_column(-1);
}

void gprint(const string& output) {
	check_new_error();
	gprint_send(output);
	g_set_error_column(-1);
}

void check_new_error() {
	if (new_error) {
		ngerror++;
		if (last_line != this_line && this_line != -1 && !trace_on && g_Source != NULL) {
			GLEErrorMessage err;
         int errorLine = this_line-1;
         if (errorLine >= 0 && errorLine < g_Source->getNbLines()) {
            GLESourceLine* line = g_Source->getLine(errorLine);
            err.setLine(line->getLineNo());
            err.setColumn(g_error_col);
            err.setFile(line->getFileName());
            ostringstream ss;
            err.setDelta(line->showLineAbbrev(ss, g_error_col));
            err.setLineAbbrev(ss.str());
         } else {
            // to prevent crash in case of incorrect line number
            err.setLine(this_line);
            err.setColumn(g_error_col);
            ostringstream ss;
            ss << "can't derive source file for internal line #" << this_line;
            err.setLineAbbrev(ss.str());
         }
         GLEInterface* iface = GLEGetInterfacePointer();
         iface->getOutput()->error(&err);
		}
		last_line = this_line;
		new_error = false;
	}
}

void gprint_send(const char *ss) {
	string output = ss;
	gprint_send(output);
}

/* No new error if does not end in \n */
void gprint_send(const string& output) {
	string strg = output;
	string::size_type pos = strg.find('\n');
	while (pos != string::npos) {
		string part = strg.substr(0, pos);
		g_message((char*)part.c_str());
		string::size_type len = strg.length();
		strg = strg.substr(pos+1, len-pos);
		pos = strg.find('\n');
	}
	if (gle_onlyspace(strg)) {
		new_error = true;
	} else {
		g_message((char*)strg.c_str());
	}
}

void reset_new_error(bool val) {
	new_error = val;
}

int get_nb_errors() {
	return ngerror;
}

void inc_nb_errors() {
	ngerror++;
}

void g_set_error_column(int col) {
	g_error_col = col;
}

void g_set_error_line(int lin) {
	this_line = lin;
}

int g_get_error_line() {
	return this_line;
}
