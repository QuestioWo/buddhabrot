//===========================================================================//
///
/// Copyright Jim Carty © 2020
///
/// This file is subject to the terms and conditions defined in file
/// 'LICENSE.txt', which is part of this source code package.
///
//===========================================================================//

#include "ComplexNumber.hpp"

#include <tuple>
#include <vector>

#ifndef SRC_CELL_HPP_
#define SRC_CELL_HPP_

class Cell {
public:
	long double x;
	long double y;

	long double imagex;
	long double imagey;

	long double width;

	long double imagewidth;

	unsigned int counter;

	ComplexNumber complex;

	Cell(long double x, long double y, long double width, long double imagewidth, const std::pair<long double, long double> *realMin, const std::pair<long double, long double> *imageMin);

	static void escape(ComplexNumber *c, std::vector<std::vector<Cell*>> *cells, const std::pair<long double, long double> *min, unsigned int iterations, unsigned int cellsPerRow, unsigned int *maxCount, bool anti);

#if USE_OPENGL
    void render(unsigned int maxCount, unsigned int colourR, unsigned int colourG, unsigned int colourB);
#endif
};

#endif // SRC_CELL_HPP_
