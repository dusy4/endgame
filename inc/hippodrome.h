#ifndef HIPPODROME_H
#define HIPPODROME_H

#include "main.h"

typedef struct {
    int index;
    double probability;
} Horse;

void constructPredictionScreen(bool *isPRedictionMade, int *playerHorseIndex, int *money, 
                             bool *invalidMoneyPick, Vector2 pos, Vector2 size, Vector2 mousePoint, 
                             Color horsesColors[], int betCoefficient[], Sound clicked, 
                             Sound prediction, int *globalBalance);
                             
Texture2D createHorseTexture(Image horseimg);
void fillHorseTextureArr(Image horseimg, Texture2D* horseTextureArr);
void fillHorsesPoses(Vector2 arr[]);
void RectHorses(Rectangle arr[], Vector2 pos[], Texture2D* horseTextureArr);
void DrawHorseHB(Rectangle rectHorses, bool isColliding);
void drawHorseArray(int *timerNumber, Vector2 pos[], float speeds[], Rectangle rectHorses[], 
                   Texture2D* horseTextureArr, Color* colorArr, float sizeMultiplier);
void generateProbabilities(double probabilities[], int fixed_index, double fixed_value);
void sort_horses(Horse horses[], int size);
void determine_race_results(double probabilities[], int results[]);
void calculate_speeds(int results[], float speeds[]);
int getWinnerId(int results[]);
void constructHippodrome(bool *isPredictionMade, bool *isFinishCollided, bool *wasStartPlayed, 
                        bool *wasHorsesRunPlayed, int *timerNumber, Texture2D bgt, Vector2 mousePoint,
                        Vector2 horsesPoses[10], int horsesSpeed[10], Vector2 rectHorses[10],
                        Texture2D horseTextureArr[10], Color horseColorArr[10], int betCoefficient[10],
                        int raceResultsUntchdCopy[10], int playerWinnerIndex, int betMoney, 
                        float probabilities[10], float playersWinChance, int raceResults[10],
                        Sound btnClicked, Sound raceStart, Sound horsesRun, int globalBal);

#endif
