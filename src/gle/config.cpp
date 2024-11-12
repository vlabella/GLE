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

/*
 * 2004 Jan Struyf
 *
 */

#include "all.h"
#include "gle-interface/gle-interface.h"
#include "tokens/stokenizer.h"
#include "mem_limits.h"
#include "token.h"
#include "core.h"
#include "file_io.h"
#include "texinterface.h"
#include "cutils.h"
#include "cmdline.h"
#include "gprint.h"
#include "config.h"
#include "drawit.h"

#ifdef __WIN32__
	#include <windows.h>
#endif

#define BEGINDEF extern
#include "begin.h"

extern string GLE_TOP_DIR;
extern string GLE_BIN_DIR;
extern ConfigCollection g_Config;
extern CmdLineObj g_CmdLine;

void doskip(char *s,int *ct);
char *un_quote(char *ct);

#define skipspace doskip(tk[ct],&ct)

void begin_config(const std::string& block, int *pln, int *pcode, int *cp) {
	string block_name(block);
	ConfigSection* section = g_Config.getSection(block_name);
	if (section == NULL) {
		g_throw_parser_error("unrecognized config section '", block_name.c_str(), "'");
	}
	// Don't do config blocks in safe mode (except in RC file)
	GLEInterface* iface = GLEGetInterfacePointer();
	if (iface->getCmdLine()->hasOption(GLE_OPT_SAFEMODE)) {
		GLEGlobalConfig* config = iface->getConfig();
		if (!config->allowConfigBlocks()) {
			g_throw_parser_error("safe mode - config blocks not allowed");
		}
	}
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
		int mode = 0;
		bool plus_is = false;
		CmdLineOption* option = NULL;
		while (ct <= ntk) {
			skipspace;
			if (section != NULL) {
				if (mode == 0) {
					option = section->getOption(tk[ct]);
					if (option == NULL) {
						gprint("Not a valid setting for section '%s': {%s}\n", block_name.c_str(), tk[ct]);
					}
				} else if (mode == 1) {
					if (strcmp(tk[ct], "=") == 0) {
						plus_is = false;
					} else if (strcmp(tk[ct], "+=") == 0) {
						plus_is = true;
					} else {
						gprint("Expected '=' or '+=', not {%s}\n", tk[ct]);
					}
				} else if (option != NULL) {
					CmdLineOptionArg* arg = option->getArg(0);
					if (!plus_is) arg->reset();
					arg->appendValue(tk[ct]);
				}
				mode++;
			}
			ct++;
		}
	}
}

