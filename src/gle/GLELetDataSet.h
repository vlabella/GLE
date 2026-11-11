#ifndef GLE_LET_DATASET_H
#define GLE_LET_DATASET_H

class GLELetDataSet {

private:

	typedef struct {
		double x, y[2];
	} DataSetVal;

	static bool DataSetValCMP(const DataSetVal& e1, const DataSetVal& e2) {
   		return e1.x < e2.x;
	}

protected:

	int m_DN;
	int m_Var;
	int m_CrP;
	bool m_IsFunction;
	bool m_IsXRangeDS;
	std::vector<DataSetVal> m_Vals;
	std::vector<double> m_MissX;

public:

   static constexpr int GLE_DS_L = 0;
   static constexpr int GLE_DS_R = 1;

	GLELetDataSet();
	~GLELetDataSet();
	void initializeFrom(int dn, int var);
	void complainNoFunction();
	bool interpolateTo(double x, int lr);
	inline int getNbValues() { return m_Vals.size(); }
	inline double getXValue(int i) { return m_Vals[i].x; }
	inline int getNbMissing() { return m_MissX.size(); }
	inline double getMissing(int i) { return m_MissX[i]; }
	inline bool isFunction() { return m_IsFunction; }
	inline int getDatasetID() { return m_DN; }
	inline int getVar() { return m_Var; }
	inline void setIsXRangeDS(bool value) { m_IsXRangeDS = value; }
	inline bool isXRangeDS() { return m_IsXRangeDS; }
};

#endif