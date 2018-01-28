
#include <stdio.h>
#include <time.h>
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"

#if __EMSCRIPTEN__
  #include <emscripten.h>
  #include <SDL/SDL_ttf.h>
  #include <SDL/SDL_mixer.h>

  #define FRAME_TIME 10
  #define BULLET_SPEED 9
  #define BULLET_GAP 10
#else
  #include "SDL2/SDL_ttf.h"
  #include <SDL2/SDL_mixer.h>

  #define FRAME_TIME 30
  #define BULLET_SPEED 3
  #define BULLET_GAP 30
#endif

#define SDL_DELAY 4
#define ROWS 13
#define COLUMNS 6
#define BULLET_SIDE 10
#define BG_W 1170
#define HAND_W 70
#define HAND_H 34
#define ENEMY_H 90
#define ENEMY_W 81
#define HITAREA_X 13
#define HITAREA_W 68
#define MAX_DEGREES 80
#define STRAIGHT_ANGLE 180
#define PI 3.14159265
#define HERO_HAND_Y 40

#define HERO_W 80
#define HERO_H 91
#define WINDOW_WIDTH 960
#define WINDOW_HEIGHT 540
#define BUTTON_WIDTH 65 /* dimensions of Music and Sound button */
#define BUTTON_HEIGHT 65
#define MARGIN 10
#define RESET_FRAME -HITAREA_W * 3
#define ENEMY_OFFSET HITAREA_W - HITAREA_X
#define FIELD_X WINDOW_WIDTH - 12
#define STEP_Y 3

typedef struct _TEXT_TEXTURE {
  SDL_Texture *txtText;
  SDL_Rect txtDstRect;
} TEXT_TEXTURE;

typedef struct _OBJECT {
  double posX;
  double posY;
  double stepX;
  double stepY;
} OBJECT;

typedef struct _ENEMY {
  int time;
  int frame;
  int posX;
  int posY;
  int resistance;
  char lifeLabel[3];
  SDL_Texture *textTexture;
  SDL_Texture *sprite;
  SDL_Rect txtDstRect;
} ENEMY;

typedef struct _PLAYER {
  int points;
  char name[25];
} PLAYER;

int gQuit = 0;
int gPoints;
char pointsText[10];
char bulletsText[3];
int gSoundCondition = 1;
int gMusicCondition = 1;
int gPausedGame = 0;
int gHeroNewY = 0;
int isMouseDown = 0;
int aim = 0;
double degrees = 0;

double radians = 0;
double ballX = 0;
double ballY = 0;
double sinRadians = 0;
double cosRadians = 0;
/* The window we'll be rendering to */
SDL_Event e;
SDL_Rect ptsDstRect;
SDL_Rect bltsDstRect;
SDL_Rect dstPauseButton;
SDL_Rect srcPauseButton;
SDL_Rect dstMusicButton;
SDL_Rect srcMusicButton;
SDL_Rect dstSoundButton;
SDL_Rect srcSoundButton;
SDL_Window *gWindow = NULL;
// The window renderer
SDL_Renderer *gRenderer = NULL;
/* The surface contained by the window */
/* The surface we'll be displaying the menu */
SDL_Texture *mainMenu;
SDL_Texture *gameTitle;
/* Current displayed ball image */
SDL_Texture *gHandSurface = NULL;
/* block' surface */
SDL_Texture *zombieLegs;
SDL_Texture *zombiePants;
SDL_Texture *zombieBody;
SDL_Texture *zombieHair;
SDL_Texture *zombiePants;
SDL_Texture *zombieBrows;
SDL_Texture *zombieEyes;
SDL_Texture *zombieArms;
SDL_Texture *zombieMouth;
SDL_Texture *zombieTorso;
SDL_Texture *stageBackground;
SDL_Texture *warningBubble;
SDL_Texture *dangerBubble;
/* bar's surface */
SDL_Texture *heroText;
/*sounds */
Mix_Chunk *hitSound;
Mix_Chunk *destroySound;
Mix_Chunk *extraSound;
Mix_Chunk *laserSound;
Mix_Music *gameMusic;
/* fonts */
TTF_Font *font20;
TTF_Font *font42;
TTF_Font *font28;
// SDL_Surface *gRankingText = NULL;
SDL_Surface *gSettingsText = NULL;
SDL_Surface *gMenuText = NULL;

SDL_Texture *pauseButton;

SDL_Rect dstBg1 = {0, 0, BG_W, WINDOW_HEIGHT};
SDL_Rect dstBg2 = {BG_W, 0, BG_W, WINDOW_HEIGHT};

TEXT_TEXTURE txtText1;
TEXT_TEXTURE txtText2;
TEXT_TEXTURE txtText3;
SDL_Texture *scrnText;
SDL_Texture *btnsText;
SDL_Texture *sndOnText;
SDL_Texture *mscOffText;
// The color of the font
SDL_Color textColor = {255, 255, 255};




OBJECT *ball;
OBJECT hero;
SDL_Point aimPnt;
ENEMY block[ROWS][COLUMNS];
OBJECT balls[200];
SDL_Point handPnt = {0, HAND_H / 2};
SDL_Rect handDstRect = {0, 0, HAND_W, HAND_H};
SDL_Rect bulletRect = {0, 0, BULLET_SIDE, BULLET_SIDE};

int i, j, ii, hitAreaY;
int quantBlocks = 0;
int bulletIconY = WINDOW_HEIGHT - MARGIN * 3 + BULLET_SIDE / 2;
int gameFrame = RESET_FRAME;
int characterFrame = 0;
int characterTime = 0;
int handPvtPntY = 0;
int firstRow = 0;

int level = 1;
int bulletsLoaded = 1;
int killedInColumn = 0;
int lastKilled = 0;

