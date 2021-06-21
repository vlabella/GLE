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

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include "BinIO.h"

using namespace std;

void write_3byte(ostream* f, int val) {
	unsigned char ch[3];
	for (int i = 0; i < 3; i++) {
		int res = (val % 256);
		ch[i] = (unsigned char)res;
		val /= 256;
	}
	f->write((char*)ch, 3);
}

int read_3byte(istream* f) {
	int val = 0;
	unsigned char ch[3];
	if (!f->good()) return -1;
	f->read((char*)ch, 3);
	for (int i = 2; i >= 0; i--) {
    		val *= 256;
		val += ch[i];
	}
	return val;
}

BinIOError::BinIOError(const string& txt, BinIO& io) {
  m_txt = txt;
  m_pos = io.getPosition();
}

ostream& BinIOError::write(ostream& os) const {
  char buf[20];
  sprintf(buf, "0x%x", m_pos);
  os << "Binary file corrupt: " << m_txt << " at pos: " << buf << endl;
  return os;
}

BinIO::BinIO(filebuf& fb, int rdwr) : m_fb(fb) {
  m_os = NULL;
  m_is = NULL;
  m_rdwr = rdwr;
  if (rdwr == BINIO_READ) m_is = new istream(&fb);
  else m_os = new ostream(&fb);
}

BinIO::~BinIO() {
  if (m_is != NULL) delete m_is;
  if (m_os != NULL) delete m_os;
}

long BinIO::getPosition() {
  return m_is->tellg();
}

void BinIO::ensure(char val, const char* errmsg) {
  char valr;
  m_is->read(&valr, sizeof(valr));
  if (valr != val) throw BinIOError(errmsg, *this);
}

int BinIO::check_version(int version, int exOnError) {
  int fver = read_int();
  if (fver != version) {
    if (exOnError == 1) {
      char buf[32];
      sprintf(buf, "%d <> %d", fver, version);
      throw BinIOError(string("Incorrect binary file version ") + buf, *this);
    }
    return 0;
  } else {
    return 1;
  }
}

int BinIO::check(char yes, char no, const char* errmsg) {
  char valr;
  m_is->read(&valr, sizeof(valr));
  if (valr == yes) return 1;
  if (valr == no) return 0;
  throw BinIOError(errmsg, *this);
}

int BinIO::read_int() {
  return read_3byte(m_is);
}

char BinIO::read_char() {
  char valr;
  m_is->read(&valr, sizeof(valr));
  return valr;
}

void BinIO::read_str(string& str) {
  int len = read_3byte(m_is);
  char *buf = new char[len+1];
  m_is->read(buf,len);
  char *buff_end = buf + len;
  *buff_end = 0;
  str = *buf;
  delete []buf;
}

void BinIO::read(void* bytes, int size) {
  m_is->read((char*)bytes, size);
}

void BinIO::write(size_t val) {
  write_3byte(m_os, val);
}

#ifdef __APPLE__
void BinIO::write(unsigned int val) {
  write_3byte(m_os, val);
}
#endif


void BinIO::write(int val) {
  write_3byte(m_os, val);
}

void BinIO::write(char val) {
  m_os->write((char*)&val, sizeof(val));
}

void BinIO::write_char(char val) {
  m_os->write((char*)&val, sizeof(val));
}

void BinIO::write(const string& val) {
  int len = val.length();
  write_3byte(m_os, len);
  m_os->write(val.c_str(), len);
}

void BinIO::write(const void* bytes, int size) {
  m_os->write((char*)bytes, size);
}

void BinIO::write(BinIOSerializable* ser) {
  if (ser == NULL) {
  	write('N');
  } else {
  	write('P');
	ser->bin_write(*this);
  }
}

int BinIO::hasSerializable() {
  return check('P', 'N', "Serializable expected");
}

int BinIO::addSerializable(BinIOSerializable* ser) {
  int res = m_ser.size();
  m_ser.push_back(ser);
  return res;
}

void BinIO::cleanSIndices() {
  for (unsigned int i = 0; i < m_ser.size(); i++) {
     m_ser[i]->setSIndex(-1);
  }
}

void BinIO::close() {
  if (m_os != NULL) m_os->flush();
  m_fb.close();
  cleanSIndices();
}

BinIOSerializable::BinIOSerializable() {
  m_sindex = -1;
}

BinIOSerializable::~BinIOSerializable() {
}

void BinIOSerializable::bin_write(BinIO& io) {
  if (m_sindex == -1) {
     m_sindex = io.addSerializable(this);
     io.write('W');
     bin_write_impl(io);
  } else {
     io.write('S');
     io.write(m_sindex);
  }
}

BinIOSerializable* bin_read_serializable(BinIO& io) {
  return NULL;
}

BinIOSerializable* bin_read_serializable(BinIO& io, const string& key) {
  if (io.check('W', 'S', "Serializable expected")) {
    /*
      specific read here?
    */
    return NULL;
  } else {
    int idx = io.read_int();
    return io.getSerializable(idx);
  }
}

BinIOSerializable* try_bin_read_serializable(BinIO& io) {
  if (io.check('W', 'S', "Serializable expected")) {
    return NULL;
  } else {
    int idx = io.read_int();
    return io.getSerializable(idx);
  }
}

BinIOSerializable* ptr_bin_read_serializable(BinIO& io) {
  if (io.hasSerializable()) {
    if (io.check('W', 'S', "Serializable expected")) {
      throw BinIOError("Serializable is no pointer", io);
    } else {
      int idx = io.read_int();
      return io.getSerializable(idx);
    }
  } else {
    return NULL;
  }
}
