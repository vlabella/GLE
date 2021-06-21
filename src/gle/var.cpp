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

#include "all.h"
#include "mem_limits.h"
#include "token.h"
#include "tokens/StringKeyHash.h"
#include "gle-interface/gle-interface.h"
#include "glearray.h"
#include "polish.h"
#include "pass.h"
#include "var.h"
#include "gprint.h"
#include "cutils.h"

#include <set>

#define true (!false)
#define false 0

GLEVars* g_VarsInstance = NULL;

GLEVars* getVarsInstance() {
	if (g_VarsInstance == NULL) g_VarsInstance = new GLEVars();
	return g_VarsInstance;
}

GLELocalVars::GLELocalVars(int num) {
	values.resize(num);
}

GLELocalVars::~GLELocalVars() {
}

void GLELocalVars::expand(int num) {
	values.ensure(num + 1);
}

void GLELocalVars::copyFrom(GLELocalVars* other, int nb) {
	expand(nb);
	for (int i = 0; i < nb; i++) {
		values.set(i, other->values.get(i));
	}
}

GLELocalVars* GLELocalVars::clone(int nb) {
	GLELocalVars* res = new GLELocalVars(nb);
	res->copyFrom(this, nb);
	return res;
}

void GLELocalVars::copyFrom(GLELocalVars* other) {
	copyFrom(other, other->size());
}

GLEVarSubMap::GLEVarSubMap(GLEVarMap* parent) {
	m_Parent = parent;
}

GLEVarSubMap::~GLEVarSubMap() {
}

void GLEVarSubMap::list() {
	for (StringIntHash::const_iterator i(m_Map.begin()); i != m_Map.end(); ++i) {
		std::cout << i->first << std::endl;
	}
}

void GLEVarSubMap::clear() {
	m_Map.clear();
	m_Idxs.clear();
}

void GLEVarSubMap::var_add(const string& name, int idx) {
	m_Map.add_item(name, idx);
	m_Idxs.push_back(idx);
}

void GLEVarSubMap::removeFromParent() {
	// remove indices and names from varlist (indicate them as empty)
	for (vector<int>::size_type i = 0; i < m_Idxs.size(); i++) {
		m_Parent->removeVar(m_Idxs[i]);
	}
}

void GLEVarSubMap::addToParent(GLEVarMap* parent) {
	m_Parent = parent;
	m_Parent->addVars(m_Map);
}

GLEVarBackup::GLEVarBackup() {
}

void GLEVarBackup::backup(GLEVars* vars, const vector<int>& ids) {
	GLEMemoryCell data;
	GLE_MC_INIT(data);
	m_ids.assign(ids.begin(), ids.end());
	m_values.resize(ids.size());
	for (size_t i = 0; i < ids.size(); ++i) {
		vars->get(ids[i], &data);
		m_values.set(i, &data);
	}
}

void GLEVarBackup::restore(GLEVars* vars) {
	for (size_t i = 0; i < m_ids.size(); ++i) {
		vars->set(m_ids[i], m_values.get(i));
	}
}

GLEVarMap::GLEVarMap() {
	m_IsTemp = false;
}

GLEVarMap::~GLEVarMap() {
	clear();
}

int GLEVarMap::getFreeID() {
	if (m_Free.size() > 0) {
		int id = m_Free.back();
		m_Free.pop_back();
		return id;
	} else {
		return -1;
	}
}

int GLEVarMap::addVarIdx(const string& name) {
	int idx = getFreeID();
	int type = str_var(name) ? 2 : 1;
	if (idx != -1) {
		m_Names[idx] = name;
		m_Types[idx] = type;
	} else {
		idx = m_Names.size();
		m_Names.push_back(name);
		m_Types.push_back(type);
	}
	return idx;
}

