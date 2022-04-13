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

char *un_quote();
const char *ns[3] = {"nothing", "number", "string"};
extern int gle_debug;

/*---------------------------------------------------------------------------*/

#include "all.h"
#include "mem_limits.h"
#include "token.h"
#include "gle-interface/gle-interface.h"
#include "glearray.h"
#include "polish.h"
#include "pass.h"
#include "var.h"
#include "sub.h"
#include "gprint.h"
#include "cutils.h"
#include "keyword.h"
#include "run.h"
#include "fn.h"

/*---------------------------------------------------------------------------*/
/* bin = 10..29, binstr = 30..49, fn= 60...139, userfn=LOCAL_START_INDEX..nnn */
#define stack_bin(i,p) stack_op(pcode, stk, stkp, &nstk, i + BINARY_OPERATOR_OFFSET, p + curpri)
#define stack_fn(i)    stack_op(pcode, stk, stkp, &nstk, i + FN_BUILTIN_MAGIC, 10 + curpri)
#define dbg if ((gle_debug & 4)>0)

// #define dbg

/*---------------------------------------------------------------------------*/
/* Input is token array, and pointer to current point, output is pcode */

void stack_op(GLEPcode& pcode, int stk[], int stkp[], int *nstk,  int i, int p);
bool valid_unquoted_string(const string& str);

GLEPolish::GLEPolish() : m_lang(), m_tokens(&m_lang, false) {
	m_vars = NULL;
}

GLEPolish::~GLEPolish() {
}

void GLEPolish::initTokenizer() {
	TokenizerLanguage* lang = m_tokens.get_language();
	lang->setSpaceTokens(" \t\r\n");
	lang->setLineCommentTokens("!");
	lang->setSingleCharTokens(",.:;[]{}()+-*/=<>|^%\\");
	lang->setDecimalDot('.');
	lang->addSubLanguages(1);
	lang->addLanguageElem(0, "<=");
	lang->addLanguageElem(0, ">=");
	lang->addLanguageElem(0, "<>");
	lang->addLanguageElem(0, "**");
	m_tokens.select_language(0);
}

void GLEPolish::get_array_index(GLEPcode& pcode) throw(ParserError) {
	int vtype = 1;
	internalPolish(pcode, &vtype);
	m_tokens.ensure_next_token("]");
}

int GLEPolish::get_params(GLEPcode& pcode, int np, int* plist, const string& name, int np_default ) throw(ParserError) {
	// called when subroutine is a left hand argument => a = myfunc(4,5)
	// returns the number of parameters found
	int nb_param = 0;
	//printf("get_params %d %d\n",np,np_default);
	//gprint("get_params %d %d",np,np_default);
	if (!m_tokens.is_next_token(")")) {
		while (true) {
			if (nb_param >= np) {
				char err_str[100];
				sprintf(err_str, "': found >= %d, expected %d to %d", nb_param+1, np-np_default,np);
				throw error(string("too many parameters in call to '")+name+err_str);
			}
			int vtype = *(plist + nb_param);
			internalPolish(pcode, &vtype);
			int next_token = m_tokens.is_next_token_in(",)");
			if (next_token == -1) {
				throw error(string("expecting ',' or ')' in parameter list of function '")+name+"'");
			}
			nb_param++;
			if (next_token == ')') break;
		}
	}
	if (nb_param < (np-np_default) ) {
		char err_str[100];
		sprintf(err_str, "': found %d, expected at least %d", nb_param, np-np_default);
		throw error(string("incorrect number of parameters in call to '")+name+err_str);
	}
	return nb_param;
}

void GLEPolish::polish(const char *expr, GLEPcode& pcode, int *rtype) throw(ParserError) {
	try {
		internalPolish(expr, pcode, rtype);
	} catch (ParserError& err) {
		err.setParserString(expr);
		throw err;
	}
}

void GLEPolish::internalPolish(const char *expr, GLEPcode& pcode, int *rtype) throw(ParserError) {
	#ifdef DEBUG_POLISH
		gprint("==== Start of expression {%s} \n",expr);
	#endif
	m_tokens.set_string(expr);
	internalPolish(pcode, rtype);
}

