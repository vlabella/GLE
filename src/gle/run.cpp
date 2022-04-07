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

#include "all.h"
#include "mem_limits.h"
#include "token.h"
#include "core.h"
#include "glearray.h"
#include "polish.h"
#include "pass.h"
#include "op_def.h"
#include "var.h"
#include "cutils.h"
#include "gprint.h"
#include "texinterface.h"
#include "keyword.h"
#include "run.h"
#include "fn.h"
#include "file_io.h"
#include "sub.h"
#include "gle-interface/gle-interface.h"

void name_get2(char *n,double *x1,double *y1,double *x2,double *y2) throw(ParserError);

#define GRAPHDEF extern
#include "graph.h"

extern OPKEY op_begin;
extern GLESubMap g_Subroutines;

extern int **gpcode;   /* gpcode is a pointer to an array of poiter to int */
extern int *gplen;     /* gpcode is a pointer to an array of int */

string get_b_name(int jj) {
	for (int i=0; op_begin[i].typ!=0; i++) {
		if(int(op_begin[i].idx) == jj){
			return string(op_begin[i].name);
		}
	}
	return "unknown";
}

void text_def(uchar *ss);
void run_bigfile(char *ss);

void begin_config(const std::string& block, int *pln, int *pcode, int *cp);
void begin_tex_preamble(int *pln, int *pcode, int *cp);
void begin_tex(GLERun* run, int *pln, int *pcode, int *cp);
void begin_surface(int *pln, int *pcode, int *cp) throw(ParserError);
void begin_letz(int *pln, GLEPcodeList* pclist, int *pcode, int *cp) throw(ParserError);
void begin_fitz(int *pln, int *pcode, int *cp) throw(ParserError);
void begin_contour(int *pln, int *pcode, int *cp) throw(ParserError);
// void begin_fitls(int *pln, int *pcode, int *cp) throw(ParserError);

class GLEBox {
public:
	GLEBox();
	void setFill(const GLERC<GLEColor>& fill);
	void setRound(double round);
	void draw(GLERun* run, double x1, double y1, double x2, double y2);
	inline void setStroke(bool stroke) { m_HasStroke = stroke; }
	inline bool hasStroke() const { return m_HasStroke; }
	inline void setReverse(bool reverse) { m_HasReverse = reverse; }
	inline bool hasReverse() const { return m_HasReverse; }
	inline bool isFilled() const { return !m_Fill->isTransparent(); }
	inline GLERC<GLEColor> getFill() const { return m_Fill; }
	inline bool isRound() const { return m_IsRound; }
	inline double getRound() const { return m_Round; }
	inline void setName(GLEString* name) { m_name = name; }
	inline bool hasName() const { return !m_name.isNull(); }
	inline GLEString* getName() const { return m_name.get(); }
	inline void setAdd(double add) { m_Add = add; }
	inline double getAdd() const { return m_Add; }
protected:
	bool m_HasStroke;
	bool m_HasReverse;
	double m_Add;
	bool m_IsRound;
	double m_Round;
	GLERC<GLEString> m_name;
	GLERC<GLEColor> m_Fill;
};

class GLEStoredBox : public GLEBox {
public:
	GLEStoredBox();
	inline bool isSecondPass() const { return m_SecondPass; }
	inline void setSecondPass(bool second) { m_SecondPass = second; }
	inline GLEDevice* getDevice() { return m_Device; }
	inline void setDevice(GLEDevice* dev) { m_Device = dev; }
	inline GLEObjectRepresention* getObjectRep() const { return m_Object.get(); }
	inline bool hasObjectRep() { return !m_Object.isNull(); }
	inline void setObjectRep(GLEObjectRepresention* obj) { m_Object = obj; }
	inline GLERectangle* getSaveBounds() { return &m_SaveBounds; }
	void setOrigin(double x0, double y0) { m_Orig.setXY(x0, y0); }
	const GLEPoint& getOrigin() { return m_Orig; }
private:
	GLERectangle m_SaveBounds;
	GLEPoint m_Orig;
	bool m_SecondPass;
	GLEDevice* m_Device;
	GLERC<GLEObjectRepresention> m_Object;
};

class GLEBoxStack {
protected:
	static GLEBoxStack m_Instance;
	vector<GLEStoredBox> m_Boxes;
public:
	static inline GLEBoxStack* getInstance() { return &m_Instance; }
	inline int size() { return m_Boxes.size(); }
	inline void removeBox() { m_Boxes.pop_back(); }
	inline GLEStoredBox* lastBox() { return &m_Boxes.back(); }
	inline GLEStoredBox* newBox() { m_Boxes.push_back(GLEStoredBox()); return lastBox(); }
};

GLEBoxStack GLEBoxStack::m_Instance;

GLEStoredBox* box_start(void);

extern int this_line;
extern int trace_on;
static int path_clip[4],path_stroke[4];
static GLERC<GLEColor> path_fill[4];
static GLERC<GLEColor> path_fill_backup[4];
static double path_x[4],path_y[4];
static int npath;
#define true (!false)
#define false 0
int done_open = false;

class GLEFile {
protected:
	bool m_ReadWrite;
	FILE* m_Output;
	StreamTokenizer* m_Input;
	string m_buffer;
	string m_FileName;
public:
	GLEFile();
	~GLEFile();
	void close();
	void open(const char* fname) throw(ParserError);
	bool eof() throw(ParserError);
	char* getToken() throw(ParserError);
	char* readLine() throw(ParserError);
	void gotoNewLine() throw(ParserError);
	void resetLang();
	void setLangChars(int type, const char* str);
	inline void setCommentChars(const std::string& str) { setLangChars(0, str.c_str()); }
	inline void setSpaceTokens(const std::string& str) { setLangChars(1, str.c_str()); }
	inline void setSingleCharTokens(const std::string& str) { setLangChars(2, str.c_str()); }
	inline bool isRead() { return m_ReadWrite; }
	inline void setRdWr(bool rd) { m_ReadWrite = rd; }
	inline FILE* getOutput() { return m_Output; }
};

vector<GLEFile*> g_Files;

int f_getchan(void);
void f_readahead(int chn);
int f_testchan(int chn) throw(ParserError);
void siffree(char **s);
void f_getline(int chn);
char *f_gettok(int chn);
static int chn;
int f_eof(int chn) throw(ParserError);
char *f_getnext(int chn);

void f_create_chan(int var, const char* fname, int rd_wr);
void f_close_chan(int idx) throw(ParserError);


/*---------------------------------------------------------------------------*/
/* pos=   Offset to find the data			                                 */
/* idx=   For switches, which can only have one value. 	                     */
/* The pos is the order the items will be placed in the pcode                */
/*                                                                           */
/* Switches 	int 	placed in directly, 1 present, 0 not present         */
/* expressions 	LONG* 	pointed to, 0 if not present.                        */
/* color/fill	LONG* 	Pointer to exp 0 if not present.                     */
/* marker	LONG*	Pointer to exp 0 if not present.                         */
/* lstyle 	LONG*	Pointer to exp 0 if not present.                         */
/* font 	int* 	Pointer to string expression.                            */
/*---------------------------------------------------------------------------*/

extern char *mainkey[];
extern int gle_debug;
int can_fillpath = false;
vector<int> g_drobj;

#define readval(x) x=evalDouble(getStack(),getPcodeList(),pcode,&cp)
#define readxy(x,y) {x=evalDouble(getStack(),getPcodeList(),pcode,&cp); y=evalDouble(getStack(),getPcodeList(),pcode,&cp);}
#define readstr(s) {ostr=evalStringPtr(getStack(),getPcodeList(),pcode,&cp); s=ostr->toUTF8();}
#define readlong(i) i = *(pcode+cp++)
#define readvalp(x,p) {zzcp=0; x=evalDouble(getStack(),getPcodeList(),p,&zzcp);}
#define readstrp(p) {zzcp=0; ostr=evalStringPtr(getStack(),getPcodeList(),p,&zzcp);}

#define PCODE_UNKNOWN_COMMAND 1

void byte_code_error(int err) throw(ParserError) {
	char str[50];
	TokenizerPos pos;
	pos.setColumn(-1);
	sprintf(str, "byte code error (code = %d)", err);
	ParserError err_exp(str, pos, NULL);
	throw err_exp;
}

void clear_run() {
	npath = 0;
	done_open = false;
	std::string ss(g_get_type());
	if (strstr(ss.c_str(), "FILLPATH")!=NULL) can_fillpath = true;
	else can_fillpath = false;
	g_drobj.clear();
}

int gle_is_open() {
	return done_open;
}

void error_before_drawing_cmds(const char* name) throw(ParserError) {
	// NOTE: this can be broken by GLEGlobalSource::performUpdates(), which puts includes at front
	string str = name;
	str += " command must appear before drawing commands";
	g_throw_parser_error(str);
}

bool isSingleInstructionLine(int line, int* opcode) {
	int cp = 0;
	int plen = gplen[line];
	if (plen <= 2) {
		// cout << "line: " << line << " opcode comment" << endl;
		*opcode = GLE_KW_COMMENT;
		return true;
	}
	int cmd_plen = gpcode[line][cp++];
	*opcode = gpcode[line][cp++];
	// cout << "line: " << line << " opcode: " << *opcode << endl;
	return cmd_plen >= plen;
}

bool tryHandleChangedPropertiesPrevSet(GLEGlobalSource* source, vector<GLEProperty*>& changed, int line, GLEPropertyStore* props) {
	const string& code = source->getLineCode(line-1);
	GLEParser* parser = get_global_parser();
	Tokenizer* tokens = parser->getTokens();
	parser->setString(code.c_str());
	ostringstream ss;
	ss << "set";
	try {
		tokens->ensure_next_token_i("SET");
		while (tokens->has_more_tokens()) {
			string set_opt = tokens->next_token();
			bool is_changed = false;
			for (vector<GLEProperty*>::size_type i = 0; i < changed.size(); i++) {
				GLEProperty* prop = changed[i];
				const char* prop_name = prop->getSetCommandName();
				if (prop_name != NULL && str_i_equals(set_opt, prop_name)) {
					is_changed = true;
					prop->createSetCommandGLECode(ss, props->getPropertyValue(prop));
					changed.erase(changed.begin()+i);
					break;
				}
			}
			const string& set_value = tokens->next_multilevel_token();
			if (!is_changed) {
				ss << " " << set_opt << " " << set_value;
			}
		}
	} catch (ParserError& err) {
		// to silence ompiler warning...
		string err_str;
		err.toString(err_str);
		return false;
	}
	for (vector<GLEProperty*>::size_type i = 0; i < changed.size(); i++) {
		GLEProperty* prop = changed[i];
		prop->createSetCommandGLECode(ss, props->getPropertyValue(prop));
	}
	source->updateLine(line-1, ss.str());
	return true;
}

