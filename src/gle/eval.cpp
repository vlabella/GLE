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
#include "tokens/stokenizer.h"
#include "core.h"
#include "glearray.h"
#include "mem_limits.h"
#include "var.h"
#include "cutils.h"
#include "gprint.h"
#include "numberformat.h"
#include "keyword.h"
#include "run.h"
#include "fn.h"
#include "polish.h"
#include "file_io.h"
#include "cmdline.h"
#include "graph.h"
#include "sub.h"
#include "file_io.h"
#ifdef __WIN32__
#include <time.h>
#endif
#include "specialfunctions.h"

using namespace std;

colortyp colvar;
//
// set this to zero if your compiler complains about acosh atanh and asinh
//
// #define HAVE_INVERSE_HYP <== obsolete should be done in config.i V.L.
//
#define true (!false)
#define false 0
char *eval_str();
void var_getstr(int varnum,char *s);
//int pass_marker(char *s);
int f_eof(int chn);
void gle_as_a_calculator(vector<string>* exprs);
void output_error_cerr(ParserError& err);
double xg3d(double x, double y, double z);
double yg3d(double x, double y, double z);

/*---------------------------------------------------------------------------*/
/* bin = 10..29, binstr = 30..49, fn= 60...139, userfn=LOCAL_START_INDEX..nnn */
/* pcode:,  1=exp,len  2=float,val 3=var,int 4,string_var, 5=string,.../0 */
/*---------------------------------------------------------------------------*/
/* Input is exp-pcode, output is number or string */

// for debugging
const char *binop[] = { "", "+", "-", "*", "/", "^", "=", "<", "<=", ">", ">=", "<>", ".AND.", ".OR.", "%", "+=", "-=", "*=" , "/=", "++", "--"};
const char *pcodes[]= {"","PCODE_EXPR","PCODE_DOUBLE","PCODE_VAR","PCODE_STRVAR","PCODE_STRING","PCODE_OBJECT","PCODE_NEXT_CMD"};
string pcode_to_string(int p){
	if( p <= PCODE_NEXT_CMD ){
		return pcodes[p];
	}
	return "";
}

struct keyw
{
	const char *word;
	int index;
	int ret,np,p[5];
} ;

extern struct keyw keywfn[] ;

int stk_var[100];
int stk_strlen[100];
char sbuf[512];
char sbuf2[112];
extern int gle_debug;
extern int ndata;
extern CmdLineObj g_CmdLine;

double string_to_number(const std::string& str) {
// FIXME support other numeric formats
	char* pend;
	return strtod(str.c_str(), &pend);
}

std::string format_number_to_string(const std::string& format, double value);
GLERC<GLEColor> memory_cell_to_color(GLEPolish* polish, GLEArrayImpl* stk, GLEMemoryCell* cell, IThrowsError* throwsError, int depth);

unsigned char float_to_color_comp(double value) {
	int color = (int)floor(value*255 + 0.5);
	if (color < 0) color = 0;
	if (color > 255) color = 255;
	return (unsigned char)color;
}

unsigned char float_to_color_comp_255(double value) {
	int color = (int)floor(value + 0.5);
	if (color < 0) color = 0;
	if (color > 255) color = 255;
	return (unsigned char)color;
}

void eval_get_extra_arg_test(int i, const char* type) {
	int max_arg = g_CmdLine.getNbExtraArgs();
	if (max_arg == 0) {
		stringstream s;
		s << "arg" << type << "(" << i << "): no command line arguments given";
		g_throw_parser_error(s.str());
	}
	if (i > max_arg || i <= 0) {
		stringstream s;
		s << "arg" << type << "(" << i << "): argument out of range (1.." << max_arg << ")";
		g_throw_parser_error(s.str());
	}
}

double eval_get_extra_arg_f(int i) {
	eval_get_extra_arg_test(i, "");
	const string& arg = g_CmdLine.getExtraArg(i-1);
	if (!is_float(arg)) {
		stringstream s;
		s << "arg(" << i << "): argument not a floating point number: " << arg;
		g_throw_parser_error(s.str());
	}
	return atof(arg.c_str());
}

const char* eval_get_extra_arg_s(int i) {
	eval_get_extra_arg_test(i, "$");
	return g_CmdLine.getExtraArg(i-1).c_str();
}

void setEvalStack(GLEArrayImpl* stk, int pos, double value) {
	stk->ensure(pos + 1);
	stk->setDouble(pos, value);
}

void setEvalStack(GLEArrayImpl* stk, int pos, const char* value) {
	stk->ensure(pos + 1);
	stk->setObject(pos, new GLEString(value));
}

void setEvalStack(GLEArrayImpl* stk, int pos, const std::string& value) {
	stk->ensure(pos + 1);
	stk->setObject(pos, new GLEString(value));
}

void setEvalStack(GLEArrayImpl* stk, int pos, int value) {
	stk->ensure(pos + 1);
	stk->setDouble(pos, (double)value);
}

void setEvalStack(GLEArrayImpl* stk, int pos, GLEDataObject* value) {
	stk->ensure(pos + 1);
	stk->setObject(pos, value);
}

void setEvalStackBool(GLEArrayImpl* stk, int pos, bool value) {
	stk->ensure(pos + 1);
	stk->setBool(pos, value);
}

double getEvalStackDouble(GLEArrayImpl* stk, int pos) {
	stk->checkType(pos, GLEObjectTypeDouble);
	return stk->getDouble(pos);
}

int getEvalStackInt(GLEArrayImpl* stk, int pos) {
	stk->checkType(pos, GLEObjectTypeDouble);
	return gle_round_int(stk->getDouble(pos));
}

unsigned int getEvalStackUnsignedInt(GLEArrayImpl* stk, int pos) {
	stk->checkType(pos, GLEObjectTypeDouble);
	return static_cast<unsigned int>(gle_round_int(stk->getDouble(pos)));
}

GLEString* getEvalStackGLEString(GLEArrayImpl* stk, int pos) {
	stk->checkType(pos, GLEObjectTypeString);
	return (GLEString*)stk->getObject(pos);
}

GLEColor* getEvalStackColor(GLEArrayImpl* stk, int pos) {
	stk->checkType(pos, GLEObjectTypeColor);
	return (GLEColor*)stk->getObject(pos);
}

std::string getEvalStackStringStd(GLEArrayImpl* stk, int pos) {
	stk->checkType(pos, GLEObjectTypeString);
	return ((GLEString*)stk->getObject(pos))->toUTF8();
}

void validateIntRange(int value, int from, int to) {
	if (value < from || value > to) {
		std::ostringstream msg;
		msg << "value " << value << " not in range " << from << ", ..., " << to;
		g_throw_parser_error(msg.str());
	}
}

void validateArrayIndexRange(int value, int from, int to) {
	if (value >= 0) {
		validateIntRange(value, from, to);
	} else {
		validateIntRange(value, -to, -from);
	}
}

void complain_operator_type(int op, int type) {
	std::ostringstream msg;
	msg << "operator " << gle_operator_to_string(op) << " does not apply to type '" << gle_object_type_to_string((GLEObjectType)type) << "'";
	g_throw_parser_error(msg.str());
}

