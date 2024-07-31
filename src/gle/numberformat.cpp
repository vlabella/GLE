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

#include "all.h"
#include "tokens/stokenizer.h"
#include "cutils.h"
#include "core.h"
#include "gprint.h"
#include "numberformat.h"

/*
	dec, hex [upper|lower], bin
	01

	fix 3 + effect van nozeros
	12.000

	sig 2 [e,E,10,10+] expdigits 2 expsign

	round 3
	round number to 3 significant digits
	pad with zeros if number too large (don't use exponential representations)

	nozeros

	sign
	always include sign in output (also for positive numbers)

	pad 7 [left|right]
	numbers less than 7 chars are padded from left with spaces

	prefix 3 (add zeros from left)

	append '%'
	prepend 'EUR '
*/

class GLENumberFormatterFix : public GLENumberFormatter {
protected:
	int m_NbDecPlaces;
public:
	virtual ~GLENumberFormatterFix();
	virtual void parseOptions(GLENumberFormat* format);
	virtual void format(double number, string* output);
};

class GLENumberFormatterPercent : public GLENumberFormatter {
protected:
	int m_NbDecPlaces;
public:
	virtual ~GLENumberFormatterPercent();
	virtual void parseOptions(GLENumberFormat* format);
	virtual void format(double number, string* output);
};

class GLENumberFormatterRound : public GLENumberFormatter {
protected:
	int m_Sig;
public:
	virtual ~GLENumberFormatterRound();
	virtual void parseOptions(GLENumberFormat* format);
	virtual void format(double number, string* output);
};

class GLENumberFormatterSci : public GLENumberFormatter {
protected:
	int m_Sig;
	int m_Mode;
	int m_ExpDigits;
	bool m_HasExpDigits;
	bool m_ExpSign;
public:
	GLENumberFormatterSci();
	virtual ~GLENumberFormatterSci();
	void doAllSci(string* output);
	virtual void parseOptions(GLENumberFormat* format);
	void formatExpPart(int exp, string* output);
	virtual void format(double number, string* output);
	inline bool hasExpDigits() { return m_HasExpDigits; }
	inline int getExpDigits() { return m_ExpDigits; }
	void setExpDigits(int digits);
	inline bool hasExpSign() { return m_ExpSign; }
	inline void setExpSign(bool ExpSign) { m_ExpSign = ExpSign; }
};

class GLENumberFormatterEng : public GLENumberFormatterSci {
protected:
	int m_Digits;
	bool m_Numeric;
public:
	GLENumberFormatterEng();
	virtual ~GLENumberFormatterEng();
	virtual void parseOptions(GLENumberFormat* format);
	virtual void format(double number, string* output);
	void myDoAll(string* output);
};

class GLENumberFormatterInt : public GLENumberFormatter {
protected:
	int m_Mode;
	bool m_Upper;
public:
	GLENumberFormatterInt(int mode);
	virtual ~GLENumberFormatterInt();
	virtual void format(double number, string* output);
	virtual void parseOptions(GLENumberFormat* format);
	inline bool hasUpper() { return m_Upper; }
	inline void setUpper(bool Upper) { m_Upper = Upper; }
};

class GLENumberFormatterFrac: public GLENumberFormatter {
protected:
	int m_Mode;
public:
	GLENumberFormatterFrac(int mode);
	virtual ~GLENumberFormatterFrac();
	virtual void format(double number, string* output);
	virtual void parseOptions(GLENumberFormat* format);
};

#define GLE_NF_INT_DEC 0
#define GLE_NF_INT_HEX 1
#define GLE_NF_INT_BIN 2

#define GLE_NF_SCI_SMALL_E 0
#define GLE_NF_SCI_BIG_E   1
#define GLE_NF_SCI_10      2

#define GLE_NF_FRAC_ONE    0
#define GLE_NF_FRAC_PI     1

GLENumberFormatter::GLENumberFormatter() {
	m_Prefix = -1;
	m_NoZeroes = false;
	m_Sign = false;
	m_PadLeft = -1;
	m_PadRight = -1;
	m_HasMin = false;
	m_HasMax = false;
}