void GLEPolish::internalPolish(GLEPcode& pcode, int *rtype) throw(ParserError) {
	GLESub* sub;
	string uc_token;
	int idx, ret, np, *plist, term_bracket = false;
	int curpri = 0;
	int nstk = 0, stk[50], stkp[50];   /* stack for operators */
	int unary = 1;                     /* binary or unary operation expected */
	bool isa_string = false;
	bool not_string = false;
	if (*rtype==1) not_string = true;
	if (*rtype>0) term_bracket = true;
	pcode.addInt(PCODE_EXPR);   /* Expression follows */
	int savelen = pcode.size(); /* Used to set actual length at end */
	pcode.addInt(0);	    /* Length of expression */
	while (true) {
		string token = m_tokens.try_next_token();
		int token_col = m_tokens.token_pos_col();
		int token_len = token.length();
		char first_char = token_len > 0 ? token[0] : ' ';
		//cout << "Token: '" << token << "'" << endl;
		// end of stream, or found ',' or ')'
		if (token_len == 0 || (token_len == 1 && (first_char == ',' || (first_char == ')' && curpri == 0) || (first_char == ']' && curpri == 0)))) {
			if (token_len != 0) {
				m_tokens.pushback_token();
			}
			*rtype = 0;
			dbg gprint("Found END OF EXPRESSION \n");
			if (curpri != 0) {
				throw error("unexpected end of expression, missing closing ')' or ']'");
			}
			/* Pop everything off the stack */
			for (int i = nstk; i > 0; i--) {
				dbg gprint("Adding left over operators  I = %d  op=%d \n",i,stk[i]);
				pcode.addInt(stk[i]);
			}
			if (unary == 1) {
				throw error("constant, function, or unary operator expected");
			}
			pcode.setInt(savelen, pcode.size() - savelen - 1);
			#ifdef DEBUG_POLISH
				pcode.show(savelen);
			#endif
			return;
		}
		dbg gprint("First word token via (1=unary %d) cts {%s}\n ", unary, token.c_str());
		switch (unary) {
		case 1:  /* a unary operator, or function, or number or variable */
			if (is_float(token)) {
				dbg gprint("Found number {%s}\n",token.c_str());
				double value = atof(token.c_str());
				pcode.addDouble(value);
				unary = 2;
				break;
			}
			str_to_uppercase(token, uc_token);
			/* NOT a number, is it a built in function? */
			find_un((char*)uc_token.c_str(), &idx, &ret, &np, &plist);
			/* 1,2 = +,- */
			if (idx > 3 && m_tokens.is_next_token("(")) {
				//
				// it is a built in function (do built in functions have default or optional arguments??)
				//
				dbg gprint("Found built in function \n");
				get_params(pcode, np, plist, uc_token);
				pcode.addFunction(idx + FN_BUILTIN_MAGIC);
				unary = 2;
				break;
			} else if (idx > 0 && idx <= 3) {
				stack_fn(idx);
				unary = 1;
				break;
			}
			/* Is it a user-defined function, identical code too above. */
			sub = sub_find((char*)uc_token.c_str());
			if (sub != NULL && m_tokens.is_next_token("(")) {
				//
				// it is a user defined function
				//
				//printf("User idx=%d ret=%d np=%d plist=%d\n",idx,ret,np,plist);
				dbg gprint("Found user function \n");
				// modify to handle default arguments
				//printf("pcode size = %d\n",pcode.size());
				int np_found = get_params(pcode, sub->getNbParam() , sub->getParamTypes(), uc_token, sub->getNbDefault());
				//printf("pcode size = %d\n",pcode.size());
				int def_param_index = 0;
				while( np_found < sub->getNbParam() ){
					// add in the default parameters if they were not specified
					const string& value = sub->getDefault(np_found);
					if (value != "") {
						// add in the default values to the pcode which must be constant int, float, or string
						pcode.addInt(PCODE_EXPR);   /* Expression follows */
						pcode.addInt(0);
						if (is_float(value)) {
							double v = atof(value.c_str());
							pcode.addDouble(v);
						}else if(is_integer(value) || is_integer_e(value)){
							int v = atoi(value.c_str());
							pcode.addInt(v);
						}else{
							pcode.addString(value);
						}
					}
					np_found++;
				}
				//printf("pcode size = %d\n",pcode.size());
				/* fill in default values taken from pass.cpp*/
				#if 0
				bool has_all = true;
				for (int i = 0; i < sub->getNbParam(); i++) {
					if (sub->getParamPos(i) == -1) {
						const string& value = sub->getDefault(i);
						if (value != "") {
							sub->setParam(i, value, -2); /* 2 indicates default */
						} else {
							has_all = false;
						}
					}
				}
				
				/* error if some not given */
				
				if (!has_all) {
					int count = 0;
					stringstream err;
					err << "insufficient arguments in call to '" << sub->getName() << "': no value for: ";
					for (int i = 0; i < sub->getNbParam(); i++) {
						if (sub->getParamPos(i) == -1) {
							if (count != 0) err << ", ";
							err << sub->getParamNameShort(i);
							count++;
						}
					}
					throw error(poscol, err.str());
				}
				*
				#endif
				pcode.addFunction(sub->getIndex()+LOCAL_START_INDEX);
				unary = 2;
				break;
			}
			/* Is it a 'known' variable */
			int v;
			var_find((char*)uc_token.c_str(), &v, &ret);
			if (v >= 0) {
				// cout << "found var: '" << uc_token << "' -> " << v << endl;
				if (ret == 2) pcode.addStrVar(v);
				else pcode.addVar(v);
				unary = 2;
				if (m_vars != NULL && m_vars->try_get(uc_token) == -1) {
					/* Add it to list of vars */
					m_vars->add_item(uc_token, v);
				}
				break;
			}
			/* Is it a string */
			if (first_char == '"' || first_char == '\'') {
				dbg gprint("Found string \n");
				string str_no_quote = token;
				str_remove_quote(str_no_quote);
				pcode.addString(str_no_quote);
				unary = 2;
				break;
			}
			if ((first_char == 'd' || first_char == 'D') && token_len == 1 && m_tokens.is_next_token("[")) {
				get_array_index(pcode);
				pcode.addFunction(FN_DI + FN_BUILTIN_MAGIC);
				unary = 2;
				break;
			}
			if (first_char == '(' && token_len == 1) {
				curpri = curpri + 100;
				break;
			}
			if ((first_char == ')' || first_char == ']') && token_len == 1) {
				throw error("constant, function, or unary operator expected");
			}
			if (m_tokens.is_next_token("(")) {
				throw error(token_col, string("call to undefined function '"+token+"'"));
			}
			/* must be unquoted string, unless a binary operator
			   was found, in which case it is an undelcared variable */
			if (not_string || str_var(token)) {
				/* name that includes '$' is also assumed to be a variable */
				dbg gprint("Found un-initialized variable {%s} /n",token.c_str());
				if (!var_valid_name(uc_token)) {
					throw error(token_col, "illegal variable name '"+uc_token+"'");
				}
				var_findadd((char*)uc_token.c_str(), &v, &ret);
				if (ret == 2) pcode.addStrVar(v);
				else pcode.addVar(v);
				not_string = true;
				unary = 2;
				if (m_vars != NULL && m_vars->try_get(uc_token) == -1) {
					/* Add it to list of vars */
					m_vars->add_item(uc_token, v);
				}
				break;
			}
			// std::cout << "Unquoted string '" << token << "'" << std::endl;
			pcode.addString(token);
			if (!valid_unquoted_string(token)) {
				throw error(token_col, "invalid unquoted string '"+token+"'");
			}
			isa_string = true;
			unary = 2;
			break;
		case 2: /* a binary operator, or space, or end of line */
			/* MIGHT (gives error with a$ = b$+c$) */
			if (first_char != '.') {
				if (isa_string) {
					throw error("left hand side contains unquoted string");
				}
				not_string = true;
			} else {
				not_string = false;
			}
			/* Binary operators, +,-,*,/,^,<,>,<=,>=,.and.,.or. */
			int priority = 0;
			if (token_len == 1) {
				switch (first_char) {
					case '+' : v = BIN_OP_PLUS;  priority = 2; break;
					case '-' : v = BIN_OP_MINUS;  priority = 2; break;
					case '*' : v = BIN_OP_MULTIPLY;  priority = 3; break;
					case '/' : v = BIN_OP_DIVIDE;  priority = 3; break;
					case '%' : v = BIN_OP_MOD; priority = 3; break;
					case '^' : v = BIN_OP_POW;  priority = 4; break;
					case '=' : v = BIN_OP_EQUALS;  priority = 1; break;
					case '&' : v = BIN_OP_AND; priority = 1; break;
					case '|' : v = BIN_OP_OR; priority = 1; break;
					case '<' : v = BIN_OP_LT;  priority = 1; break;
					case '>' : v = BIN_OP_GT;  priority = 1; break;
					case '.' : v = BIN_OP_DOT;  priority = 2; break;
					default  : v = 0;
				}
			} else {
				str_to_uppercase(token, uc_token);
				if (token == "<=") {
					v = BIN_OP_LE; priority = 1;
				} else if (token == "<>") {
					v = BIN_OP_NOT_EQUALS; priority = 1;
				} else if (token == ">=") {
					v = BIN_OP_GE; priority = 1;
				} else if (token == "**") {
					v = BIN_OP_POW;  priority = 4;
				} else if (uc_token == "AND") {
					v = BIN_OP_AND; priority = 1;
				} else if (uc_token == "OR") {
					v = BIN_OP_OR; priority = 1;
				} else {
					v = 0;
				}
			}
			if (v > 0) {
				stack_bin(v, priority);
				dbg gprint("Found binary operator \n");
				unary = 1;
			} else if (first_char == ')' && token_len == 1) {
				if (curpri > 0) {
					curpri = curpri - 100;
					unary = 2;
					break;
				}
				if (!term_bracket) {
					throw error("too many closing ')', expecting binary operator");
				}
			} else {
				throw error(string("unknown binary operator '")+token+"'");
			}
		} // end switch
	} // end for
}