void init_config(ConfigCollection* collection) {
	ConfigSection* section;
	CmdLineOption* option;
	CmdLineArgString* strarg;
	CmdLineArgSet* setarg;
/* GLE */
	section = new ConfigSection("gle");
	strarg = section->addStringOption("current", GLE_CONFIG_GLE_VERSION);
	strarg->setDefault("");
	section->addSPairListOption("versions", GLE_CONFIG_GLE_INSTALL);
	collection->addSection(section, GLE_CONFIG_GLE);
/* Tools */
	section = new ConfigSection("tools");
	/* LaTeX */
	strarg = section->addStringOption("latex", GLE_TOOL_LATEX_CMD);
#ifdef __WIN32__
	strarg->setDefault("latex.exe");
#endif
#ifdef __UNIX__
	strarg->setDefault("latex");
#endif
#ifdef __OS2__
	strarg->setDefault("vlatexp.cmd");
#endif
	section->addStringOption("latex_options", GLE_TOOL_LATEX_OPTIONS);

	/* PdfLaTeX */
	strarg = section->addStringOption("pdflatex", GLE_TOOL_PDFTEX_CMD);
#ifdef __WIN32__
	strarg->setDefault("pdflatex.exe");
#else
	strarg->setDefault("pdflatex");
#endif
	section->addStringOption("pdflatex_options", GLE_TOOL_PDFTEX_OPTIONS);

	/* DVIPS */
	strarg = section->addStringOption("dvips", GLE_TOOL_DVIPS_CMD);
#ifdef __WIN32__
	strarg->setDefault("dvips.exe");
#endif
#ifdef __UNIX__
	strarg->setDefault("dvips");
#endif
#ifdef __OS2__
	strarg->setDefault("dvips.exe");
#endif
	section->addStringOption("dvips_options", GLE_TOOL_DVIPS_OPTIONS);
	/* GhostScript Program */
	strarg = section->addStringOption("ghostscript", GLE_TOOL_GHOSTSCRIPT_CMD);
#ifdef __WIN32__
	#define ENVIRONMENT32
	#if _WIN32 || _WIN64
		// Visual Studio builds with cl.exe
		#if _WIN64
			#define ENVIRONMENT64
		#else
			#define ENVIRONMENT32
		#endif
	#endif
	#if __GNUC__
		// gcc builds
		#if __x86_64__ || __ppc64__
			#define ENVIRONMENT64
		#else
			#define ENVIRONMENT32
		#endif
	#endif
	#ifdef ENVIRONMENT32
		strarg->setDefault("gswin32c.exe");
	#endif
	#ifdef ENVIRONMENT64
		strarg->setDefault("gswin64c.exe");
	#endif
#endif
#ifdef __APPLE__
	strarg->setDefault("gs,glegs;$EXELOC/glegs");
#endif
#if defined(__UNIX__) && !defined(__APPLE__)
	strarg->setDefault("gs");
#endif
#ifdef __OS2__
	strarg->setDefault("gsos2.exe");
#endif
	section->addStringOption("ghostscript_options", GLE_TOOL_GHOSTSCRIPT_OPTIONS);

	collection->addSection(section, GLE_CONFIG_TOOLS);
	/* GhostScript Library */
	strarg = section->addStringOption("libgs", GLE_TOOL_GHOSTSCRIPT_LIB);
#ifdef __WIN32__
	#define ENVIRONMENT32
	#if _WIN32 || _WIN64
		// Visual Studio builds with cl.exe
		#if _WIN64
			#define ENVIRONMENT64
		#else
			#define ENVIRONMENT32
		#endif
	#endif
	#if __GNUC__
		// gcc builds
		#if __x86_64__ || __ppc64__
			#define ENVIRONMENT64
		#else
			#define ENVIRONMENT32
		#endif
	#endif
	#ifdef ENVIRONMENT32
		strarg->setDefault("gsdll32.dll");
	#endif
	#ifdef ENVIRONMENT64
		strarg->setDefault("gsdll64.dll");
	#endif
#endif
#ifdef __APPLE__
	strarg->setDefault("/Library/Frameworks/Ghostscript.framework,Ghostscript.framework");
#endif
#if defined(__UNIX__) && !defined(__APPLE__)
	strarg->setDefault("/usr/lib/libgs.so");
#endif
	strarg = section->addStringOption("editor", GLE_TOOL_TEXT_EDITOR);
	strarg = section->addStringOption("pdfviewer", GLE_TOOL_PDF_VIEWER);
/* TeX config */
	section = new ConfigSection("tex");
	option = new CmdLineOption("system");
	setarg = new CmdLineArgSet("device-names");
	setarg->setMaxCard(1);
	setarg->addPossibleValue("latex");
	setarg->addPossibleValue("vtex");
#ifdef __OS2__
	setarg->addDefaultValue(GLE_TEX_SYSTEM_VTEX);
#else
	setarg->addDefaultValue(GLE_TEX_SYSTEM_LATEX);
#endif
	option->addArg(setarg);
	section->addOption(option, GLE_TEX_SYSTEM);
	collection->addSection(section, GLE_CONFIG_TEX);
/* Config paper */
	section = new ConfigSection("paper");
	strarg = section->addStringOption("size", GLE_CONFIG_PAPER_SIZE);
	strarg->setDefault("a4paper");
	strarg = section->addStringOption("margins", GLE_CONFIG_PAPER_MARGINS);
	strarg->setDefault("2.54 2.54 2.54 2.54");
	collection->addSection(section, GLE_CONFIG_PAPER);
	collection->setDefaultValues();
}

