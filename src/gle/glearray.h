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

#ifndef _GLE_ARRAY_H_
#define _GLE_ARRAY_H_

class GLEObject : public RefCountObject {
public:
	GLEObject();
	virtual ~GLEObject();
	virtual int size();
	virtual double getDoubleAt(int i);
	virtual void setDoubleAt(double v, int i);
	virtual bool getBoolAt(int i);
	virtual void setBoolAt(bool v, int i);
	virtual GLEObject* getObjectAt(int i);
	virtual void setObjectAt(GLEObject* v, int i);
};

class GLEObjectArray : public GLEObject {
protected:
	RefCountVector<GLEObject> m_Elems;
public:
	GLEObjectArray();
	virtual ~GLEObjectArray();
	virtual int size();
	virtual GLEObject* getObjectAt(int i);
	virtual void setObjectAt(GLEObject* v, int i);
protected:
	void resize(int n);
};

class GLEDoubleArray : public GLEObject {
protected:
	vector<double> m_Elems;
public:
	GLEDoubleArray();
	virtual ~GLEDoubleArray();
	virtual int size();
	virtual double getDoubleAt(int i);
	virtual void setDoubleAt(double v, int i);
	double* toArray();
protected:
	void resize(int n);
};

class GLEBoolArray : public GLEObject {
protected:
	vector<bool> m_Elems;
public:
	GLEBoolArray();
	virtual ~GLEBoolArray();
	virtual int size();
	virtual bool getBoolAt(int i);
	virtual void setBoolAt(bool v, int i);
	int* toArray();
protected:
	void resize(int n);
};

class GLEZData {
protected:
	GLERectangle m_XYBounds;
	double m_ZMin, m_ZMax;
	int m_NX, m_NY;
	double* m_Data;
public:
	GLEZData();
	~GLEZData();
	void read(const string& fname) throw(ParserError);
	inline double getZMin() { return m_ZMin; }
	inline double getZMax() { return m_ZMax; }
	inline GLERectangle* getBounds() { return &m_XYBounds; }
	inline int getNX() { return m_NX; }
	inline int getNY() { return m_NY; }
	inline double* getData() { return m_Data; }
};

bool GLEReadFileOrGZIPTxt(const std::string& name, std::string* result);

typedef unsigned char GLEBYTE;

enum GLECSVDataStatus {
	GLECSVDataStatusOK,
	GLECSVDataStatusEOL,
	GLECSVDataStatusEOF
};

enum GLECSVErrorCode {
	GLECSVErrorNone,
	GLECSVErrorFileNotFound,
	GLECSVErrorUnterminatedString,
	GLECSVErrorInconsistentNrColumns
};

class GLECSVError {
public:
	GLECSVError();
	~GLECSVError();
	GLECSVErrorCode errorCode;
	unsigned int errorLine;
	unsigned int errorColumn;
	string errorString;
};

class GLECSVData {
protected:
	vector<GLEBYTE> m_buffer;
	vector<unsigned int> m_cellPos;
	vector<unsigned int> m_cellSize;
	vector<unsigned int> m_firstCell;
	GLEBYTE* m_data;
	bool* m_delims;
	unsigned int m_size;
	unsigned int m_pos;
	unsigned int m_writePos;
	unsigned int m_lines;
	unsigned int m_firstColumn;
	unsigned int m_nextLine;
	unsigned int m_ignoreHeader;
	GLECSVError m_error;
	string m_fileName;
	string m_comment;
	bool m_lastDelimWasSpace;
public:
	GLECSVData();
	~GLECSVData();
	bool read(const std::string& file);
	void readBuffer(const char* buffer);
	void print(ostream& os);
	void setDelims(const char* delims);
	void setCommentIndicator(const char* comment);
	void setIgnoreHeader(unsigned int ignore);
	GLECSVError* getError();
	unsigned int getNbLines();
	unsigned int getNbColumns(unsigned int line);
	const char* getCell(unsigned int row, unsigned int column, unsigned int* size);
	string getCellString(unsigned int row, unsigned int column);
	void setCellTrim(unsigned int row, unsigned int column, const char* data);
	unsigned int validateIdenticalNumberOfColumns();
private:
	bool readBlock(const std::string& file);
	void parseBlock();
	void initDelims();
	bool isDelim(GLEBYTE ch);
	bool isSpace(GLEBYTE ch);
	bool isEol(GLEBYTE ch);
	bool isComment(GLEBYTE ch);
	GLECSVDataStatus skipTillEol();
	GLECSVDataStatus ignoreHeader();
	GLECSVDataStatus readCellString(GLEBYTE quote);
	GLECSVDataStatus readCell();
	void createCell(unsigned int cellSize, unsigned int cellPos);
	GLECSVDataStatus readNewline(GLEBYTE prevCh);
	bool isSizeCheckOKEndOfLine(unsigned int cellSize);
	bool isSizeCheckOKAtDelim(GLEBYTE delim, unsigned int cellSize);
	void goBack();
	unsigned int lastCharPos();
	GLEBYTE readChar();
	GLEBYTE readSignificantChar();
	void initWritePos();
	void writeChar(GLEBYTE ch);
	GLECSVDataStatus skipSpacesAndFirstDelim(GLEBYTE ch);
	void createErrorString(const string& str);
	unsigned int getUTF8Column(unsigned int cellPos);
	unsigned int getFirstCell(unsigned int line);
};

#endif
