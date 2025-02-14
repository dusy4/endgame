#include "raylib.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

int globalBalance = 1000; 

// Game states
typedef enum {
    MAIN_MENU,
    CREDITS,
    MAIN_GAME,
    ROCKET_GAME,
    STAR_GAME,
    HIPPODROME_GAME,
    SLOTS_GAME  // Added new state
} GameState;

GameState currentGame = MAIN_GAME;

// Rocket game specific structs and variables
typedef enum { WAITING, LAUNCHING, INCREASE_MULTIPLIER, GAME_OVER } RocketGameState;

typedef struct {
    int bet;
    float multiplier;
    float y;
    bool cashedOut;
    RocketGameState state;
    float randomMultiplier;
} RocketGame;

typedef struct {
int index;
double probability;
} Horse;

void drawGlobalBalance(int globalBalance) {
    const int padding = 20;
    const char* balanceText = TextFormat("Balance: %d$", globalBalance);
    const int fontSize = 30;
    const int textWidth = MeasureText(balanceText, fontSize);
    
    DrawText(balanceText, 100 + textWidth + padding, padding, fontSize, WHITE);
}

#define NUM_SYMBOLS 6
#define NUM_REELS 3
#define ANIMATION_DURATION 180  // 3 seconds at 60 FPS
// #define RED              (Color){ 230, 41, 55, 255 }      // Red
#define SPIN_COOLDOWN 30        // 1 second at 30 FPS

typedef struct {
    Texture2D* frames;
    int numFrames;
    int currentFrame;
    int frameTimer;
    int frameDuration;
    bool isAnimating;
    int animationTimer;
    int width;
    int height;
} Animation;

Color GOLDEN_COLOR = { 255, 215, 0, 255 };

Texture2D LoadSlotTexture(const char* filename) {
    return LoadTexture(filename);
}

int CalculatePayout(int* reels, int betAmount, bool* jackpotTriggered, bool* freeSpinsTriggered, bool* doubleWinTriggered, bool* otherWinTriggered) {
    int payout = 0;
    *jackpotTriggered = false;
    *freeSpinsTriggered = false;
    *doubleWinTriggered = false;
    *otherWinTriggered = false;

    if (reels[0] == reels[1] && reels[1] == reels[2]) {
        switch (reels[0]) {
        case 0:  // 1.png (jackpot)
            payout = 1000 * betAmount;
            *jackpotTriggered = true;
            break;
        case 1:  // 2.png (free spins)
            payout = 500 * betAmount;
            *freeSpinsTriggered = true;
            break;
        case 2:  // 3.png (double win)
            payout = 250 * betAmount;
            *doubleWinTriggered = true;
            break;
        case 3:  // 4.png (other win)
            payout = 125 * betAmount;
            *otherWinTriggered = true;
            break;
        case 4:  // 5.png (other win)
            payout = 65 * betAmount;
            *otherWinTriggered = true;
            break;
        case 5:  // 6.png (other win)
            payout = 50 * betAmount;
            *otherWinTriggered = true;
            break;
        }
    }
    return payout;
}

void DrawAnimation(Animation* anim, int screenWidth, int screenHeight) {
    if (anim->isAnimating) {
        int x = (screenWidth - anim->width) / 2;
        int y = (screenHeight - anim->height) / 2;
        DrawTexture(anim->frames[anim->currentFrame], x, y, WHITE);
    }
}

void UpdateAnimation(Animation* anim) {
    if (anim->isAnimating) {
        anim->frameTimer++;
        if (anim->frameTimer >= anim->frameDuration) {
            anim->frameTimer = 0;
            anim->currentFrame = (anim->currentFrame + 1) % anim->numFrames;
        }

        anim->animationTimer++;
        if (anim->animationTimer >= ANIMATION_DURATION) {
            anim->isAnimating = false;
            anim->currentFrame = 0;
            anim->frameTimer = 0;
            anim->animationTimer = 0;
        }
    }
}

void DrawSlotMachine(int* reels, int width, int height, Texture2D* textures, Texture2D* waitTexture, bool gameStarted) {
    int iconWidth = 256, iconHeight = 256;
    int totalWidth = NUM_REELS * iconWidth;
    int baseStartX = (width - totalWidth) / 2;

    int reelPositions[NUM_REELS] = {
        baseStartX + 55,
        baseStartX + iconWidth - 10,
        baseStartX + (2 * iconWidth) - 80
    };

    for (int i = 0; i < NUM_REELS; i++) {
        if (gameStarted) {
            DrawTexture(textures[reels[i]], reelPositions[i], height / 2 - iconHeight / 2, WHITE);
        }
        else {
            DrawTexture(*waitTexture, reelPositions[i], height / 2 - iconHeight / 2, WHITE);
        }
    }
}

bool IsSpinZoneClicked(Rectangle* spinZone) {
    bool isHovered = CheckCollisionPointRec(GetMousePosition(), *spinZone);
    bool isClicked = isHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    return isClicked;
}

bool IsBetButtonClicked(Rectangle* buttonZone) {
    bool isHovered = CheckCollisionPointRec(GetMousePosition(), *buttonZone);
    bool isClicked = isHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    return isClicked;
}


