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

#include "../all.h"
#include "../core.h"
#include "../file_io.h"
#include "../texinterface.h"
#include "../cutils.h"
#include "../gprint.h"
#include "../tokens/RefCount.h"
#include "../glearray.h"
#include "../polish.h"
#include "../var.h"
#include "../keyword.h"
#include "../run.h"

#include <memory>

#define unit_test(a) unit_test_impl(a, #a, __FILE__, __LINE__)
#define unit_test_msg(a, b) unit_test_impl(a, b, __FILE__, __LINE__)

void unit_test_impl(bool expr, const char* exprStr, const char* file, int line) {
	if (!expr) {
		cout << "unit test failed (" << file << ":" << line << "): " << exprStr << std::endl;
		exit(1);
	}
}

void unit_test_impl(bool expr, const std::string& exprStr, const char* file, int line) {
	unit_test_impl(expr, exprStr.c_str(), file, line);
}

void test_csv_reader1() {
	const unsigned int columns[] = { 3, 2, 3, 3, 3, 3, 3, 3, 3 };
	const char* tokens[] = {
			"\"0.123\"", "5.5", "23",
            "België", "25",
            "\"Tussen \"dubbele quotes\"\"", "36", "40",
            "Tussen 'enkele quotes'", "20", "10",
            "\\textbf{hello}", "1", "23.5",
            "\"Comma: ,\"", "你好", "10",
            "\"Drie dubbele quotes \"\"\"\"", "1", "23",
            "\"\"\"", "", "32",
            "A", "B", ""
	};
	const char* input1 = "REM Line with comments\n"
	                     " REM Line with comments\n"
	                     "\"0.123\",5.5    ,23\n"
	                     "België,25          REM Hello\n"
	                     "\"Tussen \"\"dubbele quotes\"\"\",36,40\r\n"
	                     "Tussen 'enkele quotes',20,10\n"
	                     "\\textbf{hello},1,23.5\n"
	                     "\"Comma: ,\",你好, 10\n\r"
	                     "\"Drie dubbele quotes \"\"\"\"\"\"\",1,23\n"
	                     "\"\"\"\",,32\n"
	                     "A,B,";
	GLECSVData reader;
	reader.setDelims(",;\t");
	reader.setCommentIndicator("REM");
	reader.readBuffer(input1);
	GLECSVError* error = reader.getError();
	unit_test(error->errorCode == GLECSVErrorNone);
	unit_test(reader.getNbLines() == 9);
	for (unsigned int i = 0; i < reader.getNbLines(); i++) {
		unit_test(reader.getNbColumns(i) == columns[i]);
	}
	unsigned int pos = 0;
	for (unsigned int i = 0; i < reader.getNbLines(); i++) {
		for (unsigned int j = 0; j < reader.getNbColumns(i); j++) {
			string cellValue = reader.getCellString(i, j);
			unit_test(cellValue == tokens[pos++]);
		}
	}
}

void test_csv_reader2() {
	unsigned int columns = 12;
	const char* tokens[] = {
		"17", "16", "2", "16", "16", "1", "17", "161947.9820", "161947.9381", "0.0439", "*", "*",
		"100", "16", "85", "99", "16", "84", "100", "946121.8397", "946046.8191", "75.0206", "*", "23",
		"101", "16", "86", "100", "16", "85", "101", "955521.9934", "955371.2682", "150.7252", "*", "*",
		"119", "16", "104", "118", "16", "103", "119", "1122496.4662", "1122497.7256", "-1.2594", "*", "*" };
	const char* input1 =
			"!\n"
	        "! Some comments\n"
	        "!\n"
	        " 17  16   2   16  16   1    17    161947.9820  161947.9381       0.0439          *             * \n"
	        "100  16  85   99  16  84   100    946121.8397  946046.8191      75.0206          *             23\n"
	        "101  16  86  100  16  85   101    955521.9934  955371.2682     150.7252          *             * !\n"
	        "119  16 104  118  16 103   119   1122496.4662 1122497.7256      -1.2594          *             * !\n"
	        "!120 16 105  119  16 104   120   1131738.8987 1131740.7524      -1.8537          *             * \n";
	GLECSVData reader;
	reader.readBuffer(input1);
	GLECSVError* error = reader.getError();
	unit_test(error->errorCode == GLECSVErrorNone);
	unit_test(reader.getNbLines() == 4);
	for (unsigned int i = 0; i < reader.getNbLines(); i++) {
		unit_test(reader.getNbColumns(i) == columns);
	}
	unsigned int pos = 0;
	for (unsigned int i = 0; i < reader.getNbLines(); i++) {
		for (unsigned int j = 0; j < columns; j++) {
			string cellValue = reader.getCellString(i, j);
			unit_test(cellValue == tokens[pos++]);
		}
	}
}

