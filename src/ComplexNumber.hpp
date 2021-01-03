//===========================================================================//
///
/// Copyright Jim Carty © 2020
///
/// This file is subject to the terms and conditions defined in file
/// 'LICENSE.txt', which is part of this source code package.
///
//===========================================================================//

#ifndef ComplexNumber_hpp
#define ComplexNumber_hpp

struct ComplexNumber {
	long double real;
	long double imag;

	ComplexNumber() : real(0), imag(0) {};
	ComplexNumber(long double real, long double imag) : real(real), imag(imag) {};

	ComplexNumber operator+(const ComplexNumber &c);
	ComplexNumber operator*(const ComplexNumber &c);

	long double abs();
};

#endif // ComplexNumber_hpp
