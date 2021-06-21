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

#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

#include "../gle/basicconf.h"
#include "../gle/cutils.h"
#include "../gle/file_io.h"
#include "../gle/tokens/stokenizer.h"

void GLEFindRelPath(const string& p1, const string& p2, string* result, int* nb) {
	char path = '/';
	#ifdef __WIN32__
		path = '\\';
	#endif
	char spliton[2];
	spliton[0] = path;
	spliton[1] = 0;
	string nxt1;
	string nxt2;
	char_separator separator(spliton, "");
	tokenizer<char_separator> tok1(p1, separator);
	tokenizer<char_separator> tok2(p2, separator);
	bool more1 = tok1.has_more();
	bool more2 = tok2.has_more();
	while (more1 && more2) {
		nxt1 = tok1.next_token();
		nxt2 = tok2.next_token();
		if (!str_i_equals(nxt1, nxt2)) break;
		more1 = tok1.has_more();
		more2 = tok2.has_more();
	}
	string rpath;
	int nb_back = 0;
	if (str_i_equals(nxt1, nxt2)) {
		int pos = 0;
		while (tok2.has_more()) {
			if (pos != 0) rpath += spliton;
			rpath += tok2.next_token();
			pos++;
		}
	} else {
		rpath = nxt2;
		while (tok2.has_more()) {
			rpath += spliton;
			rpath += tok2.next_token();
		}
		if (more1) {
			nb_back++;
			while (tok1.has_more()) {
				tok1.next_token();
				nb_back++;
			}
		}
	}
	*result = rpath;
	*nb = nb_back;
}

int do_setrelpath(char** argv, int num_args) {
	string bin    = "";
	string lib    = "";
	string data   = "";
	string doc    = "";
	string config = "";


	if(num_args >= 1) bin = argv[0];
	if(num_args >= 2) lib = argv[1];
	if(num_args >= 3) data = argv[2];
	if(num_args >= 4) doc = argv[3];
	if(num_args >= 5) config = argv[4];

	cout << "GLE will be installed as follows:" << endl;
	cout << "Binary:   \"" << bin << "\"" << endl;
	cout << "Library:  \"" << lib << "\"" << endl;
	/* LIB dir */
	string rel_lib;
	int rel_lib_nb;
	GLEFindRelPath(bin, lib, &rel_lib, &rel_lib_nb);
	cout << "          (Up: " << rel_lib_nb << ", Relative: \"" << rel_lib << "\")" << endl;
	/* GLE_TOP */
	string rel_data;
	int rel_data_nb;
	GLEFindRelPath(bin, data, &rel_data, &rel_data_nb);
	cout << "GLE_TOP:  \"" << data << "\"" << endl;
	cout << "          (Up: " << rel_data_nb << ", Relative: \"" << rel_data << "\")" << endl;
	/* DOC dir */
	string rel_doc;
	int rel_doc_nb;
	GLEFindRelPath(data, doc, &rel_doc, &rel_doc_nb);
	cout << "DOC:      \"" << doc << "\"" << endl;
	cout << "          (Up: " << rel_doc_nb << ", Relative: \"" << rel_doc << "\")" << endl;
	/* Create config.h */
	vector<string> lines;
	ifstream ifile(config.c_str());
	if (!ifile.good()) {
		cout << "Can't open: '" << config << "'" << endl;
		return -1;
	}
	while (!ifile.eof()) {
		string line;
		ReadFileLine(ifile, line);
		if (str_i_str(line.c_str(), "[SETRELPATH]")) {
			break;
		}
		lines.push_back(line);
	}
	ifile.close();
	ofstream ofile(config.c_str());
	for (vector<string>::size_type i = 0; i < lines.size(); i++) {
		ofile << lines[i] << endl;
	}
	ofile << endl;
	ofile << "/* [SETRELPATH]: The following lines have been added by 'build/bin/setrelpath' */" << endl;
	ofile << "#define GLETOP_CD " << rel_data_nb << endl;
	ofile << "#define GLETOP_REL \"" << rel_data << "\"" << endl;
	ofile << "#define GLETOP_ABS \"" << data << "\"" << endl;
	ofile << endl;
	ofile << "#define GLELIB_CD " << rel_lib_nb << endl;
	ofile << "#define GLELIB_REL \"" << rel_lib << "\"" << endl;
	ofile << "#define GLELIB_ABS \"" << lib << "\"" << endl;
	ofile << endl;
	ofile << "#define GLEDOC_CD " << rel_doc_nb << endl;
	ofile << "#define GLEDOC_REL \"" << rel_doc << "\"" << endl;
	ofile << "#define GLEDOC_ABS \"" << doc << "\"" << endl;
	ofile << endl;
	ofile.close();
	return 0;
}

int do_latexdef(char** argv, int num_args) {
	string fname = "";
	if(num_args >= 1) fname = argv[0];
	string def = "";
	if(num_args >= 2) def = argv[1];
	string value = "";
	if(num_args >= 3) value = argv[2];
	ofstream ofile(fname.c_str());
	ofile << "\\newcommand{\\" << def << "}[1]{";
	if (value == "1") {
		ofile << "#1";
	}
	ofile << "}" << endl;
	ofile.close();
	return 0;
}

int do_latex_gle_version(char** argv, int num_args) {
	string fname = "";
	if(num_args >= 1) fname = argv[0];
	if(fname == "") return 0;
	ofstream ofile(fname.c_str(), ios::app);
	ofile << "\\newcommand{\\gleversion}{" << GLEVN << "}" << std::endl;
	ofile.close();
	return 0;
}

int main(int argc, char** argv) {
	string option = argv[1];
	cout << option;
	if (option == "-setrelpath") {
#ifndef __WIN32__
		return do_setrelpath(argv + 2, argc-2);
#else
		return 0;
#endif
	} else if (option == "-latexdef") {
		return do_latexdef(argv + 2,argc-2);
	} else if (option == "-latexversion") {
		return do_latex_gle_version(argv + 2,argc-2);
	}
	return -1;
}
