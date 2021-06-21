//
// -- specialfunctions.cpp
//	  Some simple special functions - might want to look into using boost for these
//

double myfactorial(unsigned int n)
{
	double ret = 1.0;
	while(n > 1){
		ret *= n;
		n--;
	}
	return ret;
}

double myhermite(unsigned int n, double x)
{
	// Hermite Polynomial
	//  Recursion:
	//
	//    H(0,X) = 1,
	//    H(1,X) = 2*X,
	//    H(N,X) = 2*X * H(N-1,X) - 2*(N-1) * H(N-2,X)
	double ret = 1.0;
	if( n == 1){
		ret = 2*x;
	}else if(n > 1){
		ret = 2 * x * myhermite(n-1,x) - 2 * (n-1) * myhermite(n-2,x);
	}
	return ret;
}

double mylaguerre(unsigned int n, double alpha, double x)
{
	//    Generalized Laguerre polynomial
	//		reduces to non generalized when alpha = 0
	//    L(0,alpha,X) = 1,
	//    L(1,alpha,X) = 1 - alpha - x,
	//    L(N,alpha,X) = (2*(N-1)+1+alpha-X) * L(N-1,alpha,X) - (N-1+alpha) * L(N-2,alpha,X))n
	double ret = 1.0;
	if( n == 1){
		ret = 1+alpha-x;
	}else if( n > 1){
		ret = ((2*(n-1) + 1 + alpha - x) * mylaguerre(n-1,alpha,x) - (n-1+alpha) * mylaguerre(n-2,alpha,x))/n;
	}
	return ret;
}
