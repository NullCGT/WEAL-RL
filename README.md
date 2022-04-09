![Screenshot](/img/screenshot.png)

This is a README :)

# Overview

WEAL is a traditional roguelike.

# Features
- Nothing yet, to be honest.

# FAQ

## Which Port Should I Play?
- The opengl port is intended for players who enjoy playing roguelikes with
  graphical tilesets, or are unfamiliar with the use of their machine's console.
- The notcurses port is intended for players who enjoy playing roguelikes in the console,
  and have a capable terminal emulator that supports arbitrary numbers of colors.
- The ncurses port is intended for players who enjoy playing roguelikes in the console,
  but have a terminal that is not able to support notcurses. It is also intended for
  remotely-hosted games.

## Can I Create a Fork or Variant of WEAL?
WEAL is designed to be easily modified. Forks are encouraged, as long as they follow the
terms laid out in the license.

# Building from Source
In order to compile WEAL, you will need the following libraries:
- ncurses
- notcurses
- libxml2

The compilation steps are as follows:
'''
mkdir build
cd build
cmake ..
make
'''

Game binaries will be created in the build/bin folder.

# Credits
- [Wave Function Collapse algorithm, by mxgmn](https://github.com/mxgmn/WaveFunctionCollapse).
- [Wave Function Collapse C library, by krychu](https://github.com/krychu/wfc).
- [BearLibTerminal, by  tommyettinger](https://github.com/tommyettinger/BearLibTerminal)