void eval_binary_operator_string(GLEArrayImpl* stk, int op, GLEString* a, GLEString* b) {
	switch (op) {
		case BIN_OP_PLUS:
			setEvalStack(stk, stk->last() - 1, a->concat(b));
			break;
		case BIN_OP_EQUALS:
			setEvalStackBool(stk, stk->last() - 1, a->equalsI(b));
			break;
		case BIN_OP_LT:
			setEvalStackBool(stk, stk->last() - 1, a->strICmp(b) < 0);
			break;
		case BIN_OP_LE:
			setEvalStackBool(stk, stk->last() - 1, a->strICmp(b) <= 0);
			break;
		case BIN_OP_GT:
			setEvalStackBool(stk, stk->last() - 1, a->strICmp(b) > 0);
			break;
		case BIN_OP_GE:
			setEvalStackBool(stk, stk->last() - 1, a->strICmp(b) >= 0);
			break;
		case BIN_OP_NOT_EQUALS:
			setEvalStackBool(stk, stk->last() - 1, !a->equalsI(b));
			break;
		case BIN_OP_DOT:
			{
				GLERC<GLEString> dot(new GLEString("."));
				GLERC<GLEString> temp(a->concat(dot.get()));
				setEvalStack(stk, stk->last() - 1, temp->concat(b));
			}
			break;
		default:
			complain_operator_type(op, GLEObjectTypeString);
			break;
	}
}
// constants defined in polish.h
void eval_binary_operator_double(GLEArrayImpl* stk, int op, double a, double b) {
	//cout << "EVALDOUBLE " << binop[op]<<":"<<op<<" "<<a<<" "<<b<<endl;
	switch (op) {
		case BIN_OP_PLUS:
			setEvalStack(stk, stk->last() - 1, a + b);
			break;
		case BIN_OP_MINUS:
			setEvalStack(stk, stk->last() - 1, a - b);
			break;
		case BIN_OP_MULTIPLY:
			setEvalStack(stk, stk->last() - 1, a * b);
			break;
		case BIN_OP_DIVIDE:
			// do not test on divide by zero, otherwise "let"
			// cannot plot functions with divide by zero anymore
			setEvalStack(stk, stk->last() - 1, a / b);
			break;
		case BIN_OP_POW:
			setEvalStack(stk, stk->last() - 1, pow(a, b));
			break;
		case BIN_OP_EQUALS:
			setEvalStackBool(stk, stk->last() - 1, a == b);
			break;
		case BIN_OP_LT:
			setEvalStackBool(stk, stk->last() - 1, a < b);
			break;
		case BIN_OP_LE:
			setEvalStackBool(stk, stk->last() - 1, a <= b);
			break;
		case BIN_OP_GT:
			setEvalStackBool(stk, stk->last() - 1, a > b);
			break;
		case BIN_OP_GE:
			setEvalStackBool(stk, stk->last() - 1, a >= b);
			break;
		case BIN_OP_NOT_EQUALS:
			setEvalStackBool(stk, stk->last() - 1, a != b);
			break;
		case BIN_OP_MOD:
			setEvalStack(stk, stk->last() - 1, gle_round_int(a) % gle_round_int(b));
			break;
		case BIN_OP_PLUS_EQUALS:
			setEvalStack(stk, stk->last() - 1, a += b);
			break;
		case BIN_OP_MINUS_EQUALS:
			setEvalStack(stk, stk->last() - 1, a -= b);
			break;
		case BIN_OP_MULTIPLY_EQUALS:
			setEvalStack(stk, stk->last() - 1, a *= b);
			break;
		case BIN_OP_DIVIDE_EQUALS:
			setEvalStack(stk, stk->last() - 1, a /= b);
			break;
		case BIN_OP_PLUS_PLUS:
			setEvalStack(stk, stk->last() - 1, a++ );
			break;
		case BIN_OP_MINUS_MINUS:
			setEvalStack(stk, stk->last() - 1, a-- );
			break;
		default:
			complain_operator_type(op, GLEObjectTypeDouble);
			break;
	}
	//cout << "EVALDOUBLE " << binop[op]<<":"<<op<<" "<<a<<" "<<b<<endl;
}

void eval_binary_operator_bool(GLEArrayImpl* stk, int op, bool a, bool b) {
	switch (op) {
		case BIN_OP_AND:
			setEvalStackBool(stk, stk->last() - 1, a && b);
			break;
		case BIN_OP_OR:
			setEvalStackBool(stk, stk->last() - 1, a || b);
			break;
		default:
			complain_operator_type(op, GLEObjectTypeDouble);
			break;
	}
}

void eval_unary_operator_double(GLEArrayImpl* stk, int op, double a)
{
	switch (op) {
		case BIN_OP_PLUS_PLUS:
			setEvalStack(stk, stk->last(), ++a );
			break;
		case BIN_OP_MINUS_MINUS:
			setEvalStack(stk, stk->last(), --a );
			break;
		default:
			complain_operator_type(op, GLEObjectTypeDouble);
			break;
	}
}

void eval_unary_operator_bool(GLEArrayImpl* stk, int op, bool a)
{
	// C++17 defines ++ for bool types but not --  Not sure if this impacts GLE - but here for completeness
	// but gcc on linux does not support it so this is a noop
	switch (op) {
		//case BIN_OP_PLUS_PLUS:
			//a++;
		//	setEvalStack(stk, stk->last(), a );
		//	break;
		default:
			complain_operator_type(op, GLEObjectTypeDouble);
			break;
	}
}


void eval_unary_operator(GLEArrayImpl* stk, int op)
{
	// a OP
	GLEMemoryCell* a = stk->get(stk->last());
	int a_type = gle_memory_cell_type(a);
	switch (a_type) {
		case GLEObjectTypeDouble:
			eval_unary_operator_double(stk, op, a->Entry.DoubleVal);
			break;
		case GLEObjectTypeBool:
			eval_unary_operator_bool(stk, op, a->Entry.BoolVal);
			break;
		default:
			complain_operator_type(op, a_type);
			break;
	}
	//stk->decrementSize(1);
}

void eval_binary_operator(GLEArrayImpl* stk, int op) {
	// a OP b
	GLEMemoryCell* a = stk->get(stk->last() - 1);
	int a_type = gle_memory_cell_type(a);
	GLEMemoryCell* b = stk->get(stk->last());
	int b_type = gle_memory_cell_type(b);
	if (a_type == b_type) {
		switch (a_type) {
			case GLEObjectTypeDouble:
				eval_binary_operator_double(stk, op, a->Entry.DoubleVal, b->Entry.DoubleVal);
				break;
			case GLEObjectTypeString:
				eval_binary_operator_string(stk, op, (GLEString*)a->Entry.ObjectVal, (GLEString*)b->Entry.ObjectVal);
				break;
			case GLEObjectTypeBool:
				eval_binary_operator_bool(stk, op, a->Entry.BoolVal, b->Entry.BoolVal);
				break;
			default:
				complain_operator_type(op, a_type);
				break;
		}
	} else if (op == BIN_OP_PLUS && (a_type == GLEObjectTypeString || b_type == GLEObjectTypeString)) {
		GLERC<GLEString> a_str(stk->getString(stk->last() - 1));
		GLERC<GLEString> b_str(stk->getString(stk->last()));
		eval_binary_operator_string(stk, op, a_str.get(), b_str.get());
	} else {
		std::ostringstream msg;
		msg << "operator " << gle_operator_to_string(op)
			<< " does not apply to types '" << gle_object_type_to_string((GLEObjectType)a_type)
			<< "' and '" << gle_object_type_to_string((GLEObjectType)b_type) << "'";
		g_throw_parser_error(msg.str());
	}
	stk->decrementSize(1);
}