int bulletsInTheMagazine = 0;
int shotInterval = 0;
int enemyInColumn = 0;
int hasGameStarted = 0;
int warningFrame = 0;

SDL_Rect warningDstRect = {0, 84, HITAREA_W, WINDOW_HEIGHT};
int warningAlpha = 15;



void drawCharacter() {
  SDL_Rect srcRect = {characterFrame * HERO_W, 0, HERO_W, HERO_H};
  SDL_Rect dstRect = {hero.posX, hero.posY, HERO_W, HERO_H};
  if (gPausedGame == 0) {
    if (hero.stepY || (gameFrame < 0 && gameFrame > -600)) {
      
      if (characterTime % FRAME_TIME == 0) {
        characterFrame = characterFrame == 2 ? 1 : 2;
      }
    } else if (characterTime % FRAME_TIME == 0) {
      characterFrame = characterFrame == 1 ? 0 : 1;
    }
    characterTime++;
  }
  if (gameFrame < RESET_FRAME) {
    SDL_RenderCopyEx(gRenderer, heroText, &srcRect, &dstRect, 0, NULL, SDL_FLIP_HORIZONTAL);
  } else {
    SDL_RenderCopy(gRenderer, heroText, &srcRect, &dstRect);
  }
}

void animateEnemyEnd(ENEMY *block, int alpha) {
  SDL_SetTextureAlphaMod(block->sprite, alpha);
  if(block->frame == 3) {
    block->posY -= 3;
  }
}

void drawEnemy(ENEMY *block) {
  SDL_Rect frame = (SDL_Rect){block->frame * ENEMY_W, 0, ENEMY_W, ENEMY_H};
  SDL_Rect dstBlock = (SDL_Rect){block->posX + ENEMY_OFFSET, block->posY, ENEMY_W, ENEMY_H};
  SDL_Rect txtDstRect = {block->posX + block->txtDstRect.x, block->txtDstRect.y, block->txtDstRect.w, block->txtDstRect.h};

  if (gPausedGame == 0) {    
    if (block->frame > 1 && block->time < 0) {
      if(block->time == -480 || block->time == -320 || block->time == -80) {
        animateEnemyEnd(block, 200);
      }
      else if(block->time == -460 || block->time == -340 || block->time == -60) {
        animateEnemyEnd(block, 150);
      }
      else if(block->time == -440 || block->time == -360 || block->time == -40) {
        animateEnemyEnd(block, 100);
      }
      else if(block->time == -420 || block->time == -380 ||  block->time == -20) {
        animateEnemyEnd(block, 50);
      }
      else if(block->time == -300) {
        animateEnemyEnd(block, 255);
      }
    }
    else if (block->time % FRAME_TIME == 0) {
      block->frame = block->frame == 1 ? 0 : 1;
    }

    block->time ++;
  }
  SDL_RenderCopy(gRenderer, block->sprite, &frame, &dstBlock);
  if (block->frame != 3) {
    SDL_RenderCopy(gRenderer, block->textTexture, NULL, &txtDstRect);
  }
  
}

void setTextTexture(TEXT_TEXTURE *txtText, TTF_Font *font, char *text) {
  // Render text surface
  SDL_Surface *textSurface = TTF_RenderText_Solid(font, text, textColor);
  SDL_DestroyTexture(txtText->txtText);
  // Create texture from surface pixels
  txtText->txtText = SDL_CreateTextureFromSurface(gRenderer, textSurface);
  txtText->txtDstRect = (SDL_Rect){0, 0, textSurface->w, textSurface->h};
  // Get rid of the obsolete surface
  SDL_FreeSurface(textSurface);
}

void updatePoints(int n) {
  gPoints += n;
  sprintf(pointsText, "%d", gPoints);
  setTextTexture(&txtText1, font28, pointsText);
  txtText1.txtDstRect.x = MARGIN;
  txtText1.txtDstRect.y = MARGIN;
}

void updateBullets(int n) {
  sprintf(bulletsText, "%d", n);
  setTextTexture(&txtText2, font20, bulletsText);
  txtText2.txtDstRect.x = MARGIN * 2 + BULLET_SIDE;
  txtText2.txtDstRect.y = WINDOW_HEIGHT - MARGIN - txtText2.txtDstRect.h;
}

void setZombieTextTexture(ENEMY *block) {
  SDL_DestroyTexture(block->textTexture);
  sprintf(block->lifeLabel, "%d", block->resistance);
  SDL_Surface *textSurface = TTF_RenderText_Solid(font20, block->lifeLabel, textColor);

  block->txtDstRect.w = textSurface->w;
  block->txtDstRect.h = textSurface->h;
  block->txtDstRect.x = ENEMY_W + HITAREA_X - textSurface->w / 2;
  // Create texture from surface pixels
  block->textTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
  // Get rid of the obsolete surface
  SDL_FreeSurface(textSurface);
}

void reflectX(OBJECT *ball) {
  ball->stepX *= -1;
  ball->posX += ball->stepX;
}

void reflectY(OBJECT *ball) {
  ball->stepY *= -1;
  ball->posY += ball->stepY;
}

void playSound(Mix_Chunk* sound) {
  if(gSoundCondition) {
    Mix_PlayChannel(-1, sound, 0);
  }
}

void damage(int row, int column) {
  int j;
  int allDead = 1;
  ENEMY* b = &block[row][column];
  b->resistance--;
  setZombieTextTexture(b);
  
  if (b->resistance == 0) {
    for (j = 0; j < COLUMNS; j++) {
      if(block[row][j].resistance > 0) {
        allDead = 0;
      }
    }
    b->time = allDead ? -500 : -100;
    playSound(destroySound);
  }
  else {
    b->time = 1;
    playSound(hitSound);
  }
  b->frame = 2;
  updatePoints(1);
}

