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

#include <string.h>
#include <string>
#include <vector>
#include <iostream>
#include <stdlib.h>

using namespace std;

#include "tokens/stokenizer.h"
#include "cmdline.h"
#include "cutils.h"

CmdLineOptionArg::CmdLineOptionArg(const char* name) {
	m_Name = name;
	m_MinCard = -1;
	m_MaxCard = -1;
	m_Card = 0;
}

CmdLineOptionArg::~CmdLineOptionArg() {
}

void CmdLineOptionArg::showExtraHelp() {
}

void CmdLineOptionArg::initArg() {
}

void CmdLineOptionArg::setDefaultValue() {
}

void CmdLineOptionArg::initShowError() {
	cerr << ">> Option " << getObject()->getOptionPrefix() << getOption()->getName();
	if (getOption()->getMaxNbArgs() != 1) {
		cerr << " argument '" << getName() << "'";
	}
}

bool CmdLineOptionArg::appendValue(const string& value) {
	return addValue(value);
}

CmdLineOptionList* CmdLineOptionArg::getObject() {
	return m_Option->getObject();
}

bool CmdLineOptionArg::needsComma() {
	return false;
}

CmdLineArgString::CmdLineArgString(const char* name, bool unquote) : CmdLineOptionArg(name) {
	m_Unquote = unquote;
	setMaxCard(1);
}

CmdLineArgString::~CmdLineArgString() {
}

void CmdLineArgString::write(ostream& os) {
	os << "\"" << m_Value << "\"";
}

void CmdLineArgString::reset() {
	m_Value = "";
	m_Card = 0;
}

bool CmdLineArgString::addValue(const string& value) {
	m_Value = value;
	if (m_Unquote) str_remove_quote(m_Value);
	m_Card++;
	return true;
}

void CmdLineArgString::setValue(const char* value) {
	m_Value = value;
	if (m_Unquote) str_remove_quote(m_Value);
	m_Card = 1;
}

bool CmdLineArgString::appendValue(const string& value) {
	if (m_Value == "") {
		m_Value = value;
		if (m_Unquote) str_remove_quote(m_Value);
	} else {
		string temp = value;
		if (m_Unquote) str_remove_quote(temp);
		m_Value += string(" ") + temp;
	}
	m_Card++;
	return true;
}

void CmdLineArgString::setDefaultValue() {
	m_Value = m_Default;
	m_Card++;
}

bool CmdLineArgString::isDefault() {
	return m_Value == m_Default;
}

bool CmdLineArgString::needsComma() {
	return true;
}

CmdLineArgSPairList::CmdLineArgSPairList(const char* name) : CmdLineOptionArg(name) {
	setMaxCard(1);
}

CmdLineArgSPairList::~CmdLineArgSPairList() {
}

void CmdLineArgSPairList::write(ostream& os) {
	if (size() != 0) {
		os << "\"" << getValue1(0) << "\",\"" << getValue2(0) << "\"" << endl;
		for (int i = 1; i < size(); i++) {
			os << "\t" << getName() << " += \"" << getValue1(i) << "\",\"" << getValue2(i) << "\"";
			if (i != size()-1) os << endl;
		}
	}
}

void CmdLineArgSPairList::reset() {
	m_Card = 0;
	m_Value1.clear();
	m_Value2.clear();
}

bool CmdLineArgSPairList::addValue(const string& value) {
	m_Card++;
	return true;
}

bool CmdLineArgSPairList::appendValue(const string& value) {
	level_char_separator separator(" ,", "", "\"", "\"");
	tokenizer<level_char_separator> tokens(value, separator);
	string s1 = tokens.has_more() ? tokens.next_token() : "?";
	string s2 = tokens.has_more() ? tokens.next_token() : "?";
	str_remove_quote(s1);
	str_remove_quote(s2);
	addPair(s1, s2);
	m_Card++;
	return true;
}

void CmdLineArgSPairList::setDefaultValue() {
	reset();
	m_Card++;
}

bool CmdLineArgSPairList::isDefault() {
	return m_Value1.size() == 0 && m_Value2.size() == 0;
}

void CmdLineArgSPairList::addPair(const string& s1, const string& s2) {
	m_Value1.push_back(s1);
	m_Value2.push_back(s2);
}

void CmdLineArgSPairList::addPairValue2(const string& s2) {
	m_Value1.push_back("");
	m_Value2.push_back(s2);
}

