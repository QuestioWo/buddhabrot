[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/QuestioWo/buddhabrot.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/QuestioWo/buddhabrot/context:cpp)

## Renders
Render produced for 2001 x 2001 pixels at 2500 iterations :

![Buddhabrot](https://github.com/QuestioWo/buddhabrot/blob/main/assets/2001x2500.png?raw=true)

The command that can be used for getting this result is `./build/buddhabrot -o -w 2001 -i 2500 -s 2001x2500 -c 128,0,255`. Computation takes around 3 seconds using the `-o` flag to run the command on the GPU, and around 53 seconds for the command without the flag, so running on the CPU.

This program also supports [anti-buddhabrot](https://en.wikipedia.org/wiki/Buddhabrot#Nuances) rendering. The below image is using the same 2001x2500 pixels and iterations :

![Anti-buddhabrot](https://github.com/QuestioWo/buddhabrot/blob/main/assets/2001x2500anti.png?raw=true)

The command that can be used for getting this result is `./build/buddhabrot -o -w 2001 -i 2500 -s 2001x2500anti -a -c 128,0,255`. Computation takes around 6 seconds using the `-o` flag to run the command on the GPU, and around 53 seconds for the command without the flag, so running on the CPU.

## Running

### Prerequisites
* OpenGL (optional with `-DUSE_OPENGL=OFF` argument for `cmake`)
* GLUT (optional with `-DUSE_OPENGL=OFF` argument for `cmake`)
* OpenCV
* OpenCL

### Building & Usage

```bash
git clone https://github.com/QuestioWo/buddhabrot.git
mkdir -p buddhabrot/build && cd buddhabrot/build
cmake ..
make
cd ../
./build/buddhabrot
```

A OpenGL window titled `Buddhabrot` will appear after the [buddhabrot](https://en.wikipedia.org/wiki/Buddhabrot) has been calculated. Examples of the program running can be found above

Exiting the program can be done by pressing `Ctrl + C` in the same terminal window

If the GPU connected supports double precision, either through the `cl_khr_fp64` or `cl_amd_fp64` additions to the OpenCL install, then using `cmake` with the `-DOPENCL_DOUBLE_PRECISION=ON` argument will enable the use of double precision floating point numbers when running the program with the `-o` argument. If you are unsure of if your GPU supports double precision, use the `clinfo` command, which should detail whether your GPU does or not.

### Iteration Groups

For running with the `-o` argument, on the GPU, to avoid the GPU hanging, the program runs groups of iterations across all the cells. This is because with larger numbers of pixels and iterations, the computation time to run all iterations maybe too great to not hang the GPU

To find the maximum value, and thus the most effiecient method of grouping, an equation is employed that was calculated from maximum values obtained from experimentation

![Graph of maximum values](https://github.com/QuestioWo/buddhabrot/blob/main/assets/batch_calculation.png?raw=true)

To override the value calculated with this equation, namely to run without or with far more lax groupings, the `-m ITERATION_MAX` argument can be used when running the program

### Google Colab

This project also successfully build and runs on Google Colab, Google's free cloud computing service. An example of this can be found [here](https://colab.research.google.com/drive/1cejpU7ADF30m_PSY2Mdh1M0MBTgHYzyT?usp=sharing) and its results can be found [here](https://drive.google.com/drive/folders/1q31810a88D1tNCGpoFf338rNu6K4gIFS?usp=sharing)

## Gallery

A collection of results of the program that were not quite buddhabrots, but still interesting can be found [here](https://drive.google.com/drive/folders/1y0yev1y6pWNXeVT7l7yXxJzKpcF0Bow-?usp=sharing)

## Author

This buddhabrot rendering program has been and is being written by [Jim Carty](https://questiowo.github.io). Email: cartyjim1@gmail.com

## License

This project is licensed under the terms of the MIT license. See `LICENSE.txt` file for the full license.
