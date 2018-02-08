
#include <stdio.h>
#include <time.h>
#include "SDL2/SDL.h"
#include <emscripten.h>

int numFrames = 0;
int startTime = 0;

void printFps() {
  int elapsedMS = SDL_GetTicks() - startTime; // Time since start of loop
  ++numFrames;
  if (elapsedMS) {
    double elapsedSeconds = elapsedMS / 1000.0; // Convert to seconds
    double fps = numFrames / elapsedSeconds; // FPS is Frames / Seconds
    printf("%s %f\n", "FPS: ", fps);
  }
}

int lastCalledTime;

void requestAnimFrame() {
  double delta = (emscripten_get_now() - lastCalledTime) / 1000.0;
  double fps = 1.0 / delta;
  lastCalledTime = emscripten_get_now();
  EM_ASM({
      document.getElementById('output').value = "";
    });
  printf("%s %f\n", "FPS: ", fps);
}


int main() {
  startTime = SDL_GetTicks();
  lastCalledTime = emscripten_get_now();
  emscripten_set_main_loop(printFps, -1, 1);
  return 0;
}