void collide(OBJECT *ball, int *quantBlocks) {
  int movingRight = ball->stepX > 0;
  int movingBottom = ball->stepY >= 0;
  int left = ball->posX / HITAREA_W - 1;
  int right = (ball->posX + BULLET_SIDE) / HITAREA_W - 1;
  int top = ball->posY / ENEMY_H;
  int bottom = (ball->posY + BULLET_SIDE) / ENEMY_H;

  if (COLUMNS == bottom) {
    bottom = top;
  }

  if (block[left][top].resistance == 0 && block[right][top].resistance == 0 &&
      block[left][bottom].resistance == 0 &&
      block[right][bottom].resistance == 0) {
    return;
  }

  int x1 = left;
  int x2 = right;
  int y1 = top;
  int y2 = bottom;

  if (top == bottom) {
    if (movingRight) {
      x1 = right;
    }
    if (block[x1][top].resistance) {
      damage(x1, top);
      reflectX(ball);
    }
  } else if (left == right) {
    if (movingBottom) {
      y1 = bottom;
    }
    if (block[left][y1].resistance) {
      damage(left, y1);
      reflectY(ball);
    }
  } else if (top != bottom && left != right) {
    if (movingRight) {
      x1 = right;
      x2 = left;
    }
    if (movingBottom) {
      y1 = bottom;
      y2 = top;
    }
    if (block[x1][y1].resistance) {
      damage(x1, y1);
      if (block[x2][y1].resistance == 0 && block[x1][y2].resistance == 0) {
        reflectX(ball);
        reflectY(ball);
        return;
      }
    }
    if (block[x2][y1].resistance) {
      damage(x2, y1);
      reflectY(ball);
    }
    if (block[x1][y2].resistance) {
      damage(x1, y2);
      reflectX(ball);
    }
  }
}

void drawSprite(int x, int y, int armsSpriteY, int eyesSpriteX, int mouthSpriteX, int hairSpriteX, int hairSpriteY, int browsSpriteX, int pantsSpriteX) {
  SDL_Rect srcRect = {pantsSpriteX, 0, ENEMY_W, 92};
  SDL_Rect dstRect = (SDL_Rect){x + HITAREA_X, y + 86, HITAREA_W, 8};
  dstRect = (SDL_Rect){x, 0, ENEMY_W, 92};
  SDL_RenderCopy(gRenderer, zombieLegs, &srcRect, &dstRect);
  dstRect.y = y;
  srcRect.x = 0;
  SDL_RenderCopy(gRenderer, zombieTorso, &srcRect, &dstRect);
  srcRect.x = pantsSpriteX;
  SDL_RenderCopy(gRenderer, zombiePants, &srcRect, &dstRect);
  srcRect.x = eyesSpriteX;
  SDL_RenderCopy(gRenderer, zombieEyes, &srcRect, &dstRect);
  srcRect.x = mouthSpriteX;
  SDL_RenderCopy(gRenderer, zombieMouth, &srcRect, &dstRect);
  dstRect.y += armsSpriteY;
  srcRect.x = 0;
  SDL_RenderCopy(gRenderer, zombieArms, &srcRect, &dstRect);
  dstRect.y -= armsSpriteY;
  srcRect.x = browsSpriteX;
  SDL_RenderCopy(gRenderer, zombieBrows, &srcRect, &dstRect);
  srcRect.h = dstRect.h = 40;
  srcRect.x = hairSpriteX;
  srcRect.y = hairSpriteY;
  SDL_RenderCopy(gRenderer, zombieHair, &srcRect, &dstRect);
}

ENEMY createEnemy(int posX, int posY) {
  int hairSpriteY = ENEMY_H * (rand() % 5);
  int hairColour = rand() % 7;
  int hairSpriteX = ENEMY_W * hairColour;
  int browsSpriteX = ENEMY_W * (hairColour == 6 ? rand() % 6 : hairColour);
  int pantsSpriteX = ENEMY_W * (rand() % 7);
  int eyesSpriteX = ENEMY_W * (rand() % 4);
  int blinkSpriteX = ENEMY_W * 4;
  int mouthSpriteX = ENEMY_W * (rand() % 6);

  SDL_Rect bulletRect = {ENEMY_W * 3 + HITAREA_X + BULLET_SIDE, ENEMY_H / 2 + 4 + BULLET_SIDE, BULLET_SIDE, BULLET_SIDE};
  int armsSpriteX = 2;
  ENEMY block = {1, 0, posX, posY, level + (rand() % 4) * 10 * (level / 10)};

  block.txtDstRect = (SDL_Rect){0, posY + ENEMY_H - 24, 0, 0};
  setZombieTextTexture(&block);

  block.sprite =
      SDL_CreateTexture(gRenderer, SDL_PIXELFORMAT_ARGB8888,
                        SDL_TEXTUREACCESS_TARGET, ENEMY_W * 4, ENEMY_H);
  SDL_SetRenderTarget(gRenderer, block.sprite);

  // will make pixels with alpha 0 fully transparent
  // use SDL_SetTextureBlendMode . Not SDL_SetRenderDrawBlendMode
  SDL_SetTextureBlendMode(block.sprite, SDL_BLENDMODE_BLEND);

  drawSprite(0, 0, 0, eyesSpriteX, mouthSpriteX, hairSpriteX, hairSpriteY, browsSpriteX, pantsSpriteX);
  drawSprite(ENEMY_W, 3, armsSpriteX, eyesSpriteX, mouthSpriteX, hairSpriteX, hairSpriteY, browsSpriteX, pantsSpriteX);
  drawSprite(ENEMY_W * 2, 3, 0, blinkSpriteX, mouthSpriteX, hairSpriteX, hairSpriteY, browsSpriteX, pantsSpriteX);

  SDL_SetRenderDrawColor/**/(gRenderer, 0xFF, 0x00, 0xFF, 255);
  SDL_RenderFillRect(gRenderer, &bulletRect);
  setTextTexture(&txtText3, font20, "+1");
  txtText3.txtDstRect.x = ENEMY_W * 3 + BULLET_SIDE * 4;
  txtText3.txtDstRect.y = ENEMY_H / 2 + BULLET_SIDE;
  SDL_RenderCopy(gRenderer, txtText3.txtText, NULL, &txtText3.txtDstRect);

  SDL_SetRenderTarget(gRenderer, NULL);
  return block;
}