const string& gle_config_margins() {
	ConfigSection* paper = g_Config.getSection(GLE_CONFIG_PAPER);
	return ((CmdLineArgString*)paper->getOptionValue(GLE_CONFIG_PAPER_MARGINS))->getValue();
}

const string& gle_config_papersize() {
	ConfigSection* paper = g_Config.getSection(GLE_CONFIG_PAPER);
	return ((CmdLineArgString*)paper->getOptionValue(GLE_CONFIG_PAPER_SIZE))->getValue();
}

bool try_load_config(const string& fname) {
	// FIXME: script as parameter to avoid repeated construction?
	GLERC<GLEScript> script = new GLEScript();
	script->getLocation()->fromFileNameCrDir(fname);
	if (script->getSource()->tryLoad()) {
		GLEGlobalConfig* conf = GLEGetInterfacePointer()->getConfig();
		conf->setAllowConfigBlocks(true);
		g_select_device(GLE_DEVICE_DUMMY);
		g_message_first_newline(false);
		GLEFileLocation output;
		output.createIllegal();
		DrawIt(script.get(), &output, NULL, true);
		conf->setAllowConfigBlocks(false);
		return true;
	} else {
		return false;
	}
}

bool try_save_config(const string& fname, GLEInterface* iface, bool isUser) {
	ConfigCollection* collection = iface->getConfig()->getRCFile();
	if (collection->allDefaults()) {
		return true;
	}
	if (fname == "") {
		return false;
	}
	if (IsAbsPath(fname)) {
		std::string dirname;
		GetDirName(fname, dirname);
		EnsureMkDir(dirname);
	}
	ofstream fout(fname.c_str());
	if (!fout.is_open()) {
		return false;
	}
	CmdLineOption* versionOption = collection->getSection(GLE_CONFIG_GLE)->getOption(GLE_CONFIG_GLE_VERSION);
	ostringstream out;
	out << "Save configuration to: '" << fname << "'";
	GLEOutputStream* output = iface->getOutput();
	output->println(out.str().c_str());
	for (int i = 0; i < collection->getNbSections(); i++) {
		ConfigSection* sec = collection->getSection(i);
		if (!sec->allDefaults()) {
			fout << "begin config " << sec->getName() << endl;
			for (int j = 0; j < sec->getNbOptions(); j++) {
				CmdLineOption* option = sec->getOption(j);
				if (!option->allDefaults() && (!isUser || option != versionOption)) {
					fout << "\t" << option->getName() << " = ";
					for (int k = 0; k < option->getMaxNbArgs(); k++) {
						if (k != 0) fout << " ";
						CmdLineOptionArg* arg = option->getArg(k);
						arg->write(fout);
					}
					fout << endl;
				}
			}
			fout << "end config" << endl << endl;
		}
	}
	fout.close();
	return true;
}

void get_version_soft(const string& cmdline, string& version) {
	// Running GLE gives something like: "GLE version 4.0.11"
	// Parse this string to get version number
	string gle_output;
	GLERunCommand(cmdline, gle_output);
	str_parse_get_next(gle_output, "VERSION", version);
	str_remove_quote(version);
}

void get_version_hard(const string& cmdline, string& version) {
	// If gle_version_soft fails, use this more complex method
	// Running GLE on some dummy file results in "GLE 4.0.11 [temp1234.gle]-C-R-[temp1234.eps]"
	// Parse this string to get version number
	string gle_output;
	string temp_file = "temp1234";
	// Set GLE top
	GLESetGLETop(cmdline);
	// Create dummy file
	string temp_gle_file = temp_file + ".gle";
	ofstream file(temp_gle_file.c_str());
	file << "size 10 10" << endl;
	file << "amove 0 0" << endl;
	file.close();
	// Create command line
	string cmd_line = string("\"") + cmdline + "\" " + temp_gle_file;
	GLERunCommand(cmd_line, gle_output);
	// Parse output to find version number
	str_parse_get_next(gle_output, "GLE", version);
	// Delete our temp file
	TryDeleteFile(temp_gle_file);
	TryDeleteFile(temp_file + ".ps");
}

