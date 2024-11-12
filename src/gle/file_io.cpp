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

#include "basicconf.h"

#include <list>
#include <map>
#include <fstream>
#include <algorithm>

#include <errno.h>
#include <time.h>
#include <sys/stat.h>

#ifdef HAVE_SYS_PARAM_H
	#include <sys/param.h>
#endif

#ifdef __WIN32__
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <stdlib.h>
	#include <process.h>
	#include <windows.h>
	#include <winsock.h>
#endif
#ifdef __CYGWIN__
	#include <unistd.h>
	#include <windows.h>
#endif
#ifdef __APPLE__
	#include <mach-o/dyld.h>
#endif
#if defined(__UNIX__) || defined(__APPLE__)
	#include <limits.h>
	#include <unistd.h>
	#include <fcntl.h>
	#include <sys/types.h>
	#include <sys/wait.h>
	#include <dirent.h>
	#ifdef HAVE_SOCKETS
		#include <sys/socket.h>
		#include <netinet/in.h>
		#include <arpa/inet.h>
		typedef int SOCKET;
		#define SOCKET_ERROR -1
	#endif
#endif
#if defined(__OS2__) && defined(__EMX__)
	#include <unistd.h>
	#include <sys/types.h>
	#ifdef HAVE_SOCKETS
		#include <sys/socket.h>
		#include <netinet/in.h>
		#include <arpa/inet.h>
		typedef int SOCKET;
		#define SOCKET_ERROR -1
	#endif
#endif

/* PATH_MAX is not defined on gnu-hurd */
#ifndef PATH_MAX
       #define PATH_MAX 8192
#endif

using namespace std;

#include "tokens/stokenizer.h"
#include "file_io.h"
#include "cutils.h"
#include <cstring>

#if defined(__UNIX__) || defined(__APPLE__)
	/* Cygwin too */
	string PATH_SEP = ":";
	string DIR_SEP = "/";
#endif

#ifdef __WIN32__
	string PATH_SEP = ";";
	string DIR_SEP = "\\";
#endif

#if defined(__OS2__) && defined(__EMX__)
	string PATH_SEP = ";";
	string DIR_SEP = "/";
#endif

string GLE_TOP_DIR;
string GLE_BIN_DIR;

bool GLEGetEnv(const string& name, string& result) {
	const char* env = getenv(name.c_str());
	if (env == NULL) {
		result = "";
		return false;
	} else {
		result = env;
		return true;
	}
}

//
// -- directory finding routines
//

void FillIncludePaths(vector<string> &IP) {
	// fills containor with paths to search for
	// file.  This is called if it can't find file
	// in current directory
	//
	// Paths searched are:
	// %GLE_TOP%
	// %GLE_TOP%/gleinc
	// %GLE_USRLIB%
	// note that GLE_USERLIB can contain multiple
	// directories separated by ; (PC) or : (unix)
	// as defined in PATH_SEP
	//
	//printf("string paths for GLE_PATH\n");
	string paths = GLE_TOP_DIR + DIR_SEP + "gleinc";
	IP.push_back(paths);
	//printf("paths for GLE_USRLIB\n");
	const char* usr_lib = getenv("GLE_USRLIB");
	if (usr_lib != NULL) {
		paths = getenv("GLE_USRLIB");
		GLEPathToVector(paths, &IP);
	}
}

void GLEPathToVector(const string& path, vector<string>* vec) {
	char_separator sep(PATH_SEP.c_str());
	tokenizer<char_separator> tokens(path, sep);
	while (tokens.has_more()) {
		string dir = tokens.next_token();
		CorrectDirSepStrip(dir);
		vec->push_back(dir);
	}
}

bool GLEStreamContains(istream& strm, const char* msg) {
	string line;
	while (!strm.eof()) {
		getline(strm, line);
		if (str_i_str(line, msg) != -1) return true;
	}
	return false;
}

void StripDirSep(string& fname) {
	if (str_i_ends_with(fname, DIR_SEP.c_str())) {
		int nb = DIR_SEP.length();
		fname.erase(fname.length()-nb, nb);
	}
}

void StripDirSepButNotRoot(string& fname) {
	if (str_i_ends_with(fname, DIR_SEP.c_str())) {
		if (fname == "/") {
			// don't strip unix root dir
			return;
		}
		int nb = DIR_SEP.length();
		fname.erase(fname.length()-nb, nb);
	}
}

void CorrectDirSep(string& fname) {
	int len = fname.length();
	char dirsep = DIR_SEP[0];
	for (int i = 0; i < len; i++) {
		if (fname[i] == '/' || fname[i] == '\\') {
			fname[i] = dirsep;
		}
	}
}

void CorrectDirSepStrip(string& fname) {
	CorrectDirSep(fname);
	StripDirSepButNotRoot(fname);
}

void AddDirSep(string& fname) {
	string::size_type pos = fname.length();
	if (pos >= 1) {
		if (fname[pos-1] != '/' && fname[pos-1] != '\\') {
			// only add if not already has one!
			fname += DIR_SEP;
		}
	} else {
		fname += DIR_SEP;
	}
}

string GLEAddRelPath(const string& base, int cd, const char* path) {
	string result = base;
	StripPathComponents(&result, cd);
	if (path != NULL && path[0] != 0) {
		AddDirSep(result);
		result += path;
	}
	return result;
}

bool GLEAddRelPathAndFileTry(const string& base, int cd, const char* path, const char* file, string& result) {
	result = GLEAddRelPath(base, cd, path);
	AddDirSep(result);
	result += file;
	return GLEFileExists(result);
}

bool IsAbsPath(const string& path) {
	if (path.length() >= 1) {
		if (path[0] == '/') {
			// Unix style absolute path
			return true;
		}
		if (path.length() >= 3) {
			if (path[1] == ':' && (path[2] == '/' || path[2] == '\\')) {
				// Windows style absolute path
				return true;
			}
		}
	}
	return false;
}

