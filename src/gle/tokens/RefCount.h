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

#ifndef __REFCOUNT_H__
#define __REFCOUNT_H__

class RefCountObject {
private:
	int owner_count;
public:
	RefCountObject();
	~RefCountObject();
	inline int getOwnerCount() const { return owner_count; }
	inline void use() { owner_count++; }
	inline int release() {	return (--owner_count) == 0; }
	inline int unused() const {  return owner_count == 0; }
};

// a safe RefCount object is a pointer to a RefCount object
// that implements safe behaviour for use by classes that
// only see the public section
template <class T> class RefCountPtr {
protected:
	T* unsafe_ptr;
	void setUnsafePtr(T* ptr) {
		unsafe_ptr = ptr;
		if (unsafe_ptr != NULL) unsafe_ptr->use();
	}
	void initPtr() {
		unsafe_ptr=NULL;
	}
public:
	inline bool isNull() const {
		return unsafe_ptr == NULL;
	}
	inline bool notNull() const {
		return unsafe_ptr != NULL;
	}
	void clearPtr() {
		if (unsafe_ptr == NULL) return;
		unsafe_ptr->release();
    		if (unsafe_ptr->unused()) { delete unsafe_ptr; };
		unsafe_ptr = NULL;
	}
	inline RefCountPtr<T>& operator =(RefCountPtr<T> src) {
		setPtr(src.unsafe_ptr);
		return *this;
	}
	inline RefCountPtr<T>& operator =(T *src) {
     		setPtr(src);
		return *this;
	}
	void setPtr(T *ptr) {
		// first use() the new pointer, then delete unsafe_ptr if necessary
		// order is important if ptr and unsafe_ptr point to the same object
		if (ptr != NULL) ptr->use();
		if (unsafe_ptr != NULL && unsafe_ptr->release()) {
			delete unsafe_ptr;
		}
		unsafe_ptr = ptr;
	}
	inline void SetPtr(T *ptr) { setPtr(ptr); }
	inline T& operator*() const { return *unsafe_ptr; }
	inline T* operator->() const { return unsafe_ptr; }
	inline T* get() const {return unsafe_ptr;}
	inline T* GetPtr() const {return unsafe_ptr;}
	inline RefCountPtr() {
		initPtr();
	};
	inline RefCountPtr(const RefCountPtr<T>& src) {
		setUnsafePtr(src.unsafe_ptr);
	};
	inline RefCountPtr(T* src) {
		setUnsafePtr(src);
	};
	inline ~RefCountPtr() {
		clearPtr();
	};
};

template <class T> class RefCountVector : public vector< RefCountPtr<T> > {
public:
	inline void add(T* elem) { push_back(RefCountPtr<T>(elem)); }
	inline T* get(int i) { return (*this)[i].get(); }
	void createNew(int n) {
		for (int i = 0; i < n; i++) push_back(RefCountPtr<T>(new T()));
	}
};

template <class T> class MutableRefCountPtr : public RefCountPtr<T> {
public:
	inline RefCountPtr<T>& operator =(RefCountPtr<T> src) {
		this->setPtr(src.get());
		return *this;
	};
};

#endif
