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

#ifndef STRINGKEYHASH_H
#define STRINGKEYHASH_H

#ifndef _MSC_VER
	// Use hash table except when compiling with MSVC
	//#define GLE_USE_HASHTABLE
#else
	#pragma warning( disable : 4996 )
#endif

#define EXT_HASH
#ifdef _MSC_VER
	#undef EXT_HASH
#endif
#ifdef GCC2
	#undef EXT_HASH
#endif
#ifdef EXT_HASH
//	#include <ext/hash_map>
#else
	#ifdef _MSC_VER
	#include <hash_map>
	using namespace stdext;
//	typedef hash _Hash;
	#else
	#include <hash_map.h>
	#endif
#endif
#include <algorithm>
#include <string>
#include <string.h>
#include <map>
#include <utility>
#include <iostream>
#include <vector>

// using namespace std;  should not reside in header file

#ifndef GCC2
#ifndef _MSC_VER
	// gcc on macOS is complaining about this; not sure if it's needed
	// using namespace __gnu_cxx;  // using gnu extensions such as "hash"
#endif
#endif

#include "RefCount.h"
#include "BinIO.h"

/**************************************************************************************************
 * Hash functions on strings                                                                      *
 **************************************************************************************************/

#define SKEYHASH_DELETE	1
#define SKEYHASH_KEEP   0

typedef std::string name_hash_key;

template <class ElemType> class StringKeyPair : public std::pair<const name_hash_key, ElemType> {
public:
	StringKeyPair(const name_hash_key key, ElemType value) : std::pair<const name_hash_key, ElemType>(key, value) {
	}
};

template <class ElemType> class IntKeyPair : public std::pair<int, ElemType> {
public:
	IntKeyPair(int key, ElemType value) : std::pair<int, ElemType>(key, value) {
	}
};

#ifdef GLE_USE_HASHTABLE

struct eq_name_hash_key {
	inline bool operator() (const name_hash_key& s1, const name_hash_key& s2) const {
	    return s1 == s2;
	}
};

struct hash_name_hash_key {
	inline size_t operator() (const name_hash_key& s) const {
		#ifdef _MSC_VER
		return stdext::hash_value<const char *>(s.c_str());
		#else
		return hash<const char *>()(s.c_str());
		#endif
	}
};

template <class ElemType> class StringKeyIterator : public std::hash_map<name_hash_key, ElemType>::iterator {
};

struct eq_int_key {
	inline bool operator() (int s1, int s2) const {
	    return s1 == s2;
	}
};

struct hash_int_key {
	inline size_t operator() (int s) const {
		#ifdef _MSC_VER
		return stdext::hash_value<int>(s);
		#else
		return std::hash<int>()(s);
		#endif
	}
};

template <class ElemType> class IntKeyIterator : public std::hash_map<int, ElemType>::iterator {
};

#else

struct lt_name_hash_key {
	bool operator()(const name_hash_key& s1, const name_hash_key& s2) const {
		return s1 < s2;
	}
};

struct lt_int_key {
	bool operator()(int s1, int s2) const {
		return s1 < s2;
	}
};

#endif

/**************************************************************************************************
 * Hashtable string -> ElemType                                                                   *
 **************************************************************************************************/

template <class ElemType> class StringBasicHash :
#ifdef GLE_USE_HASHTABLE
	public std::hash_map<name_hash_key, ElemType, hash_name_hash_key, eq_name_hash_key> {
#else
	public std::map<name_hash_key, ElemType, lt_name_hash_key> {
#endif
};

/**************************************************************************************************
 * Hashtable int -> ElemType                                                                      *
 **************************************************************************************************/

template <class ElemType> class IntBasicHash :
#ifdef GLE_USE_HASHTABLE
	public std::hash_map<int, ElemType, hash_int_key, eq_int_key> {
#else
	public std::map<int, ElemType, lt_int_key> {
#endif
};

/**************************************************************************************************
 * Hashtable string -> int (currently not used, might be used for variables?)                     *
 **************************************************************************************************/

//#ifdef GLE_USE_HASHTABLE

class StringIntHash : public StringBasicHash<int> {
public:
	int try_get(const name_hash_key& key) const;
	void add_item(const name_hash_key& key, int elem);
};

//#endif

/**************************************************************************************************
 * Hashtable string -> void*                                                                      *
 **************************************************************************************************/

//#ifdef GLE_USE_HASHTABLE

class StringVoidPtrHash : public StringBasicHash<void*> {
public:
	void* try_get(const name_hash_key& key) const;
	void add_item(const name_hash_key& key, void* elem);
	void deleteRecursive(int depth);
};

//#endif

/**************************************************************************************************
 * Hashtable int -> int                                                                           *
 **************************************************************************************************/

class IntIntHash : public IntBasicHash<int> {
public:
	int try_get(int key) const;
	void add_item(int key, int elem);
};

/**************************************************************************************************
 * Hashtable int -> string                                                                           *
 **************************************************************************************************/

class IntStringHash : public IntBasicHash<std::string> {
public:
	int try_get(int key, std::string* res) const;
	void add_item(int key, const std::string& elem);
};

/**************************************************************************************************
 * Hashtable string -> ElemType (used by the GLE parser)                                          *
 **************************************************************************************************/

template <class ElemType> class StringKeyHash :
#ifdef GLE_USE_HASHTABLE
    #ifdef _MSC_VER
	public std::hash_map<name_hash_key, ElemType , stdext::hash_compare<name_hash_key,eq_name_hash_key> > {
    #else
	public std::hash_map<name_hash_key, ElemType , hash_name_hash_key, eq_name_hash_key> {
    #endif
#else
	public std::map<name_hash_key, ElemType, lt_name_hash_key> {
#endif
public:
	ElemType try_add(const name_hash_key& key) {
		typename StringKeyHash<ElemType>::iterator i = this->find(key);
		if (i != this->end()) {
			return i->second;
		} else {
			ElemType nelem(key);
			this->insert(StringKeyPair<ElemType>(key, nelem));
			return nelem;
		}
	}

	ElemType try_get(const name_hash_key& key) {
		typename StringKeyHash<ElemType>::const_iterator i = this->find(key);
		if (i != this->end()) {
			return i->second;
		} else {
			return NULL;
		}
	}

	void add_item(const name_hash_key& key, const ElemType& elem) {
		insert(StringKeyPair<ElemType>(key, elem));
	}

	std::ostream& write(std::ostream &os, int tab) const {
		for (typename StringKeyHash<ElemType>::const_iterator i = this->begin(); i != this->end(); i++ ) {
			std::cerr << i->first << std::endl;
			i->second.write(os, tab);
		}
		return os;
	}

	std::ostream& writeKeys(std::ostream &os) const {
		for (typename StringKeyHash<ElemType>::const_iterator i = this->begin(); i != this->end(); i++ ) {
			std::cerr << i->first << std::endl;
		}
		return os;
	}
};

/**************************************************************************************************
 * Hashtable int -> ElemType                                                                      *
 **************************************************************************************************/

template <class ElemType> class IntKeyHash :
#ifdef GLE_USE_HASHTABLE
	public std::hash_map<int, ElemType, hash_int_key, eq_int_key> {
#else
	public std::map<int, ElemType, lt_int_key> {
#endif
public:
	ElemType try_get(int key) {
		typename IntKeyHash<ElemType>::const_iterator i = this->find(key);
		if (i != this->end()) {
			return i->second;
		} else {
			return NULL;
		}
	}

	void add_item(int key, ElemType elem) {
		this->insert(IntKeyPair<ElemType>(key, elem));
	}
};

#endif // of #ifndef STRINGKEYHASH_H