void GLENormalizePath(string& path) {
	char dir_sep = DIR_SEP[0];
	string::size_type len  = path.length();
	string::size_type orig = 0;
	string::size_type dest = 0;
	while (orig < len) {
		if (path[orig] == '/' || path[orig] == '\\') {
			if (orig + 1 < len &&
			    (path[orig+1] == '/' || path[orig+1] == '\\')) {
				// found "//" -> skip
				orig++;
			} else if (orig + 2 < len &&
			    path[orig+1] == '.' &&
			    (path[orig+2] == '/' || path[orig+2] == '\\')) {
				// found "/./" -> skip
				orig += 2;
			} else if (orig + 3 < len &&
			    path[orig+1] == '.' && path[orig+2] == '.' &&
			    (path[orig+3] == '/' || path[orig+3] == '\\')) {
				// found "/../" -> skip + remove last directory
				orig += 3;
				if (dest > 0) dest--; // backup to last written character
				while (dest > 0 && path[dest] != '/' && path[dest] != '\\') {
					// as long as this is not path separator, go back
					dest--;
				}
			} else {
				orig++;
				path[dest++] = dir_sep;
			}
		} else {
			path[dest++] = path[orig++];
		}
	}
	path.resize(dest);
}

void GLEGetFullPath(const string& dirname, const string& fname, string& fullpath) {
	if (IsAbsPath(fname)) {
		fullpath = fname;
	} else {
		fullpath = dirname;
		AddDirSep(fullpath);
		fullpath += fname;
	}
	GLENormalizePath(fullpath);
}

void FileNameDotToUnderscore(string& fname) {
	string::size_type pos = fname.length();
	while (pos >= 1 && fname[pos-1] != '/' && fname[pos-1] != '\\') {
		if (fname[pos-1] == '.') fname[pos-1] = '_';
		if (fname[pos-1] == ' ') fname[pos-1] = '_';
		pos--;
	}
}

// Get extension of filename and convert it to lower case
void GetExtension(const string& fname, string& ext) {
	string::size_type pos = fname.length();
	// Never search to before "/" or "\\": file may not have extension
	// And don't find "." in path before file name
	while (pos >= 1 && fname[pos-1] != '/' && fname[pos-1] != '\\' && fname[pos-1] != '.' ) {
		pos--;
	}
	if (pos >= 1 && fname[pos-1] == '.') {
		ext = fname.substr(pos);
		gle_strlwr(ext);
	} else {
		ext = "";
	}
}

// Get file name without extension
void GetMainName(const string& fname, string& name) {
	string::size_type pos = fname.length();
	// Never search to before "/" or "\\": file may not have extension
	// And don't find "." in path before file name
	while (pos >= 1 && fname[pos-1] != '/' && fname[pos-1] != '\\' && fname[pos-1] != '.' ) {
		pos--;
	}
	if (pos >= 1 && fname[pos-1] == '.') {
		name = fname.substr(0, pos-1);
	} else {
		name = fname;
	}
}

void GetMainNameExt(const string& fname, const char* ext, string& name) {
	if (str_i_ends_with(fname, ext)) {
		string::size_type pos = fname.length() - strlen(ext);
		name = fname.substr(0, pos);
	} else {
		name = fname;
	}
}

// Add extension to filename
void AddExtension(string& fname, const string& ext) {
	string::size_type pos = fname.length();
	// Never search to before "/" or "\\": file may not have extension
	// And don't find "." in path before file name
	while (pos >= 1 && fname[pos-1] != '/' && fname[pos-1] != '\\' && fname[pos-1] != '.' ) {
		pos--;
	}
	if (pos >= 1 && fname[pos-1] == '.') {
		fname.erase(pos);
		fname += ext;
	} else {
		fname += ".";
		fname += ext;
	}
}

// Get path of file name (assume last part after DIR_SEP is file name)
void GetDirName(const string& path, string& dir) {
	string::size_type pos = path.length();
	while (pos >= 1 && path[pos-1] != '/' && path[pos-1] != '\\') {
		pos--;
	}
	// position pos-1 is where the slash is, so if pos = 1 then the slash is the first
	// character of the path and the file is in the root of the system
	if (pos >= 1 && (path[pos-1] == '/' || path[pos-1] == '\\')) {
		dir = path.substr(0, pos);
		AddDirSep(dir);
	} else {
		dir = "";
	}
}

// Split file name in path (dir) and file name (name)
void SplitFileName(const string& path, string& dir, string& name) {
	string::size_type pos = path.length();
	while (pos >= 1 && path[pos-1] != '/' && path[pos-1] != '\\') {
		pos--;
	}
	// position pos-1 is where the slash is, so if pos = 1 then the slash is the first
	// character of the path and the file is in the root of the system
	if (pos >= 1 && (path[pos-1] == '/' || path[pos-1] == '\\')) {
		dir = path.substr(0, pos); // second argument to substr = number of characters to copy
		name = path.substr(pos);
		AddDirSep(dir);
	} else {
		name = path;
		dir = "";
	}
}

// Split file name in path (dir) and file name (name) and return only file name
void SplitFileNameNoDir(const string& path, string& name) {
	string::size_type pos = path.length();
	while (pos >= 1 && path[pos-1] != '/' && path[pos-1] != '\\') {
		pos--;
	}
	// position pos-1 is where the slash is, so if pos = 1 then the slash is the first
	// character of the path and the file is in the root of the system
	if (pos >= 1 && (path[pos-1] == '/' || path[pos-1] == '\\')) {
		name = path.substr(pos);
	} else {
		name = path;
	}
}

// Remove given absolute path from filename
void RemoveDirectoryIfEqual(string* filename, const string& directory) {
	if (IsAbsPath(directory)) {
		// Ignore trailing path separator on directory
		int pos = directory.length()-1;
		while (pos > 0 && (directory[pos] == '/' || directory[pos] == '\\')) pos--;
		// Check if directory is a prefix of the given filename
		if (strncmp(directory.c_str(), filename->c_str(), pos+1) == 0) {
			// Skip the path separator character
			pos++;
			// And erase the absolute path prefix
			if (pos < (int)filename->length() && ((*filename)[pos] == '/' || (*filename)[pos] == '\\')) {
				filename->erase(0, pos+1);
			}
		}
	}
}

