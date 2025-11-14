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

#include <vector>
#include <fstream>
#include <sstream>
#include <string.h>
#include <stdio.h>

#include "../basicconf.h"
#include "../cutils.h"
#include "Tokenizer.h"

using namespace std;

#define SPACE_CHAR	' '

//#define debug_tok_lang(x) cerr << x << endl
#define debug_tok_lang(x)

// GNU version of index not defined in MingW, ...
// Replaces by calls to strcontains
int strcontains(const char* str, char ch) {
	char cr = *str;
	while (cr != 0) {
		if (cr == ch) return 1;
		cr = *(++str);
	}
	return 0;
}

int strposition(const char* str, char ch) {
	int  pos = 0;
	char cr = *str;
	while (cr != 0) {
		if (cr == ch) return pos;
		cr = str[++pos];
	}
	return -1;
}

bool is_integer(const string& strg) {
	int len = strg.length();
	if (len == 0) return false;
	for (int i = 0; i < len; i++) {
		char ch = strg[i];
		if (ch < '0' || ch > '9') {
			if (i != 0) return false;
			if (ch != '+' && ch != '-') return false;
		}
	}
	return true;
}

bool is_integer_e(const string& strg) {
	int len = strg.length();
	if (len <= 1) return false;
	char ch = strg[len-1];
	if (ch != 'e' && ch != 'E') return false;
	for (int i = 0; i < len-1; i++) {
		char ch = strg[i];
		if (ch < '0' || ch > '9') {
			if (i != 0) return false;
			if (ch != '+' && ch != '-') return false;
		}
	}
	return true;
}

bool is_float(const string& strg) {
	int len = strg.length();
	if (len == 0) return 0;
	int pos = 0;
	char ch = strg[pos];
	// cout << "is_float(" << strg << ")" << endl;
	if (ch == '+' || ch == '-') {
		pos++;
		ch = pos < len ? strg[pos] : 0;
	}
	bool has_dot = false;
	if (ch == '.') {
		pos++; has_dot = true;
		ch = pos < len ? strg[pos] : 0;
	}
	int count1 = 0, count2 = 0;
	while (ch >= '0' && ch <= '9') {
		pos++; count1++;
		ch = pos < len ? strg[pos] : 0;
	}
	if (!has_dot) {
		if (ch == '.') {
			pos++; has_dot = true;
			ch = pos < len ? strg[pos] : 0;
			while (ch >= '0' && ch <= '9') {
				pos++; count2++;
				ch = pos < len ? strg[pos] : 0;
			}
		}
	} else {
		count2 = count1;
		count1 = 0;
	}
	if (count1 > 0 || count2 > 0) {
		if (ch == 'e' || ch == 'E') {
			pos++;
			ch = pos < len ? strg[pos] : 0;
			if (ch == '+' || ch == '-') {
				pos++;
				ch = pos < len ? strg[pos] : 0;
			}
			int counte = 0;
			while (ch >= '0' && ch <= '9') {
				pos++; counte++;
				ch = pos < len ? strg[pos] : 0;
			}
			return counte > 0;
		} else {
			return pos == len;
		}
	} else {
		return false;
	}
}

void strip_string_markers(string& strg) {
	int len = strg.length();
	if (len >= 2) {
		char ch0 = strg[0];
		if (ch0 == '"' || ch0 == '\'') {
			strg.erase(strg.begin());
			strg.resize(len-2);
		}
	}
}

ostream& mtab(ostream &os, int nb) {
	for (int i = 0; i < nb; i++) os << '\t';
	return os;
}

double tokenizer_string_to_double(const char* value) {
	char *endp;
	double dvalue = strtod(value, &endp);
	if (value != endp && *endp == 0) {
		return dvalue;
	} else {
		ostringstream err;
		err << "illegal double value '" << value << "'" << endl;
		g_throw_parser_error(err.str());
		return 0.0;
	}
}

TokenizerPos::TokenizerPos() {
	m_col = -10; m_line = -10;
}

TokenizerPos::TokenizerPos(const TokenizerPos& pos) {
	m_col = pos.m_col;
	m_line = pos.m_line;
}

void TokenizerPos::set(int line, int column) {
	m_line = line;
	m_col = column;
}

void TokenizerPos::set_line(int line) {
	m_line = line;
}

void TokenizerPos::set_col(int column) {
	m_col = column;
}

void TokenizerPos::incRow() {
	m_line++;
	m_col = 0;
}

bool TokenizerPos::isValid() const {
	return m_col >= 0 || m_line > 0;
}

ostream& TokenizerPos::write(ostream &os) const {
	if (m_col >= 0 && m_line > 0) {
		os << m_line << ":" << (m_col-1);
	} else if (m_line > 0) {
		os << "line " << m_line;
	} else if (m_col >= 0) {
		os << "column " << (m_col-1);
	}
	return os;
}

int TokenizerPos::equals(TokenizerPos& pos) const {
	return (m_col == pos.m_col && m_line == pos.m_line);
}

string TokenizerPos::getString(int tab1, int tab2) const {
	int i, pos = 0;
	char lines[15], cols[15], res[50];
	if (m_line < 0) strcpy(lines, "?");
	else sprintf(lines, "%d", m_line);
	if (m_col < 0) strcpy(cols, "?");
	else sprintf(cols, "%d", m_col-1);
	for (i = tab1-strlen(lines); i > 0; i--) {
		res[pos++] = ' ';
	}
	i = 0;
	while (lines[i] != 0) {
		res[pos++] = lines[i++];
	}
	i = 0;
	res[pos++] = ':';
	while (cols[i] != 0) {
		res[pos++] = cols[i++];
	}
	for (i = tab2-strlen(cols); i > 0; i--) {
		res[pos++] = ' ';
	}
	res[pos] = 0;
	return string(res);
}

