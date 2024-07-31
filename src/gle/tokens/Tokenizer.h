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

#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include <stdarg.h>

#include "RefCount.h"
#include "CharBitMap.h"
#include "StringKeyHash.h"

#define TOKENIZER_LANG_NONE 	-1

int strcontains(const char* str, char ch);

int strposition(const char* str, char ch);

bool is_float(const string& strg);
bool is_integer(const string& strg);
bool is_integer_e(const string& strg);

void strip_string_markers(string& strg);

ostream& mtab(ostream &os, int nb);

class TokenizerPos {
private:
	int m_col;
	int m_line;
public:
	TokenizerPos();
	TokenizerPos(const TokenizerPos& pos);
	void set(int line, int column);
	void set_line(int line);
	void set_col(int column);
	bool isValid() const;
	inline int getColumn() const { return m_col; };
	inline void setColumn(int col) { m_col = col; };
	inline int getLine() const { return m_line; };
	inline void incCol(int nb) { m_col += nb; };
	inline void incCol() { m_col ++; };
	inline void incTab() { m_col = (m_col/8 + 1)*8; };
	void incRow();
	string getString(int tab1, int tab2) const;
	ostream& write(ostream &os) const;
	int equals(TokenizerPos& pos) const;
};

inline ostream& operator<<(ostream& os, const TokenizerPos& pos) {
	return pos.write(os);
}

#define TOK_PARSER_ERROR_PSTRING 1
#define TOK_PARSER_ERROR_ATEND   2

class ParserError {
private:
	int m_flag;
	string m_txt;
	string m_fname;
	string m_parsestr;
	TokenizerPos m_pos;
public:
	ParserError(const string& txt, const TokenizerPos& pos, const char* fname);
	ParserError(const ParserError& err);
	ostream& write(ostream& os) const;
	void toString(string& str) const;
	void setParserString(const char* str);
	void setParserString(const string& str);
	int equals(ParserError* err) const;
	inline const string& msg() const { return m_txt; }
	inline const string& file() const { return m_fname; }
	inline int getColumn() { return m_pos.getColumn(); };
	inline void setColumn(int col) { m_pos.setColumn(col); };
	inline void incColumn(int nb) { m_pos.incCol(nb); };
	inline void setMessage(const string& msg) { m_txt = msg; };
	inline bool hasFlag(int flag) { return (m_flag & flag) != 0; };
	inline void setFlag(int flag) { m_flag |= flag; }
	inline const string& getParserString() { return m_parsestr; }
};

inline ostream& operator<<(ostream& os,const ParserError& err) {
	return err.write(os);
}

class IThrowsError {
public:
   IThrowsError();
   virtual ~IThrowsError();

   virtual ParserError throwError(int pos, const string& error);
   virtual ParserError throwError(const char* s1, const char* s2, const char* s3);
   virtual ParserError throwError(const string& error);
   virtual int getErrorPosition() const;
};

IThrowsError* g_get_throws_error();

double tokenizer_string_to_double(const char* value);

#if defined(__UNIX__) || defined(__MAC__)
	ParserError g_format_parser_error(const char* format, ...);
#else
	ParserError g_format_parser_error(va_list format, ...);
#endif

void g_throw_parser_error(const string& err);

void g_throw_parser_error(const char* err, int idx);

void g_throw_parser_error(const char* str1, const char* str2, const char* str3);

void g_throw_parser_error_sys(const char* str1, const char* str2 = NULL, const char* str3 = NULL);

#define MAX_PUSHBACK_CH 2

class Tokenizer;

class TokenizerLangElem : public RefCountObject  {
protected:
	string m_name;
public:
	inline void addName(string& add) { m_name += add; };
	inline const string& getName() const { return m_name; };
	inline int length() const { return m_name.length(); };
};

class TokenizerLangHash;

