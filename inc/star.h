#ifndef STAR_H
#define STAR_H

#include "main.h"

#define GRID_SIZE 4
#define CELL_SIZE 100
#define MINE_COUNT 5
#define MENU_WIDTH 200

typedef struct {
    bool isMine;
    bool isRevealed;
    float multiplier;
    int nearbyMines;
    bool isHovered;
} Cell;

typedef struct {
    Cell grid[GRID_SIZE][GRID_SIZE];
    int currentBet;
    int currentWin;
    bool gameOver;
    bool gameWon;
    bool betPlaced;
    int selectedBetIndex;
    int betValues[10];
} StarGame;

void InitializeStarGame(StarGame* game);
void RevealCell(StarGame* game, int x, int y);

#endif