bool CmdLineArgSPairList::hasValue2(const string& s2) {
	for (vector<string>::size_type i = 0; i < m_Value2.size(); i++) {
		if (m_Value2[i] == s2) return true;
	}
	return false;
}

const string* CmdLineArgSPairList::lookup(const string& s1) {
	for (vector<string>::size_type i = 0; i < m_Value1.size(); i++) {
		if (m_Value1[i] == s1) return &m_Value2[i];
	}
	return NULL;
}

CmdLineArgInt::CmdLineArgInt(const char* name) : CmdLineOptionArg(name) {
	setMaxCard(1);
}

CmdLineArgInt::~CmdLineArgInt() {
}

void CmdLineArgInt::write(ostream& os) {
	os << m_Value;
}

void CmdLineArgInt::reset() {
	m_Value = 0;
	m_Card = 0;
}

bool CmdLineArgInt::addValue(const string& value) {
	for (string::size_type i = 0; i < value.length(); i++) {
		if (value[i] < '0' || value[i] > '9') {
			initShowError();
			cerr << " illegal value '" << value << "'" << endl;
			return false;
		}
	}
	m_Value = atoi(value.c_str());
	m_Card++;
	return true;
}

void CmdLineArgInt::setValue(int value) {
	m_Value = value;
	m_Card++;
}

void CmdLineArgInt::setDefaultValue() {
	m_Value = m_Default;
	m_Card++;
}

bool CmdLineArgInt::isDefault() {
	return m_Value == m_Default;
}

CmdLineArgSet::CmdLineArgSet(const char* name) : CmdLineOptionArg(name) {
}

CmdLineArgSet::~CmdLineArgSet() {
}

bool CmdLineArgSet::addValue(const string& value) {
	for (vector<string>::size_type i = 0; i < m_Values.size(); i++) {
		if (str_i_equals(m_Values[i], value)) {
			if (m_HasValue[i] == CMDLINE_NO) {
				m_HasValue[i] = CMDLINE_YES;
				m_Card++;
				return true;
			}
		}
	}
	initShowError();
	cerr << " illegal value '" << value << "'" << endl;
	return false;
}

void CmdLineArgSet::removeValue(int i) {
	if (m_HasValue[i] == CMDLINE_YES) {
		m_HasValue[i] = CMDLINE_NO;
		m_Card--;
	}
}

void CmdLineArgSet::addValue(int i) {
	if (m_HasValue[i] == CMDLINE_NO) {
		m_HasValue[i] = CMDLINE_YES;
		m_Card++;
	}
}

int CmdLineArgSet::getFirstValue() {
	for (vector<int>::size_type i = 0; i < m_Values.size(); i++) {
		if (m_HasValue[i] == CMDLINE_YES) {
			return i;
		}
	}
	return -1;
}

void CmdLineArgSet::write(ostream& os) {
	bool prev = false;
	for (vector<int>::size_type i = 0; i < m_Values.size(); i++) {
		if (m_HasValue[i] == CMDLINE_YES) {
			if (prev) { os << " "; } else { prev = true; }
			os << m_Values[i];
		}
	}
}

void CmdLineArgSet::reset() {
	for (vector<int>::size_type i = 0; i < m_Values.size(); i++) {
		if (m_HasValue[i] != CMDLINE_UNSUPPORTED) {
			m_HasValue[i] = CMDLINE_NO;
		}
	}
	m_Card = 0;
}

void CmdLineArgSet::setDefaultValue() {
	for (vector<int>::size_type i = 0; i < m_Defaults.size(); i++) {
		m_HasValue[m_Defaults[i]] = CMDLINE_YES;
		m_Card++;
	}
}

bool CmdLineArgSet::isDefault() {
	for (vector<int>::size_type i = 0; i < m_Values.size(); i++) {
		if (m_HasValue[i] != CMDLINE_UNSUPPORTED) {
			bool yes = m_HasValue[i] == CMDLINE_YES;
			bool default_yes = false;
			for (vector<int>::size_type j = 0; j < m_Defaults.size(); j++) {
				if (m_Defaults[j] == (int)i) {
					default_yes = true;
				}
			}
			if (yes != default_yes) return false;
		}
	}
	return true;
}

void CmdLineArgSet::showExtraHelp() {
	cerr << "   Possible values: ";
	for (vector<int>::size_type i = 0; i < m_Values.size(); i++) {
		if (m_HasValue[i] != CMDLINE_UNSUPPORTED) {
			if (i != 0) cerr << ", ";
			cerr << m_Values[i];
		}
	}
	cerr << endl;
}

