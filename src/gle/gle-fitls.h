//
// -- GLE fitting routine based on Powell Minimization - See Numerical Recipes Chapter 10
//
#ifndef INCLUDE_GLE_FITLS
#define INCLUDE_GLE_FITLS

class GLEPowellFunc {
public:
	GLEPowellFunc();
	virtual ~GLEPowellFunc();
	virtual double fitMSE(double* vals) = 0;
};

class GLEFitLS : public GLEPowellFunc {
protected:
	int m_IdxX, m_NIter;
	double m_RSquare;
	std::vector<int> m_Vars;
	std::vector<double>* m_X;
	std::vector<double>* m_Y;
	StringIntHash m_VarMap;
	std::string m_FunctionStr;
	GLERC<GLEFunctionParserPcode> m_Function;
public:
	GLEFitLS();
	virtual ~GLEFitLS();
	void polish(const std::string& str);
	void setXY(std::vector<double>* x, std::vector<double>* y);
	void fit();
	void testFit();
	void setVarsVals(double* vals);
	void toFunctionStr(const std::string& format, std::string* str);
	virtual double fitMSE(double* vals);
	inline GLEFunctionParserPcode* getFunction() { return m_Function.get(); }
	inline double getRSquare() { return m_RSquare; }
};
#endif

