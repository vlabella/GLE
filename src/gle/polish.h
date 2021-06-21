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

#ifndef INCLUDE_POLISH
#define INCLUDE_POLISH

#define PCODE_EXPR 1
#define PCODE_DOUBLE 2
#define PCODE_VAR 3
#define PCODE_STRVAR 4
#define PCODE_STRING 5
#define PCODE_OBJECT 6
#define PCODE_NEXT_CMD 7

class GLESub;

class GLEArrayImpl;

const int BINARY_OPERATOR_OFFSET = 10;

enum BinaryOperators {
	BIN_OP_PLUS = 1,
	BIN_OP_MINUS,
	BIN_OP_MULTIPLY,
	BIN_OP_DIVIDE,
	BIN_OP_POW,
	BIN_OP_EQUALS,
	BIN_OP_LT,
	BIN_OP_LE,
	BIN_OP_GT,
	BIN_OP_GE,
	BIN_OP_NOT_EQUALS,
	BIN_OP_AND,
	BIN_OP_OR,
	BIN_OP_MOD,
	BIN_OP_DOT
};

std::string gle_operator_to_string(int op);

void setEvalStack(GLEArrayImpl* stk, int pos, double value);
void setEvalStack(GLEArrayImpl* stk, int pos, const char* value);
void setEvalStack(GLEArrayImpl* stk, int pos, int value);
double getEvalStackDouble(GLEArrayImpl* stk, int pos);
char* getEvalStackString(GLEArrayImpl* stk, int pos);
GLEColor* getEvalStackColor(GLEArrayImpl* stk, int pos);

class GLEPcodeList : public RefCountObject {
public:
	inline int size() const { return m_ConstObjects.size(); }
	inline GLEDataObject* get(int i) { return m_ConstObjects[i].get(); }
	inline void push_back(GLEDataObject* obj) { m_ConstObjects.push_back(obj); }

protected:
	RefCountVector<GLEDataObject> m_ConstObjects;
};

class GLEPcode : public vector<int> {
protected:
	GLEPcodeList* m_PCodeList;
public:
	GLEPcode(GLEPcodeList* list);
	inline void addInt(int val) { push_back(val); };
	inline int getInt(int idx) { return (*this)[idx]; };
	inline void setInt(int idx, int val) { (*this)[idx] = val; };
	inline void setLast(int val) { (*this)[size()-1] = val; };
	inline GLEPcodeList* getPcodeList() { return m_PCodeList; }
	void addColor(GLEColor* color);
	void addDoubleExpression(double val);
	void addStringExpression(const char* val);
	void addDouble(double val);
	void addFunction(int idx);
	void addVar(int var);
	void addStrVar(int var);
	void addString(const string& str);
	void addStringNoID(const string& str);
	void addStringChar(const char* str);
	void addStringNoIDChar(const char* str);
	void show();
	void show(int start);
};

class GLEPcodeIndexed : public GLEPcode {
protected:
	vector<int> m_Index;
public:
	GLEPcodeIndexed(GLEPcodeList* list);
	inline int getNbEntries() { return m_Index.size(); }
	inline void addIndex(int idx) { m_Index.push_back(idx); }
	inline int getIndex(int idx) { return m_Index[idx]; }
	inline int getSize(int idx) { return m_Index[idx+1]-m_Index[idx]; }
};

class GLEFunctionParserPcode : public GLERefCountObject {
protected:
	GLEPcode m_Pcode;
	GLEPcodeList m_PcodeList;
public:
	GLEFunctionParserPcode();
	~GLEFunctionParserPcode();
	void polish(const char* fct, StringIntHash* vars = NULL) throw(ParserError);
	void polishPos(const char* fct, int pos, StringIntHash* vars = NULL) throw(ParserError);
	void polishX() throw(ParserError);
	double evalDouble();
	bool evalBool();
	inline GLEPcode* getCode() { return &m_Pcode; }
};

class GLEPolish : public RefCountObject {
protected:
	TokenizerLanguage m_lang;
	StringTokenizer m_tokens;
	StringIntHash* m_vars;
public:
	GLEPolish();
	~GLEPolish();
	void internalPolish(const char *expr, GLEPcode& pcode, int *rtype) throw(ParserError);
	void internalPolish(GLEPcode& pcode, int *rtype) throw(ParserError);
	void internalEval(const char *exp, double *x) throw(ParserError);
    void internalEvalString(const char* exp, string* str) throw(ParserError);
	void polish(const char *expr, GLEPcode& pcode, int *rtype) throw(ParserError);
	void eval(GLEArrayImpl* stk, const char *exp, double *x) throw(ParserError);
	void evalString(GLEArrayImpl* stk, const char *exp, string *str, bool allownum = false) throw(ParserError);
	GLEMemoryCell* evalGeneric(GLEArrayImpl* stk, const char *exp) throw(ParserError);
	void get_params(GLEPcode& pcode, int np, int* plist, const string& name) throw(ParserError);
	void get_array_index(GLEPcode& pcode) throw(ParserError);
	double evalTokenToDouble() throw(ParserError);
	Tokenizer* getTokens(const string& str);
	void initTokenizer();
	inline void setExprVars(StringIntHash* vars) { m_vars = vars; }
	inline ParserError error(int column, const string& src) const {
		return m_tokens.error(column, src);
	};
	inline ParserError error(const string& src) const {
		return m_tokens.error(src);
	};
};

void polish_eval(char *exp, double *x) throw(ParserError);
void polish_eval_string(const char *exp, string *str, bool allownum = false) throw(ParserError);
void polish(char *expr, GLEPcode& pcode, int *rtype) throw(ParserError);
void eval_pcode(GLEPcode& pcode, double* x);
void eval_pcode_str(GLEPcode& pcode, string& x);

GLEPolish* get_global_polish();

void stack_op(char *pcode, int *plen, int stk[] , int stkp[], int *nstk,  int i, int p);

#endif