void tryDeleteAmove(GLEGlobalSource* source, int line) {
	// FIXME: inconsistency pcode first line = 1, source file first line = 0
	int opcode;
	int next_line = line + 2;
	bool has_next_amove = false;
	/* skip comments and set commands */
	while (next_line < source->getNbLines() && isSingleInstructionLine(next_line, &opcode) && (opcode == GLE_KW_COMMENT || opcode == GLE_KW_SET)) {
		next_line++;
	}
	if (next_line < source->getNbLines() && isSingleInstructionLine(next_line, &opcode) && opcode == GLE_KW_AMOVE) {
		has_next_amove = true;
	}
	/* if has subsequent amove, then OK to delete previous one */
	if (has_next_amove) {
		if (line >= 1 && isSingleInstructionLine(line, &opcode) && opcode == GLE_KW_AMOVE) {
			source->scheduleDeleteLine(line-1);
		}
	}
}

void handleAddAmove(GLEGlobalSource* source, GLEPoint& amove) {
	// FIXME: inconsistency pcode first line = 1, source file first line = 0
	int opcode = -1;
	int line = g_get_error_line();
	int prev_line = line - 1;
	/* Check if not already at this point */
	GLEPoint crpt;
	g_get_xy(&crpt);
	if (crpt.approx(amove)) {
		return;
	}
	/* round point if close to zero */
	if (fabs(amove.getX()) < 1e-10) {
		amove.setX(0.0);
	}
	if (fabs(amove.getY()) < 1e-10) {
		amove.setY(0.0);
	}
	/* create command */
	ostringstream code;
	code << "amove " << amove.getX() << " " << amove.getY();
	/* skip comments and empty lines */
	while (prev_line > 1 && isSingleInstructionLine(prev_line, &opcode) && opcode == GLE_KW_COMMENT) {
		prev_line--;
	}
	/* previous command is amove command */
	if (prev_line >= 1 && isSingleInstructionLine(prev_line, &opcode) && opcode == GLE_KW_AMOVE) {
		source->updateLine(prev_line-1, code.str());
	} else {
		source->scheduleInsertLine(line-1, code.str());
	}
}

void handleChangedProperties(GLEGlobalSource* source, GLEPropertyStore* props) {
	// FIXME: inconsistency pcode first line = 1, source file first line = 0
	vector<GLEProperty*> changed;
	GLEPropertyStoreModel* model = props->getModel();
	for (int i = 0; i < model->getNumberOfProperties(); i++) {
		GLEProperty* prop = model->getProperty(i);
		if (!prop->isEqualToState(props)) {
			prop->updateState(props);
			changed.push_back(prop);
		}
	}
	if (changed.size() != 0) {
		int opcode = -1;
		int line = g_get_error_line();
		int prev_line = line - 1;
		/* skip comments and empty lines */
		while (prev_line > 1 && isSingleInstructionLine(prev_line, &opcode) && opcode == GLE_KW_AMOVE) {
			prev_line--;
		}
		/* previous command is set command */
		bool do_insert = false;
		if (prev_line >= 1 && isSingleInstructionLine(prev_line, &opcode) && opcode == GLE_KW_SET) {
			if (!tryHandleChangedPropertiesPrevSet(source, changed, prev_line, props)) {
				do_insert = true;
			}
		} else {
			do_insert = true;
		}
		if (do_insert) {
			ostringstream ss;
			ss << "set";
			for (vector<GLEProperty*>::size_type i = 0; i < changed.size(); i++) {
				GLEProperty* prop = changed[i];
				prop->createSetCommandGLECode(ss, props->getPropertyValue(prop));
			}
			source->scheduleInsertLine(prev_line, ss.str());
		}
	}
}

void handleNewDrawObject(GLEDrawObject* obj, bool mkdrobjs, GLEPoint* orig = NULL) {
	if (!mkdrobjs) {
		obj->draw();
	} else {
		GLEInterface* iface = GLEGetInterfacePointer();
		GLEScript* script = iface->getScript();
		GLEGlobalSource* source = script->getSource();
		if (iface->isCommitMode()) {
			GLEDrawObject* mobj = script->nextObject();
			if (mobj != NULL && mobj->getType() == obj->getType()) {
				GLEDrawObject* nobj = mobj->deepClone();
				GLEPropertyStore* props = nobj->getProperties();
				nobj->applyTransformation(false);
				handleChangedProperties(source, props);
				if (!obj->approx(nobj)) {
					GLEPoint amove;
					if (nobj->needsAMove(amove)) {
						handleAddAmove(source, amove);
					}
					if (orig != NULL) {
						orig->set(amove);
					}
					if (mobj->modified()) {
						string code;
						nobj->createGLECode(code);
						int line = g_get_error_line()-1;
						source->updateLine(line, code);
					}
				}
				if (mobj->hasFlag(GDO_FLAG_DELETED)) {
					string code;
					int line = g_get_error_line()-1;
					source->updateLine(line, code);
					source->scheduleDeleteLine(line);
					tryDeleteAmove(source, line);
				} else {
					nobj->updateBoundingBox();
				}
				delete nobj;
			}
		} else {
			GLEDrawObject* nobj = obj->deepClone();
			nobj->initProperties(iface);
			nobj->applyTransformation(true);
			script->addObject(nobj);
			obj->updateBoundingBox();
		}
	}
}

GLERun::GLERun(GLEScript* script, GLEFileLocation* outfile, GLEPcodeIndexed* pcode) {
	m_Script = script;
	m_OutFile = outfile;
	m_Vars = getVarsInstance();
	m_CrObj = new GLEObjectRepresention();
	m_stack = new GLEArrayImpl();
	m_blockTypes = 0;
	m_pcode = pcode;
	for (int i = 0; i < GLE_KW_NB; i++) {
		m_AllowBeforeSize[i] = false;
	}
	GLE_MC_INIT(m_returnValue);
	allowBeforeSize(GLE_KW_ASSIGNMENT);
	allowBeforeSize(GLE_KW_BITMAP_INFO);
	allowBeforeSize(GLE_KW_BLANK);
	allowBeforeSize(GLE_KW_CALL);
	allowBeforeSize(GLE_KW_COMMENT);
	allowBeforeSize(GLE_KW_COMPATIBILITY);
	allowBeforeSize(GLE_KW_DECLARESUB);
	allowBeforeSize(GLE_KW_DEFAULT);
	allowBeforeSize(GLE_KW_DEFCOLOR);
	allowBeforeSize(GLE_KW_DEFINE);
	allowBeforeSize(GLE_KW_ELSE);
	allowBeforeSize(GLE_KW_FCLOSE);
	allowBeforeSize(GLE_KW_FGETLINE);
	allowBeforeSize(GLE_KW_FOPEN);
	allowBeforeSize(GLE_KW_FOR);
	allowBeforeSize(GLE_KW_FREAD);
	allowBeforeSize(GLE_KW_FREADLN);
	allowBeforeSize(GLE_KW_FTOKENIZER);
	allowBeforeSize(GLE_KW_FWRITE);
	allowBeforeSize(GLE_KW_FWRITELN);
	allowBeforeSize(GLE_KW_IF);
	allowBeforeSize(GLE_KW_INCLUDE);
	allowBeforeSize(GLE_KW_MARGINS);
	allowBeforeSize(GLE_KW_NEXT);
	allowBeforeSize(GLE_KW_ORIENTATION);
	allowBeforeSize(GLE_KW_PAPERSIZE);
	allowBeforeSize(GLE_KW_PRINT);
	allowBeforeSize(GLE_KW_PSBBTWEAK);
	allowBeforeSize(GLE_KW_PSCOMMENT);
	allowBeforeSize(GLE_KW_RETURN);
	allowBeforeSize(GLE_KW_SIZE);
	allowBeforeSize(GLE_KW_SLEEP);
	allowBeforeSize(GLE_KW_SUB);
	allowBeforeSize(GLE_KW_UNTIL);
	allowBeforeSize(GLE_KW_WHILE);
	allowBeforeSize(GLE_KW_END);
}

GLERun::~GLERun() {
}

GLEPcodeList* GLERun::getPcodeList() {
	return m_pcode->getPcodeList();
}

void GLERun::setDeviceIsOpen(bool open) {
	done_open = open;
}

void GLERun::setBlockTypes(GLEBlocks* blocks) {
	m_blockTypes = blocks;
}

GLEBlocks* GLERun::getBlockTypes() {
	return m_blockTypes;
}

