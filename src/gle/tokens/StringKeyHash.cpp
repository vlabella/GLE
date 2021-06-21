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

#include "StringKeyHash.h"

//#ifdef GLE_USE_HASHTABLE

int StringIntHash::try_get(const name_hash_key& key) const {
	StringBasicHash<int>::const_iterator i = find(key);
	if (i != end()) {
		return i->second;
	} else {
		return -1;
	}
}

void StringIntHash::add_item(const name_hash_key& key, int elem) {
	insert(StringKeyPair<int>(key, elem));
}

void* StringVoidPtrHash::try_get(const name_hash_key& key) const {
	StringBasicHash<void*>::const_iterator i = find(key);
	if (i != end()) {
		return i->second;
	} else {
		return NULL;
	}
}

void StringVoidPtrHash::add_item(const name_hash_key& key, void* elem) {
	insert(StringKeyPair<void*>(key, elem));
}

void StringVoidPtrHash::deleteRecursive(int depth) {
	if (depth <= 0) return;
	for (StringVoidPtrHash::iterator iter = begin(); iter != end(); iter++) {
		void* elem = iter->second;
		if (elem != NULL) {
			StringVoidPtrHash* hash = (StringVoidPtrHash*)elem;
			hash->deleteRecursive(depth - 1);
			delete hash;
		}
	}
}

//#endif

int IntIntHash::try_get(int key) const {
	IntBasicHash<int>::const_iterator i = find(key);
	if (i != end()) {
		return i->second;
	} else {
		return -1;
	}
}

void IntIntHash::add_item(int key, int elem) {
	insert(IntKeyPair<int>(key, elem));
}

int IntStringHash::try_get(int key, string* res) const {
	IntBasicHash<string>::const_iterator i = find(key);
	if (i != end()) {
		*res = i->second;
		return 1;
	} else {
		return 0;
	}
}

void IntStringHash::add_item(int key, const string& elem) {
	insert(IntKeyPair<string>(key, elem));
}
