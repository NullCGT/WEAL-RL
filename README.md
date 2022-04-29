![Screenshot](/img/screenshot.png)

Tell no one.
Come alone.
Bring your medkit.

# Overview

WEAL is a traditional roguelike about saving lives. Delve into a ruined
research facility, provide medical assistance to the injured, and solve
the mystery of why you have been brought here. Be careful, though: You
are far from alone...

# FAQ

## Is this Playable?
Not yet. This game is very much a work in progress, and is missing many key features,
such as enemy AI, death, and an inventory system. Try it at your own risk.

## Which Port Should I Play?
- The OpenGL port is intended for players who enjoy playing roguelikes with
  graphical tilesets, or are unfamiliar with the use of their machine's console.
- The notcurses port is intended for players who enjoy playing roguelikes in the console,
  and have a capable terminal emulator that supports arbitrary numbers of colors.
- The ncurses port is intended for players who enjoy playing roguelikes in the console,
  but have a terminal that is not able to support notcurses. It is also intended for
  remotely-hosted games.

## Can My Computer Run It?
This is a console-based roguelike written in plain C. A potato can probably run this game.

In all seriousness, your computer can almost certainly run this, especially if you
are playing the ncurses version of the game. If you're reall, really hurting for
computational power, it might take a few seconds to generate level maps.

# Are save files compatible across computers?

Short answer: No.

Long answer: It depends. The save file architecture depends on one's platform, operating
system, the compiler that the binary was compiled with, and more.

# Building from Source
In order to compile WEAL, you will need the following:
- CMake
- ncurses
- notcurses
- libxml2

The compilation steps are as follows:
```
cmake -S . -B build
cmake --build build
```

Game binaries will be created in the build/bin folder.

# Credits
- [Kenney 1-Bit Tiles Pack](https://www.kenney.nl/assets/bit-pack).
- [Wave Function Collapse algorithm, by mxgmn](https://github.com/mxgmn/WaveFunctionCollapse).
- [Wave Function Collapse C library, by krychu](https://github.com/krychu/wfc).
- [BearLibTerminal, by  tommyettinger](https://github.com/tommyettinger/BearLibTerminal)