void init_installed_versions(CmdLineObj& cmdline, ConfigCollection* collection) {
	CmdLineArgSet* versions = (CmdLineArgSet*)cmdline.getOption(GLE_OPT_VERSION)->getArg(0);
	ConfigSection* gle = collection->getSection(GLE_CONFIG_GLE);
	CmdLineArgSPairList* installs = (CmdLineArgSPairList*)gle->getOption(GLE_CONFIG_GLE_INSTALL)->getArg(0);
	if (installs->size() == 0) {
		versions->addPossibleValue("no older GLE versions found (run \"gle -finddeps\")");
	} else {
		for (int i = 0; i < installs->size(); i++) {
			versions->addPossibleValue(installs->getValue1(i).c_str());
		}
	}
}

void do_wait_for_enter() {
	if (g_CmdLine.hasOption(GLE_OPT_PAUSE)) {
		cout << "Press enter to continue ..." << endl;
		GLEReadConsoleInteger();
	}
}

void do_wait_for_enter_exit(int exitcode) {
	do_wait_for_enter();
	exit(exitcode);
}

class GLEProgressIndicatorInterface : public GLEProgressIndicator {
protected:
	GLEInterface* m_Iface;
public:
	GLEProgressIndicatorInterface(GLEInterface* iface);
	virtual ~GLEProgressIndicatorInterface();
	virtual void indicate();
};

GLEProgressIndicatorInterface::GLEProgressIndicatorInterface(GLEInterface* iface) {
	m_Iface = iface;
}

GLEProgressIndicatorInterface::~GLEProgressIndicatorInterface() {
}

void GLEProgressIndicatorInterface::indicate() {
	m_Iface->getOutput()->printflush(".");
}

string get_tool_path(int tool, ConfigSection* tools) {
	string path(tools->getOptionString(tool));
	string::size_type pos = path.find(',');
	if (pos != string::npos) {
		path.erase(pos);
	}
	pos = path.find(';');
	if (pos != string::npos) {
		path.erase(pos);
	}
	str_replace_all(path, "$EXELOC", GLE_BIN_DIR.c_str());
	return GLEExpandEnvironmentVariables(path);
}