void GLERun::do_pcode(GLESourceLine &sline, int *srclin, int *pcode, int plen, int *pend, bool& mkdrobjs) throw(ParserError) {
/* srclin = The source line number */
/* pcode =  a pointer to the pcode output buffer */
/* plne =   a pointer to the length of the pcode output */
	union {double d; int l; int ll[2];} both;
	int cp=*pend,i,zzcp;
	GLEString* ostr;
	GLERC<GLEString> gstr;
	string temp_str;
	double x,y,ox,oy,x1,y1,x2,y2,x3,y3,a1,a2,r,z,rx,ry;
	int t,j,jj,jj2,ptr,ptr_fill,mask_just,mask_nostroke,marrow;
	static char ss[255];
    string ss1, ss2;
	static bool jump_back = false;
	GLEPoint orig;
	GLERC<GLEColor> colorBackup;
	*pend = 0;
	this_line = *srclin;
	while (cp < plen) {
		int cmd_plen = pcode[cp++];
		int p = pcode[cp++];
		// cout << "pcode = " << p << " cp = " << cp << " cmd_plen = " << cmd_plen << " plen = " << plen << " srclin = " << *srclin << endl;
		// cout << sline.getCode() << endl;
		if (!done_open) {
			//
			// list here all the commands that can come before SIZE
			// see keywords.cpp for more numbers
			//
			// NOTE: this can be broken by GLEGlobalSource::performUpdates(), which puts includes at front
			i = *(pcode + cp);
			if (!isAllowedBeforeSize(p) &&
			    !(p == GLE_KW_BEGIN && i == OP_BEGIN_OBJECT) &&
			    !(p == GLE_KW_BEGIN && i == 22)  // begin config
			) {
				g_open(getOutput(), getSource()->getLocation()->getName());
				done_open = true;
			}
		}
		switch (p) {
		case 53: /* comment */
		case 0: /* blank line */
			break;
		case 65: // PSCOMMENT
			if (done_open) error_before_drawing_cmds("pscomment");
			strcpy(ss,(char *) (pcode+cp));
		/*	readstr(ss); */
			g_pscomment(ss);
			break;
		case 66: // BBTWEAK
			g_psbbtweak();
			break;
		case 1:  /* ALINE x y ARROW both | start | end */
			readval(x);
			readval(y);
			dbg gprint("x=%f, y=%f \n",x,y);
			marrow = *(pcode + (cp++));
			ptr = *(pcode + cp); /* curve angle1 angle2 d1 d2 */
			if (ptr) {
				cp += ptr;
				readxy(x2, y2);
				readxy(x3, y3);
				g_arrowcurve(x, y, marrow, x2, y2, x3, y3);
			} else {
				g_get_xy(&ox,&oy);
				GLELineDO drawobj(ox, oy, x, y);
				drawobj.setArrow((GLEHasArrow)marrow);
				handleNewDrawObject(&drawobj, mkdrobjs);
				if (!mkdrobjs) g_arrowline(x,y,marrow,can_fillpath);
			}
			break;
		  case 2:  /* AMOVE */
			readval(x);
			readval(y);
			g_move(x,y);
			break;
		  case GLE_KW_ABOUND:
			readval(x);
			readval(y);
			g_update_bounds(x,y);
			break;
		  case 73:  /* ASETPOS */
			readval(x);
			readval(y);
			g_set_pos(x,y);
			break;
		  case 81:  /* RSETPOS */
			readval(x);
			readval(y);
			g_rset_pos(x,y);
			break;
		  case 3: /* ARC */
			readval(r);
			readxy(a1,a2);
			g_get_xy(&ox,&oy);
			//
			// -- get options center takes two arguments
			//
			// arrow is first
			marrow = *(pcode + (cp++));
			// center is a type val 2
			ptr = *(pcode + cp); /* cx,cy */
			if (ptr) {
				readvalp(x,pcode + cp + ptr);
				ox+=x;
			}
			ptr = *(pcode + cp + 1); /* cx,cy */
			if (ptr) {
				readvalp(y,pcode + cp + ptr);
				oy+=y;
			}
			if (mkdrobjs) {
				GLEArcDO drawobj(ox, oy, r, a1, a2);
				drawobj.setArrow((GLEHasArrow)marrow);
				handleNewDrawObject(&drawobj, mkdrobjs);
			} else {
				g_arc(r, a1, a2, ox, oy, marrow);
			}
			break;
		  case 4: /* ARCTO */
			readxy(x1,y1);
			readxy(x2,y2);
			readval(r);
			g_get_xy(&ox,&oy);
			g_arcto(x1+ox,y1+oy,x2+ox+x1,y2+oy+y1,r);
			break;
		  case 51: /* Assignment  var=exp */
			readlong(jj);
			getVars()->set(jj, evalGeneric(getStack(), getPcodeList(), pcode, &cp));
			break;
		  case 5:  /* BEGIN box | path | scale | rotate | EXTERNAL */
			g_flush();
			i = *(pcode + cp++);
			switch (i) {
				case 1: /* PATH stroke fill clip */
					npath++;
					g_get_xy(&path_x[npath],&path_y[npath]);
					path_stroke[npath] = *(pcode + cp);
					ptr = *(pcode + ++cp);
					path_fill_backup[npath] = g_get_fill();
					path_clip[npath] = *(pcode + cp + 1);
					if (!path_clip[npath]) {
						path_fill[npath] = g_get_fill();
					} else {
						path_fill[npath] = g_get_fill_clear();
					}
					if (ptr) {
						path_fill[npath] = evalColor(getStack(), getPcodeList(), pcode + cp + ptr, 0);
					}
					cp++;
					g_set_path(true);
					g_newpath();
					break;
				case 2: /* BOX add, fill, nobox, name, round */
					{
						g_drobj.push_back(mkdrobjs);
						GLEStoredBox* box = box_start();
						ptr = *(pcode + cp);
						if (ptr) {
							readvalp(z, pcode+cp+ptr);
							box->setAdd(z);
						}
						ptr = *(pcode + ++cp);
						if (ptr) {
							box->setFill(evalColor(getStack(), getPcodeList(), pcode + cp + ptr, 0));
						}
						if (*(pcode + ++cp)) {
							box->setStroke(false);
						}
						ptr = *(pcode + ++cp);
						if (ptr) {
							readvalp(z,pcode+cp+ptr);
							box->setRound(z);
						}
						ptr = *(pcode + ++cp);
						if (ptr) {
							readstrp(pcode+cp+ptr);
							box->setName(ostr);
						}
						if (box->isFilled() && !g_is_dummy_device()) {
							/* Draw over filled box, measure during first pass */
							box->setDevice(g_set_dummy_device());
							mkdrobjs = false;
						}
					}
					break;
				case 3: /* SCALE */
					readxy(x,y);
					g_gsave();
					g_scale(x,y);
					g_drobj.push_back(mkdrobjs);
					if (x != y) mkdrobjs = false;
					break;
				case 21: /* shear */
					readxy(x,y);
					g_gsave();
					g_shear(x,y);
					g_drobj.push_back(mkdrobjs);
					mkdrobjs = false;
					break;
				case 4: /* ROTATE */
					readval(x);
					g_gsave();
					g_rotate(x);
					g_drobj.push_back(mkdrobjs);
					mkdrobjs = false;
					break;
				case 5: /* TRANSLATE */
					readval(x); readval(y);
					g_gsave();
					g_translate(x,y);
					g_rmove(0.0,0.0);
					break;
				case 8: /* name */
					{
						GLEStoredBox* box = box_start();
						box->setStroke(false);
						box->setName(evalStringPtr(getStack(), getPcodeList(), pcode, &cp));
						ptr = *(pcode + cp);
						if (ptr) {
							readvalp(z, pcode+cp+ptr);
							box->setAdd(z);
						}
					}
					break;
				case 9: /* text */
					z = 0;
					ptr = *(pcode + cp);
					/* read width of text box */
					if (ptr) readvalp(z,pcode+cp+ptr);
					/* get justify of text box */
					t = (int)*(pcode + cp+2);
					begin_text(srclin,pcode,&cp,z,t);
					break;
				case 18: /* tab  (tabbing, table) */
					begin_tab(srclin,pcode,&cp);
					break;
				case GLE_OPBEGIN_KEY:
				case GLE_OPBEGIN_GRAPH:
				case GLE_OPBEGIN_SURF:
					getBlockTypes()->getBlock(i)->beginExecuteBlock(sline, pcode, &cp);
					break;
				case 11: /* xaxis */
				case 12: /* yaxis */
				case 13: /* x2axis */
				case 14: /* y2axis */
					break;
				case 19: /* begin  clip */
					g_beginclip();
					break;
				case 17: /* ORIGIN */
					g_gsave();
					g_get_xy(&x,&y);
					g_translate(x,y);
					g_move(0.0,0.0);
					break;
				case 22: /* config */
					ostr = evalStringPtr(getStack(), getPcodeList(), pcode, &cp);
					begin_config(ostr->toUTF8(), srclin, pcode, &cp);
					break;
				case 23: /* tex preamble */
					begin_tex_preamble(srclin,pcode,&cp);
					break;
				case 25: /* letz */
					begin_letz(srclin, getPcodeList(), pcode, &cp);
					break;
				case 26: /* fitz */
					begin_fitz(srclin,pcode,&cp);
					break;
				case 27: /* fit */
					// begin_fitls(srclin,pcode,&cp);
					break;
				case 28: /* contour */
					begin_contour(srclin,pcode,&cp);
					break;
				case 29: /* tex */
					begin_tex(this, srclin, pcode, &cp);
					break;
				case OP_BEGIN_OBJECT:
					readlong(jj);
					if (jj == 0) {
						/* dynamic object inside sub */
						if (!done_open) {
							g_open(getOutput(), getSource()->getLocation()->getName());
							done_open = true;
						}
						readlong(jj);
						ostr = evalStringPtr(getStack(), getPcodeList(), pcode, &cp);
						begin_object(ostr->toUTF8(), getSubroutines()->get(jj));
					} else {
						/* statical object - identical to sub */
						readlong(jj);
						sub_get_startend(jj,&i,&j);
						*srclin = j;	/* skip past the subroutine */
					}
					break;
				case OP_BEGIN_LENGTH:
					readlong(jj);
					begin_length(jj);
					break;
				default: /* error  */
					g_throw_parser_error("illegal begin option code: ", i);
					break;
			}
			break;
		  case 6: /* BEZIER */
			readxy(x1,y1);
			readxy(x2,y2);
			readxy(x3,y3);
			g_bezier(x1,y1,x2,y2,x3,y3);
			break;
		  case 7:  /* BOX x y justify FILL fexp NAME string ROUND val NOSTROKE */
	  		{
				GLEBox box;
				readval(x); readval(y);
				g_get_xy(&ox,&oy);
				x += ox; y += oy;
				mask_just = *(pcode + cp);
				g_dojust(&ox,&oy,&x,&y,mask_just);
				if (g_is_filled()) {
					box.setFill(g_get_fill());
				}
				ptr = *(pcode + ++cp);
				if (ptr) {
					box.setFill(evalColor(getStack(), getPcodeList(), pcode + cp + ptr, 0));
				}
				if (*(pcode + ++cp)) {
					box.setStroke(false);
				}
				if (*(pcode + ++cp)) {
					box.setReverse(true);
				}
				ptr = *(pcode + ++cp);
				if (ptr) {
					readvalp(z,pcode+cp+ptr);
					box.setRound(z);
				}
				ptr = *(pcode + ++cp);
				if (ptr) {
					readstrp(pcode+cp+ptr);
					box.setName(ostr);
				}
				box.draw(this, ox, oy, x, y);
			}
			break;
		  case 52:  /* CALL or @ */
			evalGeneric(getStack(), getPcodeList(), pcode, &cp);
			break;
		  case 8:  /* CIRCLE */
			readval(r);
			g_get_xy(&orig);
			ox = orig.getX();
			oy = orig.getY();
			mask_just = *(pcode + cp++);
			x = ox + r;
			y = oy + r;
			g_dojust(&ox,&oy,&x,&y,mask_just);
			g_move(ox,oy);
			mask_nostroke = *(pcode + cp++);
			colorBackup = g_get_fill();
			ptr_fill = *(pcode + cp);
			if (ptr_fill) {
				g_set_fill(evalColor(getStack(), getPcodeList(), pcode + cp + ptr_fill, 0));
			}
			if (mkdrobjs) {
				GLEEllipseDO drawobj(ox, oy, r);
				handleNewDrawObject(&drawobj, mkdrobjs, &orig);
			} else {
				if (g_is_filled()) g_circle_fill(r);
				if (!mask_nostroke) g_circle_stroke(r);
			}
			g_set_fill(colorBackup);
			g_move(orig);
			break;
		case 70:  /* ELLIPSE */
			readval(rx);  //x radius
			readval(ry);  //y radius
			g_get_xy(&orig);
			ox = orig.getX();
			oy = orig.getY();
			mask_just = *(pcode + cp++);
			x = ox + rx;
			y = oy + ry;
			g_dojust(&ox,&oy,&x,&y,mask_just);
			g_move(ox,oy);
			colorBackup = g_get_fill();
			mask_nostroke = *(pcode + cp++);
			ptr_fill = *(pcode + cp);
			if (ptr_fill) {
				g_set_fill(evalColor(getStack(), getPcodeList(), pcode + cp + ptr_fill, 0));
			}
			if (mkdrobjs) {
				GLEEllipseDO drawobj(ox, oy, rx, ry);
				handleNewDrawObject(&drawobj, mkdrobjs, &orig);
			} else {
				if (g_is_filled()) g_ellipse_fill(rx,ry);
				if (!mask_nostroke) g_ellipse_stroke(rx,ry);
			}
			g_set_fill(colorBackup);
			g_move(orig);
			break;
		case 71: /* ELLIPTICAL_ARC  */
			readxy(rx,ry);
			readxy(a1,a2);
			g_get_xy(&ox,&oy);
			//
			// -- get options center takes two arguments
			//
			// arrow is first
			marrow = *(pcode + (cp++));
			// center is a type val 2
			ptr = *(pcode + cp); /* cx,cy */
			if (ptr) {
				readvalp(x,pcode + cp + ptr);
				ox+=x;
			}
			ptr = *(pcode + cp + 1); /* cx,cy */
			if (ptr) {
				readvalp(y,pcode + cp + ptr);
				oy+=y;
			}
			g_elliptical_arc(rx, ry, a1, a2, ox, oy, marrow);
			break;
		case 72: /* ELLIPTICAL_NARC  */
			readxy(rx,ry);
			readxy(a1,a2);
			g_get_xy(&ox,&oy);
			//
			// -- get options center takes two arguments
			//
			// arrow is first
			marrow = *(pcode + (cp++));
			// center is a type val 2
			ptr = *(pcode + cp); /* cx,cy */
			if (ptr) {
				readvalp(x,pcode + cp + ptr);
				ox+=x;
			}
			ptr = *(pcode + cp + 1); /* cx,cy */
			if (ptr) {
				readvalp(y,pcode + cp + ptr);
				oy+=y;
			}
			g_elliptical_narc(rx, ry, a1, a2, ox, oy, marrow);
			break;
		  case 9: /* CLOSEPATH */
			g_closepath();
			break;
		  case 10: /* CURVE  x y x y ...  change to BEGIN CURVE ... END CURVE */
			g_curve(getPcodeList(), pcode + cp);
			break;
		  case 11: /* DEFINE  MARKER name  subname */
			break;
		  case 12: /* DFONT */
			ostr = evalStringPtr(getStack(), getPcodeList(), pcode, &cp);
			g_dfont(ostr->toUTF8());
			break;
		  case 14: /* END */
			readlong(jj);
			switch (jj) {
			  case 1: /* end path  (stroke,fill,clip) */
				if (!path_fill[npath]->isTransparent()) {
					g_set_fill(path_fill[npath]);
					g_fill();
				}
				if (path_stroke[npath] == static_cast<int>(true)) g_stroke();
				if (path_clip[npath] == static_cast<int>(true)) g_clip();
				if (npath==0) {
					g_throw_parser_error("too many end path's");
					break;
				}
				g_move(path_x[npath],path_y[npath]);
				g_set_path(false);
				g_set_fill(path_fill_backup[npath]);
				npath--;
				break;
			  case 2: /* end box */
				cp++;
				readlong(jj);   /* jump to line */
				if (!last_box()->isSecondPass() && g_drobj.size() != 0) {
					mkdrobjs = g_drobj.back();
					g_drobj.pop_back();
				}
				if (box_end()) {
					*pend = 0;
					*srclin = jj;
				}
				break;
			  case 3:  /* end scale */
			  case 21: /* end shear */
			  case 4:  /* end rotate */
				if (g_drobj.size() != 0) {
					mkdrobjs = g_drobj.back();
					g_drobj.pop_back();
				}
				g_grestore();
				break;
			  case 5:  /* end translate */
				g_grestore();
				break;
			  case 6: /* end if */
				/* do nothing,  all done elsewhere I think?? */
				break;
			  case 8: /* end name */
				box_end();
				break;
			  case 19: /* clip */
				g_endclip();
				break;
			  case 18: /* tab */
			  case 9: /* text */
				break;
			  case 17: /* end origin */
				g_grestore();
				break;
			  case OP_BEGIN_OBJECT:
				cp++;
				end_object();
				break;
			  case GLE_OPBEGIN_KEY:
			  case GLE_OPBEGIN_GRAPH:
			  case GLE_OPBEGIN_SURF:
				getBlockTypes()->getBlock(jj)->endExecuteBlock();
				break;
			  case OP_BEGIN_LENGTH:
				end_length();
			    break;
			  default :
				get_global_parser()->get_block_type(jj, temp_str);
				g_throw_parser_error("invalid end of block type '", temp_str.c_str(), "'");
			}
			break;
		  case 15: /* FCLOSE */
			readval(x);
			f_close_chan((int) x);
			break;
		  case 16: /* FILL */
			g_fill();
			break;
		  case 61 : /* fread CHAN a$ x   */
		  case 62 : /* freadln */
	  		{
				readlong(t);
				if (t!=49) gprint("FREAD, PCODE ERROR, %d  cp %d plen %d\n",t,cp,plen);
				readlong(i);
				readlong(t);
				var_get(i,&x);
				chn = (int) x;
				chn = f_testchan(chn);
				if (chn == -1) break;
				GLEFile* file = g_Files[chn];
				if (p == 61 && cp >= cmd_plen) {
					gprint("FREAD requires at least two parameters\n");
					break;
				}
				while (cp < cmd_plen) {
					readlong(t);
					if (t!=49) gprint("FREAD2, PCODE ERROR, %d  cp %d plen %d\n",t,cp,plen);
					readlong(i); /* variable number */
					readlong(t); /* type of variable */
					if (t==1) {
						x = atof(file->getToken());
						var_set(i,x);
					} else {
						var_setstr(i,file->getToken());
					}
				}
				if (p==62) file->gotoNewLine();
			} break;
		  case 63 : /* fwrite */
		  case 64 : /* fwriteln */
	  		{
				readlong(t);
				readlong(t);
				readval(x);
				chn = f_testchan((int) x);
				if (chn == -1) break;
				GLEFile* file = g_Files[chn];
				if (file->isRead()) {
					g_throw_parser_error("can't write to file opened in read mode");
				}
				temp_str = "";
				while (cp < cmd_plen) {
					readlong(t);
					if (t!=49) gprint("WRITE, PCODE ERROR, %d  cp %d plen %d\n",t,cp,plen);
					readlong(t);
					gstr = evalString(getStack(), getPcodeList(), pcode, &cp, true);
					if (!temp_str.empty()) {
						temp_str += " ";
					}
					temp_str += gstr->toUTF8();
				}
				if (p==64) temp_str += "\n";
				fprintf(file->getOutput(),"%s",temp_str.c_str());
			} break;
		  case 17:  /* FOPEN "a.a" inchan read|write */
			readstr(temp_str);
			readlong(i); /* channel variable */
			readlong(jj); /* 0 = read, 1 = write */
			f_create_chan(i, temp_str.c_str(), jj);
			break;
		  case 75 : /* fgetline */
			{
				readval(x);
				chn = f_testchan((int) x);
				if (chn == -1) break;
				readlong(i);
				var_setstr(i, g_Files[chn]->readLine());
			} break;
		  case 76 : /* ftokenizer commenttoks spacetoks singlechartoks */
			{
				readval(x);
				chn = f_testchan((int) x);
				if (chn == -1) break;
				GLEFile* file = g_Files[chn];
				file->resetLang();
				ostr = evalStringPtr(getStack(), getPcodeList(), pcode, &cp);
				file->setCommentChars(ostr->toUTF8());
				ostr = evalStringPtr(getStack(), getPcodeList(), pcode, &cp);
				file->setSpaceTokens(ostr->toUTF8());
				ostr = evalStringPtr(getStack(), getPcodeList(), pcode, &cp);
				file->setSingleCharTokens(ostr->toUTF8());
			} break;
		  case 77: /* papersize */
  			if (done_open) error_before_drawing_cmds("papersize");
		  	readlong(jj);
			if (jj == 1) {
				readlong(jj);
				g_set_pagesize(jj);
			} else {
				readxy(x2, y2);
				g_set_pagesize(x2, y2);
			}
		  	break;
		  case 78: /* margins */
 			if (done_open) error_before_drawing_cmds("margins");
			readxy(x2, y2);
			readxy(x3, y3);
			g_set_margins(x2, y2, x3, y3);
		  	break;
		  case 79: /* orientation */
  			if (done_open) error_before_drawing_cmds("orientation");
	  	  	readlong(jj);
			g_set_landscape(jj);
			break;
		  case 18: /* FOR   v,exp,exp,op,exp */
			readlong(jj);        /* variable name */
			readlong(jj2);       /* jump address  */
			readval(x);          /* to value      */
			ptr = *(pcode + cp); /* step value    */
			if (ptr) {
				readvalp(z, pcode + cp + ptr);
			} else {
				z = 1;
			}
			var_get(jj, &y);
			if (jump_back) {
				jump_back = false;
				/* increment loop variable y by step value z */
				y += z;
				var_set(jj, y);
			}
			if ((z >= 0 && y > x) || (z < 0 && y < x)) {
				/* jump to line after next */
				*srclin = jj2;
			}
			break;
		  case 19: /* GOTO */
		  case 20: /* GSAVE */
			g_gsave();
			break;
		  case 54: /* GRESTORE */
			g_grestore();
			break;
		  case 21: /* ICON */
			break;
		  case 22: /* IF EXP */
		  	{
		  		bool ifValue = evalBool(getStack(), getPcodeList(), pcode, &cp);
				readlong(jj);   /* jump to line */
				readlong(jj2);  /* jump pcode offset */
				if (!ifValue) {
					*pend = jj2;
					*srclin = jj-1;
					return;
				}
		  	}
			break;
		  case 23: /* INCLUDE (done in pass,  already included) */
			break;
		  case 24: /* INPUT */
		  	break;
		  case 25: /* JOIN  str1,type,str2 */
			{
				GLERC<GLEString> s1(evalStringPtr(getStack(), getPcodeList(), pcode, &cp));
				readlong(jj);
				GLERC<GLEString> s2(evalStringPtr(getStack(), getPcodeList(), pcode, &cp));
				ptr = *(pcode + cp); /* curve angle1 angle2 d1 d2 */
				if (ptr) {
					cp += ptr;
					readxy(x2, y2);
					readxy(x3, y3);
					name_join(s1.get(), s2.get(), (int)jj, x2, y2, x3, y3);
				} else {
					name_join(s1.get(), s2.get(), (int)jj, 0, 0, 0, 0);
				}
			}
			break;
		  case 26: /* MARKER */
			readval(x);
			memcpy(&both.d,&x,sizeof(x));
			jj = both.l;
			g_get_hei(&z);
			y = 1;
			if (*(pcode+cp)!=0) readval(y);
			y = y * z;
			g_marker((int) both.l,y);
			break;
		  case 27: /* MOVE  name */
		  	{
		  		GLEPoint pt;
		  		ostr = evalStringPtr(getStack(), getPcodeList(), pcode, &cp);
		  		name_to_point(ostr, &pt);
		  		g_move(pt);
		  		break;
		  	}
		  case 28: /* NARC */
			readval(r);
			readxy(a1,a2);
			g_get_xy(&ox,&oy);
			//
			// -- get options center takes two arguments
			//
			// arrow is first
			marrow = *(pcode + (cp++));
			// center is a type val 2 shifts the center position
			ptr = *(pcode + cp); /* cx,cy */
			if (ptr) {
				readvalp(x,pcode + cp + ptr);
				ox+=x;
			}
			ptr = *(pcode + cp + 1); /* cx,cy */
			if (ptr) {
				readvalp(y,pcode + cp + ptr);
				oy+=y;
			}
			g_narc(r, a1, a2, ox, oy, marrow);
			break;
		  case 29: /* NEWPATH */
			g_newpath();
			break;
		  case 13:  /* ELSE */
			readlong(jj);   /* jump to line */
			readlong(jj2);  /* jump pcode offset */
			*pend = jj2;
			*srclin = jj-1;
			// do not execute any further commands on this line: return!
		  	return;
		  case 30:  /* NEXT */
			readlong(jj);   /* jump to line */
			readlong(jj2);  /* jump pcode offset */
			*pend = jj2;
			*srclin = jj-1;
			jump_back = true;
			break;
		  case 31: /* PIE ,, not implemented yet */
			break;
		  case 58: /* bigfile "filename" */
		  printf("BIGFILE no longer supported, sorry. use include");
	//		readstr(ss);
	//		gle_strlwr(ss);		/* bit of a kludge but ... */
	//		run_bigfile(ss);
			break;
		  case 55: /* Postscript filename x y */
			readstr(temp_str);
			readxy(x1,y1);
			g_postscript((char*)temp_str.c_str(), x1, y1);
			break;
		  case 67: /* TIFF filename x y */
			readstr(temp_str);
			readxy(x1,y1);
			g_bitmap(temp_str, x1, y1, BITMAP_TYPE_TIFF);
			break;
		  case 68: /* BITMAP file width height [type colors compress dpi greyscale resize] */
			{
				readstr(temp_str);
				readxy(x1,y1);
	// The options type, compress, dpi, greyscale, ...
	// will be implemented in the near future (Jan Struyf 01//05/05).
				cp += 5;
				int bm_type = 0;
				ptr = *(pcode + cp); /* type */
				if (ptr) {
					readstrp(pcode + cp + ptr);
					bm_type = g_bitmap_string_to_type(ostr->toUTF8());
				}
				g_bitmap(temp_str, x1, y1, bm_type);
			}
			break;
		  case 69: /* BITMAP_INFO file width, height [type] */
	  		{
				readstr(temp_str);
				readlong(jj); readlong(jj2);
				int bm_type = 0;
				ptr = *(pcode + cp); /* type */
				if (ptr) {
					readstrp(pcode + cp + ptr);
					bm_type = g_bitmap_string_to_type(ostr->toUTF8());
				}
				g_bitmap_info(temp_str, jj, jj2, bm_type);
			}
			break;
		  case GLE_KW_COLORMAP:
		  	{
				GLEColorMap map;
				GLEToRectangularView view;
				ostr = evalStringPtr(getStack(), getPcodeList(), pcode, &cp);
				map.setFunction(ostr->toUTF8());
				readxy(x1, y1);
				view.setXRange(x1, y1);
				readxy(x1, y1);
				view.setYRange(x1, y1);
				readxy(x1, y1);
				map.setWidth((int)floor(x1+0.5));
				map.setHeight((int)floor(y1+0.5));
				readxy(x1, y1);
				ptr = *(pcode + cp); /* color */
				if (ptr) {
					map.setColor(true);
				}
				ptr = *(pcode + ++cp); /* palette */
				if (ptr) {
					readstrp(pcode + cp + ptr);
					temp_str = ostr->toUTF8();
					// because palette can be subroutine name!
					str_to_uppercase(temp_str);
					map.setPalette(temp_str.c_str());
				}
				g_get_xy(&ox,&oy);
				view.setOrigin(GLEPoint(ox, oy));
				view.setSize(GLEPoint(x1, y1));
				map.draw(&view, ox, oy, x1, y1);
			}
			break;
		  case 33: /* RBEZIER */
			readxy(x1,y1);
			readxy(x2,y2);
			readxy(x3,y3);
			g_get_xy(&ox,&oy);
			x1 += ox;  x2 += ox;  x3 += ox;
			y1 += oy;  y2 += oy;  y3 += oy;
			g_bezier(x1,y1,x2,y2,x3,y3);
			break;
		  case 34: /* REGION */
			break;
		  case 50: /* RETURN exp */
			GLE_MC_COPY(&m_returnValue, evalGeneric(getStack(), getPcodeList(), pcode, &cp));
			readlong(jj);   /* jump to line */
			*srclin = jj-1;
			break;
		  case 35: /* REVERSE */
			g_reverse();
			break;
		  case 36:  /* RLINE */
			readval(x);
			readval(y);
			g_get_xy(&ox,&oy);
			marrow = *(pcode + (cp++));
			ptr = *(pcode + cp); /* curve angle1 angle2 d1 d2 */
			if (ptr) {
				cp += ptr;
				readxy(x2, y2);
				readxy(x3, y3);
				g_arrowcurve(x+ox, y+oy, marrow, x2, y2, x3, y3);
			} else {
				GLELineDO drawobj(ox, oy, ox+x, oy+y);
				drawobj.setArrow((GLEHasArrow)marrow);
				handleNewDrawObject(&drawobj, mkdrobjs);
				if (!mkdrobjs) g_arrowline(ox+x, oy+y, marrow, can_fillpath);
			}
			break;
		  case 37:  /* RMOVE */
			readval(x);
			readval(y);
			g_get_xy(&ox,&oy);
			g_move(x+ox,y+oy);
			break;
		  case 38: /* ROTATE */
			readval(x);
			g_rotate(x);
			break;
		  case 39: /* SAVE  name */
			g_get_xy(&x,&y);
			ostr = evalStringPtr(getStack(), getPcodeList(), pcode, &cp);
			name_set(ostr,x,y,x,y);
			break;
		  case 40: /* SCALE */
			readxy(x,y);
			g_scale(x,y);
			break;
		  case GLE_KW_SET: /* SET */
			while (cp < cmd_plen) {
				cp++;
				int set_cmd = *(pcode+cp-1)-500;
				dbg gprint("set sub command %d \n",set_cmd);
				switch (set_cmd) {
				  case 1: /* height */
					readval(x);
					g_set_hei(x);
					break;
				  case 2: /* font */
					readval(x);
					memcpy(&both.l,&x,4);
					g_set_font(both.l);
					break;
				  case 3: /* justify */
					readval(x);
					memcpy(&both.l,&x,4);
					g_set_just(both.l);
					break;
				  case 4: /* color */
					g_set_color(evalColor(getStack(), getPcodeList(), pcode, &cp));
					break;
				  case OP_SET_BACKGROUND: /* background */
					g_set_background(evalColor(getStack(), getPcodeList(), pcode, &cp));
					break;
				  case OP_SET_FILL: /* fill */
					g_set_fill(evalColor(getStack(), getPcodeList(), pcode, &cp));
					break;
				  case OP_SET_FILL_PATTERN: /* fill pattern */
					g_set_fill_pattern(evalColor(getStack(), getPcodeList(), pcode, &cp));
					break;
				  case 5: /* dashlen */
					readval(x);
					g_set_line_styled(x);
					break;
				  case 6: /* dash */
					readval(x);
					i = (int) x;
					sprintf(ss,"%d",i);
					g_set_line_style(ss);
					break;
				  case 7: /* lwidth */
					readval(x);
					g_set_line_width(x);
					break;
				  case 10: /* fontlwidth */
					readval(x);
					g_set_font_width(x);
					break;
				  case 8: /* join */
					readlong(jj);
					g_set_line_join(jj);
					break;
				  case 9: /* cap */
					readlong(jj);
					g_set_line_cap(jj);
					break;
				  case OP_SET_FILL_METHOD:
					readstr(ss1);
				  	g_set_fill_method(ss1.c_str());
				  	break;
				  case OP_SET_ARROW_STYLE:
					readstr(ss1);
				  	g_set_arrow_style(ss1.c_str());
				  	break;
				  case OP_SET_ARROW_TIP:
					readstr(ss1);
					g_set_arrow_tip(ss1.c_str());
					break;
				  case OP_SET_ARROW_SIZE:
				  	readval(x);
					g_set_arrow_size(x);
					break;
				  case OP_SET_ARROW_ANGLE:
					readval(x);
					g_set_arrow_angle(x);
					break;
				  case OP_SET_IMAGE_FORMAT:
					readstr(ss1);
					g_set_pdf_image_format(ss1.c_str());
					break;
				  case OP_SET_TEX_SCALE:
					readstr(ss1);
					g_set_tex_scale(ss1.c_str());
					break;
				  case OP_SET_TEX_LABELS:
				  	readval(x);
				  	g_set_tex_labels((int)floor(x+0.5));
					break;
				  case OP_SET_TITLE_SCALE:
				  case OP_SET_ATITLE_SCALE:
				  case OP_SET_ALABEL_SCALE:
				  case OP_SET_TICKS_SCALE:
				  case OP_SET_ATITLE_DIST:
				  case OP_SET_ALABEL_DIST:
				  	readval(x);
				  	g_set_fconst(set_cmd-OP_SET_TITLE_SCALE+GLEC_TITLESCALE, x);
					break;
				  default :
					gprint("Not a valid set sub command {%d} i=%d \n",*(pcode+i),i);
				}
			}
			break;
		  case 42: /* size x y [box]*/
			if (done_open) error_before_drawing_cmds("size");
			readxy(x,y);
			g_set_size(x, y, *(pcode + cp++));
			break;
		  case 43: /* STROKE */
			g_stroke();
			break;
		  case 44: /* SUB */
			readlong(jj);
			sub_get_startend(jj,&i,&j);
			*srclin = j;	/* skip past the subroutine */
			break;
		  case 45: /* TEXT */
			temp_str = (char *) (pcode+cp);
			if (mkdrobjs) {
				g_get_xy(&orig);
				GLETextDO drawobj(orig, temp_str);
				handleNewDrawObject(&drawobj, mkdrobjs);
			} else {
				g_text(temp_str);
			}
			break;
		  case 60: /* DEFMARKER */
			break;
		  case 56: /* DRAW */
			readlong(jj);
			if (jj == 0) {
				// draw static object
				string path;
				string name;
				readstr(path);
				readstr(name);
				draw_object_static(path, name, pcode, &cp, mkdrobjs);
			} else {
				// draw dynamic object (as before?)
				readstr(temp_str);
				ptr = *(pcode + ++cp); /* name */
				if (ptr) {
					readstrp(pcode + cp + ptr);
					string obj_name = ostr->toUTF8();
					draw_object(temp_str, obj_name.c_str());
				} else {
					draw_object(temp_str, NULL);
				}
			}
			break;
		  case 59: /* TEXTDEF */
			strcpy(ss,(char *) (pcode+cp));
			text_def((unsigned char*) ss);
			break;
		  case 46: /* TRANSLATE */
			readxy(x,y);
			g_translate(x,y);
			break;
		  case 47: /* UNTIL */
			readval(x);
			readlong(jj);
			jump_back = false;
			if (x) *srclin = jj;
			break;
		  case 48: /* WHILE */
		    {
				bool ifValue = evalBool(getStack(), getPcodeList(), pcode, &cp);
				readlong(jj);
				jump_back = false;
				if (!ifValue) *srclin = jj;
		  	}
			break;
		  case 32: /* PRINT */
		  case 49: /* WRITE */
			g_get_xy(&orig);
			temp_str = "";
			while (cp < cmd_plen) {
				readlong(t);
				if (t != 49 && t != 32) {
					g_throw_parser_error("pcode error in print/write");
				}
				readlong(t);
				gstr = evalString(getStack(), getPcodeList(), pcode, &cp, true);
				if (!temp_str.empty()) {
					temp_str += " ";
				}
				temp_str += gstr->toUTF8();
			}
			if (p == 49) {
				/* WRITE */
				if (g_get_tex_labels()) {
					TeXInterface::getInstance()->draw(temp_str.c_str());
				} else {
					if (mkdrobjs) {
						GLETextDO drawobj(orig, temp_str);
						handleNewDrawObject(&drawobj, mkdrobjs, &orig);
					} else {
						g_text(temp_str);
					}
				}
				g_move(orig);
			} else {
				/* PRINT */
				g_message(temp_str);
			}
			break;
		  case 74: /* TeX */
			{
				x = 0.0;
				GLERectangle box;
				ostr = evalStringPtr(getStack(), getPcodeList(), pcode, &cp);
				TeXInterface::getInstance()->draw(ostr->toUTF8(), &box);
				ptr = *(pcode + cp); /* add */
				if (ptr) {
					readvalp(x, pcode + cp + ptr);
				}
				ptr = *(pcode + ++cp); /* name */
				if (ptr) {
					readstrp(pcode + cp + ptr);
					box.getDimensions(&x1,&y1,&x2,&y2);
					x1 -= x; x2 += x; y1 -= x; y2 += x;
					name_set(ostr,x1,y1,x2,y2);
				}
			}
			break;
		  case GLE_KW_RESTOREDEFAULTS:
			g_restore_defaults();
			break;
		  case GLE_KW_SLEEP:
			readval(x);
			GLESleep((int)floor(x*1000+0.5));
			break;
		  case GLE_KW_BLOCK_COMMAND:
			readlong(jj); /* block type */
			getBlockTypes()->getBlock(jj)->executeLine(sline);
			break;
		  default :
		  	byte_code_error(PCODE_UNKNOWN_COMMAND);
		}
		// begin can't be combined with other commands
		if (p == 5) break;
		cp = cmd_plen;
	}
}

