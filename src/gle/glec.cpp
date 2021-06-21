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

/**
 * GLE command line version using libGLE.so
 ***/

#include <stdio.h>
#include <string>
#include <iostream>

#ifdef HAVE_CONFIG_H
    #include "../config.h"
#else
    #include "../config_noauto.h"
#endif

#ifdef __WIN32__
	#include <windows.h>
#endif
#ifdef __UNIX__
	#include <dlfcn.h>
#endif

using namespace std;

bool GetExeName(const char* appname, char **argv, string& exe_name);
void StripPathComponents(string* fname, int nb);
string GLEAddRelPath(const string& base, int cd, const char* path);
void AddDirSep(string& fname);

extern string DIR_SEP;

void GLEAddLibName(string* lib) {
	AddDirSep(*lib);
	*lib += "libgle-graphics-";
	*lib += GLEVN;
#ifdef __UNIX__
	#ifdef __MACOS__
		*lib += ".dylib";
	#else
		*lib += ".so";
	#endif
#endif
#ifdef __WIN32__
	*lib += ".dll";
#endif
//	cout << "Trying: " << (*lib) << endl;
}

void* tryLoadLib(const string& name, int tryCount, bool verbose) {
	void* lib = 0;
#ifdef __UNIX__
	lib = dlopen(name.c_str(), RTLD_NOW);
#endif
	if (verbose) {
		cout << "Try: " << (tryCount + 1) << " load '" << name << "':" << endl;
		if (lib == 0) {
#ifdef __UNIX__
			cout << "error: " << dlerror();
#endif
		} else {
			cout << "OK";
		}
		cout << endl;
	}
	return lib;
}

int main(int argc, char** argv) {
	string exe_name;
	int (*GLEMain)(int g_argc, char** g_argv);
	if (!GetExeName("gle", argv, exe_name)) {
		cerr << "GLE: error getting file name of 'gle' executable" << endl;
		return -1;
	}
#ifdef __UNIX__
	string lib_name;
	void* lib = NULL;
	unsigned int tryCount = 0;
	while (lib == 0 && tryCount < 2) {
#ifdef GLELIB_CD
		if (lib == NULL) {
			lib_name = GLEAddRelPath(exe_name, GLELIB_CD+1, GLELIB_REL);
			GLEAddLibName(&lib_name);
			lib = tryLoadLib(lib_name, tryCount, tryCount == 1);
		}
#endif
		if (lib == NULL) {
			// Try "../lib"
			lib_name = GLEAddRelPath(exe_name, 2, "lib");
			GLEAddLibName(&lib_name);
			lib = tryLoadLib(lib_name, tryCount, tryCount == 1);
		}
#ifdef GLELIB_ABS
		if (lib == NULL) {
			lib_name = GLELIB_ABS;
			GLEAddLibName(&lib_name);
			lib = tryLoadLib(lib_name, tryCount, tryCount == 1);
		}
#endif
		tryCount++;
	}
	if (lib == NULL) {
		cerr << "GLE: error loading library" << endl;
		return -1;
	}
	const char* error = NULL;
	GLEMain = (int (*)(int, char**))dlsym(lib, "GLEMain");
	if (GLEMain == NULL && (error = dlerror()) != NULL)  {
		cerr << "GLE: can't find main method in library '" << lib_name << "'" << endl;
		cerr << "Message: " << error << endl;
		return -1;
	}
	int result = GLEMain(argc, argv);
	dlclose(lib);
#endif
#ifdef __WIN32__
	string lib_name = exe_name;
	StripPathComponents(&lib_name, 1);
	GLEAddLibName(&lib_name);
	HMODULE lib = LoadLibraryA(lib_name.c_str());
	if (lib < (HINSTANCE)HINSTANCE_ERROR) {
		cerr << "GLE: error loading library '" << lib_name << "'" << endl;
		return -1;
	}
	GLEMain = (int (*)(int, char**))GetProcAddress(lib, "GLEMain");
	if (GLEMain == NULL)  {
		cerr << "GLE: can't find main method in library '" << lib_name << "'" << endl;
		return -1;
	}
	int result = GLEMain(argc, argv);
	// FreeLibrary(lib);
#endif
	return result;
}