class TokenizerLangHashPtr : public RefCountPtr<TokenizerLangHash> {
 public:
  inline ostream& write(ostream &os, int tab) const;
  inline void addLangElem(Tokenizer* tokens, TokenizerLangElem* elem);
  inline void addLangElem(const vector<string>& toks,
			  TokenizerLangElem* elem,
			  unsigned int pos);
  inline TokenizerLangHashPtr(const string& name);
  inline TokenizerLangHashPtr(TokenizerLangHash* src);
  inline TokenizerLangHashPtr();
};

class TokenizerLangHash :  public StringKeyHash<TokenizerLangHashPtr>, public RefCountObject  {
private:
	MutableRefCountPtr<TokenizerLangElem> m_default;
public:
	TokenizerLangHash();
	TokenizerLangHash(const string& name);
	~TokenizerLangHash();
	ostream& write(ostream &os, int depth) const;
	void addLangElem(Tokenizer* tokens, TokenizerLangElem* elem);
	void addLangElem(const vector<string>& toks,
			 TokenizerLangElem* elem,
			 unsigned int pos);
	inline int hasDefault() const { return !m_default.isNull(); };
	inline TokenizerLangElem* getDefault() const { return m_default.get(); };
};

typedef TokenizerLangElem* TokLangElemPtr;

typedef TokenizerLangHash* TokLangHashPtr;

class TokenizerLanguageMultiLevel : public RefCountObject {
private:
	char m_open_tokens[255];
	CharBitMap m_close_tokens;
	CharBitMap m_end_tokens;
public:
	TokenizerLanguageMultiLevel();
	void setOpenClose(char open, char close);
	inline void setEndToken(char end) { m_end_tokens.set(end); };
	inline void resetEndToken(char end) { m_end_tokens.reset(end); };
	inline bool isOpenToken(char tok) { return m_open_tokens[(int)tok] != 0; }
	inline char getCloseToken(char tok) { return m_open_tokens[(int)tok]; }
	inline bool isCloseToken(char tok) { return m_close_tokens.get(tok); }
	inline bool isEndToken(char tok) { return m_end_tokens.get(tok); }

};

class TokenizerLanguage : public RefCountObject {
private:
	char m_decimal_dot;
	int m_enable_c_comm;
	int m_enable_cpp_comm;
	int m_parse_strings;
	CharBitMap m_one_char_tokens;
	CharBitMap m_space_tokens;
	CharBitMap m_line_comment_tokens;
	vector<TokenizerLangHashPtr> m_sublanguage;
	TokLangElemPtr* m_index;
	RefCountPtr<TokenizerLanguageMultiLevel> m_multi;
public:
	TokenizerLanguage();
	~TokenizerLanguage();
	void addSubLanguages(int nb);
	void addLanguageElem(int sublang, const char* elem);
	void addLanguageElem(int sublang, const vector<string>& toks, TokenizerLangElem* elem);
	void addElementIndex(int size);
	void setElement(int i, TokenizerLangElem* elem);
	void resetCharMaps();
	inline int getParseStrings() const {return m_parse_strings;}
	inline void setParseStrings(int ps) {m_parse_strings=ps;}
	inline TokenizerLangElem* getElement(int i) { return m_index[i]; };
	inline int  isDecimalDot(char ch) const {return m_decimal_dot && (ch==m_decimal_dot);}
	inline char getDecimalDot() const {return m_decimal_dot;}
	inline void setDecimalDot(char ch) {m_decimal_dot=ch;}
	inline int  isSingleCharToken(char ch) const { return m_one_char_tokens.get(ch); };
	inline void setSingleCharTokens(const char* sct) { m_one_char_tokens.set(sct); };
	inline int  isSpaceToken(char ch) const { return m_space_tokens.get(ch); };
	inline void setSpaceTokens(const char* sct) { m_space_tokens.set(sct); };
	inline int  isLineCommentToken(char ch) const { return m_line_comment_tokens.get(ch); };
	inline void setLineCommentTokens(const char* sct) { m_line_comment_tokens.set(sct); };
	inline int  isEnableCComment() const { return m_enable_c_comm; };
	inline int  isEnableCPPComment() const { return m_enable_cpp_comm; };
	inline void enableCComment() { m_enable_c_comm = 1; };
	inline void enableCPPComment() { m_enable_cpp_comm = 1; };
	inline TokenizerLangHashPtr getLanguage(int i) const { return m_sublanguage[i];};
	inline void setMulti(TokenizerLanguageMultiLevel* multi) { m_multi = multi; };
	inline TokenizerLanguageMultiLevel* getMulti() { return m_multi.get(); }
	void initDefault();
	void initDefaultSingleCharTokens();
	void initDefaultSpaceTokens();
};