int moveObject(OBJECT *p) {
  if (p->posX < 0) {
    p->stepX = 0;
    p->stepY = 0;
    return 1;
  }

  if (p->posX + BULLET_SIDE > FIELD_X) {
    p->stepX = -p->stepX;
  }

  if (p->posY + BULLET_SIDE > WINDOW_HEIGHT || p->posY < 0) {
    p->stepY = -p->stepY;
  }

  p->posX += p->stepX;
  p->posY += p->stepY;

  return 0;
}

void moveHero(OBJECT *p) {
  if ((p->posY + ENEMY_H >= WINDOW_HEIGHT && p->stepY > 0) ||
      (p->posY < 0 && p->stepY < 0)) {
    p->stepY = 0;
  }
  p->posY += p->stepY;
}

int init() {
  /*Initialization flag*/
  int success = 1;

  srand(time(NULL));
  Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
  Mix_AllocateChannels(16);

  if (TTF_Init() == -1) {
    printf("TTF unable to initialize! Error: %s\n", TTF_GetError());
    success = 0;
  }

  /*Initialize SDL*/
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
    printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
    success = 0;
  } else {
    /*Create window*/
    gWindow =
        SDL_CreateWindow("Breakout game. Have fun!", SDL_WINDOWPOS_UNDEFINED,
                         SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT,
                         SDL_WINDOW_RESIZABLE);
    if (gWindow == NULL) {
      printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
      success = 0;
    } else {
      // Create renderer for window
      gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
      if (gRenderer == NULL) {
        printf("Renderer could not be created! SDL Error: %s\n",
               SDL_GetError());
        success = 0;
      } else {
        // Initialize renderer color
        SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

        // Initialize PNG loading
        int imgFlags = IMG_INIT_PNG;
        if (!(IMG_Init(imgFlags) & imgFlags)) {
          printf("SDL_image could not initialize! SDL_image Error: %s\n",
                 IMG_GetError());
          success = 0;
        }
      }

      /*Initialize JPG and PNG loading */
      int imgFlags = IMG_INIT_JPG | IMG_INIT_PNG;
      if (!(IMG_Init(imgFlags) & imgFlags)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n",
               IMG_GetError());
        success = 0;
      }
      if( Mix_OpenAudio( 22050, MIX_DEFAULT_FORMAT, 2, 4096 ) == -1 ) {
        success = 0;
      }
    }
  }
  return success;
}

SDL_Texture *loadTexture(char *path) {
  // The final texture
  SDL_Texture *newTexture = NULL;

  // Load image at specified path
  SDL_Surface *loadedSurface = IMG_Load(path);
  if (loadedSurface == NULL) {
    printf("Unable to load image %s! SDL_image Error: %s\n", path,
           IMG_GetError());
  } else {
    // Create texture from surface pixels
    newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
    if (newTexture == NULL) {
      printf("Unable to create texture from %s! SDL Error: %s\n", path,
             SDL_GetError());
    }

    // Get rid of old loaded surface
    SDL_FreeSurface(loadedSurface);
  }

  return newTexture;
}

void loadMedia() {
  /* Load menu surface */
  mainMenu = loadTexture("assets/images/menu-11-menu.png");
  gameTitle = loadTexture("assets/images/menu-11-title.png");
  pauseButton = loadTexture("assets/images/menu-11-pause.png");
  sndOnText = loadTexture("assets/images/menu-11-sound-on.png");
  mscOffText = loadTexture("assets/images/menu-11-music-off.png");

  /* load bar surface */
  heroText = loadTexture("assets/images/character-8.png");
  gHandSurface = loadTexture("assets/images/character-hand-2.png");
  warningBubble = loadTexture("assets/images/distanse-2.png");
  dangerBubble = loadTexture("assets/images/danger-2.png");

  stageBackground = loadTexture("assets/images/zombie-grid-3.png");

  /* load zombie texture */
  zombieLegs = loadTexture("assets/images/zombie-legs-3.png");
  zombieHair = loadTexture("assets/images/hair-spritesheet.png");
  zombiePants = loadTexture("assets/images/zombie-pants-3.png");
  zombieTorso = loadTexture("assets/images/zombie-torso-2.png");
  zombieBrows = loadTexture("assets/images/zombie-brows-2.png");
  zombieEyes = loadTexture("assets/images/zombie-eyes-3.png");
  zombieArms = loadTexture("assets/images/zombie-arms-2.png");
  zombieMouth = loadTexture("assets/images/zombie-mouth-2.png");

  /*load sounds */
  hitSound = Mix_LoadWAV("assets/sounds/hit.wav");
  destroySound = Mix_LoadWAV("assets/sounds/destroy.wav");
  laserSound = Mix_LoadWAV("assets/sounds/laser.wav");
  extraSound = Mix_LoadWAV("assets/sounds/extra.wav");
  gameMusic = Mix_LoadMUS("assets/sounds/music.mp3");
  

  /*load fonts*/
  // TTF_Font *font;
  font20 = TTF_OpenFont("assets/images/visitor1.ttf", 20);
  font28 = TTF_OpenFont("assets/images/visitor1.ttf", 28);
  font42 = TTF_OpenFont("assets/images/visitor1.ttf", 42);
}