void StripPathComponents(string* fname, int nb) {
	while (nb > 0) {
		string::size_type i = fname->rfind(DIR_SEP);
		if (i != string::npos) {
			*fname = fname->substr(0, i);
		} else {
			break;
		}
		nb--;
	}
}

string GLETempDirName() {
	string result;
#ifdef __WIN32__
	#define WIN32_TMPNAM_BUFSIZE 1024
	TCHAR lpPathBuffer[WIN32_TMPNAM_BUFSIZE+1];
	DWORD dwRetVal = GetTempPath(WIN32_TMPNAM_BUFSIZE, lpPathBuffer);
	if (dwRetVal > WIN32_TMPNAM_BUFSIZE || (dwRetVal == 0))  {
		return string("C:\\");
	} else {
		result = lpPathBuffer;
	}
#else
	result = "/tmp";
#endif
	AddDirSep(result);
	return result;
}

string GLETempName() {
	string result;
#ifdef __WIN32__
	#define WIN32_TMPNAM_BUFSIZE 1024
	result = "C:\\gle.tmp";
	TCHAR szTempName[WIN32_TMPNAM_BUFSIZE+1];
	TCHAR lpPathBuffer[WIN32_TMPNAM_BUFSIZE+1];
	DWORD dwRetVal = GetTempPath(WIN32_TMPNAM_BUFSIZE, lpPathBuffer);
	if (dwRetVal <= WIN32_TMPNAM_BUFSIZE && dwRetVal != 0)  {
		UINT uRetVal = GetTempFileName(lpPathBuffer, TEXT("gle"), 0, szTempName);
		if (uRetVal != 0) {
			result = szTempName;
		}
	}
#else
	const char* base = "/tmp/gle-XXXXXX";
	char* temp = (char*)malloc(strlen(base)+1);
	strcpy(temp, base);
	int fd = mkstemp(temp);
	if (fd != -1) close(fd);
	result = temp;
	free(temp);
#endif
	GetMainNameExt(result, ".tmp", result);
	return result;
}

bool GLEMoveFile(const string& from, const string& to) {
#ifdef __WIN32__
#else
	if (rename(from.c_str(), to.c_str()) == -1) return false;
#endif
	return true;
}

void GLECopyStream(istream& from, ostream& to) {
	to << from.rdbuf();
}

int GLECopyFile(const string& from, const string& to, string* err) {
	ifstream strm(from.c_str());
	if (!strm.is_open()) {
		if (err != NULL) {
			*err = string("file '") + from + "' not found";
		}
		return GLE_FILE_NOT_FOUND_ERROR;
	}
	ofstream out(to.c_str());
	if (!out.is_open()) {
		strm.close();
		if (err != NULL) {
			*err = string("can't create '") + to + "'";
		}
		return GLE_FILE_WRITE_ERROR;
	}
	GLECopyStream(strm, out);
	out.close();
	strm.close();
	if (out.fail()) {
		if (err != NULL) {
			*err = string("error while writing to '") + to + "'";
		}
		return GLE_FILE_WRITE_ERROR;
	}
	return GLE_FILE_OK;
}

// Return current directory
bool GLEGetCrDir(string* name) {
#ifdef __WIN32__
	TCHAR buffer[1024];
	if (GetCurrentDirectory(1024, buffer) != 0) {
		*name = buffer;
		return true;
	}
#else
	#ifdef _GNU_SOURCE
		char* res = get_current_dir_name();
		if (res != NULL) {
			*name = res;
			free(res);
			return true;
		}
	#else
		char buffer[1024];
		if (getcwd(buffer, 1024) != NULL) {
			*name = buffer;
			return true;
		}
	#endif
#endif
	return false;
}

// Return current directory
// -> Used for compatibility with QGLE.EXE
bool GLEGetCrDirWin32(string* name) {
#if defined(__WIN32__) || defined(__CYGWIN__)
	TCHAR buffer[1024];
	if (GetCurrentDirectory(1024, buffer) != 0) {
		*name = buffer;
		return true;
	} else {
		return false;
	}
#else
	char buffer[1024];
	if (getcwd(buffer, 1024) == NULL) {
		return false;
	} else {
		*name = buffer;
		return true;
	}
#endif
}

// Change directory
bool GLEChDir(const string& dir) {
#ifdef __WIN32__
	return SetCurrentDirectory(dir.c_str());
#else
	return chdir(dir.c_str()) == 0;
#endif
}

void GLESetGLETop(const string& cmdline) {
	string gle_top = cmdline;
	StripPathComponents(&gle_top, 1);
	// Newer versions have the executable in the "bin" subdir
	if (!GLEFileExists(gle_top + DIR_SEP + "inittex.ini")) {
		StripPathComponents(&gle_top, 1);
	}
	gle_top = "GLE_TOP="+gle_top;
#ifdef __WIN32__
	_putenv(gle_top.c_str());
#endif
}

#define GLEREAD_BUF 10000

int GLERunCommand(const string& cmd, string& result) {
	ostringstream strm;
	int res = GLESystem(cmd, true, true, NULL, &strm);
	result = strm.str();
	return res;
}

#define GLESYS_PIPE_RD 0
#define GLESYS_PIPE_WR 1

#ifdef __WIN32__

void GLEReadFileWin32(HANDLE input, ostream* strm) {
	char buffer[GLEREAD_BUF+1];
	while(true) {
		DWORD nbytes = 0;
		BOOL fSuccess = ReadFile(input, buffer, GLEREAD_BUF, &nbytes, NULL);
		if (!fSuccess || nbytes == 0) {
			CloseHandle(input);
			return;
		} else if (strm != NULL) {
			buffer[nbytes] = 0;
			nbytes = str_remove_all(buffer, '\r');
			strm->write(buffer, nbytes);
		}
	}
}

typedef struct {
   HANDLE handle;
   istream* strm;
} GLESystemWriterData;

