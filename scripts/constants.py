#
# -- constants.py - generates cpp and tex code for pysical and mathematical constants for GLE
#    uses scipy.constants for physical constants and boost::math::constants for numerical constants
#
import scipy as sp
import subprocess
import os

pc_wart      = "pc_"
mc_wart      = "mc_"
cpp_filename = "gle-constants.h"
tex_filename = "constants.tex"
gle_filename = "test_constants.gle"


def cpp_string(value):
	"""Escape a string for use inside a C++ string literal."""
	return (
		value
		.replace("\\", "\\\\")
		.replace('"', '\\"')
		.replace("\n", "\\n")
		.replace("\r", "\\r")
		.replace("\t", "\\t")
	)


def cpp_double(value):
	"""Format a Python float as a round-trip-safe C++ double literal."""
	text = format(float(value), ".17g")
	if "e" not in text.lower() and "." not in text:
		text += ".0"
	return text


# variable names in scipy.constants.
# These are the curated physical constants directly available as PC_* variables.
scipy_physical_constants = [
	"c",
	"speed_of_light",
	"mu_0",
	"epsilon_0",
	"h",
	"Planck",
	"hbar",
	#"G",  GLE is case insensitive so this clashes with little g - use graviational_constant instead
	"gravitational_constant",
	"g",
	"e",
	"elementary_charge",
	"R",
	"gas_constant",
	"alpha",
	"fine_structure",
	"N_A",
	"Avogadro",
	"k",
	"Boltzmann",
	"sigma",
	"Stefan_Boltzmann",
	"Wien",
	"Rydberg",
	"m_e",
	"electron_mass",
	"m_p",
	"proton_mass",
	"m_n",
	"neutron_mass",
	"zero_Celsius"
]

boost_math_constants = [
	"catalan",
	"degree",
	"e",
	"e_pow_pi",
	"euler",
	"exp_minus_half",
	"half",
	"half_pi",
	"half_root_two",
	"ln_phi",
	"one_div_two_pi",
	"half_root_two",
	"phi",
	"pi",
	"quarter_pi",
	"radian",
	"root_e",
	"root_half_pi",
	"root_ln_four",
	"root_pi",
	"root_three",
	"root_two",
	"root_two_pi",
	"third",
	"third_pi",
	"two_pi",
	"two_thirds",
	"zeta_three",
	"zeta_two"
]

#
# -- generate .cpp file
#
file = []
file.append('//')
file.append('// -- gle-constants.h')
file.append('//')
file.append('//  automatically generated from scripts/constants.py')
file.append('//  re-run manually and copy file to src/gle to modify')
file.append('//  uses boost::math for numerical constants')
file.append('//  uses scipy.constants for physical constants')
file.append('//  includes the complete scipy CODATA physical constants database')
file.append(f'//  mathematical constants start with {mc_wart}')
file.append(f'//  physical constants start with {pc_wart}')
file.append('//')
file.append('//  gle variables are case in-sensitive')
file.append('//')
file.append('#include <cstddef>')
file.append('#include <boost/math/constants/constants.hpp>')
file.append('//')

#
# -- complete CODATA physical constants database
#
# scipy.constants.physical_constants is used only when this script is run.
# The generated GLE code therefore has no runtime dependency on scipy.
#
file.append('struct GLECODATAConstant {')
file.append('\tconst char* name;')
file.append('\tdouble value;')
file.append('\tconst char* units;')
file.append('\tdouble uncertainty;')
file.append('};')
file.append('//')

file.append('static const GLECODATAConstant gle_codata_constants[] = {')

for name in sorted(sp.constants.physical_constants, key=str.casefold):
	value, units, uncertainty = sp.constants.physical_constants[name]

	file.append(
		'\t{"%s", %s, "%s", %s},'
		% (
			cpp_string(name),
			cpp_double(value),
			cpp_string(units),
			cpp_double(uncertainty)
		)
	)

file.append('};')
file.append('//')