SDL_Texture *createEmptySprite() {
  SDL_Texture *texture =
      SDL_CreateTexture(gRenderer, SDL_PIXELFORMAT_RGBA8888,
                        SDL_TEXTUREACCESS_TARGET, WINDOW_WIDTH, WINDOW_HEIGHT);
  SDL_SetRenderTarget(gRenderer, texture);

  // will make pixels with alpha 0 fully transparent
  // use SDL_SetTextureBlendMode . Not SDL_SetRenderDrawBlendMode
  SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
  return texture;
}

void renderXCenteredText(TTF_Font *font, char string[], int y) {
  setTextTexture(&txtText3, font, string);
  txtText3.txtDstRect.x = (WINDOW_WIDTH - txtText3.txtDstRect.w) / 2;
  txtText3.txtDstRect.y = y;
  SDL_RenderCopy(gRenderer, txtText3.txtText, NULL, &txtText3.txtDstRect);
}

void showMarket() {
  int textY = WINDOW_HEIGHT / 4;
  scrnText = createEmptySprite();
  renderXCenteredText(font42, "MARKET", textY - 42 * 2);
  renderXCenteredText(font28, "Market is currently closed!", textY + 4 * 28);
  SDL_SetRenderTarget(gRenderer, NULL);
}

void showHelp() {
  int textY = WINDOW_HEIGHT / 4;
  scrnText = createEmptySprite();
  renderXCenteredText(font42, "HOW TO PLAY", textY - 42 * 2);
  renderXCenteredText(font28, "1. Point and tap to move you character.", textY + 2 * 28);
  renderXCenteredText(font28, "2. Tap and hold down to aim at the target.",  textY + 4 * 28);
  renderXCenteredText(font28, "3. Release to shoot.", textY + 6 * 28);
  renderXCenteredText(font28, "4. Defend the city against hordes of zombies.", textY + 8 * 28);
  SDL_SetRenderTarget(gRenderer, NULL);
}

#if __EMSCRIPTEN__
void listRecords() {}
void setRank() {}
#else
void listRecords() {
  PLAYER records[5];
  int empty = 1;
  char entry[50];
  char digits[7];
  int textY = WINDOW_HEIGHT / 4;
  int i;

  scrnText = createEmptySprite();

  renderXCenteredText(font42, "TOP RANKINGS", textY - 42 * 2);

  FILE *file = fopen("records.bin", "r");
  if (!file) {
    printf("Failed to read ranking.\n");
    gQuit = 1;
  }
  fread(records, sizeof(PLAYER), 5, file);

  for (i = 0; i < 5; i++) {
    if (records[i].points > 0) {
      empty = 0;
      // memset(entry, 0, sizeof entry);
      strcpy(entry, records[i].name);
      strcat(entry, " pts: ");
      // counts number of digits in an int
      sprintf(digits, "%d", records[i].points);
      strcat(entry, digits);
      renderXCenteredText(font28, entry, textY + i * 2 * 28);
    }
  }
  if (empty) {
    renderXCenteredText(font28, "No records found!", textY + 4 * 28);
  }
  SDL_SetRenderTarget(gRenderer, NULL);
}

int setRank() {
  FILE *pRankFile;
  PLAYER player;
  PLAYER records[5];
  PLAYER aux;
  int i;
  int textY = WINDOW_HEIGHT / 4;
  char pts[12] = "PTS: ";
  char timeStr[25];
  time_t t = time(NULL);
  struct tm *tm = localtime(&t);

  strcat(pts, pointsText);
  scrnText = createEmptySprite();
  renderXCenteredText(font42, "GAME OVER", textY - 42 * 2);
  renderXCenteredText(font28, pts, textY + 4 * 28);
  SDL_SetRenderTarget(gRenderer, NULL);

  gPausedGame = 0;

  strftime(timeStr, sizeof(timeStr), "%c", tm);
  strcpy(player.name, timeStr);

  player.points = gPoints;

  pRankFile = fopen("records.bin", "r+");
  if (!pRankFile) {
    perror("Could not open the rankings file! Error: ");
    gQuit = 1;
  } else {
    fread(records, sizeof(PLAYER), 5, pRankFile);
    /* Searches for values in the top 5 ranks that are lower than the
       the new player score. */
    if (records[4].points < player.points) {
      records[4] = player;
      for (i = 4; i > 0; i--) {
        if (records[i].points > records[i - 1].points) {
          aux = records[i];
          records[i] = records[i - 1];
          records[i - 1] = aux;
        }
      }
      fclose(pRankFile);
      pRankFile = fopen("records.bin", "wb");
      if (!pRankFile) {
        perror("Ranking file could not be replaced! Error: ");
        gQuit = 1;
      } else {
        fwrite(records, sizeof(PLAYER), 5, pRankFile);
        fclose(pRankFile);
        return 1;
      }
    } else
      return 0;
  }
  return 1;
}
#endif

void closing() {
  // SDL_FreeSurface(mainMenu);
  mainMenu = NULL;

  /* Close font */
  TTF_CloseFont(font20);
  font20 = NULL;

  gMenuText = NULL;

  /* Free bar image */
  // SDL_FreeSurface(heroText);
  heroText = NULL;

  // SDL_FreeSurface(gScreenSurface);
  // gScreenSurface = NULL;

  /*Destroy window*/
  SDL_DestroyRenderer(gRenderer);
  SDL_DestroyWindow(gWindow);
  gWindow = NULL;

  /*Free sounds */
  Mix_FreeChunk(hitSound);
  Mix_FreeChunk(destroySound);
  Mix_FreeMusic(gameMusic);

  // Close the font that was used
  TTF_CloseFont(font20);
  TTF_CloseFont(font28);
  TTF_CloseFont(font42);

  /*Quit SDL subsystems*/
  TTF_Quit();
  IMG_Quit();
  SDL_Quit();
}