DWORD WINAPI GLESystemWriter(LPVOID lpParam) {
	GLESystemWriterData* data = (GLESystemWriterData*)lpParam;
	char write_buffer[GLEREAD_BUF+1];
	while(data->strm->good()) {
		data->strm->read(write_buffer, GLEREAD_BUF);
		int to_write = data->strm->gcount();
		DWORD nbWritten;
		BOOL fSuccess = WriteFile(data->handle, write_buffer, to_write, &nbWritten, NULL);
		if (!fSuccess) {
			break;
		}
	}
	CloseHandle(data->handle);
	ExitThread(0);
	return 0;
}

int GLESystem(const string& cmd, bool wait, bool redirout, istream* ins, ostream* outerrs) {
	SECURITY_ATTRIBUTES saAttr;
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;
	HANDLE hChildStdinRd, hChildStdinWr, hChildStdoutRd, hChildStdoutWr;
	if (wait) {
		// Create pipe for standard out
		if (!CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &saAttr, 0)) {
			cerr << "stdout pipe creation failed" << endl;
			return GLE_SYSTEM_ERROR;
		}
		SetHandleInformation(hChildStdoutRd, HANDLE_FLAG_INHERIT, 0);
		// Create pipe for standard in
		if (!CreatePipe(&hChildStdinRd, &hChildStdinWr, &saAttr, 0)) {
			cerr << "stdin pipe creation failed" << endl;
			return GLE_SYSTEM_ERROR;
		}
		SetHandleInformation(hChildStdinWr, HANDLE_FLAG_INHERIT, 0);
	}
	PROCESS_INFORMATION piProcInfo;
	STARTUPINFO siStartInfo;
	ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
	ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
	siStartInfo.cb = sizeof(STARTUPINFO);
	if (wait) {
		siStartInfo.hStdError = hChildStdoutWr;
		if (redirout) {
			siStartInfo.hStdOutput = hChildStdoutWr;
		} else {
			siStartInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
		}
		siStartInfo.hStdInput = hChildStdinRd;
		siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
	}
	BOOL bFuncRetn = CreateProcess(NULL,
	      (CHAR*)cmd.c_str(),                        // command line
	      NULL,                                      // process security attributes
	      NULL,                                      // primary thread security attributes
	      wait,                                      // handles are inherited
	      NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW,  // creation flags
	      NULL,                                      // use parent's environment
	      NULL,                                      // use parent's current directory
	      &siStartInfo,                              // STARTUPINFO pointer
	      &piProcInfo);                              // receives PROCESS_INFORMATION
	if (bFuncRetn == 0) {
		if (wait) {
			CloseHandle(hChildStdinRd);
			CloseHandle(hChildStdinWr);
			CloseHandle(hChildStdoutRd);
			CloseHandle(hChildStdoutWr);
		}
		return GLE_SYSTEM_ERROR;
	}
	if (wait) {
		HANDLE hWrThread = NULL;
		GLESystemWriterData writerData;
		CloseHandle(hChildStdinRd);
		CloseHandle(hChildStdoutWr);
		if (ins == NULL) {
			CloseHandle(hChildStdinWr);
		} else {
			DWORD dwThreadID;
			writerData.strm = ins;
			writerData.handle = hChildStdinWr;
			hWrThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)GLESystemWriter, (LPVOID)&writerData, 0, &dwThreadID);
		}
		GLEReadFileWin32(hChildStdoutRd, outerrs);
		if (hWrThread != NULL) {
			WaitForSingleObject(hWrThread, INFINITE);
			CloseHandle(hWrThread);
		}
		WaitForSingleObject(piProcInfo.hProcess, INFINITE);
	}
	CloseHandle(piProcInfo.hProcess);
	CloseHandle(piProcInfo.hThread);
	return GLE_SYSTEM_OK;
}

#else

void GLECloseFD(int* fd, int i) {
	if (fd[i] != -1) {
		close(fd[i]);
		fd[i] = -1;
	}
}

void GLECloseFDArray(int* fd) {
	for (int i = 0; i < 4; i++) {
		GLECloseFD(fd, i);
	}
}

void GLEDupFD(int* fd, int dup, int to) {
	if (*fd >= 0) {
		close(fd[1-dup]);
		dup2(fd[dup], to);
		close(fd[dup]);
	}
}

