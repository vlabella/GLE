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

// #define DEBUG_UPDATE

#include "all.h"
#include "gle-interface/gle-interface.h"
#include "stdio.h"
#include "cutils.h"

int showLineAbbrev(const string& text, int focuscol, ostream& out) {
	int delta = 0;
	int show_max = 60;
	if (focuscol < 0) focuscol = 0;
	int first = focuscol-show_max/2;
	if (first < 0) first = 0;
	int last = first+show_max;
	if (last > (int)text.length()-1) {
		last = text.length()-1;
		first = last-show_max;
		if (first < 0) first = 0;
	}
	if (first != 0) {
		out << "...";
		delta = first - 3;
	}
	for (int i = first; i <= last; i++) {
		out << (char)text[i];
	}
	if (last != (int)text.length()-1) {
		out << "...";
	}
	return delta;
}

GLESourceLine::GLESourceLine() {
	m_GlobalLineNo = 0;
	m_LineNo = 0;
	m_File = NULL;
	m_Delete = false;
}

GLESourceLine::~GLESourceLine() {
}

const string& GLESourceLine::getFileName() {
	return m_File->getLocation()->getName();
}

int GLESourceLine::showLineAbbrev(ostream& out, int focuscol) {
	return ::showLineAbbrev(getCode(), focuscol, out);
}

bool GLESourceLine::isEmpty() {
	return gle_onlyspace(m_Code);
}

GLESourceFile::GLESourceFile() {
}

GLESourceFile::~GLESourceFile() {
	for (unsigned int i = 0; i < m_Code.size(); i++) {
		delete m_Code[i];
	}
}

GLESourceLine* GLESourceFile::addLine() {
	int line_no = getNbLines() + 1;
	GLESourceLine* line = new GLESourceLine();
	line->setLineNo(line_no);
	line->setSource(this);
	m_Code.push_back(line);
	return line;
}

void GLESourceFile::scheduleInsertLine(int i, const string& str) {
	m_ToInsertIdx.push_back(i);
	m_ToInsertLine.push_back(str);
}

int GLESourceFile::getNextInsertIndex(int line, int pos) {
	while (pos < (int)m_ToInsertIdx.size() && m_ToInsertIdx[pos] < line) {
		pos++;
	}
	if (pos < (int)m_ToInsertIdx.size()) {
		return m_ToInsertIdx[pos];
	} else {
		return -1;
	}
}

void GLESourceFile::performUpdates() {
	/* copy to tmp */
	int nb_old = getNbLines();
	vector<GLESourceLine*> tmp;
	tmp.resize(nb_old);
	for (int i = 0; i < nb_old; i++) {
		tmp[i] = getLine(i);
	}
	m_Code.clear();
	/* copy back one by one */
	unsigned int ins_pos = 0;
	#ifdef DEBUG_UPDATE
		cout << "Performing updates: " << m_ToInsertIdx.size() << endl;
	#endif
	for (int i = 0; i < nb_old; i++) {
		GLESourceLine* line = tmp[i];
		int nxt = getNextInsertIndex(i, ins_pos);
		if (nxt == i) {
			/* may insert multiple lines */
			while (ins_pos < m_ToInsertIdx.size() && m_ToInsertIdx[ins_pos] == i) {
			#ifdef DEBUG_UPDATE
				cout << "Inserting line at: " << i << endl;
				cout << "Inserting line code: '" << m_ToInsertLine[ins_pos] << "'" << endl;
			#endif
				GLESourceLine* new_line = new GLESourceLine();
				new_line->setSource(this);
				new_line->setCode(m_ToInsertLine[ins_pos]);
				m_Code.push_back(new_line);
				ins_pos++;
			}
		}
		if (line->isDelete()) {
			/* should delete a line? */
			delete line;
		} else {
			/* just copy old line */
			#ifdef DEBUG_UPDATE
				cout << "Copy line: '" << line->getCode() << "'" << endl;
			#endif
			m_Code.push_back(line);
		}
	}
	/* renumber and clear update stacks */
	reNumber();
	m_ToInsertIdx.clear();
	m_ToInsertLine.clear();
}

void GLESourceFile::trim(int add) {
	int pos = getNbLines()-1;
	while (pos >= 0 && getLine(pos)->isEmpty()) {
		delete getLine(pos);
		pos--;
	}
	pos++;
	if (pos < getNbLines()) {
		m_Code.erase(m_Code.begin()+pos, m_Code.end());
	}
	for (int i = 0; i < add; i++) {
		addLine();
	}
}

void GLESourceFile::clear() {
	m_Code.clear();
	m_ToInsertIdx.clear();
	m_ToInsertLine.clear();
}

void GLESourceFile::reNumber() {
	for (int i = 0; i < getNbLines(); i++) {
		GLESourceLine* sline = getLine(i);
		sline->setLineNo(i + 1);
	}
}

void GLESourceFile::load(istream& input) {
	//  load the input and rememeber to look for continuation character
	const char CONT_CHAR = '&';  // should be a global define somewhere??
	bool cont = false;
	string inbuff;
	while (input.good()) {
		string linbuff;
		getline(input, linbuff);
		// trim is required to make position indicator in error messages appear correctly
		str_trim_right(linbuff);
		if (cont) {
			// -- splice this string into inbuff at location of '&'
			str_trim_left(linbuff);
			inbuff.replace(inbuff.rfind(CONT_CHAR),linbuff.length(),linbuff);
			cont = false;
		} else {
			// -- ignore BOM in Unicode input
			str_trim_left_bom(linbuff);
			inbuff = linbuff;
		}
		// -- search for continuation character '&' if found
		if (inbuff.length() > 0 && inbuff.at(inbuff.length()-1) == CONT_CHAR) {
			// -- continuation
			cont = true;
		}
		if (!cont || input.eof()){
			string prefix;
			GLESourceLine* sline = addLine();
			str_trim_left(inbuff, prefix);
			sline->setPrefix(prefix);
			sline->setCode(inbuff);
		}
	}
}

