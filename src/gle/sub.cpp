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
#include "gle-interface/gle-interface.h"
#include "var.h"
#include "sub.h"
#include "gprint.h"
#include "keyword.h"
#include "run.h"
#include "cutils.h"
#include "glearray.h"
#include "polish.h"

using namespace std;

extern int **gpcode;   /* gpcode is a pointer to an array of pointer to int */
extern int *gplen;     /* gpcode is a pointer to an array of int */
extern int ngpcode;
extern int gle_debug;
#define dbg if ((gle_debug & 128)>0)

GLESubMap g_Subroutines;

GLESubArgNames::GLESubArgNames() {
}

void GLESubArgNames::addArgName(const char* argName) {
	addArgName(m_ArgNames.size(), argName);
}

void GLESubArgNames::addArgName(unsigned int argIndex, const char* argName) {
	GLERC<GLEString> argNameStr(new GLEString(argName));
	GLEStringHashData::iterator found(m_ArgNameHash.find(argNameStr));
	if (found == m_ArgNameHash.end()) {
		m_ArgNameHash.insert(make_pair(argNameStr, argIndex));
	}
	m_ArgNames.resize(argIndex + 1);
	m_ArgNames.setObject(argIndex, argNameStr.get());
}

void GLESubArgNames::addArgNameAlias(unsigned int argIndex, const char* argName) {
	GLERC<GLEString> argNameStr(new GLEString(argName));
	GLEStringHashData::iterator found(m_ArgNameHash.find(argNameStr));
	if (found == m_ArgNameHash.end()) {
		m_ArgNameHash.insert(make_pair(argNameStr, argIndex));
	}
}

GLESubRoot::GLESubRoot(GLEString* name, GLESubArgNames* argNames) :
	m_Name(name),
	m_ArgNames(argNames),
	m_IgnoredArgNames(new GLEStringHash()),
	m_Signatures(new GLEArrayImpl())
{
}

void GLESubRoot::updateArgNames(GLESubArgNames* argNames) {
}

GLESubDefinitionHelper::GLESubDefinitionHelper(const std::string& name) {
	m_Defaults = new GLEArrayImpl();
	m_ArgNames = new GLESubArgNames();
	m_Name = new GLEString(name);
}

void GLESubDefinitionHelper::addArgumentAlias(unsigned int argIndex, const std::string& name) {
	m_ArgNames->addArgNameAlias(argIndex, name.c_str());
}

unsigned int GLESubDefinitionHelper::addArgument(const std::string& name, unsigned int type, bool mandatory) {
	unsigned int nbArguments = m_ArgTypes.size();
	m_ArgTypes.push_back(type);
	m_isMandatory.push_back(mandatory);
	m_Defaults->resize(nbArguments + 1);
	m_ArgNames->addArgName(nbArguments, name.c_str());
	return nbArguments;
}

unsigned int GLESubDefinitionHelper::addDoubleArgument(const std::string& name, double defaultValue, bool mandatory) {
	unsigned int result = addArgument(name, GLEObjectTypeDouble, mandatory);
	m_Defaults->setDouble(result, defaultValue);
	return result;
}

unsigned int GLESubDefinitionHelper::addDoubleArgumentNoDefault(const std::string& name, bool mandatory) {
	return addArgument(name, GLEObjectTypeDouble, mandatory);
}

unsigned int GLESubDefinitionHelper::addPointArgument(const std::string& name, GLEPointDataObject* defaultValue, bool mandatory) {
	unsigned int result = addArgument(name, GLEObjectTypePoint, mandatory);
	if (defaultValue != 0) {
		m_Defaults->setObject(result, defaultValue);
	}
	return result;
}

GLESubSignature::GLESubSignature(GLESubRoot* root) :
	m_Root(root),
	m_Subroutines(new GLEArrayWithFreeList()),
	m_Callables(new GLEArrayWithFreeList())
{
}

void GLECallable::execute(GLEArrayImpl* stack, unsigned int top) {
}

GLEArgTypeDefaults::GLEArgTypeDefaults(unsigned int arity) :
	m_Arity(arity),
	m_ArgTypes(new unsigned int[arity]),
	m_Defaults(new GLEArrayImpl())
{
}

GLEArgTypeDefaults::~GLEArgTypeDefaults() {
	delete[] m_ArgTypes;
}

GLEAbstractSub::GLEAbstractSub() :
	m_Root(0)
{
}

void GLEAbstractSub::setArgTypeDefaults(GLEArgTypeDefaults* defaults) {
	m_ArgTypeDefaults = defaults;
	m_ArgTypes = m_ArgTypeDefaults->getArgTypes();
}

GLEBuiltInFactory::GLEBuiltInFactory(GLESubMap* map) :
	m_Map(map)
{
	// default binary argument names (x, y)
	m_BinaryArgNamesXY = new GLESubArgNames();
	m_BinaryArgNamesXY->addArgName("x");
	m_BinaryArgNamesXY->addArgName("y");
	// binary function with two double arguments
	m_BinaryDoubleDoubleArgTypeDefaults = new GLEArgTypeDefaults(2);
	m_BinaryDoubleDoubleArgTypeDefaults->setArgType(0, GLEObjectTypeDouble);
	m_BinaryDoubleDoubleArgTypeDefaults->setArgType(1, GLEObjectTypeDouble);
}

