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

#ifndef __GLE_DATATYPE__
#define __GLE_DATATYPE__

class GLEArrayImpl;
class GLESub;
class GLELocalVars;
class gmodel;

typedef enum {
	GLEObjectTypeUnknown,
	GLEObjectTypeBool,
	GLEObjectTypeInt,
	GLEObjectTypeDouble,
	GLEObjectTypeString,
	GLEObjectTypeArray,
	GLEObjectTypeColor,
	GLEObjectTypeDynamicSub,
	GLEObjectTypeObjectRep,
	GLEObjectTypePoint,
	GLEObjectTypeClassDefinition,
	GLEObjectTypeClassInstance
} GLEObjectType;

const char* gle_object_type_to_string(GLEObjectType type);

class GLEDataObject : public GLERefCountObject {
public:
	GLEDataObject();
	virtual ~GLEDataObject();
	virtual int getType() const;
	virtual bool equals(GLEDataObject* obj) const;
	virtual void print(ostream& out) const;
};

typedef union {
	bool           BoolVal;
	int            IntVal;
	double         DoubleVal;
	GLEDataObject *ObjectVal;
} GLEMemoryCellEntry;

typedef enum {
	GLE_MC_UNKNOWN,
	GLE_MC_BOOL,
	GLE_MC_INT,
	GLE_MC_DOUBLE,
	GLE_MC_OBJECT
} GLEMemoryCellType;

typedef struct {
	GLEMemoryCellType Type;
	GLEMemoryCellEntry Entry;
} GLEMemoryCell;

#define GLE_MC_ISA(a,b) (a->Type == b)
#define GLE_MC_INIT(a)  a.Type = GLE_MC_UNKNOWN

inline void GLE_MC_DEL_INTERN(GLEMemoryCell* a) {
	if (a->Type == GLE_MC_OBJECT) {
		GLE_DEL_RC_INTERN(a->Entry.ObjectVal);
	}
}

inline void GLE_MC_SET_DOUBLE(GLEMemoryCell* a, double v) {
	GLE_MC_DEL_INTERN(a);
	a->Entry.DoubleVal = v;
	a->Type = GLE_MC_DOUBLE;
}

inline void GLE_MC_SET_INT(GLEMemoryCell* a, int v) {
	GLE_MC_DEL_INTERN(a);
	a->Entry.IntVal = v;
	a->Type = GLE_MC_INT;
}

inline void GLE_MC_SET_BOOL(GLEMemoryCell* a, bool v) {
	GLE_MC_DEL_INTERN(a);
	a->Entry.BoolVal = v;
	a->Type = GLE_MC_BOOL;
}

inline void GLE_MC_SET_UNKNOWN(GLEMemoryCell* a) {
	GLE_MC_DEL_INTERN(a);
	a->Type = GLE_MC_UNKNOWN;
}

inline void GLE_MC_SET_OBJECT(GLEMemoryCell* a, GLEDataObject* v) {
	if (a->Type == GLE_MC_OBJECT) {
		a->Entry.ObjectVal = (GLEDataObject*)GLE_SET_RC(a->Entry.ObjectVal, v);
	} else {
		a->Entry.ObjectVal = (GLEDataObject*)GLE_INIT_RC(v);
	}
	a->Type = GLE_MC_OBJECT;
}

inline void GLE_MC_COPY(GLEMemoryCell* a, GLEMemoryCell* b) {
	if (b->Type == GLE_MC_OBJECT) {
		GLE_MC_SET_OBJECT(a, b->Entry.ObjectVal);
	} else {
		GLE_MC_DEL_INTERN(a);
		a->Entry = b->Entry;
		a->Type = b->Type;
	}
}

bool gle_memory_cell_equals(GLEMemoryCell* a, GLEMemoryCell* b);
void gle_memory_cell_print(GLEMemoryCell* a, ostream& out);
bool gle_memory_cell_to_double(GLEMemoryCell* a, double* result);
void gle_memory_cell_check(GLEMemoryCell* a, int expected);
int gle_memory_cell_type(GLEMemoryCell* a);

