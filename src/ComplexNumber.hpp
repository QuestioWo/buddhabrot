//===========================================================================//
///
/// Copyright Jim Carty © 2020
///
/// This file is subject to the terms and conditions defined in file
/// 'LICENSE.txt', which is part of this source code package.
///
//===========================================================================//

struct ComplexNumber {
	double real;
	double imag;

	ComplexNumber() : real(0), imag(0) {};
	ComplexNumber(double real, double imag) : real(real), imag(imag) {};

	ComplexNumber operator+(const ComplexNumber &c);
	ComplexNumber operator*(const ComplexNumber &c);

	double abs();
};
