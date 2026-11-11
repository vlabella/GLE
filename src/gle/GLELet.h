#ifndef GLE_LET_H
#define GLE_LET_H

#include "GLELetDataSet.h"
#include "DataFill.h"
#include "GLECheckWindow.h"

class GLELet {
protected:
	GLERC<GLEVarSubMap> m_SubMap;
	GLERCVector<GLEFunctionParserPcode> m_Fcts;
	GLERC<GLEFunctionParserPcode> m_Where;
	GLEVarBackup m_varBackup;
	std::set<int> m_XRangeDS;
	double m_LetFrom, m_LetTo, m_LetStep, m_LogStep;
	bool m_FineTune, m_NoFirst, m_HasSteps, m_HasStepOption, m_HasFrom, m_HasTo;
	int m_VarX, m_Ds, m_LetNSteps;
	int m_codeLine;

	// For histogram routine
	int m_nrBins;
	int m_HistDS;

	// For fitting routines
	int m_FitDS;
	bool m_limitDataX;
	bool m_limitDataY;
	bool m_limitData;
	std::string m_fitType;
	std::string m_fitFct;
	std::string m_eqStr;
	std::string m_format;
	std::string m_rsq;
	std::string m_varSlope;
	std::string m_varIntercept;
	std::string m_varR;
	GLECheckWindow m_window;
	bool m_Materialize;

public:
	GLELet();
	~GLELet();
	void initVars();
	void initStep();
	void doLet();
	void parseFitFunction(const std::string& fct, GLEParser* parser);
	void parseHistogram(GLEParser* parser);
	void doFitFunction();
	void doHistogram();
	void complainAboutNoFunctions(GLEVectorAutoDelete<GLELetDataSet>& datasets);
	bool checkIdenticalRanges(GLEVectorAutoDelete<GLELetDataSet>& datasets);
	void transformIdenticalRangeDatasets(GLEVectorAutoDelete<GLELetDataSet>& datasets, DataFill* fill);
	void combineFunctions(GLEVectorAutoDelete<GLELetDataSet>& datasets, DataFill* fill, double logstep);
	void setStep(double value);
	void createVarBackup(GLEVars* vars, StringIntHash* varHash);
	void restoreVarBackup(GLEVars* vars);
	GLEFunctionParserPcode* insertFunction();
	GLEFunctionParserPcode* addFunction();
	GLEFunctionParserPcode* addWhere();
	inline void setVarSubMap(GLEVarSubMap* map) { m_SubMap = map; }
	inline GLEVarSubMap* getVarSubMap() { return m_SubMap.get(); }
	inline void setFrom(double value) { m_LetFrom = value; }
	inline void setTo(double value) { m_LetTo = value; }
	inline void setNSteps(int value) { m_LetNSteps = value; }
	inline void setFineTune(bool value) { m_FineTune = value; }
	inline void setNoFirst(bool value) { m_NoFirst = value; }
	inline double getFrom() { return m_LetFrom; }
	inline double getTo() { return m_LetTo; }
	inline double getStep() { return m_LetStep; }
	inline bool hasSteps() { return m_HasSteps; }
	inline bool hasFrom() { return m_HasFrom; }
	inline bool hasTo() { return m_HasTo; }
	inline void setHasStepOption(bool has) { m_HasStepOption = has; }
	inline void setHasSteps(bool has) { m_HasSteps = has; }
	inline void setHasFrom(bool has) { m_HasFrom = has; }
	inline void setHasTo(bool has) { m_HasTo = has; }
	inline void setDataSet(int ds) { m_Ds = ds; }
	inline int getDataSet() { return m_Ds; }
	inline int getDimension() { return m_Fcts.size(); }
	inline void addFunction(GLEFunctionParserPcode* fct) { m_Fcts.add(fct); }
	inline void addXRangeDS(int ds) { m_XRangeDS.insert(ds); }
	inline std::set<int>& getXRangeDS() { return m_XRangeDS; }
	inline bool isHistogram() const  { return m_HistDS != -1; }
	inline bool isFit() const  { return m_FitDS != -1; }
	inline void setCodeLine(int line) { m_codeLine = line; }
	inline int getCodeLine() const { return m_codeLine; }
	inline void setMaterialize(bool value) { m_Materialize = value;}
	inline bool shouldMaterialize() const {return m_Materialize;}
};

inline bool hasDataset(int di) {
	return di > 0 && di <= ndata && dp[di] != NULL && !dp[di]->undefined();
}

inline GLEDataSet* getDataset(int di, const char* descr = 0) {
	if (hasDataset(di)) {
		return dp[di];
	} else {
		std::ostringstream err;
		if (descr != 0) {
			err << descr << " ";
		}
		err << "dataset d" << di << " not defined";
		g_throw_parser_error(err.str());
		return 0;
	}
}

#endif