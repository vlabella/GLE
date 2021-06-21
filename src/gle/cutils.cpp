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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <algorithm>

#ifdef HAVE_CONFIG_H
    #include "../config.h"
#else
    #include "../config_noauto.h"
#endif

// For visual C++ -- should use numeric limits?
#if defined(__WIN32__) && !defined(__NOCYGWIN__)
#include<float.h>
	#define isnan _isnan
	#define isinf _isnan
#endif

#ifndef isnan
#define isnan(x) \
    (sizeof (x) == sizeof (long double) ? isnan_ld (x) \
     : sizeof (x) == sizeof (double) ? isnan_d (x) \
     : isnan_f (x))
static inline int isnan_f  (float       x) { return x != x; }
static inline int isnan_d  (double      x) { return x != x; }
static inline int isnan_ld (long double x) { return x != x; }
#endif

#ifndef isinf
#define isinf(x) \
    (sizeof (x) == sizeof (long double) ? isinf_ld (x) \
     : sizeof (x) == sizeof (double) ? isinf_d (x) \
     : isinf_f (x))
static inline int isinf_f  (float       x) { return isnan (x - x); }
static inline int isinf_d  (double      x) { return isnan (x - x); }
static inline int isinf_ld (long double x) { return isnan (x - x); }
#endif

int gle_isnan(double v) {
	// Do not include <iostream> or <cmath> before gle_isnan
	// otherwise gle_isnan wil break on Mac OS/X and OS/2
	return isnan(v);
}

int gle_isinf(double v) {
	// Do not include <iostream> or <cmath> before gle_isinf
	// otherwise gle_isinf wil break on Mac OS/X and OS/2
	return isinf(v);
}

#include <string>
#include <vector>
#include <sstream>
#include <iostream>

using namespace std;

#include "cutils.h"
#include "tokens/stokenizer.h"

bool equals_rel(double v1, double v2) {
	if (v1 != 0.0) {
		return fabs(v1 - v2)/v1 < CUTILS_REL_PREC;
	} else {
		return fabs(v1 - v2) < CUTILS_REL_PREC;
	}
}

bool equals_rel_fine(double v1, double v2) {
	if (v1 != 0.0) {
		return fabs(v1 - v2)/v1 < CUTILS_REL_PREC_FINE;
	} else {
		return fabs(v1 - v2) < CUTILS_REL_PREC_FINE;
	}
}

char* gle_strupr(char *s) {
	char *tmp = s;
	while (*s != 0) {
	    *s = toupper(*s);
	    s++;
	}
	return tmp;
}

char* gle_strlwr(char *s) {
	char *tmp = s;
	while (*s != 0) {
	    *s = tolower(*s);
	    s++;
	}
	return tmp;
}

void gle_strlwr(string& s) {
	string::size_type len = s.length();
	for (string::size_type i = 0; i < len; i++) {
		int ch = s[i];
		if (ch >= 'A' && ch <= 'Z') {
			ch += 'a' - 'A';
			s[i] = ch;
		}
	}
}

bool gle_onlyspace(const string& s) {
	string::size_type len = s.length();
	for (string::size_type i = 0; i < len; i++) {
		if (s[i] != ' ' && s[i] != '\t') return false;
	}
	return true;
}

bool gle_isnumber(const char *s) {
	while (*s!='\0') {
		if (isdigit(*s) || *s=='.' || toupper(*s)=='E' ) s++;
		else  return false;
	}
	return true;
}

bool gle_isalphanum(char ch) {
	if (ch >= 'A' && ch <= 'Z') return true;
	if (ch >= 'a' && ch <= 'z') return true;
	if (ch >= '0' && ch <= '9') return true;
	return false;
}

const char* str_find_char(const char* s, int ch1) {
	while (*s != 0) {
		if (*s == ch1) return s;
		s++;
	}
	return s;
}

const char* str_skip_brackets(const char* s, int ch1, int ch2) {
	int depth = 0;
	while (*s != 0) {
		if (*s == ch1) {
			depth++;
		} else if (*s == ch2) {
			depth--;
			if (depth <= 0) break;
		}
		s++;
	}
	return s;
}