int GLESystem(const string& cmd, bool wait, bool redirout, istream* ins, ostream* outerrs) {
	int fd[] = { -1, -1, -1, -1 };
	int* const pin = fd;
	int* const pout = fd+2;
	if (wait) {
		if (pipe(pin) != 0) {
			return GLE_SYSTEM_ERROR;
		}
		if (pipe(pout) != 0) {
			return GLE_SYSTEM_ERROR;
		}
		fcntl(pin[GLESYS_PIPE_WR], F_SETFL, O_NONBLOCK);
		fcntl(pout[GLESYS_PIPE_RD], F_SETFL, O_NONBLOCK);
	}
	pid_t pid = fork();
	if (pid == 0) {
		/* here, we're in the child process */
		GLEDupFD(pin,  GLESYS_PIPE_RD, STDIN_FILENO);
		if (redirout && *pout >= 0) {
			/* redirect both stdout and stderr */
			close(pout[GLESYS_PIPE_RD]);
			dup2(pout[GLESYS_PIPE_WR], STDOUT_FILENO);
			dup2(pout[GLESYS_PIPE_WR], STDERR_FILENO);
			close(pout[GLESYS_PIPE_WR]);
		} else {
			/* redirect just stderr */
			GLEDupFD(pout, GLESYS_PIPE_WR, STDERR_FILENO);
		}
		execl("/bin/sh", "/bin/sh", "-c", cmd.c_str(), NULL);
		_exit(0);
	} else if (pid < 0) {
		GLECloseFDArray(fd);
		return GLE_SYSTEM_ERROR;
	} else if (wait) {
		int status;
		/* first close unused pipe ends */
		GLECloseFD(pin, GLESYS_PIPE_RD);
		if (ins == NULL) {
			GLECloseFD(pin, GLESYS_PIPE_WR);
		}
		GLECloseFD(pout, GLESYS_PIPE_WR);
		if (pin[GLESYS_PIPE_WR] >= 0) {
			/* don't crash with broken pipe signal */
			signal(SIGPIPE, SIG_IGN);
		}
		/* then read/write to used ones (these functions close when done) */
		fd_set read_set, write_set;
		char read_buffer[GLEREAD_BUF+1], write_buffer[GLEREAD_BUF+1];
		int to_write = 0, offs_write = 0;
		while (true) {
			/* write data until EAGAIN error (this means pipe full) */
			while (pin[GLESYS_PIPE_WR] >= 0) {
				if (to_write == 0) {
					offs_write = 0;
					if (ins->good()) {
						ins->read(write_buffer, GLEREAD_BUF);
						to_write = ins->gcount();
						if (to_write == 0) {
							GLECloseFD(pin, GLESYS_PIPE_WR);
							break;
						}
					} else {
						GLECloseFD(pin, GLESYS_PIPE_WR);
						break;
					}
				}
				int nbytes = write(pin[GLESYS_PIPE_WR], write_buffer+offs_write, to_write);
				if (nbytes < 0) {
					/* an error occurred */
					if (errno != EAGAIN) {
						GLECloseFD(pin, GLESYS_PIPE_WR);
					}
					break;
				} else {
					offs_write += nbytes;
					to_write -= nbytes;
				}
			}
			/* read data until EAGAIN error (this means pipe empty) */
			while (pout[GLESYS_PIPE_RD] >= 0) {
				int nbytes = read(pout[GLESYS_PIPE_RD], read_buffer, GLEREAD_BUF);
				if (nbytes < 0) {
					/* an error occurred */
					if (errno != EAGAIN) {
						GLECloseFD(pout, GLESYS_PIPE_RD);
					}
					break;
				} else if (nbytes == 0) {
					/* end of file reached */
					GLECloseFD(pout, GLESYS_PIPE_RD);
					break;
				} else if (outerrs != NULL) {
					read_buffer[nbytes] = 0;
					nbytes = str_remove_all(read_buffer, '\r');
					outerrs->write(read_buffer, nbytes);
				}
			}
			/* initialize descriptor sets */
			int cnt = 0;
			FD_ZERO(&read_set);
			FD_ZERO(&write_set);
			if (pout[GLESYS_PIPE_RD] >= 0)	{
				FD_SET(pout[GLESYS_PIPE_RD], &read_set);
				cnt++;
			}
			if (pin[GLESYS_PIPE_WR] >= 0) {
				FD_SET(pin[GLESYS_PIPE_WR], &write_set);
				cnt++;
			}
			if (cnt == 0) {
				/* everything error or end of stream */
				break;
			}
			/* wait for data to become available or flushed */
			int res = select(FD_SETSIZE, &read_set, &write_set, NULL, NULL);
			if (res < 1) {
				/* no stream ready */
				break;
			}
		}
		GLECloseFDArray(fd);
		waitpid(pid, &status, 0);
	}
	return GLE_SYSTEM_OK;
}

#endif

void GLESleep(int msec) {
#ifdef __WIN32__
	Sleep(msec);
#else
	sleep(msec/1000);
#endif
}

bool GLEFileExists(const string& fname) {
	FILE* f = fopen(fname.c_str(), "rb");
	if (f != NULL) {
		fclose(f);
		return true;
	} else {
		return false;
	}
}

