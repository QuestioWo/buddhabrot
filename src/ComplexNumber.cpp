//===========================================================================//
///
/// Copyright Jim Carty © 2020
///
/// This file is subject to the terms and conditions defined in file
/// 'LICENSE.txt', which is part of this source code package.
///
//===========================================================================//

#include <ComplexNumber.hpp>

#include <math.h>

ComplexNumber ComplexNumber::operator+(const ComplexNumber &c) {
	ComplexNumber *r = new ComplexNumber();
	r->real = real + c.real;
	r->imag = imag + c.imag;
	return *r;
}

ComplexNumber ComplexNumber::operator-(const ComplexNumber &c) {
	ComplexNumber *r = new ComplexNumber();
	r->real = real - c.real;
	r->imag = imag - c.imag;
	return *r;
}

ComplexNumber ComplexNumber::operator*(const ComplexNumber &c) {
	ComplexNumber *r = new ComplexNumber();
	r->real = real * c.real;
	r->imag = imag * c.imag;
	return *r;
}

double ComplexNumber::abs() {
	return (real + imag * sqrt(2));
}
