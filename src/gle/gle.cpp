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

#define GLE_MAIN_CPP

#include "all.h"
#include "tokens/Tokenizer.h"
#include "core.h"
#include "mem_limits.h"
#include "file_io.h"
#include "cutils.h"
#include "cmdline.h"
#include "texinterface.h"
#include "config.h"
#include "drawit.h"
#include "gle-interface/gle-interface.h"
#include "gle-poppler.h"

#ifdef __WIN32__
	#include <io.h>
#endif

#include <set>

/*------------ GLOBAL VARIABLES --------------*/
//
// -- these globals are needed. they are called in other source files using extern...
//
int gle_debug;
bool control_d = true;
extern int trace_on;
bool IS_INSTALL = false;
bool GS_PREVIEW = false;

void do_find_deps(CmdLineObj& cmdline);
void gle_as_a_calculator(vector<string>* exprs);

extern string GLE_TOP_DIR;
extern string GLE_BIN_DIR;
string GLE_WORKING_DIR;

class GLEOptions {
public:
	bool ASK_DEBUG;
};

/* Better to put all global options in one class? */
GLEOptions g_Options;
CmdLineObj g_CmdLine;
ConfigCollection g_Config;

void load_one_file(const char* name, CmdLineObj& cmdline, size_t* exit_code);
void load_one_file_stdin(CmdLineObj& cmdline, size_t* exit_code);
void init_option_args(CmdLineObj& cmdline);
void do_gen_inittex(CmdLineObj& cmdline, GLEOptions& options) throw(ParserError);
void process_option_args(CmdLineObj& cmdline, GLEOptions& options);
void do_run_other_version(ConfigCollection& coll, int argc, char **argv);
void gle_do_socket(const string& commands);
void gle_cleanup();
void tex_term();
void do_show_info();
void get_out_name(const string& name, CmdLineObj& cmdline, GLEFileLocation* outname);

#include "glearray.h"

void gle_cat_csv(vector<string>* files) {
	for (unsigned int i = 0; i < files->size(); i++) {
		string fname(files->at(i));
		GLECSVData csvData;
		csvData.read(fname);
		GLECSVError* error = csvData.getError();
		if (error->errorCode != GLECSVErrorNone) {
			cout << "error: " << error->errorString << endl;
		} else {
			csvData.print(cout);
		}
	}
}

