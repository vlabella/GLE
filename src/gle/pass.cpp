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

#define GRAPHDEF extern

#include "all.h"
#include "cutils.h"
#include "mem_limits.h"
#include "token.h"
#include "core.h"
#include "glearray.h"
#include "graph.h"
#include "polish.h"
#include "pass.h"
#include "var.h"
#include "sub.h"
#include "op_def.h"
#include "gprint.h"
#include "keyword.h"
#include "gle-block.h"
#include "key.h"
#include "surface/gsurface.h"
#include "run.h"

GLEParser* g_parser;

extern GLESubMap g_Subroutines;

void get_cap(TOKENS tk,int *ntok,int *curtok,int *pcode,int *plen);
void get_join(TOKENS tk,int *ntok,int *curtok,int *pcode,int *plen);
void g_marker_def(char *s1, char *s2);
void font_load(void);
bool execute_graph(GLESourceLine& sline, bool isCommandCheck);

extern int this_line;
extern int gle_debug;

#define tok(n) tk[n]

struct sub_st{
	char name[40];
	int typ;
	int np;
	int ptyp[20];
	char *pname[20];
};
struct sub_st *psub;

#define skip_space

char *mark_name[30];
char *mrk_fname[61];
char *mrk_name[61];
char *mark_sub[30];
int mark_subp[30];
int nmark = 0;
int nmrk = 0;

/* pos=   Offset to find the data			*/
/* idx=   For switches, which can only have one value. 	*/
/* The pos is the order the items will be placed in the pcode */
/*---------------------------------------------------------------------------*/
/* Input is gle command (tokenized) , output is pcode */
static int cur_mode = 0;  /* 0 --> normal, 1 or greater ---> external begin...end */

void get_key_info(OPKEY lkey, int* count, int* width) {
	*width = 0; *count = 0;
	for (int i = 0; lkey[i].typ != typ_end; i++) {
		int p = lkey[i].pos;
		if (p > *width) *width = p;
		(*count)++;
	}
}

void GLEParserInitTokenizer(Tokenizer* tokens) {
	TokenizerLanguage* lang = tokens->get_language();
	lang->setLineCommentTokens("!");
	lang->setSpaceTokens(" \t\r\n");
	lang->enableCComment();
	lang->setSingleCharTokens(",;=@()[]{}");
}

GLESubCallAdditParam::GLESubCallAdditParam() {
}

GLESubCallAdditParam::~GLESubCallAdditParam() {
}

int GLESubCallAdditParam::isAdditionalParam(const string& str) {
	return str == "NAME" ? 1 : -1;
}

void GLESubCallAdditParam::setAdditionalParam(int idx, const string& val, int pos) {
	m_Val = val;
	m_Pos = pos;
}

GLESubCallInfo::GLESubCallInfo(GLESub* sub) : m_ParamVal(sub->getNbParam()), m_ParamPos(sub->getNbParam(), -1) {
	m_Sub = sub;
	m_Addit = NULL;
}

GLESubCallInfo::~GLESubCallInfo() {
}

void GLESubCallInfo::setParam(int i, const string& val, int pos) {
	m_ParamVal[i] = val;
	m_ParamPos[i] = pos;
}

GLEParser::GLEParser(GLEScript* script, GLEPolish* polish) : m_lang(), m_tokens(&m_lang, false) {
	m_Script = script;
	m_polish = polish;
	m_auto_endif = false;
	m_CrSub = NULL;
	m_insub = false;
	// difference between m_CrSub and m_insub:
	// m_CrSub still points to last sub also after "end sub"

	m_blockTypes = new GLEBlocks();
	m_blockTypes->addBlock(GLE_OPBEGIN_GRAPH, new GLEGraphBlockBase());
	m_blockTypes->addBlock(GLE_OPBEGIN_KEY, new GLEKeyBlockBase());
	m_blockTypes->addBlock(GLE_OPBEGIN_SURF, new GLESurfaceBlockBase());
}

GLEParser::~GLEParser() {
	delete m_blockTypes;
}

GLEBlocks* GLEParser::getBlockTypes() {
	return m_blockTypes;
}

void GLEParser::get_block_type(int type, string& result) {
	char block_type_str[20];
	sprintf(block_type_str, "%d", type);
	const char* block_type = block_type_str;
	switch (type) {
		case 1: /* path */
			block_type = "path"; break;
		case 2: /* box */
			block_type = "box"; break;
		case 3: /* scale */
			block_type = "scale"; break;
		case 4: /* rotate */
			block_type = "rotate"; break;
		case 5: /* translate */
			block_type = "translate"; break;
		case 6: /* if */
			block_type = "if"; break;
		case 7: /* sub */
			block_type = "sub"; break;
		case 8: /* name */
			block_type = "name"; break;
		case 9: /* text */
			block_type = "text"; break;
		case 10: /* graph */
			block_type = "graph"; break;
		case 11: /* xaxis */
			block_type = "xaxis"; break;
		case 12: /* yaxis */
			block_type = "yaxis"; break;
		case 13: /* x2axis */
			block_type = "x2axis"; break;
		case 14: /* y2axis */
			block_type = "y2axis"; break;
		case 15: /* curve */
			block_type = "curve"; break;
		case 16: /* KEY */
			block_type = "key"; break;
		case 17: /* origin */
			block_type = "origin"; break;
		case 18: /* table */
			block_type = "table"; break;
		case 19: /* clip */
			block_type = "clip"; break;
		case 20: /* until */
			block_type = "until"; break;
		case 21: /* shear */
			block_type = "shear"; break;
		case 22: /* config */
			block_type = "config"; break;
		case 23: /* tex preamble */
			block_type = "tex_preamble"; break;
		case 24: /* surface */
			block_type = "surface"; break;
		case 25: /* letz */
			block_type = "letz"; break;
		case 26: /* fitz */
			block_type = "fitz"; break;
		case 27: /* fit */
			block_type = "fit"; break;
		case 28: /* contour */
			block_type = "contour"; break;
		case 29: /* tex */
			block_type = "tex"; break;
		case OP_BEGIN_OBJECT:
			block_type = "object"; break;
	}
	result = block_type;
}

void GLEParser::checkmode() {
	/* Check for text mode block */
	if (cur_mode != 0) {
		string block_type;
		get_block_type(cur_mode, block_type);
		g_throw_parser_error("end of file while in block type '", block_type.c_str(), "'");
	}
	cur_mode = 0;
	/* Check for other type of block */
	GLESourceBlock* block = last_block();
	if (block != NULL) {
		stringstream err;
		err << "end of file while in block type '" << block->getName() << "'";
		err << " starting on line " << block->getFirstLine();
		g_throw_parser_error(err.str());
	}
}

void GLEParser::initTokenizer() {
	TokenizerLanguage* lang = getTokens()->get_language();
	GLEParserInitTokenizer(getTokens());
		// ; -> allows more commands on single line
		// = -> for variable assignments
		// @ -> for user defined subroutine calls
		// (,) -> for calls of the form proc(arg1,...,argn)
		// never add "." to this - e.g., required for "compatibility" command
	TokenizerLanguageMultiLevel* multi = new TokenizerLanguageMultiLevel();
	multi->setOpenClose('(',')');
	multi->setOpenClose('[',']');
	multi->setOpenClose('{','}');
	multi->setEndToken(' ');
	multi->setEndToken(';');
	multi->setEndToken(',');
	multi->setEndToken(')');
	multi->setEndToken(']');
	lang->setMulti(multi);
}

void GLEParser::setAllowSpace(bool allow) {
	TokenizerLanguageMultiLevel* multi = getTokens()->get_language()->getMulti();
	if (allow) multi->resetEndToken(' ');
	else multi->setEndToken(' ');
}