int str_skip_brackets(const string& s, int pos, int ch1, int ch2) {
	int depth = 0;
	int len = s.length();
	while (pos < len) {
		if (s[pos] == ch1) {
			depth++;
		} else if (s[pos] == ch2) {
			depth--;
			if (depth <= 0) break;
		}
		pos++;
	}
	return pos;
}

bool str_i_equals(const char* s1, const char* s2) {
	int i = 0;
	while (s1[i] != 0 && s2[i] != 0) {
		if (toupper(s1[i]) != toupper(s2[i])) return false;
		i++;
	}
	return s1[i] == 0 && s2[i] == 0;
}

bool str_ni_equals(const char* s1, const char* s2, int max) {
	int i = 0;
	while (i < max && s1[i] != 0 && s2[i] != 0) {
		if (toupper(s1[i]) != toupper(s2[i])) return false;
		i++;
	}
	if (i == max) return true;
	return s1[i] == 0 && s2[i] == 0;
}

int str_i_cmp(const char* s1, const char* s2) {
	while (true) {
		int c1 = tolower((unsigned char)*(s1++));
		int c2 = tolower((unsigned char)*(s2++));
		if (c1 == 0 || c1 != c2) return c1 - c2;
	}
}

bool str_i_less::operator()(const std::string& s1, const std::string& s2) const {
	return str_i_cmp(s1.c_str(), s2.c_str()) < 0;
}

int str_i_str(const string& haystack, int from, const char* needle) {
	int endIndex = haystack.length();
	int patternLength = strlen(needle);
	int endPattern = endIndex - patternLength + 1;
	if (endPattern < 0) {
		return -1;
	}
	if (patternLength <= 0) {
		return 0;
	}
	char patternChar0 = toupper(needle[0]);
	for (int ctrSrc = from; ctrSrc <= endPattern; ctrSrc++) {
		if (toupper(haystack[ctrSrc]) != patternChar0) {
			continue;
		}
		int ctrPat;
		for (ctrPat = 1; (ctrPat < patternLength) && (toupper(haystack[ctrSrc + ctrPat]) == toupper(needle[ctrPat])); ctrPat++) {
			; // just loop
		}
		if (ctrPat == patternLength) {
			return ctrSrc;
		}
	}
	return -1;
}

int str_i_str(const string& haystack, const char* needle) {
	return str_i_str(haystack, 0, needle);
}

char* str_i_str(const char* haystack, const char* needle) {
	int endIndex = strlen(haystack);
	int patternLength = strlen(needle);
	int endPattern = endIndex - patternLength + 1;
	if (endPattern < 0) {
		return NULL;
	}
	if (patternLength <= 0) {
		return (char*)haystack;
	}
	char patternChar0 = toupper(needle[0]);
	for (int ctrSrc = 0; ctrSrc <= endPattern; ctrSrc++) {
		if (toupper(haystack[ctrSrc]) != patternChar0) {
			continue;
		}
		int ctrPat;
		for (ctrPat = 1; (ctrPat < patternLength) && (toupper(haystack[ctrSrc + ctrPat]) == toupper(needle[ctrPat])); ctrPat++) {
			; // just loop
		}
		if (ctrPat == patternLength) {
			return (char*)haystack+ctrSrc;
		}
	}
	return NULL;
}

bool str_i_equals(const string& s1, const char* s2) {
	int l1 = s1.length();
	for (int i = 0; i < l1; i++) {
		if (toupper(s1[i]) != toupper(s2[i])) return false;
	}
	return true;
}

bool str_i_equals(const string& s1, const string& s2) {
	int l1 = s1.length();
	int l2 = s2.length();
	if (l1 != l2) return false;
	for (int i = 0; i < l1; i++) {
		if (toupper(s1[i]) != toupper(s2[i])) return false;
	}
	return true;
}

int str_starts_with_trim(const string& str, const char* test) {
	int len = str.length();
	int pos = 0;
	while (pos < len && (str[pos] == ' ' || str[pos] == '\t')) {
		pos++;
	}
	int idx = 0;
	while (pos < len && toupper(test[idx]) == toupper(str[pos])) {
		idx++;
		pos++;
	}
	if (test[idx] == 0) return pos;
	else return -1;
}

