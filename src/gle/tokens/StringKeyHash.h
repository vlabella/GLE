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
#include <algorithm>
#include <string>
#include <string.h>
#include <map>
#include <utility>
#include <iostream>
#include <vector>
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

/**************************************************************************************************
 * Hashtable string -> ElemType                                                                   *
 **************************************************************************************************/

template <class ElemType> class StringBasicHash :
	public std::map<name_hash_key, ElemType, lt_name_hash_key> {
};

/**************************************************************************************************
 * Hashtable int -> ElemType                                                                      *
 **************************************************************************************************/

template <class ElemType> class IntBasicHash :
	public std::map<int, ElemType, lt_int_key> {
};

/**************************************************************************************************
 * Hashtable string -> int
 **************************************************************************************************/

class StringIntHash : public StringBasicHash<int> {
public:
 	int try_get(const name_hash_key& key) const;
 	void add_item(const name_hash_key& key, int elem);
};

/**************************************************************************************************
 * Hashtable string -> void*                                                                      *
 **************************************************************************************************/

class StringVoidPtrHash : public StringBasicHash<void*> {
public:
 	void* try_get(const name_hash_key& key) const;
 	void add_item(const name_hash_key& key, void* elem);
 	void deleteRecursive(int depth);
};

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
	public std::map<name_hash_key, ElemType, lt_name_hash_key> {
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
	public std::map<int, ElemType, lt_int_key> {
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