void eval_pcode_loop(GLEArrayImpl* stk, GLEPcodeList* pclist, int *pcode, int plen) {
	if (plen > 1000) {
		gprint("Expression is suspiciously long %d \n",plen);
	}
	union {double d; int l[2];} both;
	double x1, y1, x2, y2;
	double xx, yy;
	int i, j;
	//cout << endl << "plen " << plen << endl;
	for (int c = 0; c < plen; c++) {
	  //cout << "pos: " << c << " pcode: " << pcode[c] <<" "<<pcode_to_string(pcode[c])<< endl;
	  switch (pcode[c]) {
		/*
		..
		Special commands 1..9  -------------------------------
		..
		*/
		case PCODE_EXPR:	/* Start of another expression (function param) */
			c++;	/* skip over exp length */
			break;
		case PCODE_DOUBLE: /* Floating point number follows */
			both.l[0] = *(pcode+(++c));
			both.l[1] = *(pcode+(++c));
			stk->incrementSize(1);
			setEvalStack(stk, stk->last(), both.d);
 			dbg gprint("Got float %f %d %f \n",getEvalStackDouble(stk, stk->last()),stk->last(),*(pcode+(c)));
			break;
		case PCODE_VAR: /* Floating_point variable number follows */
		case PCODE_STRVAR: /* string variable number follows */
			i = *(pcode + (++c));
			stk->incrementSize(1);
			stk->ensure(stk->size());
			getVarsInstance()->get(i, stk->get(stk->last()));
			//cout << i << ":"<<stk->get(stk->last())<<endl;
			//gprint("Got variable %f %d %f \n",getEvalStackDouble(stk, stk->last()),stk->last(),*(pcode+(c)));
			break;
		case PCODE_STRING: /* Null terminated string follows (int aligned) */
			c++;
			stk->incrementSize(1);
			setEvalStack(stk, stk->last(), eval_str(pcode, &c));
			break;
		case PCODE_OBJECT:
			i = *(pcode+(++c));
			stk->incrementSize(1);
			setEvalStack(stk, stk->last(), pclist->get(i));
			break;
		/*
			Binary operators 10..29 (now 31) -----------------------
		*/
		case BIN_OP_PLUS + BINARY_OPERATOR_OFFSET:
		case BIN_OP_MINUS + BINARY_OPERATOR_OFFSET:
		case BIN_OP_MULTIPLY + BINARY_OPERATOR_OFFSET:
		case BIN_OP_DIVIDE + BINARY_OPERATOR_OFFSET:
		case BIN_OP_POW + BINARY_OPERATOR_OFFSET:
		case BIN_OP_EQUALS + BINARY_OPERATOR_OFFSET:
		case BIN_OP_LT + BINARY_OPERATOR_OFFSET:
		case BIN_OP_LE + BINARY_OPERATOR_OFFSET:
		case BIN_OP_GT + BINARY_OPERATOR_OFFSET:
		case BIN_OP_GE + BINARY_OPERATOR_OFFSET:
		case BIN_OP_NOT_EQUALS + BINARY_OPERATOR_OFFSET:
		case BIN_OP_AND + BINARY_OPERATOR_OFFSET:
		case BIN_OP_OR + BINARY_OPERATOR_OFFSET:
		case BIN_OP_MOD + BINARY_OPERATOR_OFFSET:
		case BIN_OP_DOT + BINARY_OPERATOR_OFFSET:
		case BIN_OP_PLUS_EQUALS + BINARY_OPERATOR_OFFSET:
		case BIN_OP_MINUS_EQUALS + BINARY_OPERATOR_OFFSET:
		case BIN_OP_MULTIPLY_EQUALS + BINARY_OPERATOR_OFFSET:
		case BIN_OP_DIVIDE_EQUALS + BINARY_OPERATOR_OFFSET:  // 29
			eval_binary_operator(stk, pcode[c] - BINARY_OPERATOR_OFFSET);
			break;
		case BIN_OP_PLUS_PLUS + BINARY_OPERATOR_OFFSET: 	 // 30
		case BIN_OP_MINUS_MINUS + BINARY_OPERATOR_OFFSET:	 // 31
			eval_unary_operator(stk, pcode[c] - BINARY_OPERATOR_OFFSET);
			break;

	    /* look in fn.c and start indexes with 1 */
		/* Built in functions 60..199 ----------------------------- */
		/* note add 60 to the index in fn.cpp */
		case 61: /* unary plus */
			break;
		case 62: /* unary minus */
			setEvalStack(stk, stk->last(), -getEvalStackDouble(stk, stk->last()));
			break;
		case 63: /* abs */
			setEvalStack(stk, stk->last(), fabs(getEvalStackDouble(stk, stk->last())));
			break;
		case 64: /* atan */
			setEvalStack(stk, stk->last(), atan(getEvalStackDouble(stk, stk->last())));
			break;
		case 113: /* ACOS 53*/
			setEvalStack(stk, stk->last(), acos(getEvalStackDouble(stk, stk->last())));
			break;
		case 114: /* ASIN 54*/
			setEvalStack(stk, stk->last(), asin(getEvalStackDouble(stk, stk->last())));
			break;
		case 65: /* cos 5*/
			setEvalStack(stk, stk->last(), cos(getEvalStackDouble(stk, stk->last())));
			break;
		case 116: /* ACOT 56*/
			setEvalStack(stk, stk->last(), 1.0/atan(getEvalStackDouble(stk, stk->last())));
			break;
		case 117: /*ASEC 57*/
			setEvalStack(stk, stk->last(), 1.0/acos(getEvalStackDouble(stk, stk->last())));
			break;
		case 118: /*ACSC 58*/
			setEvalStack(stk, stk->last(), 1.0/asin(getEvalStackDouble(stk, stk->last())));
			break;
		case 119: /*cot 59*/
			setEvalStack(stk, stk->last(), 1.0/tan(getEvalStackDouble(stk, stk->last())));
			break;
		case 120: /*sec 60*/
			setEvalStack(stk, stk->last(), 1.0/cos(getEvalStackDouble(stk, stk->last())));
			break;
		case 121: /*csc 61*/
			setEvalStack(stk, stk->last(), 1.0/sin(getEvalStackDouble(stk, stk->last())));
			break;
		case 122: /* cosh 62*/
			setEvalStack(stk, stk->last(), cosh(getEvalStackDouble(stk, stk->last())));
			break;
		case 123: /* sinh 63*/
			setEvalStack(stk, stk->last(), sinh(getEvalStackDouble(stk, stk->last())));
			break;
		case 124: /* tanh 64*/
			setEvalStack(stk, stk->last(), tanh(getEvalStackDouble(stk, stk->last())));
			break;
		case 125: /* coth 65*/
			setEvalStack(stk, stk->last(), 1.0/tanh(getEvalStackDouble(stk, stk->last())));
			break;
		case 126: /* sech 66*/
			setEvalStack(stk, stk->last(), 1.0/cosh(getEvalStackDouble(stk, stk->last())));
			break;
		case 127: /* csch 67*/
			setEvalStack(stk, stk->last(), 1.0/sinh(getEvalStackDouble(stk, stk->last())));
			break;
		//#ifdef HAVE_INVERSE_HYP VL eliminated - in c++11 standard now
		case 128: /* acosh 68*/
			setEvalStack(stk, stk->last(), acosh(getEvalStackDouble(stk, stk->last())));
			break;
		case 129: /* asinh 69*/
			setEvalStack(stk, stk->last(), asinh(getEvalStackDouble(stk, stk->last())));
			break;
		case 130: /* atanh 70*/
			setEvalStack(stk, stk->last(), atanh(getEvalStackDouble(stk, stk->last())));
			break;
		case 131: /* acoth 71*/
			setEvalStack(stk, stk->last(), 1.0/atanh(getEvalStackDouble(stk, stk->last())));
			break;
		case 132: /* asech 72*/
			setEvalStack(stk, stk->last(), 1.0/acosh(getEvalStackDouble(stk, stk->last())));
			break;
		case 133: /* acsch 73*/
			setEvalStack(stk, stk->last(), 1.0/asinh(getEvalStackDouble(stk, stk->last())));
			break;
		//#endif
		case 134: /* todeg 74*/
			setEvalStack(stk, stk->last(), getEvalStackDouble(stk, stk->last())*180.0/GLE_PI);
			break;
		case 135: /* torad 75*/
			setEvalStack(stk, stk->last(), getEvalStackDouble(stk, stk->last())*GLE_PI/180.0);
			break;
		case FN_BUILTIN_MAGIC + FN_EVAL: /* eval */
			{
				std::string toEval(getEvalStackStringStd(stk, stk->last()));
				stk->set(stk->last(), get_global_polish()->evalGeneric(stk, toEval.c_str()));
			}
			break;
		case FN_BUILTIN_MAGIC + FN_ARG: /* arg */
			setEvalStack(stk, stk->last(), eval_get_extra_arg_f((int)getEvalStackDouble(stk, stk->last())));
			break;
		case FN_BUILTIN_MAGIC + FN_ARGS: /* arg$ */
			setEvalStack(stk, stk->last(), (char*)eval_get_extra_arg_s((int)getEvalStackDouble(stk, stk->last())));
			break;
		case FN_BUILTIN_MAGIC + FN_NARGS: /* narg */
			stk->incrementSize(1);
			setEvalStack(stk, stk->last(), g_CmdLine.getNbExtraArgs());
			break;
		case FN_BUILTIN_MAGIC + FN_MIN: /* min */
			stk->decrementSize(1);
			if (getEvalStackDouble(stk, stk->last()+1) < getEvalStackDouble(stk, stk->last())) setEvalStack(stk, stk->last(), getEvalStackDouble(stk, stk->last()+1));
			break;
		case FN_BUILTIN_MAGIC + FN_MAX: /* max */
			stk->decrementSize(1);
			if (getEvalStackDouble(stk, stk->last()+1) > getEvalStackDouble(stk, stk->last())) setEvalStack(stk, stk->last(), getEvalStackDouble(stk, stk->last()+1));
			break;
		case FN_BUILTIN_MAGIC + FN_SDIV: /* sdiv */
			if (getEvalStackDouble(stk, stk->last()) == 0.0) setEvalStack(stk, stk->last()-1, 0.0);
			else setEvalStack(stk, stk->last()-1, getEvalStackDouble(stk, stk->last()-1) / getEvalStackDouble(stk, stk->last()));
			stk->decrementSize(1);
			break;
		case FN_BUILTIN_MAGIC + FN_XBAR: /* bar x position */
			stk->decrementSize(1);
			setEvalStack(stk, stk->last(), graph_bar_pos(getEvalStackDouble(stk, stk->last()), getEvalStackInt(stk, stk->last() + 1), 1));
			break;
		case FN_BUILTIN_MAGIC + FN_XY2ANGLE:
			stk->decrementSize(1);
			xy_polar(getEvalStackDouble(stk, stk->last()), getEvalStackDouble(stk, stk->last()+1), &x1, &y1);
			setEvalStack(stk, stk->last(), y1);
			break;
		case FN_BUILTIN_MAGIC + FN_ATAN2:
			stk->decrementSize(1);
			setEvalStack(stk, stk->last(), myatan2(getEvalStackDouble(stk, stk->last()), getEvalStackDouble(stk, stk->last()+1)));
			break;
		case FN_BUILTIN_MAGIC + FN_ISNAME:
			setEvalStackBool(stk, stk->last(), getGLERunInstance()->is_name(getEvalStackGLEString(stk, stk->last())));
			break;
		case FN_BUILTIN_MAGIC + FN_FACTORIAL:
			setEvalStack(stk, stk->last(), boost::math::factorial<double>(getEvalStackDouble(stk, stk->last())));
			break;
		case FN_BUILTIN_MAGIC + FN_DOUBLE_FACTORIAL:
			setEvalStack(stk, stk->last(), boost::math::double_factorial<double>(getEvalStackDouble(stk, stk->last())));
			break;
		case FN_BUILTIN_MAGIC + FN_HERMITE:
			stk->decrementSize(1);
			setEvalStack(stk, stk->last(), boost::math::hermite(getEvalStackInt(stk, stk->last()), getEvalStackDouble(stk, stk->last()+1)));
			break;
		case FN_BUILTIN_MAGIC + FN_LAGUERRE:
			stk->decrementSize(1);
			setEvalStack(stk, stk->last(), boost::math::laguerre(getEvalStackInt(stk, stk->last()), getEvalStackDouble(stk, stk->last()+1)));
			break;
		case FN_BUILTIN_MAGIC + FN_ASSOCIATED_LAGUERRE:
			stk->decrementSize(2);
			setEvalStack(stk, stk->last(), boost::math::laguerre(getEvalStackInt(stk, stk->last()), getEvalStackInt(stk, stk->last()+1), getEvalStackDouble(stk, stk->last()+2)));
			break;
		case FN_BUILTIN_MAGIC + FN_LEGENDRE:
			stk->decrementSize(1);
			setEvalStack(stk, stk->last(), boost::math::legendre_p(getEvalStackInt(stk, stk->last()), getEvalStackDouble(stk, stk->last()+1)));
			break;
		case FN_BUILTIN_MAGIC + FN_ASSOCIATED_LEGENDRE:
			stk->decrementSize(2);
			setEvalStack(stk, stk->last(), boost::math::legendre_p(getEvalStackInt(stk, stk->last()), getEvalStackInt(stk, stk->last()+1), getEvalStackDouble(stk, stk->last()+2)));
			break;
		case FN_BUILTIN_MAGIC + FN_BESSEL_FIRST:
			stk->decrementSize(1);
			setEvalStack(stk, stk->last(), boost::math::cyl_bessel_j( getEvalStackDouble(stk, stk->last()), getEvalStackDouble(stk, stk->last()+1)));
			break;
		case FN_BUILTIN_MAGIC + FN_BESSEL_SECOND:
			stk->decrementSize(1);
			setEvalStack(stk, stk->last(), boost::math::cyl_neumann( getEvalStackDouble(stk, stk->last()) , getEvalStackDouble(stk, stk->last()+1)));
			break;
		case FN_BUILTIN_MAGIC + FN_SPHERICAL_HARMONIC:
			stk->decrementSize(3);
			setEvalStack(stk, stk->last(),
				boost::math::spherical_harmonic_r(
					getEvalStackInt(stk, stk->last()), getEvalStackInt(stk, stk->last()+1), getEvalStackDouble(stk, stk->last()+2), getEvalStackDouble(stk, stk->last()+3)
					)
				);
			break;
		case FN_BUILTIN_MAGIC + FN_ERF:
			setEvalStack(stk, stk->last(), erf(getEvalStackDouble(stk, stk->last())));
			break;
		case FN_BUILTIN_MAGIC + FN_AIRY_FIRST:
			setEvalStack(stk, stk->last(), boost::math::airy_ai( getEvalStackDouble(stk, stk->last()) ) );
			break;
		case FN_BUILTIN_MAGIC + FN_AIRY_SECOND:
			setEvalStack(stk, stk->last(), boost::math::airy_bi( getEvalStackDouble(stk, stk->last()) ) );
			break;
		case FN_BUILTIN_MAGIC + FN_CHEBYSHEV_FIRST:
			stk->decrementSize(1);
			setEvalStack(stk, stk->last(), boost::math::chebyshev_t( getEvalStackUnsignedInt(stk, stk->last()), getEvalStackDouble(stk, stk->last()+1)));
			break;
		case FN_BUILTIN_MAGIC + FN_CHEBYSHEV_SECOND:
			stk->decrementSize(1);
			setEvalStack(stk, stk->last(), boost::math::chebyshev_u( getEvalStackUnsignedInt(stk, stk->last()), getEvalStackDouble(stk, stk->last()+1)));
			break;
		case FN_BUILTIN_MAGIC + FN_TEST_PRINT: // test_print
			// simple for loop that evaluates a function that is passed to it
			//{
			//	std::string toEval(getEvalStackStringStd(stk, stk->last()));
			//	stk->set(stk->last(), get_global_polish()->evalGeneric(stk, toEval.c_str()));
			//}
			cout << endl;
			stk->decrementSize(2);
			{
			std::string toEval(getEvalStackStringStd(stk, stk->last()));
			cout << toEval << endl;
			stringstream ss;
			ss << "x="<<getEvalStackDouble(stk, stk->last()+1);
			cout << ss.str().c_str()<<endl;
			cout << getEvalStackDouble(stk, stk->last()+2)<<endl;
			double x;
			//get_global_polish()->eval(stk, ss.str().c_str(),&x);
			stk->set(stk->last(), get_global_polish()->evalGeneric(stk, ss.str().c_str() ));
			//setEvalStack(stk, stk->last(),ss.str().c_str());
			//setEvalStack(stk, stk->last(),"print x");
			//get_global_polish()->evalString(stk, ss.str().c_str());
			stk->set(stk->last(), get_global_polish()->evalGeneric(stk, toEval.c_str()));
			}
			break;
		case 137: /* pointx */
			{
				GLEPoint pt;
				getGLERunInstance()->name_to_point(getEvalStackGLEString(stk, stk->last()), &pt);
				setEvalStack(stk, stk->last(), pt.getX());
			}
			break;
		case 138: /* pointy */
			{
				GLEPoint pt;
				getGLERunInstance()->name_to_point(getEvalStackGLEString(stk, stk->last()), &pt);
				setEvalStack(stk, stk->last(), pt.getY());
			}
			break;
		case 139: /* format$ */
			stk->decrementSize(1);
			setEvalStack(stk, stk->last(), format_number_to_string(getEvalStackStringStd(stk, stk->last()+1), getEvalStackDouble(stk, stk->last())));
			break;
		case FN_BUILTIN_MAGIC + FN_GETENV: /* getenv */
			{
				string result;
				GLEGetEnv(getEvalStackStringStd(stk, stk->last()), result);
				setEvalStack(stk, stk->last(), result);
			}
			break;
		case 66: /* date$ */
			{
				time_t today;
				time(&today);
				strcpy(sbuf2,ctime(&today));
				strcpy(sbuf,sbuf2);
				strcpy(sbuf+11,sbuf2+20);
				sbuf[strlen(sbuf)-1] = 0;
				stk->incrementSize(1);
				setEvalStack(stk, stk->last(), sbuf);
			}
			break;
		case 111: /* device$ */
			stk->incrementSize(1);
			setEvalStack(stk, stk->last(), g_get_type());
			break;
		case 115: /* feof(chan) */
			setEvalStack(stk, stk->last(), f_eof((int) getEvalStackDouble(stk, stk->last())));
			break;
		case 67: /* exp */
			setEvalStack(stk, stk->last(), exp(getEvalStackDouble(stk, stk->last())));
			break;
		case 68: /* fix*/
			setEvalStack(stk, stk->last(), floor(getEvalStackDouble(stk, stk->last())));
			break;
		case 69: /* height of named object */
			getGLERunInstance()->name_to_size(getEvalStackGLEString(stk, stk->last()), &x1, &y1);
			setEvalStack(stk, stk->last(), y1);
			break;
		case 70: /* int (??int) */
			setEvalStack(stk, stk->last(), floor(fabs(getEvalStackDouble(stk, stk->last())))
				*( (getEvalStackDouble(stk, stk->last())>=0)?1:-1 ) );
			break;
		case 112: /* CHR$() */
			{
				GLERC<GLEString> str(new GLEString());
				str->setSize(1);
				str->set(0, getEvalStackInt(stk, stk->last()));
				setEvalStack(stk, stk->last(), str.get());
			}
			break;
		case 71: /* left$ */
			{
				int number = getEvalStackInt(stk, stk->last());
				GLEString* str = getEvalStackGLEString(stk, stk->last() - 1);
				validateIntRange(number, 0, str->length());
				stk->decrementSize(1);
				setEvalStack(stk, stk->last(), str->substringWithLength(0, number));
			}
			break;
		case 72: /* len */
			setEvalStack(stk, stk->last(), (int)getEvalStackGLEString(stk, stk->last())->length());
			break;
		case 73: /* log */
			setEvalStack(stk, stk->last(), log(getEvalStackDouble(stk, stk->last())));
			break;
		case 74: /* log10 */
			setEvalStack(stk, stk->last(), log10(getEvalStackDouble(stk, stk->last())));
			break;
		case 75: /* not */
			setEvalStack(stk, stk->last(), !(getEvalStackDouble(stk, stk->last())));
			break;
		case 76: /* num$ */
			sprintf(sbuf,"%g",getEvalStackDouble(stk, stk->last()));
			setEvalStack(stk, stk->last(), sbuf);
			break;
		case 77: /* num1$ */
			sprintf(sbuf,"%g ",getEvalStackDouble(stk, stk->last()));
			setEvalStack(stk, stk->last(), sbuf);
			break;
		case 78: /* pageheight */
			g_get_usersize(&xx, &yy);
			stk->incrementSize(1);
			setEvalStack(stk, stk->last(), yy);
			break;
		case 79: /* pagewidth */
			g_get_usersize(&xx, &yy);
			stk->incrementSize(1);
			setEvalStack(stk, stk->last(), xx);
			break;
		case 80: /* pos */
			{
				int from = getEvalStackInt(stk, stk->last());
				GLERC<GLEString> needle(getEvalStackGLEString(stk, stk->last() - 1));
				GLEString* hayStack(getEvalStackGLEString(stk, stk->last() - 2));
				validateArrayIndexRange(from, 1, hayStack->length());
				stk->decrementSize(2);
				setEvalStack(stk, stk->last(), hayStack->find(needle.get(), hayStack->toStringIndex(from)) + 1);
			}
			break;
		case 81: /* right$ */
			{
				int number = getEvalStackInt(stk, stk->last());
				GLEString* str = getEvalStackGLEString(stk, stk->last() - 1);
				validateIntRange(number, 0, str->length());
				stk->decrementSize(1);
				setEvalStack(stk, stk->last(), str->substringWithLength(str->toStringIndex(-number), number));
			}
			break;
		case 82: /* rnd */
			setEvalStack(stk, stk->last(), ((double) rand()/(double)RAND_MAX)*getEvalStackDouble(stk, stk->last()));
			break;
		case 83: /* seg$ */
			{
				int from = getEvalStackInt(stk, stk->last() - 1);
				int to = getEvalStackInt(stk, stk->last());
				GLEString* str = getEvalStackGLEString(stk, stk->last() - 2);
				validateArrayIndexRange(from, 1, str->length());
				validateArrayIndexRange(to, 1, str->length());
				stk->decrementSize(2);
				setEvalStack(stk, stk->last(), str->substring(str->toStringIndex(from), str->toStringIndex(to)));
			}
			break;
		case 84: /* sgn */
			if (getEvalStackDouble(stk, stk->last())>=0) setEvalStack(stk, stk->last(), 1);
			else setEvalStack(stk, stk->last(), -1);
			break;
		case 85: /* sin */
			setEvalStack(stk, stk->last(), sin(getEvalStackDouble(stk, stk->last())));
			break;
		case 86: /* sqr */
			setEvalStack(stk, stk->last(), getEvalStackDouble(stk, stk->last()) * getEvalStackDouble(stk, stk->last()));
			break;
		case 87: /* sqrt */
			setEvalStack(stk, stk->last(), sqrt(getEvalStackDouble(stk, stk->last())));
			break;
		case 88: /* tan */
			setEvalStack(stk, stk->last(), tan(getEvalStackDouble(stk, stk->last())));
			break;
		case 89: /* tdepth */
			g_get_xy(&xx,&yy);
			g_measure(getEvalStackStringStd(stk, stk->last()),&x1,&x2,&y2,&y1);
			setEvalStack(stk, stk->last(), y1);
			break;
		case 90: /* theight */
			g_get_xy(&xx,&yy);
			g_measure(getEvalStackStringStd(stk, stk->last()),&x1,&x2,&y2,&y1);
			setEvalStack(stk, stk->last(), y2);
			break;
		case 91: /* time$ */
			{
				time_t today;
				time(&today);
				ncpy(sbuf,ctime(&today)+11,9);
				stk->incrementSize(1);
				setEvalStack(stk, stk->last(), sbuf);
			}
			break;
		case FN_BUILTIN_MAGIC + FN_FILE: /* file$ */
			{
				string tmp_s = getGLERunInstance()->getScript()->getLocation()->getMainName();
				stk->incrementSize(1);
				setEvalStack(stk, stk->last(), tmp_s.c_str());
			}
			break;
		case FN_BUILTIN_MAGIC + FN_PATH: /* path$ */
			stk->incrementSize(1);
			setEvalStack(stk, stk->last(), getGLERunInstance()->getScript()->getLocation()->getDirectory().c_str());
			break;
		case FN_BUILTIN_MAGIC + FN_FONT:
			setEvalStackBool(stk, stk->last(), check_has_font(getEvalStackStringStd(stk, stk->last())) != 0);
			break;
		case 92: /* twidth */
			g_measure(getEvalStackStringStd(stk, stk->last()),&x1,&x2,&y1,&y2);
			setEvalStack(stk, stk->last(), x2-x1);
			break;
		case 93: /* val */
			setEvalStack(stk, stk->last(), string_to_number(getEvalStackStringStd(stk, stk->last())));
			break;
		case 94: /* width of named object */
			getGLERunInstance()->name_to_size(getEvalStackGLEString(stk, stk->last()), &x1, &y1);
			setEvalStack(stk, stk->last(), x1);
			break;
		case 95: /* xend */
			stk->incrementSize(1);
			setEvalStack(stk, stk->last(), tex_xend());
			break;
		case 96: /* xgraph */
			setEvalStack(stk, stk->last(), graph_xgraph(getEvalStackDouble(stk, stk->last())));
			break;
		case 97: /* xmax */
			break;
		case 98: /* xmin */
			break;
		case 99: /* xpos */
			g_get_xy(&xx,&yy);
			stk->incrementSize(1);
			setEvalStack(stk, stk->last(), xx);
			break;
		case 100: /* yend */
			stk->incrementSize(1);
			setEvalStack(stk, stk->last(), tex_yend());
			break;
		case 101: /* ygraph */
			setEvalStack(stk, stk->last(), graph_ygraph(getEvalStackDouble(stk, stk->last())));
			break;
		case 102: /* ymax */
			break;
		case 103: /* ymin */
			break;
		case 104: /* ypos */
			g_get_xy(&xx,&yy);
			stk->incrementSize(1);
			setEvalStack(stk, stk->last(), yy);
			break;
		case 105: /* CVTGRAY(.5) */
			{
				GLERC<GLEColor> color(new GLEColor(getEvalStackDouble(stk, stk->last())));
				setEvalStack(stk, stk->last(), color.get());
			}
			break;
		case 106: /* CVTINT(2) */
			setEvalStack(stk, stk->last(), (int) floor(getEvalStackDouble(stk, stk->last())));
			break;
		case 108: /* CVTMARKER(m$) */
			both.l[0] = get_marker_string(getEvalStackStringStd(stk, stk->last()), g_get_throws_error());
			both.l[1] = 0;
			setEvalStack(stk, stk->last(), both.d);
			break;
		case 110: /* CVTFONT(m$) */
			both.l[0] = pass_font(getEvalStackStringStd(stk, stk->last()));
			both.l[1] = 0;
			setEvalStack(stk, stk->last(), both.d);
			break;
		case FN_BUILTIN_MAGIC + FN_JUSTIFY: /* JUSTIFY(m$) */
			both.l[0] = pass_justify(getEvalStackStringStd(stk, stk->last()));
			both.l[1] = 0;
			setEvalStack(stk, stk->last(), both.d);
			break;
		case 109: /* CVTCOLOR(c$) */
			{
				GLERC<GLEColor> color(pass_color_var(getEvalStackStringStd(stk, stk->last())));
				setEvalStack(stk, stk->last(), color.get());
			}
			break;
		case 107: /* RGB(.4,.4,.2) */
			{
				GLERC<GLEColor> color(new GLEColor(getEvalStackDouble(stk, stk->last()-2), getEvalStackDouble(stk, stk->last()-1), getEvalStackDouble(stk, stk->last())));
				stk->decrementSize(2);
				setEvalStack(stk, stk->last(), color.get());
			}
			break;
		case 140: /* RGB255(.4,.4,.2) */
			{
				GLERC<GLEColor> color(new GLEColor());
				color->setRGB255(getEvalStackDouble(stk, stk->last()-2), getEvalStackDouble(stk, stk->last()-1), getEvalStackDouble(stk, stk->last()));
				stk->decrementSize(2);
				setEvalStack(stk, stk->last(), color.get());
			}
			break;
		case FN_BUILTIN_MAGIC + FN_RGBA:
			{
				GLERC<GLEColor> color(new GLEColor(getEvalStackDouble(stk, stk->last()-3), getEvalStackDouble(stk, stk->last()-2), getEvalStackDouble(stk, stk->last()-1), getEvalStackDouble(stk, stk->last())));
				stk->decrementSize(3);
				setEvalStack(stk, stk->last(), color.get());
			}
			break;
		case FN_BUILTIN_MAGIC + FN_RGBA255:
			{
				GLERC<GLEColor> color(new GLEColor());
				color->setRGBA255(getEvalStackDouble(stk, stk->last()-3), getEvalStackDouble(stk, stk->last()-2), getEvalStackDouble(stk, stk->last()-1), getEvalStackDouble(stk, stk->last()));
				stk->decrementSize(3);
				setEvalStack(stk, stk->last(), color.get());
			}
			break;
		case FN_BUILTIN_MAGIC + FN_NDATA: /* Number of datapoints in a dateset */
			i = get_dataset_identifier(getEvalStackStringStd(stk, stk->last()), false);
			setEvalStack(stk, stk->last(), dp[i] == 0 ? 0 : (int)dp[i]->np);
			break;
		case FN_BUILTIN_MAGIC + FN_DATAXVALUE: /* X value in a dateset */
			stk->decrementSize(1);
			i = get_dataset_identifier(getEvalStackStringStd(stk, stk->last()), true);
			j = (int) getEvalStackDouble(stk, stk->last()+1);
			if (j <= 0 || j > (int)dp[i]->np) {
				throw g_format_parser_error("index out of range: %d (1 ... %d)", j, dp[i]->np);
			} else {
				GLEArrayImpl* array = dp[i]->getDimData(0);
				if (array != 0) {
					setEvalStack(stk, stk->last(), array->getDouble(j-1));
				}
			}
			break;
		case FN_BUILTIN_MAGIC + FN_DATAYVALUE: /* Y value in a dateset */
			stk->decrementSize(1);
			i = get_dataset_identifier(getEvalStackStringStd(stk, stk->last()), true);
			j = (int) getEvalStackDouble(stk, stk->last()+1);
			if (j <= 0 || j > (int)dp[i]->np) {
				throw g_format_parser_error("index out of range: %d (1 ... %d)", j, dp[i]->np);
			} else {
				GLEArrayImpl* array = dp[i]->getDimData(1);
				if (array != 0) {
					setEvalStack(stk, stk->last(), array->getDouble(j-1));
				}
			}
			break;
		case FN_BUILTIN_MAGIC + FN_NDATASETS:
			stk->incrementSize(1);
			setEvalStack(stk, stk->last(), ndata);
			break;
		case FN_BUILTIN_MAGIC + FN_DI:
			{
				sprintf(sbuf, "D%d", getEvalStackInt(stk, stk->last()));
				GLEVars* vars = getVarsInstance();
				if (vars->getNameMode() == nameMode::DETECT || vars->getNameMode() == nameMode::RETRIEVE) {
					if (vars->getNameMode() == nameMode::DETECT) {
						vars->findAdd(sbuf, &i, &j);
						vars->setDouble(i, 0.0);
					} else {
						vars->find(sbuf, &i, &j);
					}
					if (i == -1) {
						g_throw_parser_error("no value found for data set ", sbuf, "");
					}
					vars->get(i, stk->get(stk->last()));
				} else {
					CUtilsAssert(vars->getNameMode() == nameMode::NAME);
					setEvalStack(stk, stk->last(), sbuf);
				}
			}
			break;
		case FN_BUILTIN_MAGIC + FN_XG3D:
			setEvalStack(stk, stk->last() - 2, xg3d(getEvalStackDouble(stk, stk->last() - 2), getEvalStackDouble(stk, stk->last() - 1), getEvalStackDouble(stk, stk->last())));
			stk->decrementSize(2);
			break;
		case FN_BUILTIN_MAGIC + FN_YG3D:
			setEvalStack(stk, stk->last() - 2, yg3d(getEvalStackDouble(stk, stk->last() - 2), getEvalStackDouble(stk, stk->last() - 1), getEvalStackDouble(stk, stk->last())));
			stk->decrementSize(2);
			break;
		default:  /* User function, LOCAL_START_INDEX..nnn , or error */
			/* Is it a user defined function */
			if (*(pcode+c) >= LOCAL_START_INDEX) {
				/*
				pass the address of some numbers
				pass address of variables if possible
				*/
				GLESub* sub = sub_get(*(pcode + c) - LOCAL_START_INDEX);
				getGLERunInstance()->sub_call_stack(sub, stk);
			} else {
				stringstream ss;
				ss <<"unrecognized byte code expression " <<pcode[c];
				g_throw_parser_error(ss.str());
			}
		break;
	  }
	}
}