unsigned int getUTF8NumberOfChars(const char* str, unsigned int len);

class DLLCLASS GLEPointDataObject : public GLEDataObject {
protected:
	GLEPoint m_point;
public:
	GLEPointDataObject(double x, double y);
	virtual ~GLEPointDataObject();
	virtual int getType() const;
	virtual bool equals(GLEDataObject* obj) const;
	virtual void print(ostream& out) const;
};

class DLLCLASS GLEString : public GLEDataObject {
protected:
	unsigned int* m_Data;
	unsigned int m_Length;
	unsigned int m_Alloc;
	unsigned int m_Intern;
public:
	GLEString();
	GLEString(const char* utf8);
	GLEString(const string& utf8);
	virtual ~GLEString();
	inline unsigned int length() const { return m_Length; }
	inline unsigned int get(unsigned int i) const { return m_Data[i]; }
	inline void set(unsigned int i, unsigned int v) { m_Data[i] = v; }
	unsigned int getI(unsigned int i) const;
	bool isSmallerThanI(const GLEString* s2) const;
	bool equalsI(const char* str);
	bool equalsI(GLEString* other);
	bool containsI(unsigned int ch);
	int strICmp(GLEString* other) const;
	virtual bool equals(GLEDataObject* obj) const;
	virtual void print(ostream& out) const;
	void fromUTF8(const char* str);
	void fromUTF8(const string& str);
	void fromUTF8(const char* str, unsigned int len);
	ostream& toUTF8(ostream& out) const;
	void toUTF8(string& out) const;
	void toUTF8(char* out) const;
	string toUTF8() const;
	GLEString* concat(GLEString* other) const;
	GLEString* substringWithLength(unsigned int from, unsigned int size) const;
	GLEString* substring(unsigned int from, unsigned int to) const;
	int find(GLEString* needle, unsigned int from);
	GLEArrayImpl* split(char bych) const;
	void addQuotes();
	void join(char bych, GLEArrayImpl* arr, int from = 0, int to = -1);
	void resize(unsigned int size);
	unsigned int toStringIndex(int value);
	void DLLFCT setSize(unsigned int size);
	virtual int getType() const;
	static GLEString* getEmptyString();
};

inline ostream& operator<<(ostream& os, const GLEString& pos) {
	return pos.toUTF8(os);
}

typedef GLERC<GLEString> GLEStringKey;

struct GLEStringCompare {
	bool operator()(const GLEStringKey& s1, const GLEStringKey& s2) const {
		return s1->isSmallerThanI(s2.get());
	}
};

class GLEArray : public GLEDataObject {
public:
	GLEArray();
	virtual ~GLEArray();
};

class DLLCLASS GLEArrayImpl : public GLEArray {
protected:
	GLEMemoryCell* m_Data;
	unsigned int m_Length;
	unsigned int m_Alloc;
public:
	GLEArrayImpl();
	virtual ~GLEArrayImpl();
	void clear();
	void set(unsigned int i, const GLEMemoryCell* cell);
	inline GLEMemoryCell* get(unsigned int i) const { return &m_Data[i]; }
	inline GLEDataObject* getObjectUnsafe(unsigned int i) { return m_Data[i].Entry.ObjectVal; }
	inline unsigned int size() const { return m_Length; }
	inline unsigned int last() const { return m_Length - 1; }
	inline void decrementSize(int diff) { m_Length -= diff; }
	inline void incrementSize(int diff) { m_Length += diff; }
	void init(unsigned int i);
	int getType(unsigned int i) const;
	void checkType(unsigned int i, int expected);
	double DLLFCT getDouble(unsigned int i);
	void DLLFCT setDouble(unsigned int i, double v);
	int DLLFCT getInt(unsigned int i);
	void DLLFCT setInt(unsigned int i, int v);
	bool DLLFCT getBool(unsigned int i);
	void DLLFCT setBool(unsigned int i, bool v);
	void DLLFCT setUnknown(unsigned int i);
	bool DLLFCT isUnknown(unsigned int i);
	void DLLFCT setObject(unsigned int i, GLEDataObject* v);
	DLLFCT GLEDataObject* getObject(unsigned int i);
	DLLFCT GLERC<GLEString> getString(unsigned int i);
	void addObject(GLEDataObject* v);
	void addInt(int v);
	void ensure(unsigned int size);
	void resize(unsigned int size);
	void extend(unsigned int size);
	void enumStrings(ostream& out);
	virtual int getType() const;
private:
	void resizeMemory(unsigned int size);
};