// not needed - causes compile errors on windows
// #if defined(__unix__) || defined(__APPLE__)
// 	ParserError g_format_parser_error(const char* format, ...) {
// #else
// 	ParserError g_format_parser_error(va_list format, ...) {
// #endif
// 	string str;
// 	va_list ap;
// 	va_start(ap, format);
// 	str_format(&str, (const char*)format, ap);
// 	va_end(ap);
// 	TokenizerPos pos;
// 	pos.setColumn(-1);
// 	return ParserError(str, pos, NULL);
// }


ParserError g_format_parser_error(const char* format, ...) {
    va_list ap;
    va_start(ap, format);

    std::string str;
    str_format(&str, format, ap);

    va_end(ap);

    TokenizerPos pos;
    pos.setColumn(-1);
    return ParserError(str, pos, NULL);
}



void g_throw_parser_error(const string& err) {
	TokenizerPos pos;
	pos.setColumn(-1);
	ParserError err_exp(err, pos, NULL);
	throw err_exp;
}

void g_throw_parser_error(const char* str1, const char* str2, const char* str3) {
	TokenizerPos pos;
	pos.setColumn(-1);
	string err = str1;
	if (str2 != NULL) err += str2;
	if (str3 != NULL) err += str3;
	ParserError err_exp(err, pos, NULL);
	throw err_exp;
}

void g_throw_parser_error_sys(const char* str1, const char* str2, const char* str3) {
	TokenizerPos pos;
	pos.setColumn(-1);
	ostringstream err_str;
	err_str << str1;
	if (str2 != NULL) err_str << str2;
	if (str3 != NULL) err_str << str3;
	err_str << ": ";
	str_get_system_error(err_str);
	ParserError err_exp(err_str.str(), pos, NULL);
	throw err_exp;
}

void g_throw_parser_error(const char* err, int idx) {
	char str[30];
	sprintf(str, "%d", idx);
	TokenizerPos pos;
	pos.setColumn(-1);
	ParserError err_exp(string(err)+str, pos, NULL);
	throw err_exp;
}

IThrowsError::IThrowsError() {
}

IThrowsError::~IThrowsError() {
}

ParserError IThrowsError::throwError(const char* s1, const char* s2, const char* s3) {
	TokenizerPos pos;
	pos.setColumn(-1);
	string err(s1);
	if (s2 != NULL) err += s2;
	if (s3 != NULL) err += s3;
	return ParserError(err, pos, NULL);
}

ParserError IThrowsError::throwError(int /* pos */, const string& error) {
   return throwError(error);
}

ParserError IThrowsError::throwError(const string& error) {
   TokenizerPos pos;
	pos.setColumn(-1);
	return ParserError(error, pos, NULL);
}

int IThrowsError::getErrorPosition() const {
   return 0;
}

IThrowsError* g_get_throws_error() {
   static IThrowsError instance;
   return &instance;
}

ParserError::ParserError(const string& txt, const TokenizerPos& pos, const char* fname) {
	m_txt = txt;
	str_replace_all(m_txt, "\n", "\n>> ");
	m_pos = pos;
	m_flag = 0;
	if (fname == NULL) m_fname = "";
	else m_fname = fname;
}

ParserError::ParserError(const ParserError& err) {
	m_flag = err.m_flag;
	m_txt = err.m_txt;
	m_fname = err.m_fname;
	m_parsestr = err.m_parsestr;
	m_pos = err.m_pos;
}

void ParserError::setParserString(const string& str) {
	m_parsestr = str;
	m_flag |= TOK_PARSER_ERROR_PSTRING;
}

void ParserError::setParserString(const char* str) {
	m_parsestr = str;
	m_flag |= TOK_PARSER_ERROR_PSTRING;
}

ostream& ParserError::write(ostream& os) const {
	os << m_txt;
	if (m_fname != "") {
		if (m_pos.isValid()) {
			os << " at " << m_pos;
		}
		os << ", while processing '" << m_fname << "'";
	}
	return os;
}

void ParserError::toString(string& str) const {
	if (m_fname == "") {
		str = m_txt;
	} else {
		ostringstream err_str;
		write(err_str);
		str = err_str.str();
	}
}

int ParserError::equals(ParserError* err) const {
	return (m_txt == err->m_txt &&
	        m_fname == err->m_fname &&
		m_pos.equals(err->m_pos));
}

TokenizerLangHash::TokenizerLangHash() {
}

TokenizerLangHash::TokenizerLangHash(const string& name) {
}

TokenizerLangHash::~TokenizerLangHash() {
}

ostream& TokenizerLangHash::write(ostream &os, int depth) const {
	if (!m_default.isNull()) {
		mtab(os, depth);
		os << m_default->getName() << endl;
	}
	for (const_iterator i = begin(); i != end(); i++ ) {
		const name_hash_key& key = i->first;
		const TokenizerLangHash* hash = i->second.get();
		mtab(os, depth);
		os << key << endl;
		hash->write(os, depth + 1);
	}
	return os;
}