GLESub* eval_subroutine_call(GLEArrayImpl* stk, GLEPcodeList* pclist, int *pcode, int *cp) {
	if (*(pcode+(*cp)++) != 1) {
		(*cp)--;
		gprint("PCODE, Expecting expression, v=%ld cp=%d \n", *(pcode + (*cp)), *cp);
		return NULL;
	}
	int plen = pcode[(*cp)++];
	eval_pcode_loop(stk, pclist, pcode + (*cp), plen-1);
	int sub_code = pcode[(*cp) + plen - 1];
	GLESub* result = NULL;
	if (sub_code >= LOCAL_START_INDEX) {
		result = sub_get(sub_code - LOCAL_START_INDEX);
	}
	*cp = *cp + plen;
	return result;
}

void eval_do_object_block_call(GLEArrayImpl* stk, GLESub* sub, GLEObjectDO* obj) {
	GLEObjectDOConstructor* cons = obj->getConstructor();
	obj->makePropertyStore();
	GLEArrayImpl* arr = obj->getProperties()->getArray();
	int first = 0;
	int offset = stk->last() - sub->getNbParam() + 1;
	if (cons->isSupportScale()) {
		// First two arguments are width and height
		arr->setDouble(0, getEvalStackDouble(stk, offset+0));
		arr->setDouble(1, getEvalStackDouble(stk, offset+1));
		first += 2;
	}
	for (int i = first; i < sub->getNbParam(); i++) {
		if (sub->getParamType(i) == 1) {
			ostringstream dstr;
			dstr << getEvalStackDouble(stk, offset+i);
			arr->setObject(i, new GLEString(dstr.str()));
		} else {
			GLEString* str_i = getEvalStackGLEString(stk, offset+i);
			str_i->addQuotes();
			arr->setObject(i, str_i);
		}
	}
	getGLERunInstance()->sub_call_stack(sub, stk);
}

