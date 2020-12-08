//===========================================================================//
///
/// Copyright Jim Carty © 2020
///
/// This file is subject to the terms and conditions defined in file
/// 'LICENSE.txt', which is part of this source code package.
///
//===========================================================================//

#include "ComplexNumber.hpp"

#include <math.h>

ComplexNumber ComplexNumber::operator+(const ComplexNumber &c) {
    real = real + c.real;
    imag = imag + c.imag;
	return *this;
}

ComplexNumber ComplexNumber::operator*(const ComplexNumber &c) {
    double oldReal = real;
    real = real * c.real - imag * c.imag;
    imag = 2 * oldReal * imag;
	return *this;
}

double ComplexNumber::abs() {
	return hypot(real, imag);
}