typedef MutableRefCountPtr<TokenizerLanguage> TokenizerLanguagePtr;

class TokenAndPos {
protected:
	bool m_space;
	string m_token;
	TokenizerPos m_pos;
public:
	TokenAndPos();
	TokenAndPos(const TokenAndPos& copy);
	TokenAndPos(const string& token, const TokenizerPos& pos, char space);
	~TokenAndPos();
	inline const string& getToken() const { return m_token; };
	inline const TokenizerPos& getPos() const { return m_pos; };
	inline bool getSpace() const { return m_space; };
	inline void setSpace(bool space) { m_space = space; };
	inline void setToken(const string& token) { m_token = token; };
	inline void setPos(const TokenizerPos& pos) { m_pos = pos; };
};

class Tokenizer : public IThrowsError {
protected:
	const char* m_fname;
	string m_token;
	int  m_token_at_end;
	int  m_token_has_pushback;
	int  m_token_has_pushback_ch;
	bool m_space_before;
	bool m_space_after;
	TokenizerPos m_token_start;
	TokenizerPos m_token_count;
	TokenizerLangHashPtr m_langhash;
	TokenizerLanguage* m_language;
	vector<TokenAndPos> m_pushback_tokens;
	char m_token_pushback_ch[MAX_PUSHBACK_CH];
public:

	Tokenizer();
	Tokenizer(TokenizerLanguage* lang);
	virtual ~Tokenizer();

	int has_more_tokens();
	// end of stream not reached

	string& next_token();
	// returns next token.  throws exception if no more tokens.

	string& next_continuous_string_excluding(const char* forbidden);

	string& try_next_token();

	string& read_line();

	double next_double();

	int next_integer();
	// reads an integer token.  throws exception if token is not
	// an integer

	int try_next_integer(int *i);
	// gets an integer token in [*i] and returns 1 or returns 0 if
	// next token is not an integer

	TokenizerLanguage* get_language();

	void delete_language();

	void select_language(int i);

	TokenizerLangElem* try_find_lang_elem(int i);

	void next_token_and_pos(TokenAndPos& tkpos);

	void pushback_token();

	void pushback_token(const TokenAndPos& tkpos);

	void pushback_token(const string& token, const TokenizerPos& pos);

	void pushback_token(const string& token);

	void pushback_token(const char* token);

	void peek_token(string* token);
	// look at next token without moving the current token pointer

	bool has_space_before() const { return m_space_before; };

	int token_line() const;
	// current line number

	int token_column() const;
	// current column number

	void set_line(int line);

	void inc_line();

	inline const TokenizerPos& token_pos() const { return m_token_start; };

	inline int token_pos_col() const { return m_token_start.getColumn(); };

	inline const TokenizerPos& token_stream_pos() const { return m_token_count; };

	int is_next_token(const char* token);

	int is_next_token_i(const char* token);

	int is_next_token(const string& token) {
		return is_next_token(token.c_str());
	}
	// checks whether next token is [token].  if so, the token is
	// consumed (current token pointer moved).   Otherwise,
	// the next token is not consumed.

	int is_next_token_in(const char* charlist);

	void ensure_next_token(const char* token);
	// ensures the next token is [token] and moves current token pointer
	// raises exception if next token is not [token]

	void ensure_next_token_i(const char* token);

	int ensure_next_token_in(const char* charlist);
	// reads a one-character token occurring in charlist.
	// raises exception when the next token is not as expected.