Tokenizer* GLEPolish::getTokens(const string& str) {
	m_tokens.set_string(str.c_str());
	return &m_tokens;
}

void GLEPolish::internalEval(const char *exp, double *x) throw(ParserError) {
	// difference with eval: no try / catch
	int rtype = 1, cp = 0;
	GLEPcodeList pc_list;
	GLEPcode pcode(&pc_list);
	internalPolish(exp, pcode, &rtype);
	GLERC<GLEArrayImpl> stk(new GLEArrayImpl());
	*x = evalDouble(stk.get(), &pc_list, (int*)&pcode[0], &cp);
}

void GLEPolish::internalEvalString(const char* exp, string* str) throw(ParserError) {
	// difference with eval_string: no try / catch
	int rtype = 2, cp = 0;
	GLEPcodeList pc_list;
	GLEPcode pcode(&pc_list);
	internalPolish(exp, pcode, &rtype);
	GLERC<GLEArrayImpl> stk(new GLEArrayImpl());
	GLERC<GLEString> result(::evalString(stk.get(), &pc_list, (int*)&pcode[0], &cp, true));
	*str = result->toUTF8();
}

void GLEPolish::eval(GLEArrayImpl* stk, const char *exp, double *x) throw(ParserError) {
	int rtype = 1, cp = 0;
	GLEPcodeList pc_list;
	GLEPcode pcode(&pc_list);
	polish(exp, pcode, &rtype);
	*x = evalDouble(stk, &pc_list, (int*)&pcode[0], &cp);
}

