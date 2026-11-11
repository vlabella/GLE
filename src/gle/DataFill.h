#ifndef DATAFILL_H
#define DATAFILL_H

class DataFillDimension {
protected:
	bool m_Log;
	GLERange m_Range;
	double m_Value;
	GLEFunctionParserPcode* m_Fct;
	RefCountPtr<GLEDoubleArray> m_Values;
public:
	DataFillDimension(GLEFunctionParserPcode* fct);
	~DataFillDimension();
	void setRange(GLERange* range, bool log);
	bool isYValid();
	inline bool isLog() { return m_Log; }
	inline GLERange* getRange() { return &m_Range; }
	inline void setDoubleAt(double v, int i) { m_Values->setDoubleAt(v, i); }
	inline GLEDoubleArray* getValues() { return m_Values.get(); }
	inline double getValue() { return m_Value; }
	inline void computeValue() { m_Value = m_Fct->evalDouble(); }
	inline bool isYNan() { return gle_isnan(m_Value); }
};

class DataFill {
protected:
	int m_Size;
	int m_VarX;
	bool m_PrevInvalid;
	bool m_PrevPoint;
	bool m_FineTune;
	bool m_CrEnable;
	double m_PrevXValue;
	bool m_detectDiscontinuity;
	double m_discontinuityThreshold;
	double m_tuneDistance;
	int m_tuneIterationsMin;
	int m_tuneIterationsMax;
	std::set<double> m_MissX;
	GLEVectorAutoDelete<GLELetDataSet>* m_DataSets;
	GLEVectorAutoDelete<DataFillDimension> m_Dim;
	RefCountPtr<GLEBoolArray> m_Missing;
	GLEFunctionParserPcode* m_Where;
public:
	DataFill(bool finetune);
	~DataFill();
	void addPointFineTune(double x, int lr);
	void addPointIPol(double x);
	void addPoint();
	void addPointLR(double x, int lr);
	void addPoint(double x);
	void addPoint(double x, double y);
	void addMissing();
	void addMissingLR(double x, int lr);
	bool isYValid();
	bool isYNotNan();
	bool isRangeValid();
	bool selectXValue(double x, int lr);
	void selectXValueNoIPol(double x);
	double maxDistanceTo(double x);
	void minMaxDistanceTo(double x, GLERange* distanceRange);
	void tryIPol(double other, double nanval);
	void checkDiscontinuity(double xPrev, double xNext, int lr);
	void tryAddMissing(double x, int lr);
	void setInfo(GLEFunctionParserPcode* expr, int varx);
	void setYMinYMax(double ymin, double ymax);
	void setDetectDiscontinuity(bool detect, double threshold);
	void toDataset(GLEDataSet* dataset);
	inline int size() { return m_Size; }
	inline double* getX() { return m_Dim[0]->getValues()->toArray(); }
	inline double* getY() { return m_Dim[1]->getValues()->toArray(); }
	inline int* getM() { return m_Missing->toArray(); }
	inline void addDataDimension(DataFillDimension* fill) { m_Dim.push_back(fill); }
	inline void setDataSets(GLEVectorAutoDelete<GLELetDataSet>* sets) { m_DataSets = sets; }
	inline void setVarX(int varx) { m_VarX = varx; }
	inline bool isFineTune() { return m_FineTune; }
	inline void setMissingValue(double x) { m_MissX.insert(x); }
	inline void setWhere(GLEFunctionParserPcode* where) { m_Where = where; }
};

#endif