void CmdLineArgSet::addPossibleValue(const char* value) {
	m_Values.push_back(string(value));
	m_HasValue.push_back(CMDLINE_NO);
}

bool CmdLineArgSet::hasOnlyValue(int id) {
	if (!hasValue(id)) return false;
	for (vector<int>::size_type i = 0; i < m_Values.size(); i++) {
		if (i != (unsigned int)id && m_HasValue[i] == CMDLINE_YES) {
			return false;
		}
	}
	return true;
}

vector<string> CmdLineArgSet::getValues() {
	vector<string> result;
	for (vector<int>::size_type i = 0; i < m_Values.size(); i++) {
		if (m_HasValue[i] == CMDLINE_YES) {
			result.push_back(m_Values[i]);
		}
	}
	return result;
}

CmdLineOption::CmdLineOption(const char* name) {
	addAlias(name);
	initialize();
}

CmdLineOption::CmdLineOption(const char* name, const char* alias) {
	addAlias(name); addAlias(alias);
	initialize();
}

CmdLineOption::CmdLineOption(const char* name, const char* alias1, const char* alias2) {
	addAlias(name); addAlias(alias1); addAlias(alias2);
	initialize();
}

CmdLineOption::~CmdLineOption() {
	deleteArgs();
}

void CmdLineOption::initialize() {
	m_HasOption = false;
	m_Expert = false;
	m_MinNbArgs = 0;
	m_NbArgs = 0;
}

void CmdLineOption::deleteArgs() {
	for (vector<CmdLineOptionArg*>::size_type i = 0; i < m_Args.size(); i++) {
		if (m_Args[i] != NULL) {
			delete m_Args[i];
			m_Args[i] = NULL;
		}
	}
}

void CmdLineOption::initOption() {
	for (vector<CmdLineOptionArg*>::size_type i = 0; i < m_Args.size(); i++) {
		if (m_Args[i] != NULL) {
			m_Args[i]->initArg();
		}
	}
}

void CmdLineOption::setDefaultValues() {
	for (vector<CmdLineOptionArg*>::size_type i = 0; i < m_Args.size(); i++) {
		if (m_Args[i] != NULL) {
			m_Args[i]->setDefaultValue();
		}
	}
}

bool CmdLineOption::allDefaults() {
	for (vector<CmdLineOptionArg*>::size_type i = 0; i < m_Args.size(); i++) {
		if (m_Args[i] != NULL) {
			if (!m_Args[i]->isDefault()) return false;
		}
	}
	return true;
}

void CmdLineOption::addArg(CmdLineOptionArg* arg) {
	m_Args.push_back(arg);
	arg->setOption(this);
	int nb = m_Args.size();
	if (m_MinNbArgs < nb) m_MinNbArgs = nb;
}

void CmdLineOption::addAlias(const char* alias) {
	m_Names.push_back(string(alias));
}

void CmdLineOption::showHelp() {
	cerr << "Option: " << getObject()->getOptionPrefix() << getName() << endl;
	if (getNbNames() > 1) {
		cerr << "Abbreviation(s): ";
		for (int i = 1; i < getNbNames(); i++) {
			if (i != 1) cerr << ", ";
			cerr << getObject()->getOptionPrefix() << getName(i);
		}
		cerr << endl;
	}
	cerr << getHelp() << endl;
	for (int i = 0; i < getMaxNbArgs(); i++) {
		CmdLineOptionArg* argi = getArg(i);
		cerr << "   Argument '" << argi->getName() << "': " << argi->getHelp() << endl;
		argi->showExtraHelp();
	}
}

CmdLineOptionList::CmdLineOptionList() {
	m_Error = 0;
}

CmdLineOptionList::~CmdLineOptionList() {
	deleteOptions();
}

void CmdLineOptionList::deleteOptions() {
	for (vector<CmdLineOption*>::size_type i = 0; i < m_Options.size(); i++) {
		if (m_Options[i] != NULL) {
			delete m_Options[i];
			m_Options[i] = NULL;
		}
	}
}

void CmdLineOptionList::initOptions() {
	for (vector<CmdLineOption*>::size_type i = 0; i < m_Options.size(); i++) {
		if (m_Options[i] != NULL) {
			m_Options[i]->initOption();
		}
	}
}

