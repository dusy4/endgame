#ifndef SLOTS_H
#define SLOTS_H

#include "main.h"

#define NUM_SYMBOLS 6
#define NUM_REELS 3
#define ANIMATION_DURATION 180
#define SPIN_COOLDOWN 30

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

Texture2D LoadSlotTexture(const char* filename);
int CalculatePayout(int* reels, int betAmount, bool* jackpotTriggered, bool* freeSpinsTriggered, 
                   bool* doubleWinTriggered, bool* otherWinTriggered);
void DrawAnimation(Animation* anim, int screenWidth, int screenHeight);
void UpdateAnimation(Animation* anim);
void DrawSlotMachine(int* reels, int width, int height, Texture2D* textures, 
                    Texture2D* waitTexture, bool gameStarted);
bool IsSpinZoneClicked(Rectangle* spinZone);
bool IsBetButtonClicked(Rectangle* buttonZone);

#endif
