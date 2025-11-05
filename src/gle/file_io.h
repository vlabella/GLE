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

#if defined(__unix__) || defined(__APPLE__)
	#define READ_BIN "r"
	#define WRITE_BIN "w"
#endif

#ifdef _WIN32
	#define READ_BIN "rb"
	#define WRITE_BIN "wb"
#endif

#if defined(__OS2__) && defined(__EMX__)
	#define READ_BIN "rb"
	#define WRITE_BIN "wb"
#endif

#ifndef IN_FILE_IO_H
	extern std::string PATH_SEP;
	extern std::string DIR_SEP;
#endif

class GLEProgressIndicator {
public:
	GLEProgressIndicator();
	virtual ~GLEProgressIndicator();
	virtual void indicate();
};

class GLEFindEntry {
protected:
	std::vector<std::string> m_ToFind;
	std::vector<std::string> m_Found;
	std::string* m_Result;
	std::string m_NotFound;
	bool m_Done;
public:
	GLEFindEntry(std::string* result);
	~GLEFindEntry();
	void addToFind(const std::string& tofind);
	void updateResult(bool isFinal);
	void setFound(unsigned int i, const std::string& found);
	inline void setNotFound(const std::string& notfound) { m_NotFound = notfound; }
	inline unsigned int getNbFind() { return m_ToFind.size(); }
	inline const std::string& getFind(unsigned int i) { return m_ToFind[i]; }
};

std::string fontdir(const char *fname);
char *line(int i);

bool GLEGetEnv(const std::string& name, std::string& result);
void CopyGLETop(char* buf);
std::string GetActualFilename(std::ifstream* file, const std::string& fname, const std::string* directory);
void FillIncludePaths(std::vector<std::string>& IP);
void GLEPathToVector(const std::string& path, std::vector<std::string>* vec);
bool GLEStreamContains(std::istream& strm, const char* msg);
void StripDirSep(std::string& fname);
void StripDirSepButNotRoot(std::string& fname);
void CorrectDirSep(std::string& fname);
void CorrectDirSepStrip(std::string& fname);
void AddDirSep(std::string& fname);
std::string GLEAddRelPath(const std::string& base, int cd, const char* path);
bool GLEAddRelPathAndFileTry(const std::string& base, int cd, const char* path, const char* file, std::string& result);
void FileNameDotToUnderscore(std::string& fname);
void GetExtension(const std::string& fname, std::string& ext);
void GetMainName(const std::string& fname, std::string& name);
void GetMainNameExt(const std::string& fname, const char* ext, std::string& name);
void AddExtension(std::string& fname, const std::string& ext);
void SplitFileName(const std::string& fname, std::string& dir, std::string& name);
void SplitFileNameNoDir(const std::string& fname, std::string& name);
void RemoveDirectoryIfEqual(std::string* filename, const std::string& directory);
void GetDirName(const std::string& fname, std::string& dir);
void EnsureMkDir(const std::string& dir);
bool TryDeleteDir(const std::string& fname);
bool TryDeleteFile(const std::string& fname);
bool DeleteFileWithExt(const std::string& fname, const char* ext);
bool DeleteFileWithNewExt(const std::string& fname, const char* ext);
int ReadFileLine(std::istream& file, std::string& line);
int ReadFileLineAllowEmpty(std::istream& file, std::string& line);
bool IsDirectory(const std::string& fname, bool linkok = true);
bool IsExecutable(const std::string& fname);
std::string GLETempName();
std::string GLETempDirName();
bool GLEMoveFile(const std::string& from, const std::string& to);
void GLECopyStream(std::istream& from, std::ostream& to);
int GLECopyFile(const std::string& from, const std::string& to, std::string* err = NULL);
bool GLEGetCrDir(std::string* name);
bool GLEGetCrDirWin32(std::string* name);
bool GLEChDir(const std::string& dir);
void GLEFindFiles(const std::string& dir, std::vector<GLEFindEntry*>& tofind, GLEProgressIndicator* progress);
std::string GLEFindLibrary(const char* libName, GLEProgressIndicator* progress, std::string symbol_name);
void GLEFindPrograms(std::vector<GLEFindEntry*>& tofind, GLEProgressIndicator* progress);
int GLESystem(const std::string& cmd, bool wait = true, bool redirout = true, std::istream* ins = NULL, std::ostream* outerrs = NULL);
int GLERunCommand(const std::string& cmd, std::string& result);
int GLESendSocket(const std::string& commands);
bool GetExeName(const char* appname, char **argv, std::string& exe_name);
std::string GetHomeDir();
void GLESetGLETop(const std::string& cmdline);
bool GLEFileExists(const std::string& fname);
void GLESleep(int msec);
void StripPathComponents(std::string* fname, int nb);
bool IsAbsPath(const std::string& path);
void GLEGetFullPath(const std::string& dirname, const std::string& fname, std::string& fullpath);
std::string GLEExpandEnvironmentVariables(const std::string& str);
bool GLEReadFile(const std::string& name, std::vector<std::string>* lines);
bool GLEReadFileBinary(const std::string& name, std::vector<char>* contents);

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
	std::ifstream m_File;
public:
	StreamTokenizerMax(const std::string& fname, int sep, int max);
	~StreamTokenizerMax();
	bool hasMoreTokens();
	bool isSepChar(char ch);
	const char* nextToken();
	void readNextToken();
	void close();
	inline std::ifstream& getFile() { return m_File; }
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
