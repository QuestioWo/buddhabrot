# buddhabrot
C++ [buddhabrot](https://en.wikipedia.org/wiki/Buddhabrot) fractal rendering program that uses the OpenGL library

## Running

### Prerequisites
* OpenGL
* GLU
* GLUT

### Building & Running

```bash
mkdir -p build && cd build
cmake ..
make
./buddhabrot
```

## Result
A OpenGL window titled `Buddhabrot` will appear after the buddhabrot has been calculated. For 801 pixels x 801 pixels at 2500 iterations, the below window will appear:

![Buddhabrot](https://raw.githubusercontent.com/QuestioWo/buddhabrot/main/assets/801x2500.png)

## Slight adjustments
Within the `src/Cell.cpp` file, if a switch is made to the line containing

```c++
if (z.abs() > 2.0) {
```

to be

```c++
if (z.abs() < 2.0) {
```

then an [anti-buddhabrot](https://en.wikipedia.org/wiki/Buddhabrot#Nuances) will be rendered