void updateMenuTexture() {
  btnsText = createEmptySprite();
  if (gSoundCondition) {
    SDL_RenderCopy(gRenderer, sndOnText, NULL, NULL);
  }
  if (!gMusicCondition) {
    SDL_RenderCopy(gRenderer, mscOffText, NULL, NULL);
  }
  SDL_SetRenderTarget(gRenderer, NULL);
}

void switchSound() {
  gSoundCondition = !gSoundCondition;
  updateMenuTexture();
}

void switchMusic() {
  gMusicCondition = !gMusicCondition;
  updateMenuTexture();
}

int clickButton(SDL_Event e, SDL_Rect button) {
  return e.motion.x >= button.x && e.motion.x <= button.x + button.w &&
         e.motion.y >= button.y && e.motion.y <= button.y + button.h;
}

void playMusic() {
  if (gMusicCondition) {
    if (Mix_PausedMusic()) {
      Mix_ResumeMusic();
    } else {
      Mix_PlayMusic(gameMusic, -1);
    }
  }
}

void setAngle() {
  double maxRadians = 0;
  radians = atan2(e.motion.y - handDstRect.y - handDstRect.h / 2, e.motion.x - handDstRect.x);

  degrees = radians * STRAIGHT_ANGLE / PI;
  aim = 1;
  if (degrees > MAX_DEGREES) {
    degrees = MAX_DEGREES;
    aim = 0;
  } else if (degrees < -MAX_DEGREES) {
    degrees = -MAX_DEGREES;
    aim = 0;
  }
  maxRadians = degrees * PI / STRAIGHT_ANGLE;
  sinRadians = sin(maxRadians);
  cosRadians = cos(maxRadians);

  ballY = sinRadians * BULLET_SPEED;
  ballX = cosRadians * BULLET_SPEED;
}

SDL_Point getRulerCorners(int pvtX, int pvtY, int ox, int oy) {
  // The offset of a corner in local coordinates (i.e. relative to the pivot  point)
  // which corner will depend on the coordinate reference system used in our environment
  SDL_Point pnt;

  // The rotated position of this corner in world coordinates
  pnt.x = pvtX + (ox * cosRadians) - (oy * sinRadians);
  pnt.y = pvtY + (ox * sinRadians) + (oy * cosRadians);

  return pnt;
}

void reset() {
 
  quantBlocks = 0;
  hero = (OBJECT){-HITAREA_W - 45, WINDOW_HEIGHT / 2 - HERO_W / 2, 0, 0};
  //gameFrame = RESET_FRAME;
  characterFrame = 0;
  characterTime = 0;
  handPvtPntY = 0;

  gPoints = 0;
  

  level = 1;
  bulletsLoaded = 1;
  killedInColumn = 0;
  lastKilled = 0;

  bulletsInTheMagazine = 0;
  shotInterval = 0;
  enemyInColumn = 0;
  hasGameStarted = 0;

  gPausedGame = 0;

  dstPauseButton = (SDL_Rect){WINDOW_WIDTH - BUTTON_WIDTH - MARGIN, MARGIN, BUTTON_WIDTH, BUTTON_HEIGHT};

  scrnText = NULL;


  updatePoints(gPoints);
  updateMenuTexture();
  updateBullets(1);

  for (i = 0; i < ROWS - 1; i++) {
    for (j = 0; j < COLUMNS; j++) {
      block[i][j] = (ENEMY){0};
    }
  }
  for (j = 0; j < COLUMNS; j++) {
    if (rand() % 3 == 0) {
      block[ROWS - 1][j] = createEnemy(HITAREA_W * ROWS, ENEMY_H * j);
      quantBlocks++;
    } else {
      block[i][j] = (ENEMY){0};
    }
  }
  if(quantBlocks == 0) {
    block[rand() % 6][j] = createEnemy(HITAREA_W * ROWS, ENEMY_H * j);
      quantBlocks++;
  }
}

