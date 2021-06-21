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

/*
 * 2004 Jan Struyf
 *
 */

#include "all.h"
#include "core.h"
#include "file_io.h"
#include "texinterface.h"
#include "cutils.h"
#include "gprint.h"
#include "tokens/RefCount.h"
#include "glearray.h"

#ifdef HAVE_LIBZ
   #include <zlib.h>
#endif

#include <memory>

bool GLEReadFileBinaryGZIP(const string& name, std::vector<GLEBYTE>* contents);

RefCountObject::RefCountObject() {
	owner_count = 0;
}

RefCountObject::~RefCountObject() {
}

GLEObject::GLEObject() {
}

GLEObject::~GLEObject() {
}

int GLEObject::size() {
	return 0;
}

double GLEObject::getDoubleAt(int i) {
	return 0.0;
}

void GLEObject::setDoubleAt(double v, int i) {
}

bool GLEObject::getBoolAt(int i) {
	return false;
}

void GLEObject::setBoolAt(bool v, int i) {
}

GLEObject* GLEObject::getObjectAt(int i) {
	return NULL;
}

void GLEObject::setObjectAt(GLEObject* v, int i) {
}

GLEObjectArray::GLEObjectArray() {
}

GLEObjectArray::~GLEObjectArray() {
}

int GLEObjectArray::size() {
	return m_Elems.size();
}

GLEObject* GLEObjectArray::getObjectAt(int i) {
	if (i > (int)m_Elems.size()) return NULL;
	else return m_Elems[i].get();
}

void GLEObjectArray::setObjectAt(GLEObject* v, int i) {
	resize(i);
	m_Elems[i] = v;
}

void GLEObjectArray::resize(int n) {
	int add = n - m_Elems.size() + 1;
	while (add > 0) {
		m_Elems.push_back(NULL);
		add--;
	}
}

GLEDoubleArray::GLEDoubleArray() {
}

GLEDoubleArray::~GLEDoubleArray() {
}

int GLEDoubleArray::size() {
	return m_Elems.size();
}

double GLEDoubleArray::getDoubleAt(int i) {
	if (i > (int)m_Elems.size()) return 0.0;
	else return m_Elems[i];
}

void GLEDoubleArray::setDoubleAt(double v, int i) {
	resize(i);
	m_Elems[i] = v;
}

double* GLEDoubleArray::toArray() {
	double* res = (double*)myallocz(sizeof(double) * (m_Elems.size()+1));
	for (vector<double>::size_type i = 0; i < m_Elems.size(); i++) {
		res[i] = m_Elems[i];
	}
	return res;
}

void GLEDoubleArray::resize(int n) {
	int add = n - m_Elems.size() + 1;
	while (add > 0) {
		m_Elems.push_back(0.0);
		add--;
	}
}

GLEBoolArray::GLEBoolArray() {
}

GLEBoolArray::~GLEBoolArray() {
}

int GLEBoolArray::size() {
	return m_Elems.size();
}

bool GLEBoolArray::getBoolAt(int i) {
	if (i > (int)m_Elems.size()) return false;
	else return m_Elems[i];
}

void GLEBoolArray::setBoolAt(bool v, int i) {
	resize(i);
	m_Elems[i] = v;
}

int* GLEBoolArray::toArray() {
	int* res = (int*)myallocz(sizeof(int) * (m_Elems.size()+1));
	for (vector<bool>::size_type i = 0; i < m_Elems.size(); i++) {
		res[i] = m_Elems[i] ? 1 : 0;
	}
	return res;
}

void GLEBoolArray::resize(int n) {
	int add = n - m_Elems.size() + 1;
	while (add > 0) {
		m_Elems.push_back(false);
		add--;
	}
}

GLEZData::GLEZData() {
	m_ZMin = 1e300;
	m_ZMax = -1e300;
	m_NX = 0;
	m_NY = 0;
	m_Data = NULL;
}

GLEZData::~GLEZData() {
	if (m_Data != NULL) delete[] m_Data;
}

