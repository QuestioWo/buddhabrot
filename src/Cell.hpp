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

#ifndef SRC_CELL_HPP_
#define SRC_CELL_HPP_

struct Cell {
	double x;
	double y;

	double imagex;
	double imagey;

	double width;

	double imagewidth;

	unsigned int counter;

	ComplexNumber complex;

	Cell(double x, double y, double width, double imagewidth, std::pair<double, double> *realMin, std::pair<double, double> *imageMin);

	static void escape(ComplexNumber *c, std::vector<std::vector<Cell*>> *cells, std::pair<double, double> *min, unsigned int iterations, unsigned int cellsPerRow, unsigned int *maxCount, bool anti);

    void render(unsigned int maxCount, unsigned int colourR, unsigned int colourG, unsigned int colourB);
};

#endif // SRC_CELL_HPP_