void find_deps(const string& loc, GLEInterface* iface) {
	vector<GLEFindEntry*> tofind;
	vector<string*> result;
	string gle_paths = ";";
	ConfigCollection* collection = iface->getConfig()->getRCFile();
#ifdef __WIN32__
	GLEFindEntry* findGLE = new GLEFindEntry(&gle_paths);
	findGLE->addToFind("gle.exe");
	findGLE->addToFind("gle_ps.exe");
	tofind.push_back(findGLE);
#endif
	// Create GLEFindEntry for each tool (ghostscript, pdflatex, ...)
	ConfigSection* tools = collection->getSection(GLE_CONFIG_TOOLS);
	for (int j = 0; j <= GLE_TOOL_GHOSTSCRIPT_LIB; j++) {
		CmdLineArgString* strarg = (CmdLineArgString*)tools->getOption(j)->getArg(0);
		GLEFindEntry* findTool = new GLEFindEntry(strarg->getValuePtr());
		char_separator separator(",", ";");
		tokenizer<char_separator> tokens(strarg->getDefault(), separator);
		while (tokens.has_more()) {
			const string& toolName = tokens.next_token();
			if (toolName == ";") {
				if (tokens.has_more() && strarg->isDefault()) {
					findTool->setNotFound(tokens.next_token());
				}
				break;
			} else {
				if (!IsAbsPath(toolName)) {
					findTool->addToFind(toolName);
				}
			}
		}
		if (findTool->getNbFind() != 0) tofind.push_back(findTool);
		else delete findTool;
	}
	// Initialize output and progress indicator
	GLEOutputStream* output = iface->getOutput();
	ostringstream out1;
	out1 << "Running GLE -finddeps \"";
	out1 << loc;
	out1 << ("\" to locate installed software (e.g., Ghostscript and LaTeX): ");
	output->println(out1.str().c_str());
	GLEProgressIndicatorInterface progress(iface);
	// Perform search at specified location
	if (loc != "") {
		if (IsDirectory(loc, true)) {
			GLEFindFiles(loc, tofind, &progress);
			for (unsigned int i = 0; i < tofind.size(); i++) {
				tofind[i]->updateResult(false);
			}
		} else {
			// Name of old GLERC file given
			if (try_load_config(loc)) {
				// Override old version number
				collection->setStringValue(GLE_CONFIG_GLE, GLE_CONFIG_GLE_VERSION, GLEVN);
			} else {
				ostringstream err;
				err << "Can't load configuration from '" << loc << "'" << endl;
				output->println(err.str().c_str());
			}
		}
	}
	#ifdef __UNIX__
		// Find programs in search path on Unix
		GLEFindPrograms(tofind, &progress);
	#endif
	#ifdef __APPLE__
		// Search for frameworks on Mac
		GLEFindFiles(string("/Library/Frameworks"), tofind, &progress);
		string home = GetHomeDir();
		if (home != "") {
			home += "Library/Frameworks";
			GLEFindFiles(home, tofind, &progress);
		}
	#endif
	for (unsigned int i = 0; i < tofind.size(); i++) {
		tofind[i]->updateResult(true);
	}
	#ifdef __UNIX__
		// Search for libraries in typical directories and in LD_LIBRARY_PATH
		string gslibloc = GLEFindLibrary("libgs", &progress);
		if (gslibloc != "") {
			CmdLineArgString* gslib_stra = (CmdLineArgString*)tools->getOption(GLE_TOOL_GHOSTSCRIPT_LIB)->getArg(0);
			gslib_stra->setValue(gslibloc.c_str());
		}
	#endif
	output->println();
	// Write installed GLE's to config section
	ConfigSection* gle = collection->getSection(GLE_CONFIG_GLE);
	CmdLineArgSPairList* installs = (CmdLineArgSPairList*)gle->getOption(GLE_CONFIG_GLE_INSTALL)->getArg(0);
	char_separator separator(";", "");
	tokenizer<char_separator> tokens(gle_paths, separator);
	while (tokens.has_more()) {
		string path = tokens.next_token();
		if (path.length() > 0 && !installs->hasValue2(path)) {
			installs->addPair("?", path);
		}
	}
	// Find versions of installed GLEs and set value of gleexe
	ostringstream out;
	string gle_version = GLEVN;
	if (installs->size() > 1) {
		// Only need to find out versions if more than one installed
		// otherwise assume it is "this" version
		for (int i = 0; i < installs->size(); i++) {
			const string& cr_gle = installs->getValue2(i);
			string& version = installs->getValue1(i);
			if (version == "?") {
				get_version_soft(cr_gle, version);
				if (version == "?") {
					// cout << "Use hard method for: " << cr_gle << endl;
					get_version_hard(cr_gle, version);
				}
			}
			if (str_i_equals(version, gle_version)) {
				out << "Found: GLE " << version << " in " << cr_gle << " (*)" << endl;
			} else {
				out << "Found: GLE " << version << " in " << cr_gle << endl;
			}
		}
	} else if (installs->size() == 1) {
		out << "Found: GLE in " << installs->getValue2(0) << endl;
		// Do not need to remember installed GLEs if there is only one
		// because then the "-v" option makes no sense
		installs->reset();
	}
	// Show locations of other tools
	for (int j = 0; j <= GLE_TOOL_GHOSTSCRIPT_LIB; j++) {
		CmdLineOption* opt = tools->getOption(j);
		CmdLineArgString* strarg = (CmdLineArgString*)opt->getArg(0);
		if (strarg->isDefault()) {
			out << "Found: " << opt->getName() << " in '?'" << endl;
		} else {
			out << "Found: " << opt->getName() << " in '" << strarg->getValue() << "'" << endl;
		}
	}
	output->println(out.str().c_str());
	for (unsigned int i = 0; i < tofind.size(); i++) {
		delete tofind[i];
	}
}

