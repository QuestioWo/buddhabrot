C++ [buddhabrot](https://en.wikipedia.org/wiki/Buddhabrot) fractal rendering program that uses the OpenGL library

## Running

### Prerequisites
* OpenGL
* GLU
* GLUT

### Building & Usgae

```bash
mkdir -p build && cd build
cmake ..
make
./buddhabrot
```

## Result
A OpenGL window titled `Buddhabrot` will appear after the buddhabrot has been calculated. For 801 pixels x 801 pixels at 2500 iterations, the below window will appear:

![Buddhabrot](https://raw.githubusercontent.com/QuestioWo/buddhabrot/main/assets/801x2500.png)

The command that can be used for getting this result is `./buddhabrot -w 801 -i 2500`

This program also supports [anti-buddhabrot](https://en.wikipedia.org/wiki/Buddhabrot#Nuances) rendering. The below render is using the 801x2500 pixels and iterations :

![Anti-buddhabrot](https://raw.githubusercontent.com/QuestioWo/buddhabrot/main/assets/801x2500anti.png)

The command that can be used for getting this result is `./buddhabrot -w 801 -i 2500 -a`
