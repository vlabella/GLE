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

#ifndef INCLUDE_PASS
#define INCLUDE_PASS

//typedef struct op_key* OPKEY;
typedef struct op_key OPKEY[];

#define GLE_PARSER_NONE 0
#define GLE_PARSER_INCLUDE 1

#define GLE_SRCBLK_UNTIL  0
#define GLE_SRCBLK_WHILE  1
#define GLE_SRCBLK_FOR    2
#define GLE_SRCBLK_NEXT   3
#define GLE_SRCBLK_ELSE   4
#define GLE_SRCBLK_RETURN 5
#define GLE_SRCBLK_MAGIC  100

#define GLE_OPBEGIN_IF    6
#define GLE_OPBEGIN_SUB   7
#define GLE_OPBEGIN_BOX   2
#define GLE_OPBEGIN_GRAPH 10
#define GLE_OPBEGIN_KEY   16
#define GLE_OPBEGIN_SURF  24

class GLESubMap;

void GLEParserInitTokenizer(Tokenizer* tokens);

class GLESourceBlock {
protected:
	int m_block_type;
	int m_variable;
	int m_first_line;
	int m_pcode_offs1; /* jump address */
	int m_pcode_offs2; /* fill in address */
	bool m_dangling;
	vector<GLESourceBlock>* m_deps;
public:
	GLESourceBlock(int type, int first_line);
	GLESourceBlock(const GLESourceBlock& block);
	~GLESourceBlock();
	const char* getName();
	const char* getKindName();
	GLESourceBlock* addDependendBlock(int type, int first_line);
	int getNbDependendingBlocks();
	inline int getFirstLine() { return m_first_line; }
	inline int getType() { return m_block_type; }
	inline void setVariable(int var) { m_variable = var; }
	inline int getVariable() { return m_variable; }
	inline int getOffset1() { return m_pcode_offs1; }
	inline int getOffset2() { return m_pcode_offs2; }
	inline void setOffset1(int offs) { m_pcode_offs1 = offs; }
	inline void setOffset2(int offs) { m_pcode_offs2 = offs; }
	inline bool isDangling() { return m_dangling; }
	inline void setDangling(int yes) { m_dangling = yes; }
	inline bool isDanglingElse() { return m_dangling && m_block_type == GLE_SRCBLK_ELSE; }
	inline GLESourceBlock* getDependingBlock(int i) { return &(*m_deps)[i]; }
};

const char* GLESourceBlockName(int type);
const char* GLESourceBlockEndName(int type);
const char* GLESourceBlockBeginName(int type);

class GLESubCallAdditParam {
/* current implementation just for "name" option of draw command */
/* later: implement for all common options, such as color, lwidth, ... */
protected:
	string m_Val;
	int m_Pos;
public:
	GLESubCallAdditParam();
	~GLESubCallAdditParam();
	int isAdditionalParam(const string& str);
	void setAdditionalParam(int idx, const string& val, int pos);
	inline const string& getVal() { return m_Val; }
	inline int getPos() { return m_Pos; }
};

class GLESubCallInfo {
protected:
	vector<string> m_ParamVal;
	vector<int> m_ParamPos;
	GLESubCallAdditParam* m_Addit;
	GLESub* m_Sub;
public:
	GLESubCallInfo(GLESub* sub);
	~GLESubCallInfo();
	void setParam(int i, const string& val, int pos);
	inline int getParamPos(int i) { return m_ParamPos[i]; }
	inline const string& getParamVal(int i) { return m_ParamVal[i]; }
	inline void setAdditParam(GLESubCallAdditParam* addit) { m_Addit = addit; }
	inline GLESubCallAdditParam* getAdditParam() { return m_Addit; }
	inline GLESub* getSub() { return m_Sub; }
};

class GLEBlocks;