GLESubArgNames* GLEBuiltInFactory::getBinaryArgNamesXY() {
	return m_BinaryArgNamesXY.get();
}

GLEArgTypeDefaults* GLEBuiltInFactory::getBinaryDoubleDoubleArgTypeDefaults() {
	return m_BinaryDoubleDoubleArgTypeDefaults.get();
}

GLESubRoot* GLEBuiltInFactory::createRoot(const char* name, GLESubArgNames* argNames)
{
	return m_Map->createRoot(name, argNames);
}

GLESub::GLESub() {
	m_ParentSub = NULL;
	m_Typ = m_Idx = 0;
	m_Start = m_End = -1;
	m_IsObject = false;
	m_Script = NULL;
	m_Cons = NULL;
}

GLESub::~GLESub() {
}

void GLESub::addParam(const string& name, int type) {
	int len = name.length();
	if (len > 1 && name[len-1] == '$') {
		string shortname = name;
		shortname.erase(len-1);
		m_PNameS.push_back(shortname);
	} else {
		m_PNameS.push_back(name);
	}
	m_PName.push_back(name);
	m_PType.push_back(type);
	m_PDefault.push_back("");
}

void GLESub::setStartEnd(int start, int end) {
	m_Start = start;
	m_End = end;
}

void GLESub::clear() {
	m_Start = -1; m_End = -1;
	m_PName.clear();
	m_PType.clear();
	m_LocalVars.clear();
}

int GLESub::findParameter(const string& name) {
	for (int i = 0; i < getNbParam(); i++) {
		if (str_i_equals(name, getParamNameShort(i))) {
			return i;
		}
	}
	return -1;
}

void GLESub::listArgNames(ostream& out) {
	for (int i = 0; i < getNbParam(); i++) {
		if (i != 0) out << ",";
		out << getParamNameShort(i);
	}
}

GLEObjectDOConstructor* GLESub::getObjectDOConstructor() {
	if (m_Cons == NULL) m_Cons = new GLEObjectDOConstructor(this);
	return m_Cons;
}

GLESubMap::GLESubMap() :
	m_SubRoots(new GLEStringHash())
{
}

GLESubMap::~GLESubMap() {
	clear();
}

GLESubRoot* GLESubMap::getRoot(const char* name) {
	GLERC<GLEString> strName(new GLEString(name));
	return (GLESubRoot*)m_SubRoots->getObjectByKey(strName);
}

GLESubRoot* GLESubMap::createRoot(const char* name, GLESubArgNames* argNames) {
	GLERC<GLEString> strName(new GLEString(name));
	GLESubRoot* root = (GLESubRoot*)m_SubRoots->getObjectByKey(strName);
	if (root != NULL) {
		root->updateArgNames(argNames);
		return root;
	} else {
		GLESubRoot* newRoot = new GLESubRoot(strName.get(), argNames);
		m_SubRoots->setObjectByKey(strName, newRoot);
		return newRoot;
	}
}

void GLESubMap::add(GLEAbstractSub* sub) {
}

void GLESubMap::clear(int i) {
	delete m_Subs[i];
	m_Subs[i] = NULL;
}

void GLESubMap::clear() {
	for (vector<GLESub*>::size_type i = 0; i < m_Subs.size(); i++) {
		clear(i);
	}
	m_Subs.clear();
	m_Map.clear();
}

GLESub* GLESubMap::add() {
	GLESub* sub = new GLESub();
	int idx = size();
	sub->setIndex(idx);
	m_Subs.push_back(sub);
	sub->clear();
	return sub;
}

GLESub* GLESubMap::add(GLESub* parent) {
	GLESub* sub = add();
	sub->setParentSub(parent);
	return sub;
}

GLESub* GLESubMap::add(const string& name) {
	GLESub* sub = add();
	m_Map.add_item(name, sub->getIndex());
	sub->setName(name);
	return sub;
}

GLESub* GLESubMap::get(const string& name) {
	int idx = getIndex(name);
	if (idx < 0) return NULL;
	else return m_Subs[idx];
}

void GLESubMap::list() {
	cout << "List:" << endl;
	for (vector<GLESub*>::size_type i = 0; i < m_Subs.size(); i++) {
		GLESub* sub = m_Subs[i];
		cout << "  NAME = " << sub->getName() << "/" << sub->getNbParam() << endl;
	}
}

void sub_param(GLESub* sub, const string& name) {
	int vi, vt;
	/* should be set ptype according to num/string variable */
	var_add_local(name, &vi, &vt);
	sub->addParam(name, vt);
}

GLESub* sub_find(const string& s) {
	return g_Subroutines.get(s);
}

void sub_clear(bool undef) {
	if (undef) {
		for (int i = 0; i < g_Subroutines.size(); i++) {
			g_Subroutines.get(i)->setStartEnd(-1, -1);
		}
	} else {
		g_Subroutines.clear();
	}
}