void evalDoConstant(GLEArrayImpl* stk, int *pcode, int *cp)
{
	union {double d; int l[2];} both;
	both.l[0] = *(pcode+ ++(*cp));
	both.l[1] = 0;
	stk->incrementSize(1);
	stk->ensure(stk->last());
	stk->setDouble(stk->last(), both.d);
}

GLEMemoryCell* evalGeneric(GLEArrayImpl* stk, GLEPcodeList* pclist, int *pcode, int *cp) {
	int fixed_cp;
	if (cp == 0) {
		fixed_cp = 0;
		cp = &fixed_cp;
	}
	//cout << pcode[(*cp)] << endl;
	if (pcode[(*cp)] == 8) {
		evalDoConstant(stk, pcode, cp);
		*cp = *cp + 1;
	} else {
		if (pcode[(*cp)++] != PCODE_EXPR) {
			g_throw_parser_error("pcode error: expected expression");
		}
		int plen = pcode[(*cp)++];
		//cout << plen << endl;
		eval_pcode_loop(stk, pclist, pcode + (*cp), plen);
		*cp = *cp + plen;
	}
	if (stk->size() < 1) {
		g_throw_parser_error("pcode error: stack underflow in eval");
	}
	stk->decrementSize(1);
	return stk->get(stk->last() + 1);
}

