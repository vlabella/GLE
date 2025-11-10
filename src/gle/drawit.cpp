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

#include "all.h"
#include "tokens/stokenizer.h"
#include "tokens/Tokenizer.h"
#include "mem_limits.h"
#include "token.h"
#include "core.h"
#include "var.h"
#include "glearray.h"
#include "polish.h"
#include "pass.h"
#include "file_io.h"
#include "gprint.h"
#include "cmdline.h"
#include "config.h"
#include "drawit.h"
#include "cutils.h"
#include "gle-interface/gle-interface.h"
#include "keyword.h"
#include "run.h"
#include <boost/math/constants/constants.hpp>
#include "gle-constants.h"

using namespace std;

#define dbg if (gle_debug>0)
extern int gle_debug;
extern int this_line;
int trace_on;
int **gpcode;   /* gpcode is a pointer to an array of pointer to int */
int *gplen;     /* gpcode is a pointer to an array of int */
int ngpcode=0;
int ngerror;
extern int last_line;
char *dr_nextline(int *srclin);
void dr_init(void);
int getch(void);
int abort_flag;
GLEGlobalSource* g_Source = NULL;
void g_close();
int gle_is_open();
GLERun* g_GLERun = NULL;

extern CmdLineObj g_CmdLine;

GLERun* getGLERunInstance() {
	return g_GLERun;
}

void text_load_include(GLEParser* parser, const string& fname, GLESourceLine* code, GLESourceFile* file);

void gle_set_constants(){
	GLEMemoryCell value;
	GLE_MC_INIT(value);
	GLE_MC_SET_BOOL(&value, true);
	var_findadd_set("TRUE", &value);
	GLE_MC_SET_BOOL(&value, false);
	var_findadd_set("FALSE", &value);
	var_findadd_set("PI",boost::math::double_constants::pi);
	gle_set_math_and_physical_constants();
}

void do_set_vars() {
	var_findadd_set("XGMIN", 0.0);
	var_findadd_set("YGMIN", 0.0);
	var_findadd_set("XGMAX", 0.0);
	var_findadd_set("YGMAX", 0.0);
	var_findadd_set("X2GMIN", 0.0);
	var_findadd_set("Y2GMIN", 0.0);
	var_findadd_set("X2GMAX", 0.0);
	var_findadd_set("Y2GMAX", 0.0);
	var_findadd_set("ZGMIN", 0.0);
	var_findadd_set("ZGMAX", 0.0);
	gle_set_constants();
}

/*---------------------------------------------------------------------------*/

void output_error(ParserError& err) {
	g_set_error_column(-1);
	if (err.hasFlag(TOK_PARSER_ERROR_ATEND)) {
		// Otherwise, the message would be "unexpected end of file"
		err.setMessage("unexpected end of line");
	}
	if (err.hasFlag(TOK_PARSER_ERROR_PSTRING)) {
		if (err.file() == "") {
			gprint(string(">> Error: ")+err.msg()+"\n");
		} else {
			string err_str;
			err.toString(err_str);
			gprint(string(">> Error: ")+err_str+"\n");
		}
		if (err.getColumn() != -1) {
			stringstream pos_strm;
			pos_strm << ">> In: '";
			int delta = showLineAbbrev(err.getParserString(), err.getColumn(), pos_strm);
			pos_strm << "'" << endl;
			pos_strm << ">>";
			for (int i = 0; i < err.getColumn()+5-delta; i++) {
				pos_strm << " ";
			}
			pos_strm << "^" << endl;
			gprint(pos_strm.str());
		}
	} else {
		if (err.file() == "") {
			// Error in the GLE file
			g_set_error_column(err.getColumn());
			gprint(string(">> Error: ")+err.msg()+"\n");
		} else {
			// Error while reading a data file
			string err_str;
			err.toString(err_str);
			gprint(string(">> Error: ")+err_str+"\n");
		}
	}
}

void output_error_cerr(ParserError& err) {
	// Used by GLE "as a calculator", i.e., if no GLE file is active
	if (err.hasFlag(TOK_PARSER_ERROR_ATEND)) {
		// Otherwise, the message would be "unexpected end of file"
		err.setMessage("unexpected end of line");
	}
	if (err.hasFlag(TOK_PARSER_ERROR_PSTRING)) {
		cerr << ">> Error: " << err.msg() << endl;
		if (err.getColumn() != -1) {
			cerr << ">> In: '" << err.getParserString() << "'" << endl;
			stringstream pos_strm;
			pos_strm << ">>";
			for (int i = 0; i < err.getColumn()+5; i++) {
				pos_strm << " ";
			}
			pos_strm << "^" << endl;
			cerr << pos_strm.str();
		}
	} else {
		cerr << ">> Error: " << err.msg() << endl;
	}
}