class GLEParser {
protected:
	GLEScript* m_Script;
	TokenizerLanguage m_lang;
	StringTokenizer m_tokens;
	GLEPolish* m_polish;
	GLESub* m_CrSub;
	GLEBlocks* m_blockTypes;
	string m_include;
	int m_special;
	bool m_auto_endif;
	bool m_insub;
	vector<GLESourceBlock> m_blocks;
public:
	GLEParser(GLEScript* script, GLEPolish* polish);
	~GLEParser();
	GLEBlocks* getBlockTypes();
	void initTokenizer();
	double evalTokenToDouble() throw(ParserError);
	void evalTokenToString(string* str) throw(ParserError);
	void evalTokenToFileName(string* str) throw(ParserError);
	bool pass_block_specific(GLESourceLine& sourceLine, GLEPcode& pcode);
	void passt(GLESourceLine &SLine, GLEPcode& pcode) throw(ParserError);
	void polish_eol(GLEPcode& pcode, int *rtype) throw(ParserError);
	void polish(GLEPcode& pcode, int *rtype) throw(ParserError);
	void polish(const char* str, GLEPcode& pcode, int *rtype) throw(ParserError);
	void polish_pos(const string& arg, int pos, GLEPcode& pcode, int* rtype) throw(ParserError);
	void get_var(GLEPcode& pcode) throw (ParserError);
	void get_xy(GLEPcode& pcode) throw(ParserError);
	void get_exp(GLEPcode& pcode) throw(ParserError);
	void get_exp_eol(GLEPcode& pcode) throw(ParserError);
	void get_strexp(GLEPcode& pcode) throw(ParserError);
	int get_anyexp(GLEPcode& pcode) throw(ParserError);
	void pass_subroutine_call(GLESubCallInfo* info, int poscol) throw(ParserError);
	void gen_subroutine_call_code(GLESubCallInfo* info, GLEPcode& pcode) throw(ParserError);
	void gen_subroutine_call_polish_arg(GLESubCallInfo* info, int i, GLEPcode& pcode) throw(ParserError);
	void evaluate_subroutine_arguments(GLESubCallInfo* info, GLEArrayImpl* arguments);
	void get_subroutine_call(GLEPcode& pcode, string* name = NULL, int poscol = 0) throw(ParserError);
	GLESub* get_subroutine_declaration(GLEPcode& pcode) throw(ParserError);
	void get_subroutine_default_param(GLESub* sub) throw(ParserError);
	void get_if(GLEPcode& pcode) throw(ParserError);
	void parse_if(int srclin, GLEPcode& pcode) throw(ParserError);
	GLESourceBlock* add_else_block(int srclin, GLEPcode& pcode, bool dangling);
	GLESourceBlock* add_else_block_update(int srclin, GLEPcode& pcode, int start_offs, bool dangling);
	void do_endif(int srclin, GLEPcode& pcode);
	void do_endsub(int srclin, GLEPcode& pcode);
	int get_optional(OPKEY lkey, GLEPcode& pcode) throw(ParserError);
	int get_first(OPKEY lkey) throw(ParserError);
   int get_first(const string& token, OPKEY lkey) throw(ParserError);
	void get_token(const char* token) throw(ParserError);
	bool try_get_token(const char* token) throw(ParserError);
	void get_fill(GLEPcode& pcode) throw (ParserError);
	void get_marker(GLEPcode& pcode) throw (ParserError);
	void get_var_add(int *var, int *vtype) throw (ParserError);
	int pass_marker(const string& marker) throw (ParserError);
	void define_marker_1(GLEPcode& pcode) throw (ParserError);
	void define_marker_2(GLEPcode& pcode) throw (ParserError);
	void get_font(GLEPcode& pcode) throw (ParserError);
	void get_justify(GLEPcode& pcode) throw (ParserError);
	void get_color(GLEPcode& pcode) throw (ParserError);
	void get_join(GLEPcode& pcode) throw (ParserError);
	void get_cap(GLEPcode& pcode) throw (ParserError);
	void get_papersize(GLEPcode& pcode) throw (ParserError);
	void do_text_mode(GLESourceLine &SLine, Tokenizer* tokens, GLEPcode& pcode) throw (ParserError);
	void checkmode() throw(ParserError);
	void get_block_type(int type, string& result);
	ParserError create_option_error(OPKEY lkey, int count, const string& token);
	int get_one_option(op_key* lkey, GLEPcode& pcode, int plen) throw(ParserError);
	void duplicate_error(GLEPcode& pcode, int pos) throw(ParserError);
	void checkValidName(const string& name, const char* type, int pos) throw(ParserError);
	void setAllowSpace(bool allow);
	bool not_at_end_command();
	bool test_not_at_end_command();
	GLESub* is_draw_sub(const string& str);
	GLESourceBlock* add_block(int type, int first_line);
	GLESourceBlock* last_block();
	GLESourceBlock* find_block(int type);
	void remove_last_block();
	void check_loop_variable(int var) throw (ParserError);
	GLESourceBlock* check_block_type(int pos, int t0, int t1, int t2) throw (ParserError);
	GLESubMap* getSubroutines();
	inline Tokenizer* getTokens() { return &m_tokens; }
	inline GLEPolish* getPolish() { return m_polish; }
	inline void setString(const char* str) { m_tokens.set_string(str); }
	inline void resetSpecial() { m_special = GLE_PARSER_NONE; }
	inline bool hasSpecial(int special) { return (m_special & special) != 0; }
	inline void setSpecial(int special) { m_special |= special; }
	inline const string& getInclude() { return m_include; }
	inline void setInclude(const string& name) { m_include = name; }
	inline bool isInSub() { return m_insub; }
	inline void setInSub(bool insub) { m_insub = insub; }
	inline ParserError error(const string& src) const {
		return m_tokens.error(src);
	};
	inline ParserError error(int column, const string& src) const {
		return m_tokens.error(column, src);
	};
	inline GLEScript* getScript() { return m_Script; }
	inline GLEGlobalSource* getSource() { return m_Script->getSource(); }
};

int gt_firstval(OPKEY lkey, const char *s);
bool gt_firstval_err(OPKEY lkey, const char *s, int* result);
int gt_index(OPKEY lkey,char *s);
int pass_justify(const std::string& s);
int pass_marker(char *s) throw(ParserError);
void mark_clear(void);
void pass_checkmode(void);
void spop(int v);
void spush(int v);
void scheck(int v);
//
// used in utils
//

void set_global_parser(GLEParser* parser);
GLEParser* get_global_parser();

#endif