double evalDouble(GLEArrayImpl* stk, GLEPcodeList* pclist, int *pcode, int *cp) {
	GLEMemoryCell* mc = evalGeneric(stk, pclist, pcode, cp);
	gle_memory_cell_check(mc, GLEObjectTypeDouble);
	return mc->Entry.DoubleVal;
}

bool evalBool(GLEArrayImpl* stk, GLEPcodeList* pclist, int *pcode, int *cp) {
	GLEMemoryCell* mc = evalGeneric(stk, pclist, pcode, cp);
	gle_memory_cell_check(mc, GLEObjectTypeBool);
	return mc->Entry.BoolVal;
}

GLEString* evalStringPtr(GLEArrayImpl* stk, GLEPcodeList* pclist, int *pcode, int *cp) {
	GLEMemoryCell* mc = evalGeneric(stk, pclist, pcode, cp);
	gle_memory_cell_check(mc, GLEObjectTypeString);
	return (GLEString*)mc->Entry.ObjectVal;
}

GLERC<GLEColor> evalColor(GLEArrayImpl* stk, GLEPcodeList* pclist, int *pcode, int *cp) {
	GLEMemoryCell* mc = evalGeneric(stk, pclist, pcode, cp);
	return memory_cell_to_color(get_global_polish(), stk, mc, g_get_throws_error(), 0);
}

