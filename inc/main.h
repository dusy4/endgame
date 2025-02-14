#ifndef MAIN_H
#define MAIN_H

#include "raylib.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// Global variables
extern int globalBalance;

// Game states
typedef enum {
    MAIN_MENU,
    CREDITS,
    MAIN_GAME,
    ROCKET_GAME,
    STAR_GAME,
    HIPPODROME_GAME,
    SLOTS_GAME
} GameState;

extern GameState currentGame;

// Common utility functions
void drawGlobalBalance(int globalBalance);
Rectangle getRect(Vector2 pos, int width, int height);
void DrawHitbox(Vector2 pos, int width, int height, bool isColliding);
bool checkAllCollisions(Rectangle playerRec, Rectangle squares[], int squareCount);
void collidingController(Vector2 LookVect, Vector2* pos, Rectangle* playerRec, float speed);

#endif