void validate_open_input_stream(ifstream& input, const string& fname) {
	string expanded(GLEExpandEnvironmentVariables(fname));
	validate_file_name(expanded, true);
	input.open(expanded.c_str());
	if (!input.is_open()) {
		g_throw_parser_error_sys("unable to open file '", expanded.c_str(), "'");
	}
}

FILE* validate_fopen(const string& fname, const char *mode, bool isread) {
	string expanded(GLEExpandEnvironmentVariables(fname));
	validate_file_name(expanded, isread);
	FILE* result = fopen(expanded.c_str(), mode);
	if (result == NULL) {
		if (isread) g_throw_parser_error_sys("unable to open file '", expanded.c_str(), "'");
		else  g_throw_parser_error_sys("unable to create file '", expanded.c_str(), "'");
	}
	return result;
}

void validate_file_name(const string& fname, bool isread) {
	GLEInterface* iface = GLEGetInterfacePointer();
	if (iface->hasFileInfos()) {
		GLEFileLocation finfo;
		finfo.fromFileNameCrDir(fname);
		iface->addFileInfo(finfo);
	}
	GLEGlobalConfig* conf = iface->getConfig();
	if (conf->getCmdLine()->hasOption(GLE_OPT_SAFEMODE)) {
		bool allow = false;
		string fullpath, dirname;
		GLEGetCrDir(&dirname);
		GLEGetFullPath(dirname, fname, fullpath);
		// cout << "full path: " << fullpath << endl;
		GetDirName(fullpath, dirname);
		StripDirSepButNotRoot(dirname);
		int nbread = conf->getNumberAllowReadDirs();
		if (nbread > 0 && isread) {
			for (int i = 0; i < nbread; i++) {
				if (conf->getAllowReadDir(i) == dirname) {
					allow = true;
				}
			}
			if (allow) {
				return;
			} else {
				g_throw_parser_error("safe mode - reading not allowed in directory '", dirname.c_str(), "'");
			}
		}
		int nbwrite = conf->getNumberAllowWriteDirs();
		if (nbwrite > 0 && !isread) {
			for (int i = 0; i < nbwrite; i++) {
				if (conf->getAllowWriteDir(i) == dirname) {
					allow = true;
				}
			}
			if (allow) {
				return;
			} else {
				g_throw_parser_error("safe mode - writing not allowed in directory '", dirname.c_str(), "'");
			}
		}
		g_throw_parser_error("safe mode - can not access '", fname.c_str(), "': file system access has been disabled");
	}
}