GLERC<GLEString> evalString(GLEArrayImpl* stk, GLEPcodeList* pclist, int *pcode, int *cp, bool allowOther) {
	GLERC<GLEString> result;
	GLEMemoryCell* mc = evalGeneric(stk, pclist, pcode, cp);
	int type = gle_memory_cell_type(mc);
	if (type == GLEObjectTypeString) {
		result = (GLEString*)mc->Entry.ObjectVal;
	} else {
		if (allowOther) {
			result = stk->getString(stk->last() + 1);
		} else {
			std::ostringstream msg;
			msg << "found type '" << gle_object_type_to_string((GLEObjectType)type) << "' but expected 'string'";
			g_throw_parser_error(msg.str());
		}
	}
	return result;
}

void debug_polish(int *pcode,int *zcp)
{
	int *cp,cpval;
	int plen,c,cde;
	cpval = *zcp;
	cp = &cpval;
	if (*(pcode+(*cp)++)!=1) {
		gprint("Expecting expression, v=%d \n",(int) *(pcode+--(*cp)) );
		return;
	}
	plen = *(pcode+*(cp));
	gprint("Expression length %d current point %d \n",plen,(int) *cp);
	if (plen>1000) gprint("Expression is suspiciously int %d \n",plen);
	for (c=(*cp)+1;(c-*cp)<=plen;c++) {
	  cde = *(pcode+c);
	  gprint("Code=%d ",cde);
		if (cde==0) {
			gprint("# ZERO \n");
		} else if (cde==1) {
			gprint("# Expression, length ??? \n");
			c++;
		} else if (cde==2) {
			gprint("# Floating point number %8x \n",*(pcode+(++c)));
			c++;	/* because it's a DOUBLE which is a quad word */
		} else if (cde==3) {
			gprint("# Variable \n");  c++;
		} else if (cde==4) {
			gprint("# String Variable \n"); c++;
		} else if (cde==5) {
			c++;
			gprint("# String constant {%s} \n",eval_str(pcode,&c));
		} else if (cde<29) {
			gprint("# Binary operator {%s} \n",binop[cde-10]);
		} else if (cde<49) {
			gprint("# Binary string op {%s} \n",binop[cde-30]);
		} else if (cde<LOCAL_START_INDEX) {
			gprint("# Built in function (with salt) {%s} \n",keywfn[cde-60].word);
		} else {
			gprint("# User defined function %d \n",cde);
		}

	}
}