bool str_starts_with(const string& str, const char* find) {
	int idx = 0;
	int len = str.length();
	while (idx < len && find[idx] == str[idx]) {
		idx++;
	}
	return find[idx] == 0;
}

bool str_i_starts_with(const string& str, const char* find) {
	int idx = 0;
	int len = str.length();
	while (idx < len && toupper(find[idx]) == toupper(str[idx])) {
		idx++;
	}
	return find[idx] == 0;
}

bool str_i_ends_with(const string& str, const char* find) {
	int f_len = strlen(find);
	int s_len = str.length();
	if (f_len > s_len) {
		return false;
	} else {
		int pos = 0;
		for (int i = s_len-f_len; i < s_len; i++) {
			if (toupper(str[i]) != toupper(find[pos++])) {
				return false;
			}
		}
		return true;
	}
}

void str_delete_start(string& str, char ch) {
	if (str.length() > 0 && str[0] == ch) {
		str.erase(0, 1);
	}
}

void str_replace_start(string& str, const char* find, const char* repl) {
	if (str_starts_with(str, find)) {
		str.erase(0, strlen(find));
		str.insert(0, repl);
	}
}

int str_remove_all(char* str, char ch) {
	int pos = 0;
	int from = 0;
	while (str[from] != 0) {
		while (str[from] == ch) {
			from++;
		}
		str[pos++] = str[from++];
	}
	str[pos] = 0;
	return pos;
}

bool str_contains(const char* str, char ch) {
	int i = 0;
	while (str[i] != 0 && str[i] != ch) {
		i++;
	}
	return str[i] == ch;
}

bool str_contains(const string& str, const char* elems) {
	int len = str.length();
	for (int i = 0; i < len; i++) {
		if (str_contains(elems, str[i])) return true;
	}
	return false;
}

void str_prefix(int count, char ch, string* str) {
	if (count > 0) {
		stringstream prefix;
		for (int i = 0; i < count; i++) {
			prefix << ch;
		}
		prefix << (*str);
		*str = prefix.str();
	}
}

void str_to_uppercase(const string& input, string& output) {
	output = input;
	int len = input.length();
	for (int i = 0; i < len; i++) {
		output[i] = toupper(output[i]);
	}
}

void str_to_uppercase(string& output) {
	int len = output.length();
	for (int i = 0; i < len; i++) {
		output[i] = toupper(output[i]);
	}
}

std::vector<std::string> strs_to_uppercase(const std::vector<std::string>& input) {
	std::vector<std::string> result;
	result.reserve(input.size());
	for (std::vector<std::string>::size_type i(0); i != input.size(); ++i) {
		std::string value(input[i]);
		str_to_uppercase(value);
		result.push_back(value);
	}
	return result;
}

std::string str_join(const std::vector<std::string>& input, const char* joinStr) {
	std::ostringstream strm;
	for (std::vector<std::string>::size_type i(0); i != input.size(); ++i) {
		if (i != 0) {
			strm << joinStr;
		}
		strm << input[i];
	}
	return strm.str();
}


void str_uppercase_initial_capital(string& str) {
	if (str.length() >= 1) str[0] = toupper(str[0]);
}

void str_remove_quote(string& str) {
	int len = str.length();
	if (len >= 2) {
		if ((str[0] == '\"' && str[len-1] == '\"') || (str[0] == '\'' && str[len-1] == '\'')) {
			str.erase(len-1);
			str.erase(0, 1);
		}
	}
}

void str_try_add_quote(string& str) {
	if (str.find(' ') == string::npos) return;
	str.insert(0, "\"");
	str += "\"";
}

bool str_only_space(const string& str) {
	unsigned int len = str.length();
	for (unsigned int i = 0; i < len; i++) {
		if (str[i] != ' ') return false;
	}
	return true;
}