TokenAndPos::TokenAndPos() {
};

TokenAndPos::~TokenAndPos() {
};

TokenAndPos::TokenAndPos(const TokenAndPos& copy) {
	m_token = copy.m_token;
	m_pos = copy.m_pos;
	m_space = copy.m_space;
}

TokenAndPos::TokenAndPos(const string& token, const TokenizerPos& pos, char space) {
	m_token = token; m_pos = pos; m_space = space;
};

TokenizerLangElem* Tokenizer::findLangElem(const TokenizerLangHash* hash) {
	string backup_token = m_token;
	TokenizerPos backup_pos = m_token_start;
	get_token_2();
	if (m_token.length() != 0) {
		debug_tok_lang("Tokenizer::first part of token '" << m_token << "'");
		if (m_space_before) {
			pushback_token();
		} else {
			TokenizerLangElem* result = findLangElem2(hash);
			if (result != NULL) {
				debug_tok_lang("Tokenizer::recursive call returns NULL");
				m_token_start = backup_pos;
				return result;
			}
		}
	}
	TokenizerLangElem* elem = hash->getDefault();
	debug_tok_lang("Tokenizer::default elem " << elem);
	if (elem != NULL) {
		return elem;
	} else {
		m_token = backup_token;
		m_token_start = backup_pos;
		return NULL;
	}
}

TokenizerLangElem* Tokenizer::findLangElem2(const TokenizerLangHash* hash) {
	TokenAndPos my_tkpos(m_token, m_token_start, m_space_before);
	TokenizerLangHash::const_iterator i = hash->find(my_tkpos.getToken());
	if (i != hash->end()) {
		get_token_2();
		const TokenizerLangHash* child = i->second.get();
		if (m_token.length() != 0) {
			if (m_space_before) {
				pushback_token();
			} else {
				TokenizerLangElem* result = findLangElem2(child);
				if (result != NULL) return result;
			}
		}
		TokenizerLangElem* elem = child->getDefault();
		if (elem != NULL) return elem;
		pushback_token(my_tkpos);
		return NULL;
	} else {
		pushback_token(my_tkpos);
		return NULL;
	}
}

void TokenizerLangHash::addLangElem(Tokenizer* tokens, TokenizerLangElem* elem) {
	string& token = tokens->try_next_token();
	if (token.length() == 0) {
		m_default = elem;
	} else {
		elem->addName(token);
		TokenizerLangHashPtr hash = try_add(token);
		hash.addLangElem(tokens, elem);
	}
}

void TokenizerLangHash::addLangElem(const vector<string>& toks, TokenizerLangElem* elem, unsigned int pos) {
	if (pos >= toks.size()) {
		m_default = elem;
	} else {
		TokenizerLangHashPtr hash = try_add(toks[pos]);
		hash.addLangElem(toks, elem, pos+1);
	}
}

TokenizerLanguage::TokenizerLanguage() {
	m_index = NULL;
	m_enable_c_comm = 0;
	m_enable_cpp_comm = 0;
	m_parse_strings = 1;
	m_decimal_dot = 0;
}

TokenizerLanguage::~TokenizerLanguage() {
	if (m_index != NULL) delete[] m_index;
}

void TokenizerLanguage::addElementIndex(int size) {
	if (m_index != NULL) delete[] m_index;
	m_index = new TokLangElemPtr[size];
}

void TokenizerLanguage::setElement(int i, TokenizerLangElem* elem) {
	m_index[i] = elem;
}

void TokenizerLanguage::addSubLanguages(int nb) {
	for (int i = 0; i < nb; i++) {
		m_sublanguage.push_back(new TokenizerLangHash());
	}
}

void TokenizerLanguage::addLanguageElem(int sublang, const char* elem) {
	StringTokenizer parser(elem, this);
	TokenizerLangElem* le = new TokenizerLangElem();
	m_sublanguage[sublang].addLangElem(&parser, le);
}

void TokenizerLanguage::addLanguageElem(int sublang, const vector<string>& toks, TokenizerLangElem* elem) {
	m_sublanguage[sublang].addLangElem(toks, elem, 0);
}

void TokenizerLanguage::resetCharMaps() {
	m_one_char_tokens.clear();
	m_space_tokens.clear();
	m_line_comment_tokens.clear();
}

void TokenizerLanguage::initDefault() {
	initDefaultSingleCharTokens();
	initDefaultSpaceTokens();
}

void TokenizerLanguage::initDefaultSingleCharTokens() {
	setSingleCharTokens(",.:;[]{}()+-*/=#<>|^@");
}

void TokenizerLanguage::initDefaultSpaceTokens() {
	setSpaceTokens(" \t\n\r");
}

TokenizerLanguageMultiLevel::TokenizerLanguageMultiLevel() {
	for (int i = 0; i < 255; i++) {
		m_open_tokens[i] = 0;
	}
}

void TokenizerLanguageMultiLevel::setOpenClose(char open, char close) {
	m_open_tokens[(int)open] = close;
	m_close_tokens.set(close);
}

Tokenizer::Tokenizer() {
	m_language = new TokenizerLanguage();
	init();
}

Tokenizer::Tokenizer(TokenizerLanguage* language) {
	m_language = language;
	init();
}

Tokenizer::~Tokenizer() {
}

void Tokenizer::select_language(int i) {
	if (i == TOKENIZER_LANG_NONE) m_langhash = NULL;
	else m_langhash = m_language->getLanguage(i);
}