void GLEZData::read(const string& fname) throw(ParserError) {
	string expanded(GLEExpandEnvironmentVariables(fname));
	validate_file_name(expanded, false);
	TokenizerLanguage lang;
	std::auto_ptr<Tokenizer> tokens;
	std::vector<GLEBYTE> contents;
	if (str_i_ends_with(expanded, ".gz")) {
		if (GLEReadFileBinaryGZIP(expanded, &contents)) {
			contents.push_back(0);
			tokens.reset(new StringTokenizer((const char*)&contents[0], &lang));
		} else {
			g_throw_parser_error("can't open: '", expanded.c_str(), "'");
		}
	} else {
		StreamTokenizer* streamTokens = new StreamTokenizer(&lang);
		tokens.reset(streamTokens);
		streamTokens->open_tokens(expanded.c_str());
	}
	lang.setSpaceTokens(" \t\r,");
	lang.setSingleCharTokens("\n!");
	// Read the header of the z file
	GLERectangle* bounds = getBounds();
	tokens->ensure_next_token("!");
	while (tokens->has_more_tokens()) {
		string& token = tokens->next_token();
		if (token == "\n") {
			break;
		} else if (str_i_equals(token, "NX")) {
			m_NX = tokens->next_integer();
		} else if (str_i_equals(token, "NY")) {
			m_NY = tokens->next_integer();
		} else if (str_i_equals(token, "XMIN")) {
			bounds->setXMin(tokens->next_double());
		} else if (str_i_equals(token, "XMAX")) {
			bounds->setXMax(tokens->next_double());
		} else if (str_i_equals(token, "YMIN")) {
			bounds->setYMin(tokens->next_double());
		} else if (str_i_equals(token, "YMAX")) {
			bounds->setYMax(tokens->next_double());
		} else {
			stringstream str;
			str << "unknown .z header token '" << token << "'";
			throw tokens->error(str.str());
		}
	}
	lang.setLineCommentTokens("!");
	lang.setSingleCharTokens("");
	lang.setSpaceTokens(" \t\n\r,");
	// Allocate data
	if (m_NX == 0 || m_NY == 0) {
		throw tokens->error("data file header should contain valid NX and NY parameters");
	}
	m_Data = new double[m_NX * m_NY];
	for (int y = 0; y < m_NY; y++) {
		for (int x = 0; x < m_NX; x++) {
			double v = tokens->next_double();
			if (v < m_ZMin) m_ZMin = v;
			if (v > m_ZMax) m_ZMax = v;
			m_Data[x + y * m_NX] = v;
		}
	}
}

bool GLEReadFileBinaryGZIP(const string& name, std::vector<GLEBYTE>* contents) {
#ifdef HAVE_LIBZ
   gzFile file = gzopen(name.c_str(), "rb");
   if (file == 0) {
      return false;
   }
   bool returnCode = true;
   const int GLE_READ_GZIP_BUFFER_SIZE = 100000;
   char* buffer = new char[GLE_READ_GZIP_BUFFER_SIZE];
   bool done = false;
   while (!done) {
      int result = gzread(file, (voidp)buffer, GLE_READ_GZIP_BUFFER_SIZE);
      if (result == -1) {
         done = true;
         returnCode = false;
      } else if (result == 0) {
         done = true;
      } else {
         contents->reserve(contents->size() + result);
         for (int i = 0; i < result; ++i) {
            contents->push_back(buffer[i]);
         }
      }
   }
   delete[] buffer;
   gzclose(file);
   return returnCode;
#else
   return false;
#endif
}

bool GLEReadFileOrGZIP(const std::string& name, vector<string>* lines) {
   bool res = GLEReadFile(name, lines);
   if (!res) {
      std::vector<GLEBYTE> contents;
      res = GLEReadFileBinaryGZIP(name + ".gz", &contents);
      if (res) {
         split_into_lines(&contents, lines);
      }
   }
   return res;
}

bool GLEReadFileOrGZIPTxt(const std::string& name, std::string* result) {
   vector<string> lines;
   bool res = GLEReadFileOrGZIP(name, &lines);
   result->clear();
   if (res) {
      std::ostringstream strm;
      for (std::vector<string>::size_type i = 0; i < lines.size(); ++i) {
         strm << lines[i] << std::endl;
      }
      *result = strm.str();
   }
   return res;
}

GLECSVError::GLECSVError() {
}

GLECSVError::~GLECSVError() {
}

GLECSVData::GLECSVData() {
	initDelims();
	m_lines = 0;
	m_ignoreHeader = 0;
	m_nextLine = true;
	m_firstColumn = 0;
	m_error.errorCode = GLECSVErrorNone;
	m_error.errorLine = 0;
	m_error.errorColumn = 0;
	m_comment = "!";
}

GLECSVData::~GLECSVData() {
	delete[] m_delims;
}

