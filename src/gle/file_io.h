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

#ifndef INCLUDE_FILE_IO
#define INCLUDE_FILE_IO

#if defined(__UNIX__) || defined(__MAC__)
	#define READ_BIN "r"
	#define WRITE_BIN "w"
#endif

#ifdef __WIN32__
	#define READ_BIN "rb"
	#define WRITE_BIN "wb"
#endif

#if defined(__OS2__) && defined(__EMX__)
	#define READ_BIN "rb"
	#define WRITE_BIN "wb"
#endif

#ifndef IN_FILE_IO_H
	extern string PATH_SEP;
	extern string DIR_SEP;
#endif

class GLEProgressIndicator {
public:
	GLEProgressIndicator();
	virtual ~GLEProgressIndicator();
	virtual void indicate();
};

class GLEFindEntry {
protected:
	vector<string> m_ToFind;
	vector<string> m_Found;
	string* m_Result;
	string m_NotFound;
	bool m_Done;
public:
	GLEFindEntry(string* result);
	~GLEFindEntry();
	void addToFind(const string& tofind);
	void updateResult(bool isFinal);
	void setFound(unsigned int i, const string& found);
	inline void setNotFound(const string& notfound) { m_NotFound = notfound; }
	inline unsigned int getNbFind() { return m_ToFind.size(); }
	inline const string& getFind(unsigned int i) { return m_ToFind[i]; }
};

string fontdir(const char *fname);
char *line(int i);

bool GLEGetEnv(const string& name, string& result);
void CopyGLETop(char* buf);
string GetActualFilename(ifstream* file, const string& fname, const string* directory);
void FillIncludePaths(vector<string>& IP);
void GLEPathToVector(const string& path, vector<string>* vec);
bool GLEStreamContains(istream& strm, const char* msg);
void StripDirSep(string& fname);
void StripDirSepButNotRoot(string& fname);
void CorrectDirSep(string& fname);
void CorrectDirSepStrip(string& fname);
void AddDirSep(string& fname);
string GLEAddRelPath(const string& base, int cd, const char* path);
bool GLEAddRelPathAndFileTry(const string& base, int cd, const char* path, const char* file, string& result);
void FileNameDotToUnderscore(string& fname);
void GetExtension(const string& fname, string& ext);
void GetMainName(const string& fname, string& name);
void GetMainNameExt(const string& fname, const char* ext, string& name);
void AddExtension(string& fname, const string& ext);
void SplitFileName(const string& fname, string& dir, string& name);
void SplitFileNameNoDir(const string& fname, string& name);
void RemoveDirectoryIfEqual(string* filename, const string& directory);
void GetDirName(const string& fname, string& dir);
void EnsureMkDir(const string& dir);
bool TryDeleteDir(const string& fname);
bool TryDeleteFile(const string& fname);
bool DeleteFileWithExt(const string& fname, const char* ext);
bool DeleteFileWithNewExt(const string& fname, const char* ext);
int ReadFileLine(istream& file, string& line);
int ReadFileLineAllowEmpty(istream& file, string& line);
bool IsDirectory(const string& fname, bool linkok = true);
bool IsExecutable(const string& fname);
string GLETempName();
string GLETempDirName();
bool GLEMoveFile(const string& from, const string& to);
void GLECopyStream(istream& from, ostream& to);
int GLECopyFile(const string& from, const string& to, string* err = NULL);
bool GLEGetCrDir(string* name);
bool GLEGetCrDirWin32(string* name);
bool GLEChDir(const string& dir);
void GLEFindFiles(const string& dir, vector<GLEFindEntry*>& tofind, GLEProgressIndicator* progress);
string GLEFindLibrary(const char* name, GLEProgressIndicator* progress);
void GLEFindPrograms(vector<GLEFindEntry*>& tofind, GLEProgressIndicator* progress);
int GLESystem(const string& cmd, bool wait = true, bool redirout = true, istream* ins = NULL, ostream* outerrs = NULL);
int GLERunCommand(const string& cmd, string& result);
int GLESendSocket(const string& commands);
bool GetExeName(const char* appname, char **argv, string& exe_name);
string GetHomeDir();
void GLESetGLETop(const string& cmdline);
bool GLEFileExists(const string& fname);
void GLESleep(int msec);
void StripPathComponents(string* fname, int nb);
bool IsAbsPath(const string& path);
void GLEGetFullPath(const string& dirname, const string& fname, string& fullpath);
string GLEExpandEnvironmentVariables(const string& str);
bool GLEReadFile(const string& name, vector<string>* lines);
bool GLEReadFileBinary(const string& name, std::vector<char>* contents);

#define GLE_SYSTEM_OK            0
#define GLE_SYSTEM_ERROR         1
#define GLE_FILE_OK              0
#define GLE_FILE_WRITE_ERROR     2
#define GLE_FILE_READ_ERROR      3
#define GLE_FILE_NOT_FOUND_ERROR 4

class StreamTokenizerMax {
protected:
	char* m_LastToken;
	int m_Sep, m_Max, m_IsOK;
	ifstream m_File;
public:
	StreamTokenizerMax(const string& fname, int sep, int max);
	~StreamTokenizerMax();
	bool hasMoreTokens();
	bool isSepChar(char ch);
	const char* nextToken();
	void readNextToken();
	void close();
	inline ifstream& getFile() { return m_File; }
};

class GLEFileIO {
public:
	GLEFileIO();
	~GLEFileIO();
	void open(const char* fname, const char* flags);
	bool isOpen() const;
	void close();
	FILE* getFile();
	std::string getName() const;
	void setName(const std::string& name);
	void fread(void *ptr, size_t size, size_t nmemb);
	void fwrite(const void *ptr, size_t size, size_t nmemb);
	int fgetc();
	void fputc(int value);
	int feof();
	long ftell();
	int fseek(long offset, int whence);
	void fgetcstr(char* s);
	void fsendstr(const char* s);

private:
	FILE* m_file;
	std::string m_fname;
};

int GLEReadConsoleInteger();

#endif