void complain_about_gletop(bool has_top, ostream& out) {
	if (has_top) {
		// If the user has GLE_TOP, then it points to the wrong location
		// Let him first try to remove it, to see if that works
		out << "GLE_TOP might be pointing to an incorrect location." << endl;
		out << "Try removing GLE_TOP from your environment." << endl;
	} else {
		// The user did not set GLE_TOP and still something is wrong
		// Ask him to correct this by setting GLE_TOP as a workaround
		out << "Please set GLE_TOP to the correct location." << endl;
	}
}

bool check_correct_version(const string name, bool has_top, bool has_config, const vector<string>& triedLocations, ConfigCollection& coll) {
	if (has_config) {
		const string& val = coll.getStringValue(GLE_CONFIG_GLE, GLE_CONFIG_GLE_VERSION);
		if (!str_i_equals(val.c_str(), GLEVN)) {
			ostringstream out;
			out << "Error: GLE's configuration file:" << endl;
			out << "       '" << name << "'" << endl;
			out << "Is from GLE version '";
			if (val == "") { out << "unknown"; } else { out << val; }
			out << "' (and not '" << GLEVN << "' as expected)." << endl;
			complain_about_gletop(has_top, out);
			g_message(out.str().c_str());
			return false;
		}
	} else {
		ostringstream out;
		out << "Error: GLE is unable to locate its configuration file." << endl;
		out << "       GLE searched these locations:" << endl;
		for (size_t i = 0; i < triedLocations.size(); i++) {
			out << "       '" << triedLocations[i] << "'" << endl;
		}
		complain_about_gletop(has_top, out);
		g_message(out.str().c_str());
		return false;
	}
	coll.setStringValue(GLE_CONFIG_GLE, GLE_CONFIG_GLE_VERSION, GLEVN);
	return true;
}

bool try_load_config_sub(string& conf_name, vector<string>& triedLocations) {
	StripDirSep(GLE_TOP_DIR);
	string conf_name2 = GLE_TOP_DIR + DIR_SEP + "glerc";
	triedLocations.push_back(conf_name2);
	bool has_config = try_load_config(conf_name2);
	if (has_config) {
		// only update name if configuration file found here
		conf_name = conf_name2;
	}
	return has_config;
}