GLEBox::GLEBox():
	m_HasStroke(true),
	m_HasReverse(false),
	m_Add(0.0),
	m_IsRound(false),
	m_Round(0.0),
	m_Fill(g_get_fill_clear())
{
}

void GLEBox::setFill(const GLERC<GLEColor>& fill) {
	m_Fill = fill;
}

void GLEBox::setRound(double round) {
	if (round == 0.0) {
		m_IsRound = false;
	} else {
		m_IsRound = true;
		m_Round = round;
	}
}

void do_arcto(double x1, double y1, double x2, double y2, double r) {
	double ox, oy;
	g_get_xy(&ox,&oy);
	g_arcto(x1+ox,y1+oy,x2+ox+x1,y2+oy+y1,r);
}

void GLEBox::draw(GLERun* run, double x1, double y1, double x2, double y2) {
	double ox, oy;
	GLERectangle rect(x1, y1, x2, y2);
	rect.normalize();
	rect.grow(getAdd());
	g_get_xy(&ox, &oy);
	GLERC<GLEColor> cur_fill(g_get_fill());
	if (isRound()) {
		int oldjoin;
		g_get_line_join(&oldjoin);
		g_set_line_join(1);
		g_set_path(true);
		g_newpath();
		g_move(rect.getXMin()+getRound(), rect.getYMax());
		g_arcto(rect.getXMin(), rect.getYMax(), rect.getXMin(), rect.getYMax()-getRound(), getRound());
		g_line(rect.getXMin(), rect.getYMin()+getRound());
		g_arcto(rect.getXMin(), rect.getYMin(), rect.getXMin()+getRound(), rect.getYMin(), getRound());
		g_line(rect.getXMax()-getRound(), rect.getYMin());
		g_arcto(rect.getXMax(), rect.getYMin(), rect.getXMax(), rect.getYMin()+getRound(), getRound());
		g_line(rect.getXMax(), rect.getYMax()-getRound());
		g_arcto(rect.getXMax(), rect.getYMax(), rect.getXMax()-getRound(), rect.getYMax(), getRound());
		g_closepath();
		if (isFilled()) {
			g_set_fill(getFill());
			g_fill();
		}
		if (hasStroke()) {
			g_stroke();
		}
		g_set_path(false);
		g_set_line_join(oldjoin);
	} else {
		if (isFilled()) {
			g_set_fill(getFill());
			g_box_fill(&rect);
		}
		if (hasStroke()) {
			g_box_stroke(&rect, hasReverse());
		}
	}
	g_set_fill(cur_fill);
	if (hasName()) {
		run->name_set(getName(), rect.getXMin(), rect.getYMin(), rect.getXMax(), rect.getYMax());
	}
	g_move(ox, oy);
}