//ôóíêöèÿ îòðèñîâêè áåòòèíãîâîãî ýêðàíà
void constructPredictionScreen(bool *isPRedictionMade, int *playerHorseIndex, int *money, bool *invalidMoneyPick, 
                             Vector2 pos, Vector2 size, Vector2 mousePoint, Color horsesColors[], 
                             int betCoefficient[], Sound clicked, Sound prediction, int *globalBalance) {
    int moneyFontSize = 42;
    int moneyX = 550;
    int moneyY = 210;
    
    DrawRectangle(0, 0, 1200, 600, DARKGRAY);
    DrawText("Choose money amount to bet\n", 300, 50, 42, WHITE);
    
    // Display global balance at top of screen
    DrawText(TextFormat("Balance: %i$", *globalBalance), 20, 20, 32, WHITE);
    
    // Adjust money display formatting based on amount
    if (*money / 1000 >= 10 && *money / 1000 < 99) {
        moneyFontSize = 40;
        moneyX -= 15;
        moneyY += 5;
    }
    else if (*money / 1000 >= 100) {
        moneyFontSize = 34;
        moneyX -= 20;
        moneyY += 5;
    }
    else {
        moneyFontSize = 42;
        moneyX = 550;
        moneyY = 210;
    }
    
    DrawText(TextFormat("%i$", *money), moneyX, moneyY, moneyFontSize, WHITE);
    
    // Less button logic - prevent betting more than global balance
    Rectangle LessButton = {390, 150, 75, 75};
    DrawRectangleRec(LessButton, RED);
    DrawLineEx((Vector2) { 395, 187.5 }, (Vector2) { 460, 187.5 }, 8.5, WHITE);
    if (CheckCollisionPointRec(mousePoint, LessButton)) {
        DrawRectangleRec((Rectangle) { LessButton.x, LessButton.y, LessButton.width * 1.1, LessButton.height * 1.1 }, RED);
        DrawLineEx((Vector2) { 395, 190 }, (Vector2) { 467.5, 190 }, 8.5, WHITE);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            PlaySound(clicked);
            if(*money > 0){
                *money -= 100;
            }
        }
    }
    
    // Validate money against global balance
    if (*money <= 0 || *money > *globalBalance) {
        *invalidMoneyPick = 1;
    }
    else {
        *invalidMoneyPick = 0;
    }
    
    // More button logic - prevent betting more than global balance
    Rectangle MoreButton = { 620, 150, 75, 75 };
    DrawRectangleRec(MoreButton, GREEN);
    DrawLineEx((Vector2) { 625, 187.5 }, (Vector2) { 690, 187.5 }, 8.5, WHITE);
    DrawLineEx((Vector2) { 625 + 33, 157.5 }, (Vector2) { 625 + 33, 217.5 }, 8.5, WHITE);
    if (CheckCollisionPointRec(mousePoint, MoreButton)) {
        DrawRectangleRec((Rectangle) { MoreButton.x, MoreButton.y, MoreButton.width * 1.1, MoreButton.height * 1.1 }, GREEN);
        DrawLineEx((Vector2) { 625, 192 }, (Vector2) { 697.5, 192 }, 8.5, WHITE);
        DrawLineEx((Vector2) { 625 + 37, 157.5 }, (Vector2) { 625 + 37, 225 }, 8.5, WHITE);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            PlaySound(clicked);
            if (*money + 100 <= *globalBalance) {
                *money += 100;
            }
        }
    }
    
    // Preset amount buttons - limit to global balance
    Rectangle OneKButton = { 715, 160, 50, 50 };
    DrawRectangleRec(OneKButton, GRAY);
    DrawText("1k", 725, 165, 42, WHITE);
    if (CheckCollisionPointRec(mousePoint, OneKButton)) {
        DrawRectangleRec((Rectangle) { OneKButton.x, OneKButton.y, OneKButton.width * 1.1, OneKButton.height * 1.1 }, GRAY);
        DrawText("1k", 725, 165, 47, WHITE);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            PlaySound(clicked);
            if (1000 <= *globalBalance) {
                *money = 1000;
            }
        }
    }
    
    Rectangle TenKButton = { 785, 155, 60, 60 };
    DrawRectangleRec(TenKButton, GRAY);
    DrawText("10k", 787, 165, 40, WHITE);
    if (CheckCollisionPointRec(mousePoint, TenKButton)) {
        DrawRectangleRec((Rectangle) { TenKButton.x, TenKButton.y, TenKButton.width * 1.1, TenKButton.height * 1.1 }, GRAY);
        DrawText("10k", 787, 165, 45, WHITE);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            PlaySound(clicked);
            if (10000 <= *globalBalance) {
                *money = 10000;
            }
        }
    }
    
    Rectangle HundredKButton = { 870, 150, 85, 70 };
    DrawRectangleRec(HundredKButton, GRAY);
    DrawText("100k", 873, 168, 40, WHITE);
    if (CheckCollisionPointRec(mousePoint, HundredKButton)) {
        DrawRectangleRec((Rectangle) { HundredKButton.x, HundredKButton.y, HundredKButton.width * 1.1, HundredKButton.height * 1.1 }, GRAY);
        DrawText("100k", 873, 168, 45, WHITE);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            PlaySound(clicked);
            if (100000 <= *globalBalance) {
                *money = 100000;
            }
        }
    }
    
    DrawText("\t\tChoose horse to bet on", 280, 265, 42, WHITE);

    // Horse selection buttons
    Rectangle horsesButtons[10];
    for (int i = 0; i < 10; i++) {
        DrawText(TextFormat("%ist place = %ix", i+1, betCoefficient[i]), 35, 80+(25*i), 28, WHITE);
        horsesButtons[i] = (Rectangle){ 285 + (65 * i), 325, 50, 50 };
        DrawRectangleRec(horsesButtons[i], horsesColors[i]);
        if (i == 9) {
            DrawText(TextFormat("%i", i + 1), 298 + ((65 * i)), 335, 32, BLACK);
        }
        else {
            DrawText(TextFormat("%i", i + 1), 305 + ((65 * i)), 335, 32, BLACK);
        }
        if (CheckCollisionPointRec(mousePoint, horsesButtons[i])) {
            DrawRectangleRec((Rectangle){ horsesButtons[i].x, horsesButtons[i].y, horsesButtons[i].width * 1.1, horsesButtons[i].height * 1.1 }, horsesColors[i]);
            DrawText(TextFormat("%i", i+1), 305 + ((65 * i)), 335, 42, BLACK);
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                if (!*invalidMoneyPick) {
                    *playerHorseIndex = i;
                    *isPRedictionMade = 1;
                    PlaySound(prediction);
                }
            }
        }
    }
    
    // Error messages
    if (*invalidMoneyPick) {
        if (*money > *globalBalance) {
            DrawText("Insufficient balance", 250, 400, 42, RED);
        } else {
            DrawText("Choose amount of money to bet", 250, 400, 42, RED);
        }
    }
}

Texture2D createHorseTexture(Image horseimg) {
    Texture2D horset = LoadTextureFromImage(horseimg);
    return horset;
}

void fillHorseTextureArr(Image horseimg, Texture2D* horseTextureArr) {
    for (int i = 0; i < 10; i++) {
        horseTextureArr[i] = createHorseTexture(horseimg);
    }
}

void fillHorsesPoses(Vector2 arr[]) {
    for (int i = 0; i < 10; i++) {
        arr[i] = (Vector2){0, 0+(i*58)};
    }
}

void RectHorses(Rectangle arr[], Vector2 pos[], Texture2D* horseTextureArr) {
    for (int i = 0; i < 10; i++) {
        arr[i] = (Rectangle){ pos[i].x, pos[i].y, horseTextureArr[i].width*0.6, horseTextureArr[i].height*0.6};
    }
    
}

void DrawHorseHB(Rectangle rectHorses, bool isColliding) {
    Color outlineColor = isColliding ? RED : DARKGRAY;
        DrawRectangleLinesEx(rectHorses, 0, RED);
}

void drawHorseArray(int *timerNumber,Vector2 pos[], float speeds[], Rectangle rectHorses[], Texture2D* horseTextureArr, Color* colorArr, float sizeMultiplier) {
    if (*timerNumber > 0) {
        *timerNumber -= 1;
    }
    else {
        for (int i = 0; i < 10; i++) {
            DrawTextureEx(horseTextureArr[i], (Vector2) { pos[i].x += speeds[i], pos[i].y }, 0, sizeMultiplier, colorArr[i]);
            DrawHorseHB((Rectangle) { rectHorses[i].x += speeds[i], rectHorses[i].y, rectHorses[i].width, rectHorses[i].height }, 0);
        }
    }
}

int find_max_index(float arr[]) {
    int max_index = 0; 
    for (int i = 1; i < 10; i++) {
        if (arr[i] > arr[max_index]) {
            max_index = i;  
        }
    }
    return max_index;
}

bool checkHorsesCollision(Rectangle rectHorses[], Rectangle finishRect, float speeds[]) {
    int maxSpeedIndex = find_max_index(speeds);
        if (CheckCollisionRecs(rectHorses[maxSpeedIndex],  finishRect)) {
            return true;
        }
        else {
            return false;
        }   
}

void generateProbabilities(double probabilities[], int fixed_index, double fixed_value) {
    double sum = 0.0;

    for (int i = 0; i < 10; i++) {
        if (i == fixed_index) continue;
        probabilities[i] = (double)rand() / RAND_MAX; 
        sum += probabilities[i];
    }

    double scale = (1.0 - fixed_value) / sum;
    for (int i = 0; i < 10; i++) {
        if (i == fixed_index) continue;
        probabilities[i] *= scale;
    }

    probabilities[fixed_index] = fixed_value;
}

void sort_horses(Horse horses[], int size) {
    for (int i = 0; i < size - 1; i++) {
        for (int j = i + 1; j < size; j++) {
            if (horses[i].probability < horses[j].probability) {
                Horse temp = horses[i];
                horses[i] = horses[j];
                horses[j] = temp;
            }
        }
    }
}

void determine_race_results(double probabilities[], int results[]) {
    Horse horses[10];

    for (int i = 0; i < 10; i++) {
        horses[i].index = i; 
        horses[i].probability = probabilities[i]; 
    }
    sort_horses(horses, 10);

    for (int i = 0; i < 10; i++) {
        results[i] = horses[i].index;
    }
}

