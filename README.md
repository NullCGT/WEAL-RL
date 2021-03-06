# Weal

![Screenshot](/img/screenshot.png)
## Overview

WEAL is a traditional roguelike. It's still pretty early in development, so please don't
judge it too harshly yet.

## Command Line Options
| Syntax      | Example | Purpose           |
| ------------|---------|-------------------|
| -u\[User\]  | -uBob   | Specify the user.
| -D          | -D      | Enter debug mode. In debug mode, the high score list is disabled, but the user has access to special debug functionality.
| -X          | -X      | Enter explore mode. In explore mode, the high scorel list is disabled, but the user can elect not to die.

## Building from Source
WEAL is built using CMake.

In order to compile the game for local testing, run the following commands:
```
mkdir build
cmake --preset=dev
cmake --build build
```
The game binary and necessary data files will appear in the newly-created build
directory.

## FAQ

### Is this Playable Yet?

While both the win and lose state are reachable, the game is still very far from what
I would consider playable.

### Can My Computer Run It?
This is a console-based roguelike written in plain C. A potato can probably run this game.

In all seriousness, your computer can almost certainly run this. At worst, using
autoexplore might eat up some RAM in a low memory environment.

### Why C?

This is a traditional roguelike, and follows the Berlin Interpretation
fairly closely. Why not make it even more traditional by writing it in pure C?

In all honesty, though, I simply like C.

### Are save files compatible across computers?

Short answer: No.

Long answer: It depends. Save files are written in binary with fwrite(). 
This means that the save file architecture depends on one's platform,
operating system, the compiler that the binary was compiled with, and a host
of other factors. It's easiest to assume that a save file made on one computer
will not be compatible with a save file made on another.

In the future, I would like to refactor save files to be in human-readable
json, but that's a long way off.

## Influences

WEAL was influenced by numerous games, the most prominent of which appear here:

* [NetHack](https://github.com/nethack/nethack)

* [Dungeon Crawl Stone Soup](https://github.com/crawl/crawl)

* [The Slimy Lichmummy](http://www.happyponyland.net/the-slimy-lichmummy)

* [Shin Megami Tensei](https://en.wikipedia.org/wiki/Megami_Tensei)

* [Sil-Q](https://github.com/sil-quirk/sil-q)

