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

#ifndef INCLUDE_CUTILS_H
#define INCLUDE_CUTILS_H

#include <stdarg.h>

bool gle_onlyspace(const string& s);
bool gle_isnumber(const char *s);

bool gle_isalphanum(char ch);

int gle_isnan(double v);
int gle_isinf(double v);

#define CUTILS_REL_PREC 1e-6
#define CUTILS_REL_PREC_FINE 1e-13

bool equals_rel(double actual, double value);
bool equals_rel_fine(double actual, double value);

const char* str_find_char(const char* s, int ch);

const char* str_skip_brackets(const char* s, int ch1, int ch2);

char* gle_strupr(char* s);
char* gle_strlwr(char *s);
void gle_strlwr(string& s);

int str_skip_brackets(const string& s, int pos, int ch1, int ch2);

// for compatibility between different compilers
bool str_i_equals(const char* s1, const char* s2);
bool str_ni_equals(const char* s1, const char* s2, int max);
int str_i_cmp(const char* s1, const char* s2);

struct str_i_less
{
  bool operator()(const std::string& s1, const std::string& s2) const;
};

char* str_i_str(const char* haystack, const char* needle);
int str_i_str(const string& haystack, const char* needle);
int str_i_str(const string& haystack, int from, const char* needle);

bool str_i_equals(const string& s1, const string& s2);

bool str_contains(const char* str, char ch);

bool str_contains(const string& str, const char* elems);

int str_remove_all(char* str, char ch);

void str_prefix(int count, char ch, string* str);

void str_to_uppercase(const string& input, string& output);

void str_to_uppercase(string& output);

std::vector<std::string> strs_to_uppercase(const std::vector<std::string>& input);

std::string str_join(const std::vector<std::string>& input, const char* joinStr = ", ");

void str_uppercase_initial_capital(string& str);

bool str_only_space(const string& str);

void str_trim_both(string& str);

void str_trim_right(string& str);

void str_trim_left(string& str);

void str_trim_left(string& str, string& prefix);

void str_trim_left_bom(string& str);

int str_starts_with_trim(const string& str, const char* test);

bool str_starts_with(const string& str, const char* find);

bool str_i_starts_with(const string& str, const char* find);

bool str_i_ends_with(const string& str, const char* find);

void str_remove_quote(string& str);

void str_try_add_quote(string& str);

void str_parse_get_next(const string& strg, const char* find, string& res);

void gle_int_to_string_bin(int value, string* binary);

void gle_int_to_string(int value, string* str);

int gle_double_digits(double value, int prec);

int gle_int_digits(int value);

char *un_quote(char *cts);

int lastchar(char *s, char c);

int gle_pow_ii(int x, int n);

int f_pow_ii(int* ap, int* bp);

double f_r_sign(double* a, double* b);

int f_i_sign(int *a, int* b);

void str_delete_start(string& str, char ch);

void str_replace_start(string& str, const char* find, const char* repl);

void str_replace_all(char* str, const char* find, const char* repl);
void str_replace_all(string& str, const char* find, const char* repl);

void str_get_system_error(ostream& error);

int gle_round_int(double val);

int gle_make_zero_based(int val);

#if defined(__UNIX__)
	string str_format(const char* arg_list, ...);
#else
	string str_format(va_list arg_list, ...);
#endif

void str_format(string* str, const char *format, va_list ap);

int gle_pass_hex(const char* str, int from, int digits, int* err);

void bool_vector_set_expand(vector<bool>* v, unsigned int i, bool value);

bool bool_vector_is(vector<bool>* v, unsigned int i);

void split_into_lines(const std::vector<unsigned char>* input, std::vector<std::string>* output);

void CUtilsAssertImpl(const char* expr, const char* file, int line, const char* function);

#define CUtilsAssertMessage(msg) \
   CUtilsAssertImpl(msg, __FILE__, __LINE__, __FUNCTION__)

#define CUtilsAssert(exp) \
   if (!(exp)) CUtilsAssertImpl(#exp, __FILE__, __LINE__, __FUNCTION__)

#endif

double gle_limit_range(double value, double minValue, double maxValue);