void CmdLineOptionList::setDefaultValues() {
	for (vector<CmdLineOption*>::size_type i = 0; i < m_Options.size(); i++) {
		CmdLineOption* opt = m_Options[i];
		if (opt != NULL && !opt->hasOption()) {
			opt->setDefaultValues();
		}
	}
}

void CmdLineOptionList::clearAll() {
	for (vector<CmdLineOption*>::size_type i = 0; i < m_Options.size(); i++) {
		CmdLineOption* opt = m_Options[i];
		if (opt != NULL) opt->setHasOption(false);
	}
	setDefaultValues();
}

bool CmdLineOptionList::allDefaults() {
	for (vector<CmdLineOption*>::size_type i = 0; i < m_Options.size(); i++) {
		if (m_Options[i] != NULL) {
			if (!m_Options[i]->allDefaults()) return false;
		}
	}
	return true;
}

CmdLineOption* CmdLineOptionList::getOption(const string& name) {
	for (vector<CmdLineOption*>::size_type i = 0; i < m_Options.size(); i++) {
		CmdLineOption* opt = m_Options[i];
		if (opt != NULL) {
			for (int j = 0; j < opt->getNbNames(); j++) {
				if (str_i_equals(opt->getName(j), name)) {
					return opt;
				}
			}
		}
	}
	return NULL;
}

void CmdLineOptionList::setHasOption(const string& name) {
	CmdLineOption* opt = getOption(name);
	if (opt != NULL) opt->setHasOption(true);
}

void CmdLineOptionList::setOptionString(const string& name, const string& value, int arg) {
	CmdLineOption* opt = getOption(name);
	if (opt != NULL) {
		opt->setHasOption(true);
		CmdLineOptionArg* carg = opt->getArg(arg);
		carg->addValue(value);
	}
}

CmdLineOption* CmdLineOptionList::createOption(int id) {
	CmdLineOption* opt = getOption(id);
	if (opt != NULL) opt->setHasOption(true);
	return opt;
}

void CmdLineOptionList::showHelp(int helpid) {
	bool expert = false;
	CmdLineOption* help = getOption(helpid);
	CmdLineArgString* item = (CmdLineArgString*)help->getArg(0);
	if (item->getCard() == 1) {
		const string& value = item->getValue();
		if (value == "expert") {
			expert = true;
		} else {
			CmdLineOption* opt = getOption(value);
			if (opt == NULL) {
				cerr << ">> Unknown option '" << getOptionPrefix() << value << "'" << endl;
			} else {
				cerr << endl;
				opt->showHelp();
			}
			return;
		}
	}
	cerr << endl << "Options:" << endl;
	for (vector<CmdLineOption*>::size_type i = 0; i < m_Options.size(); i++) {
		CmdLineOption* opt = m_Options[i];
		if (opt != NULL && (!opt->isExpert() || expert)) {
			string str = " ";
			str += getOptionPrefix();
			str += opt->getName();
			cerr << str;
			for (int i = str.length(); i < 17; i++) cerr << ' ';
			cerr << opt->getHelp() << endl;
		}
	}
	if (!expert) cerr << endl << "Show expert options: " << getOptionPrefix() << "help expert" << endl;
}

bool CmdLineOptionList::hasOption(const string& name) {
	CmdLineOption* option = getOption(name);
	return option != 0 && option->hasOption();
}

bool CmdLineOptionList::hasOption(int id) {
	if (id >= (int)m_Options.size()) return false;
	if (m_Options[id] == NULL) return false;
	return m_Options[id]->hasOption();
}

void CmdLineOptionList::setHasOption(int id, bool set) {
	return m_Options[id]->setHasOption(set);
}

char CmdLineOptionList::getOptionPrefix() {
#ifdef __WIN32__
	return '/';
#else
	return '-';
#endif
}

void CmdLineOptionList::addOption(CmdLineOption* option, int id) {
	int size = m_Options.size();
	if (id >= size) {
		m_Options.reserve(id+1);
		for (int i = size; i <= id; i++) m_Options.push_back(NULL);
	}
	option->setObject(this);
	m_Options[id] = option;
}

CmdLineArgString* CmdLineOptionList::addStringOption(const char* name, int id) {
	CmdLineOption* opt = new CmdLineOption(name);
	CmdLineArgString* arg = new CmdLineArgString("");
	opt->addArg(arg);
	addOption(opt, id);
	return arg;
}