// Create new directory
void MakeDirectory(const string& dir) {
	/* Create directory user read/write/exec, group & others read/exec */
#ifdef __WIN32__
	SECURITY_ATTRIBUTES sec_attr;
	sec_attr.nLength = sizeof(SECURITY_ATTRIBUTES);
	sec_attr.lpSecurityDescriptor = NULL;
	sec_attr.bInheritHandle = FALSE;
	CreateDirectory(dir.c_str(), &sec_attr);
#else
	mkdir(dir.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
#endif
}

// Recursively create directory if it does not yet exist
void EnsureMkDir(const string& dir) {
	if (!IsDirectory(dir)) {
		int done = 0;
		string temp = dir;
		vector<string> comps;
		do {
			string::size_type i = temp.rfind(DIR_SEP);
			if (i != string::npos) {
				comps.push_back(temp.substr(i+1));
				temp = temp.substr(0, i);
			} else {
				comps.push_back(temp);
				done = 1;
			}
		} while (done == 0 && (!IsDirectory(temp)));
		if (done == 0) {
			temp += DIR_SEP;
		} else {
			temp = "";
		}
		for (int i = comps.size()-1; i >= 0; i--) {
			temp += comps[i];
			MakeDirectory(temp);
			if (i > 0) temp += DIR_SEP;
		}
	}
}

bool TryDeleteDir(const string& fname) {
	// cout << "Delete: '" << fname << "'" << endl;
#ifdef __WIN32__
	RemoveDirectory(fname.c_str());
#else
	if (rmdir(fname.c_str()) != 0) {
		return false;
	}
#endif
	return true;
}

bool TryDeleteFile(const string& fname) {
	// cout << "Delete: '" << fname << "'" << endl;
#ifdef __WIN32__
	DeleteFile(fname.c_str());
#else
	if (unlink(fname.c_str()) != 0) {
		return false;
	}
#endif
	return true;
}

// Delete file with extension replaced by given one
bool DeleteFileWithNewExt(const string& fname, const char* ext) {
	string main_name;
	GetMainName(fname, main_name);
	main_name += ext;
	return TryDeleteFile(main_name);
}

// Delete file with extension replaced by given one
bool DeleteFileWithExt(const string& fname, const char* ext) {
	string name(fname);
	name += ext;
	return TryDeleteFile(name);
}

// Read line from file
int ReadFileLine(istream& file, string& line) {
	line = "";
	int count = 0;
	char ch = '\n';
	while ((ch == '\n' || ch == '\r') && file.good()) {
		file.read(&ch, 1);
	}
	while (ch != '\n' && ch != '\r' && file.good()) {
		count++;
		line += ch;
		file.read(&ch, 1);
	}
	return count;
}

int ReadFileLineAllowEmpty(istream& file, string& line) {
	line = "";
	char ch;
	int count = 0;
	file.read(&ch, 1);
	while (ch != '\n' && ch != '\r' && !file.eof()) {
		count++;
		line += ch;
		file.read(&ch, 1);
	}
	return count;
}

bool IsExecutable(const string& fname) {
#if defined(__UNIX__) || defined(__APPLE__) || ( defined(__OS2__) && defined(__EMX__) )
	struct stat stat_buf;
	if (stat((const char *)fname.c_str(), &stat_buf) == 0) {
		return (stat_buf.st_mode & S_IXOTH) != 0;
	} else {
		return false;
	}
#else
	return true;
#endif
}

bool IsDirectory(const string& fname, bool linkok) {
#if defined(__UNIX__) || defined(__APPLE__) || ( defined(__OS2__) && defined(__EMX__) )
	struct stat stat_buf;
	if (linkok) {
		if (stat((const char *)fname.c_str(), &stat_buf) == 0) {
			return S_ISDIR(stat_buf.st_mode);
		}
	} else {
		if (lstat((const char *)fname.c_str(), &stat_buf) == 0) {
			return S_ISDIR(stat_buf.st_mode);
		}
	}
#else
	struct _stat stat_buf;
	if (_stat((const char *)fname.c_str(), &stat_buf) == 0) {
		return (stat_buf.st_mode & _S_IFDIR);
	}
#endif
	return false;
}

GLEProgressIndicator::GLEProgressIndicator() {
}

GLEProgressIndicator::~GLEProgressIndicator() {
}

void GLEProgressIndicator::indicate() {
}

GLEFindEntry::GLEFindEntry(string* result) {
	m_Result = result;
	m_Done = false;
}

GLEFindEntry::~GLEFindEntry() {
}

void GLEFindEntry::addToFind(const string& tofind) {
	m_ToFind.push_back(tofind);
	m_Found.push_back(string());
}

void GLEFindEntry::updateResult(bool isFinal) {
	for (unsigned int i = 0; i < getNbFind(); i++) {
		if (!m_Done && m_Found[i] != "") {
			// The first one is to be preferred
			*m_Result = m_Found[i];
			m_Done = true;
			break;
		}
	}
	if (!m_Done && isFinal && m_NotFound != "") {
		*m_Result = m_NotFound;
	}
}

void GLEFindEntry::setFound(unsigned int i, const string& found) {
	unsigned int len = m_Result->length();
	if (len >= 1 && (*m_Result)[len-1] == ';') {
		// If result ends with ";", then find all results
		if (len == 1) *m_Result = found + ";";
		else *m_Result += found + ";";
	} else {
		// Only update if not yet found
		if (!m_Done && m_Found[i] == "") m_Found[i] = found;
	}
}

void GLEFindFilesUpdate(const char* fname, const string& dir, vector<GLEFindEntry*>& tofind) {
	for (vector<string>::size_type i = 0; i < tofind.size(); i++) {
		GLEFindEntry* entry = tofind[i];
		for (unsigned int j = 0; j < entry->getNbFind(); j++) {
			if (str_i_equals(fname, entry->getFind(j).c_str())) {
				string place = dir + DIR_SEP + fname;
				if (IsExecutable(place)) {
					entry->setFound(j, place);
				}
			}
		}
	}
}

void GLEFindFiles(const string& dirname, vector<GLEFindEntry*>& tofind, GLEProgressIndicator* progress) {
	static int find_files_progress = 0;
	vector<string> subdirs;
	if (find_files_progress++ == 10) {
		progress->indicate();
		find_files_progress = 0;
	}
	// cout << "Dir: " << dirname << endl;
#ifdef __WIN32__
	WIN32_FIND_DATA FindFileData;
	string findpattern = dirname + DIR_SEP + "*.*";
	HANDLE hFind = FindFirstFile(findpattern.c_str(), &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) {
		return;
	}
	do {
		const char* name = FindFileData.cFileName;
		if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
			if (!str_i_equals(name, ".") && !str_i_equals(name, "..")) {
				subdirs.push_back(name);
			}
		} else {
			GLEFindFilesUpdate(name, dirname, tofind);
		}
	} while (FindNextFile(hFind, &FindFileData) != 0);
	FindClose(hFind);
#endif
#if defined(__UNIX__) || defined(__APPLE__)
	DIR* dir = opendir(dirname.c_str());
	if (dir != NULL) {
		struct dirent* entry = readdir(dir);
		while (entry != NULL) {
			string path = dirname + DIR_SEP + entry->d_name;
			if (IsDirectory(path, false)) {
				if (!str_i_equals(entry->d_name, ".") && !str_i_equals(entry->d_name, "..")) {
					subdirs.push_back(entry->d_name);
				}
				if (str_i_str(entry->d_name, ".framework") != NULL) {
					GLEFindFilesUpdate(entry->d_name, dirname, tofind);
				}
			} else {
				GLEFindFilesUpdate(entry->d_name, dirname, tofind);
			}
			entry = readdir(dir);
		}
		closedir(dir);
	}
#endif
	for (vector<string>::size_type i = 0; i < subdirs.size(); i++) {
		string nextdir = dirname + DIR_SEP + subdirs[i];
		GLEFindFiles(nextdir, tofind,  progress);
	}
}

string GLEFindLibrary(const char* name, GLEProgressIndicator* progress) {
#ifdef __UNIX__
	string libpath;
	const char* ldlibpath = getenv("LD_LIBRARY_PATH");
	if (ldlibpath != NULL && ldlibpath[0] != 0) {
		libpath = ldlibpath;
		libpath += ":";
	}
	#ifdef __x86_64__
		libpath += "/usr/lib64:/usr/local/lib64:";
	#endif
	libpath += "/usr/lib:/usr/local/lib";
	string tofind = name;
	tofind += ".";
	char_separator separator(":", "");
	tokenizer<char_separator> tokens(libpath, separator);
	while (tokens.has_more()) {
		progress->indicate();
		const string& dirname = tokens.next_token();
		DIR* dir = opendir(dirname.c_str());
		if (dir != NULL) {
			struct dirent* entry = readdir(dir);
			while (entry != NULL) {
				string fname = entry->d_name;
				if (str_starts_with(fname, tofind.c_str()) && str_i_str(fname, ".so") != -1) {
					string result = dirname + DIR_SEP + fname;
					return result;
				}
#ifdef __APPLE__
				if (str_starts_with(fname, tofind.c_str()) && str_i_str(fname, ".dylib") != -1) {
					string result = dirname + DIR_SEP + fname;
					return result;
				}
#endif
				entry = readdir(dir);
			}
			closedir(dir);
		}
	}
#endif
	return string("");
}