void Tokenizer::init() {
	m_fname = NULL;
	m_langhash = NULL;
	reset_all();
}

void Tokenizer::reset_nopos() {
	m_token_at_end = 0;
	m_token_has_pushback = 0;
	m_token_has_pushback_ch = 0;
	m_token_start.set(0, 0);
	m_space_before = false;
	m_space_after = false;
	m_pushback_tokens.clear();
}

void Tokenizer::reset_all() {
	reset_nopos();
	m_token_count.set(1, 0);
}

TokenizerLanguage* Tokenizer::get_language() {
	return m_language;
}

void Tokenizer::delete_language() {
	if (m_language != NULL) {
		delete m_language;
		m_language = NULL;
	}
}

void Tokenizer::reset_position() {
	m_token_count.set(1, 0);
	m_token_start.set(0, 0);
}

int Tokenizer::has_more_tokens() {
  if (m_token_has_pushback > 0) {
	  return 1;
  }
  if (m_token_at_end == 1) {
	  return 0;
  }
  char token_ch = token_read_sig_char();
  if (m_token_at_end == 1) {
    return 0;
  } else {
    token_pushback_ch(token_ch);
    return 1;
  }
}

string& Tokenizer::next_token() {
	get_check_token();
//	cerr << m_token_start << "\t" << m_token << endl;
	return m_token;
}

string& Tokenizer::try_next_token() {
	get_token();
	return m_token;
}

double Tokenizer::next_double() {
	char *ptr;
	get_check_token();
	double result = strtod(m_token.c_str(), &ptr);
   if (*ptr != 0) throw error("expected floating point number, not '" + m_token + "'");
	return result;
}

int Tokenizer::next_integer() {
	char* ptr;
	get_check_token();
	int result = strtol(m_token.c_str(), &ptr, 10);
	if (*ptr != 0) throw error("expected integer, not '" + m_token + "'");
	return result;
}

int Tokenizer::try_next_integer(int *i) {
	char* ptr;
	get_check_token();
	*i = strtol(m_token.c_str(), &ptr, 10);
	return *ptr != 0 ? 0 : 1;
}

void Tokenizer::get_token() {
	get_token_2();
	//cout << "GT "<<m_token<<endl;
	if ((!m_langhash.isNull()) && m_token.length() > 0) {
		//cout << " if ";
		TokenizerLangHash::const_iterator i = m_langhash->find(m_token);
		if (i != m_langhash->end()) {
			const TokenizerLangElem* elem = findLangElem(i->second.get());
			if (elem != NULL) m_token = elem->getName();
		}
		//cout << m_token <<endl;
	}
}

TokenizerLangElem* Tokenizer::try_find_lang_elem(int i) {
  get_token_2();
  if (m_token.length() > 0) {
    const TokenizerLangHash* hash = m_language->getLanguage(i).get();
    TokenizerLangHash::const_iterator i = hash->find(m_token);
    if (i != hash->end()) {
      debug_tok_lang("Tokenizer::find first part " << m_token);
      TokenizerLangElem* elem = findLangElem(i->second.get());
      if (elem != NULL) {
        debug_tok_lang("Tokenizer::found elem " << elem->getName());
		  return elem;
      } else {
        debug_tok_lang("Tokenizer::call returns NULL, pushback " << m_token);
		  pushback_token();
      }
    } else {
      pushback_token();
    }
  }
  return NULL;
}

void Tokenizer::get_token_2() {
	if (m_token_has_pushback > 0) {
		const TokenAndPos& tkpos = m_pushback_tokens.back();
		m_token = tkpos.getToken();
		m_token_start = tkpos.getPos();
		m_space_before = tkpos.getSpace();
		m_pushback_tokens.pop_back();
		m_token_has_pushback--;
		return;
	}
	m_space_before = m_space_after;
	m_space_after = false;
	char token_ch = token_read_sig_char();
	m_token_start = m_token_count;
	if (m_token_at_end == 1) {
		 m_token = "";
		 return;
	}
	// String with \" or \' marks
	if ((token_ch  == '\"' || token_ch == '\'') && m_language->getParseStrings()) {
		char string_delim = token_ch;
		m_token = token_ch;
		do {
			token_ch = token_read_char_no_comment();
			m_token += token_ch;
			if (token_ch == string_delim) {
				token_ch = token_read_char_no_comment();
				if (token_ch != string_delim) {
					token_pushback_ch(token_ch);
					return;
				}
			}
		} while (m_token_at_end == 0);
		throw error("unterminated string constant");
	}
	// No string found
	//cout << "GT2 " <<token_ch <<" "<<m_token <<endl;
	if (m_language->isSingleCharToken(token_ch)) {
		if (m_language->isDecimalDot(token_ch)) {
			// Number starting with decimal dot
			m_token = "";
			read_number_term(token_ch, false, false);
		} else {
			m_token = token_ch;
		}
	} else {
		// Support the possibility not to token on space
		bool token_on_space = m_language->isSpaceToken(SPACE_CHAR);
		m_token = token_ch;
		do {
			token_ch = token_read_char();
			if (m_language->isDecimalDot(token_ch)) {
				if (is_integer(m_token)) {
					// Number starting with "123."
					read_number_term(token_ch, false, true);
					return;
				} else {
					token_pushback_ch(token_ch);
					return;
				}
			} else {
				if (m_language->isSingleCharToken(token_ch)) {
					// Support numbers of the form 1e-9
					if ((token_ch == '+' || token_ch == '-') && is_integer_e(m_token)) {
						read_number_term(token_ch, true, true);
						return;
					}
					token_pushback_ch(token_ch);
					return;
				}
			}
			if (token_ch == SPACE_CHAR && token_on_space) {
				m_space_after = true;
				on_trailing_space();
				return;
			}
			m_token += token_ch;
			//cout << "GT2 " <<token_ch <<" "<<m_token <<endl;
		} while (m_token_at_end == 0);
	}
}