CmdLineArgSPairList* CmdLineOptionList::addSPairListOption(const char* name, int id) {
	CmdLineOption* opt = new CmdLineOption(name);
	CmdLineArgSPairList* arg = new CmdLineArgSPairList(name);
	opt->addArg(arg);
	addOption(opt, id);
	return arg;
}

ConfigSection::ConfigSection(const char* name) {
	m_Name = name;
}

ConfigSection::~ConfigSection() {
}

ConfigCollection::ConfigCollection() {
}

ConfigCollection::~ConfigCollection() {
	deleteSections();
}

void ConfigCollection::deleteSections() {
	for (vector<ConfigSection*>::size_type i = 0; i < m_Sections.size(); i++) {
		if (m_Sections[i] != NULL) {
			delete m_Sections[i];
			m_Sections[i] = NULL;
		}
	}
}

void ConfigCollection::setDefaultValues() {
	for (vector<ConfigSection*>::size_type i = 0; i < m_Sections.size(); i++) {
		if (m_Sections[i] != NULL) {
			m_Sections[i]->setDefaultValues();
		}
	}
}

bool ConfigCollection::allDefaults() {
	for (vector<ConfigSection*>::size_type i = 0; i < m_Sections.size(); i++) {
		if (m_Sections[i] != NULL) {
			if (!m_Sections[i]->allDefaults()) return false;
		}
	}
	return true;
}

void ConfigCollection::addSection(ConfigSection* section, int id) {
	int size = m_Sections.size();
	if (id >= size) {
		m_Sections.reserve(id+1);
		for (int i = size; i <= id; i++) m_Sections.push_back(NULL);
	}
	m_Sections[id] = section;
}

ConfigSection* ConfigCollection::getSection(const string& name) {
	for (vector<ConfigSection*>::size_type i = 0; i < m_Sections.size(); i++) {
		ConfigSection* sec = m_Sections[i];
		if (sec != NULL && str_i_equals(sec->getName(), name)) {
			return sec;
		}
	}
	return NULL;
}

void ConfigCollection::setStringValue(int section, int value, const char* str) {
	ConfigSection* sec = getSection(section);
	((CmdLineArgString*)sec->getOptionValue(value))->setValue(str);
}

const string& ConfigCollection::getStringValue(int section, int value) {
	ConfigSection* sec = getSection(section);
	return ((CmdLineArgString*)sec->getOptionValue(value))->getValue();
}

CmdLineObj::CmdLineObj() {
	m_ArgC = 0;
	m_ArgIdx = 0;
	m_ArgV = NULL;
	m_MArgSepPos = -1;
	m_HasStdin = false;
}

CmdLineObj::~CmdLineObj() {
}

const char* CmdLineObj::getNextArg() {
	return m_ArgIdx >= m_ArgC ? NULL : m_ArgV[m_ArgIdx++];
}

bool CmdLineObj::parseOptionArg(bool inmainargs, const string& optionname, int argidx, CmdLineOption** cropt_p) {
	if (inmainargs) {
		cerr << ">> Options should come before " << m_MainArgType << " arguments" << endl;
		m_Error = 1;
		return false;
	}
	CmdLineOption* cropt = *cropt_p;
	if (cropt != NULL) {
		if (argidx < cropt->getMinNbArgs()) {
			cerr << ">> Option '" << cropt->getName() << "' requires "
			     << cropt->getMinNbArgs() << " arguments" << endl;
			m_Error = 1;
			return false;
		}
		// Set default values for all arguments not given
		for (int i = argidx; i < cropt->getMaxNbArgs(); i++) {
			cropt->getArg(i)->setDefaultValue();
		}
	}
	cropt = *cropt_p = getOption(optionname);
	if (cropt == NULL) {
		cerr << ">> Unknown option '" << getOptionPrefix() << optionname << "'" << endl;
		m_Error = 1;
		return false;
	} else {
		cropt->setHasOption(true);
	}
	return true;
}

bool CmdLineObj::isMainArgSeparator(const string& arg) {
	for (vector<string>::size_type i = 0; i < m_MArgSep.size(); i++) {
		// cout << "Sep = " << m_MArgSep[i] << " Arg = " << arg << endl;
		if (str_i_equals(m_MArgSep[i], arg)) {
			return true;
		}
	}
	return false;
}

int CmdLineObj::getIntValue(int option, int arg) {
	CmdLineArgInt* intval = (CmdLineArgInt*)getOption(option)->getArg(arg);
	return intval->getValue();
}

