#ifndef ROCKET_H
#define ROCKET_H

#include "main.h"

typedef enum { 
    WAITING, 
    LAUNCHING, 
    INCREASE_MULTIPLIER, 
    GAME_OVER 
} RocketGameState;

typedef struct {
    int bet;
    float multiplier;
    float y;
    bool cashedOut;
    RocketGameState state;
    float randomMultiplier;
} RocketGame;

void ResetRocketGame(RocketGame* game);

#endif