void GLEParser::checkValidName(const string& name, const char* type, int pos) {
	if (name.length() <= 0) {
		throw getTokens()->error(pos, string("zero length ")+type+" name");
	}
	if (name[0] >= '0' && name[0] <= '9') {
		throw getTokens()->error(pos, string(type)+" name should not start with a digit");
	}
	for (string::size_type i = 0; i < name.length(); i++) {
		char ch = name[i];
		if (!((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') ||
		      (ch >= '0' && ch <= '9') || (ch == '$') || (ch == '_'))) {
			throw getTokens()->error(pos+i, string("invalid character '")+ch+"' in "+type+" name");
		}
	}
}

double GLEParser::evalTokenToDouble() {
	double x = 0.0;
	Tokenizer* tokens = getTokens();
	string& expr = tokens->next_multilevel_token();
	int pos = tokens->token_pos_col();
	try {
		m_polish->internalEval(expr.c_str(), &x);
	} catch (ParserError& err) {
		err.incColumn(pos-1);
		throw err;
	}
	return x;
}

void GLEParser::evalTokenToString(string* str) {
	Tokenizer* tokens = getTokens();
	string& expr = tokens->next_multilevel_token();
	int pos = tokens->token_pos_col();
	try {
		m_polish->internalEvalString(expr.c_str(), str);
	} catch (ParserError& err) {
		err.incColumn(pos-1);
		throw err;
	}
}

void GLEParser::evalTokenToFileName(string* str) {
	Tokenizer* tokens = getTokens();
   const string& token = tokens->next_continuous_string_excluding("\"$+");
   if (token != "") {
      *str = token;
   } else {
      evalTokenToString(str);
   }
}

void GLEParser::polish(GLEPcode& pcode, int *rtype) {
	Tokenizer* tokens = getTokens();
	string& expr = tokens->next_multilevel_token();
	int pos = tokens->token_pos_col();
	// cout << "pos = " << pos << endl;
	try {
		// cout << "Polish: '" << expr << "'" << endl;
		m_polish->internalPolish(expr.c_str(), pcode, rtype);
	} catch (ParserError& err) {
		err.incColumn(pos-1);
		throw err;
	}
}

void GLEParser::polish_eol(GLEPcode& pcode, int *rtype) {
	setAllowSpace(true);
	polish(pcode, rtype);
	setAllowSpace(false);
}

void GLEParser::polish(const char* str, GLEPcode& pcode, int *rtype) {
	m_polish->polish(str, pcode, rtype);
}

void GLEParser::polish_pos(const string& arg, int pos, GLEPcode& pcode, int* rtype) {
	try {
		m_polish->internalPolish(arg.c_str(), pcode, rtype);
	} catch (ParserError& err) {
		err.incColumn(pos-1);
		throw err;
	}
}

void GLEParser::get_xy(GLEPcode& pcode) {
	int vtype = 1;
	polish(pcode, &vtype);
	vtype = 1;
	polish(pcode, &vtype);
}

void GLEParser::get_exp(GLEPcode& pcode) {
	int vtype = 1;
	polish(pcode, &vtype);
}

void GLEParser::get_exp_eol(GLEPcode& pcode) {
	int vtype = 1;
	polish_eol(pcode, &vtype);
}

void GLEParser::get_strexp(GLEPcode& pcode) {
	int vtype = 2;
	polish(pcode, &vtype);
}

int GLEParser::get_anyexp(GLEPcode& pcode) {
	int vtype = 0;
	polish(pcode, &vtype);
	return vtype;
}

void GLEParser::get_if(GLEPcode& pcode) {
	Tokenizer* tokens = getTokens();
	string expr = tokens->next_multilevel_token();
	int pos = tokens->token_pos_col();
	/* Support spaces between in if expression */
	while (true) {
		string& token = tokens->next_multilevel_token();
		if (str_i_equals(token.c_str(), "THEN")) {
			break;
		} else if (token == "") {
			throw error("'THEN' expected after if condition");
		}
		expr += " ";
		expr += token;
	}
	try {
		int rtype = 1;
		// cout << "Polish: '" << expr << "'" << endl;
		m_polish->internalPolish(expr.c_str(), pcode, &rtype);
	} catch (ParserError& err) {
		err.incColumn(pos-1);
		throw err;
	}
}

void GLEParser::parse_if(int srclin, GLEPcode& pcode) {
	get_if(pcode);
	GLESourceBlock* block = add_block(GLE_SRCBLK_MAGIC+GLE_OPBEGIN_IF, srclin);
	block->setOffset2(pcode.size());
	pcode.addInt(0);
	pcode.addInt(0);
}

void GLEParser::get_subroutine_call(GLEPcode& pcode, string* name, int poscol) {
	string fct_name;
	if (name != NULL) {
		fct_name = *name;
	} else {
		fct_name = m_tokens.next_token();
		str_to_uppercase(fct_name);
		poscol = m_tokens.token_pos_col();
	}
	GLESub* sub = sub_find((char*)fct_name.c_str());
	if (sub == NULL) {
		throw error(poscol, "function '"+fct_name+"' not defined");
	}
	GLESubCallInfo info(sub);
	pass_subroutine_call(&info, poscol);
	gen_subroutine_call_code(&info, pcode);
}

void GLEParser::pass_subroutine_call(GLESubCallInfo* info, int poscol) {
	GLESub* sub = info->getSub();
	int np = sub->getNbParam();
	string uc_token;
	bool mustname = false;
	int argcnt = 0, maxargpos = -1;
	// cout << "reading args for: '" << fct_name << "'" << endl;
	bool has_parenthesis = false;
	if (m_tokens.is_next_token("(")) {
		if (m_tokens.has_space_before()) {
			// in this case, "(" is part of expression in first argument
			m_tokens.pushback_token();
		} else {
			has_parenthesis = true;
		}
	}
	while (has_parenthesis || not_at_end_command()) {
		/* check if argument name is given */
		//printf("foo\n");
		int argno = -1;
		int addidx = -1;
		string& token = m_tokens.next_multilevel_token();
		if (token == "") {
			/* empty - can happen if, e.g., "," is given instead of token */
			break;
		}
		str_to_uppercase(token, uc_token);
		argno = sub->findParameter(uc_token);
		if (info->getAdditParam() != NULL) {
			addidx = info->getAdditParam()->isAdditionalParam(uc_token);
		}
		if (argno != -1 || addidx != -1) {
			/* don't assume it is a name if it is also a local variable */
			int var_idx, var_type;
			var_find((char*)uc_token.c_str(), &var_idx, &var_type);
			if (var_idx != -1) {
				argno = -1;
				addidx = -1;
			}
		}
		if (argno == -1 && addidx == -1) {
			if (mustname) {
				/* all arguments after the first optional one must be named */
				stringstream err;
				err << "name expected before optional argument, such as: ";
				sub->listArgNames(err);
				throw error(err.str());
			}
			/* no argument name given, just assume subsequent argument */
			argno = argcnt;
			argcnt++;
		} else {
			/* first named argument found */
			/* from now on, all must be named */
			mustname = true;
			/* previous token was just the name, get value */
			token = m_tokens.next_multilevel_token();
		}
		if (argno > maxargpos) {
			/* compute maximum for 'too many arguments error' */
			maxargpos = argno;
		}
		if (addidx != -1) {
			info->getAdditParam()->setAdditionalParam(addidx, token, m_tokens.token_pos_col());
		}
		if (argno != -1 && argno < np) {
			if (info->getParamPos(argno) != -1) {
				/* make sure not to give the same argument twice */
				stringstream err;
				err << "two values given for argument '" << sub->getParamNameShort(argno);
				err << "' of '" << sub->getName() << "'";
				throw error(err.str());
			}
			info->setParam(argno, token, m_tokens.token_pos_col());
		}
		if (has_parenthesis) {
			int token = m_tokens.ensure_next_token_in(",)");
			if (token == ')') break;
		}
	}
	/* error if too many arguments are given */
	if (maxargpos >= np) {
		stringstream err;
		err << "too many arguments in call to '" << sub->getName() << "': ";
		err << (maxargpos+1) << " > " << np;
		throw error(poscol, err.str());
	}
	/* fill in default values */
	bool has_all = true;
	for (int i = 0; i < np; i++) {
		if (info->getParamPos(i) == -1) {
			const string& value = sub->getDefault(i);
			if (value != "") {
				info->setParam(i, value, -2); /* 2 indicates default */
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
		for (int i = 0; i < np; i++) {
			if (info->getParamPos(i) == -1) {
				if (count != 0) err << ", ";
				err << sub->getParamNameShort(i);
				count++;
			}
		}
		throw error(poscol, err.str());
	}
}

void GLEParser::gen_subroutine_call_polish_arg(GLESubCallInfo* info, int i, GLEPcode& pcode) {
	GLESub* sub = info->getSub();
	try {
		int vtype = sub->getParamType(i);
		m_polish->internalPolish(info->getParamVal(i).c_str(), pcode, &vtype);
	} catch (ParserError& err) {
		if (info->getParamPos(i) != -2) {
			/* in place given argument */
			err.incColumn(info->getParamPos(i)-1);
		} else {
			/* default argument */
			err.setParserString(info->getParamVal(i));
		}
		throw err;
	}
}

void GLEParser::evaluate_subroutine_arguments(GLESubCallInfo* info, GLEArrayImpl* arguments) {
	GLESub* sub = info->getSub();
	int np = sub->getNbParam();
	arguments->resize(np);
	GLEPcodeList pcodeList;
	GLERC<GLEArrayImpl> stk(new GLEArrayImpl());
	for (int i = 0; i < np; i++) {
		GLEPcode pcode(&pcodeList);
		gen_subroutine_call_polish_arg(info, i, pcode);
		int cp = 0;
		arguments->set(i, evalGeneric(stk.get(), &pcodeList, (int*)&pcode[0], &cp));
	}
}

void GLEParser::gen_subroutine_call_code(GLESubCallInfo* info, GLEPcode& pcode) {
	/* pass all arguments */
	GLESub* sub = info->getSub();
	int np = sub->getNbParam();
	pcode.addInt(PCODE_EXPR);    /* Expression follows */
	int savelen = pcode.size();  /* Used to set actual length at end */
	pcode.addInt(0);             /* Length of expression */
	for (int i = 0; i < np; i++) {
		gen_subroutine_call_polish_arg(info, i, pcode);
	}
	pcode.addFunction(sub->getIndex()+LOCAL_START_INDEX);
	pcode.setInt(savelen, pcode.size() - savelen - 1);
}

GLESub* GLEParser::get_subroutine_declaration(GLEPcode& pcode) {
	string uc_token;
	string& token = m_tokens.next_token();
	str_to_uppercase(token, uc_token);
	GLESub* sub = sub_find(uc_token);
	if (sub != NULL) {
		// Re-declaration: check if number of arguments and argument names are ok
		vector<int> poss;
		vector<string> args;
		while (not_at_end_command()) {
			token = m_tokens.next_token();
			str_to_uppercase(token);
			args.push_back(token);
			poss.push_back(m_tokens.token_column());
		}
		if ((int)args.size() != sub->getNbParam()) {
			stringstream err;
			err << "subroutine '" << uc_token << "' number of arguments: ";
			err << args.size() << " <> " << sub->getNbParam();
			if (sub->getStart() != -1) {
				// May be declared from previous run - if QGLE commits changes back to GLE script
				err << " as declared at: ";
				getSource()->sourceLineFileAndNumber(sub->getStart()-1, err);
			}
			throw error(err.str());
		}
		for (int i = 0; i < sub->getNbParam(); i++) {
			if (!str_i_equals(args[i], sub->getParamName(i))) {
				stringstream err;
				err << "subroutine '" << uc_token << "' argument " << (i+1) << ": '";
				err << args[i] << "' <> '" << sub->getParamName(i) << "'";
				if (sub->getStart() != -1) {
					// May be declared from previous run - if QGLE commits changes back to GLE script
					err << " as declared at: ";
					getSource()->sourceLineFileAndNumber(sub->getStart()-1, err);
				}
				throw error(poss[i], err.str());
			}
		}
		var_set_local_map(sub->getLocalVars());
	} else {
		sub = getSubroutines()->add(uc_token);
		var_set_local_map(sub->getLocalVars());
		for (int np = 0; not_at_end_command(); np++) {
			token = m_tokens.next_token();
			str_to_uppercase(token, uc_token);
			sub_param(sub, uc_token);
			if (!valid_var((char*)uc_token.c_str())) {
				throw error("invalid subroutine parameter");
			}
		}
	}
	return sub;
}

void GLEParser::get_subroutine_default_param(GLESub* sub) {
	if (sub == NULL) {
		return;
	}
	/* Find parameter */
	string uc_token;
	string& token = m_tokens.next_token();
	str_to_uppercase(token, uc_token);
	int idx = sub->findParameter(uc_token);
	if (idx == -1) {
		stringstream err;
		err << "subroutine '" << sub->getName() << "' has no parameter named '" << token << "'";
		throw m_tokens.error(err.str());
	}
	token = m_tokens.next_multilevel_token();
	sub->setDefault(idx, token);
}

int GLEParser::get_optional(OPKEY lkey, GLEPcode& pcode) {
	// find the largest width
	int count, width;
	get_key_info(lkey, &count, &width);
	// the location in the pcode tells what option it is, must remember
	// zero all the optional parameters.
	// puts zero in for each option wheather it is there or not
	int plen = pcode.size();
	for (int i = 0; i < width+1; i++) {
		pcode.addInt(0);
	}
	int ret = -1;
	while (m_tokens.has_more_tokens()) {
		string& token = m_tokens.next_token();
		if (token == ";") {
			m_tokens.pushback_token();
			return ret;
		}
		bool found = false;
		for (int i = 0; i < count && !found; i++) {
			if (str_i_equals(token.c_str(), lkey[i].name)) {
			 	ret = get_one_option(&lkey[i], pcode, plen);
				found = true;
			}
		}
		if (!found) {
			throw create_option_error(lkey, count, token);
		}
	}
	return ret;
}

ParserError GLEParser::create_option_error(OPKEY lkey, int count, const string& token) {
	stringstream strm;
	if (count == 1) {
		strm << "found '" << token << "', but expecting '" << lkey[0].name << "'";
	} else {
		strm << "found '" << token << "', but expecting one of:";
		for (int i = 0; i < count; i++) {
			if (i % 5 == 0) {
				strm << endl << "       ";
			} else {
				strm << " ";
			}
			strm << lkey[i].name;
			if (i < count-1) strm << ",";
		}
	}
	return m_tokens.error(strm.str());
}

void GLEParser::duplicate_error(GLEPcode& pcode, int pos) {
	if (pcode.getInt(pos) != 0) throw error("duplicate or illegal combination of qualifiers");
}

int GLEParser::get_one_option(op_key* lkey, GLEPcode& pcode, int plen) {
// switches 	int 	placed in directly, 1 present, 0 not present
// expressions 	LONG* 	pointed to, 0 if not present.
// color/fill	LONG* 	Pointer to exp 0 if not present.
// marker	LONG*	Pointer to exp 0 if not present.
// lstyle 	LONG*	Pointer to exp 0 if not present.
// font 	int* 	Pointer to string expression.
// justify 	int
	int pos = plen + lkey->pos - 1;
	duplicate_error(pcode, pos);
	switch (lkey->typ) {
	case typ_val:
		pcode.setInt(pos, pcode.size() - pos);
		get_exp(pcode);
	 	break;
	case typ_val2:
		pcode.setInt(pos, pcode.size() - pos);
		get_exp(pcode);
		pos++;
		duplicate_error(pcode, pos);
		pcode.setInt(pos, pcode.size() - pos);
		get_exp(pcode);
		break;
	case typ_val4:
		pcode.setInt(pos, pcode.size() - pos);
		get_exp(pcode);
		get_exp(pcode);
		get_exp(pcode);
		get_exp(pcode);
		break;
	case typ_str:
		pcode.setInt(pos, pcode.size() - pos);
		get_strexp(pcode);
	 	break;
	case typ_switch:
		pcode.setInt(pos, lkey->idx);
		return lkey->idx;
		break;
	case typ_color:
	case typ_fill:
		pcode.setInt(pos, pcode.size() - pos);
		get_fill(pcode);
		break;
	case typ_marker:
		pcode.setInt(pos, pcode.size() - pos);
		get_marker(pcode);
		break;
	case typ_lstyle:
		pcode.setInt(pos, pcode.size() - pos);
		get_exp(pcode);
		break;
	case typ_justify:
		pcode.setInt(pos, get_first(op_justify));
		break;
	case typ_arrow:
		pcode.setInt(pos, get_first(op_arrow));
		break;
	default :
		gprint("*** error non existent type ***");
		break;
	}
	return -1;
}

int GLEParser::get_first(OPKEY lkey) {
	return get_first(m_tokens.next_token(), lkey);
}

int GLEParser::get_first(const string& token, OPKEY lkey) {
	int count, width;
	get_key_info(lkey, &count, &width);
	for (int i = 0; i < count; i++) {
		if (str_i_equals(token.c_str(), lkey[i].name)) {
			return lkey[i].idx;
		}
	}
	throw create_option_error(lkey, count, token);
}

bool GLEParser::try_get_token(const char* token) {
	string& my_token = m_tokens.try_next_token();
	if (str_i_equals(token, my_token.c_str())) {
		return true;
	} else if (my_token != "") {
		m_tokens.pushback_token();
	}
	return false;
}

void GLEParser::get_token(const char* token) {
	string& my_token = m_tokens.next_token();
	if (!str_i_equals(token, my_token.c_str())) {
		throw error(string("expected '")+token+"', but found '"+my_token+"' instead");
	}
}

void GLEParser::get_fill(GLEPcode& pcode) {
	get_color(pcode);
}

bool pass_color_hash_value(const string& color, int* result, IThrowsError* error) {
	if (color.length() > 1 && color[0] == '#') {
		if (color.length() != 7) {
			throw error->throwError("illegal color specification '", color.c_str(), "'");
		}
		colortyp c;
		int err = g_hash_string_to_color(color, &c);
		if (err != 0) {
			int pos = error->getErrorPosition() + err;
			throw error->throwError(pos, string("illegal color specification '") + color + "'");
		}
		*result = c.l;
      return true;
   } else {
      return false;
   }
}

GLERC<GLEColor> pass_color_list_or_fill(const string& color, IThrowsError* error) {
	GLERC<GLEColor> result;
	string uc_color;
	str_to_uppercase(color, uc_color);
	GLEColor* gleColor = GLEGetColorList()->get(uc_color);
	if (gleColor != NULL) {
		result = gleColor->clone();
	} else {
		int fillDescr = 0;
		if (gt_firstval_err(op_fill_typ, uc_color.c_str(), &fillDescr)) {
			result = new GLEColor();
			if (unsigned(fillDescr) == GLE_FILL_CLEAR) {
				result->setTransparent(true);
			} else {
				result->setFill(new GLEPatternFill(fillDescr));
			}
		} else {
			char *endp;
			const char* colorCStr = color.c_str();
			double dvalue = strtod(colorCStr, &endp);
			if (colorCStr != endp && *endp == 0) {
				result = new GLEColor(dvalue);
			} else if (error != 0) {
				throw error->throwError("found '", color.c_str(), "', but expecting color or fill specification");
			}
		}
	}
	return result;
}

GLERC<GLEColor> memory_cell_to_color(GLEPolish* polish, GLEArrayImpl* stk, GLEMemoryCell* cell, IThrowsError* throwsError, int depth) {
	if (depth >= 5) {
		throwsError->throwError("maximum depth exceeded while parsing color expression");
	}
	GLERC<GLEColor> color(new GLEColor());
	switch (gle_memory_cell_type(cell)) {
		case GLEObjectTypeDouble:
			color->setGray(cell->Entry.DoubleVal);
			break;
		case GLEObjectTypeString:
			{
				int result = 0;
				std::string token(((GLEString*)cell->Entry.ObjectVal)->toUTF8());
				if (token.empty()) {
					throwsError->throwError("expecting color name, but found empty string");
				} else if (pass_color_hash_value(token, &result, throwsError)) {
					color->setHexValue(result);
				} else if (((GLEString*)cell->Entry.ObjectVal)->containsI('(')) {
					color = memory_cell_to_color(polish, stk, polish->evalGeneric(stk, token.c_str()), throwsError, depth + 1);
				} else {
					color = pass_color_list_or_fill(token, throwsError);
				}
				break;
			}
		default:
			gle_memory_cell_check(cell, GLEObjectTypeColor);
			color = (GLEColor*)cell->Entry.ObjectVal;
			break;
	}
	return color;
}

GLERC<GLEColor> pass_color_var(const std::string& token) {
	GLERC<GLEColor> color(new GLEColor());
	int result = 0;
	if (token.empty()) {
		g_throw_parser_error("expecting color name, but found empty string");
	} else if (pass_color_hash_value(token, &result, g_get_throws_error())) {
		color->setHexValue(result);
	} else {
		GLEPolish* polish = get_global_polish();
		GLERC<GLEArrayImpl> stk(new GLEArrayImpl());
		color = memory_cell_to_color(polish, stk.get(), polish->evalGeneric(stk.get(), token.c_str()), g_get_throws_error(), 0);
	}
	return color;
}

void GLEParser::get_color(GLEPcode& pcode) {
	int result = 0;
	GLERC<GLEColor> color;
	string& token = m_tokens.next_token();
	if (pass_color_hash_value(token, &result, &m_tokens)) {
		color = new GLEColor();
		color->setHexValue(result);
	} else {
		color = pass_color_list_or_fill(token, 0);
	}
	if (color.isNull()) {
		m_tokens.pushback_token();
		get_exp(pcode);
	} else {
		pcode.addColor(color.get());
	}
}

int get_marker_string(const string& marker, IThrowsError* error) {
	/* if 0, maybe its a user defined marker, ie a subroutine */
	/* Use -ve to signify subroutine instead of normal marker */
	int mark_idx = 0;
	for (int i = 0; i < nmark; i++) {
		if (str_i_equals(mark_name[i], marker.c_str())) {
			mark_idx = -(++i);
			break;
		}
	}
	if (mark_idx == 0)  {
		for (int i = nmrk-1; i >= 0; i--) {
			if (str_i_equals(mrk_name[i], marker.c_str())) {
				mark_idx = ++i;
				break;
			}
		}
	}
	if (mark_idx == 0) {
      throw error->throwError("invalid marker name '", marker.c_str(), "'");
   }
   return mark_idx;
}

void GLEParser::get_marker(GLEPcode& pcode) {
	int vtype = 1;
	string& token = m_tokens.next_token();
	if (token == "(" || is_float(token)) {
		string expr = string("CVTINT(")+token+")";
		polish(expr.c_str(), pcode, &vtype);
	} else if (str_starts_with(token, "\"") || str_var_valid_name(token)) {
		string expr = string("CVTMARKER(")+token+")";
		polish(expr.c_str(), pcode, &vtype);
	} else {
		pcode.addInt(8);
		pcode.addInt(get_marker_string(token, &m_tokens));
	}
}

int pass_marker(char *name) {
	string marker;
	polish_eval_string(name, &marker);
   return get_marker_string(marker, g_get_throws_error());   
}

void GLEParser::define_marker_1(GLEPcode& pcode) {
	string name;
	Tokenizer* tokens = getTokens();
	str_to_uppercase(tokens->next_token(), name);
	string font = tokens->next_token();
	int ccc = tokens->next_integer();
	double sz = tokens->next_double();
	double dx = tokens->next_double();
	double dy = tokens->next_double();
	g_defmarker((char*)name.c_str(), (char*)font.c_str(), ccc, dx, dy, sz, true);
}

void GLEParser::define_marker_2(GLEPcode& pcode) {
	string name, sub;
	Tokenizer* tokens = getTokens();
	tokens->ensure_next_token_i("marker");
	str_to_uppercase(tokens->next_token(), name);
	str_to_uppercase(tokens->next_token(), sub);
	g_marker_def((char*)name.c_str(), (char*)sub.c_str());
}


/*

    Data types:
    
    - fill
    - color
    - marker
    - font
    - justify
    - join
    - cap

*/

int get_font_index(const string& token, IThrowsError* error) {
	if (get_nb_fonts() == 0) font_load();
	int count = get_nb_fonts();
	// font zero is a dummy!
	for (int i = 1; i < count; i++) {
		const char* name = get_font_name(i);
		if (str_i_equals(name, token.c_str())) {
			return i;
		}
	}
	stringstream strm;
	strm << "invalid font name {" << token << "}, expecting one of:";
	int idx = 0;
	for (int i = 1; i < count; i++) {
		if (idx % 5 == 0) {
			strm << endl << "       ";
		} else {
			strm << " ";
		}
		if (get_font_name(i) != NULL) {
			strm << get_font_name(i);
			bool has_more = false;
			for (int j = i+1; j < count; j++) {
				if (get_font_name(j) != NULL) {
					has_more = true;
					break;
				}
			}
			if (has_more) strm << ",";
			idx++;
		}
	}
	throw error->throwError(strm.str());
}

void GLEParser::get_font(GLEPcode& pcode) {
	string& token = m_tokens.next_token();
	if (str_starts_with(token, "\"") || str_var_valid_name(token)) {
   	int etype = 1;
		string parse("CVTFONT(" + token + ")");
		polish(parse.c_str(), pcode, &etype);
		return;
	}
	pcode.addInt(8);
    pcode.addInt(get_font_index(token, &m_tokens));
}

int pass_font(const std::string& token) {
	if (str_starts_with(token, "\"") || str_var_valid_name(token)) {
      int result = 0;
      double xx = 0.0;
		string parse("CVTFONT(" + token + ")");
		polish_eval((char*)parse.c_str(), &xx);
		memcpy(&result, &xx, sizeof(int));
		return result;
	} else {
      return get_font_index(token, g_get_throws_error());
	}
}

void GLEParser::get_papersize(GLEPcode& pcode) {
	const string& token = m_tokens.next_token();
	int type = g_papersize_type(token);
	if (type == GLE_PAPER_UNKNOWN) {
		m_tokens.pushback_token();
		pcode.addInt(0);
		get_xy(pcode);
	} else {
		pcode.addInt(1);
		pcode.addInt(type);
	}
}

void GLEParser::get_justify(GLEPcode& pcode) {
	const string& token = m_tokens.next_token();
	if (str_starts_with(token, "\"") || str_var_valid_name(token)) {
		int etype = 1;
		string parse("JUSTIFY(" + token + ")");
		polish(parse.c_str(), pcode, &etype);
		return;
	}
    pcode.addInt(8);
	pcode.addInt(get_first(token, op_justify));
}

int pass_justify(const std::string& token) {
	if (str_starts_with(token, "\"") || str_var_valid_name(token)) {
		int result = 0;
      	double xx = 0.0;
		string parse("JUSTIFY(" + token + ")");
		polish_eval((char*)parse.c_str(), &xx);
		memcpy(&result, &xx, sizeof(int));
		return result;
	}
	return gt_firstval(op_justify, token.c_str());
}

void GLEParser::get_join(GLEPcode& pcode) {
	pcode.addInt(get_first(op_join));
}

void GLEParser::get_cap(GLEPcode& pcode) {
	pcode.addInt(get_first(op_cap));
}

void GLEParser::get_var_add(int *var, int *vtype) {
	string uc_token;
	string& token = m_tokens.next_token();
	str_to_uppercase(token, uc_token);
	var_findadd((char*)uc_token.c_str(), var, vtype);
}

void GLEParser::get_var(GLEPcode& pcode) {
	int var;
	int vtype = 0;
	get_var_add(&var, &vtype);
	pcode.addInt(var);
}

bool GLEParser::not_at_end_command() {
	string& token = m_tokens.try_next_token();
	if (token == "") return false;
	if (token == ";") {
		m_tokens.pushback_token();
		return false;
	}
	m_tokens.pushback_token();
	return true;
}

bool GLEParser::test_not_at_end_command() {
	string& token = m_tokens.try_next_token();
	if (token == "") return false;
	if (token == ";") return false;
	m_tokens.pushback_token();
	return true;
}

GLESub* GLEParser::is_draw_sub(const string& str) {
	string subname;
	string::size_type i = str.find('.');
	if (i != string::npos) {
		subname = str.substr(0, i);
	} else {
		subname = str;
	}
	str_to_uppercase(subname);
	return sub_find((char*)subname.c_str());
}

bool GLEParser::pass_block_specific(GLESourceLine& sourceLine, GLEPcode& pcode) {
	for (int i = m_blocks.size() - 1; i >= 0; i--) {
		GLESourceBlock* block = &m_blocks[i];
		GLEBlockBase* blockType = getBlockTypes()->getBlockIfExists(block->getType() - GLE_SRCBLK_MAGIC);
		if (blockType != 0) {
			if (blockType->checkLine(sourceLine)) {
				int pos_start = pcode.size();
				pcode.addInt(0);
				pcode.addInt(GLE_KW_BLOCK_COMMAND);
				pcode.addInt(block->getType() - GLE_SRCBLK_MAGIC);
				pcode.setInt(pos_start, pcode.size()-pos_start);
				return true;
			}
		}
	}
	return false;
}

void GLEParser::passt(GLESourceLine &SLine, GLEPcode& pcode) {
	resetSpecial();
	static int i,f,vtyp,v,vidx;
	int srclin = SLine.getGlobalLineNo();
	this_line = srclin;
	int fctkey;
	int nbcmd = 0;
	int position;
	GLESourceBlock* block = last_block();
	string first, temp_str;
	if (cur_mode != 0) {
		do_text_mode(SLine, getTokens(), pcode);
		return;
	}
	setAllowSpace(false);
	bool single_cmd = false;
	Tokenizer* tokens = getTokens();
	if (block != 0) {
		if (m_auto_endif) {
			// Allow for "IF test THEN code" blocks	without "END IF"
			if (block->getType() == GLE_SRCBLK_MAGIC+GLE_OPBEGIN_IF) {
				string& token = tokens->try_next_token();
				if (str_i_equals(token, "ELSE")) m_auto_endif = false;
				if (token != "") tokens->pushback_token();
			} else if (block->getType() != GLE_SRCBLK_ELSE) {
				m_auto_endif = false;
			}
			if (m_auto_endif) {
				m_auto_endif = false;
				do_endif(srclin, pcode);
			}
		}
		if (pass_block_specific(SLine, pcode)) {
			return;
		}
	}
	int pos_start = pcode.size();
	while (tokens->has_more_tokens()) {
		int vtype = 0;
		int etype = 1;
		bool allow_extra_tokens = false;
		int pos_endoffs = pcode.size();
		pcode.addInt(0); // save space for end offset
		str_to_uppercase(tokens->next_token(), first);
		int pos_first = tokens->token_pos_col();
		find_mkey((char*)first.c_str(), &fctkey);
		// cout << "first = " << first << " idx = " << fctkey << endl;
		if (fctkey == 0) {
			if (first == "@") {
				pcode.addInt(52);
				get_subroutine_call(pcode);
			} else if (first == "LOCAL") {
				// Declaration of local variable
				if (!has_local_var_map()) {
					throw error("can't define a local variable outside of sub");
				}
				// Get variable name
				str_to_uppercase(tokens->next_token(), temp_str);
				checkValidName(temp_str, "variable", tokens->token_pos_col());
				pcode.addInt(51);
				var_add_local((char*)temp_str.c_str(),&vidx,&vtype);
				pcode.addInt(vidx);
				// Immediately assign a value to it
				if (tokens->is_next_token("=")) {
					polish_eol(pcode, &vtype);
				} else {
					// Or assign the value of zero to it
					if (vtype == 1) pcode.addDoubleExpression(0);
					else pcode.addStringExpression("");
					// And possibly define more local variables in one go
					while (tokens->is_next_token(",")) {
						str_to_uppercase(tokens->next_token(), temp_str);
						checkValidName(temp_str, "variable", tokens->token_pos_col());
						pcode.addInt(51);
						var_add_local((char*)temp_str.c_str(),&vidx,&vtype);
						pcode.addInt(vidx);
						if (vtype == 1) pcode.addDoubleExpression(0);
						else pcode.addStringExpression("");
					}
				}
			} else if (tokens->is_next_token("=")) {
				// Variable assignment
				checkValidName(first, "variable", pos_first);
				pcode.addInt(51);
				var_findadd((char*)first.c_str(),&vidx,&vtype);
				pcode.addInt(vidx);
				polish_eol(pcode, &vtype);
			} else {
				/* call subroutine without @ sign */
				pcode.addInt(52);
				get_subroutine_call(pcode, &first, pos_first);
			}
		} else {
			pcode.addInt(fctkey);
			switch (fctkey) {
			case 65: /* PSCOMMENT */
				temp_str = m_tokens.read_line();
				str_remove_quote(temp_str);
				pcode.addStringNoID(temp_str);
				break;
			case 66: /* BB_TWEAK */
				break;
			  case 1:  /* ALINE */
				get_xy(pcode);
				get_optional(op_line, pcode);
				break;
			  case 2: /* AMOVE */
				get_xy(pcode);
				break;
			  case GLE_KW_ABOUND:
				get_xy(pcode);
				break;
			  case 73: /* ASETPOS */
			  case 81: /* RSETPOS */
				get_xy(pcode);
				break;
			  case 3: /* ARC r a1 a2  */
				get_exp(pcode);
				get_xy(pcode);
				get_optional(op_arc, pcode);
				break;
			  case 4: /* ARCTO x1 y1 x2 y2 r */
				get_xy(pcode);
				get_xy(pcode);
				get_exp(pcode);
				break;
			  case 5: /* BEGIN  "PATH"  "BOX"  "SCALE"  "ROTATE"  "TRANSLATE" "SHEAR" "GRAPH" ... */
				single_cmd = true;
				f = get_first(op_begin);
				pcode.addInt(f);
				/*------------------------------------------*/
				/*       Check if begin variable matches    */
				/*------------------------------------------*/
				switch (f) {
				  case 1: /* path */
					get_optional(op_begin_path, pcode);
					break;
				  case 2: /* box */
					get_optional(op_begin_box, pcode);
					break;
				  case 3: /* scale */
					get_xy(pcode);
					get_optional(op_begin_scale, pcode);
					break;
				  case 21: /* shear */
					get_xy(pcode);
					get_optional(op_begin_scale, pcode);
					break;
				  case 4: /* rotate */
					get_exp(pcode);
					get_optional(op_begin_scale, pcode);
					break;
				  case 5: /* translate */
					get_xy(pcode);
					get_optional(op_begin_scale, pcode);
					break;
				  case 19: /* clip */
				  case 17: /* origin */
					break;
				  case 6: /* if */
				  case 7: /* sub */
					throw error("not a valid begin option");
					break;
				  case 8: /* name joe */
					get_strexp(pcode);
					get_optional(op_begin_name, pcode);
					break;
				  case 9: /* text */
					cur_mode = 9;
					get_optional(op_begin_text, pcode);
					break;
				  case 10: /* graph */
					break;
				  case 11: /* xaxis */
					cur_mode = 11;
					break;
				  case 12: /* yaxis */
					cur_mode = 12;
					break;
				  case 13: /* x2axis */
					cur_mode = 13;
					break;
				  case 14: /* y2axis */
					cur_mode = 14;
					break;
				  case 15: /* curve */
					break;
				  case 18: /* table */
					cur_mode = 18;
					break;
				  case 22: /* config */
					cur_mode = 22;
					get_strexp(pcode);
					break;
				  case 23: /* tex preamble */
					cur_mode = 23;
					break;
				  case 25: /* letz */
					cur_mode = 25;
					break;
				  case 26: /* fitz */
					cur_mode = 26;
					break;
				  case 27: /* fit */
					cur_mode = 27;
					break;
				  case 28: /* contour */
					cur_mode = 28;
					break;
				  case 29: /* tex */
					cur_mode = 29;
					get_optional(op_tex, pcode);
					break;
				  case OP_BEGIN_OBJECT:
					if (isInSub()) {
						m_CrSub = getSubroutines()->add(m_CrSub);
						pcode.addInt(0);
						pcode.addInt(m_CrSub->getIndex());
						get_strexp(pcode); /* name for object */
					} else {
						setInSub(true);
						m_CrSub = get_subroutine_declaration(pcode);
						m_CrSub->setIsObject(true);
						pcode.addInt(1);
						pcode.addInt(m_CrSub->getIndex());
					}
					break;
				  case OP_BEGIN_LENGTH:
					get_var(pcode);
					break;
				}
				/* here should copy source line across for "begin width 3 */
				if (cur_mode>0)	pcode.addInt(0);
				else add_block(GLE_SRCBLK_MAGIC+f, srclin);
				break;
			  case 6: /* BEZIER x1 y1 x2 y2 x3 y3 */
				get_xy(pcode);
				get_xy(pcode);
				get_xy(pcode);
				break;
			  case 7 :	/* box x y  [left | center | right]  [fill xxx] name*/
				get_xy(pcode);
				get_optional(op_box, pcode);
				break;
			  case 52: /* call subroutine */
				get_subroutine_call(pcode);
				break;
			  case 8 :	/* circle rad fill */
				get_exp(pcode);
				get_optional(op_circle, pcode);
				break;
			  case 70 :	/* ellipse major minor fill */
				get_xy(pcode);
				get_optional(op_circle, pcode);
				break;
			  case 71 :	/* elliptical_arc major minor a1 a2 fill */
				get_xy(pcode);
				get_xy(pcode);
				get_optional(op_arc, pcode);
				break;
			  case 72 :	/* elliptical_narc major minor a1 a2 fill */
				get_xy(pcode);
				get_xy(pcode);
				get_optional(op_arc, pcode);
				break;
			  case 9 : /* closepath */
				break;
			  case GLE_KW_COMMENT: /* comment !  or blank line */
				break;
			  case 10 : /* curve sx sy x y x y x y ... ex ey */
				while (not_at_end_command()) {
					pcode.addInt(111);
					get_xy(pcode);
				}
				pcode.addInt(999);
				break;
			  case 60 : /* defmarker xyz rm 33 1 -.4 -.2 */
				define_marker_1(pcode);
				break;
			  case GLE_KW_DEFCOLOR:
				pcode.setLast(GLE_KW_COMMENT);
				temp_str =  m_tokens.next_token();
				str_remove_quote(temp_str);
				{
					GLERC<GLEColor> color(pass_color_var(m_tokens.next_token().c_str()));
					GLEGetColorList()->defineColor(temp_str, color.get());
				}
				break;
			  case GLE_KW_COMPATIBILITY:
				pcode.setLast(GLE_KW_COMMENT);
				temp_str = m_tokens.next_token();
				g_set_compatibility(temp_str);
				if (g_get_compatibility() < GLE_COMPAT_MOST_RECENT) {
					setSpecial(GLE_PARSER_INCLUDE);
					setInclude(string("compatibility.gle"));
				}
				break;
			  case 11 :  /* define marker jj subname */
				define_marker_2(pcode);
				break;
			  case 12 :
				pcode.addStringNoID(SLine.getCode());
				break;
			  case 13 :/* ELSE ... */
				if (nbcmd > 0) {
					throw error(pos_first, "command must be the first command on the line");
				}
				block = check_block_type(pos_first, GLE_SRCBLK_ELSE, GLE_SRCBLK_MAGIC+GLE_OPBEGIN_IF, -1);
				if (try_get_token("IF")) {
					add_else_block_update(srclin, pcode, pos_start, true);
					pcode.setInt(pos_endoffs, pcode.size()-pos_start);
					pos_endoffs = pcode.size();
					pcode.addInt(0);
					pcode.addInt(22);
					parse_if(srclin, pcode);
					allow_extra_tokens = true;
				} else if (tokens->has_more_tokens()) {
					add_else_block_update(srclin, pcode, pos_start, false);
					m_auto_endif = true;
					allow_extra_tokens = true;
				} else {
					pcode.setInt(block->getOffset2(), srclin+1);
					add_else_block(srclin, pcode, false);
				}
				break;
			  case 14 : /* END if, sub, path, box, scale, translate, rotate */
				single_cmd = true;
				i = get_optional(op_begin, pcode);
				if (i == 0) throw error("type of 'end' missing, e.g. end if, end sub");
				if (i == GLE_OPBEGIN_IF) {
					check_block_type(pos_first, GLE_SRCBLK_MAGIC+i, GLE_SRCBLK_MAGIC+i, GLE_SRCBLK_ELSE);
					do_endif(srclin+1, pcode);
				} else {
					block = check_block_type(pos_first, GLE_SRCBLK_MAGIC+i, GLE_SRCBLK_MAGIC+i, -1);
					if (i == GLE_OPBEGIN_SUB || i == OP_BEGIN_OBJECT) {
						if (!isInSub() || m_CrSub == NULL) {
							throw error("'end sub' without corresponding 'sub [name]'");
						}
						m_CrSub->setStartEnd(block->getFirstLine(), srclin);
						do_endsub(srclin, pcode);
						if (m_CrSub->getParentSub() == NULL) {
							/* subroutine or static object */
							var_clear_local();
							setInSub(false);
						}
						m_CrSub = m_CrSub->getParentSub();
					} else if (i == GLE_OPBEGIN_BOX) {
						pcode.addInt(block->getFirstLine());
					}
					remove_last_block();
				}
				break;
			  case 16 : /* FILL (fillpath) */
				break;
			  case 15 : /* FCLOSE inchan */
				get_exp(pcode);
				break;
			  case 17: /* fopen "a.a" inchan read|write */
				get_strexp(pcode);
				get_var_add(&v,&vtyp);
				pcode.addInt(v);
				if (try_get_token("WRITE")) {
					pcode.addInt(1);
				} else {
					get_token("READ");
					pcode.addInt(0);
				}
				break;
			  case 61 : /* fread CHAN a$ x   */
			  case 62 : /* freadln */
				while (not_at_end_command()) {
					get_var_add(&v, &vtyp);
					pcode.addInt(49);
					pcode.addInt(v);
					pcode.addInt(vtyp);
				}
				break;
			  case 63 : /* fwrite */
			  case 64 : /* fwriteln */
				while (not_at_end_command()) {
					pcode.addInt(49);
					position = pcode.size();
					pcode.addInt(0);
					pcode.setInt(position, get_anyexp(pcode));
				}
				break;
			  case 75 : /* fgetline */
				get_exp(pcode);
				get_var_add(&v, &vtyp);
				pcode.addInt(v);
				break;
			  case 76 : /* ftokenizer chan commenttoks spacetoks singlechartoks */
				get_exp(pcode);
				get_strexp(pcode);
				get_strexp(pcode);
				get_strexp(pcode);
				break;
			  case 77: /* papersize */
			  	get_papersize(pcode);
			  	break;
			  case 78: /* margins */
				get_xy(pcode);
				get_xy(pcode);
			  	break;
			  case 79: /* orientation */
		  		pcode.addInt(get_first(op_orientation));
			  	break;
			  case 18 :  /* for var = exp1 to exp2 [step exp3] */
			  	/* create new for block */
				block = add_block(GLE_SRCBLK_FOR, srclin);
				get_var_add(&v, &vtyp);
				block->setVariable(v);
				/* translate first part of FOR into variable assignment */
				pcode.setInt(pcode.size()-1, 51);
				pcode.addInt(v);
				get_token("=");
				get_exp(pcode);
				pcode.setInt(pos_endoffs, pcode.size()-pos_start);
				/* now add real FOR, jump by next should go to this location */
				pos_endoffs = pcode.size();
				block->setOffset1(pos_endoffs-pos_start);
				pcode.addInt(0);
				pcode.addInt(18); /* opcode FOR */
				pcode.addInt(v);
				block->setOffset2(pcode.size());
				pcode.addInt(0);
				get_token("TO");
				get_exp(pcode);
				get_optional(op_for_step, pcode);
				break;
			  case 19 :/* goto */
				throw error("GOTO is considered bad programming practice :-)");
				break;
			  case 20 : /* gsave */
				break;
			  case 54 : /* grestore */
				break;
			  case 21 : /* icon x y */
				get_xy(pcode);
				break;
			  case 22 :  /* IF exp THEN ...  */
				if (nbcmd > 0) {
					// Do not move to parse_if (the latter is also used for "else if")
					throw error(pos_first, "command must be the first command on the line");
				}
				parse_if(srclin, pcode);
				if (tokens->has_more_tokens()) {
					m_auto_endif = true;
					allow_extra_tokens = true;
				}
				break;
			  case 55 : /* POSTCRIPT file$  width  height */
				get_strexp(pcode);
				get_xy(pcode);
				break;
				//
				// -- no need to ifdef these as
				//
			  case 67 : /* TIFF file$  width  height */
				printf("\nWarning: TIFF is deprecated: use BITMAP instead\n");
				get_strexp(pcode);
				get_xy(pcode);
				get_optional(op_bitmap, pcode);
				break;
			  case 68 : /* BITMAP file width height [type colors compress dpi greyscale resize] */
				get_strexp(pcode);
				get_xy(pcode);
				get_optional(op_bitmap, pcode);
				break;
			  case 69 : /* BITMAP_INFO file width, height [type] */
				get_strexp(pcode);
				get_var(pcode);
				get_var(pcode);
				get_optional(op_bitmap_info, pcode);
				break;
			  case GLE_KW_COLORMAP:
			  	get_strexp(pcode); // function
				get_xy(pcode);     // xrange
				get_xy(pcode);     // yrange
				get_xy(pcode);     // bitmap size
				get_xy(pcode);     // screen size
				get_optional(op_colormap, pcode);
			  	break;
			  case 56 : /* draw a previously defined object */
			  	{
					temp_str = tokens->next_token();
					GLESub* sub = is_draw_sub(temp_str);
					tokens->pushback_token();
					if (sub != NULL) {
						/* name of object = subroutine -> static call */
						pcode.addInt(0);
						get_strexp(pcode);
						GLESubCallInfo info(sub);
						GLESubCallAdditParam addit;
						info.setAdditParam(&addit);
						pass_subroutine_call(&info, tokens->token_pos_col());
						vtyp = 2;
						if (addit.getVal() == "") polish_pos("\"\"", 0, pcode, &vtyp);
						else polish_pos(addit.getVal(), addit.getPos(), pcode, &vtyp);
						gen_subroutine_call_code(&info, pcode);
					} else {
						/* call to object defined by variable */
						pcode.addInt(1);
						get_strexp(pcode);
						get_optional(op_draw, pcode);
					}
				}
				break;
			  case 23 : /* include "string" */
				pcode.setLast(GLE_KW_COMMENT);
				if (m_tokens.is_next_token("[")) {
					/* include [0-4.0.12] "somefile.gle" */
					/* support for compatibility mode */
					temp_str = tokens->next_token();
					int from = g_parse_compatibility(temp_str);
					m_tokens.ensure_next_token("-");
					temp_str = tokens->next_token();
					int to = g_parse_compatibility(temp_str);
					m_tokens.ensure_next_token("]");
					temp_str = tokens->next_token();
					if (g_get_compatibility() >= from && g_get_compatibility() <= to) {
						setSpecial(GLE_PARSER_INCLUDE);
						str_remove_quote(temp_str);
						setInclude(temp_str);
					}
				} else {
					setSpecial(GLE_PARSER_INCLUDE);
					temp_str = tokens->next_token();
					str_remove_quote(temp_str);
					setInclude(temp_str);
				}
				break;
			  case 58 : /* bigfile "string" This waits until 'run' to read the file*/
				get_strexp(pcode);
				break;
			  case 24 : /* input 1 a$=20,fill$=10,j=6 prompt "Age and name " */
				    /* input 1 a$,yval prompt "Age and name " */
			  case 25 : /* join a.tl->b.br   ,   string, arrows&line, string */
				get_strexp(pcode);
				pcode.addInt(get_first(op_joinname));
				get_strexp(pcode);
				get_optional(op_curve, pcode);
				break;
			  case 26 : /* marker square [2.2] */
				get_marker(pcode);
				if (not_at_end_command()) {
					get_exp(pcode);
				} else {
					pcode.addInt(0);
				}
				break;
			  case 27 : /* MOVE name */
				get_strexp(pcode);
				break;
			  case 28 : /* narc, Arc in clockwise direction */
				get_exp(pcode);
				get_xy(pcode);
				get_optional(op_arc, pcode);
				break;
			  case 29 : /* newpath */
				break;
			  case 30 : /* next */
			  	single_cmd = true;
				if (not_at_end_command()) {
			  		block = check_block_type(pos_first, GLE_SRCBLK_NEXT, GLE_SRCBLK_FOR, -1);
					get_var_add(&v, &vtyp);
					check_loop_variable(v);
				} else {
			  		block = check_block_type(pos_first, GLE_SRCBLK_NEXT, GLE_SRCBLK_WHILE, GLE_SRCBLK_UNTIL);
				}
				/* Set jump address */
				pcode.addInt(block->getFirstLine());
				pcode.addInt(block->getOffset1());
				/* Update address in first line @ getOffset2() */
				pcode.setInt(block->getOffset2(), srclin);
				/* And remove block from stack */
				remove_last_block();
				break;
			  case 31 : /* pie r a1 a2 fill pattern */
				get_exp(pcode);
				get_xy(pcode);
				get_optional(op_fill, pcode);
				break;
			  case 33 :
				get_xy(pcode);
				get_xy(pcode);
				get_xy(pcode);
				break;
			  case 34 : /* region */
				throw error("REGION is not yet implemented");
				break;
			  case 50 : /* Return EXP */
				if (not_at_end_command()) {
					get_exp_eol(pcode);
				} else {
					etype=1;
					polish("0", pcode, &etype);
				}
				if (tokens->has_more_tokens()) {
					throw error(pos_first, "return should be the last command on a line");
				}
				block = find_block(GLE_SRCBLK_MAGIC+GLE_OPBEGIN_SUB);
				if (block == NULL) {
					throw error(pos_first, "return outside of subroutine");
				}
				block = block->addDependendBlock(GLE_SRCBLK_RETURN, srclin);
				block->setOffset2(pcode.size());
				pcode.addInt(0);
				break;
			  case 35 : /* Reverse the current path */
				break;
			  case 36 : /* rline */
				get_xy(pcode);
				get_optional(op_line, pcode);
				break;
			  case 37 : /* rmove */
				get_xy(pcode);
				break;
			  case 38 : /* rotate */
				get_exp(pcode);
				break;
			  case 39 : /* save joe */
				get_strexp(pcode);
				break;
			  case 40: /* scale x y */
				get_xy(pcode);
				break;
			  case 41: /* SET color font hei just lwidth lstyle ldist */
				while (not_at_end_command()) {
				 f = get_first(op_set);
				 pcode.addInt(500+f);
				 switch (f) {
				  case 1: /* height */
					get_exp(pcode);
					break;
				  case 2: /* font */
					get_font(pcode);
					break;
				  case 3: /* justify */
					get_justify(pcode);
					break;
				  case 4: /* color */
				  case OP_SET_BACKGROUND:
				  case OP_SET_FILL:
				  case OP_SET_FILL_PATTERN:
					get_color(pcode);
					break;
				  case 5: /* dashlen */
					get_exp(pcode);
					break;
				  case 6: /* dash */
					get_exp(pcode);
					break;
				  case 7: /* lwidth */
					get_exp(pcode);
					break;
				  case 8: /* join */
					/* get_join(); */
					get_join(pcode);
					break;
				  case 9: /* cap */
					/* get_cap(); */
					get_cap(pcode);
					break;
				  case 10: /* fontlwidth */
					get_exp(pcode);
					break;
				  case OP_SET_FILL_METHOD:
				  case OP_SET_ARROW_STYLE:
				  case OP_SET_ARROW_TIP:
				  case OP_SET_IMAGE_FORMAT:
				  case OP_SET_TEX_SCALE:
				  	get_strexp(pcode);
					break;
				  case OP_SET_TEX_LABELS:
				  	get_exp(pcode);
					break;
				  case OP_SET_ARROW_SIZE:
				  case OP_SET_ARROW_ANGLE:
				  case OP_SET_TITLE_SCALE:
				  case OP_SET_ATITLE_SCALE:
				  case OP_SET_ALABEL_SCALE:
				  case OP_SET_TICKS_SCALE:
				  case OP_SET_ATITLE_DIST:
				  case OP_SET_ALABEL_DIST:
				  	get_exp(pcode);
					break;
				  }
				}
				break;
			  case 42 : /* size */
				get_xy(pcode);
				get_optional(op_size, pcode);
				break;
			  case 43 : /* STROKE */
				break;
			  case 44 : /* SUB JOE X Y$ Z   ... END SUB  */
				single_cmd = true;
				if (isInSub()) {
					throw error("can't define a subroutine within a sub ");
				}
				setInSub(true);
				m_CrSub = get_subroutine_declaration(pcode);
				pcode.addInt(m_CrSub->getIndex());
				add_block(GLE_SRCBLK_MAGIC+GLE_OPBEGIN_SUB, srclin);
				break;
			  case GLE_KW_DECLARESUB:
				pcode.setLast(GLE_KW_COMMENT);
				m_tokens.ensure_next_token_i("SUB");
				m_CrSub = get_subroutine_declaration(pcode);
				m_CrSub->setStart(srclin);
				var_clear_local();
				break;
			  case GLE_KW_DEFAULT:
				pcode.setLast(GLE_KW_COMMENT);
				get_subroutine_default_param(m_CrSub);
				break;
			  case 45 : /* text */
				temp_str = m_tokens.read_line();
				pcode.addStringNoID(temp_str);
				break;
			  case 59 : /* textdef */
				temp_str = m_tokens.read_line();
				pcode.addStringNoID(temp_str);
				break;
			  case 46 : /* translate x y */
				get_xy(pcode);
				break;
			  case 47 : /* until */
			  	single_cmd = true;
				get_exp_eol(pcode);
				block = add_block(GLE_SRCBLK_UNTIL, srclin);
				block->setOffset2(pcode.size());
				pcode.addInt(0);
				break;
			  case 48 : /* while */
			  	single_cmd = true;
				get_exp_eol(pcode);
				block = add_block(GLE_SRCBLK_WHILE, srclin);
				block->setOffset2(pcode.size());
				pcode.addInt(0);
				break;
			  case 32 : /* print numexp,strexp,strexp */
			  case 49 : /* write numexp,strexp,strexp */
				while (not_at_end_command()) {
					pcode.addInt(fctkey);
					position = pcode.size();
					pcode.addInt(0);
					pcode.setInt(position, get_anyexp(pcode));
				}
				break;
			  case 74 : /* tex */
				get_strexp(pcode);
				get_optional(op_tex, pcode);
				break;
			  case GLE_KW_RESTOREDEFAULTS:
				break;
			  case GLE_KW_SLEEP:
				get_exp(pcode);
				break;
			  default:
				throw error("unrecognised command {"+first+"}");
			}
		}
		if (!allow_extra_tokens && test_not_at_end_command()) {
			temp_str = tokens->read_line();
			throw error(string("extra tokens after command '")+first+"': '"+temp_str+"'");
		}
		pcode.setInt(pos_endoffs, pcode.size()-pos_start);
		nbcmd++;
		if (nbcmd > 1 && single_cmd) {
			throw error(pos_first, "command must occur on a separate line");
		}
	}
}

void GLEParser::do_text_mode(GLESourceLine &SLine, Tokenizer* tokens, GLEPcode& pcode) {
	int pos_endoffs = pcode.size();
	// Save space for end offset
	pcode.addInt(0);
	pcode.addInt(5);
	string str = tokens->read_line();
	// Handle comment symbol '!' at start of line
	if (str.length() > 0 && str[0] == '!') {
		str = "";
	}
	str_replace_start(str, "\\!", "!");
	// Check for end of block
	int pos = str_starts_with_trim(str, "END");
	if (pos != -1) {
		int len = str.length();
		string second_token = str.substr(pos, len-pos);
		str_trim_both(second_token);
		int idx = gt_index(op_begin, (char*)second_token.c_str());
		if (idx == cur_mode) {
			pcode.addInt(0);
			cur_mode = 0;
			return;
		}
	}
	// Add string to the block
	pcode.addInt(cur_mode);
	pcode.addStringNoID(str);
	pcode.setInt(pos_endoffs, pcode.size()-pos_endoffs);
}

GLESourceBlock* GLEParser::add_else_block(int srclin, GLEPcode& pcode, bool dangling) {
	remove_last_block();
	GLESourceBlock* block = add_block(GLE_SRCBLK_ELSE, srclin);
	block->setOffset2(pcode.size());
	block->setDangling(dangling);
	pcode.addInt(0);
	pcode.addInt(0);
	return block;
}

GLESourceBlock* GLEParser::add_else_block_update(int srclin, GLEPcode& pcode, int start_offs, bool dangling) {
	GLESourceBlock* if_block = last_block();
	int offs = if_block->getOffset2();
	GLESourceBlock* else_block = add_else_block(srclin, pcode, dangling);
	pcode.setInt(offs, srclin);
	pcode.setInt(offs+1, pcode.size()-start_offs);
	return else_block;
}

void GLEParser::do_endif(int srclin, GLEPcode& pcode) {
	GLESourceBlock* block = last_block();
	pcode.setInt(block->getOffset2(), srclin);
	remove_last_block();
	block = last_block();
	while (block != NULL && block->isDanglingElse()) {
		pcode.setInt(block->getOffset2(), srclin);
		remove_last_block();
		block = last_block();
	}
}

void GLEParser::do_endsub(int srclin, GLEPcode& pcode) {
	GLESourceBlock* block = last_block();
	int nb = block->getNbDependendingBlocks();
	for (int i = 0; i < nb; i++) {
		GLESourceBlock* dep = block->getDependingBlock(i);
		pcode.setInt(dep->getOffset2(), srclin);
	}
}

GLESourceBlock* GLEParser::find_block(int type) {
	int last = m_blocks.size()-1;
	while (last >= 0 && m_blocks[last].getType() != type) {
		last--;
	}
	return last >= 0 ? &m_blocks[last] : NULL;
}

GLESourceBlock* GLEParser::add_block(int type, int first_line) {
	m_blocks.push_back(GLESourceBlock(type, first_line));
	return &m_blocks.back();
}

GLESourceBlock* GLEParser::last_block() {
	return m_blocks.size() > 0 ? &m_blocks.back() : NULL;
}

void GLEParser::remove_last_block() {
	m_blocks.pop_back();
}

void GLEParser::check_loop_variable(int var) {
	GLESourceBlock* block = last_block();
	if (block == NULL || var != block->getVariable()) {
		stringstream err;
		err << "illegal variable '" << var_get_name(var);
		err << "': loop variable is '" << var_get_name(block->getVariable()) << "'";
		throw error(err.str());
	}
}

GLESourceBlock* GLEParser::check_block_type(int pos, int t0, int t1, int t2) {
	GLESourceBlock* block = last_block();
	if (block == NULL) {
		stringstream err;
		const char* end_t0 = GLESourceBlockEndName(t0);
		if (end_t0 != NULL) err << end_t0 << " ";
		err << "'" << GLESourceBlockName(t0) << "' without corresponding ";
		const char* begin_t1 = GLESourceBlockBeginName(t1);
		if (begin_t1 != NULL) err << begin_t1 << " ";
		err << "'" << GLESourceBlockName(t1) << "'";
		if (t2 != -1) {
			err << " or ";
			const char* begin_t2 = GLESourceBlockBeginName(t2);
			if (begin_t2 != NULL) err << begin_t2 << " ";
			err << "'" << GLESourceBlockName(t2) << "'";
		}
		throw error(pos, err.str());
	}
	if (block->getType() != t1 && block->getType() != t2) {
		stringstream err;
		err << "unterminated '" << block->getName() << "'";
		err << " " << block->getKindName();
		err << " (starting on line " << block->getFirstLine() << ") before ";
		const char* end_name = GLESourceBlockEndName(t0);
		if (end_name != NULL) err << end_name << " ";
		err << "'" << GLESourceBlockName(t0) << "'";
		throw error(pos, err.str());
	}
	return block;
}

GLESubMap* GLEParser::getSubroutines() {
	return &g_Subroutines;
}

GLESourceBlock::GLESourceBlock(int type, int first_line) {
	m_block_type = type;
	m_first_line = first_line;
	m_variable = -1;
	m_pcode_offs1 = 0;
	m_pcode_offs2 = 0;
	m_dangling = false;
	m_deps = NULL;
}

GLESourceBlock::GLESourceBlock(const GLESourceBlock& block) {
	m_block_type = block.m_block_type;
	m_first_line = block.m_first_line;
	m_variable = block.m_variable;
	m_pcode_offs1 = block.m_pcode_offs1;
	m_pcode_offs2 = block.m_pcode_offs2;
	m_dangling = block.m_dangling;
	m_deps = NULL;
	if (block.m_deps != NULL) {
		int size = block.m_deps->size();
		m_deps = new vector<GLESourceBlock>();
		for (int i = 0; i < size; i++) {
			m_deps->push_back((*block.m_deps)[i]);
		}
	}
}

GLESourceBlock::~GLESourceBlock() {
	if (m_deps != NULL) delete m_deps;
}

GLESourceBlock* GLESourceBlock::addDependendBlock(int type, int first_line) {
	if (m_deps == NULL) m_deps = new vector<GLESourceBlock>();
	m_deps->push_back(GLESourceBlock(type, first_line));
	return &m_deps->back();
}

int GLESourceBlock::getNbDependendingBlocks() {
	return m_deps != NULL ? m_deps->size() : 0;
}

const char* GLESourceBlock::getName() {
	return GLESourceBlockName(m_block_type);
}

const char* GLESourceBlock::getKindName() {
	switch (m_block_type) {
		case GLE_SRCBLK_UNTIL:
		case GLE_SRCBLK_WHILE:
		case GLE_SRCBLK_FOR:
			return "loop";
		default:
			return "block";
	}
}

const char* GLESourceBlockEndName(int type) {
	switch (type) {
		case GLE_SRCBLK_NEXT:
		case GLE_SRCBLK_ELSE:
			return NULL;
		default:
			return "end";
	}
}

const char* GLESourceBlockBeginName(int type) {
	switch (type) {
		case GLE_SRCBLK_FOR:
		case GLE_SRCBLK_UNTIL:
		case GLE_SRCBLK_WHILE:
		case GLE_SRCBLK_ELSE:
		case GLE_SRCBLK_MAGIC+GLE_OPBEGIN_IF:
			return NULL;
		default:
			return "begin";
	}
}

const char* GLESourceBlockName(int type) {
	if (type > GLE_SRCBLK_MAGIC) {
		int count, width;
		get_key_info(op_begin, &count, &width);
		for (int i = 0; i < count; i++) {
			if (int(op_begin[i].idx) == type - GLE_SRCBLK_MAGIC) return op_begin[i].name;
		}
	}
	switch (type) {
		case GLE_SRCBLK_UNTIL: return "until";
		case GLE_SRCBLK_WHILE: return "while";
		case GLE_SRCBLK_FOR:   return "for";
		case GLE_SRCBLK_NEXT:  return "next";
		case GLE_SRCBLK_ELSE:  return "else";
		default: return "unknown";
	}
}

void gt_find_error(const char* found, OPKEY lkey, int nk) {
	stringstream ss;
	ss << "found '" << found << "', but expecting one of:" << endl;
	ss << "\t";
	for (int i=0; i<nk; i++) {
		ss << lkey[i].name;
		if (i != nk-1) ss << ", ";
		if ((i+1) % 3 == 0) ss << endl << "\t";
	}
	if (nk % 3 != 0) ss << endl;
	g_throw_parser_error(ss.str());
}

int gt_first(OPKEY lkey, int *curtok, TOKENS tk, int *ntok, int *pcode, int *plen) {
	int nk,i,width=0,p;
	for (i=0; lkey[i].typ!=typ_end; i++) {
		p = lkey[i].pos;
		if (p>width) width = p ;
	}
	nk = i;
	for (i=0; i<nk; i++) {
		if (str_i_equals(lkey[i].name,tok(*curtok))) {
			(*curtok)++;
			return lkey[i].idx;
		}
	}
	gt_find_error(tok(*curtok), lkey, nk);
	(*curtok)++;
	return 0;
}

bool gt_firstval_err(OPKEY lkey, const char *s, int* result) {
	for (int i=0; lkey[i].typ!=typ_end; i++) {
		if (str_i_equals(lkey[i].name,s)) {
			*result = lkey[i].idx;
			return true;
		}
	}
	return false;
}

int gt_firstval(OPKEY lkey, const char *s) {
	int nk = 0;
	for (int i=0; lkey[i].typ!=typ_end; i++) {
		if (str_i_equals(lkey[i].name,s)) {
			return lkey[i].idx;
		}
		nk++;
	}
	gt_find_error(s, lkey, nk);
	return 0;
}

int gt_index(OPKEY lkey,char *s) {
	for (int i = 0; lkey[i].typ!=typ_end; i++) {
		if (str_i_equals(lkey[i].name,s)) {
			return lkey[i].idx;
		}
	}
	return 0;
}

#undef get_first
#define get_first(key) gt_first(key,curtok,tk,ntok,pcode,plen)

void mystrcpy(char **d, const char *s) {
	if (*d!=0) myfree(*d);
	*d = 0;
	*d = (char*) myallocz(strlen(s)+1);
	strcpy(*d,s);
}

#undef get_exp
#define get_exp() polish(tok((*curtok)++),(char *) pcode,plen,&etype)

/* pos=   Offset to find the data			*/
/* idx=   For switches, which can only have one value. 	*/

struct mark_struct { const char *name; const char *font; int cc; double rx; double ry; double scl; bool center;};

struct mark_struct stdmark_v35[] = {
	{ "DOT",      "RM",      46,-.125,-.0435,3.0,false },   /* dot */
	{ "CROSS",    "TEXSY",   2,-.375,-.24,1.0,   false },   /* cross */
	{ "FCIRCLE",  "GLEMARK", 8,-.5,-.5,0.7,      false },   /* fcircle */
	{ "FSQUARE",  "GLEMARK", 5,-.5,-.5,0.7,      false },   /* fsquare */
	{ "FTRIANGLE","GLEMARK", 2,-.5,-.433,0.7,    false },   /* ftriangle */
	{ "FDIAMOND", "GLEMARK", 11,-.5,-.5,0.7,     false },   /* fdiamond */
	{ "CIRCLE",   "GLEMARK", 7,-.5,-.5,0.7,      false },   /* circle */
	{ "SQUARE",   "GLEMARK", 4,-.5,-.5,0.7,      false },   /* square */
	{ "TRIANGLE", "GLEMARK", 1,-.5,-.433,0.7,    false },   /* triangle */
	{ "DIAMOND",  "GLEMARK", 10,-.5,-.5,0.7,     false },   /* diamond */
	{ "PLUS",     "TEXCMR",  43,-.375,-.24,1.0,  false },   /* plus, fixed */
	{ "CLUB",     "TEXSY",   124,-.38,-.3,1.0,   false },   /* club */
	{ "HEART",    "TEXSY",   126,-.38,-.34,1.0,  false },   /* heart */
	{ "DIAMONDZ", "TEXSY",   125,-.38,-.26,1.0,  false },   /* diamondz */
	{ "SPADE",    "TEXSY",   127,-.375,-.24,1.0, false },   /* spade (needs fixing) */
	{ "STAR",     "TEXMI",   63,-.25,-.21,1.0,   false },   /* star */
	{ "SNAKE",    "TEXSY",   120,-.21,-.22,1.0,  false },   /* snake */
	{ "DAG",      "TEXSY",   121,-.21,-.22,1.0,  false },   /* dag */
	{ "DDAG",     "TEXSY",   122,-.21,-.22,1.0,  false },   /* dagg */
	{ "ASTERIX",  "TEXSY",   3,-.25,-.24,1.0,    false },   /* asterix */
	{ "ASTERISK", "TEXSY",   3,-.25,-.24,1.0,    false },   /* asterix */
	{ "OPLUS",    "TEXSY",   8,-.40,-.24,1.0,    false },   /* oplus */
	{ "OMINUS",   "TEXSY",   9,-.40,-.24,1.0,    false },   /* ominus */
	{ "OTIMES",   "TEXSY",   10,-.40,-.24,1.0,   false },   /* otimes */
	{ "ODOT",     "TEXSY",   12,-.40,-.24,1.0,   false },   /* odot */
	{ "TRIANGLEZ","TEXSY",   52,-.44,-.26,1.0,   false },   /* trianglez */
	{ "DIAMONDZ", "TEXSY",   125,-.38,-.26,1.0,  false },   /* diamondz */
	{ "WCIRCLE",  "GLEMARK", 9,-.5,-.5,0.7,      false },   /* wcircle */
	{ "WTRIANGLE","GLEMARK", 3,-.5,-.433,0.7,    false },   /* wtriangle */
	{ "WSQUARE",  "GLEMARK", 6,-.5,-.5,0.7,      false },   /* wsquare */
	{ "WDIAMOND", "GLEMARK", 12,-.5,-.5,0.7,     false },   /* wdiamond */
	{ "PLANE",    "PSZD",    40,0.0,0.0,1.0,     false },   /* ZapDingbats */
	{ "HANDPEN",  "PSZD",    45,0.0,0.0,1.0,     false },   /* ZapDingbats */
	{ "SCIRCLE",  "PSZD",    109,0.0,0.0,1.0,    false },   /* ZapDingbats */
	{ "SSQUARE",  "PSZD",    111,0.0,0.0,1.0,    false },   /* ZapDingbats */
	{ "PHONE",    "PSZD",    37,0.0,0.0,1.0,     false },   /* ZapDingbats */
	{ "LETTER",   "PSZD",    41,0.0,0.0,1.0,     false },   /* ZapDingbats */
	{ "STAR2",    "PSZD",    69,0.0,0.0,1.0,     false },   /* ZapDingbats */
	{ "STAR3",    "PSZD",    79,0.0,0.0,1.0,     false },   /* ZapDingbats */
	{ "STAR4",    "PSZD",    98,0.0,0.0,1.0,     false },   /* ZapDingbats */
	{ "FLOWER",   "PSZD",    96,0.0,0.0,1.0,     false },   /* ZapDingbats */
	{ NULL,       NULL,      0,0,0,0,            false }
};

struct mark_struct stdmark[] = {
	{ "ASTERISK", "TEXSY",   3,0,0.0,1.67,       true  },   /* asterisk */
	{ "ASTERIX",  "TEXSY",   3,0,0.0,1.67,       true  },   /* asterisk (for compatibility) */
	{ "CIRCLE",   "GLEMARK", 7,-0.5,-0.5,0.7,    false },   /* circle */
	{ "CLUB",     "PSZD",    168,0,0,1.0,        true  },   /* club */
	{ "CROSS",    "GLEMARK", 13,-0.5,-0.5,0.7,   false },   /* cross */
	{ "DAG",      "TEXSY",   121,0,0.005,0.78,   true  },   /* dag */
	{ "DDAG",     "TEXSY",   122,0,0,0.79,       true  },   /* dagg */
	{ "DIAMOND",  "GLEMARK", 10,-0.5,-0.5,0.7,   false },   /* diamond */
	{ "DIAMONDZ", "TEXSY",   125,0,0,0.81,       true  },   /* diamondz */
	{ "DOT",      "GLEMARK", 8,-0.5,-0.5,0.3325, false },   /* dot */
	{ "FCIRCLE",  "GLEMARK", 8,-0.5,-0.5,0.7,    false },   /* fcircle */
	{ "FDIAMOND", "GLEMARK", 11,-0.5,-0.5,0.7,   false },   /* fdiamond */
	{ "FLOWER",   "PSZD",    96,0,0.03,0.97,     true  },   /* ZapDingbats */
	{ "FSQUARE",  "GLEMARK", 5,-0.5,-0.5,0.7,    false },   /* fsquare */
	{ "FTRIANGLE","GLEMARK", 2,-0.5,-0.35,0.7,   false },   /* ftriangle */
	{ "HANDPEN",  "PSZD",    45,0,0,0.91,        true  },   /* ZapDingbats */
	{ "HEART",    "TEXSY",   126,0,-0.06,0.96,   true  },   /* heart */
	{ "LETTER",   "PSZD",    41,0,0,1.15,        true  },   /* ZapDingbats */
	{ "MINUS",    "GLEMARK", 15,-0.5,-0.5,0.7,   false },   /* minus */
	{ "ODOT",     "TEXSY",   12,0,0,1.07,        true  },   /* odot */
	{ "OMINUS",   "TEXSY",   9,0,0,1.07,         true  },   /* ominus */
	{ "OPLUS",    "TEXSY",   8,0,0,1.07,         true  },   /* oplus */
	{ "OTIMES",   "TEXSY",   10,0,0,1.07,        true  },   /* otimes */
	{ "PHONE",    "PSZD",    37,0,0,1.11,        true  },   /* ZapDingbats */
	{ "PLANE",    "PSZD",    40,0,0,1.0,         true  },   /* ZapDingbats */
	{ "PLUS",     "GLEMARK", 14,-0.5,-0.5,0.7,   false },   /* plus */
	{ "SCIRCLE",  "PSZD",    109,0,0,0.90,       true  },   /* ZapDingbats */
	{ "SNAKE",    "TEXSY",   120,0,0,0.785,      true  },   /* snake */
	{ "SPADE",    "PSZD",    171,0,0,1.0,        true  },   /* spade */
	{ "SQUARE",   "GLEMARK", 4,-0.5,-0.5,0.7,    false },   /* square */
	{ "SSQUARE",  "PSZD",    111,0,0,1.0,        true  },   /* ZapDingbats */
	{ "STAR",     "TEXMI",   63,0,0.03,1.5,      true  },   /* star */
	{ "STAR2",    "PSZD",    69,0,0,1.0,         true  },   /* ZapDingbats */
	{ "STAR3",    "PSZD",    79,0,0.04,1.0,      true  },   /* ZapDingbats */
	{ "STAR4",    "PSZD",    98,0,0,1.0,         true  },   /* ZapDingbats */
	{ "TRIANGLE", "GLEMARK", 1,-0.5,-0.35,0.7,   false },   /* triangle */
	{ "TRIANGLEZ","TEXSY",   52,0,0,1.0,         true  },   /* trianglez */
	{ "WCIRCLE",  "GLEMARK", 9,-0.5,-0.5,0.7,    false },   /* wcircle */
	{ "WTRIANGLE","GLEMARK", 3,-0.5,-0.35,0.7,   false },   /* wtriangle */
	{ "WSQUARE",  "GLEMARK", 6,-0.5,-0.5,0.7,    false },   /* wsquare */
	{ "WDIAMOND", "GLEMARK", 12,-0.5,-0.5,0.7,   false },   /* wdiamond */
	{ NULL,       NULL,      0,0,0,0,            false }
};	/* change range check below when adding markers */

/*
GLEMARK
1    Triangle
2    Filled Triangle
3    White Triangle
4    Square
5    Filled Square
6    White Square
7    Circle
8    Filled Circle
9    White Circle
10   Diamond
11   Filled Diamond
12   White Diamond
13   Cross
14   Plus
15   Minus
*/

void mark_clear(void) {
	for (int i = 0; i < nmark; i++) {
		if (mark_sub[i] != NULL) { myfree(mark_sub[i]); mark_sub[i] = NULL; }
		if (mark_name[i] != NULL) { myfree(mark_name[i]); mark_name[i] = NULL; }
	}
	for (int i = 0; i < nmrk; i++) {
		if (mrk_name[i] != NULL) { myfree(mrk_name[i]); mrk_name[i] = NULL; }
		if (mrk_fname[i] != NULL) { myfree(mrk_fname[i]); mrk_fname[i] = NULL; }
	}
	nmrk = 0;
	nmark = 0;
	struct mark_struct *p;
	if (g_get_compatibility() <= GLE_COMPAT_35) {
		for (int i = 0; stdmark_v35[i].name != NULL; i++) {
			p = &stdmark_v35[i];
			bool fg = (p->rx == 0);
			g_defmarker(p->name,p->font,p->cc,p->rx,p->ry,p->scl,fg);
		}
	} else {
		for (int i = 0; stdmark[i].name !=NULL; i++) {
			p = &stdmark[i];
			g_defmarker(p->name,p->font,p->cc,p->rx,p->ry,p->scl,stdmark[i].center);
		}
	}
}

void pass_file_name(const char* name, string& file) {
	// Support operations on strings, but make sure it also works with unquoted strings containing "/" chars
	if (str_contains(name, '"') || str_contains(name, '$') || str_contains(name, '+')) {
		polish_eval_string(name, &file);
	} else {
		file = name;
	}
}

void set_global_parser(GLEParser* parser) {
	g_parser = parser;
}

GLEParser* get_global_parser() {
	return g_parser;
}

GLEPolish* get_global_polish() {
	return g_parser != NULL ? g_parser->getPolish() : NULL;
}