#if defined(HAVE_LIBGLE) || defined(HAVE_LIBGLE_STATIC)
int GLEMain(int argc, char **argv) {
#else
int main(int argc, char **argv) {
#endif
	try {
      gle_glib_init(argc, argv);
		g_init();

		/* Init and process command line arguments */
		init_config(&g_Config);
		init_option_args(g_CmdLine);
		if (!do_load_config("gle", argv, g_CmdLine, g_Config)) {
			return -1;
		}
		do_run_other_version(g_Config, argc, argv);
		g_CmdLine.parse(argc, argv);

		/* Error in command line arguments? */
		if (g_CmdLine.hasError()) {
			return -1;
		}

		/* Has calculator option? */
		if (g_CmdLine.hasOption(GLE_OPT_CALC)) {
			if (g_CmdLine.getNbMainArgs() == 0) {
				gle_as_a_calculator(NULL);
			} else {
				gle_as_a_calculator(g_CmdLine.getMainArgs());
			}
			return 0;
		}

		/* Cat CSV file to stdout? */
		if (g_CmdLine.hasOption(GLE_OPT_CATCSV) && g_CmdLine.getNbMainArgs() != 0) {
			gle_cat_csv(g_CmdLine.getMainArgs());
			return 0;
		}

		/* Should regenerate inittex.ini? */
		do_gen_inittex(g_CmdLine, g_Options);

		/* Find dependencies */
		do_find_deps(g_CmdLine);

		/* Get working directory */
		GLEGetCrDir(&GLE_WORKING_DIR);

		/* This is used by the GUI */
		if (g_CmdLine.hasOption(GLE_OPT_INFO)) {
			do_show_info();
		}

		if (g_CmdLine.getNbMainArgs() == 0 || g_CmdLine.hasOption(GLE_OPT_HELP)) {
			string version;
			g_get_version(&version);
			cerr << "GLE version " << version << endl;
			cerr << "Usage: gle [options] filename.gle" << endl;
			cerr << "More information: gle " << g_CmdLine.getOptionPrefix() << "help" << endl;
			if (g_CmdLine.hasOption(GLE_OPT_HELP)) {
				g_CmdLine.showHelp(GLE_OPT_HELP);
				if (!g_CmdLine.getOption(GLE_OPT_HELP)->hasArgument()) {
					cerr << "Give more help about a given option: " << g_CmdLine.getOptionPrefix() << "help option" << endl;
				}
			}
			return 0;
		}

		/* Process optional arguments */
		process_option_args(g_CmdLine, g_Options);
	} catch (ParserError& err) {
		ostringstream err_str;
		err_str << "Error: ";
		err.write(err_str);
		g_message(err_str.str());
		return -1;
	}

	/* iterate over "file" arguments */
	size_t exit_code = 0;
	if (g_CmdLine.hasStdin()) {
		// read file from standard input (in that case there are no other main arguments)
		load_one_file_stdin(g_CmdLine, &exit_code);
	}
	for (int i = 0; i < g_CmdLine.getNbMainArgs(); i++) {
	  // the standard CMD shell didn't expand the wildcards
#ifdef __WIN32__
		//
		// -- globbing and wildcards
		//
		_finddata_t fileinfo;
		string file_dir, file_pattern;
		SplitFileName(g_CmdLine.getMainArg(i), file_dir, file_pattern);
		intptr_t File = _findfirst((char*)g_CmdLine.getMainArg(i).c_str(), &fileinfo);
		if (File != -1){
			do {
				string combined_name = file_dir + fileinfo.name;
				load_one_file(combined_name.c_str(), g_CmdLine, &exit_code);
			} while(!_findnext(File,&fileinfo));
			_findclose(File);
		} else {
			exit_code = 1;
			cout << "Can't open file: " << g_CmdLine.getMainArg(i) << endl;
		}
#else
#if defined(__OS2__) && defined(__EMX__)
		char **list;
		list = _fnexplode((char*)g_CmdLine.getMainArg(i).c_str());
		if ( list == NULL ) {
			load_one_file( g_CmdLine.getMainArg(i).c_str(), g_CmdLine, &exit_code);
		} else {
			for ( int i = 0; list[i] != NULL; i++ ) {
				load_one_file(list[i], g_CmdLine, &exit_code);
			}
		}
		_fnexplodefree(list);
#else
		// on Unix the shell expands the wildcards for us
		load_one_file(g_CmdLine.getMainArg(i).c_str(), g_CmdLine, &exit_code);
#endif
#endif
	}
	// will exit with the number of files it could NOT open
	gle_cleanup();
	if (exit_code != 0 || g_has_console_output()) {
		do_wait_for_enter();
	}
	return exit_code;
}

void graph_init();

void gle_cleanup() {
	tex_term();
	graph_init();
	delete GLEGetColorList();
	delete GLEGetInterfacePointer();
}

void do_run_other_version(ConfigCollection& coll, int argc, char **argv) {
	// Scan command line for -v option
	// Note: can not use cmdline here because older versions might have different options
	// which would result in an error message
	string version = "";
	for (int i = 1; i < argc-1; i++) {
		if (cmdline_is_option(argv[i], "v") ||
		    cmdline_is_option(argv[i], "version")) {
			version = argv[i+1];
			str_remove_quote(version);
		}
	}
	if (version != "") {
		ConfigSection* gle = coll.getSection(GLE_CONFIG_GLE);
		CmdLineArgSPairList* installs = (CmdLineArgSPairList*)gle->getOption(GLE_CONFIG_GLE_INSTALL)->getArg(0);
		const string* path = installs->lookup(version);
		if (path != NULL) {
			GLESetGLETop(*path);
			ostringstream torun;
			torun << "\"" << (*path) << "\"";
			for (int i = 1; i < argc; i++) {
				string arg = argv[i];
				str_remove_quote(arg);
				if (cmdline_is_option(arg.c_str(), "v")) i++;
				else torun << " \"" << arg << "\"";
			}
			int res = GLESystem(torun.str());
			if (res != GLE_SYSTEM_OK) {
				cerr << "Error while running: " << (*path) << endl;
			}
		} else {
			cerr << "Don't know path for version: '" << version << "'" << endl;
		}
		exit(0);
	}
}

void do_show_info() {
	string version, bdate;
	g_get_version(&version);
	g_get_build_date(&bdate);
	cout << "GLE version:             " << version << endl;
	if (!bdate.empty()) {
		cout << "Build date:              " << bdate << endl;
	}
	cout << "GLE_TOP:                 " << GLE_TOP_DIR << endl;
	cout << "GLE_BIN:                 " << GLE_BIN_DIR << endl;
	/* Location of GhostScript */
	string gs_dir;
	ConfigSection* tools = g_Config.getSection(GLE_CONFIG_TOOLS);
	const string& gs_cmd = ((CmdLineArgString*)tools->getOptionValue(GLE_TOOL_GHOSTSCRIPT_CMD))->getValue();
	if (gs_cmd != "") {
		cout << "GhostScript:             " << gs_cmd << endl;
	}
	CmdLineArgString* gslib = (CmdLineArgString*)tools->getOptionValue(GLE_TOOL_GHOSTSCRIPT_LIB);
	if (!gslib->isDefault()) {
		cout << "GS library:              " << gslib->getValue() << endl;
	}
	/* Bitmap import */
	cout << "Bitmap import:           " << g_bitmap_supported_types() << endl;
	/* Cairo support */
#ifdef HAVE_CAIRO
	cout << "Cairo rendering support: Yes" << endl;
#else
	cout << "Cairo rendering support: No" << endl;
#endif
#ifdef HAVE_POPPLER
	cout << "Poppler PDF support:     Yes" << endl;
#else
	cout << "Poppler PDF support:     No" << endl;
#endif
	do_wait_for_enter_exit(0);
}

void do_gen_inittex(CmdLineObj& cmdline, GLEOptions& options) throw(ParserError) {
	// Generate inittex.ini from init.tex
	if (cmdline.hasOption(GLE_OPT_MKINITTEX)) {
		IS_INSTALL = 1;
		string inittex = GLE_TOP_DIR + DIR_SEP + "init.tex";
		GLERC<GLEScript> script = new GLEScript();
		script->getLocation()->fromFileNameCrDir(inittex);
		script->getSource()->load();
		string inittex_ini = GLE_TOP_DIR + DIR_SEP + "inittex.ini";
		TryDeleteFile(inittex_ini);
		g_select_device(GLE_DEVICE_DUMMY);
		GLEFileLocation output;
		output.createIllegal();
		DrawIt(script.get(), &output, &cmdline);
		exit(0);
	}
}

void process_option_args(CmdLineObj& cmdline, GLEOptions& options) {
	if (cmdline.hasOption(GLE_OPT_COMPAT)) {
		const string& compat = cmdline.getStringValue(GLE_OPT_COMPAT);
		g_set_compatibility(compat);
	} else {
		g_set_compatibility(GLE_COMPAT_MOST_RECENT);
	}
	trace_on = cmdline.hasOption(GLE_OPT_TRACE);
	options.ASK_DEBUG = cmdline.hasOption(GLE_OPT_DEBUG);
	control_d = !cmdline.hasOption(GLE_OPT_NO_CTRL_D);
	if (cmdline.hasOption(GLE_OPT_NO_MAXPATH)) {
		setMaxPSVector(-1);
	}
	if (cmdline.hasOption(GLE_OPT_BBTWEAK)) g_psbbtweak();
	GS_PREVIEW = cmdline.hasOption(GLE_OPT_GSPREVIEW);
	// .ps output implies full page
	CmdLineArgSet* device = (CmdLineArgSet*)cmdline.getOption(GLE_OPT_DEVICE)->getArg(0);
	if (device->hasValue(GLE_DEVICE_PS)) {
		cmdline.setHasOption(GLE_OPT_FULL_PAGE, true);
	}
	if (cmdline.hasOption(GLE_OPT_LANDSCAPE)) {
		cmdline.setHasOption(GLE_OPT_FULL_PAGE, true);
	}
	// remove stdin marker "-" from main arguments
	cmdline.checkForStdin();
	// Auto detect extra arguments (after .GLE / .TEX files)
	// Note that extra arguments can also be supplied by means of -args
	if (cmdline.getMainArgSepPos() == -1) {
		int nargs = cmdline.getNbMainArgs();
		for (int i = 0; i < nargs; i++) {
			const string& arg = cmdline.getMainArg(i);
			if (!str_i_ends_with(arg, ".GLE")) {
				if (i != 0) cmdline.setMainArgSepPos(i);
				break;
			}
		}
	}
	// Distable TeX interface
	if (cmdline.hasOption(GLE_OPT_SAFEMODE)) {
		TeXInterface* interface = TeXInterface::getInstance();
		interface->setEnabled(false);
	}
	// Process further args into global config object
	GLEGlobalConfig* conf = GLEGetInterfacePointer()->getConfig();
	conf->initCmdLine();
}

void init_option_args(CmdLineObj& cmdline) {
	CmdLineOption* option;
	CmdLineArgString* strarg;
	CmdLineArgInt* intarg;
	CmdLineArgSet* setarg;
	cmdline.setMainArgType("file name");
	option = new CmdLineOption("help", "h", "?");
	option->setHelp("Shows help about command line options");
	strarg = new CmdLineArgString("option");
	strarg->setHelp("show specific help about 'option'");
	strarg->setCardLimits(0, 1);
	option->addArg(strarg);
	cmdline.addOption(option, GLE_OPT_HELP);
	option = new CmdLineOption("device", "d");
	option->setHelp("Selects output device(s)");
	option->setMinNbArgs(1);
	setarg = new CmdLineArgSet("device-names");
	setarg->setHelp("set output device(s)");
	setarg->setMinCard(1);
	/* Order of values is important! (must match GLE_DEVICE_ constants in core.h) */
	setarg->addPossibleValue("eps");
	setarg->addPossibleValue("ps");
	setarg->addPossibleValue("pdf");
	setarg->addPossibleValue("svg");
	setarg->addPossibleValue("jpg");
	setarg->addPossibleValue("png");
	setarg->addPossibleValue("x11");
	setarg->addPossibleValue("emf");
#ifndef HAVE_X11
	setarg->setUnsupportedValue(GLE_DEVICE_X11);
#endif
#ifndef HAVE_CAIRO
	setarg->setUnsupportedValue(GLE_DEVICE_SVG);
	setarg->setUnsupportedValue(GLE_DEVICE_EMF);
#endif
#ifndef __WIN32__
	setarg->setUnsupportedValue(GLE_DEVICE_EMF);
#endif
	setarg->addDefaultValue(GLE_DEVICE_EPS);
	option->addArg(setarg);
	cmdline.addOption(option, GLE_OPT_DEVICE);
#ifdef HAVE_CAIRO
	option = new CmdLineOption("cairo");
	option->setHelp("Use cairo output device");
	cmdline.addOption(option, GLE_OPT_CAIRO);
#endif
	option = new CmdLineOption("fullpage");
	option->setHelp("Selects full page output");
	cmdline.addOption(option, GLE_OPT_FULL_PAGE);
	option = new CmdLineOption("landscape");
	option->setHelp("Selects full page landscape output");
	cmdline.addOption(option, GLE_OPT_LANDSCAPE);
	option = new CmdLineOption("nocolor", "bw");
	option->setHelp("Forces grayscale output");
	cmdline.addOption(option, GLE_OPT_NO_COLOR);
	option = new CmdLineOption("inverse");
	option->setHelp("Render black as white for using on dark backgrounds");
	cmdline.addOption(option, GLE_OPT_INVERSE);
	option = new CmdLineOption("transparent", "tr");
	option->setHelp("Creates transparent output (with -d png)");
	cmdline.addOption(option, GLE_OPT_TRANSPARENT);
	option = new CmdLineOption("noctrl-d");
	option->setHelp("Excludes CTRL-D from the PostScript output");
	cmdline.addOption(option, GLE_OPT_NO_CTRL_D);
	option = new CmdLineOption("resolution", "r", "dpi");
	option->setHelp("Sets the resolution for bitmap and PDF output");
	intarg = new CmdLineArgInt("dpi");
	intarg->setHelp("set the resolution (measured in dots per inch)");
	intarg->setCardLimits(0, 1);
	intarg->setDefault(72);
	option->addArg(intarg);
	cmdline.addOption(option, GLE_OPT_DPI);
	option = new CmdLineOption("tex");
	option->setHelp("Indicates that the script includes LaTeX expressions");
	cmdline.addOption(option, GLE_OPT_TEX);
	option = new CmdLineOption("nopdftex");
	option->setHelp("Disable PdfLaTeX for .pdf creation");
	option->setExpert(true);
	cmdline.addOption(option, GLE_OPT_NO_PDFTEX);
	option = new CmdLineOption("inc");
	option->setHelp("Creates an .inc file with LaTeX code");
	cmdline.addOption(option, GLE_OPT_CREATE_INC);
	option = new CmdLineOption("texincprefix");
	option->setHelp("Adds the given subdirectory to the path in the .inc file");
	strarg = new CmdLineArgString("path");
	strarg->setHelp("adds 'path' to path in .inc file");
	strarg->setCardLimits(1, 1);
	option->addArg(strarg);
	cmdline.addOption(option, GLE_OPT_TEXINCPREF);
	option = new CmdLineOption("finddeps");
	option->setHelp("Automatically finds dependencies");
	strarg = new CmdLineArgString("path");
	strarg->setHelp("find dependencies in 'path'");
	strarg->setCardLimits(0, 1);
	// Default value should also be set if no value is given?
	option->addArg(strarg);
	cmdline.addOption(option, GLE_OPT_FINDDEPS);
	option = new CmdLineOption("preview", "p");
	option->setHelp("Previews the output with QGLE");
	cmdline.addOption(option, GLE_OPT_PREVIEW);
#ifdef ENABLE_GS_PREVIEW
	option = new CmdLineOption("gs");
	option->setHelp("Previews the output with GhostScript");
	cmdline.addOption(option, GLE_OPT_GSPREVIEW);
#endif
	option = new CmdLineOption("calc", "c");
	option->setHelp("Runs GLE in \"calculator\" mode");
	cmdline.addOption(option, GLE_OPT_CALC);
	option = new CmdLineOption("catcsv", "csv");
	option->setHelp("Pretty print a CSV file to standard output");
	cmdline.addOption(option, GLE_OPT_CATCSV);
	option = new CmdLineOption("output", "o");
	option->setHelp("Specifies the name of the output file");
	strarg = new CmdLineArgString("name");
	strarg->setHelp("writes output to file 'name'");
	strarg->setCardLimits(1, 1);
	option->addArg(strarg);
	cmdline.addOption(option, GLE_OPT_OUTPUT);
	option = new CmdLineOption("nosave");
	option->setHelp("Don't write output file to disk (dry-run)");
	cmdline.addOption(option, GLE_OPT_NOSAVE);
#if defined(__WIN32__) && defined(HAVE_CAIRO)
	option = new CmdLineOption("copy");
	option->setHelp("Copy resulting figure to clipboard (only EMF)");
	cmdline.addOption(option, GLE_OPT_COPY);
#endif
	option = new CmdLineOption("compatibility", "cm");
	option->setHelp("Selects a GLE compatibility mode");
	strarg = new CmdLineArgString("version");
	strarg->setHelp("specifies GLE version to emulate (can be any existing GLE version)");
	option->addArg(strarg);
	cmdline.addOption(option, GLE_OPT_COMPAT);
	option = new CmdLineOption("version", "v");
	option->setHelp("Selects a GLE version to run");
	setarg = new CmdLineArgSet("version");
	setarg->setHelp("run GLE version 'version'");
	setarg->setMinCard(1);
	setarg->setMaxCard(1);
	option->addArg(setarg);
	cmdline.addOption(option, GLE_OPT_VERSION);
	option = new CmdLineOption("noligatures");
	option->setHelp("Disable the use of ligatures for 'fl' and 'fi'");
	cmdline.addOption(option, GLE_OPT_NO_LIGATURES);
	option = new CmdLineOption("gsoptions");
	option->setHelp("Specify additional options for GhostScript");
	strarg = new CmdLineArgString("value");
	option->addArg(strarg);
	cmdline.addOption(option, GLE_OPT_GSOPTIONS);
	option = new CmdLineOption("safemode");
	option->setHelp("Disables reading/writing to the file system");
	cmdline.addOption(option, GLE_OPT_SAFEMODE);
	option = new CmdLineOption("allowread");
	option->setHelp("Allows reading from the given path");
	strarg = new CmdLineArgString("path");
	option->addArg(strarg);
	cmdline.addOption(option, GLE_OPT_ALLOWREAD);
	option = new CmdLineOption("allowwrite");
	option->setHelp("Allows writing to the given path");
	strarg = new CmdLineArgString("path");
	option->addArg(strarg);
	cmdline.addOption(option, GLE_OPT_ALLOWWRITE);
	option = new CmdLineOption("keep");
	option->setHelp("Don't delete temporary files");
	cmdline.addOption(option, GLE_OPT_KEEP);
	option = new CmdLineOption("trace");
	option->setHelp("Trace GLE");
	option->setExpert(true);
	cmdline.addOption(option, GLE_OPT_TRACE);
	option = new CmdLineOption("debug");
	option->setHelp("Debug GLE");
	option->setExpert(true);
	cmdline.addOption(option, GLE_OPT_DEBUG);
	option = new CmdLineOption("nomaxpath");
	option->setHelp("Disables the upper-bound on the drawing path complexity");
	cmdline.addOption(option, GLE_OPT_NO_MAXPATH);
	option = new CmdLineOption("mkinittex");
	option->setHelp("Creates \"inittex.ini\" from \"init.tex\"");
	cmdline.addOption(option, GLE_OPT_MKINITTEX);
	option = new CmdLineOption("info");
	option->setHelp("Outputs software version, build date, GLE_TOP, GLE_BIN, etc.");
	cmdline.addOption(option, GLE_OPT_INFO);
	option = new CmdLineOption("pause");
	option->setHelp("Pause if output has been generated");
	option->setExpert(true);
	cmdline.addOption(option, GLE_OPT_PAUSE);
	option = new CmdLineOption("verbosity", "vb");
	option->setHelp("Sets the verbosity level of GLE console output");
	intarg = new CmdLineArgInt("verbosity");
	intarg->setHelp("sets verbosity level to 'verbosity' (0..20)");
	intarg->setCardLimits(0, 1);
	intarg->setDefault(1);
	option->addArg(intarg);
	cmdline.addOption(option, GLE_OPT_VERBOSITY);
	cmdline.addMainArgSep("args");
	cmdline.addMainArgSep("a");
	cmdline.initOptions();
}

void gle_preview_file(const char* name, CmdLineObj& cmdline) {
	ostringstream commands;
	commands << "glefile: \"" << name << "\"" << endl;
	if (cmdline.hasOption(GLE_OPT_DPI)) {
		int dpi = ((CmdLineArgInt*)cmdline.getOption(GLE_OPT_DPI)->getArg(0))->getValue();
		commands << "dpi: \"" << dpi << "\"" << endl;
	}
	commands << "*DONE*" << endl;
	// cout << "Sending: " << commands.str() << endl;
	int result = GLESendSocket(commands.str());
	if (result == -3) {
		cerr << "Note: GLE is trying to launch QGLE, the GLE preview application" << endl;
#ifdef __WIN32__
		string qgle = "\"" + GLE_BIN_DIR + DIR_SEP + "qgle.exe\"";
#else
		string qgle = "\"" + GLE_BIN_DIR + DIR_SEP + "qgle\"";
#endif
		int sysres = GLESystem(qgle, false, false, NULL, NULL);
		if (sysres == GLE_SYSTEM_OK) {
			bool done = false;
			while (!done) {
				GLESleep(1000);
				result = GLESendSocket(commands.str());
				if (result != -3) {
					done = true;
				}
			}
		} else {
			cerr << "Error: failed to start QGLE: '" << qgle << "'" << endl;
			result = 0;
		}
	}
	if (result != 0) {
		cerr << "Error: could not connect to GLE preview application, code = " << result << endl;
	}
	cerr << endl;
}

bool has_pdflatex(CmdLineObj* cmdline) {
	if (cmdline->hasOption(GLE_OPT_NO_PDFTEX)) {
		return false;
	}
	ConfigSection* tex = g_Config.getSection(GLE_CONFIG_TEX);
	CmdLineArgSet* texsys =	(CmdLineArgSet*)tex->getOptionValue(GLE_TEX_SYSTEM);
	if (texsys->hasValue(GLE_TEX_SYSTEM_VTEX)) {
		return false;
	} else {
		return true;
	}
}

bool has_eps_or_pdf_based_device(CmdLineArgSet* device, CmdLineObj& cmdline) {
	if (cmdline.hasOption(GLE_OPT_TEX)) return true;
	if (device->hasValue(GLE_DEVICE_EPS)) return true;
	if (device->hasValue(GLE_DEVICE_PDF)) return true;
	if (device->hasValue(GLE_DEVICE_JPEG)) return true;
	if (device->hasValue(GLE_DEVICE_PNG)) return true;
	return false;
}

bool requires_tex(CmdLineArgSet* device, CmdLineObj* cmdline) {
	if (!cmdline->hasOption(GLE_OPT_CREATE_INC)) {
		if (device->hasValue(GLE_DEVICE_EPS)) return true;
		if (device->hasValue(GLE_DEVICE_PDF)) return true;
	}
	if (device->hasValue(GLE_DEVICE_PS)) return true;
	if (device->hasValue(GLE_DEVICE_JPEG)) return true;
	if (device->hasValue(GLE_DEVICE_PNG)) return true;
	return false;
}

bool has_bitmap_or_pdf_device(CmdLineArgSet* device) {
	if (device->hasValue(GLE_DEVICE_JPEG)) return true;
	if (device->hasValue(GLE_DEVICE_PNG)) return true;
	if (device->hasValue(GLE_DEVICE_PDF)) return true;
	return false;
}

bool is_bitmap_device(int device) {
	if (device == GLE_DEVICE_JPEG) return true;
	if (device == GLE_DEVICE_PNG) return true;
	return false;
}

void force_device(int dev, CmdLineObj& cmdline) {
	CmdLineArgSet* device = (CmdLineArgSet*)cmdline.getOption(GLE_OPT_DEVICE)->getArg(0);
	device->removeValue(GLE_DEVICE_EPS);
	device->addValue(dev);
}

void get_out_name(GLEFileLocation* inname, CmdLineObj& cmdline, GLEFileLocation* outname) {
	if (cmdline.hasOption(GLE_OPT_OUTPUT)) {
		const string& o_file = cmdline.getOptionString(GLE_OPT_OUTPUT);
		if (str_i_equals(o_file, "STDOUT")) {
			outname->createStdout();
		} else {
			if (str_i_ends_with(o_file, ".ps"))  force_device(GLE_DEVICE_PS,   cmdline);
			if (str_i_ends_with(o_file, ".pdf")) force_device(GLE_DEVICE_PDF,  cmdline);
			if (str_i_ends_with(o_file, ".svg")) force_device(GLE_DEVICE_SVG,  cmdline);
			if (str_i_ends_with(o_file, ".jpg")) force_device(GLE_DEVICE_JPEG, cmdline);
			if (str_i_ends_with(o_file, ".png")) force_device(GLE_DEVICE_PNG,  cmdline);
			string main_name;
			GetMainName(o_file, main_name);
			outname->fromFileNameDir(main_name, GLE_WORKING_DIR);
		}
	} else {
		if (inname->isStdin()) {
			outname->createStdout();
		} else {
			string main_name;
			GetMainNameExt(inname->getFullPath(), ".gle", main_name);
			outname->fromAbsolutePath(main_name);
		}
	}
}

GLERC<GLEScript> load_gle_code_sub(const char* name, CmdLineObj& cmdline) throw(ParserError) {
	string in_name = name;
	GLERC<GLEScript> script = new GLEScript();
	GLEFileLocation* loc = script->getLocation();
	loc->fromFileNameDir(in_name, GLE_WORKING_DIR);
	script->getSource()->load();
	return script;
}

GLERC<GLEScript> load_gle_code_sub_stdin(CmdLineObj& cmdline) throw(ParserError) {
	GLERC<GLEScript> script = new GLEScript();
	GLEFileLocation* loc = script->getLocation();
	loc->createStdin();
	loc->setDirectory(GLE_WORKING_DIR);
	script->getSource()->load();
	return script;
}

void delete_temp_file(const string& file, const char* ext) {
	int verbosity = g_verbosity();
	bool keep = g_CmdLine.hasOption(GLE_OPT_KEEP);
	if ((verbosity >= 5 && keep) || verbosity > 10) {
		string fname = file + ext;
		ostringstream todel;
		if (keep) {
			todel << "keep: " << fname;
		} else {
			todel << "delete: " << fname;
		}
		g_message(todel.str());
	}
	if (!g_CmdLine.hasOption(GLE_OPT_KEEP)) {
		DeleteFileWithExt(file, ext);
	}
}

void writeRecordedOutputFile(const string& fname, int deviceCode, string* buffer) throw (ParserError) {
	string outf = fname;
	outf.append(g_device_to_ext(deviceCode));
	ofstream out(outf.c_str(), ios::out | ios::binary);
	if (!out.is_open()) {
		g_throw_parser_error("failed to create file '", outf.c_str(), "'");
	}
	out.write(buffer->data(), buffer->size());
	out.close();
}

void writeRecordedOutputFile(const string& fname, int deviceCode, GLEScript* script) throw (ParserError) {
	string* buffer = script->getRecordedBytesBuffer(deviceCode);
	writeRecordedOutputFile(fname, deviceCode, buffer);
}

class GLELoadOneFileManager {
protected:
	GLEScript* m_Script;
	CmdLineObj* m_CmdLine;
	GLEFileLocation* m_OutName;
	GLEDevice* m_Device;
	GLEFileLocation m_IncName;
	bool m_HasTeXFile;
	bool m_HasTempDotDir;
	bool m_HasTempFile;
	std::set<int> m_hasGenerated;
	std::set<int> m_hasFile;
	std::set<int> m_hasIncFile;
public:
	GLELoadOneFileManager(GLEScript* script, CmdLineObj* cmdline, GLEFileLocation* outname);
	~GLELoadOneFileManager();
	void update_bounding_box();
	void delete_previous_output(int deviceCode);
	void create_cairo_eps() throw(ParserError);
	bool process_one_file_tex() throw(ParserError);
	void do_output_type(const char* type);
	void cat_stdout(const char* ext);
	void cat_stdout_and_del(const char* ext);
	void create_latex_eps_ps_pdf() throw(ParserError);
	void convert_eps_to_pdf_no_latex() throw(ParserError);
	istream* get_eps_stream();
	bool hasGenerated(int deviceCode);
	bool hasFile(int deviceCode);
	bool hasIncFile(int deviceCode);
	void setHasGenerated(int deviceCode, bool value);
	void setHasFile(int deviceCode, bool value);
	void setHasIncFile(int deviceCode, bool value);
	void write_recorded_data(int deviceCode) throw(ParserError);
	void clean_tex_temp_files();
	void clean_inc_file(int deviceCode);
	void delete_original_eps_pdf();
	void delete_original_eps_pdf_impl(int device);
	bool requires_tex_eps(CmdLineArgSet* device, CmdLineObj* cmdline);
	bool requires_tex_pdf(CmdLineArgSet* device, CmdLineObj* cmdline);
	bool has_cairo_pdf_based_device(CmdLineArgSet* device, CmdLineObj* cmdline);
};

GLELoadOneFileManager::GLELoadOneFileManager(GLEScript* script, CmdLineObj* cmdline, GLEFileLocation* outname) {
	m_Script = script;
	m_CmdLine = cmdline;
	m_OutName = outname;
	m_Device = NULL;
	m_HasTeXFile = false;
	m_HasTempDotDir = false;
	m_HasTempFile = false;
}

GLELoadOneFileManager::~GLELoadOneFileManager() {
}



void GLELoadOneFileManager::update_bounding_box() {
	double size_x, size_y;
	g_get_usersize(&size_x, &size_y);
	m_Script->setSize(size_x, size_y);
	PSGLEDevice* psdev = (PSGLEDevice*)g_get_device_ptr();
	m_Script->setBoundingBox(psdev->getBoundingBox()->getX(), psdev->getBoundingBox()->getY());
	m_Script->setBoundingBoxOrigin(0.0, 0.0);
}

void GLELoadOneFileManager::delete_previous_output(int deviceCode) {
	CmdLineArgSet* device = (CmdLineArgSet*)m_CmdLine->getOption(GLE_OPT_DEVICE)->getArg(0);
	if (device->hasValue(deviceCode) && !m_OutName->isStdout()) {
		/* Delete output so that QGLE does not display previous rendering if there are errors */
		DeleteFileWithExt(m_OutName->getFullPath(), g_device_to_ext(deviceCode));
	}
}

void GLELoadOneFileManager::create_cairo_eps() throw(ParserError) {
	CmdLineArgSet* device = (CmdLineArgSet*)m_CmdLine->getOption(GLE_OPT_DEVICE)->getArg(0);
	if (!hasGenerated(GLE_DEVICE_EPS) && device->hasValue(GLE_DEVICE_EPS)) {
		setHasGenerated(GLE_DEVICE_EPS, true);
		m_Device = g_select_device(GLE_DEVICE_CAIRO_EPS);
		m_Device->setRecordingEnabled(true);
		if (g_verbosity() > 0) {
			cerr << endl;
		}
		DrawIt(m_Script, m_OutName, m_CmdLine);
		m_Device->getRecordedBytes(m_Script->getRecordedBytesBuffer(GLE_DEVICE_EPS));
	}
}

bool GLELoadOneFileManager::process_one_file_tex() throw(ParserError) {
	CmdLineArgSet* device = (CmdLineArgSet*)m_CmdLine->getOption(GLE_OPT_DEVICE)->getArg(0);
	delete_previous_output(GLE_DEVICE_EPS);
	delete_previous_output(GLE_DEVICE_PDF);
	if (m_CmdLine->hasOption(GLE_OPT_CAIRO)) {
		if (has_cairo_pdf_based_device(device, m_CmdLine)) {
			setHasGenerated(GLE_DEVICE_PDF, true);
			m_Device = g_select_device(GLE_DEVICE_CAIRO_PDF);
		} else {
			setHasGenerated(GLE_DEVICE_EPS, true);
			m_Device = g_select_device(GLE_DEVICE_CAIRO_EPS);
		}
	} else {
		setHasGenerated(GLE_DEVICE_EPS, true);
		m_Device = g_select_device(GLE_DEVICE_EPS);
	}
	if (m_CmdLine->hasOption(GLE_OPT_DPI)) {
		m_Device->setResolution(((CmdLineArgInt*)m_CmdLine->getOption(GLE_OPT_DPI)->getArg(0))->getValue());
	}
	m_Device->setRecordingEnabled(true);
	/* In some cases two passes are required to measure size of TeX objects */
	int done = 1;
	TeXInterface* interface = TeXInterface::getInstance();
	if (m_Script->getLocation()->isStdin()) {
		interface->initialize(m_OutName, m_OutName);
	} else {
		interface->initialize(m_Script->getLocation(), m_OutName);
	}
	int iter = 0;
	do {
		interface->reset();
		if (iter != 0 && g_verbosity() > 0) cerr << endl;
		DrawIt(m_Script, m_OutName, m_CmdLine);
		if (get_nb_errors() > 0) {
			return false;
		}
		if (iter == 0 && interface->hasObjects() && m_OutName->isStdout()) {
			/* this means we need temporary files */
			m_HasTempFile = true;
			m_OutName->setFullPath(GLETempName());
			if (m_Script->getLocation()->isStdin()) {
				m_HasTempDotDir = true;
				interface->updateNames(m_OutName, m_OutName);
			} else {
				interface->updateOutName(m_OutName);
			}
		}
		done = interface->tryCreateHash();
		if (done == 2) {
			inc_nb_errors();
			return false;
		}
		iter++;
	} while (done == 1);
	/* get recorded PostScript code */
	if (hasGenerated(GLE_DEVICE_PDF)) {
		m_Device->getRecordedBytes(m_Script->getRecordedBytesBuffer(GLE_DEVICE_PDF));
	} else {
		m_Device->getRecordedBytes(m_Script->getRecordedBytesBuffer(GLE_DEVICE_EPS));
	}
	update_bounding_box();
	interface->checkObjectDimensions();
	create_cairo_eps();
	if (m_CmdLine->hasOption(GLE_OPT_CREATE_INC)) {
		interface->createInc(m_CmdLine->getStringValue(GLE_OPT_TEXINCPREF));
	}
	if (interface->hasObjects() && requires_tex(device, m_CmdLine)) {
		bool use_geom = false;
		bool has_pdftex = has_pdflatex(m_CmdLine);
		bool create_inc = m_CmdLine->hasOption(GLE_OPT_CREATE_INC);
		if (device->hasValue(GLE_DEVICE_PS)) use_geom = true;
		if (has_pdftex && !create_inc) use_geom = true;
		interface->createTeX(use_geom);
		m_HasTeXFile = true;
	}
	if (!interface->isEnabled()) {
		/* No TeX objects in safe mode */
		return false;
	}
	if (m_CmdLine->hasOption(GLE_OPT_CREATE_INC)) {
		/* See bug #2165591 */
		return true;
	}
	return interface->hasObjects();
}

void GLELoadOneFileManager::do_output_type(const char* type) {
	if (g_verbosity() > 0) {
		cerr << "[" << type << "]";
		g_set_console_output(false);
	}
}

void GLELoadOneFileManager::cat_stdout(const char* ext) {
	string outf = m_OutName->getFullPath() + ext;
	ifstream in(outf.c_str(), ios::in | ios::binary);
	GLECopyStream(in, cout);
	in.close();
}

void GLELoadOneFileManager::cat_stdout_and_del(const char* ext) {
	cat_stdout(ext);
	delete_temp_file(m_OutName->getFullPath(), ext);
}

void GLELoadOneFileManager::convert_eps_to_pdf_no_latex() throw(ParserError) {
	CmdLineArgSet* device = (CmdLineArgSet*)m_CmdLine->getOption(GLE_OPT_DEVICE)->getArg(0);
	if (device->hasValue(GLE_DEVICE_PDF) && !hasGenerated(GLE_DEVICE_PDF)) {
		setHasFile(GLE_DEVICE_PDF, true);
		int dpi = m_CmdLine->getIntValue(GLE_OPT_DPI);
		create_pdf_file_ghostscript(m_OutName, dpi, m_Script);
		do_output_type(".pdf");
	}
}

bool GLELoadOneFileManager::requires_tex_eps(CmdLineArgSet* device, CmdLineObj* cmdline) {
	if (!cmdline->hasOption(GLE_OPT_CREATE_INC)) {
		if (device->hasValue(GLE_DEVICE_EPS)) return true;
		if (device->hasValue(GLE_DEVICE_PDF) && !has_pdflatex(cmdline)) return true;
	}
	if (!hasGenerated(GLE_DEVICE_PDF)) {
		if (device->hasValue(GLE_DEVICE_JPEG)) return true;
		if (device->hasValue(GLE_DEVICE_PNG)) return true;
	}
	return false;
}

bool GLELoadOneFileManager::has_cairo_pdf_based_device(CmdLineArgSet* device, CmdLineObj* cmdline) {
	if (m_CmdLine->hasOption(GLE_OPT_CAIRO)) {
		if (device->hasValue(GLE_DEVICE_PDF)) return true;
		if (device->hasValue(GLE_DEVICE_JPEG)) return true;
		if (device->hasValue(GLE_DEVICE_PNG)) return true;
	}
	return false;
}

bool GLELoadOneFileManager::requires_tex_pdf(CmdLineArgSet* device, CmdLineObj* cmdline) {
	if (!cmdline->hasOption(GLE_OPT_CREATE_INC)) {
		if (device->hasValue(GLE_DEVICE_PDF)) return true;
	}
	if (hasGenerated(GLE_DEVICE_PDF)) {
		if (device->hasValue(GLE_DEVICE_JPEG)) return true;
		if (device->hasValue(GLE_DEVICE_PNG)) return true;
	}
	return false;
}

void GLELoadOneFileManager::create_latex_eps_ps_pdf() throw(ParserError) {
	/* m_OutName has no path and no extension */
	m_IncName.fromAbsolutePath(m_OutName->getFullPath() + "_inc");
	/* includegraphics does not handle "." in filenames */
	FileNameDotToUnderscore(m_IncName.getFullPathNC());
	bool create_inc = m_CmdLine->hasOption(GLE_OPT_CREATE_INC);
	bool has_pdftex = has_pdflatex(m_CmdLine);
	int dpi = m_CmdLine->getIntValue(GLE_OPT_DPI);
	CmdLineArgSet* device = (CmdLineArgSet*)m_CmdLine->getOption(GLE_OPT_DEVICE)->getArg(0);
	/* Create "_inc.eps" */
	if (device->hasOnlyValue(GLE_DEVICE_PDF) && (has_pdftex || create_inc)) {
		/* "_inc.eps" not required in this case */
	} else if (hasGenerated(GLE_DEVICE_EPS)) {
		setHasIncFile(GLE_DEVICE_EPS, true);
		writeRecordedOutputFile(m_IncName.getFullPath(), GLE_DEVICE_EPS, m_Script);
	}
	/* Create "_inc.pdf" */
	if ((device->hasValue(GLE_DEVICE_PDF) || hasGenerated(GLE_DEVICE_PDF)) && (has_pdftex || create_inc)) {
		setHasIncFile(GLE_DEVICE_PDF, true);
		if (hasGenerated(GLE_DEVICE_PDF)) {
			writeRecordedOutputFile(m_IncName.getFullPath(), GLE_DEVICE_PDF, m_Script);
		} else {
			create_pdf_file_ghostscript(&m_IncName, dpi, m_Script);
			do_output_type(".pdf");
		}
	}
	/* Exit if no EPS/PS/PDF required */
	if (!requires_tex_eps(device, m_CmdLine) &&
		!requires_tex_pdf(device, m_CmdLine) &&
	    !device->hasValue(GLE_DEVICE_PS)) {
			return;
	}
	/* Create EPS/PS/PDF based on .TeX */
	string out_name_dir, out_name_np;
	SplitFileName(m_OutName->getFullPath(), out_name_dir, out_name_np);
	/* LaTeX does not like to run with absolute paths */
	GLEChDir(out_name_dir);
	if (requires_tex_eps(device, m_CmdLine)) {
		create_eps_file_latex_dvips(out_name_np, m_Script);
		writeRecordedOutputFile(m_OutName->getFullPath(), GLE_DEVICE_EPS, m_Script);
		setHasFile(GLE_DEVICE_EPS, true);
	}
	if ((device->hasValue(GLE_DEVICE_PDF) && !create_inc) || requires_tex_pdf(device, m_CmdLine)) {
		setHasFile(GLE_DEVICE_PDF, true);
		if (has_pdftex) {
			create_pdf_file_pdflatex(out_name_np, m_Script);
		} else {
			create_pdf_file_ghostscript(m_OutName, dpi, m_Script);
			do_output_type(".pdf");
		}
	}
	if (device->hasValue(GLE_DEVICE_PS)) {
		create_ps_file_latex_dvips(out_name_np);
		if (m_OutName->isStdout()) cat_stdout_and_del(".ps");
		do_output_type(".ps");
	}
	GLEChDir(m_Script->getLocation()->getDirectory());
}

istream* GLELoadOneFileManager::get_eps_stream() {
	return NULL;
}

bool GLELoadOneFileManager::hasGenerated(int deviceCode) {
	return m_hasGenerated.find(deviceCode) != m_hasGenerated.end();
}

bool GLELoadOneFileManager::hasFile(int deviceCode) {
	return m_hasFile.find(deviceCode) != m_hasFile.end();
}

bool GLELoadOneFileManager::hasIncFile(int deviceCode) {
	return m_hasIncFile.find(deviceCode) != m_hasIncFile.end();
}

void GLELoadOneFileManager::setHasGenerated(int deviceCode, bool value) {
	if (value) {
		m_hasGenerated.insert(deviceCode);
	} else {
		m_hasGenerated.erase(deviceCode);
	}
}

void GLELoadOneFileManager::setHasFile(int deviceCode, bool value) {
	if (value) {
		m_hasFile.insert(deviceCode);
	} else {
		m_hasFile.erase(deviceCode);
	}
}

void GLELoadOneFileManager::setHasIncFile(int deviceCode, bool value) {
	if (value) {
		m_hasIncFile.insert(deviceCode);
	} else {
		m_hasIncFile.erase(deviceCode);
	}
}

void GLELoadOneFileManager::write_recorded_data(int deviceCode) throw (ParserError) {
	CmdLineArgSet* device = (CmdLineArgSet*)m_CmdLine->getOption(GLE_OPT_DEVICE)->getArg(0);
	if (!device->hasValue(deviceCode)) {
		return;
	}
	if (m_CmdLine->hasOption(GLE_OPT_CREATE_INC) || m_CmdLine->hasOption(GLE_OPT_NOSAVE)) {
		return;
	}
	if (m_OutName->isStdout()) {
		/* Write to stdout */
		if (hasFile(deviceCode)) {
			cat_stdout(g_device_to_ext(deviceCode));
		} else {
			string* buffer = m_Script->getRecordedBytesBuffer(deviceCode);
			cout.write(buffer->data(), buffer->size());
		}
	} else {
		if (!hasFile(deviceCode)) {
			writeRecordedOutputFile(m_OutName->getFullPath(), deviceCode, m_Script);
		}
	}
}

void GLELoadOneFileManager::clean_inc_file(int deviceCode) {
	/* Remove _inc.* if not required anymore */
	bool create_inc = m_CmdLine->hasOption(GLE_OPT_CREATE_INC);
	CmdLineArgSet* device = (CmdLineArgSet*)m_CmdLine->getOption(GLE_OPT_DEVICE)->getArg(0);
	if (hasIncFile(deviceCode) && (!create_inc || !device->hasValue(deviceCode))) {
		delete_temp_file(m_IncName.getFullPath(), g_device_to_ext(deviceCode));
	}
}

void GLELoadOneFileManager::clean_tex_temp_files() {
	clean_inc_file(GLE_DEVICE_EPS);
	clean_inc_file(GLE_DEVICE_PDF);
	/* Remove .tex file */
	if (m_HasTeXFile) {
		delete_temp_file(m_OutName->getFullPath(), ".tex");
	}
	/* Remove files in ".gle" directory */
	if (m_HasTempDotDir) {
		TeXInterface* interface = TeXInterface::getInstance();
		interface->removeDotFiles();
	}
}

void GLELoadOneFileManager::delete_original_eps_pdf_impl(int deviceCode) {
	CmdLineArgSet* device = (CmdLineArgSet*)m_CmdLine->getOption(GLE_OPT_DEVICE)->getArg(0);
	bool should_del = hasFile(deviceCode);
	if (device->hasValue(deviceCode)) {
		/* don't delete .eps/.pdf if we wanted this as output */
		/* and we did not wanted this printed to stout */
		if (!m_OutName->isStdout() && !m_CmdLine->hasOption(GLE_OPT_CREATE_INC)) {
			should_del = false;
		}
	}
	if (should_del) {
		/* delete original .eps/.pdf file */
		delete_temp_file(m_OutName->getFullPath(), g_device_to_ext(deviceCode));
	}
}

void GLELoadOneFileManager::delete_original_eps_pdf() {
	delete_original_eps_pdf_impl(GLE_DEVICE_EPS);
	delete_original_eps_pdf_impl(GLE_DEVICE_PDF);
	/* has temporary output file */
	if (m_HasTempFile) {
		#ifdef __WIN32__
			delete_temp_file(m_OutName->getFullPath(), ".tmp");
		#else
			delete_temp_file(m_OutName->getFullPath(), "");
		#endif
	}
}

void complain_latex_not_supported(int device) {
	if (TeXInterface::getInstance()->hasObjects()) {
		g_throw_parser_error(">> LaTeX expressions not supported in '", g_device_to_ext(device), "' output");
	}
}

void load_one_file_sub(GLEScript* script, CmdLineObj& cmdline, size_t* exit_code) throw(ParserError) {
	GLEFileLocation out_name; /* out_name has no extension */
	GLEGetInterfacePointer()->getConfig()->setAllowConfigBlocks(false);
	GLEChDir(script->getLocation()->getDirectory());
	get_out_name(script->getLocation(), cmdline, &out_name);
	g_set_console_output(false);
	g_message_first_newline(true);
	GLEGetColorList()->reset();
	if (cmdline.hasOption(GLE_OPT_DEBUG)){
		printf("Debug options 16=do_pcode, 8=pass 4=polish, 2=eval ");
		printf("Debug "); gle_debug = GLEReadConsoleInteger();
		printf("Trace "); trace_on = GLEReadConsoleInteger();
	}
	GLELoadOneFileManager manager(script, &cmdline, &out_name);
	CmdLineArgSet* device = (CmdLineArgSet*)cmdline.getOption(GLE_OPT_DEVICE)->getArg(0);
	if (has_eps_or_pdf_based_device(device, cmdline)) {
		bool has_tex = manager.process_one_file_tex();
		/* Return if there were errors */
		if (get_nb_errors() > 0) {
			if (g_verbosity() > 0) cerr << endl;
			(*exit_code)++;
			return;
		}
		/* Get device information */
		int dpi = cmdline.getIntValue(GLE_OPT_DPI);
		/* File has TeX code -> move it and create again */
		if (has_tex) {
			manager.create_latex_eps_ps_pdf();
		} else {
			manager.convert_eps_to_pdf_no_latex();
		}
		/* Create bitmap outputs */
		int output_options = 0;
		if (cmdline.hasOption(GLE_OPT_NO_COLOR)) {
			output_options |= GLE_OUTPUT_OPTION_GRAYSCALE;
		}
		if (cmdline.hasOption(GLE_OPT_TRANSPARENT)) {
			output_options |= GLE_OUTPUT_OPTION_TRANSPARENT;
		}
		for (int i = 0; i < device->getNbValues(); i++) {
			if (is_bitmap_device(i) && device->hasValue(i)) {
				create_bitmap_file(&out_name, i, dpi, output_options, script);
				manager.do_output_type(g_device_to_ext(i));
			}
		}
		/* Output .eps to stdout? */
		manager.write_recorded_data(GLE_DEVICE_EPS);
		manager.write_recorded_data(GLE_DEVICE_PDF);
		manager.delete_original_eps_pdf();
		if (has_tex) manager.clean_tex_temp_files();
		if (g_verbosity() > 0) cerr << endl;
	}
	if (device->hasValue(GLE_DEVICE_PS) && !cmdline.hasOption(GLE_OPT_TEX)) {
		GLEDevice* psdev = g_select_device(GLE_DEVICE_PS);
		DrawIt(script, &out_name, &cmdline);
		if (TeXInterface::getInstance()->hasObjects()) {
			g_message(">> To include LaTeX expressions, use \"gle -tex -d ps file.gle\"");
		}
		if (psdev->isRecordingEnabled()) {
			std::string recorded;
			psdev->getRecordedBytes(&recorded);
			writeRecordedOutputFile(out_name.getFullPath(), GLE_DEVICE_PS, &recorded);
		}
		if (out_name.isStdout()) {
			manager.cat_stdout_and_del(".ps");
		}
		cerr << endl;
	}
	if (device->hasValue(GLE_DEVICE_SVG)) {
		g_select_device(GLE_DEVICE_CAIRO_SVG);
		DrawIt(script, &out_name, &cmdline);
		complain_latex_not_supported(GLE_DEVICE_SVG);
		if (out_name.isStdout()) {
			manager.cat_stdout_and_del(".svg");
		}
		cerr << endl;
	}
	if (device->hasValue(GLE_DEVICE_X11)) {
		g_select_device(GLE_DEVICE_X11);
		DrawIt(script, &out_name, &cmdline);
	}
#if defined(__WIN32__) && defined(HAVE_CAIRO)
	if (device->hasValue(GLE_DEVICE_EMF)) {
		GLECairoDeviceEMF* dev = (GLECairoDeviceEMF*)g_select_device(GLE_DEVICE_EMF);
		if (cmdline.hasOption(GLE_OPT_COPY)) {
			dev->setCopyClipboard(true);
		}
		if (cmdline.hasOption(GLE_OPT_DPI)) {
			int dpi = ((CmdLineArgInt*)cmdline.getOption(GLE_OPT_DPI)->getArg(0))->getValue();
			dev->setDPI(dpi);
		} else {
			dev->setDPI(2000);
		}
		DrawIt(script, &out_name, &cmdline);
		complain_latex_not_supported(GLE_DEVICE_EMF);
		cerr << endl;
	}
#endif
}

void load_one_file(const char* name, CmdLineObj& cmdline, size_t* exit_code) {
	if (cmdline.hasOption(GLE_OPT_PREVIEW)) {
		GLEFileLocation loc;
		loc.fromFileNameDir(name, GLE_WORKING_DIR);
		gle_preview_file(loc.getFullPath().c_str(), cmdline);
		return;
	}
	try {
		GLERC<GLEScript> script = load_gle_code_sub(name, cmdline);
		load_one_file_sub(script.get(), cmdline, exit_code);
	} catch (ParserError& err) {
		string err_str;
		err.toString(err_str);
		str_uppercase_initial_capital(err_str);
		g_message(err_str);
		(*exit_code)++;
	}
}

void load_one_file_stdin(CmdLineObj& cmdline, size_t* exit_code) {
	try {
		GLERC<GLEScript> script = load_gle_code_sub_stdin(cmdline);
		load_one_file_sub(script.get(), cmdline, exit_code);
	} catch (ParserError& err) {
		string err_str;
		err.toString(err_str);
		str_uppercase_initial_capital(err_str);
		g_message(err_str);
		(*exit_code)++;
	}
}
