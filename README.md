## Renders
Render produced for 2001 x 2001 pixels at 2500 iterations :

![Buddhabrot](https://github.com/QuestioWo/buddhabrot/blob/main/assets/2001x2500.png?raw=true)

The command that can be used for getting this result is `./build/buddhabrot -o -w 2001 -i 2500 -s 2001x2500 -r 128`

This program also supports [anti-buddhabrot](https://en.wikipedia.org/wiki/Buddhabrot#Nuances) rendering. The below image is using the same 2001x2500 pixels and iterations :

![Anti-buddhabrot](https://github.com/QuestioWo/buddhabrot/blob/main/assets/2001x2500anti.png?raw=true)

The command that can be used for getting this result is `./build/buddhabrot -o -w 2001 -i 2500 -s 2001x2500anti -a -r 128`

## Running

### Prerequisites
* OpenGL
* GLUT
* libpng

### Building & Usgae

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

## Author

This buddhabrot rendering program has been and is being written by [Jim Carty](https://questiowo.github.io). Email: cartyjim1@gmail.com

## License

This project is licensed under the terms of the MIT license. See `LICENSE.txt` file for the full license.