bool GLECSVData::read(const std::string& file) {
	bool result = readBlock(file);
	if (!result) return false;
	parseBlock();
	return true;
}

void GLECSVData::readBuffer(const char* buffer) {
	unsigned int size = strlen(buffer);
	m_buffer.resize(size + 1);
	memcpy(&m_buffer[0], buffer, size);
	m_buffer[size] = 0;
	parseBlock();
}

bool GLECSVData::readBlock(const std::string& fileName) {
	m_fileName = fileName;
	if (str_i_ends_with(fileName, ".gz")) {
		if (GLEReadFileBinaryGZIP(fileName, &m_buffer)) {
			return true;
		} else {
			m_error.errorCode = GLECSVErrorFileNotFound;
			ostringstream errStr;
			errStr << "can't open: '" << fileName << "'";
			m_error.errorString = errStr.str();
			return false;
		}
	} else {
		ifstream file(fileName.c_str(), ios::in | ios::binary | ios::ate);
		if (file.is_open()) {
			unsigned int size = file.tellg();
			m_buffer.resize(size + 1);
			file.seekg(0, ios::beg);
			file.read((char*)&m_buffer[0], size);
			file.close();
			return true;
		} else {
			m_error.errorCode = GLECSVErrorFileNotFound;
			ostringstream errStr;
			errStr << "can't open: '" << fileName << "': ";
			str_get_system_error(errStr);
			m_error.errorString = errStr.str();
			return false;
		}
	}
}

unsigned int GLECSVData::getNbLines() {
	return m_firstCell.size();
}

unsigned int GLECSVData::getFirstCell(unsigned int line) {
	return m_firstCell[line];
}

unsigned int GLECSVData::getNbColumns(unsigned int line) {
	unsigned int startCell = m_firstCell[line];
	if (line + 1 >= m_firstCell.size()) {
		return m_cellPos.size() - startCell;
	} else {
		return m_firstCell[line + 1] - startCell;
	}
}

const char* GLECSVData::getCell(unsigned int row, unsigned int column, unsigned int* size) {
	unsigned int idx = m_firstCell[row] + column;
	*size = m_cellSize[idx];
	return (const char*)&m_buffer[m_cellPos[idx]];
}

string GLECSVData::getCellString(unsigned int row, unsigned int column) {
	unsigned int size;
	const char* buffer = getCell(row, column, &size);
	return string(buffer, size);
}

void GLECSVData::setCellTrim(unsigned int row, unsigned int column, const char* data) {
	unsigned int idx = m_firstCell[row] + column;
	unsigned int size = std::min<unsigned int>(m_cellSize[idx], strlen(data));
	for (unsigned int i = 0; i < size; i++) {
		m_buffer[m_cellPos[idx] + i] = data[i];
	}
	m_cellSize[idx] = size;
}

unsigned int GLECSVData::validateIdenticalNumberOfColumns() {
	bool found = false;
	unsigned int dataColumns = 0;
	for (unsigned int row = 0; row < getNbLines(); ++row) {
		if (!found) {
			found = true;
			dataColumns = getNbColumns(row);
		} else {
			if (m_error.errorCode == GLECSVErrorNone && getNbColumns(row) != dataColumns) {
				m_error.errorCode = GLECSVErrorInconsistentNrColumns;
				m_error.errorLine = row;
				m_error.errorColumn = 0;
				ostringstream err;
				err << "inconsistent number of columns " << getNbColumns(row) << " <> " << dataColumns;
				createErrorString(err.str());
				return dataColumns;
			}
		}
	}
	return dataColumns;
}

void GLECSVData::print(ostream& os) {
	vector<unsigned int> columnWidth;
	for (unsigned int row = 0; row < getNbLines(); row++) {
		unsigned int nbColumns = getNbColumns(row);
		for (unsigned int col = 0; col < nbColumns; col++) {
			unsigned int size;
			const char* cell = getCell(row, col, &size);
			unsigned int chars = getUTF8NumberOfChars(cell, size);
			while (columnWidth.size() <= col) {
				columnWidth.push_back(0);
			}
			columnWidth[col] = std::max<unsigned int>(columnWidth[col], chars + 1);
		}
	}
	for (unsigned int row = 0; row < getNbLines(); row++) {
		unsigned int nbColumns = getNbColumns(row);
		for (unsigned int col = 0; col < nbColumns; col++) {
			unsigned int size;
			const char* cell = getCell(row, col, &size);
			unsigned int chars = getUTF8NumberOfChars(cell, size);
			for (unsigned int idx = 0; idx < size; idx++) {
				os << cell[idx];
			}
			if (col != nbColumns - 1) {
				os << ",";
				for (unsigned int idx = chars; idx < columnWidth[col]; idx++) {
					os << (char)' ';
				}
			}
		}
		os << endl;
	}
}

