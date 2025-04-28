//
// -- gle-constants.h
//
//  automatically generated from scripts/constants.py
//  re-run manually and copy file to src/gle to modify
//  uses boost::math for numerical constants
//  uses scipy.constants for physical constants
//  mathematical constants start with mc_
//  physical constants start with pc_
//
//  gle variables are case in-sensitive
//
#include <boost/math/constants/constants.hpp>
//
void gle_set_math_and_physical_constants(bool numerical=true,bool physical=true)
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
		var_findadd_set("PC_G",6.6743e-11);
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