void GLESourceFile::load() throw(ParserError) {
	// called to load the main script or an include file
	if (getLocation()->isStdin()) {
		load(cin);
	} else {
		ifstream input;
		input.open(getLocation()->getFullPath().c_str());
		if (!input.is_open()) {
			g_throw_parser_error("file not found: '", getLocation()->getName().c_str(), "'");
		}
		load(input);
		input.close();
	}
}

bool GLESourceFile::tryLoad() {
	// called to read configuration file
	ifstream input;
	input.open(getLocation()->getFullPath().c_str());
	if (!input.is_open()) {
		return false;
	}
	load(input);
	input.close();
	return true;
}

GLEGlobalSource::GLEGlobalSource() {
}

GLEGlobalSource::~GLEGlobalSource() {
}

void GLEGlobalSource::clear() {
	m_Code.clear();
	m_Files.clear();
	m_Main.clear();
}

GLESourceFile* GLEGlobalSource::createNew() {
	clear();
	return &m_Main;
}

void GLEGlobalSource::addLine(const string& code) {
	GLESourceLine* sline = getMainFile()->addLine();
	sline->setCode(code);
	addLine(sline);
}

void GLEGlobalSource::updateLine(int i, const string& code) {
	getLine(i)->setCode(code);
}

void GLEGlobalSource::scheduleDeleteLine(int i) {
	getLine(i)->setDelete(true);
}

void GLEGlobalSource::scheduleInsertLine(int i, const string& str) {
	GLESourceLine* sline = getLine(i);
	GLESourceFile* src = sline->getSource();
	src->scheduleInsertLine(sline->getLineNo() - 1, str);
}

void GLEGlobalSource::performUpdates() {
	m_Main.performUpdates();
	for (int i = 0; i < getNbFiles(); i++) {
		getFile(i)->performUpdates();
	}
	/* Note: this compiles as if all include files came up front */
	m_Code.clear();
	for (int i = 0; i < getNbFiles(); i++) {
		GLESourceFile* file = getFile(i);
		for (int i = 0; i < file->getNbLines(); i++) {
			m_Code.push_back(file->getLine(i));
		}
	}
	GLESourceFile* main = getMainFile();
	for (int i = 0; i < main->getNbLines(); i++) {
		m_Code.push_back(main->getLine(i));
	}
	reNumber();
}

void GLEGlobalSource::reNumber() {
	for (int i = 0; i < getNbLines(); i++) {
		GLESourceLine* sline = getLine(i);
		sline->setGlobalLineNo(i + 1);
	}
}

void GLEGlobalSource::initFromMain() {
	m_Code.clear();
	GLESourceFile* main = getMainFile();
	for (int i = 0; i < main->getNbLines(); i++) {
		m_Code.push_back(main->getLine(i));
	}
	reNumber();
}

void GLEGlobalSource::insertInclude(int offs, GLESourceFile* file) {
	m_Files.push_back(file);
	if (file->getNbLines() > 0) {
		m_Code.insert(m_Code.begin()+offs+1, file->getNbLines()-1, (GLESourceLine*)NULL);
		for (int i = 0; i < file->getNbLines(); i++) {
			m_Code[offs+i] = file->getLine(i);
		}
		reNumber();
	}
}

void GLEGlobalSource::insertIncludeNoOverwrite(int offs, GLESourceFile* file) {
	m_Files.push_back(file);
	if (file->getNbLines() > 0) {
		m_Code.insert(m_Code.begin()+offs, file->getNbLines(), (GLESourceLine*)NULL);
		for (int i = 0; i < file->getNbLines(); i++) {
			m_Code[offs+i] = file->getLine(i);
		}
		reNumber();
	}
}

bool GLEGlobalSource::includes(const string& file) {
	for (vector<GLESourceFile*>::size_type i = 0; i < m_Files.size(); i++) {
		if (str_i_equals(m_Files[i]->getLocation()->getName(), file)) return true;
	}
	return false;
}

void GLEGlobalSource::sourceLineFileAndNumber(int line, ostream& err) {
	int max = getNbLines();
	if (line < 0 || line >= max) {
		err << "[OUT OF RANGE: " << line << "]";
	} else {
		GLESourceLine* srclin = getLine(line);
		err << srclin->getFileName() << ":" << srclin->getLineNo();
	}
}

void GLEGlobalSource::load() throw(ParserError) {
	GLESourceFile* main = getMainFile();
	main->load();
	main->trim(0);
	initFromMain();
}

bool GLEGlobalSource::tryLoad() {
	// called to read configuration file
	GLESourceFile* main = getMainFile();
	bool res = main->tryLoad();
	main->trim(0);
	initFromMain();
	return res;
}

void GLEGlobalSource::clearObjectDOConstructors() {
	getMainFile()->clearObjectDOConstructors();
	for (int i = 0; i < getNbFiles(); i++) {
		getFile(i)->clearObjectDOConstructors();
	}
}
