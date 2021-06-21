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

#ifndef __GLE_BASE__
#define __GLE_BASE__

// #include <iostream>

// #include <stdlib.h>
// #include <string.h>

using namespace std;

class GLERefCountObject {
private:
	int m_OwnerCount;
public:
	GLERefCountObject();
	virtual ~GLERefCountObject();
	inline int getOwnerCount() const { return m_OwnerCount; }
	inline void use() { m_OwnerCount++; }
	inline int release() {	return (--m_OwnerCount) == 0; }
	inline int unused() const {  return m_OwnerCount == 0; }
};

template <class T> class GLERC {
private:
	T* m_Object;
public:
	GLERC() {
		m_Object = NULL;
	}
	GLERC(T* obj) {
		m_Object = obj;
		if (m_Object != NULL) m_Object->use();
	}
	GLERC(const GLERC<T>& obj) {
		m_Object = obj.m_Object;
		if (m_Object != NULL) m_Object->use();
	}
	~GLERC() {
		if (m_Object != NULL && m_Object->release()) {
			delete m_Object;
		}
	}
	inline GLERC<T>& operator=(GLERC<T> src) {
		set(src.m_Object);
		return *this;
	}
	inline GLERC<T>& operator=(T *src) {
		set(src);
		return *this;
	}
	inline bool isNull() const { return m_Object == NULL; }
	inline T& operator*() const { return *m_Object; }
	inline T* operator->() const { return m_Object; }
	inline T* get() const { return m_Object; }
	void set(T* obj) {
		if (obj != NULL) obj->use();
		if (m_Object != NULL && m_Object->release()) {
			delete m_Object;
		}
		m_Object = obj;
	}
	void clear() {
		if (m_Object != NULL && m_Object->release()) {
			delete m_Object;
		}
		m_Object = NULL;
	}
};

inline void GLE_DEL_RC(GLERefCountObject* rc) {
	if (rc == NULL) return;
	rc->release();
	if (rc->unused()) delete rc;
}

inline void GLE_DEL_RC_INTERN(GLERefCountObject* rc) {
	if (rc->release()) {
		delete rc;
	}
}

inline GLERefCountObject* GLE_INIT_RC(GLERefCountObject* value) {
	value->use();
	return value;
}

inline GLERefCountObject* GLE_SET_RC(GLERefCountObject* rc, GLERefCountObject* value) {
	value->use();
	GLE_DEL_RC_INTERN(rc);
	return value;
}

template <class T> class GLERCVector : public vector< GLERC<T> > {
public:
	inline void add(T* elem) { this->push_back(GLERC<T>(elem)); }
	inline T* get(int i) { return (*this)[i].get(); }
};

template <class T> class GLEVectorAutoDelete : public vector<T*> {
public:
	GLEVectorAutoDelete() : vector<T*>() {
	}
	~GLEVectorAutoDelete() {
		this->deleteAll();
	}
	void clear() {
		this->deleteAll();
		vector<T*>::clear();
	}
	void deleteAll() {
		for (typename vector<T*>::size_type i = 0; i < vector<T*>::size(); i++) {
			T* elem = this->at(i);
			if (elem != NULL) delete elem;
		}
	}
};

template <class T> class GLEAutoDelete {
protected:
	T* m_Object;
public:
	GLEAutoDelete() {
		m_Object = NULL;
	}
	GLEAutoDelete(T* data) {
		m_Object = data;
	}
	~GLEAutoDelete() {
		if (m_Object != NULL) delete m_Object;
	}
	inline GLEAutoDelete<T>& operator=(T* data) {
		if (m_Object != NULL) delete m_Object;
		m_Object = data;
		return *this;
	}
	inline T* get() { return m_Object; }
	inline T* operator->() const { return m_Object; }
};

#endif
