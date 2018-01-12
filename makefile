CC=gcc

CFLAGS=-g -Wall -D_GNU_SOURCE=1 -D_REENTRANT -pedantic -lm -I/usr/include/SDL2 
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

clean:
	rm -rf *.o *.exe *.bak *.c~ $(BINARIES) core a.out
