//===========================================================================//
///
/// Copyright Jim Carty © 2020
///
/// This file is subject to the terms and conditions defined in file
/// 'LICENSE.txt', which is part of this source code package.
///
//===========================================================================//

#ifdef cl_amd_fp64
    #pragma OPENCL EXTENSION cl_amd_fp64 : enable
#else
    #pragma OPENCL EXTENSION cl_khr_fp64 : enable
#endif

__kernel void escape(__global double *cells, const double minx, const double miny, const unsigned int iterations, const unsigned int cellsPerRow, const double cellRealWidth, const unsigned int antiPassed, __global double *output) {
    __private unsigned int count = get_global_id(0);
    
    __private bool anti = antiPassed == 1 ? true : false;
    
    __private double zreal = 0, zimag = 0;
    
    __private unsigned int x = floor((float)(count / cellsPerRow));
    __private unsigned int y = count % cellsPerRow;
    
    __private double creal = cells[x * cellsPerRow * 3 + y * 3 + 0],
        cimag = cells[x * cellsPerRow * 3 + y * 3 + 1];
    
    __private int visited[iterations * 2];
    __private unsigned int realIterations = 0;

    for (__private unsigned int i = 0; i < iterations; ++i) {
        // z = z^2 + c
        __private double oldReal = zreal;
        zreal = zreal * zreal - zimag * zimag;
        zimag = 2 * oldReal * zimag;
        
        zreal = zreal + creal;
        zimag = zimag + cimag;
        // advance ^^^^
        
        __private int visitedx = floor((float)((zreal - minx) / cellRealWidth));
        __private int visitedy = floor((float)((zimag - miny) / cellRealWidth));

        __private int oneLess = cellsPerRow - 1;
        
        if (visitedx < oneLess && visitedy < oneLess && visitedx >= 0 && visitedy >= 0 && i != 0) {
            visited[realIterations * 2 + 0] = visitedx;
            visited[realIterations * 2 + 1] = visitedy;
            ++realIterations;
        } else if (i != 0)
            break;
    }

    __private double abs = sqrt((float)(zreal * zreal + zimag * zimag));
    
    if (anti ? abs < 2.0 : abs > 2.0) { // regular buddhabrot means that the point escapes, thus > 2.0 for !anti
        for (__private unsigned int i = 0; i < realIterations; ++i)
            ++output[visited[i * 2 + 0] * cellsPerRow + visited[i * 2 + 1]];
    }
}