void handleButtons() {
  SDL_Rect playDstRect = {WINDOW_WIDTH / 2 - 84 / 2, WINDOW_HEIGHT - 100, 84, 84};
  SDL_Rect exitDstRect = {208, 457, BUTTON_WIDTH, BUTTON_HEIGHT};
  SDL_Rect sndDstRect = {296, 457, BUTTON_WIDTH, BUTTON_HEIGHT};
  SDL_Rect mscDstRect = {384, 457, BUTTON_WIDTH, BUTTON_HEIGHT};
  SDL_Rect recordsDstRect = {538, 457, BUTTON_WIDTH, BUTTON_HEIGHT};
  SDL_Rect marketDstRect = {616, 457, BUTTON_WIDTH, BUTTON_HEIGHT};
  SDL_Rect helpDstRect = {694, 457, BUTTON_WIDTH, BUTTON_HEIGHT};

  if (gPausedGame == 0 && hasGameStarted) {
    switch (e.type) {
      case SDL_QUIT:
        gQuit = 1;
        break;
      case SDL_MOUSEBUTTONDOWN:  // if the event is mouse click
        if (clickButton(e, dstPauseButton)) {
          gPausedGame = 1;
          if (gMusicCondition) {
            Mix_PauseMusic();
          }
          break;
        }
        if (e.motion.x < 120 && (gameFrame == shotInterval || gameFrame < 2)) {
          gHeroNewY = e.motion.y - HERO_HAND_Y;
          hero.stepY = gHeroNewY > hero.posY ? STEP_Y : -STEP_Y;

          characterTime = 1;
          characterFrame = 2;
          break;
        }
        if (gameFrame == 1 || gameFrame == 0) {
          setAngle();
          isMouseDown = 1;
          gameFrame = 2;
        }
        break;
      case SDL_MOUSEBUTTONUP:
        if (e.motion.x < 110 && (gameFrame == 2 || gameFrame == 0)) {
          gameFrame = 1;
        }
        isMouseDown = 0;
        break;
      case SDL_MOUSEMOTION:
        if (isMouseDown == 1) {
          setAngle();
        }
        break;
    }
    return;
  }

  switch (e.type) {
    case SDL_QUIT:
      gQuit = 1;
      break;
    case SDL_MOUSEBUTTONUP:
      if (clickButton(e, exitDstRect)) {
        if (hasGameStarted == 0 && scrnText == NULL) {
          gQuit = 1;
        } else if (hasGameStarted == 1 && gPausedGame) {
          gameFrame = -WINDOW_WIDTH;
          setRank();
        }
        break;
      } else if (clickButton(e, playDstRect)) {
        scrnText = NULL;
        if (gPausedGame) {
          gPausedGame = 0;
          playMusic();
        } else if (hasGameStarted == 0) {
          hasGameStarted = 1;
          playMusic();
        }
      } else if (hasGameStarted == 0 || (hasGameStarted == 1 && gPausedGame)) {
        if (clickButton(e, recordsDstRect)) {
          listRecords();
        } else if (clickButton(e, marketDstRect)) {
          showMarket();
        } else if (clickButton(e, helpDstRect)) {
          showHelp();
        } else if (clickButton(e, sndDstRect)) {
          switchSound();
        } else if (clickButton(e, mscDstRect)) {
          switchMusic();
        }
      }
      break;
  }
}

