//
// -- specialfunctions.h - use boost for all special functions
//
#include <boost/math/special_functions/factorials.hpp>
#include <boost/math/special_functions/legendre.hpp>
#include <boost/math/special_functions/laguerre.hpp>
#include <boost/math/special_functions/hermite.hpp>
#include <boost/math/special_functions/spherical_harmonic.hpp>
#include <boost/math/special_functions/erf.hpp>
#include <boost/math/special_functions/gamma.hpp>
#include <boost/math/special_functions/bessel.hpp>
#include <boost/math/special_functions/airy.hpp>
#include <boost/math/special_functions/chebyshev.hpp>

double myfactorial(unsigned int n);
double myhermite(unsigned int n, double x);
double mylaguerre(unsigned int n, double alpha, double x);

