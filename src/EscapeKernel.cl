//===========================================================================//
///
/// Copyright Jim Carty © 2020
///
/// This file is subject to the terms and conditions defined in file
/// 'LICENSE.txt', which is part of this source code package.
///
//===========================================================================//

#ifndef CELLS_CURRENT
    #define CELLS_CURRENT 0lu
#endif

#ifndef CELLS_PER_ROW
    #define CELLS_PER_ROW 0lu
#endif

#if defined(cl_khr_fp64)
    #pragma OPENCL EXTENSION cl_khr_fp64 : enable
    #define DOUBLE_SUPPORT_AVAILABLE
#elif defined(cl_amd_fp64)
    #pragma OPENCL EXTENSION cl_amd_fp64 : enable
    #define DOUBLE_SUPPORT_AVAILABLE
#endif

#if defined(DOUBLE_SUPPORT_AVAILABLE)
    // double
    typedef double Real;
#else
    // float
    typedef float Real;
#endif


kernel void escape(global Real *originalCells, global Real *currentCells, const Real minx, const Real miny, const Real cellRealWidth, const unsigned int iterationsCurrent, volatile global Real *interimResults, volatile global unsigned int *counts) {

    private unsigned int count = get_global_id(0);

    if (count <= CELLS_CURRENT) {

        private int x = count / CELLS_PER_ROW;
        private int y = count % CELLS_PER_ROW;

#if CHECK
        private Real zreal = currentCells[x * CELLS_PER_ROW * 2 + y * 2 + 0],
            zimag = currentCells[x * CELLS_PER_ROW * 2 + y * 2 + 1];

        private Real creal = originalCells[x * CELLS_PER_ROW * 2 + y * 2 + 0],
            cimag = originalCells[x * CELLS_PER_ROW * 2 + y * 2 + 1];

#else
        private Real zreal = currentCells[count * 2 + 0],
            zimag = currentCells[count * 2 + 1];

        private Real creal = originalCells[count * 2 + 0],
            cimag = originalCells[count * 2 + 1];

#endif

        private int oneLess = CELLS_PER_ROW - 1;

        for (private unsigned int i = 0; i < iterationsCurrent; ++i) {
            if (!isinf(zreal) && !isinf(zimag) && zreal != 0 && zimag != 0) {
                // if a cell is invalid then it may be passed from the previous group and added updated by the advance section of the loop before the loop is broken. This means that the check for validity has to occur before the advancing of the complex number in order to have valid results and counts
                private int visitedx = floor((zreal - minx) / cellRealWidth);
                private int visitedy = floor((zimag - miny) / cellRealWidth);

                if (visitedx > oneLess || visitedy > oneLess || visitedx < 0 || visitedy < 0)
                    break;

#if !CHECK
                atomic_inc(&counts[visitedx * CELLS_PER_ROW + visitedy]);
#endif

                // z = z^2 + c
                private Real oldReal = zreal;
                zreal = zreal * zreal - zimag * zimag;
                zimag = 2 * oldReal * zimag;

                zreal = zreal + creal;
                zimag = zimag + cimag;
                // advance ^^^^
            } else if (i == 0 && zreal == 0 && zimag == 0) {
                // only runs on first group of iterations
                private Real oldReal = zreal;
                zreal = zreal * zreal - zimag * zimag;
                zimag = 2 * oldReal * zimag;

                zreal = zreal + creal;
                zimag = zimag + cimag;
            } else
                break;
        }

        private Real abs = hypot(zreal, zimag);

        interimResults[x * CELLS_PER_ROW * 2 + y * 2 + 0] = zreal;
        interimResults[x * CELLS_PER_ROW * 2 + y * 2 + 1] = zimag;

#if CHECK
        if (ANTI ? abs < 2.0 : abs > 2.0) // regular buddhabrot means that the point escapes, thus > 2.0 for !anti
            atomic_inc(&counts[x * CELLS_PER_ROW + y]);
#endif
    }
}
