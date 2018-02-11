# Zombie Breakout

Sample C / Webassembly game.

![Zombie breakout](zombie-breakout.gif)

## Compilation

### Native

These are instructions for Mac OS. In other UNIX environments, it should be similar but changing the way to install SDL itself.


1. Install SDL2: `sudo apt-get install libsdl2-dev`
2. Install Image: `sudo apt-get install libsdl2-image-dev`
3. Install SDL2 Mixer: `sudo apt-get install libsdl2-mixer-dev`
4. Install SDL2 Ttf `sudo apt-get install libsdl2-ttf-dev`.

### Emscripten

### Requirements

Build the source as [shown in the MDN](https://developer.mozilla.org/en-US/docs/Mozilla/Projects/SpiderMonkey/Build_Documentation).

### Compiling

Compiling is rather easy:

```
make clean
make
```
