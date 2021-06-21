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

/*
 * 2004 Jan Struyf
 *
 */

#ifndef INCLUDE_CMDLINE
#define INCLUDE_CMDLINE

class CmdLineOptionList;
class CmdLineOption;

bool cmdline_is_option(const char* opt, const char* val);

class CmdLineOptionArg {
protected:
	int m_MinCard, m_MaxCard, m_Card;
	string m_Help, m_Name;
	CmdLineOption* m_Option;
public:
	CmdLineOptionArg(const char* name);
	virtual ~CmdLineOptionArg();
	virtual bool isDefault() = 0;
	virtual void reset() = 0;
	virtual void write(ostream& os) = 0;
	virtual bool addValue(const string& value) = 0;
	virtual bool appendValue(const string& value);
	virtual void showExtraHelp();
	virtual void initArg();
	virtual void setDefaultValue();
	virtual bool needsComma();
	void initShowError();
	CmdLineOptionList* getObject();
	inline int getCard() { return m_Card; }
	inline int getMinCard() { return m_MinCard; }
	inline int getMaxCard() { return m_MaxCard; }
	inline void setCardLimits(int min, int max) { m_MinCard = min; m_MaxCard = max; }
	inline void setMinCard(int card) { m_MinCard = card; }
	inline void setMaxCard(int card) { m_MaxCard = card; }
	inline const string& getHelp() { return m_Help; }
	inline void setHelp(const char* help) { m_Help = help; }
	inline const string& getName() { return m_Name; }
	inline CmdLineOption* getOption() { return m_Option; }
	inline void setOption(CmdLineOption* option) { m_Option = option; }
};

class CmdLineArgString : public CmdLineOptionArg {
protected:
	bool m_Unquote;
	string m_Value;
	string m_Default;
public:
	CmdLineArgString(const char* name, bool unquote = true);
	~CmdLineArgString();
	virtual bool isDefault();
	virtual void reset();
	virtual void write(ostream& os);
	virtual bool addValue(const string& value);
	virtual bool appendValue(const string& value);
	virtual void setDefaultValue();
	virtual bool needsComma();
	void setValue(const char* value);
	inline const string& getValue() { return m_Value; }
	inline void setDefault(const char* value) { m_Default = value; }
	inline const string& getDefault() { return m_Default; }
	inline string* getValuePtr() { return &m_Value; }
};

class CmdLineArgSPairList : public CmdLineOptionArg {
protected:
	vector<string> m_Value1;
	vector<string> m_Value2;
public:
	CmdLineArgSPairList(const char* name);
	~CmdLineArgSPairList();
	virtual bool isDefault();
	virtual void reset();
	virtual void write(ostream& os);
	virtual bool addValue(const string& value);
	virtual bool appendValue(const string& value);
	virtual void setDefaultValue();
	void addPair(const string& s1, const string& s2);
	void addPairValue2(const string& s2);
	bool hasValue2(const string& s2);
	const string* lookup(const string& s1);
	inline string& getValue1(int i) { return m_Value1[i]; }
	inline string& getValue2(int i) { return m_Value2[i]; }
	inline void setValue1(int i, const string& s1) { m_Value1[i] = s1; }
	inline void setValue2(int i, const string& s2) { m_Value2[i] = s2; }
	inline int size() { return m_Value1.size(); }
};

class CmdLineArgInt : public CmdLineOptionArg {
protected:
	int m_Value;
	int m_Default;
public:
	CmdLineArgInt(const char* name);
	~CmdLineArgInt();
	virtual bool isDefault();
	virtual void reset();
	virtual void write(ostream& os);
	virtual bool addValue(const string& value);
	virtual void setDefaultValue();
	virtual void setValue(int value);
	inline const int getValue() { return m_Value; }
	inline void setDefault(int value) { m_Default = value; }
};

#define CMDLINE_NO          0
#define CMDLINE_YES         1
#define CMDLINE_UNSUPPORTED 2

class CmdLineArgSet : public CmdLineOptionArg {
protected:
	vector<string> m_Values;
	vector<int> m_HasValue;
	vector<int> m_Defaults;
public:
	CmdLineArgSet(const char* name);
	~CmdLineArgSet();
	virtual bool isDefault();
	virtual void reset();
	virtual void write(ostream& os);
	virtual bool addValue(const string& value);
	virtual void showExtraHelp();
	virtual void setDefaultValue();
	int getFirstValue();
	void addValue(int i);
	void removeValue(int i);
	void addPossibleValue(const char* value);
	bool hasOnlyValue(int id);
	vector<string> getValues();
	inline void setUnsupportedValue(int id) { m_HasValue[id] = CMDLINE_UNSUPPORTED; }
	inline void addDefaultValue(int id) { m_Defaults.push_back(id); }
	inline int getNbValues() { return m_Values.size(); }
	inline bool hasValue(int id) { return m_HasValue[id] == CMDLINE_YES; }
	inline const string& getStringValue(int id) { return m_Values[id]; }
};