void Tokenizer::copy_string(char string_delim) {
	TokenizerPos pos = token_stream_pos();
	while (m_token_at_end == 0) {
		char token_ch = token_read_char_no_comment();
		m_token += token_ch;
		if (token_ch == string_delim) {
			token_ch = token_read_char_no_comment();
			if (token_ch != string_delim) {
				token_pushback_ch(token_ch);
				return;
			}
		}
	}
	throw error(pos, "unterminated string constant");
}

void Tokenizer::multi_level_do_multi(char open) {
	vector<char> m_open_token;
	m_open_token.push_back(open);
	TokenizerLanguageMultiLevel* multi = m_language->getMulti();
	char token_ch = token_read_char();
	while (m_token_at_end == 0) {
		if (m_open_token.size() == 0 && multi->isEndToken(token_ch)) {
			if (token_ch != SPACE_CHAR) {
				token_pushback_ch(token_ch);
			}
			return;
		} else {
			m_token += token_ch;
			if ((token_ch == '\"' || token_ch == '\'') && m_language->getParseStrings()) {
				copy_string(token_ch);
			} else if (multi->isOpenToken(token_ch)) {
				m_open_token.push_back(token_ch);
			} else if (multi->isCloseToken(token_ch)) {
				if (m_open_token.size() == 0) {
					throw error(token_stream_pos(), string("illegal closing '")+token_ch+"'");
				} else {
					char expected = multi->getCloseToken(m_open_token.back());
					if (expected == token_ch) {
						m_open_token.pop_back();
					} else {
						throw error(token_stream_pos(), string("illegal closing '")+token_ch+"', expected a closing '"+expected+"' first");
					}
				}
			}
			token_ch = token_read_char();
		}
	}
	if (m_open_token.size() != 0) {
		char expected = multi->getCloseToken(m_open_token.back());
		throw error(token_stream_pos(), string("expected closing '")+expected+"'");
	}
}

string& Tokenizer::next_continuous_string_excluding(const char* forbidden) {
   undo_pushback_token();
	m_token = "";
	char token_ch = token_read_sig_char();
	m_token_start = m_token_count;
	if (m_token_at_end == 1) {
		return m_token;
	}
	do {
		if (token_ch == SPACE_CHAR) {
         break;
		} else {
         if (str_contains(forbidden, token_ch)) {
            m_token = "";
            goto_position(m_token_start);
            break;
         } else {
            m_token += token_ch;
            token_ch = token_read_char();
         }
		}
	} while (m_token_at_end == 0);
	return m_token;
}

void Tokenizer::undo_pushback_token()
{
	if (m_token_has_pushback > 0) {
		TokenAndPos& tkpos = m_pushback_tokens.back();
		goto_position(tkpos.getPos());
		m_pushback_tokens.clear();
		m_token_has_pushback = 0;
	}
}

string& Tokenizer::next_multilevel_token() {
   undo_pushback_token();
	m_token = "";
	char token_ch = token_read_sig_char();
	//cout << "token_ch: '" << token_ch << "'" << endl;
	m_token_start = m_token_count;
	if (m_token_at_end == 1) {
		return m_token;
	}
	TokenizerLanguageMultiLevel* multi = m_language->getMulti();
	do {
		if (multi->isEndToken(token_ch)) {
			if (token_ch != SPACE_CHAR) {
				token_pushback_ch(token_ch);
			}
			break;
		} else {
			m_token += token_ch;
			if ((token_ch == '\"' || token_ch == '\'') && m_language->getParseStrings()) {
				copy_string(token_ch);
				//cout << "token: '" << m_token << "'" << endl;
			} else if (multi->isOpenToken(token_ch)) {
				/* Use subroutine for efficiency: */
				/* no vector constructed if not multi-level */
				multi_level_do_multi(token_ch);
				break;
			} else if (multi->isCloseToken(token_ch)) {
				throw error(token_stream_pos(), string("illegal closing '")+token_ch+"'");
			}
			token_ch = token_read_char();
		}
	} while (m_token_at_end == 0);
	return m_token;
}

void Tokenizer::on_trailing_space() {
}

void Tokenizer::goto_position(const TokenizerPos& pos) {
	//cout << "goto position : " << pos << endl;
	m_token_count = pos;
	m_token_count.incCol(-1);
	m_token_has_pushback_ch	= 0;
}

string& Tokenizer::read_line() {
	m_token = "";
	while (m_token_has_pushback > 0) {
		TokenAndPos& tkpos = m_pushback_tokens.back();
		m_token += tkpos.getToken();
		m_pushback_tokens.pop_back();
		m_token_has_pushback--;
	}
	while (m_token_has_pushback_ch > 0) {
		m_token += m_token_pushback_ch[--m_token_has_pushback_ch];
	}
	while (1) {
		char ch = stream_get();
		if (!stream_ok()) break;
		if (ch == '\n') break;
		m_token += ch;
	}
	return m_token;
}