char *eval_str(int *pcode,int *plen)
{
	char *s;
	int sl;
	s = (char *) (pcode+*plen);
	sl = strlen(s)+1;
	sl = ((sl + 3) & 0xfffc);
	*plen = *plen + sl/4 - 1;
	return s;
}

void gle_as_a_calculator_eval(GLEPolish& polish, const string& line) {
	GLERC<GLEArrayImpl> stk(new GLEArrayImpl());
	try {
		string value;
		polish.evalString(stk.get(), line.c_str(), &value, true);
		cout << "  " << value << endl;
	} catch (ParserError& err) {
		// do not use the gprint version as no GLE file is "active"
		output_error_cerr(err);
	}
}

void gle_as_a_calculator(vector<string>* exprs) {
	g_select_device(GLE_DEVICE_DUMMY);
	g_clear();
	sub_clear(false);
	clear_run();
	f_init();
	gle_set_constants();
	GLEPolish polish;
	polish.initTokenizer();
	string line;
	if (exprs != NULL) {
		for (vector<string>::size_type i = 0; i < exprs->size(); i++) {
			cout << "> " << (*exprs)[i] << endl;
			gle_as_a_calculator_eval(polish, (*exprs)[i]);
		}
	} else {
		while (true) {
			cout << "> "; fflush(stdout);
			ReadFileLineAllowEmpty(cin, line);
			str_trim_both(line);
			if (line == "") break;
			gle_as_a_calculator_eval(polish, line);
		}
	}
}