bool sub_is_valid(int idx) {
	return idx >= 0 && idx < g_Subroutines.size();
}

void sub_get_startend(int idx, int *ss, int *ee) {
	GLESub* sub = g_Subroutines.get(idx);
	*ss = sub->getStart();
	*ee = sub->getEnd();
}

GLESub* sub_get(int idx) {
	if (!sub_is_valid(idx)) {
		g_throw_parser_error("illegal subroutine identifier: ", idx);
	}
	return g_Subroutines.get(idx);
}

extern int this_line;

/*
int pos = --npm;
std::cerr << "param: " << i << " from stack pos: " << pos << " = ";
gle_memory_cell_print(stk->get(pos), std::cerr);
std::cerr << std::endl;
*/

/* 	Run a user defined function  */
void GLERun::sub_call_stack(GLESub* sub, GLEArrayImpl* stk) {
	// Save current return value
	//gprint(sub->getName());
	GLEMemoryCell save_return_value;
	GLE_MC_INIT(save_return_value);
	GLE_MC_COPY(&save_return_value, &m_returnValue);
	// Set local variable map
	GLEVarMap* sub_map = sub->getLocalVars();
	GLEVarMap* save_var_map = var_swap_local_map(sub_map);
	var_alloc_local(sub_map);
	// Copy parameters to local variables
	int npm = stk->size();
	for (int i = sub->getNbParam()-1; i >= 0; i--) {
		getVars()->set(i | GLE_VAR_LOCAL_BIT, stk->get(--npm));
	}
	// Run subroutine
	int endp = 0;
	bool mkdrobjs = false;
	int oldline = this_line;
	for (int i = sub->getStart() + 1; i < sub->getEnd(); i++) {
		GLESourceLine* line = getSource()->getLine(i - 1);
		do_pcode(*line, &i, gpcode[i], gplen[i], &endp, mkdrobjs);
	}
	// FIXME: Find more elegant way to back up current line
	this_line = oldline;
	stk->decrementSize(sub->getNbParam() - 1);
	stk->ensure(npm + 1);
	stk->set(npm, &m_returnValue);
	var_set_local_map(save_var_map);
	GLE_MC_COPY(&m_returnValue, &save_return_value);
	var_free_local();
}

/* 	Run a user defined function  */
void GLERun::sub_call(GLESub* sub, GLEArrayImpl* arguments) {
	GLEMemoryCell save_return_value;
	GLE_MC_INIT(save_return_value);
	GLE_MC_COPY(&save_return_value, &m_returnValue);
	// Set local variable map
	GLEVarMap* sub_map = sub->getLocalVars();
	GLEVarMap* save_var_map = var_swap_local_map(sub_map);
	var_alloc_local(sub_map);
	// Copy parameters to local variables
	if (arguments != 0) {
		CUtilsAssert(sub->getNbParam() == (int)arguments->size());
		for (int i = 0; i < sub->getNbParam(); ++i) {
			getVars()->set(i | GLE_VAR_LOCAL_BIT, arguments->get(i));
		}
	}
	// Run subroutine
	int endp = 0;
	bool mkdrobjs = false;
	int oldline = this_line;
	for (int i = sub->getStart() + 1; i < sub->getEnd(); i++) {
		GLESourceLine* line = getSource()->getLine(i - 1);
		do_pcode(*line, &i, gpcode[i], gplen[i], &endp, mkdrobjs);
	}
	// FIXME: Find more elegant way to back up current line
	this_line = oldline;
	var_set_local_map(save_var_map);
	GLE_MC_COPY(&m_returnValue, &save_return_value);
	var_free_local();
}

void call_sub_byname(const string& name, double* args, int nb, const char* err_inf) {
	GLESub* sub = sub_find(name);
	if (sub == NULL)  {
		stringstream err;
		err << "subroutine '" << name << "' not found";
		if (err_inf != NULL) err << " " << err_inf;
		g_throw_parser_error(err.str());
	} else if (sub->getNbParam() != nb) {
		stringstream err;
		err << "subroutine '" << name << "' should take " << nb << " parameter(s), not " << sub->getNbParam();
		if (err_inf != NULL) err << " " << err_inf;
		g_throw_parser_error(err.str());
	}
	GLERC<GLEArrayImpl> stk(doublesToArray(args, nb));
	getGLERunInstance()->sub_call(sub, stk.get());
}

void call_sub_byid(int idx, double* args, int nb, const char* err_inf) {
	GLESub* sub = sub_get(idx);
	if (sub == NULL) return;
	if (sub->getNbParam() != nb) {
		stringstream err;
		err << "subroutine '" << sub->getName() << "' should take " << nb << " parameter(s), not " << sub->getNbParam();
		if (err_inf != NULL) err << " " << err_inf;
		g_throw_parser_error(err.str());
	}
	GLERC<GLEArrayImpl> stk(doublesToArray(args, nb));
	getGLERunInstance()->sub_call(sub, stk.get());
}