void GLEFindPrograms(vector<GLEFindEntry*>& tofind, GLEProgressIndicator* progress) {
#if defined(__UNIX__) || defined(__APPLE__)
	const char* path = getenv("PATH");
	if (path == NULL) {
		return;
	}
	char_separator separator(":", "");
	tokenizer<char_separator> tokens(path, separator);
	while (tokens.has_more()) {
		progress->indicate();
		const string& dirname = tokens.next_token();
		DIR* dir = opendir(dirname.c_str());
		if (dir != NULL) {
			struct dirent* entry = readdir(dir);
			while (entry != NULL) {
				GLEFindFilesUpdate(entry->d_name, dirname, tofind);
				entry = readdir(dir);
			}
			closedir(dir);
		}
	}
#endif
}

string GetActualFilename(ifstream* file, const string& fname, const string* directory) {
	// will return the filename
	// if found in ./ or in any of the include
	// paths, will return empty string if
	// file not found
	if (directory == NULL) {
		file->open(fname.c_str());
		if (file->is_open()) return fname;
	} else {
		string fullpath;
		GLEGetFullPath(*directory, fname, fullpath);
		file->open(fullpath.c_str());
		if (file->is_open()) return fullpath;
	}
	// -- try the IncludePaths
	vector<string> IncludePaths;
	FillIncludePaths(IncludePaths);
	vector<string>::iterator vsi = IncludePaths.begin();
	while(vsi != IncludePaths.end()) {
		file->clear();
		const string path = *vsi + DIR_SEP + fname;
		file->open(path.c_str());
		if (file->is_open()) return path;
		vsi++;
	}
	return "";
}

#ifdef HAVE_SOCKETS
void GLECloseSocket(SOCKET sock) {
#ifdef __WIN32__
	closesocket(sock);
	WSACleanup();
#endif
#if defined(__UNIX__) || defined(__APPLE__)
	close(sock);
#endif
}
#endif

