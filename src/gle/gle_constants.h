//
// -- gle_constants.h
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
		var_findadd_set("mc_catalan",boost::math::double_constants::catalan);
		var_findadd_set("mc_degree",boost::math::double_constants::degree);
		var_findadd_set("mc_e",boost::math::double_constants::e);
		var_findadd_set("mc_e_pow_pi",boost::math::double_constants::e_pow_pi);
		var_findadd_set("mc_euler",boost::math::double_constants::euler);
		var_findadd_set("mc_exp_minus_half",boost::math::double_constants::exp_minus_half);
		var_findadd_set("mc_half",boost::math::double_constants::half);
		var_findadd_set("mc_half_pi",boost::math::double_constants::half_pi);
		var_findadd_set("mc_half_root_two",boost::math::double_constants::half_root_two);
		var_findadd_set("mc_ln_phi",boost::math::double_constants::ln_phi);
		var_findadd_set("mc_one_div_two_pi",boost::math::double_constants::one_div_two_pi);
		var_findadd_set("mc_half_root_two",boost::math::double_constants::half_root_two);
		var_findadd_set("mc_phi",boost::math::double_constants::phi);
		var_findadd_set("mc_pi",boost::math::double_constants::pi);
		var_findadd_set("mc_quarter_pi",boost::math::double_constants::quarter_pi);
		var_findadd_set("mc_radian",boost::math::double_constants::radian);
		var_findadd_set("mc_root_e",boost::math::double_constants::root_e);
		var_findadd_set("mc_root_half_pi",boost::math::double_constants::root_half_pi);
		var_findadd_set("mc_root_ln_four",boost::math::double_constants::root_ln_four);
		var_findadd_set("mc_root_pi",boost::math::double_constants::root_pi);
		var_findadd_set("mc_root_three",boost::math::double_constants::root_three);
		var_findadd_set("mc_root_two",boost::math::double_constants::root_two);
		var_findadd_set("mc_root_two_pi",boost::math::double_constants::root_two_pi);
		var_findadd_set("mc_third",boost::math::double_constants::third);
		var_findadd_set("mc_third_pi",boost::math::double_constants::third_pi);
		var_findadd_set("mc_two_pi",boost::math::double_constants::two_pi);
		var_findadd_set("mc_two_thirds",boost::math::double_constants::two_thirds);
		var_findadd_set("mc_zeta_three",boost::math::double_constants::zeta_three);
		var_findadd_set("mc_zeta_two",boost::math::double_constants::zeta_two);
	}
	if(physical){
		var_findadd_set("pc_c",299792458.0);
		var_findadd_set("pc_speed_of_light",299792458.0);
		var_findadd_set("pc_mu_0",1.25663706127e-06);
		var_findadd_set("pc_epsilon_0",8.8541878188e-12);
		var_findadd_set("pc_h",6.62607015e-34);
		var_findadd_set("pc_Planck",6.62607015e-34);
		var_findadd_set("pc_hbar",1.0545718176461565e-34);
		var_findadd_set("pc_G",6.6743e-11);
		var_findadd_set("pc_gravitational_constant",6.6743e-11);
		var_findadd_set("pc_g",9.80665);
		var_findadd_set("pc_e",1.602176634e-19);
		var_findadd_set("pc_elementary_charge",1.602176634e-19);
		var_findadd_set("pc_R",8.31446261815324);
		var_findadd_set("pc_gas_constant",8.31446261815324);
		var_findadd_set("pc_alpha",0.0072973525643);
		var_findadd_set("pc_fine_structure",0.0072973525643);
		var_findadd_set("pc_N_A",6.02214076e+23);
		var_findadd_set("pc_Avogadro",6.02214076e+23);
		var_findadd_set("pc_k",1.380649e-23);
		var_findadd_set("pc_Boltzmann",1.380649e-23);
		var_findadd_set("pc_sigma",5.6703744191844314e-08);
		var_findadd_set("pc_Stefan_Boltzmann",5.6703744191844314e-08);
		var_findadd_set("pc_Wien",0.0028977719551851727);
		var_findadd_set("pc_Rydberg",10973731.568157);
		var_findadd_set("pc_m_e",9.1093837139e-31);
		var_findadd_set("pc_electron_mass",9.1093837139e-31);
		var_findadd_set("pc_m_p",1.67262192595e-27);
		var_findadd_set("pc_proton_mass",1.67262192595e-27);
		var_findadd_set("pc_m_n",1.67492750056e-27);
		var_findadd_set("pc_neutron_mass",1.67492750056e-27);
		var_findadd_set("pc_zero_Celsius",273.15);
	}
}