void GLEVarMap::addVars(const StringIntHash& submap) {
	set<int> free(m_Free.begin(), m_Free.end());
	for (StringIntHash::const_iterator i(submap.begin()); i != submap.end(); ++i) {
		int idx = i->second;
		set<int>::const_iterator found(free.find(idx));
		if (found == free.end()) {
			unsigned int origSize = m_Names.size();
			if (idx >= (int)origSize) {
				unsigned int newSize = idx + 1;
				m_Names.resize(newSize, "?");
				m_Types.resize(newSize, 0);
				for (unsigned int j = origSize; j < newSize - 1; j++) {
					free.insert(j);
				}
				string name(i->first);
				int type = str_var(name) ? 2 : 1;
				m_Names[idx] = name;
				m_Types[idx] = type;
			} else {
				ostringstream err;
				err << "GLE internal error: variable not free when adding submap (name = " << i->first << ", id = " << idx << ")";
				g_throw_parser_error(err.str());
			}
		} else {
			free.erase(found);
			string name(i->first);
			int type = str_var(name) ? 2 : 1;
			m_Names[idx] = name;
			m_Types[idx] = type;
		}
	}
	m_Free.assign(free.begin(), free.end());
}

int GLEVarMap::var_find_add(const string& name, bool* isnew) {
	*isnew = false;
	int idx = m_Map.try_get(name);
	if (idx == -1) {
		idx = addVarIdx(name);
		m_Map.add_item(name, idx);
		*isnew = true;
	}
	return idx;
}

int GLEVarMap::var_find_add_submap(const string& name, bool* isnew) {
	*isnew = false;
	GLEVarSubMap* sub = m_SubMap.back();
	int idx = sub->var_get(name);
	if (idx == -1) {
		idx = addVarIdx(name);
		sub->var_add(name, idx);
		*isnew = true;
	}
	return idx;
}

int GLEVarMap::var_get(const string& name) {
	int idx = -1;
	int submap = m_SubMap.size()-1;
	while (submap >= 0 && (idx = m_SubMap[submap]->var_get(name)) == -1) {
		submap--;
	}
	if (idx != -1) return idx;
	else return m_Map.try_get(name);
}

const string& GLEVarMap::var_name(int idx) {
	return m_Names[idx];
}

void GLEVarMap::list() {
	for (vector<string>::size_type i = 0; i < m_Names.size(); i++) {
		if (m_Types[i] != -1) {
			cout << m_Names[i] << " (" << i << ")" << endl;
		}
	}
}

void GLEVarMap::removeVar(int idx) {
	m_Free.push_back(idx);
	m_Names[idx] = "?";
	m_Types[idx] = -1;
}

GLEVarSubMap* GLEVarMap::pushSubMap() {
	GLEVarSubMap* sub = new GLEVarSubMap(this);
	m_SubMap.push_back(sub);
	return sub;
}

void GLEVarMap::pushSubMap(GLEVarSubMap* submap) {
	submap->addToParent(this);
	m_SubMap.push_back(submap);
}

void GLEVarMap::popSubMap() {
	GLEVarSubMap* sub = m_SubMap.back();
	sub->removeFromParent();
	m_SubMap.pop_back();
}

void GLEVarMap::clearSubMaps() {
	m_SubMap.clear();
}

void GLEVarMap::clear() {
	m_Names.clear();
	m_Types.clear();
	m_Map.clear();
	m_Free.clear();
	clearSubMaps();
}

GLEVars::GLEVars() {
	m_LocalMap = NULL;
	local_var = NULL;
	local_var_stack_level = 0;
	m_nameMode = nameMode::NAME;
}

GLEVars::~GLEVars() {
}

bool GLEVars::check(int *j) {
	int var = *j;
	/* convert var index and return true if var is local */
	if (GLE_VAR_IS_LOCAL(var)) {
		var &= ~GLE_VAR_LOCAL_BIT;
		if (m_LocalMap == NULL) {
			gprint("No local variables assigned");
			*j = 0;
		} else if (var < 0 || var >= m_LocalMap->size() || var >= NUM_LOCAL) {
			gprint("Local variable index out of range: %d is not in 0-%d", var, m_LocalMap->size());
			*j = 0;
		} else {
			*j = var;
			return true;
		}
	} else {
		if (var < 0 || var >= m_GlobalMap.size()) {
			gprint("Global variable index out of range: %d is not in 0-%d", var, m_GlobalMap.size());
			*j = 0;
		}
	}
	return false;
}

char* GLEVars::getName(int var) {
	if (check(&var)) {
		return (char*)m_LocalMap->var_name(var).c_str();
	} else {
		return (char*)m_GlobalMap.var_name(var).c_str();
	}
}

