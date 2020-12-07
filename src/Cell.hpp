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

	Cell(double x, double y, double width, double imagewidth,
			std::pair<double, double> min, std::pair<double, double> max);

	static void escape(Cell *cell,
			std::vector<std::vector<Cell*>> *cells,
			std::pair<double, double> min, unsigned int iterations,
			unsigned int cellsPerRow);

};

#endif // SRC_CELL_HPP_