file.append(
	'static const std::size_t gle_codata_constants_count = '
	'sizeof(gle_codata_constants) / sizeof(gle_codata_constants[0]);'
)
file.append('//')

#
# -- set the standard mathematical and physical constants
#
file.append('static void gle_set_math_and_physical_constants(bool numerical=true,bool physical=true)')
file.append("{")
tb="\t"
file.append(tb+'// option to omit inclusion of types if desired -- possible future GLE option')
file.append(tb+"if(numerical){")
tb="\t\t"

for c in boost_math_constants:
	#print(f"{c} {eval(f"sp.constants.{c}")}")
	s = f"{mc_wart}{c}"
	file.append(f"{tb}var_findadd_set(\"{s.upper()}\",boost::math::double_constants::{c});")

tb="\t"
file.append(tb+"}")
file.append(tb+"if(physical){")
tb="\t\t"

for c in scipy_physical_constants:
	#print(f"{c} {eval(f"sp.constants.{c}")}")
	s = f"{pc_wart}{c}"
	file.append(f"{tb}var_findadd_set(\"{s.upper()}\",{eval(f'sp.constants.{c}')});")

tb="\t"
file.append(tb+"}")
file.append("}")

#
# -- generate CODATA lookup functions
#
file.append('')
file.append('//')
file.append('// -- CODATA lookup')
file.append('//')
file.append('static const GLECODATAConstant* gle_codata_find(const std::string& name)')
file.append('{')
file.append('\tfor (std::size_t i = 0; i < gle_codata_constants_count; ++i) {')
file.append('\t\tif (str_i_equals(gle_codata_constants[i].name, name)) {')
file.append('\t\t\treturn &gle_codata_constants[i];')
file.append('\t\t}')
file.append('\t}')
file.append('')
file.append('\treturn nullptr;')
file.append('}')
file.append('')
#
# -- codata()
#
file.append('static double gle_codata(const std::string& name)')
file.append('{')
file.append('\tconst GLECODATAConstant* constant = gle_codata_find(name);')
file.append('')
file.append('\tif (constant == nullptr) {')
file.append('\t\tthrow std::runtime_error(')
file.append('\t\t\t"unknown CODATA physical constant: \\"" + name + "\\""')
file.append('\t\t);')
file.append('\t}')
file.append('')
file.append('\treturn constant->value;')
file.append('}')
file.append('')
#
# -- codata_units()
#
file.append('static std::string gle_codata_units(const std::string& name)')
file.append('{')
file.append('\tconst GLECODATAConstant* constant = gle_codata_find(name);')
file.append('')
file.append('\tif (constant == nullptr) {')
file.append('\t\tthrow std::runtime_error(')
file.append('\t\t\t"unknown CODATA physical constant: \\"" + name + "\\""')
file.append('\t\t);')
file.append('\t}')
file.append('')
file.append('\treturn constant->units;')
file.append('}')
file.append('')
#
# -- codata_uncertainty()
#
file.append('static double gle_codata_uncertainty(const std::string& name)')
file.append('{')
file.append('\tconst GLECODATAConstant* constant = gle_codata_find(name);')
file.append('')
file.append('\tif (constant == nullptr) {')
file.append('\t\tthrow std::runtime_error(')
file.append('\t\t\t"unknown CODATA physical constant: \\"" + name + "\\""')
file.append('\t\t);')
file.append('\t}')
file.append('')
file.append('\treturn constant->uncertainty;')
file.append('}')
file.append('')


with open(f'../src/gle/{cpp_filename}', 'w') as fp:
	# Iterate over the list and write each item to the file
	for line in file:
		fp.write(line + '\n')

#
# -- generate .tex file for documentation
#
file = []
file.append('\\begin{tabular}{ll} \\hline')
file.append('Mathematical Constants & Value  \\\\ \\hline')
file.append('{\\tt pi} \\index{pi}      & 3.14159265358979323846  \\\\')

# read in boost/math/constants/constants.hpp and parse this file
# have environment variable set BOOSTDIR to point to top level boot folder
boost_dir = os.getenv('BOOSTDIR')
content = []