void invert_array(int arr[]) {
    double min_val = arr[0];
    double max_val = arr[0];

    for (int i = 1; i < 10; i++) {
        if (arr[i] < min_val) min_val = arr[i];
        if (arr[i] > max_val) max_val = arr[i];
    }
    for (int i = 0; i < 10; i++) {
        arr[i] = max_val - (arr[i] - min_val);
    }
}

void calculate_speeds(int results[], float speeds[]) {
    /*for (int i = 0; i < 6; i++) {
        printf("before inversion index %i; value%i\n", i, results[i]);
    }*/
    invert_array(results);
    for (int i = 0; i < 10; i++) {
        speeds[i] = (((float)results[i]+1)*0.65);
    }
}

int getWinnerId(int results[]) {
    for (int i = 0; i < 10; i++) {
        if (results[i] == 0) {
            return i;
        }
    }
}

//òåïåðü îôèöèàëüíî ðàáîòàåò
void constructHippodrome(bool *isPredictionMade, bool *isFinishCollided, bool *wasStartPlayed, 
                        bool *wasHorsesRunPlayed, int *timerNumber, Texture2D bgt, Vector2 mousePoint,
                        Vector2 horsesPoses[10], int horsesSpeed[10], Vector2 rectHorses[10],
                        Texture2D horseTextureArr[10], Color horseColorArr[10], int betCoefficient[10],
                        int raceResultsUntchdCopy[10], int playerWinnerIndex, int betMoney, 
                        float probabilities[10], float playersWinChance, int raceResults[10],
                        Sound btnClicked, Sound raceStart, Sound horsesRun, int globalBal) {  // Added back globalBal parameter
    
    static bool hasUpdatedBalance = false;
    Rectangle retryBTN = { 400, 305, 120, 40 };
    Rectangle exitBTN = { 595, 305, 120, 40 };
    static int updatedBalance = 0;  // Store the updated balance value
    
    DrawTexture(bgt, 0, 0, WHITE);
    drawGlobalBalance(globalBal);
    
    if (!*isFinishCollided) {
        if (!*wasStartPlayed) {
            PlaySound(raceStart);
            *wasStartPlayed = 1;
        }
        drawHorseArray(timerNumber, horsesPoses, horsesSpeed, rectHorses, horseTextureArr, horseColorArr, 0.6);
        printf("local timer = %i\n", *timerNumber);
        drawGlobalBalance(globalBal);
        if (*timerNumber == 0) {
            if (!*wasHorsesRunPlayed) {
                PlaySound(horsesRun);
                *wasHorsesRunPlayed = 1;
            }
        }
        if (*timerNumber % 60) {
            if (*timerNumber / 60 == 0) {
                DrawText("START", 1200 / 2 - 110, 600 / 2, 64, WHITE);
                if (*wasHorsesRunPlayed) {
                    PlaySound(horsesRun);
                    *wasHorsesRunPlayed = 1;
                }
            }
            else if (*timerNumber / 60 > 0) {
                DrawText(TextFormat("%i", (*timerNumber / 60)), 1200 / 2, 600 / 2, 64, WHITE);
            }
        }
    }
    else {
        StopSound(horsesRun);
        int moneyWon = betCoefficient[raceResultsUntchdCopy[playerWinnerIndex]] * betMoney;
        DrawRectangle(355, 220, 400, 150, GRAY);
        
        // Calculate and store the updated balance only once
        if (!hasUpdatedBalance) {
            if (moneyWon > 0) {
                updatedBalance = globalBal + moneyWon;
                globalBalance = updatedBalance;  // Update the global variable
            } else {
                updatedBalance = globalBal - betMoney;
                globalBalance = updatedBalance;  // Update the global variable
            }
            hasUpdatedBalance = true;
        }
        
        // Display result using the stored updated balance
        if (moneyWon > 0) {
            DrawText(TextFormat("YOU WON %i $", moneyWon), 415, 245, 32, GREEN);
        } else {
            DrawText(TextFormat("YOU LOST %i $", betMoney), 415, 245, 32, MAROON);
        }
        drawGlobalBalance(updatedBalance);  // Use the stored updated balance
        
        DrawRectangleRec(retryBTN, GREEN);
        DrawText("Retry", 405, 305, 38, WHITE);
        if (CheckCollisionPointRec(mousePoint, retryBTN)) {
            DrawRectangleRec((Rectangle) { retryBTN.x, retryBTN.y, retryBTN.width * 1.1, retryBTN.height * 1.1 }, GREEN);
            DrawText("Retry", 405, 305, 43, WHITE);
            drawGlobalBalance(updatedBalance);
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && *isPredictionMade) {
                PlaySound(btnClicked);
                *wasStartPlayed = 0;
                *wasHorsesRunPlayed = 0;
                *isPredictionMade = 0;
                *isFinishCollided = 0;
                *timerNumber = 360;
                betMoney = 100;
                hasUpdatedBalance = false;  // Reset the balance update flag
                fillHorsesPoses(horsesPoses);
                RectHorses(rectHorses, horsesPoses, horseTextureArr);
                generateProbabilities(probabilities, playerWinnerIndex, playersWinChance);
                determine_race_results(probabilities, raceResults);
                calculate_speeds(raceResults, horsesSpeed);
            }
        }
        
        DrawRectangleRec(exitBTN, RED);
        DrawText("Exit", 620, 305, 42, WHITE);
        if (CheckCollisionPointRec(mousePoint, exitBTN)) {
            DrawRectangleRec((Rectangle) { exitBTN.x, exitBTN.y, exitBTN.width * 1.1, exitBTN.height * 1.1 }, RED);
            DrawText("Exit", 620, 305, 47, WHITE);
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && *isPredictionMade) {
                PlaySound(btnClicked);
                drawGlobalBalance(updatedBalance);
                EndDrawing();
                currentGame = MAIN_GAME;
            }
        }
    }
}

// Star game specific variables
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


// Helper functions
Rectangle getRect(Vector2 pos, int width, int height) {
    return (Rectangle) { pos.x, pos.y, width, height };
}

void DrawMainMenu(Rectangle playButton, Rectangle creditsButton, Rectangle quitButton, Vector2 mousePos) {
    // Draw background (placeholder gray gradient)
    DrawRectangleGradientV(0, 0, GetScreenWidth(), GetScreenHeight(), DARKGRAY, GRAY);
    
    // Draw title
    const char* title = "PLACEHOLDER";
    int titleFontSize = 60;
    Vector2 titlePos = {
        GetScreenWidth()/2 - MeasureText(title, titleFontSize)/2,
        GetScreenHeight()/4
    };
    DrawText(title, titlePos.x, titlePos.y, titleFontSize, WHITE);

    // Draw buttons with hover effect
    Color playColor = CheckCollisionPointRec(mousePos, playButton) ? GREEN : DARKGREEN;
    Color creditsColor = CheckCollisionPointRec(mousePos, creditsButton) ? BLUE : DARKBLUE;
    Color quitColor = CheckCollisionPointRec(mousePos, quitButton) ? RED : MAROON;

    DrawRectangleRec(playButton, playColor);
    DrawRectangleRec(creditsButton, creditsColor);
    DrawRectangleRec(quitButton, quitColor);

    // Draw button text
    DrawText("PLAY", playButton.x + playButton.width/4, playButton.y + 10, 30, WHITE);
    DrawText("CREDITS", creditsButton.x + creditsButton.width/6, creditsButton.y + 10, 30, WHITE);
    DrawText("QUIT", quitButton.x + quitButton.width/4, quitButton.y + 10, 30, WHITE);
}


void DrawHitbox(Vector2 pos, int width, int height, bool isColliding) {
    Color outlineColor = isColliding ? RED : DARKGRAY;
    DrawRectangleLinesEx(getRect(pos, width, height), 3, outlineColor);
}

