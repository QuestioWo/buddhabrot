//===========================================================================//
///
/// Copyright Jim Carty © 2021
///
/// This file is subject to the terms and conditions defined in file
/// 'LICENSE.txt', which is part of this source code package.
///
//===========================================================================//

#include <cstddef>

typedef unsigned char* fixedpoint;

fixedpoint fixedPointCreate(size_t bytes) {
	return new unsigned char[bytes];
}

fixedpoint fixedPointAdd(size_t bytes, fixedpoint x, fixedpoint y) {
	int currentOffset = bytes;
	unsigned char carry = 0;

	fixedpoint result = new unsigned char[bytes];

	while (currentOffset >= 0) {
		*(result + currentOffset) = (unsigned char)*(x + currentOffset) + (unsigned char)*(y + currentOffset) + carry;

		if ((unsigned char)*(result + currentOffset) < (unsigned char)*(x + currentOffset))
			carry = 1;
		else
			carry = 0;

		currentOffset -= 1;
	}

    return result;
}