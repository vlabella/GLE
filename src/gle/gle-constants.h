//
// -- gle-constants.h
//
//  automatically generated from scripts/constants.py
//  re-run manually and copy file to src/gle to modify
//  uses boost::math for numerical constants
//  uses scipy.constants for physical constants
//  includes the complete scipy CODATA physical constants database
//  mathematical constants start with mc_
//  physical constants start with pc_
//
//  gle variables are case in-sensitive
//
#include <cstddef>
#include <boost/math/constants/constants.hpp>
//
struct GLECODATAConstant {
	const char* name;
	double value;
	const char* units;
	double uncertainty;
};
//
static const GLECODATAConstant gle_codata_constants[] = {
	{"alpha particle mass", 6.6446573449999998e-27, "kg", 2.0999999999999999e-36},
	{"alpha particle mass energy equivalent", 5.9719201997000001e-10, "J", 1.9e-19},
	{"alpha particle mass energy equivalent in MeV", 3727.3794118000001, "MeV", 1.1999999999999999e-06},
	{"alpha particle mass in u", 4.0015061791289996, "u", 6.2000000000000006e-11},
	{"alpha particle molar mass", 0.0040015061833000003, "kg mol^-1", 1.1999999999999999e-12},
	{"alpha particle relative atomic mass", 4.0015061791289996, "", 6.2000000000000006e-11},
	{"alpha particle rms charge radius", 1.6785e-15, "m", 2.1e-18},
	{"alpha particle-electron mass ratio", 7294.2995417100001, "", 1.6999999999999999e-07},
	{"alpha particle-proton mass ratio", 3.9725996902520002, "", 7.0000000000000004e-11},
	{"Angstrom star", 1.0000149499999999e-10, "m", 8.9999999999999996e-17},
	{"atomic mass constant", 1.6605390689199999e-27, "kg", 5.1999999999999996e-37},
	{"atomic mass constant energy equivalent", 1.49241808768e-10, "J", 4.5999999999999998e-20},
	{"atomic mass constant energy equivalent in MeV", 931.49410372, "MeV", 2.8999999999999998e-07},
	{"atomic mass unit-electron volt relationship", 931494103.72000003, "eV", 0.28999999999999998},
	{"atomic mass unit-hartree relationship", 34231776.921999998, "E_h", 0.010999999999999999},
	{"atomic mass unit-hertz relationship", 2.25234272185e+23, "Hz", 70000000000000.0},
	{"atomic mass unit-inverse meter relationship", 751300662090000.0, "m^-1", 230000.0},
	{"atomic mass unit-joule relationship", 1.49241808768e-10, "J", 4.5999999999999998e-20},
	{"atomic mass unit-kelvin relationship", 10809540206700.0, "K", 3400.0},
	{"atomic mass unit-kilogram relationship", 1.6605390689199999e-27, "kg", 5.1999999999999996e-37},
	{"atomic unit of 1st hyperpolarizability", 3.2063612995999999e-53, "C^3 m^3 J^-2", 1.5000000000000001e-62},
	{"atomic unit of 1st hyperpolarizablity", 3.2063615099999998e-53, "C^3 m^3 J^-2", 2.8000000000000002e-60},
	{"atomic unit of 2nd hyperpolarizability", 6.2353799735000003e-65, "C^4 m^4 J^-3", 3.9000000000000001e-74},
	{"atomic unit of 2nd hyperpolarizablity", 6.2353808e-65, "C^4 m^4 J^-3", 1.1e-71},
	{"atomic unit of action", 1.0545718176461565e-34, "J s", 0.0},
	{"atomic unit of charge", 1.6021766339999999e-19, "C", 0.0},
	{"atomic unit of charge density", 1081202386770.0, "C m^-3", 510.0},
	{"atomic unit of current", 0.0066236182375082001, "A", 7.2000000000000002e-15},
	{"atomic unit of electric dipole mom.", 8.4783536198000007e-30, "C m", 1.3e-39},
	{"atomic unit of electric dipole moment", 8.4783530900000004e-30, "C m", 7.2999999999999997e-37},
	{"atomic unit of electric field", 514220675112.0, "V m^-1", 80.0},
	{"atomic unit of electric field gradient", 9.7173624424000001e+21, "V m^-2", 3000000000000.0},
	{"atomic unit of electric polarizability", 1.64877727212e-41, "C^2 m^2 J^-1", 5.0999999999999997e-51},
	{"atomic unit of electric polarizablity", 1.6487772740000001e-41, "C^2 m^2 J^-1", 1.6e-49},
	{"atomic unit of electric potential", 27.211386245981, "V", 3e-11},
	{"atomic unit of electric quadrupole mom.", 4.4865515185e-40, "C m^2", 1.4e-49},
	{"atomic unit of electric quadrupole moment", 4.4865512399999996e-40, "C m^2", 3.8999999999999998e-47},
	{"atomic unit of energy", 4.3597447222059999e-18, "J", 4.7999999999999997e-30},
	{"atomic unit of force", 8.2387235038000003e-08, "N", 1.3e-17},
	{"atomic unit of length", 5.2917721054399998e-11, "m", 8.1999999999999999e-21},
	{"atomic unit of mag. dipole mom.", 1.8548020131499999e-23, "J T^-1", 5.8000000000000001e-33},
	{"atomic unit of mag. flux density", 235051.75707699999, "T", 7.2999999999999999e-05},
	{"atomic unit of magn. dipole moment", 1.8548019e-23, "J T^-1", 1.6000000000000001e-30},
	{"atomic unit of magn. flux density", 235051.75707699999, "T", 7.2999999999999999e-05},
	{"atomic unit of magnetizability", 7.8910365793999994e-29, "J T^-2", 4.9000000000000004e-38},
	{"atomic unit of mass", 9.1093837138999998e-31, "kg", 2.8e-40},
	{"atomic unit of mom.um", 1.992851882e-24, "kg m s^-1", 2.4000000000000001e-32},
	{"atomic unit of momentum", 1.9928519154499998e-24, "kg m s^-1", 3.0999999999999998e-34},
	{"atomic unit of permittivity", 1.1126500562000001e-10, "F m^-1", 1.6999999999999999e-20},
	{"atomic unit of time", 2.4188843265863999e-17, "s", 2.6000000000000002e-29},
	{"atomic unit of velocity", 2187691.2621599999, "m s^-1", 0.00034000000000000002},
	{"Avogadro constant", 6.0221407599999999e+23, "mol^-1", 0.0},
	{"Bohr magneton", 9.2740100656999999e-24, "J T^-1", 2.9e-33},
	{"Bohr magneton in eV/T", 5.7883817982e-05, "eV T^-1", 1.7999999999999999e-14},
	{"Bohr magneton in Hz/T", 13996244917.1, "Hz T^-1", 4.4000000000000004},
	{"Bohr magneton in inverse meter per tesla", 46.686447719, "m^-1 T^-1", 1.4999999999999999e-08},
	{"Bohr magneton in inverse meters per tesla", 46.686448140000003, "m^-1 T^-1", 2.8999999999999998e-07},
	{"Bohr magneton in K/T", 0.67171381472000002, "K T^-1", 2.1e-10},
	{"Bohr radius", 5.2917721054399998e-11, "m", 8.1999999999999999e-21},
	{"Boltzmann constant", 1.3806490000000001e-23, "J K^-1", 0.0},
	{"Boltzmann constant in eV/K", 8.6173332621451788e-05, "eV K^-1", 0.0},
	{"Boltzmann constant in Hz/K", 20836619123.327576, "Hz K^-1", 0.0},
	{"Boltzmann constant in inverse meter per kelvin", 69.503480048612744, "m^-1 K^-1", 0.0},
	{"Boltzmann constant in inverse meters per kelvin", 69.503456999999997, "m^-1 K^-1", 4.0000000000000003e-05},
	{"characteristic impedance of vacuum", 376.73031341199999, "ohm", 5.8999999999999999e-08},
	{"classical electron radius", 2.8179403204999999e-15, "m", 1.3e-24},
	{"Compton wavelength", 2.42631023538e-12, "m", 7.6000000000000005e-22},
	{"Compton wavelength over 2 pi", 3.8615926763999998e-13, "m", 1.7999999999999999e-22},
	{"conductance quantum", 7.7480917298636487e-05, "S", 0.0},
	{"conventional value of ampere-90", 1.0000000888714378, "A", 0.0},
	{"conventional value of coulomb-90", 1.0000000888714378, "C", 0.0},
	{"conventional value of farad-90", 0.99999998220633246, "F", 0.0},
	{"conventional value of henry-90", 1.0000000177936679, "H", 0.0},
	{"conventional value of Josephson constant", 483597900000000.0, "Hz V^-1", 0.0},
	{"conventional value of ohm-90", 1.0000000177936679, "ohm", 0.0},
	{"conventional value of volt-90", 1.0000001066651072, "V", 0.0},
	{"conventional value of von Klitzing constant", 25812.807000000001, "ohm", 0.0},
	{"conventional value of watt-90", 1.0000001955365543, "W", 0.0},
	{"Copper x unit", 1.00207697e-13, "m", 2.8000000000000003e-20},
	{"Cu x unit", 1.00207697e-13, "m", 2.8000000000000003e-20},
	{"deuteron g factor", 0.85743823350000004, "", 2.1999999999999998e-09},
	{"deuteron mag. mom.", 4.3307350869999997e-27, "J T^-1", 1.1e-35},
	{"deuteron mag. mom. to Bohr magneton ratio", 0.00046697545680000001, "", 1.1999999999999999e-12},
	{"deuteron mag. mom. to nuclear magneton ratio", 0.85743823350000004, "", 2.1999999999999998e-09},
	{"deuteron magn. moment", 4.3307348200000001e-27, "J T^-1", 3.8000000000000001e-34},
	{"deuteron magn. moment to Bohr magneton ratio", 0.00046697545669999998, "", 4.9999999999999997e-12},
	{"deuteron magn. moment to nuclear magneton ratio", 0.85743823289999999, "", 9.1999999999999997e-09},
	{"deuteron mass", 3.3435837768000001e-27, "kg", 9.9999999999999994e-37},
	{"deuteron mass energy equivalent", 3.00506323491e-10, "J", 9.4000000000000003e-20},
	{"deuteron mass energy equivalent in MeV", 1875.6129450000001, "MeV", 5.7999999999999995e-07},
	{"deuteron mass in u", 2.0135532125440001, "u", 1.5e-11},
	{"deuteron molar mass", 0.0020135532146599998, "kg mol^-1", 6.3000000000000004e-13},
	{"deuteron relative atomic mass", 2.0135532125440001, "", 1.5e-11},
	{"deuteron rms charge radius", 2.12778e-15, "m", 2.7000000000000001e-19},
	{"deuteron-electron mag. mom. ratio", -0.00046643455500000001, "", 1.1999999999999999e-12},
	{"deuteron-electron magn. moment ratio", -0.00046643455479999999, "", 4.9999999999999997e-12},
	{"deuteron-electron mass ratio", 3670.4829676549998, "", 6.2999999999999995e-08},
	{"deuteron-neutron mag. mom. ratio", -0.44820652, "", 1.1000000000000001e-07},
	{"deuteron-neutron magn. moment ratio", -0.44820652, "", 1.1000000000000001e-07},
	{"deuteron-proton mag. mom. ratio", 0.30701220930000001, "", 7.8999999999999996e-10},
	{"deuteron-proton magn. moment ratio", 0.30701220839999999, "", 4.4999999999999998e-09},
	{"deuteron-proton mass ratio", 1.9990075012699, "", 8.3999999999999998e-12},
	{"electric constant", 8.8541878188000004e-12, "F m^-1", 1.4e-21},
	{"electron charge to mass quotient", -175882000838.0, "C kg^-1", 55.0},
	{"electron g factor", -2.00231930436092, "", 3.5999999999999998e-13},
	{"electron gyromag. ratio", 176085962784.0, "s^-1 T^-1", 55.0},
	{"electron gyromag. ratio in MHz/T", 28024.951386100001, "MHz T^-1", 8.6999999999999997e-06},
	{"electron gyromag. ratio over 2 pi", 28024.951639999999, "MHz T^-1", 0.00017000000000000001},
	{"electron gyromagn. ratio", 176085962784.0, "s^-1 T^-1", 55.0},
	{"electron gyromagn. ratio over 2 pi", 28024.9532, "MHz T^-1", 0.0023999999999999998},
	{"electron mag. mom.", -9.2847646917e-24, "J T^-1", 2.9e-33},
	{"electron mag. mom. anomaly", 0.00115965218046, "", 1.7999999999999999e-13},
	{"electron mag. mom. to Bohr magneton ratio", -1.00115965218046, "", 1.7999999999999999e-13},
	{"electron mag. mom. to nuclear magneton ratio", -1838.2819718769999, "", 3.2000000000000002e-08},
	{"electron magn. moment", -9.2847641200000005e-24, "J T^-1", 8.0000000000000007e-31},
	{"electron magn. moment anomaly", 0.0011596521859, "", 3.8e-12},
	{"electron magn. moment to Bohr magneton ratio", -1.0011596521859001, "", 3.8e-12},
	{"electron magn. moment to nuclear magneton ratio", -1838.2819710700001, "", 8.5000000000000001e-07},
	{"electron mass", 9.1093837138999998e-31, "kg", 2.8e-40},
	{"electron mass energy equivalent", 8.1871057880000001e-14, "J", 2.6e-23},
	{"electron mass energy equivalent in MeV", 0.51099895069000001, "MeV", 1.5999999999999999e-10},
	{"electron mass in u", 0.00054857990904409997, "u", 9.7000000000000006e-15},
	{"electron molar mass", 5.4857990961999995e-07, "kg mol^-1", 1.7e-16},
	{"electron relative atomic mass", 0.00054857990904409997, "", 9.7000000000000006e-15},
	{"electron to alpha particle mass ratio", 0.00013709335547329999, "", 3.1999999999999999e-15},
	{"electron to shielded helion mag. mom. ratio", 864.05823985999996, "", 6.9999999999999997e-07},
	{"electron to shielded helion magn. moment ratio", 864.05825500000003, "", 1.0000000000000001e-05},
	{"electron to shielded proton mag. mom. ratio", -658.2275856, "", 2.7e-06},
	{"electron to shielded proton magn. moment ratio", -658.22759559999997, "", 7.0999999999999998e-06},
	{"electron volt", 1.6021766339999999e-19, "J", 0.0},
	{"electron volt-atomic mass unit relationship", 1.0735441008299999e-09, "u", 3.2999999999999998e-19},
	{"electron volt-hartree relationship", 0.036749322175664997, "E_h", 4e-14},
	{"electron volt-hertz relationship", 241798924208491.81, "Hz", 0.0},
	{"electron volt-inverse meter relationship", 806554.39373492112, "m^-1", 0.0},
	{"electron volt-joule relationship", 1.6021766339999999e-19, "J", 0.0},
	{"electron volt-kelvin relationship", 11604.518121550082, "K", 0.0},
	{"electron volt-kilogram relationship", 1.7826619216278975e-36, "kg", 0.0},
	{"electron-deuteron mag. mom. ratio", -2143.9234921000002, "", 5.5999999999999997e-06},
	{"electron-deuteron magn. moment ratio", -2143.9234929999998, "", 2.3e-05},
	{"electron-deuteron mass ratio", 0.00027244371076290001, "", 4.6999999999999999e-15},
	{"electron-helion mass ratio", 0.00018195430746489999, "", 5.3000000000000001e-15},
	{"electron-muon mag. mom. ratio", 206.76698809999999, "", 4.6e-06},
	{"electron-muon magn. moment ratio", 206.7669894, "", 5.4e-06},
	{"electron-muon mass ratio", 0.0048363316999999999, "", 1.0999999999999999e-10},
	{"electron-neutron mag. mom. ratio", 960.92048, "", 0.00023000000000000001},
	{"electron-neutron magn. moment ratio", 960.92049999999995, "", 0.00023000000000000001},
	{"electron-neutron mass ratio", 0.00054386734416000005, "", 2.2e-13},
	{"electron-proton mag. mom. ratio", -658.21068789000003, "", 1.9000000000000001e-07},
	{"electron-proton magn. moment ratio", -658.21068620000005, "", 6.6000000000000003e-06},
	{"electron-proton mass ratio", 0.00054461702148890002, "", 9.3999999999999997e-15},
	{"electron-tau mass ratio", 0.00028758499999999998, "", 1.9000000000000001e-08},
	{"electron-triton mass ratio", 0.0001819200062327, "", 6.8000000000000001e-15},
	{"elementary charge", 1.6021766339999999e-19, "C", 0.0},
	{"elementary charge over h", 241798926200000.0, "A J^-1", 1500000.0},
	{"elementary charge over h-bar", 1519267447878626.0, "A J^-1", 0.0},
	{"Faraday constant", 96485.332123310014, "C mol^-1", 0.0},
	{"Faraday constant for conventional electric current", 96485.325100000002, "C_90 mol^-1", 0.0011999999999999999},
	{"Fermi coupling constant", 1.1663786999999999e-05, "GeV^-2", 6.0000000000000003e-12},
	{"fine-structure constant", 0.0072973525643, "", 1.1e-12},
	{"first radiation constant", 3.7417718521927573e-16, "W m^2", 0.0},
	{"first radiation constant for spectral radiance", 1.1910429723971884e-16, "W m^2 sr^-1", 0.0},
	{"Hartree energy", 4.3597447222059999e-18, "J", 4.7999999999999997e-30},
	{"Hartree energy in eV", 27.211386245981, "eV", 3e-11},
	{"hartree-atomic mass unit relationship", 2.9212623179699999e-08, "u", 9.0999999999999993e-18},
	{"hartree-electron volt relationship", 27.211386245981, "eV", 3e-11},
	{"hartree-hertz relationship", 6579683920499900.0, "Hz", 7200.0},
	{"hartree-inverse meter relationship", 21947463.136314001, "m^-1", 2.4000000000000001e-05},
	{"hartree-joule relationship", 4.3597447222059999e-18, "J", 4.7999999999999997e-30},
	{"hartree-kelvin relationship", 315775.02480398002, "K", 3.3999999999999997e-07},
	{"hartree-kilogram relationship", 4.8508702095419004e-35, "kg", 5.3e-47},
	{"helion g factor", -4.2552506995000003, "", 3.3999999999999998e-09},
	{"helion mag. mom.", -1.07461755198e-26, "J T^-1", 9.2999999999999996e-36},
	{"helion mag. mom. to Bohr magneton ratio", -0.00115874098083, "", 9.4000000000000003e-13},
	{"helion mag. mom. to nuclear magneton ratio", -2.1276253498000002, "", 1.6999999999999999e-09},
	{"helion mass", 5.0064127861999999e-27, "kg", 1.6000000000000001e-36},
	{"helion mass energy equivalent", 4.4995394184999999e-10, "J", 1.4e-19},
	{"helion mass energy equivalent in MeV", 2808.3916111200001, "MeV", 8.8000000000000004e-07},
	{"helion mass in u", 3.0149322469320001, "u", 7.4000000000000003e-11},
	{"helion molar mass", 0.0030149322500999999, "kg mol^-1", 9.4000000000000003e-13},
	{"helion relative atomic mass", 3.0149322469320001, "", 7.4000000000000003e-11},
	{"helion shielding shift", 5.9967029000000003e-05, "", 2.3000000000000001e-11},
	{"helion-electron mass ratio", 5495.8852798400003, "", 1.6e-07},
	{"helion-proton mass ratio", 2.993152671552, "", 7.0000000000000004e-11},
	{"hertz-atomic mass unit relationship", 4.4398216590000001e-24, "u", 1.3999999999999999e-33},
	{"hertz-electron volt relationship", 4.135667696923859e-15, "eV", 0.0},
	{"hertz-hartree relationship", 1.5198298460574e-16, "E_h", 1.7e-28},
	{"hertz-inverse meter relationship", 3.3356409519815204e-09, "m^-1", 0.0},
	{"hertz-joule relationship", 6.6260701499999998e-34, "J", 0.0},
	{"hertz-kelvin relationship", 4.799243073366221e-11, "K", 0.0},
	{"hertz-kilogram relationship", 7.3724973238127084e-51, "kg", 0.0},
	{"hyperfine transition frequency of Cs-133", 9192631770.0, "Hz", 0.0},
	{"inverse fine-structure constant", 137.03599917700001, "", 2.0999999999999999e-08},
	{"inverse meter-atomic mass unit relationship", 1.33102504824e-15, "u", 4.0999999999999999e-25},
	{"inverse meter-electron volt relationship", 1.2398419843320026e-06, "eV", 0.0},
	{"inverse meter-hartree relationship", 4.5563352529132001e-08, "E_h", 4.9999999999999999e-20},
	{"inverse meter-hertz relationship", 299792458.0, "Hz", 0.0},
	{"inverse meter-joule relationship", 1.9864458571489286e-25, "J", 0.0},
	{"inverse meter-kelvin relationship", 0.014387768775039337, "K", 0.0},
	{"inverse meter-kilogram relationship", 2.2102190943042335e-42, "kg", 0.0},
	{"inverse of conductance quantum", 12906.403729652257, "ohm", 0.0},
	{"Josephson constant", 483597848416983.62, "Hz V^-1", 0.0},
	{"joule-atomic mass unit relationship", 6700535247.1000004, "u", 2.1000000000000001},
	{"joule-electron volt relationship", 6.2415090744607631e+18, "eV", 0.0},
	{"joule-hartree relationship", 2.2937122783968998e+17, "E_h", 250000.0},
	{"joule-hertz relationship", 1.5091901796421518e+33, "Hz", 0.0},
	{"joule-inverse meter relationship", 5.0341165675427099e+24, "m^-1", 0.0},
	{"joule-kelvin relationship", 7.2429705160399199e+22, "K", 0.0},
	{"joule-kilogram relationship", 1.1126500560536185e-17, "kg", 0.0},
	{"kelvin-atomic mass unit relationship", 9.2510872884e-14, "u", 2.9000000000000002e-23},
	{"kelvin-electron volt relationship", 8.6173332621451788e-05, "eV", 0.0},
	{"kelvin-hartree relationship", 3.1668115634564e-06, "E_h", 3.4999999999999999e-18},
	{"kelvin-hertz relationship", 20836619123.327576, "Hz", 0.0},
	{"kelvin-inverse meter relationship", 69.503480048612744, "m^-1", 0.0},
	{"kelvin-joule relationship", 1.3806490000000001e-23, "J", 0.0},
	{"kelvin-kilogram relationship", 1.5361791872403723e-40, "kg", 0.0},
	{"kilogram-atomic mass unit relationship", 6.0221407537e+26, "u", 1.9e+17},
	{"kilogram-electron volt relationship", 5.6095886038044519e+35, "eV", 0.0},
	{"kilogram-hartree relationship", 2.0614857887415001e+34, "E_h", 2.2e+22},
	{"kilogram-hertz relationship", 1.3563924896521321e+50, "Hz", 0.0},
	{"kilogram-inverse meter relationship", 4.524438335443823e+41, "m^-1", 0.0},
	{"kilogram-joule relationship", 89875517873681760.0, "J", 0.0},
	{"kilogram-kelvin relationship", 6.5096572607289584e+39, "K", 0.0},
	{"lattice parameter of silicon", 5.4310205110000003e-10, "m", 8.9000000000000004e-18},
	{"lattice spacing of ideal Si (220)", 1.9201557159999999e-10, "m", 3.1999999999999999e-18},
	{"lattice spacing of silicon", 1.920155762e-10, "m", 5.0000000000000004e-18},
	{"Loschmidt constant (273.15 K, 100 kPa)", 2.6516458048837345e+25, "m^-3", 0.0},
	{"Loschmidt constant (273.15 K, 101.325 kPa)", 2.6867801117984439e+25, "m^-3", 0.0},
	{"luminous efficacy", 683.0, "lm W^-1", 0.0},
	{"mag. constant", 1.2566370612700001e-06, "N A^-2", 2e-16},
	{"mag. flux quantum", 2.0678338484619295e-15, "Wb", 0.0},
	{"magn. constant", 1.2566370614359173e-06, "N A^-2", 0.0},
	{"magn. flux quantum", 2.0678338484619295e-15, "Wb", 0.0},
	{"Mo x unit", 1.0020995200000001e-13, "m", 5.3000000000000002e-20},
	{"molar gas constant", 8.3144626181532395, "J mol^-1 K^-1", 0.0},
	{"molar mass constant", 0.00100000000105, "kg mol^-1", 3.0999999999999999e-13},
	{"molar mass of carbon-12", 0.012000000012599999, "kg mol^-1", 3.7e-12},
	{"molar Planck constant", 3.9903127128934311e-10, "J Hz^-1 mol^-1", 0.0},
	{"molar Planck constant times c", 0.11962656558199999, "J m mol^-1", 5.4000000000000001e-11},
	{"molar volume of ideal gas (273.15 K, 100 kPa)", 0.022710954641485571, "m^3 mol^-1", 0.0},
	{"molar volume of ideal gas (273.15 K, 101.325 kPa)", 0.022413969545014137, "m^3 mol^-1", 0.0},
	{"molar volume of silicon", 1.205883199e-05, "m^3 mol^-1", 5.9999999999999997e-13},
	{"Molybdenum x unit", 1.0020995200000001e-13, "m", 5.3000000000000002e-20},
	{"muon Compton wavelength", 1.17344411e-14, "m", 2.6e-22},
	{"muon Compton wavelength over 2 pi", 1.8675943079999998e-15, "m", 4.2000000000000002e-23},
	{"muon g factor", -2.0023318412300002, "", 8.1999999999999996e-10},
	{"muon mag. mom.", -4.4904482999999998e-26, "J T^-1", 1.0000000000000001e-33},
	{"muon mag. mom. anomaly", 0.0011659206200000001, "", 4.0999999999999998e-10},
	{"muon mag. mom. to Bohr magneton ratio", -0.0048419704799999999, "", 1.0999999999999999e-10},
	{"muon mag. mom. to nuclear magneton ratio", -8.8905970399999994, "", 1.9999999999999999e-07},
	{"muon magn. moment", -4.4904479900000003e-26, "J T^-1", 4.0000000000000002e-33},
	{"muon magn. moment to Bohr magneton ratio", -0.0048419704500000001, "", 1.2999999999999999e-10},
	{"muon magn. moment to nuclear magneton ratio", -8.8905969799999998, "", 2.2999999999999999e-07},
	{"muon mass", 1.8835316270000001e-28, "kg", 4.1999999999999998e-36},
	{"muon mass energy equivalent", 1.6928338039999999e-11, "J", 3.8e-19},
	{"muon mass energy equivalent in MeV", 105.65837550000001, "MeV", 2.3e-06},
	{"muon mass in u", 0.1134289257, "u", 2.5000000000000001e-09},
	{"muon molar mass", 0.0001134289258, "kg mol^-1", 2.4999999999999998e-12},
	{"muon-electron mass ratio", 206.76828269999999, "", 4.6e-06},
	{"muon-neutron mass ratio", 0.1124545168, "", 2.5000000000000001e-09},
	{"muon-proton mag. mom. ratio", -3.1833451460000002, "", 7.1e-08},
	{"muon-proton magn. moment ratio", -3.1833451180000001, "", 8.9000000000000003e-08},
	{"muon-proton mass ratio", 0.1126095262, "", 2.5000000000000001e-09},
	{"muon-tau mass ratio", 0.059463500000000002, "", 3.9999999999999998e-06},
	{"natural unit of action", 1.0545718176461565e-34, "J s", 0.0},
	{"natural unit of action in eV s", 6.582119569509067e-16, "eV s", 0.0},
	{"natural unit of energy", 8.1871057880000001e-14, "J", 2.6e-23},
	{"natural unit of energy in MeV", 0.51099895069000001, "MeV", 1.5999999999999999e-10},
	{"natural unit of length", 3.8615926743999999e-13, "m", 1.2e-22},
	{"natural unit of mass", 9.1093837138999998e-31, "kg", 2.8e-40},
	{"natural unit of mom.um", 2.730924488e-22, "kg m s^-1", 3.4000000000000003e-30},
	{"natural unit of mom.um in MeV/c", 0.51099894609999996, "MeV/c", 3.1e-09},
	{"natural unit of momentum", 2.730924488e-22, "kg m s^-1", 3.4000000000000003e-30},
	{"natural unit of momentum in MeV/c", 0.51099894609999996, "MeV/c", 3.1e-09},
	{"natural unit of time", 1.28808866644e-21, "s", 4.0000000000000003e-31},
	{"natural unit of velocity", 299792458.0, "m s^-1", 0.0},
	{"neutron Compton wavelength", 1.31959090382e-15, "m", 6.7000000000000003e-25},
	{"neutron Compton wavelength over 2 pi", 2.1001941535999999e-16, "m", 1.3999999999999999e-25},
	{"neutron g factor", -3.8260855199999999, "", 8.9999999999999996e-07},
	{"neutron gyromag. ratio", 183247174.0, "s^-1 T^-1", 43.0},
	{"neutron gyromag. ratio in MHz/T", 29.164693499999998, "MHz T^-1", 6.9e-06},
	{"neutron gyromag. ratio over 2 pi", 29.1646933, "MHz T^-1", 6.9e-06},
	{"neutron gyromagn. ratio", 183247174.0, "s^-1 T^-1", 43.0},
	{"neutron gyromagn. ratio over 2 pi", 29.164694999999998, "MHz T^-1", 7.3000000000000004e-06},
	{"neutron mag. mom.", -9.6623653e-27, "J T^-1", 2.2999999999999999e-33},
	{"neutron mag. mom. to Bohr magneton ratio", -0.0010418756500000001, "", 2.5000000000000002e-10},
	{"neutron mag. mom. to nuclear magneton ratio", -1.91304276, "", 4.4999999999999998e-07},
	{"neutron magn. moment", -9.6623644999999999e-27, "J T^-1", 2.4e-33},
	{"neutron magn. moment to Bohr magneton ratio", -0.0010418756299999999, "", 2.5000000000000002e-10},
	{"neutron magn. moment to nuclear magneton ratio", -1.9130427299999999, "", 4.4999999999999998e-07},
	{"neutron mass", 1.6749275005600001e-27, "kg", 8.5000000000000007e-37},
	{"neutron mass energy equivalent", 1.50534976514e-10, "J", 7.5999999999999995e-20},
	{"neutron mass energy equivalent in MeV", 939.56542193999996, "MeV", 4.7999999999999996e-07},
	{"neutron mass in u", 1.0086649160600001, "u", 4.0000000000000001e-10},
	{"neutron molar mass", 0.00100866491712, "kg mol^-1", 5.1000000000000005e-13},
	{"neutron relative atomic mass", 1.0086649160600001, "", 4.0000000000000001e-10},
	{"neutron to shielded proton mag. mom. ratio", -0.68499694, "", 1.6e-07},
	{"neutron to shielded proton magn. moment ratio", -0.68499694, "", 1.6e-07},
	{"neutron-electron mag. mom. ratio", 0.0010406688399999999, "", 2.4e-10},
	{"neutron-electron magn. moment ratio", 0.00104066882, "", 2.5000000000000002e-10},
	{"neutron-electron mass ratio", 1838.6836619999999, "", 7.4000000000000001e-07},
	{"neutron-muon mass ratio", 8.8924840799999991, "", 1.9999999999999999e-07},
	{"neutron-proton mag. mom. ratio", -0.68497934999999999, "", 1.6e-07},
	{"neutron-proton magn. moment ratio", -0.68497934000000005, "", 1.6e-07},
	{"neutron-proton mass difference", 2.3055746100000001e-30, "kg", 6.7e-37},
	{"neutron-proton mass difference energy equivalent", 2.0721471199999999e-13, "J", 6.0000000000000006e-20},
	{"neutron-proton mass difference energy equivalent in MeV", 1.2933325099999999, "MeV", 3.8000000000000001e-07},
	{"neutron-proton mass difference in u", 0.00138844948, "u", 4.0000000000000001e-10},
	{"neutron-proton mass ratio", 1.0013784194599999, "", 4.0000000000000001e-10},
	{"neutron-tau mass ratio", 0.528779, "", 3.6000000000000001e-05},
	{"Newtonian constant of gravitation", 6.6742999999999994e-11, "m^3 kg^-1 s^-2", 1.4999999999999999e-15},
	{"Newtonian constant of gravitation over h-bar c", 6.7088300000000006e-39, "(GeV/c^2)^-2", 1.5e-43},
	{"nuclear magneton", 5.0507837393000001e-27, "J T^-1", 1.6000000000000001e-36},
	{"nuclear magneton in eV/T", 3.1524512541699999e-08, "eV T^-1", 9.8000000000000002e-18},
	{"nuclear magneton in inverse meter per tesla", 0.025426234100899999, "m^-1 T^-1", 7.8999999999999999e-12},
	{"nuclear magneton in inverse meters per tesla", 0.025426234320000001, "m^-1 T^-1", 1.5999999999999999e-10},
	{"nuclear magneton in K/T", 0.00036582677705999998, "K T^-1", 1.1e-13},
	{"nuclear magneton in MHz/T", 7.6225932187999996, "MHz T^-1", 2.4e-09},
	{"Planck constant", 6.6260701499999998e-34, "J Hz^-1", 0.0},
	{"Planck constant in eV s", 4.1356676619999999e-15, "eV s", 2.5000000000000001e-23},
	{"Planck constant in eV/Hz", 4.135667696923859e-15, "eV Hz^-1", 0.0},
	{"Planck constant over 2 pi", 1.0545718e-34, "J s", 1.3e-42},
	{"Planck constant over 2 pi in eV s", 6.5821195139999997e-16, "eV s", 3.9999999999999997e-24},
	{"Planck constant over 2 pi times c in MeV fm", 197.32697880000001, "MeV fm", 1.1999999999999999e-06},
	{"Planck length", 1.6162550000000001e-35, "m", 1.8e-40},
	{"Planck mass", 2.1764340000000001e-08, "kg", 2.3999999999999999e-13},
	{"Planck mass energy equivalent in GeV", 1.22089e+19, "GeV", 140000000000000.0},
	{"Planck temperature", 1.416784e+32, "K", 1.6000000000000001e+27},
	{"Planck time", 5.3912470000000002e-44, "s", 6e-49},
	{"proton charge to mass quotient", 95788331.430000007, "C kg^-1", 0.029999999999999999},
	{"proton Compton wavelength", 1.3214098536e-15, "m", 4.0999999999999999e-25},
	{"proton Compton wavelength over 2 pi", 2.10308910109e-16, "m", 9.7000000000000001e-26},
	{"proton g factor", 5.5856946893000003, "", 1.6000000000000001e-09},
	{"proton gyromag. ratio", 267522187.08000001, "s^-1 T^-1", 0.11},
	{"proton gyromag. ratio in MHz/T", 42.577478460999998, "MHz T^-1", 1.7999999999999999e-08},
	{"proton gyromag. ratio over 2 pi", 42.577478919999997, "MHz T^-1", 2.8999999999999998e-07},
	{"proton gyromagn. ratio", 267522187.08000001, "s^-1 T^-1", 0.11},
	{"proton gyromagn. ratio over 2 pi", 42.577481300000002, "MHz T^-1", 3.7000000000000002e-06},
	{"proton mag. mom.", 1.4106067954500001e-26, "J T^-1", 6.0000000000000003e-36},
	{"proton mag. mom. to Bohr magneton ratio", 0.0015210322023, "", 4.5e-13},
	{"proton mag. mom. to nuclear magneton ratio", 2.7928473446300002, "", 8.1999999999999996e-10},
	{"proton mag. shielding correction", 2.56715e-05, "", 4.1000000000000003e-09},
	{"proton magn. moment", 1.4106067099999999e-26, "J T^-1", 1.2e-33},
	{"proton magn. moment to Bohr magneton ratio", 0.0015210322059999999, "", 1.5e-11},
	{"proton magn. moment to nuclear magneton ratio", 2.7928473509999998, "", 2.7999999999999999e-08},
	{"proton magn. shielding correction", 2.56715e-05, "", 4.1000000000000003e-09},
	{"proton mass", 1.6726219259499999e-27, "kg", 5.1999999999999996e-37},
	{"proton mass energy equivalent", 1.50327761802e-10, "J", 4.7000000000000001e-20},
	{"proton mass energy equivalent in MeV", 938.27208943000005, "MeV", 2.8999999999999998e-07},
	{"proton mass in u", 1.0072764665789, "u", 8.2999999999999998e-12},
	{"proton molar mass", 0.0010072764676399999, "kg mol^-1", 3.0999999999999999e-13},
	{"proton relative atomic mass", 1.0072764665789, "", 8.2999999999999998e-12},
	{"proton rms charge radius", 8.4075e-16, "m", 6.3999999999999996e-19},
	{"proton-electron mass ratio", 1836.1526734260001, "", 3.2000000000000002e-08},
	{"proton-muon mass ratio", 8.8802433799999996, "", 1.9999999999999999e-07},
	{"proton-neutron mag. mom. ratio", -1.45989802, "", 3.3999999999999997e-07},
	{"proton-neutron magn. moment ratio", -1.4598980500000001, "", 3.3999999999999997e-07},
	{"proton-neutron mass ratio", 0.99862347796999995, "", 4.0000000000000001e-10},
	{"proton-tau mass ratio", 0.52805100000000005, "", 3.6000000000000001e-05},
	{"quantum of circulation", 0.00036369475467000001, "m^2 s^-1", 1.1e-13},
	{"quantum of circulation times 2", 0.00072738950934000002, "m^2 s^-1", 2.2999999999999998e-13},
	{"reduced Compton wavelength", 3.8615926743999999e-13, "m", 1.2e-22},
	{"reduced muon Compton wavelength", 1.867594306e-15, "m", 4.2000000000000002e-23},
	{"reduced neutron Compton wavelength", 2.1001941519999999e-16, "m", 1.1000000000000001e-25},
	{"reduced Planck constant", 1.0545718176461565e-34, "J s", 0.0},
	{"reduced Planck constant in eV s", 6.582119569509067e-16, "eV s", 0.0},
	{"reduced Planck constant times c in MeV fm", 197.32698045930249, "MeV fm", 0.0},
	{"reduced proton Compton wavelength", 2.1030891005099999e-16, "m", 6.5999999999999997e-26},
	{"reduced tau Compton wavelength", 1.1105380000000001e-16, "m", 7.5000000000000007e-21},
	{"Rydberg constant", 10973731.568157, "m^-1", 1.2e-05},
	{"Rydberg constant times c in Hz", 3289841960250000.0, "Hz", 3600.0},
	{"Rydberg constant times hc in eV", 13.605693122990001, "eV", 1.5e-11},
	{"Rydberg constant times hc in J", 2.1798723611029999e-18, "J", 2.3999999999999998e-30},
	{"Sackur-Tetrode constant (1 K, 100 kPa)", -1.1517075349599999, "", 4.7000000000000003e-10},
	{"Sackur-Tetrode constant (1 K, 101.325 kPa)", -1.1648705214899999, "", 4.7000000000000003e-10},
	{"second radiation constant", 0.014387768775039337, "m K", 0.0},
	{"shielded helion gyromag. ratio", 203789460.78, "s^-1 T^-1", 0.17999999999999999},
	{"shielded helion gyromag. ratio in MHz/T", 32.434100033, "MHz T^-1", 2.7999999999999999e-08},
	{"shielded helion gyromag. ratio over 2 pi", 32.434099660000001, "MHz T^-1", 4.3000000000000001e-07},
	{"shielded helion gyromagn. ratio", 203789460.78, "s^-1 T^-1", 0.17999999999999999},
	{"shielded helion gyromagn. ratio over 2 pi", 32.434101499999997, "MHz T^-1", 2.7999999999999999e-06},
	{"shielded helion mag. mom.", -1.07455311035e-26, "J T^-1", 9.2999999999999996e-36},
	{"shielded helion mag. mom. to Bohr magneton ratio", -0.00115867149457, "", 9.4000000000000003e-13},
	{"shielded helion mag. mom. to nuclear magneton ratio", -2.1274977624, "", 1.6999999999999999e-09},
	{"shielded helion magn. moment", -1.0745530239999999e-26, "J T^-1", 9.3000000000000002e-34},
	{"shielded helion magn. moment to Bohr magneton ratio", -0.0011586714740000001, "", 1.4e-11},
	{"shielded helion magn. moment to nuclear magneton ratio", -2.1274977229999998, "", 2.4999999999999999e-08},
	{"shielded helion to proton mag. mom. ratio", -0.76176657720999996, "", 6.6e-10},
	{"shielded helion to proton magn. moment ratio", -0.76176656200000004, "", 1.2e-08},
	{"shielded helion to shielded proton mag. mom. ratio", -0.76178613340000001, "", 3.1e-09},
	{"shielded helion to shielded proton magn. moment ratio", -0.76178613129999995, "", 3.3000000000000002e-09},
	{"shielded proton gyromag. ratio", 267515319.40000001, "s^-1 T^-1", 1.1000000000000001},
	{"shielded proton gyromag. ratio in MHz/T", 42.576385430000002, "MHz T^-1", 1.6999999999999999e-07},
	{"shielded proton gyromag. ratio over 2 pi", 42.576385070000001, "MHz T^-1", 5.3000000000000001e-07},
	{"shielded proton mag. mom.", 1.410570583e-26, "J T^-1", 5.8000000000000004e-35},
	{"shielded proton mag. mom. to Bohr magneton ratio", 0.0015209931550999999, "", 6.2000000000000002e-12},
	{"shielded proton mag. mom. to nuclear magneton ratio", 2.7927756480000001, "", 1.0999999999999999e-08},
	{"shielded proton magn. moment", 1.4105704700000001e-26, "J T^-1", 1.2e-33},
	{"shielded proton magn. moment to Bohr magneton ratio", 0.0015209931319999999, "", 1.6e-11},
	{"shielded proton magn. moment to nuclear magneton ratio", 2.792775604, "", 2.9999999999999997e-08},
	{"shielding difference of d and p in HD", 1.9877e-08, "", 9.9999999999999998e-13},
	{"shielding difference of t and p in HT", 2.3945e-08, "", 2e-12},
	{"speed of light in vacuum", 299792458.0, "m s^-1", 0.0},
	{"standard acceleration of gravity", 9.8066499999999994, "m s^-2", 0.0},
	{"standard atmosphere", 101325.0, "Pa", 0.0},
	{"standard-state pressure", 100000.0, "Pa", 0.0},
	{"Stefan-Boltzmann constant", 5.6703744191844314e-08, "W m^-2 K^-4", 0.0},
	{"tau Compton wavelength", 6.9777100000000001e-16, "m", 4.7000000000000001e-20},
	{"tau Compton wavelength over 2 pi", 1.11056e-16, "m", 9.9999999999999995e-21},
	{"tau energy equivalent", 1776.8599999999999, "MeV", 0.12},
	{"tau mass", 3.1675399999999999e-27, "kg", 2.0999999999999999e-31},
	{"tau mass energy equivalent", 2.8468399999999998e-10, "J", 1.9000000000000001e-14},
	{"tau mass energy equivalent in MeV", 1776.8199999999999, "MeV", 0.16},
	{"tau mass in u", 1.90754, "u", 0.00012999999999999999},
	{"tau molar mass", 0.0019075400000000001, "kg mol^-1", 1.3e-07},
	{"tau-electron mass ratio", 3477.23, "", 0.23000000000000001},
	{"tau-muon mass ratio", 16.817, "", 0.0011000000000000001},
	{"tau-neutron mass ratio", 1.8911500000000001, "", 0.00012999999999999999},
	{"tau-proton mass ratio", 1.8937600000000001, "", 0.00012999999999999999},
	{"Thomson cross section", 6.6524587051000002e-29, "m^2", 6.1999999999999997e-38},
	{"triton g factor", 5.9579249299999999, "", 1.2e-08},
	{"triton mag. mom.", 1.5046095177999999e-26, "J T^-1", 2.9999999999999999e-35},
	{"triton mag. mom. to Bohr magneton ratio", 0.0016223936648, "", 3.2000000000000001e-12},
	{"triton mag. mom. to nuclear magneton ratio", 2.9789624649999999, "", 5.8999999999999999e-09},
	{"triton mass", 5.0073567511999998e-27, "kg", 1.6000000000000001e-36},
	{"triton mass energy equivalent", 4.5003878118999999e-10, "J", 1.4e-19},
	{"triton mass energy equivalent in MeV", 2808.92113668, "MeV", 8.8000000000000004e-07},
	{"triton mass in u", 3.01550071597, "u", 1e-10},
	{"triton molar mass", 0.0030155007191299999, "kg mol^-1", 9.4000000000000003e-13},
	{"triton relative atomic mass", 3.01550071597, "", 1e-10},
	{"triton to proton mag. mom. ratio", 1.0666399189, "", 2.1000000000000002e-09},
	{"triton-electron mag. mom. ratio", -0.001620514423, "", 2.0999999999999999e-11},
	{"triton-electron mass ratio", 5496.9215355099996, "", 2.1e-07},
	{"triton-neutron mag. mom. ratio", -1.5571855299999999, "", 3.7e-07},
	{"triton-proton mag. mom. ratio", 1.066639908, "", 1e-08},
	{"triton-proton mass ratio", 2.9937170340299999, "", 1e-10},
	{"unified atomic mass unit", 1.6605390689199999e-27, "kg", 5.1999999999999996e-37},
	{"vacuum electric permittivity", 8.8541878188000004e-12, "F m^-1", 1.4e-21},
	{"vacuum mag. permeability", 1.2566370612700001e-06, "N A^-2", 2e-16},
	{"von Klitzing constant", 25812.807459304513, "ohm", 0.0},
	{"W to Z mass ratio", 0.88144999999999996, "", 0.00012999999999999999},
	{"weak mixing angle", 0.22305, "", 0.00023000000000000001},
	{"Wien displacement law constant", 0.0028977685, "m K", 5.1000000000000002e-09},
	{"Wien frequency displacement law constant", 58789257576.468254, "Hz K^-1", 0.0},
	{"Wien wavelength displacement law constant", 0.0028977719551851727, "m K", 0.0},
	{"{220} lattice spacing of silicon", 1.920155714e-10, "m", 3.1999999999999999e-18},
};
//
static const std::size_t gle_codata_constants_count = sizeof(gle_codata_constants) / sizeof(gle_codata_constants[0]);
//
static void gle_set_math_and_physical_constants(bool numerical=true,bool physical=true)
{
	// option to omit inclusion of types if desired -- possible future GLE option
	if(numerical){
		var_findadd_set("MC_CATALAN",boost::math::double_constants::catalan);
		var_findadd_set("MC_DEGREE",boost::math::double_constants::degree);
		var_findadd_set("MC_E",boost::math::double_constants::e);
		var_findadd_set("MC_E_POW_PI",boost::math::double_constants::e_pow_pi);
		var_findadd_set("MC_EULER",boost::math::double_constants::euler);
		var_findadd_set("MC_EXP_MINUS_HALF",boost::math::double_constants::exp_minus_half);
		var_findadd_set("MC_HALF",boost::math::double_constants::half);
		var_findadd_set("MC_HALF_PI",boost::math::double_constants::half_pi);
		var_findadd_set("MC_HALF_ROOT_TWO",boost::math::double_constants::half_root_two);
		var_findadd_set("MC_LN_PHI",boost::math::double_constants::ln_phi);
		var_findadd_set("MC_ONE_DIV_TWO_PI",boost::math::double_constants::one_div_two_pi);
		var_findadd_set("MC_HALF_ROOT_TWO",boost::math::double_constants::half_root_two);
		var_findadd_set("MC_PHI",boost::math::double_constants::phi);
		var_findadd_set("MC_PI",boost::math::double_constants::pi);
		var_findadd_set("MC_QUARTER_PI",boost::math::double_constants::quarter_pi);
		var_findadd_set("MC_RADIAN",boost::math::double_constants::radian);
		var_findadd_set("MC_ROOT_E",boost::math::double_constants::root_e);
		var_findadd_set("MC_ROOT_HALF_PI",boost::math::double_constants::root_half_pi);
		var_findadd_set("MC_ROOT_LN_FOUR",boost::math::double_constants::root_ln_four);
		var_findadd_set("MC_ROOT_PI",boost::math::double_constants::root_pi);
		var_findadd_set("MC_ROOT_THREE",boost::math::double_constants::root_three);
		var_findadd_set("MC_ROOT_TWO",boost::math::double_constants::root_two);
		var_findadd_set("MC_ROOT_TWO_PI",boost::math::double_constants::root_two_pi);
		var_findadd_set("MC_THIRD",boost::math::double_constants::third);
		var_findadd_set("MC_THIRD_PI",boost::math::double_constants::third_pi);
		var_findadd_set("MC_TWO_PI",boost::math::double_constants::two_pi);
		var_findadd_set("MC_TWO_THIRDS",boost::math::double_constants::two_thirds);
		var_findadd_set("MC_ZETA_THREE",boost::math::double_constants::zeta_three);
		var_findadd_set("MC_ZETA_TWO",boost::math::double_constants::zeta_two);
	}
	if(physical){
		var_findadd_set("PC_C",299792458.0);
		var_findadd_set("PC_SPEED_OF_LIGHT",299792458.0);
		var_findadd_set("PC_MU_0",1.25663706127e-06);
		var_findadd_set("PC_EPSILON_0",8.8541878188e-12);
		var_findadd_set("PC_H",6.62607015e-34);
		var_findadd_set("PC_PLANCK",6.62607015e-34);
		var_findadd_set("PC_HBAR",1.0545718176461565e-34);
		var_findadd_set("PC_GRAVITATIONAL_CONSTANT",6.6743e-11);
		var_findadd_set("PC_G",9.80665);
		var_findadd_set("PC_E",1.602176634e-19);
		var_findadd_set("PC_ELEMENTARY_CHARGE",1.602176634e-19);
		var_findadd_set("PC_R",8.31446261815324);
		var_findadd_set("PC_GAS_CONSTANT",8.31446261815324);
		var_findadd_set("PC_ALPHA",0.0072973525643);
		var_findadd_set("PC_FINE_STRUCTURE",0.0072973525643);
		var_findadd_set("PC_N_A",6.02214076e+23);
		var_findadd_set("PC_AVOGADRO",6.02214076e+23);
		var_findadd_set("PC_K",1.380649e-23);
		var_findadd_set("PC_BOLTZMANN",1.380649e-23);
		var_findadd_set("PC_SIGMA",5.6703744191844314e-08);
		var_findadd_set("PC_STEFAN_BOLTZMANN",5.6703744191844314e-08);
		var_findadd_set("PC_WIEN",0.0028977719551851727);
		var_findadd_set("PC_RYDBERG",10973731.568157);
		var_findadd_set("PC_M_E",9.1093837139e-31);
		var_findadd_set("PC_ELECTRON_MASS",9.1093837139e-31);
		var_findadd_set("PC_M_P",1.67262192595e-27);
		var_findadd_set("PC_PROTON_MASS",1.67262192595e-27);
		var_findadd_set("PC_M_N",1.67492750056e-27);
		var_findadd_set("PC_NEUTRON_MASS",1.67492750056e-27);
		var_findadd_set("PC_ZERO_CELSIUS",273.15);
	}
}

//
// -- CODATA lookup
//
static const GLECODATAConstant* gle_codata_find(const std::string& name)
{
	for (std::size_t i = 0; i < gle_codata_constants_count; ++i) {
		if (str_i_equals(gle_codata_constants[i].name, name)) {
			return &gle_codata_constants[i];
		}
	}

	return nullptr;
}

static double gle_codata(const std::string& name)
{
	const GLECODATAConstant* constant = gle_codata_find(name);

	if (constant == nullptr) {
		throw std::runtime_error(
			"unknown CODATA physical constant: \"" + name + "\""
		);
	}

	return constant->value;
}

static std::string gle_codata_units(const std::string& name)
{
	const GLECODATAConstant* constant = gle_codata_find(name);

	if (constant == nullptr) {
		throw std::runtime_error(
			"unknown CODATA physical constant: \"" + name + "\""
		);
	}

	return constant->units;
}

static double gle_codata_uncertainty(const std::string& name)
{
	const GLECODATAConstant* constant = gle_codata_find(name);

	if (constant == nullptr) {
		throw std::runtime_error(
			"unknown CODATA physical constant: \"" + name + "\""
		);
	}

	return constant->uncertainty;
}