int GLESendSocket(const string& commands) {
#ifdef HAVE_SOCKETS
#ifdef __WIN32__
	struct WSAData wsaData;
	int nCode = WSAStartup(MAKEWORD(1, 1), &wsaData);
	if (nCode != 0) {
		return -1;
	}
#endif
	struct sockaddr_in name;
	SOCKET sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock < 0) {
		#ifdef __WIN32__
		WSACleanup();
		#endif
		return -2;
	}
	name.sin_family = AF_INET;
	name.sin_port = htons(6667);
	name.sin_addr.s_addr = inet_addr("127.0.0.1");
	if (connect(sock, (struct sockaddr *) &name, sizeof (name)) < 0) {
		GLECloseSocket(sock);
		return -3;
	}
	int nbsend = send(sock, commands.c_str(), commands.length(), 0);
	if (nbsend != (int)commands.length()) {
		GLECloseSocket(sock);
		return -4;
	}
	char ch = 0;
	while (true) {
#ifdef __WIN32__
		int res1 = recv(sock, &ch, 1, 0);
		if (res1 <= 0) {
			if (res1 == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK) {
#else
		int res1 = read(sock, &ch, 1);
		if (res1 <= 0) {
			if (res1 == -1 && errno == EAGAIN) {
#endif
				fd_set read_set;
				FD_ZERO(&read_set);
				FD_SET(sock, &read_set);
				int res2 = select(FD_SETSIZE, &read_set, NULL, NULL, NULL);
				if (res2 < 1) break;
			} else {
				break;
			}
		} else {
			cerr << ch;
		}
	}
	GLECloseSocket(sock);
	return 0;
#else
	return -1;
#endif
}

bool GetExeName(const char* appname, char **argv, string& exe_name) {
#ifdef __WIN32__
	char name[1024];
	DWORD res = GetModuleFileName(NULL, name, 1023);
	if (res > 0) {
		name[res] = 0;
		exe_name = name;
		return true;
	}
#endif
#ifdef __APPLE__
	char name[1024];
	uint32_t path_len = 1023;
	if (_NSGetExecutablePath(name, &path_len) == 0) {
		exe_name = name;
		str_replace_all(exe_name, "/./", "/");
		return true;
	}
#endif
#if defined(__UNIX__) && !defined(__APPLE__) && !defined(__FREEBSD__)
	/* try to read location from the /proc/self/exe file */
	char path[PATH_MAX];
	struct stat stat_buf;
	string path2 = "/proc/self/exe";
	while (true) {
		int size = readlink(path2.c_str(), path, PATH_MAX - 1);
		if (size == -1) {
			break;
		}
		path[size] = '\0';
		int i = stat(path, &stat_buf);
		if (i == -1) {
			break;
		}
		if (!S_ISLNK(stat_buf.st_mode)) {
			exe_name = path;
			return true;
		}
		path2 = path;
	}
	/* try to read location from the /proc/self/maps file */
	ifstream maps("/proc/self/maps");
	if (maps.is_open()) {
		string f1 = DIR_SEP + appname;
		string f2 = f1 + ".exe";
		while (!maps.eof()) {
			string line;
			ReadFileLine(maps, line);
			char_separator separator(" ", "");
			tokenizer<char_separator> tokens(line, separator);
			while (tokens.has_more()) {
				exe_name = tokens.next_token();
				if (str_i_ends_with(exe_name, f1.c_str())
				    || str_i_ends_with(exe_name, f2.c_str())) {
					return true;
				}
			}
		}
		maps.close();
	}
#endif
	string arg0 = argv[0];
	if (IsAbsPath(arg0)) {
		exe_name = arg0;
		return true;
	}
	if (GLEGetCrDir(&exe_name)) {
		AddDirSep(exe_name);
		exe_name += arg0;
		GLENormalizePath(exe_name);
		return true;
	}
	return false;
}

string GetHomeDir() {
	#if defined(__UNIX__) || defined(__APPLE__) || defined (__OS2__)
		const char* home = getenv("HOME");
		if (home != NULL && home[0] != 0) {
			string home_str(home);
			AddDirSep(home_str);
			return home_str;
		}
	#endif
	return string();
}

StreamTokenizerMax::StreamTokenizerMax(const string& fname, int sep, int max) : m_File(fname.c_str()) {
	m_Sep = sep; m_Max = max; m_IsOK = 1;
	m_LastToken = new char[m_Max+1];
	if (!m_File.is_open()) {
		m_IsOK = 0;
	}
}

StreamTokenizerMax::~StreamTokenizerMax() {
	delete[] m_LastToken;
}

bool StreamTokenizerMax::hasMoreTokens() {
	if (m_IsOK == 1) readNextToken();
	return m_IsOK == 1;
}

const char* StreamTokenizerMax::nextToken() {
	return m_LastToken;
}

void StreamTokenizerMax::close() {
	m_File.close();
}

bool StreamTokenizerMax::isSepChar(char ch) {
	return ch == m_Sep || ch == '\n' || ch == '\r' || ch == 0;
}

void StreamTokenizerMax::readNextToken() {
	char ch = m_Sep;
	while (isSepChar(ch) && !m_File.eof()) {
		m_File.read(&ch, 1);
	}
	int count = 0;
	while (count < m_Max && !isSepChar(ch) && !m_File.eof()) {
		if (ch != m_Sep) m_LastToken[count++] = ch;
		m_File.read(&ch, 1);
	}
	m_LastToken[count] = 0;
	while (!isSepChar(ch) && !m_File.eof()) {
		m_File.read(&ch, 1);
	}
	if (m_File.eof()) {
		m_IsOK = 0;
	}
}

void CopyGLETop(char* buf) {
	strcpy(buf, GLE_TOP_DIR.c_str());
}

string fontdir(const char *fname) {
	// simply returns %GLE_TOP%/font
	string result = GLE_TOP_DIR;
	result += DIR_SEP;
	result += "font";
	result += DIR_SEP;
	result += fname;
	return result;
}

string gledir(const char *fname) {
	// simply returns %GLE_TOP%
	string result = GLE_TOP_DIR;
	result += DIR_SEP;
	result += fname;
	return result;
}

string GLEExpandEnvironmentVariables(const string& str) {
	ostringstream result;
	for (unsigned int i = 0; i < str.size(); i++) {
		if (str[i] == '$') {
			string envVar;
			for (unsigned int j = i+1; j < str.size(); j++) {
				if (toupper(str[j]) >= 'A' && toupper(str[j]) <= 'Z') {
					envVar += str[j];
				} else {
					break;
				}
			}
			bool replaced = false;
			if (!envVar.empty()) {
				char* var = getenv(envVar.c_str());
				if (var != NULL) {
					replaced = true;
					result << var;
				}
			}
			if (!replaced) {
				result << "$";
				result << envVar;
			}
			i += envVar.size();
		} else {
			result << str[i];
		}
	}
	return result.str();
}

bool GLEReadFile(const string& name, vector<string>* lines) {
	ifstream inFile(name.c_str());
	if (!inFile.is_open()) {
		return false;
	}
	while (inFile.good()) {
		string line;
		getline(inFile, line);
		lines->push_back(line);
	}
	inFile.close();
	return true;
}

bool GLEReadFileBinary(const string& name, std::vector<char>* contents) {
	ifstream inFile(name.c_str(), ios::in | ios::binary | ios::ate);
	if (!inFile.is_open()) {
		return false;
	}
	int length = inFile.tellg();
	inFile.seekg(0, ios::beg);
	contents->resize(length, 0);
	inFile.read(&(*contents)[0], length);
	inFile.close();
	return true;
}

GLEFileIO::GLEFileIO():
	m_file(0)
{
}

GLEFileIO::~GLEFileIO()
{
}

void GLEFileIO::open(const char* fname, const char* flags)
{
	m_fname = fname;
	m_file = fopen(fname, flags);
}

bool GLEFileIO::isOpen() const
{
	return m_file != 0;
}

void GLEFileIO::close()
{
	if (m_file != 0) {
		fclose(m_file);
		m_file = 0;
	}
}

void GLEFileIO::fread(void *ptr, size_t size, size_t nmemb)
{
	size_t nbRead = ::fread(ptr, size, nmemb, m_file);
	if (nbRead != nmemb) {
		std::cerr << "error reading from file '" << m_fname << "'";
	}
}

void GLEFileIO::fwrite(const void *ptr, size_t size, size_t nmemb)
{
	size_t nbWritten = ::fwrite(ptr, size, nmemb, m_file);
	if (nbWritten != nmemb) {
		std::cerr << "error writing to file '" << m_fname << "'";
	}
}

FILE* GLEFileIO::getFile()
{
	return m_file;
}

std::string GLEFileIO::getName() const
{
	return m_fname;
}

void GLEFileIO::setName(const std::string& name)
{
	m_fname = name;
}

int GLEFileIO::fgetc()
{
	return ::fgetc(m_file);
}

void GLEFileIO::fputc(int value)
{
	::fputc(value, m_file);
}

int GLEFileIO::feof()
{
	return ::feof(m_file);
}

long GLEFileIO::ftell()
{
	return ::ftell(m_file);
}

int GLEFileIO::fseek(long offset, int whence)
{
	return ::fseek(m_file, offset, whence);
}

void GLEFileIO::fgetcstr(char* s) {
	int i = fgetc();
	if (i != 0) {
		fread(s, 1, i);
		s[i] = 0;
	}
}

void GLEFileIO::fsendstr(const char *s) {
	if (s == NULL) {
		::fputc(0, m_file);
	} else {
		::fputc(strlen(s), m_file);
		fwrite(s, 1, strlen(s));
	}
}

int GLEReadConsoleInteger()
{
	char* ptr = 0;
	std::string line;
	getline(std::cin, line);
	int result = strtol(line.c_str(), &ptr, 10);
	if (ptr == 0 || *ptr != 0) {
		return 0;
	} else {
		return result;
	}
}


