
#include <stdio.h>
#include <time.h>
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"

#if __EMSCRIPTEN__
  #include <emscripten.h>
  #include <SDL/SDL_ttf.h>
  #include <SDL/SDL_mixer.h>
#else
  #include "SDL2/SDL_ttf.h"
  #include <SDL2/SDL_mixer.h>
#endif

#define FRAME_TIME 19
#define STEP_TIME 10
#define BULLET_SPEED 10
#define BULLET_GAP 5

#define SDL_DELAY 12
#define ROWS 13
#define COLUMNS 6
#define BULLET_S 10
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
#define WINDOW_W 960
#define WINDOW_H 540
#define BUTTON_S 52
#define MARGIN 10
#define SPRITE_W WINDOW_W - 1
#define RESET_FRAME -HITAREA_W * 3
#define ENEMY_OFFSET HITAREA_W - HITAREA_X
#define FIELD_X WINDOW_W - BULLET_S * 2
#define STEP_Y 6

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
  char lifeLabel[6];
  SDL_Texture *textTexture;
  SDL_Texture *sprite;
  SDL_Rect txtDstRect;
} ENEMY;

typedef struct _RECORD {
  int points;
  char name[25];
} RECORD;

int gQuit = 0;
int gPoints;
char pointsText[10];
char bulletsText[9];
int gSoundCondition = 1;
int gMusicCondition = 1;
int gPausedGame = 0;
int gHeroNewY = 0;
int isMouseDown = 0;
int aim = 0;
int mouseX = 0;
int mouseY = 0;
double degrees = 0;
double radians = 0;
double bulletX = 0;
double bulletY = 0;
double sinRadians = 0;
double cosRadians = 0;
int numFrames = 0;
int startTime = 0;
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
SDL_Window *window = NULL;
// The window renderer
SDL_Renderer *renderer = NULL;
/* The surface contained by the window */
/* The surface we'll be displaying the menu */
SDL_Texture *mainMenu;
SDL_Texture *gameTitle;
/* Current displayed bullet image */
SDL_Texture *gHandSurface = NULL;
/* block' surface */
SDL_Texture *canvas;
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
SDL_Texture *skullBubble;
SDL_Texture *runBubble;
/* bar's surface */
SDL_Texture *heroText;
SDL_Texture *extraBulletText;
/*sounds */
Mix_Chunk *hitSound;
Mix_Chunk *destroySound;
Mix_Chunk *extraSound;
Mix_Chunk *laserSound;
Mix_Chunk *gameMusic;
/* fonts */
TTF_Font *font20;
TTF_Font *font42;
TTF_Font *font28;

SDL_Texture *pauseButton;

TEXT_TEXTURE txtText1;
TEXT_TEXTURE txtText2;
TEXT_TEXTURE txtText3;
SDL_Texture *scrnText;
SDL_Texture *btnsText;
SDL_Texture *sndOnText;
SDL_Texture *mscOffText;
// The color of the font
SDL_Color textColor = {255, 255, 255, 255};


OBJECT *bullet;
OBJECT hero;
SDL_Point aimPnt;
ENEMY block[ROWS][COLUMNS];
OBJECT bullets[200];
SDL_Point handPnt = {0, HAND_H / 2};
SDL_Rect handDstRect = {0, 0, HAND_W, HAND_H};
SDL_Rect bulletRect = {0, 0, BULLET_S, BULLET_S};
SDL_Rect warningDstRects[COLUMNS];
SDL_Rect dstBg1 = {0, 0, BG_W, WINDOW_H};
SDL_Rect dstBg2 = {BG_W, 0, BG_W, WINDOW_H};
SDL_Rect bubbleDstRect = {0, 0, WINDOW_W, WINDOW_H};

SDL_Rect playDstRect = {WINDOW_W / 2 - 68 / 2, 446, 68, 68};
SDL_Rect exitDstRect = {214, 463, BUTTON_S, BUTTON_S};
SDL_Rect sndDstRect = {292, 463, BUTTON_S, BUTTON_S};
SDL_Rect mscDstRect = {368, 463, BUTTON_S, BUTTON_S};
SDL_Rect recordsDstRect = {548, 463, BUTTON_S, BUTTON_S};
SDL_Rect marketDstRect = {624, 463, BUTTON_S, BUTTON_S};
SDL_Rect helpDstRect = {700, 463, BUTTON_S, BUTTON_S};

int i, hitAreaY;
int walk = 0;
int bulletIconY = WINDOW_H - MARGIN * 3 + BULLET_S / 2;
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
int warningAlpha = 15;