void GLECSVData::parseBlock() {
	m_pos = 0;
	m_size = m_buffer.size();
	m_data = &m_buffer[0];
	GLECSVDataStatus status = ignoreHeader();
	while (status != GLECSVDataStatusEOF) {
		status = readCell();
	}
}

void GLECSVData::setDelims(const char* delims) {
	int pos = 0;
	unsigned int size = 256;
	for (unsigned int i = 0; i < size; i++) {
		m_delims[i] = false;
	}
	while (delims[pos] != 0) {
		m_delims[(int)delims[pos]] = true;
		pos++;
	}
	m_lastDelimWasSpace = isDelim(' ') || isDelim('\t');
}

void GLECSVData::setCommentIndicator(const char* comment) {
	m_comment = comment;
}

void GLECSVData::setIgnoreHeader(unsigned int ignore) {
	m_ignoreHeader = ignore;
}

void GLECSVData::initDelims() {
	unsigned int size = 256;
	m_delims = new bool[size];
	setDelims(" ,;\t");
}

bool GLECSVData::isDelim(GLEBYTE ch) {
	return m_delims[ch];
}

bool GLECSVData::isSpace(GLEBYTE ch) {
	return ch == ' ' || ch == '\t';
}

bool GLECSVData::isEol(GLEBYTE ch) {
	return ch == '\n' || ch == '\r';
}

bool GLECSVData::isComment(GLEBYTE ch) {
	int currentPos = m_pos;
	size_t commentPos = 0;
	while (commentPos < m_comment.size() && ch == m_comment[commentPos]) {
		ch = readChar();
		commentPos++;
	}
	if (commentPos == m_comment.size()) {
		goBack();
		return true;
	} else {
		m_pos = currentPos;
		return false;
	}
}

GLECSVDataStatus GLECSVData::skipTillEol() {
	while (true) {
		GLEBYTE ch = readChar();
		if (ch == 0) {
			return GLECSVDataStatusEOF;
		}
		if (isEol(ch)) {
			return readNewline(ch);
		}
	}
}

GLECSVDataStatus GLECSVData::ignoreHeader() {
	GLECSVDataStatus result = GLECSVDataStatusOK;
	for (unsigned int i = 0; i < m_ignoreHeader; i++) {
		result = skipTillEol();
	}
	return result;
}

void GLECSVData::createErrorString(const string& str) {
	ostringstream err;
	err << str;
	err << " at " << (m_error.errorLine + 1) << ":" << (m_error.errorColumn + 1);
	err << " while reading '" << m_fileName << "'";
	m_error.errorString = err.str();
}

unsigned int GLECSVData::getUTF8Column(unsigned int cellPos) {
	int size = cellPos - m_firstColumn;
	if (size < 0) size = 0;
	return getUTF8NumberOfChars((const char*)&m_buffer[m_firstColumn], size);
}

GLECSVDataStatus GLECSVData::readCellString(GLEBYTE quote) {
	unsigned int cellSize = 1;
	unsigned int cellPos = lastCharPos();
	initWritePos();
	while (true) {
		GLEBYTE ch = readChar();
		writeChar(ch);
		cellSize++;
		if (ch == 0) {
			m_error.errorCode = GLECSVErrorUnterminatedString;
			m_error.errorLine = m_lines;
			m_error.errorColumn = getUTF8Column(cellPos);
			createErrorString("unterminated string");
			return GLECSVDataStatusEOF;
		} else if (isEol(ch)) {
			m_error.errorCode = GLECSVErrorUnterminatedString;
			m_error.errorLine = m_lines;
			m_error.errorColumn = getUTF8Column(cellPos);
			createErrorString("unterminated string");
			return readNewline(ch);
		} else if (ch == quote) {
			GLEBYTE ch = readChar();
			if (ch != quote) {
				writeChar(ch);
				createCell(cellSize, cellPos);
				return skipSpacesAndFirstDelim(ch);
			}
		}
	}
	return GLECSVDataStatusOK;
}