bool do_load_config(const char* appname, char **argv, CmdLineObj& cmdline, ConfigCollection& coll) {
	// Set GLE_TOP
	// -> prefer environment var GLE_TOP
	// -> otherwise, locate relative to executable location
	string conf_name;
	bool has_top = false;
	bool has_config = false;
	const char* top = getenv("GLE_TOP");
	vector<string> triedLocations;
	if (top == NULL || top[0] == 0) {
		string exe_name;
		bool has_exe_name = GetExeName(appname, argv, exe_name);
		if (has_exe_name) {
			GetDirName(exe_name, GLE_BIN_DIR);
			StripDirSep(GLE_BIN_DIR);
			#ifdef GLETOP_CD
				// Try relative path
				GLE_TOP_DIR = GLEAddRelPath(exe_name, GLETOP_CD+1, GLETOP_REL);
				has_config = try_load_config_sub(conf_name, triedLocations);
				// Try one level higher as executable
				if (!has_config) {
					GLE_TOP_DIR = GLEAddRelPath(exe_name, 2, NULL);
					has_config = try_load_config_sub(conf_name, triedLocations);
				}
				// Try with absolute path
				if (!has_config) {
					GLE_TOP_DIR = GLETOP_ABS;
					has_config = try_load_config_sub(conf_name, triedLocations);
				}
			#else
				GLE_TOP_DIR = exe_name;
				StripPathComponents(&GLE_TOP_DIR, 2);
			#endif
		} else {
			// The user will see as error message: "$GLE_TOP/some_file" not found.
			GLE_TOP_DIR = "$GLE_TOP";
		}
	} else {
		has_top = true;
		GLE_TOP_DIR = top;
	}
	StripDirSep(GLE_TOP_DIR);
	if (!has_config) {
		// Try load config file
		// -> first $GLE_TOP/glerc
		if (conf_name == "") {
			// Only update conf_name if it was not yet set
			// To make error message if glerc not found more interpretable
			conf_name = GLE_TOP_DIR + DIR_SEP + "glerc";
			if (std::find(triedLocations.begin(), triedLocations.end(), conf_name) == triedLocations.end()) {
				triedLocations.push_back(conf_name);
				has_config = try_load_config(conf_name);
			}
		}
	}
	if (!check_correct_version(conf_name, has_top, has_config, triedLocations, coll)) {
		return false;
	}
	GLEInterface* iface = GLEGetInterfacePointer();
	string uconf = iface->getUserConfigLocation();
	if (uconf != "") {
		// -> on Unix also $HOME/.glerc
		try_load_config(uconf);
	}
	// Set values for -v option
	init_installed_versions(cmdline, &coll);
	return has_config;
}

void do_find_deps_sub(GLEInterface* iface, const string& loc) {
	string myloc = loc;
#ifdef __WIN32__
	if (myloc.length() == 0) {
		GLEGetEnv("ProgramFiles", myloc);
	}
#endif
	find_deps(myloc, iface);
}

void do_save_config() {
	GLEInterface* iface = GLEGetInterfacePointer();
	string conf_name = GLE_TOP_DIR + DIR_SEP + "glerc";
	bool is_ok = try_save_config(conf_name, iface, false);
	if (!is_ok) {
		string user_conf = iface->getUserConfigLocation();
		is_ok = try_save_config(user_conf, iface, true);
	}
	if (!is_ok) {
		ostringstream err;
		err << ">>> Can't write to config file '" << conf_name << "'" << endl;
		GLEOutputStream* output = iface->getOutput();
		output->println(err.str().c_str());
	}
}

void do_find_deps(CmdLineObj& cmdline) {
	// Save config if find deps
	if (cmdline.hasOption(GLE_OPT_FINDDEPS)) {
		GLEInterface* iface = GLEGetInterfacePointer();
		CmdLineArgString* arg = (CmdLineArgString*)cmdline.getOption(GLE_OPT_FINDDEPS)->getArg(0);
		do_find_deps_sub(iface, arg->getValue());
		do_save_config();
		do_wait_for_enter();
		exit(0);
	}
}

GLEGlobalConfig::GLEGlobalConfig() {
	m_CmdLine = NULL;
	m_Config = NULL;
	m_AllowConfigBlocks = false;
}

GLEGlobalConfig::~GLEGlobalConfig() {
}

void GLEGlobalConfig::initCmdLine() {
	m_AllowReadDirs.clear();
	if (getCmdLine()->hasOption(GLE_OPT_ALLOWREAD)) {
		GLEPathToVector(getCmdLine()->getOptionString(GLE_OPT_ALLOWREAD), &m_AllowReadDirs);
	}
	m_AllowWriteDirs.clear();
	if (getCmdLine()->hasOption(GLE_OPT_ALLOWWRITE)) {
		GLEPathToVector(getCmdLine()->getOptionString(GLE_OPT_ALLOWWRITE), &m_AllowWriteDirs);
	}
}

int g_verbosity() {
	return g_CmdLine.getIntValue(GLE_OPT_VERBOSITY);
}
