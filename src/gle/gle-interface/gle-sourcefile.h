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

#ifndef __GLE_SOURCEFILE__
#define __GLE_SOURCEFILE__

class GLESourceFile;

class GLESourceLine {
protected:
	bool m_Delete;
	int m_GlobalLineNo;
	int m_LineNo;
	std::string m_Code;
	std::string m_Prefix;
	GLESourceFile* m_File;
public:
	GLESourceLine();
	~GLESourceLine();
	inline void setCode(const std::string& code) { m_Code = code; }
	inline const std::string& getCode() { return m_Code; }
	inline const char* getCodeCStr() { return m_Code.c_str(); }
	inline void setPrefix(const std::string& prefix) { m_Prefix = prefix; }
	inline const std::string& getPrefix() { return m_Prefix; }
	inline int getLineNo() { return m_LineNo; }
	inline void setLineNo(int no) { m_LineNo = no; }
	inline int getGlobalLineNo() { return m_GlobalLineNo; }
	inline void setGlobalLineNo(int no) { m_GlobalLineNo = no; }
	inline void setSource(GLESourceFile* file) { m_File = file; }
	inline GLESourceFile* getSource() { return m_File; }
	inline void setDelete(bool del) { m_Delete = del; }
	inline bool isDelete() { return m_Delete; }
	bool isEmpty();
	const std::string& getFileName();
	int showLineAbbrev(std::ostream& out, int focuscol);
};

class GLESourceFile {
protected:
	GLEFileLocation m_File;
	std::vector<GLESourceLine*> m_Code;
	std::vector<int> m_ToInsertIdx;
	std::vector<std::string> m_ToInsertLine;
	GLERCVector<GLEObjectDOConstructor> m_Cons;
public:
	GLESourceFile();
	~GLESourceFile();
	inline int getNbLines() { return m_Code.size(); }
	inline GLESourceLine* getLine(int i) { return m_Code[i]; }
	inline GLEFileLocation* getLocation() { return &m_File; }
	void trim(int add = 0);
	void clear();
	void reNumber();
	GLESourceLine* addLine();
	void scheduleInsertLine(int i, const std::string& str);
	void performUpdates();
	int getNextInsertIndex(int line, int pos);
	void load(std::istream& input);
	void load();
	bool tryLoad();
	inline void addObjectDOConstructor(GLEObjectDOConstructor* cons) { m_Cons.add(cons); }
	inline int getNbObjectDOConstructors() { return m_Cons.size(); }
	inline GLEObjectDOConstructor* getObjectDOConstructor(int i) { return m_Cons.get(i); }
	inline void clearObjectDOConstructors() { m_Cons.clear(); }
};

class GLEGlobalSource {
protected:
	GLESourceFile m_Main;
	std::vector<GLESourceFile*> m_Files;
	std::vector<GLESourceLine*> m_Code;
public:
	GLEGlobalSource();
	~GLEGlobalSource();
	inline GLESourceFile* getMainFile() { return &m_Main; }
	inline GLEFileLocation* getLocation() { return m_Main.getLocation(); }
	inline int getNbLines() { return m_Code.size(); }
	inline GLESourceLine* getLine(int i) { return m_Code[i]; }
	inline const std::string& getLineCode(int i) { return m_Code[i]->getCode(); }
	inline void addLine(GLESourceLine* line) { m_Code.push_back(line); }
	inline int getNbFiles() { return m_Files.size(); }
	inline GLESourceFile* getFile(int i) { return m_Files[i]; }
	void clear();
	GLESourceFile* createNew();
	bool includes(const std::string& file);
	void initFromMain();
	void insertInclude(int offs, GLESourceFile* file);
	void insertIncludeNoOverwrite(int offs, GLESourceFile* file);
	void reNumber();
	void addLine(const std::string& code);
	void updateLine(int i, const std::string& code);
	void scheduleDeleteLine(int i);
	void scheduleInsertLine(int i, const std::string& str);
	void performUpdates();
	void sourceLineFileAndNumber(int line, std::ostream& err);
	void load();
	bool tryLoad();
	void clearObjectDOConstructors();
};

int showLineAbbrev(const std::string& text, int focuscol, std::ostream& out);
void sourceLineFileAndNumber(int globalLine, std::ostream& out);

#endif

