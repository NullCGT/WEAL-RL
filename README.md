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

## Can My Computer Run It?
This is a console-based roguelike written in plain C. A potato can probably run this game.

In all seriousness, your computer can almost certainly run this. At worst, using
autoexplore might eat up some RAM in a low memory environment.

# Are save files compatible across computers?

Short answer: No.

Long answer: It depends. Save files are written and read using fread() and
fwrite(). This means that the save file architecture depends on one's platform,
operating system, the compiler that the binary was compiled with, and a host
of other factors. It's easiest to assume that a save file made on one computer
will not be compatible with a save file made on another.

# Building from Source
In order to compile WEAL, you will need the following:
- CMake
- cJSON
- ncurses

To compile the game for play and installation, run the following commands:
```
mkdir build
cmake --preset=release
cmake --build build
cd build
cpack
```

In order to compile the game for testing locally, run the following commands
instead:
```
mkdir build
cmake --preset=dev
cmake --build build
```