bool checkAllCollisions(Rectangle playerRec, Rectangle squares[], int squareCount) {
    for (int i = 0; i < squareCount; i++) {
        if (CheckCollisionRecs(playerRec, squares[i])) {
            return true;
        }
    }
    return false;
}

void collidingController(Vector2 LookVect, Vector2* pos, Rectangle* playerRec, float speed) {
    if (LookVect.x > 0) {
        pos->x -= speed;
        playerRec->x -= speed;
    }
    if (LookVect.x < 0) {
        pos->x += speed;
        playerRec->x += speed;
    }
    if (LookVect.y < 0) {
        pos->y -= speed;
        playerRec->y -= speed;
    }
    if (LookVect.y > 0) {
        pos->y += speed;
        playerRec->y += speed;
    }
}

// Star game functions
void InitializeStarGame(StarGame* game) {
    // Initialize bet values
    game->betValues[0] = 10;
    game->betValues[1] = 20;
    game->betValues[2] = 50;
    game->betValues[3] = 100;
    game->betValues[4] = 200;
    game->betValues[5] = 500;
    game->betValues[6] = 1000;
    game->betValues[7] = 2000;
    game->betValues[8] = 5000;
    game->betValues[9] = 10000;
    
    // Initialize game state
    game->currentBet = game->betValues[0];
    game->currentWin = 0;
    game->gameOver = false;
    game->gameWon = false;
    game->betPlaced = false;
    game->selectedBetIndex = 0;

    // Initialize grid
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            game->grid[i][j] = (Cell){
                .isMine = false,
                .isRevealed = false,
                .multiplier = 1.0f,
                .nearbyMines = 0,
                .isHovered = false
            };
        }
    }

    // Place mines randomly
    int minesPlaced = 0;
    while (minesPlaced < MINE_COUNT) {
        int x = rand() % GRID_SIZE;
        int y = rand() % GRID_SIZE;
        if (!game->grid[x][y].isMine) {
            game->grid[x][y].isMine = true;
            minesPlaced++;
            
            // Update nearby mine counts
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    int nx = x + dx;
                    int ny = y + dy;
                    if (nx >= 0 && nx < GRID_SIZE && ny >= 0 && ny < GRID_SIZE) {
                        game->grid[nx][ny].nearbyMines++;
                    }
                }
            }
        }
    }

    // Set multipliers for non-mine cells
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (!game->grid[i][j].isMine) {
                // Base multiplier increases with nearby mines
                float baseMultiplier = 1.2f + (game->grid[i][j].nearbyMines * 0.3f);
                // Add some randomness
                float randomFactor = (float)(rand() % 50) / 100.0f;
                game->grid[i][j].multiplier = baseMultiplier + randomFactor;
            }
        }
    }
}

void RevealCell(StarGame* game, int x, int y) {
    if (x < 0 || x >= GRID_SIZE || y < 0 || y >= GRID_SIZE || 
        game->grid[x][y].isRevealed || game->gameOver) {
        return;
    }

    // Place bet if not already placed
    if (!game->betPlaced) {
        if (globalBalance < game->currentBet) return;
        globalBalance -= game->currentBet;
        game->betPlaced = true;
    }

    game->grid[x][y].isRevealed = true;

    if (game->grid[x][y].isMine) {
        game->gameOver = true;
        game->currentWin = 0;  // Clear winnings on bomb hit
        // Reveal all cells on game over
        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                game->grid[i][j].isRevealed = true;
            }
        }
    } else {
        game->currentWin += (int)(game->currentBet * game->grid[x][y].multiplier);
        
        // Auto-reveal adjacent cells with no nearby mines
        if (game->grid[x][y].nearbyMines == 0) {
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    RevealCell(game, x + dx, y + dy);
                }
            }
        }
    }
}

// Rocket game functions
void ResetRocketGame(RocketGame* game) {
    game->bet = 0;
    game->multiplier = 1.0f;
    game->y = 450;
    game->cashedOut = false;
    game->state = WAITING;
    game->randomMultiplier = (float)(rand() % 10 + 1);
}

void DrawCredits(Rectangle backButton, Vector2 mousePos) {
    // Draw background (placeholder gradient)
    DrawRectangleGradientV(0, 0, GetScreenWidth(), GetScreenHeight(), DARKGRAY, GRAY);
    
    // Draw credits title
    const char* title = "CREDITS";
    DrawText(title, GetScreenWidth()/2 - MeasureText(title, 60)/2, 100, 60, WHITE);
    
    // Draw placeholder credits text
    const char* credits[] = {
        "Lorem ipsum dolor sit amet,",
        "consetetur sadipscing elitr",
        "sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat",
        "no sea takimata sanctus est Lorem ipsum dolor sit amet.",
        "Lorem ipsum dolor sit amet, consetetur sadipscing elitr"
    };
    
    for (int i = 0; i < 5; i++) {
        DrawText(credits[i], 
                GetScreenWidth()/2 - MeasureText(credits[i], 30)/2,
                250 + i * 50, 
                30, 
                WHITE);
    }
    
    // Draw back button with hover effect
    Color backColor = CheckCollisionPointRec(mousePos, backButton) ? BLUE : DARKBLUE;
    DrawRectangleRec(backButton, backColor);
    DrawText("BACK", backButton.x + backButton.width/4, backButton.y + 10, 30, WHITE);
}

