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
Barely. Try it at your own risk.

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
- cJSON
- ncurses

The compilation steps are as follows:
```
cmake --preset=release
cmake --build build
```

Game binaries will be created in the build/bin folder.