GLEStoredBox::GLEStoredBox():
	m_SecondPass(false),
	m_Device(0)
{
}

GLEStoredBox* box_start() {
	double ox, oy;
	GLEStoredBox* box = GLEBoxStack::getInstance()->newBox();
	g_get_xy(&ox, &oy);
	box->setOrigin(ox, oy);
	g_get_bounds(box->getSaveBounds());
	g_init_bounds();
	return box;
}

GLEStoredBox* GLERun::last_box() throw (ParserError) {
	GLEBoxStack* stack = GLEBoxStack::getInstance();
	if (stack->size() <= 0) {
		g_throw_parser_error("too many end boxes");
	}
	return stack->lastBox();
}

bool GLERun::box_end() throw (ParserError) {
	double x1, y1, x2, y2;
	GLEBoxStack* stack = GLEBoxStack::getInstance();
	if (stack->size() <= 0) {
		g_throw_parser_error("too many end boxes");
	}
	g_get_bounds(&x1, &y1, &x2, &y2);
	if (x1 > (x2+100)) {
		ostringstream err;
		err << "empty box (bounds are " << x1 << "," << y1 << " x "<< x2 << "," << y2 << ")?" << endl;
		g_throw_parser_error(err.str());
	}
	GLEStoredBox* box = stack->lastBox();
	if (box->isSecondPass()) {
		stack->removeBox();
		return false;
	}
	if (box->getDevice() != NULL) {
		g_restore_device(box->getDevice());
	}
	box->setName(box->hasName() ? box->getName() : NULL);
	box->draw(this, x1, y1, x2, y2);
	if (box->getSaveBounds()->isValid()) {
		g_update_bounds(box->getSaveBounds());
	}
	if (box->getDevice() != NULL) {
		box->setSecondPass(true);
		g_move(box->getOrigin());
		return true;
	} else {
		stack->removeBox();
		return false;
	}
}