GLENumberFormatter::~GLENumberFormatter() {
}

void GLENumberFormatter::parseOptions(GLENumberFormat* format) {
}

void GLENumberFormatter::format(double number, string* output) {
}

bool GLENumberFormatter::appliesTo(double number) {
	if (hasMin() && number < getMin()) return false;
	if (hasMax() && number > getMax()) return false;
	return true;
}

void GLENumberFormatter::doAll(string* output) {
	doNoZeroes(output);
	doPrefix(output);
	doSign(output);
	doPadLeft(output);
	doPadRight(output);
}

void GLENumberFormatter::doPrefix(string* output) {
	if (hasPrefix()) {
		bool has_sign = false;
		int prefix = getPrefix();
		int length = output->length();
		string::size_type pos = output->rfind('.');
		if (pos == string::npos) pos = length;
		if (length > 0 && output->at(0) == '-') {
			prefix++;
			has_sign = true;
		}
		if (pos < (unsigned int)prefix) {
			string zeros = has_sign ? "-" : "";
			for (unsigned int i = 0; i < prefix-pos; i++) {
				zeros += "0";
			}
			if (has_sign) {
				zeros += output->substr(1, length-1);
			} else {
				zeros += *output;
			}
			*output = zeros;
		}
	}
}

void GLENumberFormatter::doNoZeroes(string* output) {
	if (hasNoZeroes() && output->rfind('.') != string::npos) {
		int nbzero = 0;
		int length = output->length();
		int pos = length-1;
		while (pos >= 0 && output->at(pos) == '0') {
			pos--; nbzero++;
		}
		// also eat '.', e.g., if number was 3.0 (not for 3.10)
		if (pos >= 0 && output->at(pos) == '.') {
			pos--; nbzero++;
		}
		*output = output->substr(0, length-nbzero);
	}
}

void GLENumberFormatter::doSign(string* output) {
	if (hasSign()) {
		if (output->length() > 0 && output->at(0) != '-') {
			output->insert(0, "+");
		}
	}
}

void GLENumberFormatter::doPadLeft(string* output) {
	if (getPrepend() != "") {
		output->insert(0, getPrepend());
	}
	if (hasPadLeft()) {
		int count = getPadLeft() - output->length();
		str_prefix(count, ' ', output);
	}
}

void GLENumberFormatter::doPadRight(string* output) {
	if (getAppend() != "") {
		output->append(getAppend());
	}
	if (hasPadRight()) {
		int count = getPadRight() - output->length();
		if (count > 0) {
			for (int i = 0; i < count; i++) {
				*output += " ";
			}
		}
	}
}

void GLENumberFormatter::setMin(double min) {
	m_Min = min;
	m_HasMin = true;
}

void GLENumberFormatter::setMax(double max) {
	m_Max = max;
	m_HasMax = true;
}

void GLENumberFormatter::setDefaults(GLENumberFormatter* def) {
	if (def->hasPrefix()) setPrefix(def->getPrefix());
	if (def->hasNoZeroes()) setNoZeroes(true);
	if (def->hasSign()) setSign(true);
	if (def->hasPadLeft()) setPadLeft(def->getPadLeft());
	if (def->hasPadRight()) setPadRight(def->getPadRight());
}

void GLENumberFormatter::formatSimple(double value, string* output, int prec, int* exp) {
	char format[20], result[100];
	double pos_num = fabs(value);
	*exp = gle_double_digits(pos_num, prec);
	if (prec > 0) {
		sprintf(format, "%%.%df", prec-1);
		sprintf(result, format, pos_num / pow(10.0, *exp));
	} else {
		result[0] = 0;
	}
	*output = result;
}

GLENumberFormatterFix::~GLENumberFormatterFix() {
}

void GLENumberFormatterFix::parseOptions(GLENumberFormat* format) {
	m_NbDecPlaces = format->nextInt();
}