void test_csv_reader3() {
	unsigned int columns = 7;
	const char* tokens[] = {
		"Sample_Number", "V_Ne", "V_Ar", "V_Kr", "V_Xe", "NGT", "err_NGT",
		"MI-1", "3.68", "4.72", "10.7", "1.59", "6.78", "0.89",
		"MI-2a", "3.76", "5.07", "*", "1.71", "*", "0.82" };
	const char* input1 =
		" Sample_Number , V_Ne , V_Ar , V_Kr , V_Xe , NGT , err_NGT \n"
		" MI-1 ,3.68,4.72,10.7,1.59,6.78,0.89\n"
		" MI-2a ,3.76,5.07, * ,1.71, *,0.82 \n"
        " \n";
	GLECSVData reader;
	reader.readBuffer(input1);
	GLECSVError* error = reader.getError();
	unit_test(error->errorCode == GLECSVErrorNone);
	unit_test(reader.getNbLines() == 3);
	for (unsigned int i = 0; i < reader.getNbLines(); i++) {
		unit_test(reader.getNbColumns(i) == columns);
	}
	unsigned int pos = 0;
	for (unsigned int i = 0; i < reader.getNbLines(); i++) {
		for (unsigned int j = 0; j < columns; j++) {
			string cellValue = reader.getCellString(i, j);
			unit_test(cellValue == tokens[pos++]);
		}
	}
}

void test_expression_evaluator_each(GLEPolish* polish, const std::string& expression, const std::string& expectedValue) {
	int cp = 0;
	int rtype = 0;
	GLEPcodeList pc_list;
	GLEPcode pcode(&pc_list);
	polish->polish(expression.c_str(), pcode, &rtype);
	GLERC<GLEArrayImpl> stk(new GLEArrayImpl());
	std::ostringstream msg;
	msg << expression << ": ";
	if (is_float(expectedValue)) {
		GLEMemoryCell* mc = evalGeneric(stk.get(), &pc_list, (int*)&pcode[0], &cp);
		gle_memory_cell_check(mc, GLEObjectTypeDouble);
		double expectedDouble = tokenizer_string_to_double(expectedValue.c_str());
		msg << mc->Entry.DoubleVal << " == " << expectedValue;
		if (expectedDouble == 0.0) {
			unit_test_msg(fabs(mc->Entry.DoubleVal) < CUTILS_REL_PREC_FINE, msg.str());
		} else {
			unit_test_msg(equals_rel_fine(mc->Entry.DoubleVal, expectedDouble), msg.str());
		}
	} else {
		GLERC<GLEString> result(evalString(stk.get(), &pc_list, (int*)&pcode[0], &cp, true));
		std::string computedString(result->toUTF8());
		msg << computedString << " == " << expectedValue;
		unit_test_msg(expectedValue == computedString, msg.str());
	}
}

void test_expression_evaluator() {
	GLECSVData reader;
	reader.setDelims(",");
	reader.read("unit-tests/expressions.txt");
	GLECSVError* error = reader.getError();
	unit_test(error->errorCode == GLECSVErrorNone);
	std::auto_ptr<GLEPolish> polish(new GLEPolish());
	polish->initTokenizer();
	for (unsigned int i = 0; i < reader.getNbLines(); i++) {
		unit_test(reader.getNbColumns(i) == 2);
		string expression(reader.getCellString(i, 0));
		string expectedValue(reader.getCellString(i, 1));
		str_remove_quote(expression);
		str_remove_quote(expectedValue);
		test_expression_evaluator_each(polish.get(), expression, expectedValue);
	}
}

int main(void) {
	try {
		test_csv_reader1();
		test_csv_reader2();
		test_csv_reader3();
		test_expression_evaluator();
	} catch (ParserError& err) {
		std::string errMsg;
		err.toString(errMsg);
		unit_test_msg(false, errMsg.c_str());
	}
	return 0;
}