void GLEPolish::evalString(GLEArrayImpl* stk, const char *exp, string *str, bool allownum) throw(ParserError) {
	int rtype = allownum ? 0 : 2;
	int cp = 0;
	GLEPcodeList pc_list;
	GLEPcode pcode(&pc_list);
	polish(exp, pcode, &rtype);
	GLERC<GLEString> result(::evalString(stk, &pc_list, (int*)&pcode[0], &cp, allownum));
	*str = result->toUTF8();
}

GLEMemoryCell* GLEPolish::evalGeneric(GLEArrayImpl* stk, const char *exp) throw(ParserError) {
	int cp = 0;
	int rtype = 0;
	GLEPcodeList pc_list;
	GLEPcode pcode(&pc_list);
	polish(exp, pcode, &rtype);
	return ::evalGeneric(stk, &pc_list, (int*)&pcode[0], &cp);
}

bool valid_unquoted_string(const string& str) {
	if (str.length() == 0) {
		return false;
	} else {
		int ch = str[0];
		if (!((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z'))) {
			return false;
		} else {
			return true;
		}
	}
}

void stack_op(GLEPcode& pcode, int stk[], int stkp[], int *nstk,  int i, int p) {
	dbg gprint("Stack oper %d priority %d \n",i,p);
	while ((*nstk)>0 && p<=stkp[*nstk]) {
		dbg gprint("ADDING oper stack = %d  oper=%d \n",*nstk,stk[(*nstk)]);
		pcode.addInt(stk[(*nstk)--]);
	}
	stk[++(*nstk)] = i;
	stkp[*nstk] = p;
}

void polish(char *expr, GLEPcode& pcode, int *rtype) throw(ParserError) {
	GLEPolish* polish = get_global_polish();
	if (polish != NULL) {
		polish->polish(expr, pcode, rtype);
	}
}

void eval_pcode(GLEPcode& pcode, double* x) {
	int cp = 0;
	GLERC<GLEArrayImpl> stk(new GLEArrayImpl());
	*x = evalDouble(stk.get(), pcode.getPcodeList(), (int*)&pcode[0], &cp);
}

void eval_pcode_str(GLEPcode& pcode, string& x) {
	int cp = 0;
	GLERC<GLEArrayImpl> stk(new GLEArrayImpl());
	GLERC<GLEString> result(::evalString(stk.get(), pcode.getPcodeList(), (int*)&pcode[0], &cp, true));
	x = result->toUTF8();
}

void polish_eval(char *exp, double *x) throw(ParserError) {
	GLEPolish* polish = get_global_polish();
	GLERC<GLEArrayImpl> stk(new GLEArrayImpl());
	if (polish != NULL) polish->eval(stk.get(), exp, x);
}

void polish_eval_string(const char *exp, string *str, bool allownum) throw(ParserError) {
	GLEPolish* polish = get_global_polish();
	GLERC<GLEArrayImpl> stk(new GLEArrayImpl());
	if (polish != NULL) polish->evalString(stk.get(), exp, str, allownum);
}

std::string gle_operator_to_string(int op) {
	switch (op) {
		case BIN_OP_PLUS:
			return "+";
		case BIN_OP_MINUS:
			return "-";
		case BIN_OP_MULTIPLY:
			return "*";
		case BIN_OP_DIVIDE:
			return "/";
		case BIN_OP_POW:
			return "^";
		case BIN_OP_EQUALS:
			return "=";
		case BIN_OP_LT:
			return "<";
		case BIN_OP_LE:
			return "<=";
		case BIN_OP_GT:
			return ">";
		case BIN_OP_GE:
			return ">=";
		case BIN_OP_NOT_EQUALS:
			return "<>";
		case BIN_OP_AND:
			return "AND";
		case BIN_OP_OR:
			return "OR";
		case BIN_OP_MOD:
			return "%";
		case BIN_OP_DOT:
			return ".";
		default:
			break;
	}
	{
		std::ostringstream msg;
		msg << "OP" << op;
		return msg.str();
	}
}

GLEPcode::GLEPcode(GLEPcodeList* list) {
	m_PCodeList = list;
}

void GLEPcode::addColor(GLEColor* color) {
	addInt(PCODE_EXPR);
	int savelen = size(); /* Used to set actual length at end */
	addInt(0);	          /* Length of expression */
	addInt(PCODE_OBJECT);
	int pos = getPcodeList()->size();
	getPcodeList()->push_back(color);
	addInt(pos);
	setInt(savelen, size() - savelen - 1);
}

void GLEPcode::addDoubleExpression(double val) {
	addInt(PCODE_EXPR);
	int savelen = size();
	addInt(0);
	addDouble(val);
	setInt(savelen, size() - savelen - 1);
}

void GLEPcode::addStringExpression(const char* val) {
	addInt(PCODE_EXPR);
	int savelen = size();
	addInt(0);
	addStringChar(val);
	setInt(savelen, size() - savelen - 1);
}

void GLEPcode::addDouble(double val) {
	union { double d ; int l[2]; short s[4]; } both;
	both.d = val;
	addInt(PCODE_DOUBLE);
	addInt(both.l[0]);
	addInt(both.l[1]);
}

void GLEPcode::addFunction(int idx) {
	addInt(idx);
}

void GLEPcode::addVar(int var) {
	addInt(PCODE_VAR);
	addInt(var);
}

void GLEPcode::addStrVar(int var) {
	addInt(PCODE_STRVAR);
	addInt(var);
}

void GLEPcode::addString(const string& str) {
	addInt(PCODE_STRING);
	addStringNoID(str);
}

void GLEPcode::addStringNoID(const string& str) {
	int slen = str.length() + 1;
	slen = ((slen + 3) & 0xfffc) / 4;
	int pos = size();
	for (int i = 0; i < slen; i++) {
		addInt(0);
	}
	char* str_target = (char*)&(*this)[pos];
	strcpy(str_target, str.c_str());
}

void GLEPcode::addStringChar(const char* str) {
	addInt(PCODE_STRING);
	addStringNoIDChar(str);
}

void GLEPcode::addStringNoIDChar(const char* str) {
	int slen = strlen(str) + 1;
	slen = ((slen + 3) & 0xfffc) / 4;
	int pos = size();
	for (int i = 0; i < slen; i++) {
		addInt(0);
	}
	char* str_target = (char*)&(*this)[pos];
	strcpy(str_target, str);
}

void GLEPcode::show() {
	show(0);
}

void GLEPcode::show(int start) {
	cout << "PCode:" << endl;
	union { double d ; int l[2]; short s[4]; } both;
	int size = getInt(start);
	int pos = start+1;
	while (pos <= start+size) {
		int varid = 0;
		int stpos = pos;
		int opcode = getInt(pos++);
		switch (opcode) {
			case PCODE_VAR:
				varid = getInt(pos++);
				cout << "VAR " << varid << " (" << stpos << ")" << endl;
				break;
			case PCODE_DOUBLE:
				both.l[0] = getInt(pos++);
				both.l[1] = getInt(pos++);
				cout << "DOUBLE " << both.d << endl;
				break;
			default:
				cout << "PCODE " << opcode << " (" << stpos << ")" << endl;

		}
	}
}

GLEPcodeIndexed::GLEPcodeIndexed(GLEPcodeList* list) : GLEPcode(list) {
}

GLEFunctionParserPcode::GLEFunctionParserPcode() : m_Pcode(&m_PcodeList) {
}

GLEFunctionParserPcode::~GLEFunctionParserPcode() {
}

void GLEFunctionParserPcode::polish(const char* fct, StringIntHash* vars) throw(ParserError) {
	GLEPolish* polish = get_global_polish();
	if (polish != NULL) {
		int rtype = 1;
		polish->setExprVars(vars);
		polish->polish(fct, m_Pcode, &rtype);
		polish->setExprVars(NULL);
	}
}

void GLEFunctionParserPcode::polishPos(const char* fct, int pos, StringIntHash* vars) throw(ParserError) {
	GLEPolish* polish = get_global_polish();
	if (polish != NULL) {
		try {
			int rtype = 1;
			polish->setExprVars(vars);
			polish->internalPolish(fct, m_Pcode, &rtype);
			polish->setExprVars(NULL);
		} catch (ParserError& err) {
			err.incColumn(pos-1);
			throw err;
		}
	}
}

void GLEFunctionParserPcode::polishX() throw(ParserError) {
	polish("x", NULL);
}

double GLEFunctionParserPcode::evalDouble() {
	double value;
	eval_pcode(m_Pcode, &value);
	return value;
}

bool GLEFunctionParserPcode::evalBool() {
	int cp = 0;
	GLERC<GLEArrayImpl> stk(new GLEArrayImpl());
	return ::evalBool(stk.get(), m_Pcode.getPcodeList(), (int*)&m_Pcode[0], &cp);
}