void GLENumberFormatterFix::format(double number, string* output) {
	char format[20], result[100];
	sprintf(format, "%%.%df", m_NbDecPlaces);
	sprintf(result, format, number);
	*output = result;
	doAll(output);
}

GLENumberFormatterPercent::~GLENumberFormatterPercent() {
}

void GLENumberFormatterPercent::parseOptions(GLENumberFormat* format) {
	m_NbDecPlaces = format->nextInt();
}

void GLENumberFormatterPercent::format(double number, string* output) {
	char format[20], result[100];
	sprintf(format, "%%.%df", m_NbDecPlaces);
	sprintf(result, format, 100*number);
	*output = result;
	*output += "%";
	doAll(output);
}

GLENumberFormatterSci::GLENumberFormatterSci() {
	m_Mode = GLE_NF_SCI_SMALL_E;
	m_HasExpDigits = false;
	m_ExpSign = false;
}

GLENumberFormatterSci::~GLENumberFormatterSci() {
}

void GLENumberFormatterSci::setExpDigits(int digits) {
	m_ExpDigits = digits;
	m_HasExpDigits = true;
}

void GLENumberFormatterSci::parseOptions(GLENumberFormat* format) {
	m_Sig = format->nextInt();
	while (format->hasMoreTokens()) {
		const string& tpe = format->nextToken();
		if (tpe == "e") {
			m_Mode = GLE_NF_SCI_SMALL_E;
			format->incTokens();
		} else if (tpe == "E") {
			m_Mode = GLE_NF_SCI_BIG_E;
			format->incTokens();
		} else if (tpe == "10") {
			m_Mode = GLE_NF_SCI_10;
			format->incTokens();
		} else if (tpe == "expdigits") {
			format->incTokens();
			setExpDigits(format->nextInt());
		} else if (tpe == "expsign") {
			format->incTokens();
			setExpSign(true);
		} else {
			break;
		}
	}
}

void GLENumberFormatterSci::formatExpPart(int exp, string* output) {
	string exp_str;
	int abs_exp = exp > 0 ? exp : -exp;
	gle_int_to_string(abs_exp, &exp_str);
	if (hasExpDigits()) {
		int nb_digits = exp_str.length();
		str_prefix(getExpDigits()-nb_digits, '0', &exp_str);
	}
	if (exp < 0) exp_str.insert(0, "-");
	else if (hasExpSign()) exp_str.insert(0, "+");
	doNoZeroes(output);
	switch (m_Mode) {
		case GLE_NF_SCI_SMALL_E:
			*output += "e";
			*output += exp_str;
			break;
		case GLE_NF_SCI_BIG_E:
			*output += "E";
			*output += exp_str;
			break;
		case GLE_NF_SCI_10:
			{
				ostringstream toAdd;
				if (g_get_tex_labels()) toAdd << "$";
				if (output->length() != 0) toAdd << "\\cdot ";
				toAdd << "10^{" << exp_str << "}";
				if (g_get_tex_labels()) toAdd << "$";
				*output += toAdd.str();
			}
			break;
	}
}

void GLENumberFormatterSci::format(double number, string* output) {
	int exp;
	formatSimple(number, output, m_Sig, &exp);
	formatExpPart(exp, output);
	if (number < 0.0) output->insert(0, "-");
	doAllSci(output);
}

void GLENumberFormatterSci::doAllSci(string* output) {
	// no doNoZeroes() here! -> this is done before adding the "Exp" part
	doPrefix(output);
	doSign(output);
	doPadLeft(output);
	doPadRight(output);
}

GLENumberFormatterEng::GLENumberFormatterEng() : GLENumberFormatterSci() {
	m_Numeric = false;
	m_Digits = 0;
}

GLENumberFormatterEng::~GLENumberFormatterEng() {
}

