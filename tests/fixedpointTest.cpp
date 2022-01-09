//===========================================================================//
///
/// Copyright Jim Carty © 2021
///
/// This file is subject to the terms and conditions defined in file
/// 'LICENSE.txt', which is part of this source code package.
///
//===========================================================================//

#include "../src/fixedpoint.hpp"

#include <gtest/gtest.h>

namespace {

TEST(fixedpointTest, addition8BitTest) {
	fixedpoint a = fixedPointCreate(1);
	fixedpoint b = fixedPointCreate(1);
	*b = 10;
	*a = 5;

	ASSERT_NO_THROW(fixedPointAdd(1, a, b));

	fixedpoint result = fixedPointAdd(1, a, b);

	ASSERT_EQ(15, *result);
}

TEST(fixedpointTest, addition16BitTest) {
	fixedpoint a = fixedPointCreate(2);
	fixedpoint b = fixedPointCreate(2);
	*(b + 1) = 255;
	*b = 6;
	*a = 128;

	ASSERT_NO_THROW(fixedPointAdd(1, a, b));

	fixedpoint result = fixedPointAdd(1, a, b);

	ASSERT_EQ(0b01111111, *(result + 1));
	ASSERT_EQ(0b00000111, *result);
}

} // close anonymous namespace