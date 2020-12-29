//===========================================================================//
///
/// Copyright Jim Carty © 2020
///
/// This file is subject to the terms and conditions defined in file
/// 'LICENSE.txt', which is part of this source code package.
///
//===========================================================================//

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

kernel void escape(global Real *cells, const Real minx, const Real miny, const Real cellRealWidth, volatile global unsigned int *output) {
    private unsigned int i = get_global_id(0);
    
    if (i <= CELLS_PER_ROW * CELLS_PER_ROW) {
        private int x = i / CELLS_PER_ROW;
        private int y = i % CELLS_PER_ROW;

        private Real zreal = 0, zimag = 0;

        private Real creal = cells[x * CELLS_PER_ROW * 3 + y * 3 + 0],
            cimag = cells[x * CELLS_PER_ROW * 3 + y * 3 + 1];

        private unsigned int visited[ITERATIONS * 2];
        private unsigned int realIterations = 0;

        for (private unsigned int i = 0; i <= ITERATIONS; ++i) {
            // z = z^2 + c
            private Real oldReal = zreal;
            zreal = zreal * zreal - zimag * zimag;
            zimag = 2 * oldReal * zimag;
            
            zreal = zreal + creal;
            zimag = zimag + cimag;
            // advance ^^^^
            
            private int visitedx = floor((zreal - minx) / cellRealWidth);
            private int visitedy = floor((zimag - miny) / cellRealWidth);

            private int oneLess = CELLS_PER_ROW - 1;
            
            if (visitedx < oneLess && visitedy < oneLess && visitedx >= 0 && visitedy >= 0 && i != 0) {
                visited[realIterations * 2 + 0] = (unsigned int)visitedx;
                visited[realIterations * 2 + 1] = (unsigned int)visitedy;
                ++realIterations;
            } else if (i != 0)
                break;
        }

        private Real abs = sqrt(zreal * zreal + zimag * zimag);
        
        if (ANTI == 1 ? abs < 2.0 : abs > 2.0) { // regular buddhabrot means that the point escapes, thus > 2.0 for !anti
            for (private unsigned int i = 0; i < realIterations; ++i)
                atomic_inc(&output[visited[i * 2 + 0] * CELLS_PER_ROW + visited[i * 2 + 1]]);
        }
    }
}