double GLEVars::getDouble(int var) {
	if (check(&var)) {
		return local_var->values.getDouble(var);
	} else {
		return m_Global.getDouble(var);
	}
}

void GLEVars::setDouble(int var, double v) {
	if (check(&var)) {
		local_var->values.setDouble(var, v);
	} else {
		m_Global.setDouble(var, v);
	}
}

void GLEVars::set(int var, GLEMemoryCell* value) {
	if (check(&var)) {
		local_var->values.set(var, value);
	} else {
		m_Global.set(var, value);
	}
}

void GLEVars::get(int var, GLEMemoryCell* value) {
	if (check(&var)) {
		GLE_MC_COPY(value, local_var->values.get(var));
	} else {
		GLE_MC_COPY(value, m_Global.get(var));
	}
}

GLEString* GLEVars::getString(int var) {
	GLEDataObject* object = 0;
	if (check(&var)) {
		object = local_var->values.getObject(var);
	} else {
		object = m_Global.getObject(var);
	}
	if (object != 0 && object->getType() == GLEObjectTypeString) {
		return (GLEString*)object;
	} else {
		return new GLEString();
	}
}

GLEDataObject* GLEVars::getObject(int var) {
	if (check(&var)) {
		return NULL;
	} else {
		return (GLEDataObject*)m_Global.getObject(var);
	}
}

void GLEVars::setString(int var, GLEString* s) {
	if (check(&var)) {
		local_var->values.setObject(var, s);
	} else {
		m_Global.setObject(var, s);
	}
}

void GLEVars::setObject(int var, GLEDataObject* obj) {
	if (check(&var)) {
		// FIXME: no support for objects in local vars
	} else {
		m_Global.setObject(var, obj);
	}
}

void GLEVars::init(int var, int type) {
	if (check(&var)) {
		if (type == 2) local_var->values.setObject(var, new GLEString());
		else local_var->values.setDouble(var, 0.0);
	} else {
		if (type == 2) m_Global.setObject(var, new GLEString());
		else m_Global.setDouble(var, 0.0);
	}
}

// First search in local vars, then in global vars
void GLEVars::find(const std::string& name, int *idx, int *type) {
	*idx = -1;
	if (m_LocalMap != NULL) {
		int l_idx = m_LocalMap->var_get(name);
		if (l_idx != -1) {
			*type = m_LocalMap->getType(l_idx);
			*idx = l_idx | GLE_VAR_LOCAL_BIT;
			return;
		}
	}
	int g_idx = m_GlobalMap.var_get(name);
	if (g_idx != -1) {
		*type = m_GlobalMap.getType(g_idx);
		*idx = g_idx;
	}
}

void GLEVars::findAdd(const char *name, int *idx, int *type) {
	bool isnew;
	if (m_LocalMap != NULL && m_LocalMap->hasSubMap()) {
		// cout << "adding to submap: " << name << endl;
		int l_idx = m_LocalMap->var_find_add_submap(name, &isnew);
		*type = m_LocalMap->getType(l_idx);
		*idx = l_idx | GLE_VAR_LOCAL_BIT;
		local_var->expand(l_idx);
		if (isnew) init(*idx, *type);
	} else {
		if (m_LocalMap != NULL) {
			// cout << "adding to local: " << name << endl;
			int l_idx = m_LocalMap->var_get(name);
			if (l_idx != -1) {
				*type = m_LocalMap->getType(l_idx);
				*idx = l_idx | GLE_VAR_LOCAL_BIT;
				return;
			}
		}
		string name_s = name;
		*idx = m_GlobalMap.var_find_add(name_s, &isnew);
		*type = m_GlobalMap.getType(*idx);
		// cout << "adding to global: " << name << " idx = " << *idx << " type = " << *type << " new = " << isnew << endl;
		if (isnew) {
			expandGlobalVars(*idx);
			init(*idx, *type);
		}
	}
}

void GLEVars::clear() {
	clearGlobal();
	clearLocal();
}

void GLEVars::clearGlobal() {
	m_GlobalMap.clear();
}

void GLEVars::clearLocal() {
	m_LocalMap = NULL;
}