void nm_adjust(GLEJustify jj, double *sx, double *sy, double ex, double ey, GLERectangle* r);

void GLERun::name_set(GLEString* name, double x1, double y1, double x2, double y2) {
	GLERC<GLEObjectRepresention> obj(new GLEObjectRepresention());
	obj->getRectangle()->setDimensions(x1, y1, x2, y2);
	g_dev(obj->getRectangle());
	if (!getCRObjectRep()->setChildObject(name, obj.get())) {
		char ostr[80];
		int idx, type;
		name->toUTF8(ostr);
		getVars()->findAdd(ostr, &idx, &type);
		getVars()->setObject(idx, obj.get());
	}
}

bool GLERun::is_name(GLEObjectRepresention* obj, GLEArrayImpl* path, unsigned int offs) {
	/* check for just object name */
	unsigned int size = path->size();
	if (size <= offs) {
		return true;
	}
	/* traverse path */
	for (unsigned int i = offs; i < size; i++) {
		GLEString* elem = (GLEString*)path->getObjectUnsafe(i);
		GLEObjectRepresention* nextobj = obj->getChildObject(elem);
		if (nextobj != NULL) {
			obj = nextobj;
		} else {
			return false;
		}
	}
	return true;
}

GLEObjectRepresention* GLERun::name_to_object(GLEObjectRepresention* obj, GLEArrayImpl* path, GLEJustify* just, unsigned int offs) throw (ParserError) {
	/* check for just object name */
	unsigned int size = path->size();
	if (size <= offs) {
		*just = GLEJusitfyBox;
		return obj;
	}
	/* traverse path */
	for (unsigned int i = offs; i < size-1; i++) {
		GLEString* elem = (GLEString*)path->getObjectUnsafe(i);
		GLEObjectRepresention* nextobj = obj->getChildObject(elem);
		if (nextobj != NULL) {
			obj = nextobj;
		} else {
			ostringstream str;
			GLEStringHash* childs = obj->getChilds();
			if (childs == NULL) {
				str << "object does not contain name '" << *elem << "'";
			} else {
				GLEArrayImpl keys;
				childs->getKeys(&keys);
				str << "object does not contain name '" << *elem << "'; ";
				if (keys.size() == 0) {
					str << "no available names";
				} else {
					str << "available names:" << endl;
					keys.enumStrings(str);
				}
			}
			g_throw_parser_error(str.str());
		}
	}
	/* should now be justify specification? */
	GLEString* last_elem = (GLEString*)path->getObjectUnsafe(size-1);
	GLEObjectRepresention* last_obj = obj->getChildObject(last_elem);
	if (last_obj == NULL) {
		char str[80];
		last_elem->toUTF8(str);
		if (!gt_firstval_err(op_justify, str, (int*)just)) {
			ostringstream str;
			GLEStringHash* childs = obj->getChilds();
			if (childs == NULL) {
				str << "'" << *last_elem << "' is not a valid justify option (e.g., 'left', 'center', ...)";
			} else {
				GLEArrayImpl keys;
				childs->getKeys(&keys);
				str << "'" << *last_elem << "' is not a child object name or justify option" << endl;
				str << "Available names:" << endl;
				keys.enumStrings(str);
			}
			g_throw_parser_error(str.str());
		}
	} else {
		obj = last_obj;
		*just = GLEJusitfyBox;
	}
	return obj;
}

bool GLERun::is_name(GLEString* name) {
	int idx, type;
	GLERC<GLEArrayImpl> path(name->split('.'));
	GLEString* objname = (GLEString*)path->getObjectUnsafe(0);
	std::string str(objname->toUTF8());
	getVars()->find(str, &idx, &type);
	if (idx != -1) {
		GLEDataObject* obj = getVars()->getObject(idx);
		if (obj != NULL && obj->getType() == GLEObjectTypeObjectRep) {
			return is_name((GLEObjectRepresention*)obj, path.get(), 1);
		}
	}
	GLEObjectRepresention* obj = getCRObjectRep();
	if (obj->isChildObjectsEnabled()) {
		return is_name(obj, path.get(), 0);
	}
	return false;
}

