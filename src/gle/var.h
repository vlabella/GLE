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

#ifndef INCLUDE_VAR
#define INCLUDE_VAR

#include "gle-interface/gle-datatype.h"

#define GLE_VAR_LOCAL_BIT 0x10000000

#define GLE_VAR_IS_LOCAL(a) (a & GLE_VAR_LOCAL_BIT) != 0

class GLEVars;
class GLEVarMap;

class GLEVarSubMap : public GLERefCountObject {
protected:
	StringIntHash m_Map;
	vector<int> m_Idxs;
	GLEVarMap* m_Parent;
public:
	GLEVarSubMap(GLEVarMap* parent);
	~GLEVarSubMap();
	void list();
	void clear();
	void removeFromParent();
	void addToParent(GLEVarMap* parent);
	void var_add(const string& name, int idx);
	inline int var_get(const string& name) { return m_Map.try_get(name); }
	inline int size() { return m_Idxs.size(); }
	inline int get(int i) { return m_Idxs[i]; }
};

class GLEVarBackup : public GLERefCountObject {
public:
	GLEVarBackup();
	void backup(GLEVars* vars, const vector<int>& ids);
	void restore(GLEVars* vars);
private:
	vector<int> m_ids;
	GLEArrayImpl m_values;
};

class GLEVarMap {
protected:
	vector<string> m_Names;
	vector<int> m_Types;
	StringIntHash m_Map;
	vector<int> m_Free;
	vector<GLEVarSubMap*> m_SubMap;
	bool m_IsTemp;
public:
	GLEVarMap();
	~GLEVarMap();
	/* FIXME: lookup should be based on GLEString */
	int var_find_add(const string& name, bool* isnew);
	int var_find_add_submap(const string& name, bool* isnew);
	int var_get(const string& name);
	const string& var_name(int idx);
	void clear();
	void list();
	int getFreeID();
	int addVarIdx(const string& name);
	void addVars(const StringIntHash& submap);
	void addVar(int idx, const string& name);
	void removeVar(int idx);
	void clearSubMaps();
	GLEVarSubMap* pushSubMap();
	void pushSubMap(GLEVarSubMap* submap);
	void popSubMap();
	inline int size() { return m_Names.size(); }
	inline int getType(int idx) { return m_Types[idx]; }
	inline void setType(int idx, int type) { m_Types[idx] = type; }
	inline bool hasSubMap() { return m_SubMap.size() > 0; }
	inline bool isTemp() { return m_IsTemp; }
	inline void setTemp(bool temp) { m_IsTemp = temp; }
};

class GLELocalVars {
public:
	GLEArrayImpl values;
public:
	GLELocalVars(int num);
	~GLELocalVars();
	void expand(int num);
	void copyFrom(GLELocalVars* other);
	void copyFrom(GLELocalVars* other, int nb);
	GLELocalVars* clone(int nb);
	inline int size() { return values.size(); }
};

namespace nameMode
{
	enum Enum {
		DETECT,
		RETRIEVE,
		NAME
	};
};

class GLEVars {
protected:
	GLEVarMap m_GlobalMap;
	GLEVarMap* m_LocalMap;
	GLEArrayImpl m_Global;
	GLEArrayImpl m_Stack;
	GLELocalVars* local_var;
	vector<GLELocalVars*> local_var_stack;
	int local_var_stack_level;
	nameMode::Enum m_nameMode;
public:
	GLEVars();
	~GLEVars();
	bool check(int *var);
	char* getName(int var);
	double getDouble(int var);
	void setDouble(int var, double v);
	GLEString* getString(int var);
	void setString(int var, GLEString* s);
	GLEDataObject* getObject(int var);
	void setObject(int var, GLEDataObject* obj);
	void set(int var, GLEMemoryCell* value);
	void get(int var, GLEMemoryCell* value);
	void init(int var, int type);
	void find(const std::string& name, int *idx, int *type);
	void findAdd(const char *name, int *idx, int *type);
	void clear();
	void clearGlobal();
	void clearLocal();
	int getNbLocal();
	void allocLocal(int num);
	void freeLocal();
	void addLocal(const string& name, int *idx, int *type);
	GLEVarMap* swapLocalMap(GLEVarMap* map);
	GLEVarSubMap* addLocalSubMap();
	void addLocalSubMap(GLEVarSubMap* submap);
	void removeLocalSubMap();
	void findDN(GLEVarSubMap* map, int *idx, int *var, int *nd);
	int getGlobalType(int var);
	const char* getObjectTypeName(int type);
	string typeError(int var, int type);
	inline void allocLocal(GLEVarMap* map) { allocLocal(map->size()); }
	inline GLELocalVars* getLocalVars() { return local_var; }
	inline bool hasLocalMap() { return m_LocalMap != NULL; }
	inline GLEVarMap* getLocalMap() { return m_LocalMap; }
	inline GLEVarMap* getGlobalMap() { return &m_GlobalMap; }
	inline void setLocalMap(GLEVarMap* map) { m_LocalMap = map; }
	inline void expandGlobalVars(int max) { m_Global.ensure(max+1); }
	inline void setNameMode(nameMode::Enum mode) { m_nameMode = mode; }
	inline nameMode::Enum getNameMode() const { return m_nameMode; }
};

GLEVars* getVarsInstance();

void var_init(int idx, int type);
int var_type(const string& name);
bool str_var(const string& s);
bool str_var(const char* s);
int valid_var(const char *s);
bool has_local_var_map();
GLEVarMap* get_local_var_map();
void var_add_local(const string& name, int *idx, int *type);
GLELocalVars* get_local_vars();
void var_alloc_local(int num);
void var_alloc_local(GLEVarMap* map);
bool var_check(int *j);
void var_clear_global(void);
void var_clear_local(void);
void var_clear();
void var_find(const char *name,int *idx,int *type);	/* Find a variable in the list */
void var_find_dn(GLEVarSubMap* map, int *idx, int *var, int *nd);
void var_findadd(const char *name,int *idx,int *type);	/* Add a variable to the list */
void var_free_local(void);
void var_get(int jj, double *v);
void var_getstr(int v, char *s);
void var_getstr(int jj, string& s);
void var_nlocal(int *l);
void var_set(int jj, double v);
void var_set(int jj, GLEMemoryCell* value);
GLEVarMap* var_swap_local_map(GLEVarMap* map);
void var_set_local_map(GLEVarMap* map);
void var_setstr(int jj, char *s);
char* var_get_name(int jj);
bool var_valid_name(const string& name);
void var_findadd_set(const char* name, double value);
void var_findadd_set(const char* name, const string& value);
void var_findadd_set(const char *s, GLEMemoryCell* value);
GLEVarSubMap* var_add_local_submap();
void var_remove_local_submap();
bool str_var_valid_name(const string& name);
void ensure_valid_var_name(const string& name) throw(ParserError);
void ensure_valid_var_name(Tokenizer* tokens, const string& name) throw(ParserError);

#endif