int GLEVars::getNbLocal() {
	return m_LocalMap == NULL ? 0 : m_LocalMap->size();
}

void GLEVars::allocLocal(int num) {
	local_var_stack_level++;
	if (local_var_stack_level >= (int)local_var_stack.size()) {
		if (local_var_stack_level == 1) {
			// Initialize first level
			local_var_stack.push_back(NULL);
		}
		local_var = new GLELocalVars(num);
		local_var_stack.push_back(local_var);
	} else {
		local_var = local_var_stack[local_var_stack_level];
		local_var->expand(num);
	}
}

void GLEVars::freeLocal() {
	if (local_var_stack_level == 0) {
		cerr << "GLE internal error: too many pops of local variable stack" << endl;
		exit(1);
	}
	local_var_stack_level--;
	local_var = local_var_stack[local_var_stack_level];
}

GLEVarMap* GLEVars::swapLocalMap(GLEVarMap* map) {
	GLEVarMap* old = m_LocalMap;
	m_LocalMap = map;
	return old;
}

void GLEVars::addLocal(const string& name, int *idx, int *type) {
	bool isnew;
	int l_idx = m_LocalMap->var_find_add(name, &isnew);
	*type = m_LocalMap->getType(l_idx);
	*idx = l_idx | GLE_VAR_LOCAL_BIT;
}

GLEVarSubMap* GLEVars::addLocalSubMap() {
	if (m_LocalMap == NULL) {
		m_LocalMap = new GLEVarMap();
		m_LocalMap->setTemp(true);
		var_alloc_local(0);
	}
	return m_LocalMap->pushSubMap();
}

void GLEVars::addLocalSubMap(GLEVarSubMap* submap) {
	if (m_LocalMap == NULL) {
		m_LocalMap = new GLEVarMap();
		m_LocalMap->setTemp(true);
		var_alloc_local(0);
	}
	m_LocalMap->pushSubMap(submap);
}

void GLEVars::removeLocalSubMap() {
	if (m_LocalMap != NULL) {
		if (m_LocalMap->isTemp()) {
			delete m_LocalMap;
			m_LocalMap = NULL;
			var_free_local();
		} else {
			m_LocalMap->popSubMap();
		}
	}
}

void GLEVars::findDN(GLEVarSubMap* map, int *idx, int *var, int *nd) {
	*nd = 0;
	for (int i = 0; i < map->size(); i++) {
		int lidx = map->get(i);
		const string& name = m_LocalMap->var_name(lidx);
		if (str_ni_equals(name.c_str(), "D", 1)) {
			int d = atoi(name.c_str()+1);
			// cout << "name = " << name << " idx = " << d << endl;
			// FIXME: calling code supports at most 10 data sets
			if (d > 0 && d < MAX_NB_DATA && (*nd) < 10) {
				*idx = lidx | GLE_VAR_LOCAL_BIT;
				*var = d;
				(*nd)++; idx++; var++;
			}
		}
	}
	// cout << "res = " << *nd << endl;
}

const char* GLEVars::getObjectTypeName(int type) {
	switch (type) {
		case GLEObjectTypeBool:
			return "boolean";
		case GLEObjectTypeInt:
			return "int";
		case GLEObjectTypeDouble:
			return "double";
		case GLEObjectTypeString:
			return "string";
		case GLEObjectTypeArray:
			return "array";
		case GLEObjectTypeObjectRep:
			return "object";
		case GLEObjectTypeDynamicSub:
			return "subroutine";
	}
	return "unknown";
}

string GLEVars::typeError(int var, int type) {
	stringstream str;
	if (check(&var)) {
		str << "local variable '" << m_LocalMap->var_name(var) << "' has unknown type";
	} else {
		str << "global variable '" << m_GlobalMap.var_name(var);
		str << "' of incorrect type: " << getObjectTypeName(m_Global.getType(var));
		str << " <> " << getObjectTypeName(type);
	}
	return str.str();
}

int var_type(const string& name) {
	return str_var(name) ? 2 : 1;
}

bool str_var(const char* s) {
	int i = strlen(s);
	if (*(s+i-1)=='$') return true;
	else return false;
}

