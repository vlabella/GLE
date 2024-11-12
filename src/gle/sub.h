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

#ifndef INCLUDE_SUB
#define INCLUDE_SUB

class GLEObjectDOConstructor;

class GLESubArgNames : public GLERefCountObject {
public:
	GLESubArgNames();
	void addArgName(const char* argName);
	void addArgName(unsigned int argIndex, const char* argName);
	void addArgNameAlias(unsigned int argIndex, const char* argName);
private:
	GLEArrayImpl m_ArgNames;
	GLEStringHashData m_ArgNameHash;
};

class GLESubRoot : public GLEDataObject {
public:
	GLESubRoot(GLEString* name, GLESubArgNames* argNames);
	void updateArgNames(GLESubArgNames* argNames);
private:
	GLERC<GLEString> m_Name;
	GLERC<GLESubArgNames> m_ArgNames;
	GLERC<GLEStringHash> m_IgnoredArgNames;
	GLERC<GLEArrayImpl> m_Signatures;
};

class GLESubDefinitionHelper : public GLERefCountObject {
public:
	GLESubDefinitionHelper(const std::string& name);
	void addArgumentAlias(unsigned int argIndex, const std::string& name);
	unsigned int addArgument(const std::string& name, unsigned int type, bool mandatory);
	unsigned int addDoubleArgument(const std::string& name, double defaultValue, bool mandatory);
	unsigned int addDoubleArgumentNoDefault(const std::string& name, bool mandatory);
	unsigned int addPointArgument(const std::string& name, GLEPointDataObject* defaultValue, bool mandatory);

public:
	std::vector<bool> m_isMandatory;
	std::vector<unsigned int> m_ArgTypes;
	GLERC<GLEArrayImpl> m_Defaults;
	GLERC<GLESubArgNames> m_ArgNames;
	GLERC<GLEString> m_Name;
};

class GLESubSignature : public GLEDataObject {
public:
	GLESubSignature(GLESubRoot* root);
private:
	GLESubRoot* m_Root;
	GLERC<GLEArrayWithFreeList> m_Subroutines;
	GLERC<GLEArrayWithFreeList> m_Callables;
};

class GLECallable : public GLEDataObject {
public:
	virtual void execute(GLEArrayImpl* stack, unsigned int top);
protected:
	unsigned int* m_ArgTypes;
};

class GLEArgTypeDefaults : public GLERefCountObject {
public:
	GLEArgTypeDefaults(unsigned int arity);
	~GLEArgTypeDefaults();
	inline unsigned int* getArgTypes() { return m_ArgTypes; }
	inline void setArgType(unsigned int i, unsigned int type) { m_ArgTypes[i] = type; }
private:
	unsigned int m_Arity;
	unsigned int* m_ArgTypes;
	GLERC<GLEArrayImpl> m_Defaults;
};

class GLEAbstractSub : public GLECallable {
public:
	GLEAbstractSub();
	inline void setRoot(GLESubRoot* root) { m_Root = root; }
	void setArgTypeDefaults(GLEArgTypeDefaults* defaults);
private:
	GLESubRoot* m_Root;
	GLERC<GLEArgTypeDefaults> m_ArgTypeDefaults;
};

class GLESubMap;

class GLEBuiltInFactory {
public:
	GLEBuiltInFactory(GLESubMap* map);
	GLESubArgNames* getBinaryArgNamesXY();
	GLEArgTypeDefaults* getBinaryDoubleDoubleArgTypeDefaults();
	GLESubRoot* createRoot(const char* name, GLESubArgNames* argNames);
private:
	GLESubMap* m_Map;
	GLERC<GLESubArgNames> m_BinaryArgNamesXY;
	GLERC<GLEArgTypeDefaults> m_BinaryDoubleDoubleArgTypeDefaults;
};

class GLEBuiltIn : public GLEAbstractSub {
public:

private:

};

class GLESub {
protected:
	std::string m_Name;
	int m_Typ, m_Idx;
	std::vector<int> m_PType;
	std::vector<std::string> m_PName;
	std::vector<std::string> m_PNameS;
	std::vector<std::string> m_PDefault;
	int m_Start, m_End;
	GLEVarMap m_LocalVars;
	GLESub* m_ParentSub;
	GLEScript* m_Script;
	GLEObjectDOConstructor* m_Cons;
	bool m_IsObject;
public:
	GLESub();
	~GLESub();
	void addParam(const std::string& name, int type);
	void setStartEnd(int start, int end);
	int findParameter(const std::string& name);
	void listArgNames(std::ostream& out);
	void clear();
	GLEObjectDOConstructor* getObjectDOConstructor();
	inline const std::string& getName() const { return m_Name; }
	inline void setName(const std::string& name) { m_Name = name; }
	inline int getStart() const { return m_Start; }
	inline void setStart(int line) { m_Start = line; }
	inline int getEnd() const { return m_End; }
	inline void setIndex(int idx) { m_Idx = idx; }
	inline int getIndex() const { return m_Idx; }
	inline GLEVarMap* getLocalVars() { return &m_LocalVars; }
	inline int getParamType(int idx) { return m_PType[idx]; }
	inline const std::string& getParamName(int idx) { return m_PName[idx]; }
	inline const std::string& getParamNameShort(int idx) { return m_PNameS[idx]; }
	inline const std::string& getDefault(int idx) const { return m_PDefault[idx]; }
	inline void setDefault(int idx, const std::string& value) { m_PDefault[idx] = value; }
	inline int getNbParam() { return m_PType.size(); }
	inline int getNbDefault() {
		// return the number of default parameters
		// these are the non-empty ones
		int ret = 0;
		for(std::string s : m_PDefault){
			if(s!="") ret++;
		}
		return ret;
	}
	inline int* getParamTypes() { return &m_PType[0]; }
	inline GLESub* getParentSub() { return m_ParentSub; }
	inline void setParentSub(GLESub* par) { m_ParentSub = par; }
	inline bool isObject() { return m_IsObject; }
	inline void setIsObject(bool isobj) { m_IsObject = isobj; }
	inline void setScript(GLEScript* script) { m_Script = script; }
	inline GLEScript* getScript() { return m_Script; }
};

class GLESubMap {
protected:
	StringIntHash m_Map;
	std::vector<GLESub*> m_Subs;
	GLERC<GLEStringHash> m_SubRoots;
public:
	GLESubMap();
	~GLESubMap();
	GLESubRoot* getRoot(const char* name);
	GLESubRoot* createRoot(const char* name, GLESubArgNames* argNames);
	void clear();
	void clear(int i);
	GLESub* add();
	GLESub* add(GLESub* parent);
	void add(GLEAbstractSub* sub);
	/* FIXME: lookup should be based on GLEString */
	GLESub* add(const std::string& name);
	GLESub* get(const std::string& name);
	void list();
	inline GLESub* get(int i) { return m_Subs[i]; }
	inline int getIndex(const std::string& name) { return m_Map.try_get(name); }
	inline int size() { return m_Subs.size(); }
};

GLESub* sub_get(int idx);
GLESub* sub_find(const std::string& s);
void sub_param(GLESub* sub, const std::string& name);

void call_sub_byname(const std::string& name, double* args, int nb, const char* err_inf);
void call_sub_byid(int idx, double* args, int nb, const char* err_inf);

#endif