void CmdLineObj::setIntValue(int option, int value, int arg) {
	CmdLineArgInt* intval = (CmdLineArgInt*)getOption(option)->getArg(arg);
	intval->setValue(value);
}

const string& CmdLineObj::getStringValue(int option, int arg) {
	CmdLineArgString* strval = (CmdLineArgString*)getOption(option)->getArg(arg);
	return strval->getValue();
}

int CmdLineObj::getNbMainArgs() {
	return m_MArgSepPos == -1 ? m_MainArgs.size() : m_MArgSepPos;
}

const string& CmdLineObj::getMainArg(int i) {
	return m_MainArgs[i];
}

int CmdLineObj::getNbExtraArgs() {
	return m_MArgSepPos == -1 ? 0 : m_MainArgs.size() - m_MArgSepPos;
}

const string& CmdLineObj::getExtraArg(int i) {
	return m_MainArgs[i + m_MArgSepPos];
}

bool CmdLineObj::checkForStdin() {
	for (int i = 0; i < getNbMainArgs(); i++) {
		const string& arg = getMainArg(i);
		if (arg == "-") {
			m_HasStdin = true;
			m_MainArgs.erase(m_MainArgs.begin()+i);
			if (i < getNbMainArgs()) {
				/* after stdin marker, assume only extra args */
				setMainArgSepPos(i);
			}
			return true;
		}
	}
	return false;
}

void CmdLineObj::parse(int argc, char** argv) {
	const char* crarg;
	m_ArgC = argc;
	m_ArgV = argv;
	m_ArgIdx = 1;
	int argidx = 0;
	bool inmainargs = false;
	CmdLineOption* cropt = NULL;
	while ((crarg = getNextArg()) != NULL) {
		int len = strlen(crarg);
#ifdef __WIN32__
		/* Also allow options with prefix '/' on Windows */
		/* On Unix, '/' might be the start of an absolute path !! */
		if (len >= 2 && (crarg[0] == '-' || crarg[0] == '/')) {
#else
		if (len >= 2 && crarg[0] == '-') {
#endif
			string optionname;
			if (crarg[1] == '-') {
				optionname = crarg+2;
			} else {
				optionname = crarg+1;
			}
			if (inmainargs && isMainArgSeparator(optionname)) {
				if (getMainArgSepPos() != -1) {
					cerr << ">> Only one extra argument separator allowed" << endl;
					m_Error = 1;
					return;
				}
				setMainArgSepPos(getNbMainArgs());
			} else {
				if (parseOptionArg(inmainargs, optionname, argidx, &cropt)) {
					argidx = 0;
				} else {
					return;
				}
			}
		} else {
			if (cropt != NULL && argidx < cropt->getMaxNbArgs()) {
				/* Assume extra argument for current option */
				addOptionArg(cropt, argidx, string(crarg));
				if (hasError()) return;
				argidx++;
			} else {
				/* Assume main argument */
				inmainargs = true;
				m_MainArgs.push_back(string(crarg));
			}
		}
	}
	setDefaultValues();
}

void CmdLineObj::addOptionArg(CmdLineOption* cropt, int argidx, const string& crarg) {
	CmdLineOptionArg* arg = cropt->getArg(argidx);
	if (arg->needsComma()) {
		if (arg->getMaxCard() == -1 || arg->getCard() < arg->getMaxCard()) {
			if (!arg->addValue(crarg)) {
				m_Error = 1;
			}
		}
		return;
	}
	char_separator separator(",", "", drop_empty_tokens);
	tokenizer<char_separator> tokens(crarg, separator);
	while (tokens.has_more()) {
		if (arg->getMaxCard() == -1 || arg->getCard() < arg->getMaxCard()) {
			if (!arg->addValue(tokens.next_token())) {
				m_Error = 1;
			}
		} else {
			cerr << ">> Option '" << getOptionPrefix() << cropt->getName() << "'";
			if (cropt->getMaxNbArgs() > 1) {
				cerr << " argument " << argidx << " (" << arg->getName() << ")";
			}
			cerr << " takes at most " << arg->getMaxCard() << " value(s)" << endl;
			m_Error = 1;
			return;
		}
	}
}

bool cmdline_is_option(const char* opt, const char* val) {
	bool is_option = false;
	if (opt != NULL && opt[0] == '-') is_option = true;
#ifdef __WIN32__
	if (opt != NULL && opt[0] == '/') is_option = true;
#endif
	return is_option && str_i_equals(opt+1, val);
}
