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

#ifndef CHARBITMAP
#define CHARBITMAP

#define CharBitMapSize 8
#define CharBitMapIntIndexSize 5
#define CharBitMapIntIndexMask 31

class CharBitMap {
 private:
  typedef unsigned char uchar;
  int map[CharBitMapSize];
  inline int thebit(char c) const;
  inline int index(char c) const;
 public:
  inline void reset(char c);
  inline void set(char c);
  inline int get(char c) const;
  inline void set(const char *cset);
  inline void reset(const char *cset);
  inline void clear();
  inline void setall();
  inline CharBitMap();
  inline CharBitMap(const char *cset);
  };


inline int CharBitMap::index(char c) const {
  return ((uchar)c)>>CharBitMapIntIndexSize;
}
inline int CharBitMap::thebit(char c) const {
  return 1<<(c&CharBitMapIntIndexMask);
}
inline void CharBitMap::set(char c) {
  map[index(c)]|=thebit(c);
}
inline void CharBitMap::reset(char c) {
  map[index(c)]&=~thebit(c);
}
inline int CharBitMap::get(char c) const {
  return (map[index(c)]&thebit(c))!=0;
}
inline void CharBitMap::set(const char *cset) {
  while(*cset!=0)
    set(*(cset++));
}
inline void CharBitMap::reset(const char *cset) {
  while(*cset!=0)
    reset(*(cset++));
}
inline void CharBitMap::clear() {
  for(int i=0;i<CharBitMapSize;i++) map[i]=0;
}
inline void CharBitMap::setall() {
  for(int i=0;i<CharBitMapSize;i++) map[i]=~0;
}
inline CharBitMap::CharBitMap() {
  clear();
}
inline CharBitMap::CharBitMap(const char *cset) {
  clear();set(cset);
}
#endif