void drawCharacter() {
  SDL_Rect srcRect = {characterFrame * HERO_W, 0, HERO_W, HERO_H};
  SDL_Rect dstRect = {hero.posX, hero.posY, HERO_W, HERO_H};
  if (gPausedGame == 0) {
    if (hero.stepY || (gameFrame < 0 && gameFrame > -600)) {
      if (characterTime % STEP_TIME == 0) {
        characterFrame = characterFrame == 2 ? 1 : 2;
      }
    } else if (characterTime % FRAME_TIME == 0) {
      characterFrame = characterFrame == 1 ? 0 : 1;
    }
    characterTime += 3;
  }
  if (gameFrame < RESET_FRAME) {
    SDL_RenderCopyEx(renderer, heroText, &srcRect, &dstRect, 0, NULL, SDL_FLIP_HORIZONTAL);
    SDL_RenderCopyEx(renderer, runBubble, NULL, &bubbleDstRect, 0, NULL, SDL_FLIP_HORIZONTAL);
  } else {
    SDL_RenderCopy(renderer, heroText, &srcRect, &dstRect);
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
      if(block->time == -480 || block->time == -321 || block->time == -81) {
        animateEnemyEnd(block, 200);
      }
      else if(block->time == -462 || block->time == -342 || block->time == -60) {
        animateEnemyEnd(block, 150);
      }
      else if(block->time == -441 || block->time == -360 || block->time == -42) {
        animateEnemyEnd(block, 100);
      }
      else if(block->time == -420 || block->time == -381 ||  block->time == -21) {
        animateEnemyEnd(block, 50);
      }
      else if(block->time == -300) {
        animateEnemyEnd(block, 255);
      }
    }
    else if (block->time % FRAME_TIME == 0) {
      block->frame = block->frame == 1 ? 0 : 1;
    }

    block->time += 3;
  }
  SDL_RenderCopy(renderer, block->sprite, &frame, &dstBlock);
  if (block->frame != 3) {
    SDL_RenderCopy(renderer, block->textTexture, NULL, &txtDstRect);
  }
}

void setTextTexture(TEXT_TEXTURE *txtText, TTF_Font *font, char *text) {
  // Render text surface
  SDL_Surface *textSurface = TTF_RenderText_Solid(font, text, textColor);
  SDL_DestroyTexture(txtText->txtText);
  // Create texture from surface pixels
  txtText->txtText = SDL_CreateTextureFromSurface(renderer, textSurface);
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
  sprintf(bulletsText, "%d/%d", n, bulletsLoaded);
  setTextTexture(&txtText2, font20, bulletsText);
  txtText2.txtDstRect.x = MARGIN * 2 + BULLET_S;
  txtText2.txtDstRect.y = WINDOW_H - MARGIN - txtText2.txtDstRect.h;
}

void setZombieTextTexture(ENEMY *block) {
  SDL_DestroyTexture(block->textTexture);
  sprintf(block->lifeLabel, "%d", block->resistance);
  SDL_Surface *textSurface = TTF_RenderText_Solid(font20, block->lifeLabel, textColor);

  block->txtDstRect.w = textSurface->w;
  block->txtDstRect.h = textSurface->h;
  block->txtDstRect.x = ENEMY_W + HITAREA_X - textSurface->w / 2;
  // Create texture from surface pixels
  block->textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
  // Get rid of the obsolete surface
  SDL_FreeSurface(textSurface);
}

void playSound(Mix_Chunk* sound) {
  if(gSoundCondition) {
    Mix_PlayChannel(-1, sound, 0);
  }
}

void reflectX(OBJECT *bullet, ENEMY *block, int movingRight) {
  bullet->stepX *= -1;
  bullet->posX = block->posX + (movingRight ? HITAREA_W - BULLET_S : HITAREA_W * 2);
}

void reflectY(OBJECT *bullet, ENEMY *block, int movingBottom) {
  bullet->stepY *= -1;
  bullet->posY = movingBottom ? block->posY - BULLET_S : block->posY + ENEMY_H;
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
    b->time = allDead ? -501 : -99;
    playSound(destroySound);
  }
  else {
    b->time = 1;
    playSound(hitSound);
  }
  b->frame = 2;
  updatePoints(1);
}

