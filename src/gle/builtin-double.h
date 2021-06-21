
#ifndef BUILTINDOUBLE_H_
#define BUILTINDOUBLE_H_

class GLEBuiltInBinaryDoubleDouble : public GLEBuiltIn {
public:
	GLEBuiltInBinaryDoubleDouble(const char* name, GLEBuiltInFactory* factory);
};

class GLEBuiltInOpPlusDouble : public GLEBuiltInBinaryDoubleDouble {
public:
	GLEBuiltInOpPlusDouble(GLEBuiltInFactory* factory);
	virtual void execute(GLEArrayImpl* stack, unsigned int top);
};




#endif
