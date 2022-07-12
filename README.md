![Screenshot](/img/screenshot.png)
# Overview

WEAL is a traditional roguelike. It's still pretty early in development, so please don't
judge it too harshly yet.

# FAQ

## Is this Playable Yet?

While both the win and lose state are reachable, the game is still very far from what
I would consider playable.

## Can My Computer Run It?
This is a console-based roguelike written in plain C. A potato can probably run this game.

In all seriousness, your computer can almost certainly run this. At worst, using
autoexplore might eat up some RAM in a low memory environment.

## Why C?

This is a traditional roguelike, which follows the Berlin Interpretation
fairly closely. Why not make it even more traditional by writing it in pure C?

In all honesty, though, I simply like C. I love the lack of overhead and the
granular control.

## Are save files compatible across computers?

Short answer: No.

Long answer: It depends. Save files are written in binary with fwrite(). 
This means that the save file architecture depends on one's platform,
operating system, the compiler that the binary was compiled with, and a host
of other factors. It's easiest to assume that a save file made on one computer
will not be compatible with a save file made on another.

In the future, I would like to refactor save files to be in human-readable
json, but that's a long way off.

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

# Influences

WEAL was influenced by numerous games, the most prominent of which appear here:

* [NetHack](https://github.com/nethack/nethack)

* [Dungeon Crawl Stone Soup](https://github.com/crawl/crawl)

* [The Slimy Lichmummy](http://www.happyponyland.net/the-slimy-lichmummy)

* [Shin Megami Tensei](https://en.wikipedia.org/wiki/Megami_Tensei)

* [Sil-Q](https://github.com/sil-quirk/sil-q)