void collide(OBJECT *bullet) {
  int movingRight = bullet->stepX > 0;
  int movingBottom = bullet->stepY >= 0;
  int left = bullet->posX / HITAREA_W - 1;
  int right = (bullet->posX + BULLET_S) / HITAREA_W - 1;
  int top = bullet->posY / ENEMY_H;
  int bottom = (bullet->posY + BULLET_S) / ENEMY_H;

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
      reflectX(bullet, &block[x1][top], movingRight);
    }
  } else if (left == right) {
    if (movingBottom) {
      y1 = bottom;
    }
    if (block[left][y1].resistance) {
      damage(left, y1);
      reflectY(bullet, &block[left][y1], movingBottom);
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
        reflectX(bullet, &block[x1][y1], movingRight);
        reflectY(bullet, &block[x1][y1], movingBottom);
        return;
      }
    }
    if (block[x2][y1].resistance) {
      damage(x2, y1);
      reflectY(bullet, &block[x2][y1], movingBottom);
    }
    if (block[x1][y2].resistance) {
      damage(x1, y2);
      reflectX(bullet, &block[x1][y2], movingRight);
    }
  }
}

void drawSprite(int x, int y, int armsSpriteY, int eyesSpriteX, int mouthSpriteX, int hairSpriteX, int hairSpriteY, int browsSpriteX, int pantsSpriteX) {
  SDL_Rect srcRect = {pantsSpriteX, 0, ENEMY_W, 92};
  SDL_Rect dstRect = (SDL_Rect){x + HITAREA_X, y + 86, HITAREA_W, 8};
  dstRect = (SDL_Rect){x, 0, ENEMY_W, 92};
  SDL_RenderCopy(renderer, zombieLegs, &srcRect, &dstRect);
  dstRect.y = y;
  srcRect.x = 0;
  SDL_RenderCopy(renderer, zombieTorso, &srcRect, &dstRect);
  srcRect.x = pantsSpriteX;
  SDL_RenderCopy(renderer, zombiePants, &srcRect, &dstRect);
  srcRect.x = eyesSpriteX;
  SDL_RenderCopy(renderer, zombieEyes, &srcRect, &dstRect);
  srcRect.x = mouthSpriteX;
  SDL_RenderCopy(renderer, zombieMouth, &srcRect, &dstRect);
  dstRect.y += armsSpriteY;
  srcRect.x = 0;
  SDL_RenderCopy(renderer, zombieArms, &srcRect, &dstRect);
  dstRect.y -= armsSpriteY;
  srcRect.x = browsSpriteX;
  SDL_RenderCopy(renderer, zombieBrows, &srcRect, &dstRect);
  srcRect.h = dstRect.h = 40;
  srcRect.x = hairSpriteX;
  srcRect.y = hairSpriteY;
  SDL_RenderCopy(renderer, zombieHair, &srcRect, &dstRect);
}

SDL_Texture *createEmptySprite(int w, int h) {
  SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);
  SDL_SetRenderTarget(renderer, texture);

  // will make pixels with alpha 0 fully transparent
  // use SDL_SetTextureBlendMode . Not SDL_SetRenderDrawBlendMode
  SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
  return texture;
}

void createExtraBulletSprite(int w, int h) {
  SDL_Rect bulletRect = {ENEMY_W * 3 + HITAREA_X + BULLET_S, ENEMY_H / 2 + 4 + BULLET_S, BULLET_S, BULLET_S};
  extraBulletText = createEmptySprite(ENEMY_W * 4, ENEMY_H);

  SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0xFF, 255);
  SDL_RenderFillRect(renderer, &bulletRect);
  setTextTexture(&txtText3, font20, "+1");
  txtText3.txtDstRect.x = ENEMY_W * 3 + BULLET_S * 4;
  txtText3.txtDstRect.y = ENEMY_H / 2 + BULLET_S;
  SDL_RenderCopy(renderer, txtText3.txtText, NULL, &txtText3.txtDstRect);
  SDL_SetRenderTarget(renderer, canvas);
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

  int armsSpriteX = 2;
  ENEMY block = {1, 0, posX, posY, level + (rand() % 3) * 10 * (level / 10)};

  block.txtDstRect = (SDL_Rect){0, posY + ENEMY_H - 24, 0, 0};
  setZombieTextTexture(&block);
  block.sprite = createEmptySprite(ENEMY_W * 4, ENEMY_H);

  drawSprite(0, 0, 0, eyesSpriteX, mouthSpriteX, hairSpriteX, hairSpriteY, browsSpriteX, pantsSpriteX);
  drawSprite(ENEMY_W, 3, armsSpriteX, eyesSpriteX, mouthSpriteX, hairSpriteX, hairSpriteY, browsSpriteX, pantsSpriteX);
  drawSprite(ENEMY_W * 2, 3, 0, blinkSpriteX, mouthSpriteX, hairSpriteX, hairSpriteY, browsSpriteX, pantsSpriteX);

  SDL_RenderCopy(renderer, extraBulletText, NULL, NULL);

  SDL_SetRenderTarget(renderer, canvas);
  return block;
}