void tick() {

  SDL_Rect bubbleDstRect = {0, hero.posY - HAND_H, WINDOW_WIDTH, WINDOW_HEIGHT};

  /* verifies if any key have been pressed */
  while (SDL_PollEvent(&e) != 0) {
    handleButtons();
  }

  SDL_RenderClear(gRenderer);
  SDL_RenderCopy(gRenderer, stageBackground, NULL, &dstBg1);
  SDL_RenderCopy(gRenderer, stageBackground, NULL, &dstBg2);

  if(firstRow) {
    if(gameFrame < 0) {
      warningDstRect.x --;
    }
    if(warningFrame == 570) {
      warningAlpha = 15;
    }
    else if(warningFrame == 10 || warningFrame == 290) {
      warningAlpha = 30;
    }
    else if(warningFrame == 20 || warningFrame == 280) {
      warningAlpha = 45;
    }
    else if(warningFrame == 30 || warningFrame == 270) {
      warningAlpha = 60;
    }
    else if(warningFrame == 40 || warningFrame == 260) {
      warningAlpha = 75;
    }
    else if(warningFrame == 50) {
      warningAlpha = 90;
    }
    
    SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, warningAlpha);
    SDL_RenderFillRect(gRenderer, &warningDstRect);
    SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_NONE);
    SDL_RenderCopy(gRenderer, firstRow == 3 ? dangerBubble : warningBubble, NULL, &bubbleDstRect);

    warningFrame ++;
    if(warningFrame == 300) {
      firstRow = warningFrame = 0;
    }
  }

  SDL_SetRenderDrawColor(gRenderer, 0xFF, 0x00, 0xFF, 255);

  for (j = 0; j < COLUMNS; j++) {
    for (i = 2; i < ROWS; i++) {
      if (block[i][j].time) {
        drawEnemy(&block[i][j]);
      }
      if (block[i][j].time == -400) {
        block[i][j].frame = 3;
        playSound(extraSound);
      }
      else if (block[i][j].time == -1) {
        SDL_DestroyTexture(block[i][j].textTexture);
        SDL_DestroyTexture(block[i][j].sprite);
        block[i][j] = (ENEMY){0};
      }
    }
  }

  if (gPausedGame == 0) {
    if (hero.stepY != 0 && fabs(gHeroNewY - hero.posY) <= STEP_Y) {
      hero.posY = gHeroNewY;
      hero.stepY = 0;
    }
    if (gameFrame > 0) {
      if (gameFrame == 2) {
        if (aim) {
          // the coordinates of Y in world coordinates
          handPvtPntY = handDstRect.y + handDstRect.h / 2;
          aimPnt = getRulerCorners(handDstRect.x, handPvtPntY, handDstRect.w / 2, -2);
          SDL_RenderDrawLine(gRenderer, aimPnt.x, aimPnt.y, e.motion.x, e.motion.y);
          SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);

          if (isMouseDown == 0) {
            gameFrame = 3;
            shotInterval = bulletsLoaded * BULLET_GAP + 1;
          }
        }
      } else if (gameFrame < shotInterval) {
        if (gameFrame % BULLET_GAP == 0) {
          ball = &balls[gameFrame / BULLET_GAP - 1];
          aimPnt = getRulerCorners(handDstRect.x, handPvtPntY, handDstRect.w, -12);
          ball->posY = aimPnt.y;
          ball->posX = aimPnt.x;
          ball->stepY = ballY;
          ball->stepX = ballX;
          updateBullets(bulletsLoaded - gameFrame / BULLET_GAP);
          playSound(laserSound);
        }
        gameFrame++;
      }

      if (gameFrame > 2) {
        for (ii = 0; ii < bulletsLoaded; ii++) {
          bulletsInTheMagazine += moveObject(&balls[ii]);
          if (balls[ii].posX > 0) {
            collide(&balls[ii], &quantBlocks);
            bulletRect.x = balls[ii].posX;
            bulletRect.y = balls[ii].posY;
            SDL_RenderFillRect(gRenderer, &bulletRect);
          }
        }
      }

      if (gameFrame == shotInterval) {
        if (bulletsInTheMagazine == bulletsLoaded) {
          for (j = 0; j < COLUMNS; j++) {
            if (block[2][j].resistance) {
              gameFrame = -1000;
              Mix_FadeOutMusic(1000 * 2);
              setRank();
              break;
            };
          }
          if (gameFrame != -1000) {
            gameFrame = -HITAREA_W - 1;
            shotInterval = 0;
            for (i = 1; i < ROWS; i++) {
              for (j = 0; j < COLUMNS; j++) {
                if (block[i][j].resistance == 0) {
                  killedInColumn++;
                  if (block[i][j].lifeLabel[0]) {
                    block[i][j].lifeLabel[0] = 0;
                    lastKilled = 1;
                  }
                }
                block[i - 1][j] = block[i][j];
              }
              if (lastKilled && killedInColumn == COLUMNS) {
                bulletsLoaded++;
              }
              if(firstRow == 0 && killedInColumn != COLUMNS && i < 5){
                firstRow = i;
                warningDstRect.x = (i + 1) * HITAREA_W;
              }
              killedInColumn = 0;
              lastKilled = 0;
            }
            level++;
            for (j = 0; j < COLUMNS; j++) {
              block[i - 1][j] = (ENEMY){0};
            }
            enemyInColumn = rand() % 5 + 1;
            for (j = 0; j < enemyInColumn; j++) {
              hitAreaY = rand() % 6;
              block[i - 1][hitAreaY] = createEnemy(HITAREA_W * i, ENEMY_H * hitAreaY);
              quantBlocks++;
            }
          }
        }
        bulletsInTheMagazine = 0;
      }
    } else if (gameFrame < -1) {
      gameFrame++;
      if (gameFrame < -600) {
        dstBg1.x += 2;
        dstBg2.x += 2;
        hero.posX += 2;
        handDstRect.x = hero.posX - 65;
        for (i = 2; i < ROWS; i++) {
          for (j = 0; j < COLUMNS; j++) {
            block[i][j].posX += 2;
          }
        }
        if (dstBg1.x > WINDOW_WIDTH - BG_W) {
          dstBg2.x = dstBg1.x - BG_W;
        }
        if (dstBg2.x > WINDOW_WIDTH - BG_W) {
          dstBg1.x = dstBg2.x - BG_W;
        }
      }

      else if (gameFrame > -HITAREA_W - 1) {
        dstBg1.x--;
        dstBg2.x--;
        if (dstBg1.x < 0) {
          dstBg2.x = dstBg1.x + BG_W;
        }
        if (dstBg2.x < 0) {
          dstBg1.x = dstBg2.x + BG_W;
        }
        for (i = 2; i < ROWS; i++) {
          for (j = 0; j < COLUMNS; j++) {
            block[i][j].posX--;
          }
        }
      } else if (gameFrame < RESET_FRAME) {
        hero.posX -= STEP_Y;
        handDstRect.x = hero.posX - 65;
      } else if (gameFrame == RESET_FRAME) {
        reset();
        return;
      } else if (gameFrame > RESET_FRAME) {
        hero.posX ++;
        handDstRect.x = hero.posX + 72;
      }

    } else if (gameFrame == -1) {
      updateBullets(bulletsLoaded);
      gameFrame++;
    }
   
  }

  moveHero(&hero);

  //characterTime++;
  handDstRect.y = hero.posY + 35;

  if (gameFrame < RESET_FRAME) {
    SDL_RenderCopyEx(gRenderer, gHandSurface, NULL, &handDstRect, 0, &handPnt, SDL_FLIP_HORIZONTAL);
  } else {
    SDL_RenderCopyEx(gRenderer, gHandSurface, NULL, &handDstRect, degrees, &handPnt, SDL_FLIP_NONE);
  }

  drawCharacter();

  if (scrnText) {
    SDL_RenderCopy(gRenderer, scrnText, NULL, NULL);
    if (gameFrame > RESET_FRAME) {
      SDL_RenderCopy(gRenderer, mainMenu, NULL, NULL);
      SDL_RenderCopy(gRenderer, btnsText, NULL, NULL);
    }
  } else if (hasGameStarted == 0) {
    SDL_RenderCopy(gRenderer, gameTitle, NULL, NULL);
    SDL_RenderCopy(gRenderer, mainMenu, NULL, NULL);
    SDL_RenderCopy(gRenderer, btnsText, NULL, NULL);
  } else {
    SDL_RenderCopy(gRenderer, txtText1.txtText, NULL, &txtText1.txtDstRect);
    SDL_RenderCopy(gRenderer, txtText2.txtText, NULL, &txtText2.txtDstRect);
    bulletRect.x = MARGIN;
    bulletRect.y = bulletIconY;
    SDL_RenderFillRect(gRenderer, &bulletRect);
    if (gPausedGame) {
      SDL_RenderCopy(gRenderer, mainMenu, NULL, NULL);
      SDL_RenderCopy(gRenderer, btnsText, NULL, NULL);
    } else {
      SDL_RenderCopy(gRenderer, pauseButton, NULL, NULL);
    }
  }

  SDL_RenderPresent(gRenderer);
}

void loop() {
#if __EMSCRIPTEN__
  emscripten_set_main_loop(tick, 0, 1);
#else
  /* Starts game main loop */
  while (!gQuit) {
    SDL_Delay(SDL_DELAY);
    tick();
  }
#endif
}

int main(int argc, char const *argv[]) {
  /* Start up SDL and create window */
  srand(time(NULL));
  if (!init()) {
    printf("SDL could not be initialized\n");
  } else {
    loadMedia();
    gQuit = 0;
    reset();
    loop();
  }
  closing();
  return 0;
}