void Tokenizer::read_number_term(char token_ch, bool has_e, bool sure_num) {
	// token_ch is "decimal dot" or integer+e+"+/-", always add!
	m_token += token_ch;
	int count_exp = 0;
	int count_num = 0;
	if (!has_e) {
		// token_ch is "decimal dot"
		bool busy_main = true;
		while (busy_main) {
			token_ch = token_read_char();
			// keep on reading numbers or "e/E"
			if (token_ch == 'e' || token_ch == 'E') {
				// should be number or "+/-"
				char next_ch = token_read_char();
				if ((next_ch < '0' || next_ch > '9') && next_ch != '+' && next_ch != '-') {
					if (sure_num) {
						throw error(token_stream_pos(), string("illegal character '")+next_ch+"' while reading exponent of floating point number");
					} else {
						token_pushback_ch(next_ch);
						token_pushback_ch(token_ch);
						return;
					}
				} else {
					m_token += token_ch;
					busy_main = false;
					m_token += next_ch;
					if (next_ch >= '0' && next_ch <= '9') {
						count_exp = 1;
					}
				}
			} else {
				if (token_ch < '0' || token_ch > '9') {
					if (token_ch == SPACE_CHAR) {
						on_trailing_space();
						return;
					}
					if (m_language->isSingleCharToken(token_ch)) {
						token_pushback_ch(token_ch);
						return;
					}
					if (count_num > 0) {
						throw error(token_stream_pos(), string("illegal character '")+token_ch+"' while reading floating point number");
					} else {
						token_pushback_ch(token_ch);
						return;
					}
				}
				m_token += token_ch;
				count_num++;
			}
		}
	}
	// read regular number, after the e+"+/-"
	while (1) {
		token_ch = token_read_char();
		if (token_ch < '0' || token_ch > '9') {
			if (count_exp == 0) {
				throw error(token_stream_pos(), string("illegal character '")+token_ch+"' while reading exponent of floating point number");
			}
			if (token_ch == SPACE_CHAR) {
				on_trailing_space();
				return;
			}
			if (m_language->isSingleCharToken(token_ch)) {
				token_pushback_ch(token_ch);
				return;
			}
			throw error(token_stream_pos(), string("illegal character '")+token_ch+"' while in exponent of floating point number");
		}
		count_exp++;
		m_token += token_ch;
	}
}

void Tokenizer::next_token_and_pos(TokenAndPos& tkpos) {
	get_check_token();
	tkpos.setToken(m_token);
	tkpos.setPos(m_token_start);
	tkpos.setSpace(m_space_before);
}

void Tokenizer::pushback_token() {
	m_pushback_tokens.push_back(TokenAndPos(m_token, m_token_start, m_space_before));
	m_token_has_pushback++;
}

void Tokenizer::pushback_token(const TokenAndPos& tkpos) {
	m_pushback_tokens.push_back(tkpos);
	m_token_has_pushback++;
}

void Tokenizer::pushback_token(const string& token, const TokenizerPos& pos) {
	m_pushback_tokens.push_back(TokenAndPos(token, pos, 0));
	m_token_has_pushback++;
}

void Tokenizer::pushback_token(const string& token) {
	pushback_token(token, m_token_start);
}

void Tokenizer::pushback_token(const char* token) {
	pushback_token(string(token), m_token_start);
}

void Tokenizer::get_check_token() {
	get_token();
	if (m_token.length() == 0) {
		throw eof_error();
	}
}

void Tokenizer::peek_token(string* token) {
	get_check_token();
	pushback_token();
	*token = m_token;
}

int Tokenizer::token_line() const {
	return m_token_start.getLine();
}

int Tokenizer::token_column() const {
	return m_token_start.getColumn();
}

void Tokenizer::set_line(int line) {
	m_token_count.set_line(line);
}

void Tokenizer::inc_line() {
	m_token_count.set_line(m_token_count.getLine()+1);
}

int Tokenizer::is_next_token(const char* token) {
	get_token();
	if (m_token.length() == 0) {
		return m_token == token;
	} else {
		if (m_token == token) {
			return 1;
		}
		pushback_token();
		return 0;
	}
}

int Tokenizer::is_next_token_i(const char* token) {
	get_token();
	if (m_token.length() == 0) {
		return m_token == token;
	} else {
		if (str_i_equals(m_token.c_str(), token)) {
			return 1;
		}
		pushback_token();
		return 0;
	}
}

int Tokenizer::is_next_token_in(const char* charlist) {
	get_check_token();
	if (m_token.length() == 1) {
		char ch = m_token[0];
		if (strcontains(charlist, ch)) return ch;
	}
	pushback_token();
	return -1;
}

int Tokenizer::ensure_next_token_in(const char* charlist) {
	get_check_token();
	if (m_token.length() == 1) {
		char ch = m_token[0];
		if (strcontains(charlist, ch)) return ch;
	}
	throw error(string("expected one of '") + charlist + "', found '" + m_token + "'");
}

void Tokenizer::ensure_next_token(const char* token) {
	get_check_token();
	if (m_token != token) {
		throw error(string("expected '") + token + "', found '" + m_token + "'");
	}
}

void Tokenizer::ensure_next_token_i(const char* token) {
	get_check_token();
	if (!str_i_equals(m_token.c_str(), token)) {
		throw error(string("expected '") + token + "', found '" + m_token + "'");
	}
}