int moveObject(OBJECT *p) {
  if (p->posX < 0) {
    p->stepX = 0;
    p->stepY = 0;
    return 1;
  }

  if (p->posX + BULLET_S > FIELD_X) {
    p->stepX = -p->stepX;
  }

  if (p->posY + BULLET_S > WINDOW_H || p->posY < 0) {
    p->stepY = -p->stepY;
  }

  p->posX += p->stepX;
  p->posY += p->stepY;

  return 0;
}

void moveHero(OBJECT *p) {
  if ((p->posY + ENEMY_H >= WINDOW_H && p->stepY > 0) ||
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
    window = SDL_CreateWindow("Zombie Breakout", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_W, WINDOW_H, SDL_WINDOW_RESIZABLE);
    if (window == NULL) {
      printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
      success = 0;
    } else {
      // Create renderer for window
      renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
      if (renderer == NULL) {
        printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
        success = 0;
      } else {

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
    newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
    if (newTexture == NULL) {
      printf("Unable to create texture from %s! SDL Error: %s\n", path,
             SDL_GetError());
    }

    // Get rid of old loaded surface
    SDL_FreeSurface(loadedSurface);
  }

  return newTexture;
}

Mix_Chunk *loadWAV(const char *filepath) {
  SDL_RWops *fileData = SDL_RWFromFile(filepath, "rb");
  if(!fileData) {
    printf("SDL_RWFromFile: %s\n", SDL_GetError());
    return NULL;
  }
  
  Mix_Chunk *chunk = Mix_LoadWAV_RW(fileData, 1);
  if(!chunk){
    printf("Mix_LoadWAV_RW: %s\n", Mix_GetError());
    SDL_FreeRW(fileData);
    return NULL;
  }
  
  return chunk;
}

void loadMedia() {
  /* Load menu surface */
  mainMenu = loadTexture("./assets/images/menu-11-menu.png");
  gameTitle = loadTexture("assets/images/menu-11-title.png");
  pauseButton = loadTexture("assets/images/menu-11-pause.png");
  sndOnText = loadTexture("assets/images/menu-11-sound-on.png");
  mscOffText = loadTexture("assets/images/menu-11-music-off.png");

  /* load bar surface */
  heroText = loadTexture("assets/images/character-8.png");
  gHandSurface = loadTexture("assets/images/character-hand-2.png");
  warningBubble = loadTexture("assets/images/warning-3.png");
  skullBubble = loadTexture("assets/images/danger-3.png");
  runBubble = loadTexture("assets/images/run-3.png");

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
  hitSound = loadWAV("assets/sounds/hit.ogg");
  destroySound = loadWAV("assets/sounds/destroy.ogg");
  laserSound = loadWAV("assets/sounds/laser.ogg");
  extraSound = loadWAV("assets/sounds/extra.ogg");
  gameMusic = loadWAV("assets/sounds/music.ogg");
  

  /*load fonts*/
  // TTF_Font *font;
  font20 = TTF_OpenFont("assets/images/visitor1.ttf", 20);
  font28 = TTF_OpenFont("assets/images/visitor1.ttf", 28);
  font42 = TTF_OpenFont("assets/images/visitor1.ttf", 42);

  createExtraBulletSprite(ENEMY_W * 4, ENEMY_H);
  canvas = createEmptySprite(WINDOW_W, WINDOW_H);
}

void renderXCenteredText(TTF_Font *font, char string[], int y) {
  setTextTexture(&txtText3, font, string);
  txtText3.txtDstRect.x = (WINDOW_W - txtText3.txtDstRect.w) / 2;
  txtText3.txtDstRect.y = y;
  SDL_RenderCopy(renderer, txtText3.txtText, NULL, &txtText3.txtDstRect);
}

void showMarket() {
  int textY = WINDOW_H / 4;
  scrnText = createEmptySprite(SPRITE_W, WINDOW_H);
  renderXCenteredText(font42, "MARKET", textY - 42 * 2);
  renderXCenteredText(font28, "Market is currently closed!", textY + 4 * 28);
  SDL_SetRenderTarget(renderer, canvas);
}

void showHelp() {
  int textY = WINDOW_H / 4;
  scrnText = createEmptySprite(SPRITE_W, WINDOW_H);
  renderXCenteredText(font42, "HOW TO PLAY", textY - 42 * 2);
  renderXCenteredText(font28, "1. Point and tap to move you character.", textY + 2 * 28);
  renderXCenteredText(font28, "2. Tap and hold down to aim at the target.",  textY + 4 * 28);
  renderXCenteredText(font28, "3. Release to shoot.", textY + 6 * 28);
  renderXCenteredText(font28, "4. Defend the city against hordes of zombies.", textY + 8 * 28);
  SDL_SetRenderTarget(renderer, canvas);
}

void showPaused() {
  int textY = WINDOW_H / 4;
  scrnText = createEmptySprite(SPRITE_W, WINDOW_H);
  renderXCenteredText(font42, "Pause", textY - 42 * 2);
  SDL_SetRenderTarget(renderer, canvas);
}

void syncRecords() {
  #if __EMSCRIPTEN__
  EM_ASM(
    FS.syncfs(false, function() {
      Module.print("File sync complete.")
    });
  );
  #endif
}

void listRecords() {
  int i;
  RECORD records[5];
  FILE *file = fopen("IDBFS/records.bin","rb");
  int empty = 1;
  char entry[50];
  int textY = WINDOW_H / 4;

  scrnText = createEmptySprite(SPRITE_W, WINDOW_H);
  renderXCenteredText(font42, "TOP RANKINGS", textY - 42 * 2);

  if(file) {
    fread(records, sizeof(RECORD), 5, file);
    for (i = 0; i < 5; i++) {
      if (records[i].points > 0) {
        empty = 0;
        // counts number of digits in an int
        sprintf(entry, "%s pts: %d", records[i].name, records[i].points);
        renderXCenteredText(font28, entry, textY + i * 2 * 28);
      }
    }
    fclose(file);
    syncRecords();
  }

  if (empty) {
    renderXCenteredText(font28, "No records found!", textY + 4 * 28);
  }
  SDL_SetRenderTarget(renderer, canvas);
}

void setRank() {
  FILE *file = fopen("IDBFS/records.bin", "rb");
  RECORD player;
  RECORD records[5];
  RECORD aux;
  int i = 0;
  int textY = WINDOW_H / 4;
  char pts[12] = "PTS: ";
  char timeStr[25];
  time_t t = time(NULL);
  struct tm *tm = localtime(&t);

  strcat(pts, pointsText);
  scrnText = createEmptySprite(SPRITE_W, WINDOW_H);
  renderXCenteredText(font42, "GAME OVER", textY - 42 * 2);
  renderXCenteredText(font28, pts, textY + 4 * 28);
  SDL_SetRenderTarget(renderer, canvas);

  gPausedGame = 0;

  strftime(timeStr, sizeof(timeStr), "%c", tm);
  strcpy(player.name, timeStr);

  player.points = gPoints;

  if(file == 0) {
    file = fopen("IDBFS/records.bin", "wb");
    if (player.points) {
      records[0] = player;
      for (i = 1; i < 5; i++) {
        records[i] = (RECORD){0};
      }
    }
  }
  else {
    fread(records, sizeof(RECORD), 5, file);
    fclose(file);
    file = fopen("IDBFS/records.bin", "wb");
    /* Searches for values in the top 5 ranks that are lower than the
     the new player score. */
    if (player.points > records[4].points) {
      records[4] = player;
      for (i = 4; i > 0; i--) {
        if (records[i].points > records[i - 1].points) {
          aux = records[i];
          records[i] = records[i - 1];
          records[i - 1] = aux;
        }
      }
    }
  }
  
  fwrite(records, sizeof(RECORD), 5, file);
  fclose(file);
  syncRecords();
}

void updateMenuTexture() {
  btnsText = createEmptySprite(SPRITE_W, WINDOW_H);
  if (gSoundCondition) {
    SDL_RenderCopy(renderer, sndOnText, NULL, NULL);
  }
  if (!gMusicCondition) {
    SDL_RenderCopy(renderer, mscOffText, NULL, NULL);
  }
  SDL_SetRenderTarget(renderer, canvas);
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
  return mouseX >= button.x && mouseX <= button.x + button.w &&
         mouseY >= button.y && mouseY <= button.y + button.h;
}

void playMusic() {
  if (gMusicCondition) {
    if (Mix_Paused(1)) {
      Mix_Resume(1);
    } else {
      Mix_PlayChannel(1, gameMusic, -1);
    }
  }
}

void setAngle() {
  double maxRadians = 0;
  radians = atan2(mouseY - handDstRect.y - handDstRect.h / 2, mouseX - handDstRect.x);

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

  bulletY = sinRadians * BULLET_SPEED;
  bulletX = cosRadians * BULLET_SPEED;

  gameFrame = 2;

  if(aim) {
    walk = 0;
  }
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
  int i, j;
  int randomColumn = rand() % 6;
  hero = (OBJECT){-HITAREA_W - 45, WINDOW_H / 2 - HERO_W / 2, 0, 0};
  //gameFrame = RESET_FRAME;
  characterFrame = 0;
  characterTime = 0;
  handPvtPntY = 0;

  gPoints = 0;
  
  firstRow = 0;
  warningFrame = 0;
  level = 1;
  bulletsLoaded = 1;
  killedInColumn = 0;
  lastKilled = 0;

  bulletsInTheMagazine = 0;
  shotInterval = 0;
  enemyInColumn = 0;
  hasGameStarted = 0;

  gPausedGame = 0;

  dstPauseButton = (SDL_Rect){WINDOW_W - BUTTON_S - MARGIN, 16, BUTTON_S, BUTTON_S};

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
    block[i][j] = rand() % 5 == 0 ? createEnemy(HITAREA_W * ROWS, ENEMY_H * j) : (ENEMY){0};
  }
  block[i][randomColumn] = createEnemy(HITAREA_W * ROWS, ENEMY_H * randomColumn);
}

void interpolateFinger() {
  int w, h;
  SDL_GetWindowSize(window, &w, &h);
  mouseX = (float)(WINDOW_W) * e.tfinger.x;
  mouseY = (float)(WINDOW_H) * e.tfinger.y;
}

void interpolateMouse() {
  int w, h;
  SDL_GetWindowSize(window, &w, &h);
  mouseX = (float)(WINDOW_W) / w * e.motion.x;
  mouseY = (float)(WINDOW_H) / h * e.motion.y;
  printf("Finger %d %d\n", mouseX, mouseY);
}

void handleDown(){
  if (gPausedGame == 0 && hasGameStarted) {
    if (clickButton(e, dstPauseButton)) {
      gPausedGame = 1;
      showPaused();
      if (gMusicCondition) {
        Mix_Pause(1);
      }
    }
    else if (gameFrame == shotInterval || gameFrame < 3) {
      isMouseDown = 1;
      walk = 1;
    }
  }
}

void handleButtons() {

  switch (e.type) {
    case SDL_QUIT:
      gQuit = 1;
      break;
    case SDL_FINGERDOWN:
      interpolateFinger();
      handleDown();
      break;
    case SDL_MOUSEBUTTONDOWN:  // if the event is mouse click
      interpolateMouse();
      handleDown();
      break;
    case SDL_FINGERUP:
    case SDL_MOUSEBUTTONUP:
      if (gPausedGame == 0 && hasGameStarted) {
        if(walk) {
          gHeroNewY = mouseY - HERO_HAND_Y;
          hero.stepY = gHeroNewY > hero.posY ? STEP_Y : -STEP_Y;
        }
        isMouseDown = 0;
        break;
      }
      if (clickButton(e, exitDstRect)) {
        if (hasGameStarted == 0) {
          if(scrnText) {
            scrnText = NULL;
          }
          else {
            gQuit = 1;
          }
        } else if (hasGameStarted == 1 && gPausedGame) {
          gameFrame = -999;
          gPausedGame = 0;
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
    case SDL_FINGERMOTION:
      if (isMouseDown && gameFrame > -1 && gameFrame < 3) {
        interpolateFinger();
        setAngle();
      }
      break;
    case SDL_MOUSEMOTION:
      if (isMouseDown && gameFrame > -1 && gameFrame < 3) {
        interpolateMouse();
        setAngle();
      }
      break;
  }
}

void printFps() {
  int elapsedMS = SDL_GetTicks() - startTime; // Time since start of loop
  ++numFrames;
  if (elapsedMS) { // Skip this the first frame
    double elapsedSeconds = elapsedMS / 1000.0; // Convert to seconds
    double fps = numFrames / elapsedSeconds; // FPS is Frames / Seconds
    printf("%s %f\n", "Frame rate: ", fps);
  }
}

void tick() {
  int i, j;
  ENEMY enemy;
  bubbleDstRect = (SDL_Rect){0, hero.posY, WINDOW_W, WINDOW_H};

  //printFps();

  SDL_RenderClear(renderer);
  SDL_SetRenderTarget(renderer, canvas);
  SDL_RenderCopy(renderer, stageBackground, NULL, &dstBg1);
  SDL_RenderCopy(renderer, stageBackground, NULL, &dstBg2);

  /* verifies if any key have been pressed */
  while (SDL_PollEvent(&e) != 0) {
    handleButtons();
  }

  if(firstRow) {
    for (j = 0; j < COLUMNS; j++) {
      enemy = block[firstRow][j];
      warningDstRects[j] = enemy.resistance > 0 ? (SDL_Rect){enemy.posX + HITAREA_W, enemy.posY + 80, HITAREA_W, 20} : (SDL_Rect){0};
    }
    if(warningFrame == 93) {
      warningAlpha = 20;
    }
    else if(warningFrame == 3 || warningFrame == 90) {
      warningAlpha = 40;
    }
    else if(warningFrame == 6 || warningFrame == 87) {
      warningAlpha = 60;
    }
    else if(warningFrame == 9 || warningFrame == 84) {
      warningAlpha = 80;
    }
    else if(warningFrame == 12 || warningFrame == 81) {
      warningAlpha = 100;
    }
    else if(warningFrame == 15) {
      warningAlpha = 120;
    }
    
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);   

    if(gameFrame > RESET_FRAME) {
      if(firstRow == 2) {
        SDL_SetRenderDrawColor(renderer, 255, 153, 0, warningAlpha);
        SDL_RenderCopy(renderer, skullBubble, NULL, &bubbleDstRect);
      }
      else {
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, warningAlpha);
        SDL_RenderCopy(renderer, warningBubble, NULL, &bubbleDstRect);
      }
      if(warningFrame == 96) {
        firstRow = warningFrame = 0;
      }
    }
    else {
      SDL_SetRenderDrawColor(renderer, 255, 0, 0, warningAlpha);
    }

    SDL_RenderFillRects(renderer, warningDstRects, COLUMNS);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    warningFrame ++;
  }

  SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0xFF, 255);

  for (j = 0; j < COLUMNS; j++) {
    for (i = 2; i < ROWS; i++) {
      if (block[i][j].time) {
        drawEnemy(&block[i][j]);
      }
      if (block[i][j].time == -399) {
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
      walk = hero.stepY = 0;
    }
    if (gameFrame > 0) {
      if (gameFrame == 2) {
        if (aim) {
          // the coordinates of Y in world coordinates
          handPvtPntY = handDstRect.y + handDstRect.h / 2;
          aimPnt = getRulerCorners(handDstRect.x, handPvtPntY, handDstRect.w / 2, -2);
          SDL_RenderDrawLine(renderer, aimPnt.x, aimPnt.y, mouseX, mouseY);
          SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

          if (isMouseDown == 0) {
            gameFrame = 3;
            shotInterval = bulletsLoaded * BULLET_GAP + 1;
          }
        }
      } else if (gameFrame < shotInterval) {
        if (gameFrame % BULLET_GAP == 0) {
          bullet = &bullets[gameFrame / BULLET_GAP - 1];
          aimPnt = getRulerCorners(handDstRect.x, handPvtPntY, handDstRect.w, -12);
          bullet->posY = aimPnt.y;
          bullet->posX = aimPnt.x;
          bullet->stepY = bulletY;
          bullet->stepX = bulletX;
          updateBullets(bulletsLoaded - gameFrame / BULLET_GAP);
          playSound(laserSound);
        }
        gameFrame++;
      }

      if (gameFrame > 2) {
        for (i = 0; i < bulletsLoaded; i++) {
          bulletsInTheMagazine += moveObject(&bullets[i]);
          if (bullets[i].posX > 0) {
            collide(&bullets[i]);
          }
        }
      }

      if (gameFrame == shotInterval) {
        if (bulletsInTheMagazine == bulletsLoaded) {
          for (j = 0; j < COLUMNS; j++) {
            if (block[2][j].resistance) {
              gameFrame = -999;
              firstRow = 2;
              #if __EMSCRIPTEN__
                Mix_HaltChannel(-1);
              #else
                Mix_FadeOutChannel(1, 999 * 2);
              #endif
              setRank();
              break;
            };
          }
          if (gameFrame != -999) {
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
                firstRow = i - 1;
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
            }
          }
        }
        bulletsInTheMagazine = 0;
      }
    } else if (gameFrame < -1) {
      gameFrame += 3;
      if (gameFrame < -600) {
        dstBg1.x += 6;
        dstBg2.x += 6;
        hero.posX += 6;
        handDstRect.x = hero.posX - 65;
        bubbleDstRect.x = handDstRect.x - 800;

        for (i = 2; i < ROWS; i++) {
          for (j = 0; j < COLUMNS; j++) {
            block[i][j].posX += 6;
          }
        }
        if (dstBg1.x > WINDOW_W - BG_W) {
          dstBg2.x = dstBg1.x - BG_W;
        }
        if (dstBg2.x > WINDOW_W - BG_W) {
          dstBg1.x = dstBg2.x - BG_W;
        }
      }
      else if(gameFrame == -600) {
        gHeroNewY = WINDOW_H / 2;
        hero.stepY = gHeroNewY > hero.posY ? 1 : -1;
      }
      else if (gameFrame > -HITAREA_W - 1) {
        dstBg1.x-=3;
        dstBg2.x-=3;
        if (dstBg1.x < 0) {
          dstBg2.x = dstBg1.x + BG_W;
        }
        if (dstBg2.x < 0) {
          dstBg1.x = dstBg2.x + BG_W;
        }
        for (i = 2; i < ROWS; i++) {
          for (j = 0; j < COLUMNS; j++) {
            block[i][j].posX-=3;
          }
        }
      } else if (gameFrame < RESET_FRAME) {
        hero.posX -= STEP_Y;
        handDstRect.x = hero.posX - 65;
        bubbleDstRect.x = handDstRect.x - 800;
      } else if (gameFrame == RESET_FRAME) {
        reset();
        return;
      } else if (gameFrame > RESET_FRAME) {
        hero.posX +=3;
        handDstRect.x = hero.posX + 72;
      }

    } else if (gameFrame == 0) {
      updateBullets(bulletsLoaded);
    }
  }

  moveHero(&hero);

  handDstRect.y = hero.posY + 35;

  if (gameFrame < RESET_FRAME) {
    SDL_RenderCopyEx(renderer, gHandSurface, NULL, &handDstRect, 0, &handPnt, SDL_FLIP_HORIZONTAL);
  } else {
    SDL_RenderCopyEx(renderer, gHandSurface, NULL, &handDstRect, degrees, &handPnt, SDL_FLIP_NONE);
  }

  drawCharacter();

  if (gameFrame > 2) {
    for (i = 0; i < bulletsLoaded; i++) {
      if (bullets[i].posX > 0) {
        bulletRect.x = bullets[i].posX;
        bulletRect.y = bullets[i].posY;
        SDL_RenderFillRect(renderer, &bulletRect);
      }
    }
  }

  if (scrnText) {
    SDL_RenderCopy(renderer, scrnText, NULL, NULL);
    if (gameFrame > RESET_FRAME) {
      SDL_RenderCopy(renderer, mainMenu, NULL, NULL);
      SDL_RenderCopy(renderer, btnsText, NULL, NULL);
    }
  } else if (hasGameStarted == 0) {
    SDL_RenderCopy(renderer, gameTitle, NULL, NULL);
    SDL_RenderCopy(renderer, mainMenu, NULL, NULL);
    SDL_RenderCopy(renderer, btnsText, NULL, NULL);
  } else {
    SDL_RenderCopy(renderer, txtText1.txtText, NULL, &txtText1.txtDstRect);
    SDL_RenderCopy(renderer, txtText2.txtText, NULL, &txtText2.txtDstRect);
    bulletRect.x = MARGIN;
    bulletRect.y = bulletIconY;
    SDL_RenderFillRect(renderer, &bulletRect);
    if (gPausedGame) {
      SDL_RenderCopy(renderer, mainMenu, NULL, NULL);
      SDL_RenderCopy(renderer, btnsText, NULL, NULL);
    } else {
      SDL_RenderCopy(renderer, pauseButton, NULL, NULL);
    }
  }

  SDL_SetRenderTarget(renderer, NULL);
  SDL_RenderCopy(renderer, canvas, NULL, NULL);
  SDL_RenderPresent(renderer);
}

void loop() {
  startTime = SDL_GetTicks();
#if __EMSCRIPTEN__
  EM_ASM(
    FS.mkdir('IDBFS');
    FS.mount(IDBFS, {}, 'IDBFS');

    //populate persistent_data directory with existing persistent source data 
    //stored with Indexed Db
    //first parameter = "true" mean synchronize from Indexed Db to 
    //Emscripten file system,
    // "false" mean synchronize from Emscripten file system to Indexed Db
    //second parameter = function called when data are synchronized
    FS.syncfs(true, function() {
      Module.print("IDBFS initialized.");
    });
  );

  emscripten_set_main_loop(tick, 0, 0);
#else
  /* Starts game main loop */
  while (!gQuit) {
    SDL_Delay(SDL_DELAY);
    tick();
  }
#endif
}

void quit() {
  //Destroy window
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  //Quit SDL subsystems
  TTF_Quit();
  IMG_Quit();
  SDL_Quit();
}

int main() {
  /* Start up SDL and create window */
  if (!init()) {
    printf("SDL could not be initialized\n");
  } else {
    loadMedia();
    reset();
    loop();
  }
  #if !__EMSCRIPTEN__
    quit();
  #endif
  
  return 0;
}
