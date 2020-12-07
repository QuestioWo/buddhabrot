//===========================================================================//
///
/// Copyright Jim Carty © 2020
///
/// This file is subject to the terms and conditions defined in file
/// 'LICENSE.txt', which is part of this source code package.
///
//===========================================================================//

#include <Cell.hpp>

#include <iostream>
#include <vector>

Cell::Cell(double x, double y, double width, double imagewidth,
		std::pair<double, double> min, std::pair<double, double> max) :
		x(x), y(y), width(width), imagewidth(imagewidth) {
	imagex = (x + min.first) * width;
	imagey = (y + min.second) * width;

	counter = 0;

	complex = ComplexNumber(x, y);
}

void Cell::escape(Cell *cell,
		std::vector<std::vector<Cell*>> *cells, std::pair<double, double> min,
		unsigned int iterations, unsigned int cellsPerRow) {
	ComplexNumber c = cell->complex;
	ComplexNumber z = ComplexNumber();
	std::vector<Cell*> visited;
	visited.clear();

	for (unsigned int i = 0; i < iterations; ++i) {
		z = z * z + c;

		int visitedx = int((z.real - min.first) / cell->width);
		int visitedy = int((z.imag - min.second) / cell->width);

		if (visitedx < (cellsPerRow - 1) && visitedy < (cellsPerRow - 1)
				&& visitedx > 0 && visitedy > 0)
			visited.push_back(cells->at(visitedx)[visitedy]);
		else
			break;
	}

	if (z.abs() > 2.0) {
		for (Cell *escapedbox : visited) {
			int visitedx = int(
					(escapedbox->complex.real - min.first) / cell->width);
			int visitedy = int(
					(escapedbox->complex.imag - min.second) / cell->width);

			cells->at(visitedx)[visitedy]->counter++;
		}
	}
}
