![Screenshot](/img/screenshot.png)

_________,

__________________________.

Tell no one.
Come alone.
Bring your medkit.

- Kate

# Overview

WEAL is a traditional roguelike about saving lives. Delve into a ruined
research facility, provide medical assistance to the injured, and solve
the mystery of why you have been brought here. Be careful, though: You
are far from alone...

Although this game is a traditional roguelike in many senses, I'm taking every
step I can to make it palatable for a modern audience:
- The game's control scheme is very simple, although a wealth of keybinds exist
  for us grognards as well.
- Mouse input is supported.
- The game can be played in ASCII or graphical tiles mode, depending on your
  preference. Save files should be completely compatible from version to version.
- Game data is stored in human-readable XML files for easy perusal and
  modification.

# Missing Features
Currently Missing:
- Mouse input
- Tutorial
- Gameplay
- Everything

# FAQ

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

## How Do I Play?
Select "Basic Training" from the main menu to play through a quick tutorial.
Alternatively, press the question mark key at any time to view the manual.
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