GLERC<GLEArrayImpl> doublesToArray(double* args, int nb);

class DLLCLASS GLEArrayWithFreeList : public GLEArrayImpl {

};

typedef map<GLEStringKey, unsigned int, GLEStringCompare> GLEStringHashData;

class GLEStringHash : public GLEArrayImpl {
protected:
	GLEStringHashData m_Map;
	// FIXME: add freelist
public:
	GLEStringHash();
	virtual ~GLEStringHash();
	void setObjectByKey(const GLEStringKey& key, GLEDataObject* v);
	GLEDataObject* getObjectByKey(const GLEStringKey& key);
	void getKeys(GLEArrayImpl* keys);
	inline GLEStringHashData* getHash() { return &m_Map; }
};

class GLEDynamicSub : public GLEDataObject {
protected:
	GLESub* m_Sub;
	GLELocalVars* m_VarValues; // FIXME: replace by GLEArrayImpl
	                           // to store values of local variables
	gmodel* m_State;
public:
	GLEDynamicSub();
	GLEDynamicSub(GLESub* sub);
	virtual ~GLEDynamicSub();
	virtual int getType() const;
	inline GLESub* getSub() { return m_Sub; }
	inline void setSub(GLESub* sub) { m_Sub = sub; }
	inline GLELocalVars* getLocalVars() { return m_VarValues; }
	inline void setLocalVars(GLELocalVars* vars) { m_VarValues = vars; }
	inline void setState(gmodel* state) { m_State = state; }
	inline gmodel* getState() { return m_State; }
};

class GLEObjectRepresention : public GLEDataObject {
protected:
	GLERectangle m_Rect;
	GLERC<GLEStringHash> m_SubObjs;
	GLERC<GLEDynamicSub> m_DynSub;
public:
	GLEObjectRepresention();
	virtual ~GLEObjectRepresention();
	inline GLERectangle* getRectangle() { return &m_Rect; }
	GLEObjectRepresention* getChildObject(GLEString* elem);
	bool setChildObject(GLEString* elem, GLEObjectRepresention* obj);
	void enableChildObjects();
	void translateChildrenRecursive(GLEPoint* trans);
	void copyChildrenRecursive(GLEObjectRepresention* newobj, gmodel* oldstate);
	void printNames();
	virtual int getType() const;
	inline bool isChildObjectsEnabled() { return !m_SubObjs.isNull(); }
	inline GLEStringHash* getChilds() { return m_SubObjs.get(); }
	inline bool hasSub() { return !m_DynSub.isNull(); }
	inline GLEDynamicSub* getSub() { return m_DynSub.get(); }
	inline void setSub(GLEDynamicSub* sub) { m_DynSub = sub; }
};

class GLEClassDefinition : public GLEDataObject {
protected:
	GLERC<GLEString> m_Name;
	GLERC<GLEArrayImpl> m_FieldNames;
public:
	GLEClassDefinition(const char* name);
	void addField(const char* fieldName);
	virtual int getType() const;
	virtual bool equals(GLEDataObject* obj) const;
	virtual void print(ostream& out) const;
};

class GLEClassInstance : public GLEDataObject {
protected:
	GLERC<GLEClassDefinition> m_Definition;
	GLEArrayImpl m_Data;
public:
	GLEClassInstance(GLEClassDefinition* definition);
	virtual int getType() const;
	virtual bool equals(GLEDataObject* obj) const;
	virtual void print(ostream& out) const;
	inline GLEClassDefinition* getDefinition() { return m_Definition.get(); }
	inline GLEArrayImpl* getArray() { return &m_Data; }
};

GLEClassInstance* getGLEClassInstance(GLEMemoryCell* object, GLEClassDefinition* def);

#endif