void Tokenizer::ensure_next_token_list(const char* charlist) {
	char err = 0;
	int len = strlen(charlist);
	TokenizerPos start = m_token_start;
	for (int i = 0; i < len; i++) {
		get_check_token();
		if (m_token.length() != 1) {
			err = 1;
			break;
		} else {
			char ch = m_token[0];
			if (ch != charlist[i]) {
				err = 1;
				break;
			}
		}
	}
	if (err == 1) {
		throw error(start, string("expected ") + charlist);
	}
}

char Tokenizer::token_read_sig_char() {
	char token_ch;
	while (1) {
		do {
			token_ch = token_read_char();
			if (m_token_at_end == true) {
				return SPACE_CHAR;
			}
			if (token_ch == SPACE_CHAR) {
				m_space_before = true;
			}
		} while (token_ch == SPACE_CHAR);
		if (m_language->isLineCommentToken(token_ch)) {
			m_space_before = true;
			token_skip_to_end();
		} else if (token_ch == '/') {
			char next_token_ch = token_read_char();
			if (next_token_ch == '/' && m_language->isEnableCPPComment()) {
				m_space_before = true;
				token_skip_to_end();
			} else if (next_token_ch == '*' && m_language->isEnableCComment()) {
				m_space_before = true;
				read_till_close_comment();
			} else if (next_token_ch == '=' ) {
				// /= operator put char back and move the column back one
				token_pushback_ch(next_token_ch);
				m_token_count.incCol(-1);
				return token_ch;
			} else {
				token_pushback_ch(next_token_ch);
				return token_ch;
			}
		} else {
			return token_ch;
		}
	}
}

char Tokenizer::token_read_char() {
	if (m_token_has_pushback_ch > 0) {
		return m_token_pushback_ch[--m_token_has_pushback_ch];
	}
	while (1) {
		char ch = stream_get();
		//cout << "CH "<<ch<<endl;
		if (stream_ok()) {
			if (ch == '\t') m_token_count.incTab();
			else m_token_count.incCol();
			if (ch == '\n') m_token_count.incRow();
			if (m_language->isLineCommentToken(ch)) {
				token_skip_to_end();
				return SPACE_CHAR;
			}
			if (m_language->isSpaceToken(ch)) {
				return SPACE_CHAR;
			} else {
				return ch;
			}
		} else {
			if (m_token_at_end == 0) {
				m_token_count.incCol();
			}
			m_token_at_end = 1;
			return SPACE_CHAR;
		}
	}
}

char Tokenizer::token_read_char_no_comment() {
	if (m_token_has_pushback_ch > 0) {
		return m_token_pushback_ch[--m_token_has_pushback_ch];
	}
	while (1) {
		char ch = stream_get();
		if (stream_ok()) {
			if (ch == '\t') m_token_count.incTab();
			else m_token_count.incCol();
			if (ch == '\n') m_token_count.incRow();
			if (m_language->isSpaceToken(ch)) {
				return SPACE_CHAR;
			} else {
				return ch;
			}
		} else {
			if (m_token_at_end == 0) {
				m_token_count.incCol();
			}
			m_token_at_end = 1;
			return SPACE_CHAR;
		}
	}
}

void Tokenizer::token_skip_to_end() {
	while (1) {
		char ch = stream_get();
		if (stream_ok()) {
			m_token_count.incCol();
			if (ch == '\n') {
				m_token_count.incRow();
				if (!m_language->isSpaceToken('\n')) token_pushback_ch('\n');
				return;
			}
		} else {
			m_token_at_end = 1;
			return;
		}
	}
}

void Tokenizer::read_till_close_comment() {
	TokenizerPos start = m_token_count;
	int prev_ch = 0;
	while (1) {
		int token_ch = token_read_char();
		if (prev_ch == '*' && token_ch == '/') {
			return;
		}
		if (m_token_at_end == 1) {
			start.incCol(-1);
			throw error(start, "comment block '/*' not terminated");
		}
		prev_ch = token_ch;
	}
}

ParserError Tokenizer::throwError(const char* s1, const char* s2, const char* s3) {
   return error(s1, s2, s3);
}

ParserError Tokenizer::throwError(int pos, const string& msg) {
   return error(pos, msg);
}

ParserError Tokenizer::throwError(const string& msg) {
   return error(msg);
}

int Tokenizer::getErrorPosition() const {
   return token_pos_col();
}

ParserError Tokenizer::error(const char* s1, const char* s2, const char* s3) const {
   ostringstream err;
   err << s1 << s2 << s3;
   return error(err.str());
}

ParserError Tokenizer::error(const string& src) const {
	ParserError err(src, token_pos(), m_fname);
	const char* parse_str = parse_string_in_error();
	if (parse_str != NULL) err.setParserString(parse_str);
	return err;
}

ParserError Tokenizer::error(const TokenizerPos& pos, const string& src) const {
	ParserError err(src, pos, m_fname);
	const char* parse_str = parse_string_in_error();
	if (parse_str != NULL) err.setParserString(parse_str);
	return err;
}

ParserError Tokenizer::eof_error() const {
	ParserError err("unexpected end of file", token_pos(), m_fname);
	err.setFlag(TOK_PARSER_ERROR_ATEND);
	const char* parse_str = parse_string_in_error();
	if (parse_str != NULL) err.setParserString(parse_str);
	return err;
}

