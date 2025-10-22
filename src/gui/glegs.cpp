/***********************************************************************************
 * QGLE - A Graphical Interface to GLE                                             *
 * Copyright (C) 2006  A. S. Budden & J. Struyf                                    *
 *                                                                                 *
 * This program is free software; you can redistribute it and/or                   *
 * modify it under the terms of the GNU General Public License                     *
 * as published by the Free Software Foundation; either version 2                  *
 * of the License, or (at your option) any later version.                          *
 *                                                                                 *
 * This program is distributed in the hope that it will be useful,                 *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of                  *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                   *
 * GNU General Public License for more details.                                    *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License               *
 * along with this program; if not, write to the Free Software                     *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA. *
 *                                                                                 *
 * Also add information on how to contact you by electronic and paper mail.        *
 ***********************************************************************************/

#ifdef HAVE_CONFIG_H
    #include "../config.h"
#else
    #include "../config_noauto.h"
#endif

// ghostscript headers
#include <ierrors.h>
#include <iapi.h>

#if STDC_HEADERS
	#include <stdlib.h>
#else
	#if HAVE_STDLIB_H
		#include <stdlib.h>
	#endif
#endif
#if HAVE_STRING_H
	#if !STDC_HEADERS && HAVE_MEMORY_H
		#include <memory.h>
	#endif
	#include <string.h>
#endif
#if HAVE_STRINGS_H
	#include <strings.h>
#endif
#if defined(__unix__) || defined(__APPLE__)
	#include <dlfcn.h>
#endif

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

#include "../gle/file_io.h"
#include "../gle/cutils.h"

// ./build/bin/glegs -I/Users/dtai/Library/Frameworks/Ghostscript.framework/Resources/lib -I/Users/dtai/Library/Frameworks/Ghostscript.framework/Resources/fonts -q -dAutoFilterColorImages=false -dAutoFilterGrayImages=false -dEncodeColorImages=true -dEncodeGrayImages=true -dEncodeMonoImages=true -dColorImageFilter=/FlateEncode -dGrayImageFilter=/FlateEncode -dMonoImageFilter=/FlateEncode -dBATCH -dNOPAUSE -r72 -g286x286 -sDEVICE=pdfwrite -sOutputFile="src/samples/sample.pdf" src/samples/sample.eps
// ./build/bin/glegs -q -dAutoFilterColorImages=false -dAutoFilterGrayImages=false -dEncodeColorImages=true -dEncodeGrayImages=true -dEncodeMonoImages=true -dColorImageFilter=/FlateEncode -dGrayImageFilter=/FlateEncode -dMonoImageFilter=/FlateEncode -dBATCH -dNOPAUSE -r72 -g286x286 -sDEVICE=pdfwrite -sOutputFile="src/samples/sample.pdf" src/samples/sample.eps

bool try_load_config(const string& confname, string* gsname) {
	ifstream file;
	file.open(confname.c_str());
	if (!file.is_open()) {
		return false;
	}
	string line;
	while (file.good()) {
		getline(file, line);
		string::size_type pos = line.find("libgs =");
		if (pos != string::npos) {
			line = line.substr(pos+7);
			str_trim_both(line);
			str_remove_quote(line);
			if (str_i_ends_with(line, ".framework")) {
				AddDirSep(line);
				line += "Ghostscript";
			}
			*gsname = line;
		}
	}
	file.close();
	return true;
}

bool try_load_config_sub(const string& topdir, vector<string>& tries, string* gsname) {
	string confname = topdir;
	AddDirSep(confname);
	confname += "glerc";
	tries.push_back(confname);
	return try_load_config(confname, gsname);
}

char* my_sdup(const char* str) {
	char* res = (char*)malloc(strlen(str)+1);
	strcpy(res, str);
	return res;
}

void clean_up(char** argv, char** margv) {
	if (argv != margv) {
		free(margv[1]);
		free(margv[2]);
		free(margv);
	}
}