void GLENumberFormatterEng::parseOptions(GLENumberFormat* format) {
	m_Numeric = false;
	m_Digits = format->nextInt();
	if (m_Digits < 0) m_Digits = 0;
	while (format->hasMoreTokens()) {
		const string& tpe = format->nextToken();
		if (tpe == "e") {
			m_Mode = GLE_NF_SCI_SMALL_E;
			format->incTokens();
		} else if (tpe == "E") {
			m_Mode = GLE_NF_SCI_BIG_E;
			format->incTokens();
		} else if (tpe == "10") {
			m_Mode = GLE_NF_SCI_10;
			format->incTokens();
		} else if (tpe == "expdigits") {
			format->incTokens();
			setExpDigits(format->nextInt());
		} else if (tpe == "expsign") {
			format->incTokens();
			setExpSign(true);
		} else if (tpe == "num") {
			m_Numeric = true;
			format->incTokens();
		} else {
			break;
		}
	}
}

#define GLE_FMT_ENG_PREFIX_START (-24)
#define GLE_FMT_ENG_PREFIX_END   (GLE_FMT_ENG_PREFIX_START+(int)((sizeof(prefix)/sizeof(char *)-1)*3))

void GLENumberFormatterEng::format(double number, string* output) {
	static const char *prefix[] = {
	    "y", "z", "a", "f", "p", "n", "\\mu{}", "m", "",
	    "k", "M", "G", "T", "P", "E", "Z", "Y"
	};
	int expof10;
	char result[100];
	char *res = result;
	int digits = m_Digits;
	result[0] = 0;
	if (number == 0.0) {
		// take special care of the number zero
		if (digits != 0) {
			sprintf(res, "%.*f", digits-1, number);
			*output = result;
			doNoZeroes(output);
			if (!m_Numeric) {
				*output += " ";
			}
		}
		myDoAll(output);
		return;
	}
	if (number < 0.0) {
		*res++ = '-';
		number = -number;
	}
	expof10 = (int) log10(number);
	if (expof10 > 0) expof10 = (expof10/3)*3;
	else expof10 = (-expof10+3)/3*(-3);
	number *= pow(10.0, -expof10);
	if (number >= 1000.) {
		number /= 1000.0; expof10 += 3;
	} else if (number >= 100.0) {
		digits -= 2;
	} else if (number >= 10.0) {
		digits -= 1;
	}
	if (m_Digits == 0) {
		// no significant digits?
		if (m_Numeric || (expof10 < GLE_FMT_ENG_PREFIX_START) ||
			         (expof10 > GLE_FMT_ENG_PREFIX_END)) {
			res[0] = 0;
			*output = result;
			formatExpPart(expof10, output);
		} else {
			const char* fmt = g_get_tex_labels() ? "$\\mathrm{%s}$" : "%s";
			sprintf(res, fmt, prefix[(expof10-GLE_FMT_ENG_PREFIX_START)/3]);
			*output = result;
		}
		myDoAll(output);
		return;
	}
	while (digits <= 0) {
		// too few significant digits (happens with eng 2)
		number /= 1000.0;
		expof10 += 3;
		digits = m_Digits;
		if (number >= 100.0) {
			digits -= 2;
		} else if (number >= 10.0) {
			digits -= 1;
		}
	}
	if (m_Numeric || (expof10 < GLE_FMT_ENG_PREFIX_START) ||
		         (expof10 > GLE_FMT_ENG_PREFIX_END)) {
		sprintf(res, "%.*f", digits-1, number);
		*output = result;
		formatExpPart(expof10, output);
	} else {
		sprintf(res, "%.*f", digits-1, number);
		*output = result;
		doNoZeroes(output);
		*output += " ";
		if (g_get_tex_labels()) *output += "$\\mathrm{";
		*output += prefix[(expof10-GLE_FMT_ENG_PREFIX_START)/3];
		if (g_get_tex_labels()) *output += "}$";
	}
	myDoAll(output);
}

void GLENumberFormatterEng::myDoAll(string* output) {
	if (getAppend() == "") {
		// no append, then remove trailing space introduced if "eng" prefix is empty
		str_trim_right(*output);
	}
	doAllSci(output);
}