	void ensure_next_token_list(const char* charlist);
	// reads a list of one-character tokens

	void token_skip_to_end();
	// skips to end of line

	void read_till_close_comment();

	string& next_multilevel_token();

	virtual char token_read_sig_char();

	virtual void on_trailing_space();

	char token_read_char();

	char token_read_char_no_comment();

	void read_number_term(char token_ch, bool has_e, bool sure_num);

	virtual int stream_ok() = 0;

	virtual int stream_get() = 0;

	virtual void goto_position(const TokenizerPos& pos);

	inline void set_fname(const char* fname) { m_fname = fname; };

	inline void set_fname(const string& fname) { m_fname = fname.c_str(); };

	inline const char* get_fname() { return m_fname; };

	inline void token_pushback_ch(char ch) {
		m_token_pushback_ch[m_token_has_pushback_ch++] = ch;
	}

	virtual ParserError throwError(const char* s1, const char* s2, const char* s3);

	virtual ParserError throwError(int pos, const string& error);

	virtual ParserError throwError(const string& error);

	virtual int getErrorPosition() const;

	ParserError error(const string& src) const;

	ParserError error(const char* s1, const char* s2, const char* s3) const;

	ParserError error(const TokenizerPos& pos, const string& src) const;

	ParserError error(int column, const string& src) const;

	ParserError eof_error() const;

	virtual const char* parse_string_in_error() const;

protected:
	void init();
	void reset_position();
	void reset_nopos();
	void reset_all();
  	void undo_pushback_token();
	void copy_string(char endch);
	void multi_level_do_multi(char open);
	void get_token();
	// consumes one token (and forgets it?)
	// raises exception when e.g. a string constant is not terminated.
	void get_token_2();
	void get_check_token();
	TokenizerLangElem* findLangElem(const TokenizerLangHash* hash);
	TokenizerLangElem* findLangElem2(const TokenizerLangHash* hash);
};

class StreamTokenizer : public Tokenizer {
protected:
	filebuf* m_fb;
	istream* m_is;
public:
	StreamTokenizer();
	StreamTokenizer(TokenizerLanguage* lang);
	StreamTokenizer(istream* _is);
	StreamTokenizer(istream* _is, TokenizerLanguage* lang);
	virtual ~StreamTokenizer();
	void open_tokens(const char* fname);
	void open_tokens(const string& fname);
	void open_tokens(istream* strm, const char* fname);
	void close_tokens();
	virtual int stream_ok();
	virtual int stream_get();
	inline istream* getStream() { return m_is; };
};

class StringTokenizer : public Tokenizer {
protected:
	const char* m_tokens;
	int m_len;
	int m_pos;
	bool m_show_str_err;
protected:
	void init_st();
	void init_st(const char* tokens);
public:
	StringTokenizer(bool show_str_err = true);
	StringTokenizer(TokenizerLanguage* lang, bool show_str_err = true);
	StringTokenizer(const char* tokens);
	StringTokenizer(const char* tokens, TokenizerLanguage* lang);
	virtual ~StringTokenizer();
	void set_string(const char* tokens);
	void set_string(const string& tokens);
	virtual int stream_ok();
	virtual int stream_get();
	virtual void goto_position(const TokenizerPos& pos);
	virtual const char* parse_string_in_error() const;
	inline void setShowStringInError(bool show) { m_show_str_err = show; }
};

class SpaceStringTokenizer : public StringTokenizer {
public:
	SpaceStringTokenizer(const char* tokens);
	virtual ~SpaceStringTokenizer();
};

class MyOutputFile {
protected:
	filebuf m_FB;
	ostream* m_OS;
public:
	MyOutputFile();
	~MyOutputFile();
	void open(const char* fname);
	void open(const string& fname);
	void close();
	inline ostream& get() { return *m_OS; }
	inline int isOpen() { return m_OS != NULL; }
};

#include "Tokenizer.i"

#endif