GLEObjectRepresention* GLERun::name_to_object(GLEString* name, GLEJustify* just) throw(ParserError) {
	int idx, type;
	GLERC<GLEArrayImpl> path(name->split('.'));
	GLEString* objname = (GLEString*)path->getObjectUnsafe(0);
	std::string str(objname->toUTF8());
	getVars()->find(str, &idx, &type);
	if (idx != -1) {
		GLEDataObject* obj = getVars()->getObject(idx);
		if (obj != NULL && obj->getType() == GLEObjectTypeObjectRep) {
			return name_to_object((GLEObjectRepresention*)obj, path.get(), just, 1);
		} else {
			string err = getVars()->typeError(idx, GLEObjectTypeObjectRep);
			g_throw_parser_error(err);
		}
	} else {
		GLEObjectRepresention* obj = getCRObjectRep();
		if (obj->isChildObjectsEnabled()) {
			return name_to_object(obj, path.get(), just, 0);
		} else {
			ostringstream str;
			str << "name '" << *objname << "' not defined";
			g_throw_parser_error(str.str());
		}
	}
	return NULL;
}

void GLERun::name_to_point(GLEString* name, GLEPoint* point) throw(ParserError) {
	GLEJustify just;
	GLEObjectRepresention* obj = name_to_object(name, &just);
	if (obj != NULL) {
		GLERectangle rect;
		rect.copy(obj->getRectangle());
		g_undev(&rect);
		rect.toPoint(just, point);
	} else {
		point->setXY(0, 0);
	}
}

void GLERun::name_to_size(GLEString* name, double *wd, double *hi) throw(ParserError) {
	GLEJustify just;
	GLEObjectRepresention* obj = name_to_object(name, &just);
	if (obj != NULL) {
		GLERectangle rect;
		rect.copy(obj->getRectangle());
		g_undev(&rect);
		*wd = rect.getWidth();
		*hi = rect.getHeight();
	} else {
		*wd = 0.0; *hi = 0.0;
	}
}

void GLERun::name_join(GLEString *n1, GLEString *n2, int marrow, double a1, double a2, double d1, double d2)  throw(ParserError) {
	GLEJustify j1, j2;
	GLEObjectRepresention* obj1 = name_to_object(n1, &j1);
	GLEObjectRepresention* obj2 = name_to_object(n2, &j2);
	if (j1 == GLEJustifyHorz || j1 == GLEJustifyVert) {
		GLEObjectRepresention* tmpo = obj1; obj1 = obj2; obj2 = tmpo;
		GLEJustify tmpj = j1; j1 = j2; j2 = tmpj;
		if (marrow==2) marrow = 1;
		else if (marrow==1) marrow = 2;
	}
	GLERectangle rect1, rect2;
	rect1.copy(obj1->getRectangle());
	rect2.copy(obj2->getRectangle());
	g_undev(&rect1);
	g_undev(&rect2);
	GLEPoint p1, p2;
	rect1.toPoint(j1, &p1);
	p2.set(p1); /* start with location of p1 for .h and .v joining */
	rect2.toPoint(j2, &p2);
	double sx = p1.getX();
	double sy = p1.getY();
	double ex = p2.getX();
	double ey = p2.getY();
	nm_adjust(j1, &sx, &sy, p2.getX(), p2.getY(), &rect1);
	nm_adjust(j2, &ex, &ey, p1.getX(), p1.getY(), &rect2);
	g_move(sx,sy);
	if (marrow==2) marrow = 1;
	else if (marrow==1) marrow = 2;
	g_arrowcurve(ex, ey, marrow, a1, a2, d1, d2);
}

void GLERun::draw_object_static(const string& path, const string& name, int* pcode, int* cp, bool mkdrobjs) throw (ParserError) {
	int cp_backup = *cp;
	GLEPoint orig;
	g_get_xy(&orig);
	GLEString s_path(path.c_str());
	GLERC<GLEArrayImpl> a_path(s_path.split('.'));
	bool hasoffs = (a_path->size() > 1);
	GLEDevice* olddev = NULL;
	if (hasoffs && !g_is_dummy_device()) {
		/* Do not actually draw */
		olddev = g_set_dummy_device();
	}
	/* backup old object and create new one */
	GLERC<GLEObjectRepresention> crobj(getCRObjectRep());
	GLEObjectRepresention* newobj = new GLEObjectRepresention();
	newobj->enableChildObjects();
	setCRObjectRep(newobj);
	/* draw to measure */
	GLEMeasureBox measure;
	measure.measureStart();
	g_move(0.0, 0.0);
	GLERC<GLEArrayImpl> stk(new GLEArrayImpl());
	if (mkdrobjs) {
		GLESub* sub = eval_subroutine_call(stk.get(), getPcodeList(), pcode, cp);
		sub->setScript(getScript());
		GLEObjectDOConstructor* cons = sub->getObjectDOConstructor();
		GLEObjectDO objdo(cons);
		objdo.setPosition(orig);
		GLEString* refpt = new GLEString();
		refpt->join('.', a_path.get(), 1);
		objdo.setRefPointString(refpt);
		eval_do_object_block_call(stk.get(), sub, &objdo);
		handleNewDrawObject(&objdo, mkdrobjs, &orig);
	} else {
		evalGeneric(getStack(), getPcodeList(), pcode, cp);
	}
	if (hasoffs) measure.measureEndIgnore();
	else measure.measureEnd();
	newobj->getRectangle()->copy(&measure);
	/* restore device */
	g_restore_device(olddev);
	/* draw for real */
	if (hasoffs) {
		/* find offset */
		GLEJustify just;
		GLEPoint transl;
		GLEObjectRepresention* obj = name_to_object(newobj, a_path.get(), &just, 1);
		GLERectangle rect(obj->getRectangle());
		if (obj != newobj) g_undev(&rect);
		rect.toPoint(just, &transl);
		transl.subtractFrom(&orig);
		newobj->getRectangle()->translate(&transl);
		if (olddev != NULL && !mkdrobjs) {
			/* draw for real */
			g_gsave();
			g_translate(transl.getX(), transl.getY());
			*cp = cp_backup;
			g_move(0.0, 0.0);
			evalGeneric(getStack(), getPcodeList(), pcode, cp);
			g_grestore();
		} else {
			/* dummy device, just update bounds */
			g_update_bounds(newobj->getRectangle());
			/* move child objects */
			g_dev_rel(&transl);
			newobj->translateChildrenRecursive(&transl);
		}
	}
	/* convert rectangle to device coordinates */
	g_dev(newobj->getRectangle());
	/* store object representation to variable */
	GLERC<GLEString> objname = (GLEString*)a_path->getObjectUnsafe(0);
	if (name != "") {
		objname = new GLEString(name);
	}
	if (!crobj->setChildObject(objname.get(), newobj)) {
		char ostr[500];
		int idx, type;
		objname->toUTF8(ostr);
		getVars()->findAdd(ostr, &idx, &type);
		getVars()->setObject(idx, newobj);
	}
	/* restore old object */
	setCRObjectRep(crobj.get());
	g_move(orig);
}

void GLERun::draw_object_subbyname(GLESub* sub, GLEObjectRepresention* newobj, GLEArrayImpl* path, GLEPoint* orig) throw (ParserError) {
	bool hasoffs = (path->size() > 1);
	GLEDevice* olddev = NULL;
	if (hasoffs && !g_is_dummy_device()) {
		/* Do not actually draw */
		olddev = g_set_dummy_device();
	}
	/* draw to measure */
	GLEMeasureBox measure;
	measure.measureStart();
	g_move(0.0, 0.0);
	/* call subroutine */
	sub_call(sub);
	if (hasoffs) measure.measureEndIgnore();
	else measure.measureEnd();
	newobj->getRectangle()->copy(&measure);
	/* draw for real */
	if (hasoffs) {
		/* find offset */
		GLEJustify just;
		GLEPoint transl;
		GLEObjectRepresention* obj = name_to_object(newobj, path, &just, 1);
		GLERectangle rect(obj->getRectangle());
		if (obj != newobj) g_undev(&rect);
		rect.toPoint(just, &transl);
		transl.subtractFrom(orig);
		newobj->getRectangle()->translate(&transl);
		if (olddev != NULL) {
			/* draw for real */
			g_restore_device(olddev);
			g_gsave();
			g_translate(transl.getX(), transl.getY());
			g_move(0.0, 0.0);
			/* call subroutine */
			sub_call(sub);
			g_grestore();
		} else {
			/* dummy device, just update bounds */
			g_update_bounds(newobj->getRectangle());
			/* move child objects */
			g_dev_rel(&transl);
			GLEObjectRepresention* newobj = getCRObjectRep();
			newobj->translateChildrenRecursive(&transl);
		}
	}
}

void GLERun::draw_object_dynamic(int idx, GLEObjectRepresention* newobj, GLEArrayImpl* path, GLEPoint* orig) throw (ParserError) {
	GLEDataObject* obj = getVars()->getObject(idx);
	if (obj == NULL || obj->getType() != GLEObjectTypeObjectRep) {
		string err = getVars()->typeError(idx, GLEObjectTypeObjectRep);
		g_throw_parser_error(err);
	}
	GLEObjectRepresention* drawobj = (GLEObjectRepresention*)obj;
	GLEDynamicSub* dynsub = drawobj->getSub();
	if (dynsub == NULL) {
		string err = getVars()->typeError(idx, GLEObjectTypeDynamicSub);
		g_throw_parser_error(err);
	}
	/* backup old object and create new one */
	gmodel* oldstate = dynsub->getState();
	newobj->getRectangle()->copy(drawobj->getRectangle());
	g_undev(newobj->getRectangle(), oldstate);
	/* find out translation offset */
	GLEPoint transl;
	if (path->size() > 1) {
		/* find offset */
		GLEJustify just;
		GLEObjectRepresention* obj = name_to_object(drawobj, path, &just, 1);
		GLERectangle rect(obj->getRectangle());
		g_undev(&rect, oldstate);
		rect.toPoint(just, &transl);
		transl.subtractFrom(orig);
		newobj->getRectangle()->translate(&transl);
	}
	/* draw object */
	if (!g_is_dummy_device()) {
		g_gsave();
		g_translate(transl.getX(), transl.getY());
		GLESub* sub = dynsub->getSub();
		GLEVarMap* save_var_map = NULL;
		GLELocalVars* local_vars = dynsub->getLocalVars();
		if (local_vars != NULL) {
			var_alloc_local(local_vars->size());
			GLELocalVars* new_local_vars = get_local_vars();
			new_local_vars->copyFrom(local_vars);
			GLEVarMap* local_var_map = sub->getParentSub()->getLocalVars();
			save_var_map = var_swap_local_map(local_var_map);
		}
		g_move(0.0, 0.0);
		g_set_partial_state(oldstate);
		int endp = 0;
		bool mkdrobjs = false;
		for (int i = sub->getStart() + 1; i < sub->getEnd(); i++) {
			GLESourceLine* line = getSource()->getLine(i - 1);
			do_pcode(*line, &i, gpcode[i], gplen[i], &endp, mkdrobjs);
		}
		if (local_vars != NULL) {
			var_free_local();
			var_set_local_map(save_var_map);
		}
		g_grestore();
	} else {
		/* dummy device, just update bounds */
		g_update_bounds(newobj->getRectangle());
		/* copy names */
		drawobj->copyChildrenRecursive(newobj, oldstate);
		/* move child objects */
		g_dev_rel(&transl);
		newobj->translateChildrenRecursive(&transl);
	}
}

