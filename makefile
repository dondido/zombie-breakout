CC=gcc

CFLAGS=-g -Wall -O3 -D_GNU_SOURCE=1 -D_REENTRANT -pedantic -lm -I/usr/include/SDL2 
IMAGE_FLAGS=-lSDL2_image
MIXER_FLAGS=-lSDL2_mixer
FONTS_FLAGS=-lSDL2_ttf
SFLAGS=-lSDL2
SOURCES=main.c
OBJECTS=main.o
BINARIES=zombie-breakout

all: $(BINARIES)

zombie-breakout: main.o
	$(CC) -o zombie-breakout main.o $(CFLAGS) $(SFLAGS) $(IMAGE_FLAGS) $(MIXER_FLAGS) $(FONTS_FLAGS)

main.o: main.c
	$(CC) -c main.c $(CFLAGS) $(SFLAGS) $(IMAGE_FLAGS) $(MIXER_FLAGS) $(FONTS_FLAGS)

emcc: main.c
	emcc main.c -s USE_SDL=2 -s USE_SDL_TTF=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]' -s WASM=1 --shell-file html_template/resize-with-aspect-ratio.html -s USE_OGG=1 -s USE_VORBIS=1 -o dist/index.html -O2 --preload-file assets

clean:
	rm -rf *.o *.exe *.bak *.c~ $(BINARIES) core a.out