int main(int argc, char** argv) {
	string exe_name;
	bool has_exe_name = GetExeName("glegs", argv, exe_name);
	if (!has_exe_name) {
		cout << "GLE-gs error: Can't locate executable" << endl;
		return 1;
	}
	string gsname;
	vector<string> tries;
	bool has_help = false;
	#ifdef GLETOP_CD
		// Try relative path
		string GLE_TOP_DIR = exe_name;
		StripPathComponents(&GLE_TOP_DIR, GLETOP_CD+1);
		AddDirSep(GLE_TOP_DIR);
		GLE_TOP_DIR += GLETOP_REL;
		bool has_config = try_load_config_sub(GLE_TOP_DIR, tries, &gsname);
		// Try one level higher as executable
		if (!has_config) {
			GLE_TOP_DIR = exe_name;
			StripPathComponents(&GLE_TOP_DIR, 2);
			has_config = try_load_config_sub(GLE_TOP_DIR, tries, &gsname);
		}
		// Try with absolute path
		if (!has_config) {
			GLE_TOP_DIR = GLETOP_ABS;
			has_config = try_load_config_sub(GLE_TOP_DIR, tries, &gsname);
		}
	#else
		string GLE_TOP_DIR = exe_name;
		StripPathComponents(&GLE_TOP_DIR, 2);
		bool has_config = try_load_config_sub(GLE_TOP_DIR, tries, &gsname);
	#endif
	if (!has_config) {
		cout << "GLE-gs error: Can't locate configuration file (glerc)" << endl;
		for (unsigned int i = 0; i < tries.size(); i++) {
			cout << "Try: " << tries[i] << endl;
		}
		return 1;
	}
	string userconf = GetHomeDir();
	AddDirSep(userconf);
	userconf += ".glerc";
	try_load_config(userconf, &gsname);
	if (gsname == "") {
		cout << "GLE-gs error: No 'libgs' in configuration file (run 'gle -finddeps' to locate it)" << endl;
		return 1;
	}
	char** margv = argv;
	string::size_type fwpos = gsname.rfind(".framework");
	if (fwpos != string::npos) {
		// If the Ghostscript framework is not installed in a default location it won't find its resources and fonts
		string loc = gsname.substr(0, fwpos+10);
		string libpath = string("-I") + loc + string("/Resources/lib");
		string fontpath = string("-I") + loc + string("/Resources/fonts");
		margv = (char**)malloc((argc+2)*sizeof(char*));
		margv[0] = argv[0];
		margv[1] = my_sdup(libpath.c_str());
		margv[2] = my_sdup(fontpath.c_str());
		for (int i = 1; i < argc; i++) {
			margv[i+2] = argv[i];
		}
		argc += 2;
	}
	for (int i = 0; i < argc; i++) {
		// cout << "arg: " << margv[i] << endl;
		if (str_i_equals(margv[i], "-h")) has_help = true;
	}
	void* hmodule = dlopen(gsname.c_str(), RTLD_NOW);
	if (hmodule == NULL) {
		cout << "GLE-gs error: Can't load '" << gsname << "'" << endl;
		cout << dlerror() << endl;
		clean_up(argv, margv);
		return 1;
	}
	PFN_gsapi_new_instance gsapi_new_instance = (PFN_gsapi_new_instance) dlsym(hmodule,"gsapi_new_instance");
	if (gsapi_new_instance == NULL) {
		dlclose(hmodule);
		clean_up(argv, margv);
		return 1;
	}
	PFN_gsapi_delete_instance gsapi_delete_instance = (PFN_gsapi_delete_instance) dlsym(hmodule, "gsapi_delete_instance");
	if (gsapi_delete_instance == NULL) {
		dlclose(hmodule);
		clean_up(argv, margv);
		return 1;
	}
	PFN_gsapi_init_with_args gsapi_init_with_args = (PFN_gsapi_init_with_args) dlsym(hmodule, "gsapi_init_with_args");
	if (gsapi_init_with_args == NULL) {
		dlclose(hmodule);
		clean_up(argv, margv);
		return 1;
	}
	PFN_gsapi_run_string gsapi_run_string = (PFN_gsapi_run_string) dlsym(hmodule, "gsapi_run_string");
	if (gsapi_run_string == NULL) {
		dlclose(hmodule);
		clean_up(argv, margv);
		return 1;
	}
	PFN_gsapi_exit gsapi_exit = (PFN_gsapi_exit) dlsym(hmodule, "gsapi_exit");
	if (gsapi_exit == NULL) {
		dlclose(hmodule);
		clean_up(argv, margv);
		return 1;
	}
	void *minst = NULL;
	int code = gsapi_new_instance(&minst, NULL);
	if (code < 0) {
		dlclose(hmodule);
		clean_up(argv, margv);
		return 1;
	}
	code = gsapi_init_with_args(minst, argc, margv);
	if (code == 0) {
		int exit_code;
		const char start_string[] = "systemdict /start get exec\n";
		code = gsapi_run_string(minst, start_string, 0, &exit_code);
	}
	int code1 = gsapi_exit(minst);
	if (code == 0 || code == e_Quit) {
		code = code1;
	}
	gsapi_delete_instance(minst);
	dlclose(hmodule);
	clean_up(argv, margv);
	if (code == 0 || code == e_Quit) {
		return 0;
	} else {
		return 1;
	}
}