void GLERun::draw_object(const string& path, const char* newname) throw (ParserError) {
	int idx, type;
	char ostr[255];
	GLEPoint orig;
	g_get_xy(&orig);
	GLESub* sub = NULL;
	GLEString s_path(path.c_str());
	GLERC<GLEArrayImpl> a_path(s_path.split('.'));
	GLERC<GLEString> objname = (GLEString*)a_path->getObjectUnsafe(0);
	objname->toUTF8(ostr);
	getVars()->find(ostr, &idx, &type);
	if (idx == -1) {
		gle_strupr(ostr);
		string myname = ostr;
		sub = getSubroutines()->get(myname);
		if (sub != NULL && sub->getNbParam() != 0) sub = NULL;
	}
	if (idx == -1 && sub == NULL) {
		ostringstream err;
		err << "no object named '" << *objname << "'";
		g_throw_parser_error(err.str());
	}
	GLERC<GLEObjectRepresention> crobj(getCRObjectRep());
	GLEObjectRepresention* newobj = new GLEObjectRepresention();
	newobj->enableChildObjects();
	setCRObjectRep(newobj);
	if (sub != NULL) draw_object_subbyname(sub, newobj, a_path.get(), &orig);
	else draw_object_dynamic(idx, newobj, a_path.get(), &orig);
	/* convert rectangle to device coordinates */
	g_dev(newobj->getRectangle());
	if (newname != NULL) {
		objname = new GLEString(newname);
	}
	if (!crobj->setChildObject(objname.get(), newobj)) {
		int idx, type;
		objname->toUTF8(ostr);
		getVars()->findAdd(ostr, &idx, &type);
		getVars()->setObject(idx, newobj);
	}
	/* restore old object */
	setCRObjectRep(crobj.get());
	g_move(orig);
}

void GLERun::begin_object(const std::string& name, GLESub* sub) throw (ParserError) {
	GLEStoredBox* box = box_start();
	box->setStroke(false);
	box->setObjectRep(getCRObjectRep());
	/* create new object */
	GLEObjectRepresention* newobj = new GLEObjectRepresention();
	newobj->enableChildObjects();
	setCRObjectRep(newobj);
	/* set name */
	int idx, type;
	getVars()->findAdd(name.c_str(), &idx, &type);
	getVars()->setObject(idx, newobj);
	/* create corresponding dynamic subroutine */
	GLEDynamicSub* dynsub = new GLEDynamicSub(sub);
	newobj->setSub(dynsub);
	/* store local variables */
	GLESub* parent = sub->getParentSub();
	if (parent != NULL) {
		GLEVarMap* local_var_map = parent->getLocalVars();
		GLELocalVars* local_vars = get_local_vars();
		if (local_vars != NULL && local_var_map != NULL) {
			dynsub->setLocalVars(local_vars->clone(local_var_map->size()));
		}
	}
	/* store graphics state */
	g_move(0.0, 0.0);
	gmodel* state = new gmodel();
	g_get_state(state);
	dynsub->setState(state);
	/* Do not actually draw */
	if (!g_is_dummy_device()) {
		box->setDevice(g_set_dummy_device());
	}
}

void GLERun::end_object() throw (ParserError) {
	GLEBoxStack* stack = GLEBoxStack::getInstance();
	if (stack->size() <= 0) {
		g_throw_parser_error("too many end boxes");
	}
	GLEStoredBox* box = stack->lastBox();
	/* Get coords of box */
	GLERectangle measure;
	g_get_bounds(&measure);
	if (!measure.isValid()) {
		ostringstream err;
		err << "empty box: " << measure << endl;
		g_throw_parser_error(err.str());
	}
	/* Store coordinates of object */
	GLEObjectRepresention* newobj = getCRObjectRep();
	if (newobj != NULL) {
		newobj->getRectangle()->copy(&measure);
		/* convert rectangle to device coordinates */
		g_dev(newobj->getRectangle());
	}
	/* restore old object */
	setCRObjectRep(box->getObjectRep());
	/* Restore out device */
	if (box->getDevice() != NULL) {
		g_restore_device(box->getDevice());
	}
	/* Reset bounds as if nothing happened */
	g_set_bounds(box->getSaveBounds());
	g_move(box->getOrigin());
	/* Remove box from stack */
	stack->removeBox();
}

void GLERun::begin_length(int var)
{
	GLELengthBlock previous;
	GLECore* core = g_get_core();
	previous.varIndex = var;
	previous.wasEnabled = core->isComputingLength();
	previous.previousValue = core->getTotalLength();
	m_lengthBlocks.push_back(previous);
	core->setComputingLength(true);
	core->setTotalLength(0.0);
}

void GLERun::end_length()
{
	GLECore* core = g_get_core();
	CUtilsAssert(m_lengthBlocks.size() > 0);
	CUtilsAssert(core->isComputingLength());
	GLELengthBlock block(m_lengthBlocks.back());
	m_lengthBlocks.pop_back();
	double length = core->getTotalLength();
	core->setComputingLength(block.wasEnabled);
	core->setTotalLength(block.previousValue + length);
	getVars()->setDouble(block.varIndex, length);
}

GLESubMap* GLERun::getSubroutines() {
	return &g_Subroutines;
}

// What does this accomplish?
// What is justify.box? Not in manual?
void nm_adjust(GLEJustify jj, double *sx, double *sy, double ex, double ey, GLERectangle* r) {
	double dr, da, rz, dx, dy;
	if ((jj & 0xf000) == 0x5000) {
		double r1 = r->getWidth()/2;
		double r2 = r->getHeight()/2;
		xy_polar(*sx - ex, *sy - ey, &dr, &da);
		double xa = da - 180;
		while ((xa<0) || (xa> 180)) {
			if (xa > 180) xa = xa - 180;
			if (xa < 0) xa = xa + 180;
		}
		if (r1==0) return;
		double ca = atan(r2/r1)*180/GLE_PI;
		if (xa < 90) {
			rz = r1/cos(GLE_PI*xa/180);
			if (xa>ca) rz = r2/sin(GLE_PI*xa/180);
		} else {
			xa = xa - 90;
			rz = r2/cos(GLE_PI*xa/180);
			if (xa>(90-ca)) rz = r1/sin(GLE_PI*xa/180);
		}
		dr = dr - rz ;
		polar_xy(dr, da, &dx, &dy);
		*sx = ex + dx;
		*sy = ey + dy;
	}
	if ((jj & 0xff00) == 0x1000) {
		// .ci -> for circle or ellipse
		double rx = r->getWidth()/2;
		double ry = r->getHeight()/2;
		xy_polar(ex-(*sx), ey-(*sy), &dr, &da);
		if (fabs(rx-ry) > 1e-18) {
			double rad_alpha = GLE_PI*da/180;
			da = atan2(rx*sin(rad_alpha), ry*cos(rad_alpha))/GLE_PI*180;
		}
		polar_xy(rx, ry, da, &dx, &dy);
		*sx += dx;
		*sy += dy;
	}
}

int f_eof(int chn) throw(ParserError) {
	if (f_testchan(chn) == -1) return 1;
	else return (int)g_Files[chn]->eof();
}

void f_init() {
}

void siffree(char **s) {
	if (*s != NULL) myfree(*s);
	*s = NULL;
}

int f_testchan(int chn) throw(ParserError) {
	if (chn < 0 || chn >= (int)g_Files.size() || g_Files[chn] == NULL) {
		char chn_s[10];
		sprintf(chn_s, "%d", chn);
		g_throw_parser_error("file not open (file id = ",chn_s,")");
		return -1;
	}
	return chn;
}

void f_create_chan(int var, const char* fname, int rd_wr) {
	GLEFile* file = new GLEFile();
	int freechn = -1;
	for (vector<GLEFile*>::size_type i = 0; i < g_Files.size() && freechn == -1; i++) {
		if (g_Files[i] == NULL) {
			freechn = i;
		}
	}
	if (freechn == -1) {
		freechn = g_Files.size();
		g_Files.push_back(file);
	} else {
		g_Files[freechn] = file;
	}
	file->setRdWr(rd_wr == 0 ? true : false);
	var_set(var, freechn);
	file->open(fname);
}

void f_close_chan(int idx) throw(ParserError) {
	if (f_testchan(idx) != -1) {
		GLEFile* file = g_Files[idx];
		file->close();
		delete file;
		g_Files[idx] = NULL;
	}
}

GLEFile::GLEFile() {
	m_ReadWrite = true;
	m_Output = NULL;
	m_Input = NULL;
}

GLEFile::~GLEFile() {
	close();
}

void GLEFile::close() {
	if (m_Output != NULL) {
		fclose(m_Output);
		m_Output = NULL;
	}
	if (m_Input != NULL) {
		m_Input->close_tokens();
		m_Input->delete_language();
		delete m_Input;
		m_Input = NULL;
	}
}

void GLEFile::open(const char* fname) throw(ParserError) {
	m_FileName = fname;
	if (isRead()) {
		validate_file_name(m_FileName, true);
		m_Input = new StreamTokenizer();
		m_Input->open_tokens(m_FileName.c_str());
		TokenizerLanguage* lang = m_Input->get_language();
		lang->setSpaceTokens(" ,\t\r\n");
		lang->setLineCommentTokens("!");
	} else {
		validate_file_name(m_FileName, false);
		m_Output = fopen(m_FileName.c_str(), "w");
		if (m_Output == NULL) {
			ostringstream err_str;
			err_str << "can't create: '" << m_FileName << "': ";
			str_get_system_error(err_str);
			g_throw_parser_error(err_str.str());
		}
	}
}

char* GLEFile::readLine() throw(ParserError) {
	m_buffer = m_Input->read_line();
	return (char*)m_buffer.c_str();
}

char* GLEFile::getToken() throw(ParserError) {
	m_buffer = m_Input->next_token();
	str_remove_quote(m_buffer);
	return (char*)m_buffer.c_str();
}

void GLEFile::gotoNewLine() throw(ParserError) {
	m_Input->token_skip_to_end();
}

bool GLEFile::eof() throw(ParserError) {
	return m_Input->has_more_tokens() == 0 ? true : false;
}

void GLEFile::resetLang() {
	if (m_Input != NULL) {
		TokenizerLanguage* lang = m_Input->get_language();
		lang->resetCharMaps();
	}
}

void GLEFile::setLangChars(int type, const char* str) {
	if (m_Input != NULL) {
		char set[2];
		set[1] = 0;
		char prev_ch = -1;
		TokenizerLanguage* lang = m_Input->get_language();
		while ((*str) != 0) {
			char ch = str[0];
			bool ok = true;
			// Convert escape sequences in the string !
			if (prev_ch == '\\') {
				if (ch == 'n') ch = '\n';
				else if (ch == 't') ch = '\t';
				else if (ch == 'r') ch = '\r';
			} else if (ch == '\\') {
				ok = false;
			}
			if (ok) {
				set[0] = ch;
				switch (type) {
					case 0: lang->setLineCommentTokens(set); break;
					case 1: lang->setSpaceTokens(set); break;
					case 2: lang->setSingleCharTokens(set); break;
				}
			}
			prev_ch = ch;
			str++;
		}
	}
}