int main(void) {
    const int screenWidth = 1200;
    const int screenHeight = 600;



    InitWindow(screenWidth, screenHeight, "PLACEHOLDER");
    SetTargetFPS(60);

    currentGame = MAIN_MENU;

    InitAudioDevice();

    bool otherWinSoundPlayed = false;
    bool doubleWinSoundPlayed = false;

    Sound doubleWinSound = LoadSound("resources/doublewin.mp3");
    SetSoundVolume(doubleWinSound, 0.5f);

    Sound freespinSound = LoadSound("resources/freespin.mp3");
    SetSoundVolume(freespinSound, 0.5f);

    Sound jackpotSound = LoadSound("resources/jackpot.mp3");
    SetSoundVolume(jackpotSound, 0.5f);

    Sound otherWinSound = LoadSound("resources/other.mp3");
    SetSoundVolume(otherWinSound, 0.5f);

    Sound betSound = LoadSound("resources/bet.mp3");
    SetSoundVolume(betSound, 0.20f);

    Sound bgMusic2 = LoadSound("resources/ambient.mp3");
    SetSoundVolume(bgMusic2, 0.5f);

    Sound spinSound = LoadSound("resources/spin.mp3");
    SetSoundVolume(spinSound, 0.20f);

    srand(time(NULL));

    Texture2D slotTextures[NUM_SYMBOLS];
    for (int i = 0; i < NUM_SYMBOLS; i++) {
        char filename[20];
        snprintf(filename, sizeof(filename), "%d.png", i + 1);
        slotTextures[i] = LoadSlotTexture(filename);
    }

    Texture2D waitTexture = LoadSlotTexture("resources/wait.png");
    Texture2D backgroundTexture = LoadSlotTexture("resources/background.png");
    Texture2D backgroundSpinTexture = LoadSlotTexture("resources/background_spin.png");

    Texture2D jackpotFrames[5];
    for (int i = 0; i < 5; i++) {
        char filename[20];
        snprintf(filename, sizeof(filename), "resources/jackpot_%d.png", i + 1);
        jackpotFrames[i] = LoadSlotTexture(filename);
    }
    Animation jackpotAnim = { jackpotFrames, 5, 0, 0, 10, false, 0, jackpotFrames[0].width, jackpotFrames[0].height };

    Texture2D freeSpinsFrames[4];
    for (int i = 0; i < 4; i++) {
        char filename[20];
        snprintf(filename, sizeof(filename), "resources/freespins_%d.png", i + 1);
        freeSpinsFrames[i] = LoadSlotTexture(filename);
    }
    Animation freeSpinsAnim = { freeSpinsFrames, 4, 0, 0, 10, false, 0, freeSpinsFrames[0].width, freeSpinsFrames[0].height };

    Texture2D doubleWinFrames[3];
    for (int i = 0; i < 3; i++) {
        char filename[20];
        snprintf(filename, sizeof(filename), "resources/doublewin_%d.png", i + 1);
        doubleWinFrames[i] = LoadSlotTexture(filename);
    }
    Animation doubleWinAnim = { doubleWinFrames, 3, 0, 0, 10, false, 0, doubleWinFrames[0].width, doubleWinFrames[0].height };

    int reels[NUM_REELS] = { 0, 0, 0 };
    int credits = 10000, betAmount = 10;
    bool gameStarted = false;
    bool isSpinning = false;
    int spinTimer = 0;
    bool jackpotTriggered = false, freeSpinsTriggered = false, doubleWinTriggered = false, otherWinTriggered = false;
    bool inFreeSpins = false;
    int freeSpinsRemaining = 0;
    int spinCooldown = 0;
    bool canSpin = true;
    bool nextWinDouble = false;

    Rectangle spinZone = { 980, screenHeight / 2 - 500, 150, 700 };
    Rectangle betPlusZone = { screenWidth - 785, screenHeight - 160, 90, 50 };
    Rectangle betMinusZone = { screenWidth - 895, screenHeight - 160, 90, 50 };

    Font customFont = LoadFont("resources/slots.ttf");
    Font analogFont = LoadFont("resources/analog.ttf");

    // Load textures
    Texture2D bgt = LoadTexture("resources/bg.png");
    Texture2D background = LoadTexture("resources/map.png");
    Texture2D playerTexture = LoadTexture("resources/0.png");
    Texture2D rocketBackground = LoadTexture("resources/fon.png");
    Texture2D rocketTexture = LoadTexture("resources/raketka.png");
    Texture2D hiddenTexture = LoadTexture("resources/squere.png");
    Texture2D starTexture = LoadTexture("resources/zvezda.png");
    Texture2D bombTexture = LoadTexture("resources/bomb.png");

    // Player setup
    float speed = 2.75f;
    Vector2 playerPos = {500, 225};
    Vector2 playerLookVect = { 0, 0 };
    Rectangle playerRec = getRect(playerPos, playerTexture.width, playerTexture.height);

    Rectangle playButton = { screenWidth/2 - 100, screenHeight/2, 200, 50 };
    Rectangle creditsButton = { screenWidth/2 - 100, screenHeight/2 + 70, 200, 50 };
    Rectangle quitButton = { screenWidth/2 - 100, screenHeight/2 + 140, 200, 50 };
    Rectangle backButton = { screenWidth/2 - 100, screenHeight - 100, 200, 50 };
    Rectangle backButton2 = {
    10,                         // x position (left side)
    screenHeight - 60,          // y position (bottom, leaving 10px margin)
    100,                        // width
    50                          // height
};
    Rectangle backButton3 = {
    10,                         // x position (left side)
    screenHeight - 60,          // y position (bottom, leaving 10px margin)
    100,                        // width
    50                          // height
};

    // Square setup for main game
   Rectangle topSquare = { 50, 30, 100, 100 };          // Star Game (leftmost)
Rectangle leftSquare = { 400, 30, 100, 100 };     // Rocket Game (center-left)
Rectangle rightSquare = { 700 , 30, 100, 100 };    // Hippodrome Game (center-right)
Rectangle bottomSquare = { 1050, 30, 100, 100 };  // Slots Game (rightmost)
Rectangle topBorder = { 0, 0, screenWidth, 10 };          // Top edge
Rectangle bottomBorder = { 0, screenHeight - 10, screenWidth, 10 }; // Bottom edge
Rectangle leftBorder = { 0, 0, 10, screenHeight };        // Left edge
Rectangle rightBorder = { screenWidth - 10, 0, 10, screenHeight }; // Right edge
Rectangle squares[] = { 
    topSquare, leftSquare, rightSquare, bottomSquare,  // Game squares
    topBorder, bottomBorder, leftBorder, rightBorder   // Border squares
};
const int squareCount = 8;

int timerNumber = 240;
int animFrames = 0;
Image horseimg = LoadImageAnim("resources/horse.gif", &animFrames);
Texture2D horseTextureArr[10];
fillHorseTextureArr(horseimg, horseTextureArr);
Rectangle rectHorses[10];
Vector2 horsesPoses[10];
fillHorsesPoses(horsesPoses);
RectHorses(rectHorses, horsesPoses, horseTextureArr);
Color horseColorArr[10] = { BEIGE, BLUE, RED, GREEN, YELLOW, PURPLE, DARKGREEN, MAGENTA, SKYBLUE, DARKPURPLE };
Rectangle finishHB = {1190, 0, 50, 600};
int betMoney = 100;
bool isPredictionMade = 0;
int playerWinnerIndex = 0;
float playersWinChance = 0.12;
double probabilities[10];
int raceResults[10] = {0};
int raceResultsUntchdCopy[10] = { 0 };
bool invalidMoneyPick = 0;
bool wasStartPlayed = 0;
bool wasHorsesRunPlayed = 0;
float horsesSpeed[10] = { 0 };
int betCoefficient[10] = {15, 5, 3, 1, -5, -15, -1, -1, -1, -1};
unsigned int nextFrameDataOffset = 0;
    int currentAnimFrame = 0;
    int frameDelay = 5;
    int frameCounter = 0;


    // Rocket game setup
    RocketGame rocketGame = { 1000, 0, 1.0f, 450, false, WAITING};
    Rectangle buttonStart = { screenWidth/2 + 100, 500, 100, 40 };
    Rectangle buttonCashOut = { screenWidth/2 + 220, 500, 100, 40 };
    Rectangle buttonBack = { screenWidth/2 + 340, 500, 100, 40 };
    char betString[10] = "";

    // Star game setup
    StarGame starGame;
    InitializeStarGame(&starGame);

    // Audio setup
    InitAudioDevice();
    Music bgMusic = LoadMusicStream("resources/fon1.mp3");
    Sound clickSound = LoadSound("resources/klik1.wav");
    Sound cashOutSound = LoadSound("resources/cash.wav");
    Sound startSound = LoadSound("resources/go1.wav");
    Sound btnClicked = LoadSound("resources/clicked.mp3");
Sound prediction = LoadSound("resources/predict.mp3");
Sound raceStart = LoadSound("resources/racestart.mp3");
Sound horsesRun = LoadSound("resources/horsesrun.mp3");

    while (!WindowShouldClose()) {
        Vector2 mousePos = GetMousePosition();

                switch (currentGame) {
            case MAIN_MENU: {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (CheckCollisionPointRec(mousePos, playButton)) {
                        currentGame = MAIN_GAME;
                        PlaySound(clickSound);
                    }
                    else if (CheckCollisionPointRec(mousePos, creditsButton)) {
                        currentGame = CREDITS;
                        PlaySound(clickSound);
                    }
                    else if (CheckCollisionPointRec(mousePos, quitButton)) {
                        CloseWindow();
                        return 0;
                    }
                }

                BeginDrawing();
                    DrawMainMenu(playButton, creditsButton, quitButton, mousePos);
                EndDrawing();
                break;
            }

            case CREDITS: {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (CheckCollisionPointRec(mousePos, backButton)) {
                        currentGame = MAIN_MENU;
                        PlaySound(clickSound);
                    }
                }

                BeginDrawing();
                    DrawCredits(backButton, mousePos);
                EndDrawing();
                break;
            }
            case MAIN_GAME: {
    playerLookVect = (Vector2){ 0, 0 };
    
    if (IsKeyDown(KEY_RIGHT)) {
        playerLookVect.x = 1;
        playerPos.x += speed;
        playerRec.x += speed;
    }
    if (IsKeyDown(KEY_LEFT)) {
        playerLookVect.x = -1;
        playerPos.x -= speed;
        playerRec.x -= speed;
    }
    if (IsKeyDown(KEY_UP)) {
        playerLookVect.y = 1;
        playerPos.y -= speed;
        playerRec.y -= speed;
    }
    if (IsKeyDown(KEY_DOWN)) {
        playerLookVect.y = -1;
        playerPos.y += speed;
        playerRec.y += speed;
    }

    bool isColliding = checkAllCollisions(playerRec, squares, squareCount);
    if (isColliding) {
        collidingController(playerLookVect, &playerPos, &playerRec, speed);
    }

    // Check proximity only for game squares, not borders
    bool isNearTop = CheckCollisionCircleRec(playerPos, 50, topSquare);
    bool isNearLeft = CheckCollisionCircleRec(playerPos, 50, leftSquare);
    bool isNearRight = CheckCollisionCircleRec(playerPos, 50, rightSquare);
    bool isNearBottom = CheckCollisionCircleRec(playerPos, 50, bottomSquare);

    if (IsKeyPressed(KEY_E)) {
        if (isNearTop) {
            currentGame = STAR_GAME;
            InitializeStarGame(&starGame);
        }
        else if (isNearLeft) {
            currentGame = ROCKET_GAME;
            ResetRocketGame(&rocketGame);
            PlayMusicStream(bgMusic);
        }
        else if (isNearRight) {
            currentGame = HIPPODROME_GAME;
            isPredictionMade = 0;
            timerNumber = 240;
            betMoney = 100;
            fillHorsesPoses(horsesPoses);
            RectHorses(rectHorses, horsesPoses, horseTextureArr);
            generateProbabilities(probabilities, playerWinnerIndex, playersWinChance);
            determine_race_results(probabilities, raceResults);
            calculate_speeds(raceResults, horsesSpeed);
        }
        else if (isNearBottom) {
            currentGame = SLOTS_GAME;
            // Placeholder for slots game initialization
        }
    }

    BeginDrawing();
        ClearBackground(RAYWHITE);
        
        DrawTexture(background, 0, 0, WHITE);
        
        
        
        // Draw game squares (first 4 in the array)
        for (int i = 0; i < 4; i++) {
            DrawRectangleRec(squares[i], RED);
        }
        
        // Draw border squares (last 4 in the array)
        for (int i = 4; i < squareCount; i++) {
            DrawRectangleRec(squares[i], BLACK);
        }
        
        DrawTextureV(playerTexture, playerPos, RAYWHITE);
        DrawHitbox(playerPos, playerTexture.width, playerTexture.height, isColliding);
        
        if (isNearTop || isNearLeft || isNearRight || isNearBottom) {
            DrawText("Press E to play", screenWidth / 2 - MeasureText("Press E to play", 20) / 2, 
                    screenHeight / 2 - 20, 20, WHITE);
        }
    EndDrawing();
    break;
}
            
            case HIPPODROME_GAME: {
    // Getting and setting mouse cursor pos
    mousePos = GetMousePosition();
    
    // Updating music
    UpdateMusicStream(bgMusic);
    
    // Horse animation frames change
    if (isPredictionMade && timerNumber <= 0) {
        for (int i = 0; i < 6; i++) {
            frameCounter++;
            if (frameCounter >= frameDelay)
            {
                currentAnimFrame++;
                if (currentAnimFrame >= animFrames)
                    currentAnimFrame = 0;
                nextFrameDataOffset = horseimg.width * horseimg.height * 4 * currentAnimFrame;
                UpdateTexture(horseTextureArr[i], ((unsigned char*)horseimg.data) + nextFrameDataOffset);
                if (i == 5) {
                    for (int j = 1; j < 5; j++) {
                        UpdateTexture(horseTextureArr[i + j], ((unsigned char*)horseimg.data) + nextFrameDataOffset);
                    }
                }
                frameCounter = 0;
            }
        }
    }
    
    bool isFinishCollided = checkHorsesCollision(rectHorses, finishHB, horsesSpeed);
    
    BeginDrawing();
    if (!isPredictionMade) {
        // Updated to include globalBalance in constructPredictionScreen call
        constructPredictionScreen(
            &isPredictionMade,    // Prediction state
            &playerWinnerIndex,   // Selected horse index
            &betMoney,            // Pointer to bet amount (changed from direct value)
            &invalidMoneyPick,    // Invalid pick flag
            (Vector2){0, 0},      // Position
            (Vector2){1200, 600}, // Size
            mousePos,             // Mouse position
            horseColorArr,        // Horse colors
            betCoefficient,       // Bet multipliers
            btnClicked,           // Button click sound
            prediction,           // Prediction sound
            &globalBalance        // Added global balance pointer
        );
        DrawRectangleRec(backButton3, BLUE);
DrawText("BACK", backButton3.x + 25, backButton3.y + 15, 20, WHITE);

// Check for button click
if (CheckCollisionPointRec(GetMousePosition(), backButton3) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    currentGame = MAIN_GAME;
    // Optional: Play button click sound if you have one
    // PlaySound(btnClicked);
}
    } else {
        // constructHippodrome remains mostly the same as it already has globalBalance
        constructHippodrome(
            &isPredictionMade,
            &isFinishCollided,
            &wasStartPlayed,
            &wasHorsesRunPlayed,
            &timerNumber,
            bgt,
            mousePos,
            horsesPoses,
            horsesSpeed,
            rectHorses,
            horseTextureArr,
            horseColorArr,
            betCoefficient,
            raceResultsUntchdCopy,
            playerWinnerIndex,
            betMoney,            // This is now controlled by prediction screen
            probabilities,
            playersWinChance,
            raceResults,
            btnClicked,
            raceStart,
            horsesRun,
            globalBalance        // Already included
        );
    }
    
    EndDrawing();
    break;
}
            
            case ROCKET_GAME: {
    UpdateMusicStream(bgMusic);
        if (rocketGame.state == WAITING) {
            int key = GetKeyPressed();
            if (key >= '0' && key <= '9') {
                int length = strlen(betString);
                if (length < 9) {
                    betString[length] = (char)key;
                    betString[length + 1] = '\0';
                }
            }
            else if (key == KEY_BACKSPACE && strlen(betString) > 0) {
                betString[strlen(betString) - 1] = '\0';
            }

            if (strlen(betString) > 0) {
                rocketGame.bet = atoi(betString);
                if (rocketGame.bet > globalBalance) {
                    rocketGame.bet = globalBalance;
                    snprintf(betString, sizeof(betString), "%d", rocketGame.bet);
                }
            }

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (CheckCollisionPointRec(mousePos, buttonStart)) {
                    if (globalBalance >= rocketGame.bet && rocketGame.bet > 0) {
                        rocketGame.state = LAUNCHING;
                        globalBalance -= rocketGame.bet;
                        PlaySound(startSound);
                    }
                }
                else if (CheckCollisionPointRec(mousePos, buttonBack)) {
                    currentGame = MAIN_GAME;
                    StopMusicStream(bgMusic);
                }
            }
        }
        else if (rocketGame.state == LAUNCHING) {
            rocketGame.y -= 4;
            if (rocketGame.y <= screenHeight / 3) {
                rocketGame.state = INCREASE_MULTIPLIER;
            }
        }
        else if (rocketGame.state == INCREASE_MULTIPLIER) {
            rocketGame.multiplier += 0.02f;

            if (rocketGame.multiplier >= rocketGame.randomMultiplier) {
                rocketGame.state = GAME_OVER;
            }

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (CheckCollisionPointRec(mousePos, buttonCashOut)) {
                    globalBalance += (int)(rocketGame.bet * rocketGame.multiplier);
                    rocketGame.cashedOut = true;
                    rocketGame.state = GAME_OVER;
                    PlaySound(cashOutSound);
                }
            }
        }
        else if (rocketGame.state == GAME_OVER) {
            ResetRocketGame(&rocketGame);
            betString[0] = '\0';
        }

        BeginDrawing();
            ClearBackground(RAYWHITE);
            
            // Draw background centered
            DrawTexturePro(rocketBackground,
                (Rectangle){ 0, 0, rocketBackground.width, rocketBackground.height },
                (Rectangle){ 0, 0, screenWidth, screenHeight },
                (Vector2){ 0, 0 }, 0.0f, WHITE);

            // Draw rocket centered
            DrawTexturePro(rocketTexture,
                (Rectangle){ 0, 0, rocketTexture.width, rocketTexture.height },
                (Rectangle){ screenWidth/2 - rocketTexture.width/30, (int)rocketGame.y,
                           rocketTexture.width/15, rocketTexture.height/15 },
                (Vector2){ 0, 0 }, 0.0f, WHITE);

            // Center UI elements
            // Update button positions to be centered
            buttonStart = (Rectangle){ screenWidth/2 - 160, 500, 100, 40 };
            buttonCashOut = (Rectangle){ screenWidth/2 - 40, 500, 100, 40 };
            buttonBack = (Rectangle){ screenWidth/2 + 80, 500, 100, 40 };

            // Draw UI elements with centered layout
            DrawText(TextFormat("Balance: %d", globalBalance), 20, 20, 20, WHITE);
            DrawText(TextFormat("Bet: %d", rocketGame.bet), 20, 50, 20, WHITE);
            DrawText(TextFormat("x%.2f", rocketGame.multiplier), 
                    screenWidth/2 - 30, rocketGame.y - 30, 20, RED);

            // Draw bet input centered
            //DrawText("Enter Bet:", screenWidth/2 - 150, 500, 30, WHITE);
            //DrawText(strlen(betString) > 0 ? betString : " ", 
            //        screenWidth/2 - 150, 530, 30, WHITE);

            // Draw buttons
            DrawRectangleRec(buttonStart, GREEN);
            DrawText("Start", buttonStart.x + 20, buttonStart.y + 10, 20, BLACK);

            DrawRectangleRec(buttonCashOut, RED);
            DrawText("Cash Out", buttonCashOut.x + 10, buttonCashOut.y + 10, 20, BLACK);

            DrawRectangleRec(buttonBack, BLUE);
            DrawText("Back", buttonBack.x + 30, buttonBack.y + 10, 20, BLACK);
        EndDrawing();
    break;
}
             case SLOTS_GAME: {
                if (!IsSoundPlaying(bgMusic2)) {
            PlaySound(bgMusic2);
        }

        if (!canSpin) {
            spinCooldown--;
            if (spinCooldown <= 0 && !IsSoundPlaying(spinSound)) {
                canSpin = true;
            }
        }

        if (IsBetButtonClicked(&betPlusZone)) {
            if (globalBalance >= betAmount + 10) {
                betAmount += 10;
                PlaySound(betSound);
            }
        }
        if (IsBetButtonClicked(&betMinusZone)) {
            if (betAmount > 10) {
                betAmount -= 10;
                PlaySound(betSound);
            }
        }

        if (globalBalance >= betAmount && canSpin) {
            if (IsSpinZoneClicked(&spinZone)) {
                StopSound(jackpotSound);
                StopSound(freespinSound);
                StopSound(doubleWinSound);
                StopSound(otherWinSound);

                otherWinSoundPlayed = false;
                doubleWinSoundPlayed = false;

                if (!isSpinning) {
                    PlaySound(spinSound);
                    canSpin = false;
                    spinCooldown = SPIN_COOLDOWN;

                    if (!inFreeSpins) {
                        globalBalance -= betAmount;
                    }

                    spinTimer = 0;
                    isSpinning = true;
                    gameStarted = true;

                    jackpotTriggered = false;
                    freeSpinsTriggered = false;
                    doubleWinTriggered = false;
                    otherWinTriggered = false;
                    jackpotAnim.isAnimating = false;
                    freeSpinsAnim.isAnimating = false;
                    doubleWinAnim.isAnimating = false;

                    for (int i = 0; i < NUM_REELS; i++) {
                        reels[i] = rand() % NUM_SYMBOLS;
                    }
                }
            }
        }

        if (isSpinning) {
            spinTimer++;
            for (int i = 0; i < NUM_REELS; i++) {
                reels[i] = (reels[i] + 1) % NUM_SYMBOLS;
            }

            if (spinTimer > 30) {
                isSpinning = false;
                for (int i = 0; i < NUM_REELS; i++) {
                    reels[i] = rand() % NUM_SYMBOLS;
                }

                int payout = CalculatePayout(reels, betAmount, &jackpotTriggered, &freeSpinsTriggered, &doubleWinTriggered, &otherWinTriggered);

                if (nextWinDouble && payout > 0 && !doubleWinTriggered) {
                    payout *= 2;
                    nextWinDouble = false;
                }

                globalBalance += payout;

                if (doubleWinTriggered) {
                    nextWinDouble = true;
                }

                if (freeSpinsTriggered && !inFreeSpins) {
                    inFreeSpins = true;
                    freeSpinsRemaining += 6;
                }

                if (!isSpinning && inFreeSpins) {
                    freeSpinsRemaining--;
                    if (freeSpinsRemaining <= 0) {
                        inFreeSpins = false;
                    }
                }
            }
        }

        if (!isSpinning) {
            if (reels[0] == reels[1] && reels[1] == reels[2]) {
                if (reels[0] == 0 && !jackpotAnim.isAnimating && jackpotTriggered) {
                    jackpotAnim.isAnimating = true;
                    jackpotAnim.currentFrame = 0;
                    jackpotAnim.frameTimer = 0;
                    jackpotAnim.animationTimer = 0;
                    PlaySound(jackpotSound);
                }
                else if (reels[0] == 1 && !freeSpinsAnim.isAnimating && freeSpinsTriggered) {
                    freeSpinsAnim.isAnimating = true;
                    freeSpinsAnim.currentFrame = 0;
                    freeSpinsAnim.frameTimer = 0;
                    freeSpinsAnim.animationTimer = 0;
                    PlaySound(freespinSound);
                }
                else if (reels[0] == 2 && !doubleWinAnim.isAnimating && doubleWinTriggered && !doubleWinSoundPlayed) {
                    doubleWinAnim.isAnimating = true;
                    doubleWinAnim.currentFrame = 0;
                    doubleWinAnim.frameTimer = 0;
                    doubleWinAnim.animationTimer = 0;
                    PlaySound(doubleWinSound);
                    doubleWinSoundPlayed = true;
                }
                else if ((reels[0] == 3 || reels[0] == 4 || reels[0] == 5) && otherWinTriggered && !otherWinSoundPlayed) {
                    PlaySound(otherWinSound);
                    otherWinSoundPlayed = true;
                }
            }
        }

        UpdateAnimation(&jackpotAnim);
        UpdateAnimation(&freeSpinsAnim);
        UpdateAnimation(&doubleWinAnim);

        BeginDrawing();
        ClearBackground(DARKGRAY);
        
        

        if (isSpinning && spinTimer < 30 / 2) {
            DrawTexture(backgroundSpinTexture, 0, 0, WHITE);
        }
        else {
            DrawTexture(backgroundTexture, 0, 0, WHITE);
        }

        DrawSlotMachine(reels, screenWidth, screenHeight, slotTextures, &waitTexture, gameStarted);

        DrawTextEx(analogFont, TextFormat("Bet: %d", betAmount), (Vector2) { 312, screenHeight - 566 }, 25, 1, RED);

        if (!inFreeSpins) {
            DrawTextEx(analogFont, TextFormat("$ %d", globalBalance), (Vector2) { screenWidth - 550, screenHeight - 150 }, 30, 1, RED);
        }

        if (inFreeSpins) {
            const char* freeSpinsText = TextFormat("Free Spins: %d", freeSpinsRemaining);
            DrawTextEx(analogFont, freeSpinsText, (Vector2) { screenWidth / 2 - 50 + 102, 90 + 360 }, 30, 1, RED);
        }

        if (jackpotAnim.isAnimating) {
            DrawAnimation(&jackpotAnim, screenWidth, screenHeight);
        }
        if (freeSpinsAnim.isAnimating) {
            DrawAnimation(&freeSpinsAnim, screenWidth, screenHeight);
        }
        if (doubleWinAnim.isAnimating) {
            DrawAnimation(&doubleWinAnim, screenWidth, screenHeight);
        }

        if (!canSpin) {
            DrawText("x", spinZone.x - 145, spinZone.y + spinZone.height - 58, 40, RED);
        }
        DrawRectangleRec(backButton2, BLUE);
DrawText("BACK", backButton2.x + 25, backButton2.y + 15, 20, WHITE);

// Check for button click
if (CheckCollisionPointRec(GetMousePosition(), backButton2) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    currentGame = MAIN_GAME;
    // Optional: Play button click sound if you have one
    // PlaySound(btnClicked);
}

        EndDrawing();
        break;
    }
    

            
            case STAR_GAME: {
    if (IsKeyPressed(KEY_ESCAPE)) {
        currentGame = MAIN_GAME;
    }

    // Handle mouse input for Star game
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mousePos = GetMousePosition();
        
        // Check menu button clicks
        Rectangle betButton = { 20, 260, 160, 60 };
        Rectangle takeButton = { 20, 340, 160, 60 };
        Rectangle restartButton = { 20, 420, 160, 60 };
        Rectangle backButton = { 20, 500, 160, 60 };  // New back button

        if (CheckCollisionPointRec(mousePos, betButton) && !starGame.betPlaced && !starGame.gameOver) {
        starGame.selectedBetIndex = (starGame.selectedBetIndex + 1) % 10;
        starGame.currentBet = starGame.betValues[starGame.selectedBetIndex];
        PlaySound(clickSound);
    }
    // Only allow take button if bet is placed AND game is not over (no bomb hit)
    if (CheckCollisionPointRec(mousePos, takeButton) && starGame.betPlaced && !starGame.gameOver) {
        globalBalance += starGame.currentWin;
        starGame.currentWin = 0;
        starGame.betPlaced = false;
        InitializeStarGame(&starGame);
        PlaySound(cashOutSound);
    }
    else if (CheckCollisionPointRec(mousePos, restartButton)) {
        InitializeStarGame(&starGame);
        PlaySound(clickSound);
    }
    else if (CheckCollisionPointRec(mousePos, backButton)) {
        currentGame = MAIN_GAME;
        PlaySound(clickSound);
    }
        // Handle grid clicks
        else if (mousePos.x >= MENU_WIDTH) {
            int gridX = (mousePos.x - MENU_WIDTH) / CELL_SIZE;
            int gridY = mousePos.y / CELL_SIZE;
            if (gridX >= 0 && gridX < GRID_SIZE && gridY >= 0 && gridY < GRID_SIZE) {
                RevealCell(&starGame, gridX, gridY);
                PlaySound(clickSound);
            }
        }
    }

    BeginDrawing();
        ClearBackground(SKYBLUE);
        
        // Draw grid and UI
        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                Rectangle cellRect = {
                    MENU_WIDTH + i * CELL_SIZE,
                    j * CELL_SIZE,
                    CELL_SIZE,
                    CELL_SIZE
                };

                // Update hover state
                starGame.grid[i][j].isHovered = CheckCollisionPointRec(mousePos, cellRect);

                // Draw cell
                Color cellColor = SKYBLUE;
                if (!starGame.betPlaced && globalBalance < starGame.currentBet) {
                    cellColor = (Color){ 200, 200, 200, 100 };
                } 
                else if (starGame.grid[i][j].isHovered && !starGame.grid[i][j].isRevealed) {
                    cellColor = (Color){ 230, 230, 230, 255 };
                }

                if (starGame.grid[i][j].isRevealed) {
                    if (starGame.grid[i][j].isMine) {
                        DrawTexturePro(bombTexture,
                            (Rectangle){ 0, 0, bombTexture.width, bombTexture.height },
                            cellRect, (Vector2){ 0, 0 }, 0, cellColor);
                    } else {
                        DrawTexturePro(starTexture,
                            (Rectangle){ 0, 0, starTexture.width, starTexture.height },
                            cellRect, (Vector2){ 0, 0 }, 0, cellColor);
                        DrawText(TextFormat("x%.2f", starGame.grid[i][j].multiplier),
                            cellRect.x + 10, cellRect.y + 10, 20, BLACK);
                    }
                } else {
                    DrawRectangleRec(cellRect, cellColor);
                    DrawTexturePro(hiddenTexture,
                        (Rectangle){ 0, 0, hiddenTexture.width, hiddenTexture.height },
                        cellRect, (Vector2){ 0, 0 }, 0, cellColor);
                }

                DrawRectangleLines(cellRect.x, cellRect.y, CELL_SIZE, CELL_SIZE, BLACK);
            }
        }

                            // Draw menu
        DrawRectangle(0, 0, MENU_WIDTH, screenHeight, DARKBLUE);
        DrawText(TextFormat("Balance: %d", globalBalance), 20, 100, 25, WHITE);
        DrawText(TextFormat("Bet: %d", starGame.currentBet), 20, 130, 25, WHITE);
        DrawText(TextFormat("Win: %d", starGame.currentWin), 20, 160, 25, WHITE);

        // Draw buttons
        Rectangle betButton = { 20, 260, 160, 60 };
        Rectangle takeButton = { 20, 340, 160, 60 };
        Rectangle restartButton = { 20, 420, 160, 60 };
        Rectangle backButton = { 20, 500, 160, 60 };

        // Draw bet button - grey out if bet is placed
        Color betButtonColor = (!starGame.betPlaced && !starGame.gameOver) ? YELLOW : GRAY;
        DrawRectangleRec(betButton, betButtonColor);
        DrawText(TextFormat("Bet %d", starGame.currentBet),
            betButton.x + 40, betButton.y + 20, 20, 
            (!starGame.betPlaced && !starGame.gameOver) ? BLACK : DARKGRAY);

        // Draw take button only if bet is placed
        if (starGame.betPlaced && !starGame.gameOver) {
    DrawRectangleRec(takeButton, GREEN);
    DrawText("Take", takeButton.x + 55, takeButton.y + 20, 20, BLACK);
}

        // Always draw restart button
        DrawRectangleRec(restartButton, RED);
        DrawText("Restart", restartButton.x + 45, restartButton.y + 20, 20, WHITE);

        // Draw back button
        DrawRectangleRec(backButton, BLUE);
        DrawText("Back", backButton.x + 55, backButton.y + 20, 20, WHITE);

    EndDrawing();
    break;
       }
    }
}

    // Cleanup
    UnloadTexture(bgt);
    for (int i = 0; i < 10; i++) {
    UnloadTexture(horseTextureArr[i]);
    }   
    UnloadSound(btnClicked);
    UnloadSound(prediction);
    UnloadSound(raceStart);
    UnloadSound(horsesRun);
    UnloadTexture(background);
    UnloadTexture(playerTexture);
    UnloadTexture(rocketBackground);
    UnloadTexture(rocketTexture);
    UnloadTexture(hiddenTexture);
    UnloadTexture(starTexture);
    UnloadTexture(bombTexture);
    UnloadMusicStream(bgMusic);
    UnloadSound(clickSound);
    UnloadSound(cashOutSound);
    UnloadSound(startSound);
    CloseAudioDevice();
    CloseWindow();

    return 0;
    }