GLENumberFormatterRound::~GLENumberFormatterRound() {
}

void GLENumberFormatterRound::parseOptions(GLENumberFormat* format) {
	m_Sig = format->nextInt();
}


void GLENumberFormatterRound::format(double number, string* output) {
	int exp;
	formatSimple(number, output, m_Sig, &exp);
	string::size_type pos = output->find('.');
	if (exp < 0) {
		if (pos != string::npos) output->erase(pos, 1);
		string prefix = "0.";
		for (int i = 0; i < -exp-1; i++) {
			prefix += "0";
		}
		*output = prefix + (*output);
	} else {
		if (pos != string::npos) {
			exp -= output->length() - pos - 1;
			output->erase(pos, 1);
			if (exp < 0) output->insert(output->length()+exp, ".");
		}
		for (int i = 0; i < exp; i++) {
			(*output) += "0";
		}
	}
	if (number < 0.0) output->insert(0, "-");
	doAll(output);
}

GLENumberFormatterInt::GLENumberFormatterInt(int mode) : GLENumberFormatter() {
	m_Mode = mode;
	m_Upper = true;
}

GLENumberFormatterInt::~GLENumberFormatterInt() {
}

void GLENumberFormatterInt::parseOptions(GLENumberFormat* format) {
	if (m_Mode == GLE_NF_INT_HEX) {
		const string& up = format->nextToken();
		if (up == "upper") {
			format->incTokens();
		} else if (up == "lower") {
			setUpper(false);
			format->incTokens();
		}
	}
}

void GLENumberFormatterInt::format(double number, string* output) {
	char result[100];
	int number_int = (int)floor(number + 0.5);
	switch (m_Mode) {
		case GLE_NF_INT_DEC:
			sprintf(result, "%d", number_int);
			*output = result;
			break;
		case GLE_NF_INT_HEX:
			if (hasUpper()) sprintf(result, "%X", number_int);
			else sprintf(result, "%x", number_int);
			*output = result;
			break;
		case GLE_NF_INT_BIN:
			gle_int_to_string_bin(number_int, output);
			break;
	}
	doAll(output);
}

GLENumberFormatterFrac::GLENumberFormatterFrac(int mode) {
	m_Mode = mode;
}

GLENumberFormatterFrac::~GLENumberFormatterFrac() {
}

void GLENumberFormatterFrac::format(double number, string* output) {
	bool sign = false;
	double num = number;
	if (num < 0.0) {
		sign = true;
		num = fabs(num);
	}
	if (m_Mode == GLE_NF_FRAC_PI) num /= GLE_PI;
	double int_part = floor(num);
	num = num - int_part;
	bool  found = false;
	float denom = 0.0;
	while (!found && denom <= 100.0) {
		denom = denom + 1.0;
		if (fabs(floor(num*denom+1e-7) - num*denom) < 1e-6) {
			found = true;
		}
	}
	if (found) {
		string str;
		num *= denom;
		num += int_part * denom;
		if (sign) *output += "-";
		if (m_Mode == GLE_NF_FRAC_PI) {
			if (floor(num+1e-7) != 1.0) {
				gle_int_to_string((int)floor(num+1e-7), &str);
				*output += str;
			}
			if (number != 0.0) {
				*output += g_get_tex_labels() ? "$\\pi$" : "\\pi";
			}
		} else {
			gle_int_to_string((int)floor(num+1e-7), &str);
			*output += str;
		}
		if (denom != 1.0) {
			*output += "/";
			gle_int_to_string((int)floor(denom+1e-7), &str);
			*output += str;
		}
	} else {
		char result[100];
		sprintf(result, "%f", number);
		*output = result;
	}
	doAll(output);
}

void GLENumberFormatterFrac::parseOptions(GLENumberFormat* format) {
}

