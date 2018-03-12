#include <complex.h>
#include <math.h>

// http://www.mathpropress.com/stan/bibliography/complexSquareRoot.pdf
double complex csqrt(double complex z)
{
	// 1 div sqrt(2)
	static double sqrt2 = 0.7071067811865475;
	double a = creal(z);
	double b = cimag(z);
	double sqrt_a_b = sqrt(a*a + b*b);
	
	double realpart = sqrt(a + sqrt_a_b) * sqrt2;
	double imagpart = sqrt(sqrt_a_b - a) / sqrt2;
	if (b < 0)
		imagpart = -imagpart;

	return realpart + imagpart * I;
}