void DrawIt(GLEScript* script, GLEFileLocation* outfile, CmdLineObj* cmdline, bool silent) {
	GLEGlobalSource* glecode = script->getSource();
	GLEInterface* iface = script->getGLEInterface();
	script->cleanUp();
	g_Source = glecode;
	abort_flag = false;
	ngerror = 0;
	last_line = 0;
	if (!silent && g_verbosity() > 0) {
		string version;
		g_get_version(&version);
		string GLE = "GLE";
		string colorized_GLE = "\033[1;31mG\033[1;32mL\033[1;34mE\033[0m";  // red green blue
		cerr << colorized_GLE << " " << version << "[" << script->getLocation()->getName() << "]-C";
		cerr.flush();
		g_set_console_output(false);
	}
	g_clear();
	var_clear();
	mark_clear();
	sub_clear(iface->isCommitMode());
	clear_run();
	f_init();
	if (cmdline != NULL) {
		int devtype = g_get_device();
		if (devtype == GLE_DEVICE_PS) {
			g_set_fullpage(true);
		} else {
			g_set_fullpage(cmdline->hasOption(GLE_OPT_FULL_PAGE));
		}
		g_set_rotate_fullpage(cmdline->hasOption(GLE_OPT_LANDSCAPE));
	}
	g_set_pagesize(gle_config_papersize());
	g_set_margins(gle_config_margins());
	do_set_vars();
	GLEPcodeList* pc_list = new GLEPcodeList();
	GLEPcodeIndexed* pcode = new GLEPcodeIndexed(pc_list);
	script->setPcode(pcode);
	GLEPolish* polish = new GLEPolish();
	script->setPolish(polish);
	polish->initTokenizer();
	// Create tokenizer
	GLEParser* parser = new GLEParser(script, polish);
	script->setParser(parser);
	parser->initTokenizer();
	try {
		// Auto-include "compatibility.gle" if compatibility mode is not most recent
		string compat = "compatibility.gle";
		if ((g_get_compatibility() < GLE_COMPAT_MOST_RECENT) && !glecode->includes(compat)) {
			GLESourceFile* includefile = new GLESourceFile();
			text_load_include(parser, compat, NULL, includefile);
			glecode->insertIncludeNoOverwrite(0, includefile);
		}
		set_global_parser(parser);
		pcode->addIndex(pcode->size());
		for (int crline = 0; crline < glecode->getNbLines(); crline++) {
			// call passt to convert the tokens into PCode
			// pcode is simply numbers that instruct the drawing
			// engine which commands to draw
			// cout << "Line = " << *crline;

			int prevSize = pcode->size();
			GLESourceLine* code = glecode->getLine(crline);
			parser->setString(code->getCodeCStr());
			try {
				parser->passt(*code, *pcode);
			} catch (ParserError& err) {
				output_error(err);
			}
			bool add_pcode = true;
			if (parser->hasSpecial(GLE_PARSER_INCLUDE) && !glecode->includes(parser->getInclude())) {
				// this is a file to include load the file and insert it into glecode here
				GLESourceFile* includefile = new GLESourceFile();
				text_load_include(parser, parser->getInclude(), code, includefile);
				glecode->insertInclude(crline, includefile);
				crline--;
				if (g_verbosity() > 5) {
					cerr << "{" << parser->getInclude() << "}";
				}
				// don't need this pcode
				add_pcode = false;
			}
			if (add_pcode) {
				pcode->addIndex(pcode->size());
			} else {
				pcode->resize(prevSize);
			}
		}
		parser->checkmode();
		/* build global pcode index */
		ngpcode = pcode->getNbEntries()-1;
		if (gpcode != NULL) free(gpcode);
		if (gplen != NULL) free(gplen);
		gpcode = (int**)malloc(sizeof(int*)*(ngpcode+1));
		gplen = (int*)malloc(sizeof(int)*(ngpcode+1));
		for (int i = 0; i < ngpcode; i++) {
			gplen[i+1] = pcode->getSize(i);
			gpcode[i+1] = &(*pcode)[0] + pcode->getIndex(i);
		}
	} catch (ParserError& err) {
		output_error(err);
	}
	// Create GLERun
	GLERun* run = new GLERun(script, outfile, pcode);
	run->setBlockTypes(parser->getBlockTypes());
	script->setRun(run);
	g_GLERun = run;
	g_compatibility_settings();
	// Exit if parser error (not: this should be done after GLERun has been created!)
	if (ngerror > 0){
		reset_new_error(true);
		g_message("");
		g_throw_parser_error("errors, aborting");
	}
	try {
		//
		// -- now run the pcode in the driver
		//
		if (!silent && g_verbosity() > 0) cerr << "-R-";
		if (ngpcode != glecode->getNbLines()) {
			cerr << "error pcode and text size mismatch"<<endl;
			cerr << "pcode size = " << ngpcode <<" text size = "<<glecode->getNbLines()<<endl;
		}
		token_space();
		int endp = 0;
		// should get rid of glecode here!
		bool mkdrobjs = iface->isMakeDrawObjects();
		for (int i=1;i<=ngpcode;i++){
			this_line = i;
			GLESourceLine* code = glecode->getLine(i-1);
			run->do_pcode(*code,&i,gpcode[i],gplen[i],&endp,mkdrobjs);
		}
	} catch (ParserError& err) {
		output_error(err);
	}
	if (!gle_is_open()) {
		// If .gle file is empty, create empty postscript file
		if (!g_has_size()) g_set_size(10, 10, false);
		g_open(outfile, glecode->getLocation()->getName());
	}
	bool has_console = g_reset_message();
	g_close();
	g_set_console_output(has_console);
}

void text_load_include(GLEParser* parser, const string& fname, GLESourceLine* code, GLESourceFile* file) {
	GLEFileLocation* loc = file->getLocation();
	loc->setName(fname);
	const string* dirname = NULL;
	if (code != NULL) {
		dirname = &(code->getSource()->getLocation()->getDirectory());
	}
	ifstream input;
	string expanded(GLEExpandEnvironmentVariables(fname));
	const string fullname(GetActualFilename(&input, expanded, dirname));
	if (fullname == "") {
		ostringstream errs;
		errs << "include file not found: '" << expanded << "'";
		throw parser->error(errs.str());
	} else {
		loc->setFullPath(fullname);
		loc->initDirectory();
		validate_file_name(fullname, true);
		file->load(input);
		input.close();
		file->trim(0);
	}
}