GLENumberFormat::GLENumberFormat(const string& formats) :
		m_Separator(" \"", "", "\'", "\'"), m_Tokens(formats, m_Separator) {
	GLENumberFormatter* previous = &m_Default;
	while (hasMoreTokens()) {
		const string& name = nextToken();
		GLENumberFormatter* format = NULL;
		if (name == "fix") {
			incTokens();
			format = new GLENumberFormatterFix();
		} else if (name == "percent") {
			incTokens();
			format = new GLENumberFormatterPercent();
		} else if (name == "dec") {
			incTokens();
			format = new GLENumberFormatterInt(GLE_NF_INT_DEC);
		} else if (name == "hex") {
			incTokens();
			format = new GLENumberFormatterInt(GLE_NF_INT_HEX);
		} else if (name == "bin") {
			incTokens();
			format = new GLENumberFormatterInt(GLE_NF_INT_BIN);
		} else if (name == "round") {
			incTokens();
			format = new GLENumberFormatterRound();
		} else if (name == "sci") {
			incTokens();
			format = new GLENumberFormatterSci();
		} else if (name == "eng") {
			incTokens();
			format = new GLENumberFormatterEng();
		} else if (name == "frac") {
			incTokens();
			format = new GLENumberFormatterFrac(GLE_NF_FRAC_ONE);
		} else if (name == "pi") {
			incTokens();
			format = new GLENumberFormatterFrac(GLE_NF_FRAC_PI);
		} else if (name == "prefix") {
			incTokens();
			previous->setPrefix(nextInt());
		} else if (name == "nozeroes") {
			incTokens();
			previous->setNoZeroes(true);
		} else if (name == "nozero") {
			incTokens();
			previous->setNoZeroes(true);
		} else if (name == "sign") {
			incTokens();
			previous->setSign(true);
		} else if (name == "pad") {
			incTokens();
			int padvalue = nextInt();
			const string& lr = nextToken();
			if (lr == "left") {
				previous->setPadLeft(padvalue);
				incTokens();
			} else if (lr == "right") {
				previous->setPadRight(padvalue);
				incTokens();
			}
		} else if (name == "min") {
			incTokens();
			previous->setMin(nextDouble());
		} else if (name == "max") {
			incTokens();
			previous->setMax(nextDouble());
		} else if (name == "append") {
			incTokens();
			nextString(previous->getAppend());
		} else if (name == "add") {
			incTokens();
			nextString(previous->getAppend());
		} else if (name == "prepend") {
			incTokens();
			nextString(previous->getPrepend());
		} else if (name == "otherwise") {
			incTokens();
		} else {
			gprint("Unknown specifier in number format string: '%s'", name.c_str());
			incTokens();
		}
		if (format != NULL) {
			format->setDefaults(&m_Default);
			format->parseOptions(this);
			addFormat(format);
			previous = format;
		}
	}
}

GLENumberFormat::~GLENumberFormat() {
	for (vector<GLENumberFormatter*>::size_type i = 0; i < m_Format.size(); i++) {
		delete m_Format[i];
	}
}

void GLENumberFormat::format(double number, string* output) {
	for (vector<GLENumberFormatter*>::size_type i = 0; i < m_Format.size(); i++) {
		if (m_Format[i]->appliesTo(number)) {
			m_Format[i]->format(number, output);
			return;
		}
	}
	*output = "ERR";
}

void GLENumberFormat::incTokens() {
	if (hasMoreTokens()) m_Tokens.next_token();
}

int GLENumberFormat::nextInt() {
	if (hasMoreTokens()) {
		const string& token = nextToken();
		int result = atoi(token.c_str());
		incTokens();
		return result;
	} else {
		return 0;
	}
}

double GLENumberFormat::nextDouble() {
	char* err = NULL;
	const string& token = nextToken();
	double result = strtod(token.c_str(), &err);
	incTokens();
	return result;
}

void GLENumberFormat::nextString(string& value) {
	value = nextToken();
	str_remove_quote(value);
	incTokens();
}

std::string format_number_to_string(const std::string& format, double value) {
	string result;
	GLENumberFormat fmt(format);
	fmt.format(value, &result);
	return result;
}