void str_trim_right(string& str) {
	int len = str.length();
	if (len > 0) {
		bool is_space;
		int pos = len;
		do {
			pos--;
			char ch = str.at(pos);
			is_space = (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n');
		} while (pos > 0 && is_space);
		if (pos == 0 && is_space) {
			str = "";
		} else if (pos < len-1) {
			str.erase(pos+1);
		}
	}
}

void str_trim_left(string& str) {
	int len = str.length();
	if (len > 0) {
		bool is_space;
		int pos = -1;
		do {
			pos++;
			char ch = str.at(pos);
			is_space = (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n');
		} while (pos < len-1 && is_space);
		if (pos >= len-1 && is_space) {
			str = "";
		} else if (pos > 0) {
			str.erase(0,pos);
		}
	}
}

void str_trim_left(string& str, string& prefix) {
	int len = str.length();
	if (len > 0) {
		char ch;
		bool is_space;
		int pos = -1;
		do {
			pos++;
			ch = str.at(pos);
			is_space = (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n');
		} while (pos < len-1 && is_space);
		if (pos >= len-1 && is_space) {
			prefix = str;
			str = "";
		} else if (pos > 0) {
			prefix = str.substr(0,pos);
			str.erase(0,pos);
		}
	}
}

void str_trim_left_bom(string& str) {
	int len = str.length();
	if (len >= 3 && ((unsigned char)str[0]) == 0xEF &&
	    ((unsigned char)str[1]) == 0xBB && ((unsigned char)str[2]) == 0xBF) {
		str.erase(0, 3);
	}
}

void str_trim_both(string& str) {
	str_trim_right(str);
	str_trim_left(str);
}

void str_replace_all(char* str, const char* find, const char* repl) {
	char* find_res = str_i_str((const char*)str, find);
	int len_repl = strlen(repl);
	int len_find = strlen(find);
	while (find_res != NULL) {
		int pos = (find_res - (char*)str);
		int len = strlen((const char*)str);
		for (int k = len; k > pos; k--) {
			str[k+len_repl-len_find] = str[k];
		}
		strncpy((char*)str+pos, repl, len_repl);
		find_res = str_i_str((const char*)str, find);
	}
}

void str_replace_all(string& str, const char* find, const char* repl) {
	int find_res = str_i_str(str, find);
	int len_find = strlen(find);
	int len_repl = strlen(repl);
	while (find_res != -1) {
		str.erase(find_res, len_find);
		str.insert(find_res, repl);
		find_res = str_i_str(str, find_res+len_repl, find);
	}
}

void str_parse_get_next(const string& strg, const char* find, string& res) {
	char_separator sep(" \n\r", "");
	tokenizer<char_separator> tok(strg, sep);
	while (tok.has_more()) {
		string token = tok.next_token();
		if (str_i_equals(token, find) && tok.has_more()) {
			res = tok.next_token();
			break;
		}
	}
}

void gle_int_to_string(int value, string* str) {
	char str_value[80];
	sprintf(str_value, "%d", value);
	*str = str_value;
}

int gle_double_digits(double value, int prec) {
	if (value == 0.0) {
		return 0;
	} else {
		int digits = (int)floor(log10(value));
		// Round value to desired precision, and check if overflows, i.e., > 10
		value = floor(value / pow(10.0, digits-prec+1)+0.5+1e-6) / pow(10.0, prec-1);
		return value >= 10.0 ? digits+1 : digits;
	}
}

int gle_int_digits(int value) {
	return (int)floor(log10((double)value));
}

void gle_int_to_string_bin(int value, string* binary) {
	vector<unsigned char> values;
	while (value > 0) {
		values.push_back((unsigned char)(value % 2));
		value /= 2;
	}
	stringstream out;
	for (int i = values.size()-1; i >= 0; i--) {
		out << (int)values[i];
	}
	*binary = out.str();
}

int lastchar(char *s, char c) {
	while (*s != '\0') { s++; }
	return *(--s) == c;
}

char *un_quote(char *cts) {
	int i = strlen(cts);
	if (*cts == '"') {
		*(cts+i-1) = 0;
		cts = cts + 1;
	}
	return cts;
}

int gle_pow_ii(int x, int n) {
	int pow = 1;
	if(n > 0) {
		while (true) {
			if ((n & 1) != 0) {
				pow *= x;
			}
			if ((n >>= 1) != 0) {
				x *= x;
			} else {
				break;
			}
		}
	}
	return pow;
}

int f_pow_ii(int* ap, int* bp) {
	return gle_pow_ii(*ap, *bp);
}

double f_r_sign(double* a, double* b) {
	double x = *a >= 0 ? *a : - *a;
	return *b >= 0 ? x : -x;
}

int f_i_sign(int *a, int* b) {
	int x = *a >= 0 ? *a : - *a;
	return *b >= 0 ? x : -x;
}

int gle_round_int(double val) {
	return (int)floor(val + 0.5);
}

int gle_make_zero_based(int val) {
	if (val >= 1) {
		return val - 1;
	} else {
		return val;
	}
}

void str_get_system_error(ostream& error) {
	int error_id = errno;
	char* error_str = strerror(error_id);
	if (error_str == NULL || error_str[0] == 0) {
		error << "error #" << error_id;
	} else {
		error << error_str;
	}
}

#if defined(__UNIX__)
	string str_format(const char* format, ...) {
#else
	string str_format(va_list format, ...) {
#endif
	va_list ap;
	va_start(ap, format);
	string result;
	str_format(&result, (const char*)format, ap);
	va_end(ap);
	return result;
}

void str_format(string* str, const char *format, va_list ap) {
	/* this could be implemented with vsnprintf, but this does not seem to */
	/* be compatible with many different OSes */
	int pos = 0;
	stringstream sstr;
	while (format[pos] != 0) {
		if (format[pos] == '%') {
			if (format[pos+1] == '%') {
				sstr << (char)'%';
				pos++;
			} else if (format[pos+1] == 'd') {
				int val = va_arg(ap, int);
				sstr << val;
				pos++;
			} else if (format[pos+1] == 's') {
				char* val = va_arg(ap, char*);
				sstr << val;
				pos++;
			} else {
				sstr << (char)format[pos];
			}
		} else {
			sstr << (char)format[pos];
		}
		pos++;
	}
	*str = sstr.str();
}

int gle_pass_hex(const char* str, int from, int digits, int* err) {
	int value = 0;
	for (int i = 0; i < digits; i++) {
		value *= 16;
		int digit = str[from + i];
		if (digit >= '0' && digit <= '9') {
			value += digit - '0';
		} else if (digit >= 'a' && digit <= 'f') {
			value += digit - 'a' + 10;
		} else if (digit >= 'A' && digit <= 'F') {
			value += digit - 'A' + 10;
		} else {
			*err = from + i;
		}
	}
	return value;
}

void bool_vector_set_expand(vector<bool>* v, unsigned int i, bool value) {
	while (v->size() <= i) {
		v->push_back(false);
	}
	(*v)[i] = value;
}

bool bool_vector_is(vector<bool>* v, unsigned int i) {
	if (i >= v->size()) return false;
	else return (*v)[i];
}

void split_into_lines(const std::vector<unsigned char>* input, std::vector<std::string>* output) {
   bool done = false;
   unsigned int pos = 0;
   while (!done) {
      std::ostringstream crLine;
      bool lineDone = false;
      while (!lineDone) {
         if (pos >= input->size()) {
            done = true;
            lineDone = true;
         } else {
            char ch = (char)input->at(pos++);
            if (ch == '\n' || ch == '\r') {
               if (pos < input->size() && input->at(pos) != ch && (input->at(pos) == '\n' || input->at(pos) == '\r')) {
                  pos++;
               }
               lineDone = true;
            } else {
               crLine << ch;
            }
         }
      }
      output->push_back(crLine.str());
   }
}

void CUtilsAssertImpl(const char* expr, const char* file, int line, const char* function) {
	cerr << "Internal error: '" << expr << "' in file '" << file << "' function: '" << function << "' line: " << line << endl;
	exit(1);
}

double gle_limit_range(double value, double minValue, double maxValue)
{
	return std::min<double>(std::max<double>(value, minValue), maxValue);
}