bool str_var(const string& s) {
	if (s[s.length()-1] == '$') return true;
	else return false;
}

int valid_var(const char* s) {
	return true;
}

bool str_var_valid_name(const string& name) {
   return str_var(name) && var_valid_name(name);
}

bool var_valid_name(const string& name) {
	if (name.length() <= 0) {
		return false;
	}
	if (name[0] >= '0' && name[0] <= '9') {
		return false;
	}
	for (string::size_type i = 0; i < name.length(); i++) {
		char ch = name[i];
		if (!((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') ||
		      (ch >= '0' && ch <= '9') || (ch == '$') || (ch == '_'))) {
			return false;
		}
	}
   return true;
}

void ensure_valid_var_name(const string& name) throw(ParserError) {
	if (!var_valid_name(name)) {
		g_throw_parser_error("illegal variable name '", name.c_str(), "'");
	}
}

void ensure_valid_var_name(Tokenizer* tokens, const string& name) throw(ParserError) {
	if (!var_valid_name(name)) {
		throw tokens->error(string("illegal variable name '")+name+"'");
	}
}

/*
 * DEPRECATED CODE
 */

bool var_check(int *j) {
	return getVarsInstance()->check(j);
}

GLELocalVars* get_local_vars() {
	return getVarsInstance()->getLocalVars();
}

void var_alloc_local(int num) {
	getVarsInstance()->allocLocal(num);
}

void var_alloc_local(GLEVarMap* map) {
	getVarsInstance()->allocLocal(map);
}

void var_free_local() {
	getVarsInstance()->freeLocal();
}

void var_set(int var, GLEMemoryCell* value) {
	getVarsInstance()->set(var, value);
}

void var_set(int var, double v) {
	getVarsInstance()->setDouble(var, v);
}

void var_get(int var, double *v) {
	*v = getVarsInstance()->getDouble(var);
}

void var_setstr(int var, char *s) {
	GLERC<GLEString> str = new GLEString(s);
	getVarsInstance()->setString(var, str.get());
}

void var_getstr(int var, char *s) {
	GLERC<GLEString> str = getVarsInstance()->getString(var);
	str->toUTF8(s);
}

void var_getstr(int var, string& s) {
	GLERC<GLEString> str = getVarsInstance()->getString(var);
	str->toUTF8(s);
}

char* var_get_name(int var) {
	return getVarsInstance()->getName(var);
}

void var_nlocal(int *l) {
	*l = getVarsInstance()->getNbLocal();
}

void var_clear_global() {
	getVarsInstance()->clearGlobal();
}

void var_clear() {
	getVarsInstance()->clear();
}

GLEVarMap* var_swap_local_map(GLEVarMap* map) {
	return getVarsInstance()->swapLocalMap(map);
}

void var_set_local_map(GLEVarMap* map) {
	getVarsInstance()->setLocalMap(map);
}

void var_clear_local() {
	getVarsInstance()->clearLocal();
}

bool has_local_var_map() {
	return getVarsInstance()->hasLocalMap();
}

GLEVarMap* get_local_var_map() {
	return getVarsInstance()->getLocalMap();
}

void var_findadd_set(const char* name, double value) {
	int idx, type = 1;
	var_findadd(name, &idx, &type);
	var_set(idx, value);
}

void var_findadd_set(const char* name, const string& value) {
	int idx, type = 2;
	var_findadd((char*)name, &idx, &type);
	var_setstr(idx, (char*)value.c_str());
}

void var_findadd_set(const char *s, GLEMemoryCell* value) {
	int idx, type=1;
	var_findadd(s, &idx, &type);
	getVarsInstance()->set(idx, value);
}

void var_add_local(const string& name, int *idx, int *type) {
	getVarsInstance()->addLocal(name, idx, type);
}

void var_findadd(const char *name, int *idx, int *type) {
	getVarsInstance()->findAdd(name, idx, type);
}

// First search in local vars, then in global vars
void var_find(const char *name, int *idx, int *type) {
	getVarsInstance()->find(name, idx, type);
}

void var_find_dn(GLEVarSubMap* map, int *idx, int *var, int *nd) {
	return getVarsInstance()->findDN(map, idx, var, nd);
}
