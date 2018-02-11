# Zombie Breakout

Native C game ported to WebAssembly.

![Zombie breakout](zombie-breakout.gif)

## Demo

Compiled with Emscripten to WASM: https://dondido.github.io/zombie-breakout/

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

Compiling to native code:

```
make clean
make
```

Porting the game to Emscripten

```
make emcc
```