GLECSVDataStatus GLECSVData::readCell() {
	GLEBYTE ch = readSignificantChar();
	if (ch == '"' || ch == '\'') {
		return readCellString(ch);
	}
	unsigned int cellCount = 0;
	unsigned int cellSize = 0;
	unsigned int cellPos = lastCharPos();
	while (true) {
		if (ch == 0) {
			if (isSizeCheckOKEndOfLine(cellSize)) {
				createCell(cellSize, cellPos);
			}
			return GLECSVDataStatusEOF;
		} else if (isEol(ch)) {
			if (isSizeCheckOKEndOfLine(cellSize)) {
				createCell(cellSize, cellPos);
			}
			return readNewline(ch);
		} else if (isDelim(ch)) {
			m_lastDelimWasSpace = isSpace(ch);
			if (isSizeCheckOKAtDelim(ch, cellSize)) {
				createCell(cellSize, cellPos);
			}
			return skipSpacesAndFirstDelim(ch);
		} else if (isComment(ch)) {
			if (isSizeCheckOKEndOfLine(cellSize)) {
				createCell(cellSize, cellPos);
			}
			return skipTillEol();
		}
		cellCount++;
		if (!isSpace(ch)) {
			cellSize = cellCount;
		}
		ch = readChar();
	}
	return GLECSVDataStatusOK;
}

bool GLECSVData::isSizeCheckOKEndOfLine(unsigned int cellSize) {
	if (cellSize == 0) {
		if (m_nextLine) {
			return false;
		}
		if (m_lastDelimWasSpace) {
			return false;
		}
	}
	return true;
}

bool GLECSVData::isSizeCheckOKAtDelim(GLEBYTE delim, unsigned int cellSize) {
	if (cellSize == 0) {
		if (delim == ' ' || delim == '\t') {
			return false;
		}
	}
	return true;
}

void GLECSVData::createCell(unsigned int cellSize, unsigned int cellPos) {
	if (m_nextLine) {
		m_firstCell.push_back(m_cellPos.size());
		m_nextLine = false;
	}
	m_cellSize.push_back(cellSize);
	m_cellPos.push_back(cellPos);
}

GLECSVDataStatus GLECSVData::readNewline(GLEBYTE prevCh) {
	m_lines++;
	m_nextLine = true;
	GLEBYTE ch = readChar();
	if (ch == 0) {
		m_firstColumn = m_pos;
		return GLECSVDataStatusEOF;
	}
	if (isEol(ch) && (ch != prevCh)) {
		// Found, e.g., CR followed by LF (Windows encoding of new line)
		m_firstColumn = m_pos;
		return GLECSVDataStatusEOL;
	}
	// Found just CR or just LF (Unix / Mac encoding of new line)
	goBack();
	m_firstColumn = m_pos;
	return GLECSVDataStatusEOL;
}

void GLECSVData::goBack() {
	if (m_pos > 0) {
		m_pos = m_pos - 1;
	}
}

unsigned int GLECSVData::lastCharPos() {
	if (m_pos > 0) {
		return m_pos - 1;
	} else {
		return 0;
	}
}

GLEBYTE GLECSVData::readChar() {
	if (m_pos == m_size) {
		return 0;
	} else {
		GLEBYTE ch = m_data[m_pos];
		m_pos = m_pos + 1;
		return ch;
	}
}

void GLECSVData::initWritePos() {
	m_writePos = m_pos;
}

void GLECSVData::writeChar(GLEBYTE ch) {
	m_data[m_writePos++] = ch;
}

GLEBYTE GLECSVData::readSignificantChar() {
	GLEBYTE ch;
	do {
		ch = readChar();
		if (ch == 0) {
			return 0;
		}
	} while (isSpace(ch));
	return ch;
}

GLECSVDataStatus GLECSVData::skipSpacesAndFirstDelim(GLEBYTE ch) {
	while (true) {
		if (!isSpace(ch)) {
			if (ch == 0) {
				return GLECSVDataStatusEOF;
			} else if (isEol(ch)) {
				return readNewline(ch);
			} else if (isDelim(ch)) {
				m_lastDelimWasSpace = isSpace(ch);
				return GLECSVDataStatusOK;
			} else {
				goBack();
				return GLECSVDataStatusOK;
			}
		}
		ch = readChar();
	}
	return GLECSVDataStatusOK;
}

GLECSVError* GLECSVData::getError() {
	return &m_error;
}
