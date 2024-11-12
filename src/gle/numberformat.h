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

#ifndef NUMBER_FORMAT_H
#define NUMBER_FORMAT_H

class GLENumberFormat;

class GLENumberFormatter {
protected:
	int m_Prefix;
	bool m_NoZeroes;
	bool m_Sign;
	int m_PadLeft;
	int m_PadRight;
	double m_Min;
	double m_Max;
	bool m_HasMin;
	bool m_HasMax;
	std::string m_Prepend;
	std::string m_Append;
public:
	GLENumberFormatter();
	virtual ~GLENumberFormatter();
	virtual void parseOptions(GLENumberFormat* format);
	virtual void format(double number, std::string* output);
	virtual bool appliesTo(double number);
	inline bool hasPrefix() { return m_Prefix != -1; }
	inline int getPrefix() { return m_Prefix; }
	inline void setPrefix(int prefix) { m_Prefix = prefix;  }
	inline bool hasNoZeroes() { return m_NoZeroes; }
	inline void setNoZeroes(bool noz) { m_NoZeroes = noz; }
	inline bool hasSign() { return m_Sign; }
	inline void setSign(bool sign) { m_Sign = sign; }
	void doPrefix(std::string* output);
	void doNoZeroes(std::string* output);
	void doSign(std::string* output);
	void doPadLeft(std::string* output);
	void doPadRight(std::string* output);
	void doAll(std::string* output);
	inline bool hasPadRight() { return m_PadRight != -1; }
	inline int getPadRight() { return m_PadRight; }
	inline void setPadRight(int PadRight) { m_PadRight = PadRight;  }
	inline bool hasPadLeft() { return m_PadLeft != -1; }
	inline int getPadLeft() { return m_PadLeft; }
	inline void setPadLeft(int PadLeft) { m_PadLeft = PadLeft;  }
	inline std::string& getPrepend() { return m_Prepend; }
	inline std::string& getAppend() { return m_Append; }
	void setMin(double min);
	void setMax(double max);
	inline bool hasMax() { return m_HasMax; }
	inline bool hasMin() { return m_HasMin; }
	inline double getMax() { return m_Max; }
	inline double getMin() { return m_Min; }
	void setDefaults(GLENumberFormatter* def);
	void formatSimple(double value, std::string* output, int prec, int* exp);
};

class GLENumberFormat {
protected:
	level_char_separator m_Separator;
	tokenizer<level_char_separator> m_Tokens;
	std::vector<GLENumberFormatter*> m_Format;
	GLENumberFormatter m_Default;
public:
	GLENumberFormat(const std::string& format);
	~GLENumberFormat();
	inline bool hasMoreTokens() { return m_Tokens.has_more(); }
	inline const std::string& nextToken() { return m_Tokens.cr_token(); }
	inline void addFormat(GLENumberFormatter* format)  { m_Format.push_back(format); }
	void format(double number, std::string* output);
	void incTokens();
	int nextInt();
	double nextDouble();
	void nextString(std::string& value);
};

#endif