const char* Tokenizer::parse_string_in_error() const {
	return NULL;
}

ParserError Tokenizer::error(int column, const string& src) const {
	TokenizerPos pos;
	pos.set(-1, column);
	return ParserError(src, pos, m_fname);
}

StreamTokenizer::StreamTokenizer() : Tokenizer() {
	m_fb = NULL;
	m_is = NULL;
}

StreamTokenizer::StreamTokenizer(TokenizerLanguage* lang) : Tokenizer(lang) {
	m_fb = NULL;
	m_is = NULL;
}

StreamTokenizer::StreamTokenizer(istream* _is) : Tokenizer(), m_is(_is)  {
	m_fb = NULL;
}

StreamTokenizer::StreamTokenizer(istream* _is, TokenizerLanguage* lang) : Tokenizer(lang), m_is(_is) {
	m_fb = NULL;
}

StreamTokenizer::~StreamTokenizer() {
	close_tokens();
}

void StreamTokenizer::open_tokens(const char* fname) {
	m_fb = new filebuf();
	m_fb->open(fname, ios::in);
	if (!m_fb->is_open()) {
		ostringstream err_str;
		err_str << "can't open: '" << fname << "': ";
		str_get_system_error(err_str);
		throw error(err_str.str());
	}
	m_fname = fname;
	m_is = new istream(m_fb);
}

void StreamTokenizer::open_tokens(const string& fname) {
	open_tokens(fname.c_str());
}

void StreamTokenizer::open_tokens(istream* strm, const char* fname) {
	m_fb = NULL;
	m_is = strm;
	m_fname = fname;
}

void StreamTokenizer::close_tokens() {
	if (m_fb != NULL) {
		m_fb->close();
		delete m_fb;
		m_fb = NULL;
		delete m_is;
		m_is = NULL;
	}
}

int StreamTokenizer::stream_ok() {
	return m_is->good();
}

int StreamTokenizer::stream_get() {
	return m_is->get();
}

StringTokenizer::StringTokenizer(bool show_str_err) : Tokenizer() {
	init_st();
	m_token_count.set_line(0);
	m_show_str_err = show_str_err;
}

StringTokenizer::StringTokenizer(TokenizerLanguage* lang, bool show_str_err) : Tokenizer(lang) {
	init_st();
	m_token_count.set_line(0);
	m_show_str_err = show_str_err;
}

StringTokenizer::StringTokenizer(const char* tokens) {
	init_st(tokens);
	m_token_count.set_line(0);
	m_show_str_err = true;
}

StringTokenizer::StringTokenizer(const char* tokens, TokenizerLanguage* lang) : Tokenizer(lang) {
	init_st(tokens);
	m_token_count.set_line(0);
	m_show_str_err = true;
}

StringTokenizer::~StringTokenizer() {
}

void StringTokenizer::init_st() {
	m_tokens = NULL;
	m_len = 0;
	m_pos = -1;
}

void StringTokenizer::init_st(const char* tokens) {
	m_tokens = tokens;
	m_len = strlen(tokens);
	m_pos = -1;
}

void StringTokenizer::set_string(const char* tokens) {
	init_st(tokens);
	reset_nopos();
	m_token_count.set_col(0);
}

void StringTokenizer::set_string(const string& tokens) {
	init_st(tokens.c_str());
	reset_nopos();
	m_token_count.set_col(0);
}

int StringTokenizer::stream_ok() {
	return (int)(m_pos < m_len);
}

int StringTokenizer::stream_get() {
	m_pos++;
	return m_pos < m_len ? m_tokens[m_pos] : ' ';
}

const char* StringTokenizer::parse_string_in_error() const {
	if (m_show_str_err) return m_tokens;
	else return NULL;
}

void StringTokenizer::goto_position(const TokenizerPos& pos) {
	Tokenizer::goto_position(pos);
	int offs = 0;
	for (int idx = 0; idx < m_len; idx++) {
		char ch = m_tokens[idx];
		if (ch == '\t') {
			offs = (offs/8 + 1)*8;
		} else {
			offs++;
		}
		if (offs == pos.getColumn() - 1) {
			m_pos = idx;
			if (m_pos < m_len) {
				m_token_at_end = 0;
			}
			break;
		}
	}
}

TokenizerLanguagePtr g_SpaceLang;

TokenizerLanguage* createSpaceLanguage() {
	if (g_SpaceLang.isNull()) {
		g_SpaceLang = new TokenizerLanguage();
		g_SpaceLang->setSpaceTokens(" ,\t\r\n");
	}
	return g_SpaceLang.get();
}

SpaceStringTokenizer::SpaceStringTokenizer(const char* tokens) : StringTokenizer(tokens, createSpaceLanguage()) {
}

SpaceStringTokenizer::~SpaceStringTokenizer() {
}

MyOutputFile::MyOutputFile() {
	m_OS = NULL;
}

MyOutputFile::~MyOutputFile() {
	close();
}

void MyOutputFile::open(const char* fname) {
	close();
	m_FB.open(fname, ios::out);
	m_OS = new ostream(&m_FB);
}

void MyOutputFile::open(const string& fname) {
	close();
	m_FB.open(fname.c_str(), ios::out);
	m_OS = new ostream(&m_FB);
}

void MyOutputFile::close() {
	if (m_OS != NULL) {
		m_FB.close();
		delete m_OS;
		m_OS = NULL;
	}
}

