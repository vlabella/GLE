
#include "all.h"
#include "gle-interface/gle-base.h"
#include "gle-interface/gle-interface.h"
#include "gle-interface/gle-datatype.h"
#include "var.h"
#include "sub.h"
#include "builtin-double.h"

GLEBuiltInBinaryDoubleDouble::GLEBuiltInBinaryDoubleDouble(const char* name, GLEBuiltInFactory* factory) {
	setRoot(factory->createRoot(name, factory->getBinaryArgNamesXY()));
	setArgTypeDefaults(factory->getBinaryDoubleDoubleArgTypeDefaults());
}

GLEBuiltInOpPlusDouble::GLEBuiltInOpPlusDouble(GLEBuiltInFactory* factory) : GLEBuiltInBinaryDoubleDouble("+", factory) {}

void GLEBuiltInOpPlusDouble::execute(GLEArrayImpl* stack, unsigned int top) {
}

void initializeBuiltInDouble(GLESubMap* submap, GLEBuiltInFactory* factory) {



	submap->add(new GLEBuiltInOpPlusDouble(factory)); /* +/2 operator */
}
