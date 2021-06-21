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

#ifndef BINIO_H
#define BINIO_H

#include <string>

// This class contains stuff for serializing graphs

#define BINIO_READ	0
#define BINIO_WRITE	1

using namespace std;

class BinIO;
class BinIOSerializable;

class BinIOError {
private:
	string m_txt;
	int m_pos;
public:
	BinIOError(const string& txt, BinIO& io);
	ostream& write(ostream& os) const;
};

inline ostream& operator<<(ostream& os, const BinIOError& err) {
	return err.write(os);
}

class BinIO {
protected:
	int m_rdwr;
	ostream* m_os;
	istream* m_is;
	filebuf& m_fb;
	vector<BinIOSerializable*> m_ser;
public:
	BinIO(filebuf& fb, int rdwr);
	~BinIO();
	inline ostream& os() { return *m_os; }
	long getPosition();
	void write(size_t val);
#ifdef __APPLE__
	void write(unsigned int val);
#endif
	void write(int val);
	void write(char val);
	void write_char(char val);
	void write(const string& val);
	void write(const void* bytes, int size);
	void write(BinIOSerializable* ser);
	void ensure(char val, const char* errmsg);
	int check_version(int version, int exOnError);
	int check(char yes, char no, const char* errmsg);
	int read_int();
	char read_char();
	void read_str(string& str);
	void read(void* bytes, int size);
	int hasSerializable();
	int addSerializable(BinIOSerializable* ser);
	inline BinIOSerializable* getSerializable(int idx) { return m_ser[idx]; }
	void cleanSIndices();
	void close();
};


template <class ElemType>
class BinIOVector : public vector<ElemType> {
public:
	void bin_write(BinIO& io) const {
		io.write('V');
		io.write(this->size());
		for (typename BinIOVector<ElemType>::const_iterator iter = this->begin(); iter != this->end(); iter++) {
			iter->bin_write(io);
		}
	}

	void bin_read(BinIO& io) {
		io.ensure('V', "Expected vector");
		int size = io.read_int();
		for (int i = 0; i < size; i++) {
			push_back(bin_read_elem(io));
		}
	}

	ElemType bin_read_elem(BinIO& io);
};

class BinIOSerializable {
protected:
	int m_sindex;
public:
	BinIOSerializable();
	virtual ~BinIOSerializable();
	inline int getSIndex() { return m_sindex; }
	inline void setSIndex(int idx) { m_sindex = idx; }
	void bin_write(BinIO& io);
	virtual void bin_write_impl(BinIO& io) = 0;
};

BinIOSerializable* bin_read_serializable(BinIO& io, const string& key);
BinIOSerializable* bin_read_serializable(BinIO& io);
BinIOSerializable* try_bin_read_serializable(BinIO& io);
BinIOSerializable* ptr_bin_read_serializable(BinIO& io);

#endif