class CmdLineOption {
protected:
	bool m_HasOption, m_Expert;
	int m_MinNbArgs, m_NbArgs;
	vector<string> m_Names;
	vector<CmdLineOptionArg*> m_Args;
	CmdLineOptionList* m_Object;
	string m_Help;
public:
	CmdLineOption(const char* name);
	CmdLineOption(const char* name, const char* alias);
	CmdLineOption(const char* name, const char* alias1, const char* alias2);
	~CmdLineOption();
	void initialize();
	void deleteArgs();
	void addAlias(const char* alias);
	void showHelp();
	void initOption();
	void setDefaultValues();
	void addArg(CmdLineOptionArg* arg);
	bool allDefaults();
	inline int getNbNames() { return m_Names.size(); }
	inline const string& getName(int i) { return m_Names[i]; }
	inline const string& getName() { return m_Names[0]; }
	inline int getMinNbArgs() { return m_MinNbArgs; }
	inline int getMaxNbArgs() { return m_Args.size(); }
	inline int getNbArgs() { return m_NbArgs; }
	inline CmdLineOptionArg* getArg(int i) { return m_Args[i]; }
	inline bool hasOption() { return m_HasOption; }
	inline void setHasOption(bool has) { m_HasOption = has; }
	inline void setMinNbArgs(int min) { m_MinNbArgs = min; }
	inline const string& getHelp() { return m_Help; }
	inline void setHelp(const char* help) { m_Help = help; }
	inline bool isExpert() { return m_Expert; }
	inline void setExpert(bool expert) { m_Expert = expert; }
	inline CmdLineOptionList* getObject() { return m_Object; }
	inline void setObject(CmdLineOptionList* obj) { m_Object = obj; }
	inline bool hasArgument() { return getArg(0)->getCard() != 0; }
};

class CmdLineOptionList {
protected:
	vector<CmdLineOption*> m_Options;
	int m_Error;
public:
	CmdLineOptionList();
	~CmdLineOptionList();
	void addOption(CmdLineOption* option, int id);
	CmdLineOption* getOption(const string& name);
	CmdLineOption* createOption(int id);
	CmdLineArgString* addStringOption(const char* name, int id);
	CmdLineArgSPairList* addSPairListOption(const char* name, int id);
	void deleteOptions();
	char getOptionPrefix();
	void showHelp(int helpid);
	void initOptions();
	void setDefaultValues();
	bool allDefaults();
	void clearAll();
	bool hasOption(const string& name);
	bool hasOption(int id);
	void setHasOption(const string& name);
	void setOptionString(const string& name, const string& value, int arg = 0);
	void setHasOption(int id, bool set);
	inline int hasError() { return m_Error == 1; }
	inline int getNbOptions() { return m_Options.size(); }
	inline CmdLineOption* getOption(int i) { return m_Options[i]; }
	inline CmdLineOptionArg* getOptionValue(int i) { return m_Options[i]->getArg(0); }
	inline const string& getOptionString(int id, int nr = 0) {
		return ((CmdLineArgString*)m_Options[id]->getArg(nr))->getValue();
	}
	inline int getOptionInt(int id, int nr = 0) {
		return ((CmdLineArgInt*)m_Options[id]->getArg(nr))->getValue();
	}
};

class ConfigSection : public CmdLineOptionList {
protected:
	string m_Name;
public:
	ConfigSection(const char* name);
	~ConfigSection();
	inline const string& getName() { return m_Name; }
	inline void setName(const char* name) { m_Name = name; }
};

class ConfigCollection {
protected:
	vector<ConfigSection*> m_Sections;
public:
	ConfigCollection();
	~ConfigCollection();
	void deleteSections();
	bool allDefaults();
	void setDefaultValues();
	void addSection(ConfigSection* section, int id);
	ConfigSection* getSection(const string& name);
	const string& getStringValue(int section, int value);
	void setStringValue(int section, int value, const char* str);
	inline int getNbSections() { return m_Sections.size(); }
	inline ConfigSection* getSection(int id) { return m_Sections[id]; };
};

class CmdLineObj : public CmdLineOptionList {
protected:
	string m_MainArgType;
	vector<string> m_MainArgs;
	vector<string> m_MArgSep;
	int m_ArgC, m_ArgIdx;
	int m_MArgSepPos;
	bool m_HasStdin;
	char** m_ArgV;
public:
	CmdLineObj();
	~CmdLineObj();
	void parse(int argc, char** argv);
	const char* getNextArg();
	void addOptionArg(CmdLineOption* cropt, int argidx, const string& crarg);
	bool parseOptionArg(bool inmainargs, const string& optionname, int argidx, CmdLineOption** cropt_p);
	bool isMainArgSeparator(const string& arg);
	int getNbMainArgs();
	const string& getMainArg(int i);
	int getNbExtraArgs();
	const string& getExtraArg(int i);
	const string& getStringValue(int option, int arg = 0);
	int getIntValue(int option, int arg = 0);
	void setIntValue(int option, int value, int arg = 0);
	bool checkForStdin();
	inline bool hasStdin() { return m_HasStdin; }
	inline bool supportsExtraArgs() { return m_MArgSep.size() != 0; }
	inline const string& getMainArgSep(int i) { return m_MArgSep[i]; }
	inline int getMainArgSepPos() { return m_MArgSepPos; }
	inline void setMainArgSepPos(int pos) { m_MArgSepPos = pos; }
	inline void setMainArgType(const char* type) { m_MainArgType = type; }
	inline void addMainArgSep(const char* sep) { m_MArgSep.push_back(sep); }
	inline vector<string>* getMainArgs() { return &m_MainArgs; }
};

#endif