with open(os.path.join(boost_dir,'boost','math','constants','constants.hpp'), 'r') as fp:
	content = fp.readlines()

lines = [line.strip() for line in content]

if content == None:
	print("error opening boost constants file")
	exit()

for c in boost_math_constants:
	line = [string for string in content if f"BOOST_DEFINE_MATH_CONSTANT({c}" in string][0]
	#print(line)
	value=line.split(",")[1]
	#print(f"{c} {value}")
	name = f"{mc_wart}{c}"
	name = name.replace("_","\\_")
	file.append(f"{{\\tt {name}}} \\index{{{name}}}  & {float(value):.16f} \\\\")

file.append('\\end{tabular}')
file.append('')
file.append('')
file.append('\\begin{tabular}{lll} \\hline')
file.append('Physical Constants & Value & Units \\\\ \\hline')

for c in scipy_physical_constants:
	#print(f"{c} {eval(f"sp.constants.{c}")}")
	name = f"{pc_wart}{c}"
	name = name.replace("_","\\_")
	units = "SI"
	file.append(f"{{\\tt {name}}} \\index{{{name}}}  & {eval(f'sp.constants.{c}')} & {units} \\\\")

file.append('\\end{tabular}')

with open(f'../../gle-manual/appendix/{tex_filename}', 'w') as fp:
	# Iterate over the list and write each item to the file
	for line in file:
		fp.write(line + '\n')

#
# -- generate gle file for testing
#
file = []
file.append('!')
file.append('! -- testing of constants')
file.append(f"print \"pi = \"  pi")

for c in boost_math_constants:
	file.append(f"print \"{mc_wart}{c} = \"  {mc_wart}{c}")

for c in scipy_physical_constants:
	file.append(f"print \"{pc_wart}{c} = \" {pc_wart}{c}")

with open(f'../src/gletests/{gle_filename}', 'w') as fp:
	# Iterate over the list and write each item to the file
	for line in file:
		fp.write(line + '\n')

# void gle_set_numerical_constants() {
# 	//
# 	// -- numerical (or mathematical) constants preceded with nc_ in GLE
# 	//
# 	//	  it is equivalent to the user adding the code
# 	//    PI = 3.14159....
# 	//    the GLE_PI constants are defined in all.h
# 	//    var_findadd_set("PI", GLE_PI);
# 	//    use boost::math::constants for best precisison
# 	//    GLE varaibles are case insensitve so add as uppercase
# 	//
# 	var_findadd_set("NC_PI",boost::math::double_constants::pi);
# 	var_findadd_set("NC_TWO_PI",boost::math::double_constants::two_pi);
# 	var_findadd_set("NC_ROOT_PI",boost::math::double_constants::root_two);
# 	var_findadd_set("NC_HALF_PI",boost::math::double_constants::half_pi);
# 	var_findadd_set("NC_ROOT_TWO",boost::math::double_constants::root_two);
# 	var_findadd_set("NC_ROOT_THREE",boost::math::double_constants::root_three);
# 	var_findadd_set("NC_E",boost::math::double_constants::e);
# }

# void gle_set_physical_constants() {
# 	//
# 	// -- physical constants pc_ in GLE
# 	//
# 	//    GLE variables are case insensitve so add as uppercase
# 	//    code autogenerated from scripts\constants.py and scipy.constants library
# 	//
# 	var_findadd_set("PI",boost::math::double_constants::pi);
# 	var_findadd_set("NC_PI",boost::math::double_constants::pi);
# 	var_findadd_set("NC_TWO_PI",boost::math::double_constants::two_pi);
# 	var_findadd_set("NC_ROOT_PI",boost::math::double_constants::root_two);
# 	var_findadd_set("NC_HALF_PI",boost::math::double_constants::half_pi);
# 	var_findadd_set("NC_ROOT_TWO",boost::math::double_constants::root_two);
# 	var_findadd_set("NC_ROOT_THREE",boost::math::double_constants::root_three);
# 	var_findadd_set("NC_E",boost::math::double_constants::e);
# }