#define _CRT_SECURE_NO_WARNINGS
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> // Для генерации случайных чисел

#define TILE_SIZE 50
#define SOURCE_TILE_SIZE 189
#define BALL_SPEED 200.0f
#define ENEMY_SPEED 100.0f
#define ENEMY_MOVE_TIME 0.3f // Время перед сменой направления

void count_rows_cols(const char* filename, int* rows, int* cols) {
    FILE* file = fopen(filename, "r");
    if (!file) return;

    char line[1024];
    *rows = *cols = 0;

    while (fgets(line, sizeof(line), file)) {
        (*rows)++;
        int temp_cols = 0;
        char* token = strtok(line, " \t\n");
        while (token) {
            temp_cols++;
            token = strtok(NULL, " \t\n");
        }
        if (temp_cols > *cols) *cols = temp_cols;
    }
    fclose(file);
}

int** load_map(const char* filename, int* rows, int* cols) {
    count_rows_cols(filename, rows, cols);
    if (*rows == 0 || *cols == 0) return NULL;

    int** map = (int**)malloc(*rows * sizeof(int*));
    for (int i = 0; i < *rows; i++) map[i] = (int*)malloc(*cols * sizeof(int));

    FILE* file = fopen(filename, "r");
    if (!file) return NULL;

    char line[1024];
    int row = 0;
    while (fgets(line, sizeof(line), file) && row < *rows) {
        int col = 0;
        char* token = strtok(line, " \t\n");
        while (token && col < *cols) {
            map[row][col] = atoi(token);
            token = strtok(NULL, " \t\n");
            col++;
        }
        row++;
    }
    fclose(file);
    return map;
}

// Функция для выбора случайного направления
Vector2 get_random_direction() {
    int dir = rand() % 4; // 0 - вверх, 1 - вниз, 2 - влево, 3 - вправо
    if (dir == 0) return (Vector2) { 0, -1 };
    if (dir == 1) return (Vector2) { 0, 1 };
    if (dir == 2) return (Vector2) { -1, 0 };
    return (Vector2) { 1, 0 };
}

int main(void) {
    srand(time(NULL)); // Инициализация генератора случайных чисел

    const int screenWidth = 2000;
    const int screenHeight = 700;
    InitWindow(screenWidth, screenHeight, "Tilemap Example");

    Texture2D tile = LoadTexture("0.png");
    int rows, cols;
    int** map = load_map("map.txt", &rows, &cols);
    if (!map) return -1;

    Vector2 ballPosition = { 1, 1 }; // Позиция в клетках (не в пикселях)
    Vector2 enemy1Position = { cols / 2, rows / 2 };
    Vector2 enemy2Position = { cols / 2, rows / 2 + 1 };
    Vector2 enemy1Dir = get_random_direction();
    Vector2 enemy2Dir = get_random_direction();

    float enemy1MoveTimer = 0, enemy2MoveTimer = 0;
    bool gameOver = false;

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        if (!gameOver) {
            // Движение игрока (по клеткам)
            if (IsKeyDown(KEY_RIGHT) && map[(int)ballPosition.y][(int)(ballPosition.x + 1)] == 0)
                ballPosition.x += BALL_SPEED * dt / TILE_SIZE;
            if (IsKeyDown(KEY_LEFT) && map[(int)ballPosition.y][(int)(ballPosition.x - 1)] == 0)
                ballPosition.x -= BALL_SPEED * dt / TILE_SIZE;
            if (IsKeyDown(KEY_DOWN) && map[(int)(ballPosition.y + 1)][(int)ballPosition.x] == 0)
                ballPosition.y += BALL_SPEED * dt / TILE_SIZE;
            if (IsKeyDown(KEY_UP) && map[(int)(ballPosition.y - 1)][(int)ballPosition.x] == 0)
                ballPosition.y -= BALL_SPEED * dt / TILE_SIZE;

            // Таймер смены направления врагов
            enemy1MoveTimer += dt;
            enemy2MoveTimer += dt;

            if (enemy1MoveTimer >= ENEMY_MOVE_TIME) {
                Vector2 newPos = { enemy1Position.x + enemy1Dir.x, enemy1Position.y + enemy1Dir.y };
                if (map[(int)newPos.y][(int)newPos.x] == 0) {
                    enemy1Position = newPos;
                }
                else {
                    enemy1Dir = get_random_direction(); // Меняем направление
                }
                enemy1MoveTimer = 0;
            }

            if (enemy2MoveTimer >= ENEMY_MOVE_TIME) {
                Vector2 newPos = { enemy2Position.x + enemy2Dir.x, enemy2Position.y + enemy2Dir.y };
                if (map[(int)newPos.y][(int)newPos.x] == 0) {
                    enemy2Position = newPos;
                }
                else {
                    enemy2Dir = get_random_direction();
                }
                enemy2MoveTimer = 0;
            }

            // Проверка столкновения с врагами
            if ((int)ballPosition.x == (int)enemy1Position.x && (int)ballPosition.y == (int)enemy1Position.y ||
                (int)ballPosition.x == (int)enemy2Position.x && (int)ballPosition.y == (int)enemy2Position.y) {
                gameOver = true;
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                if (map[i][j] == 0) {
                    Rectangle sourceRect = { 0, 0, (float)SOURCE_TILE_SIZE, (float)SOURCE_TILE_SIZE };
                    Rectangle destRect = { (float)(j * TILE_SIZE), (float)(i * TILE_SIZE), (float)TILE_SIZE, (float)TILE_SIZE };
                    DrawTexturePro(tile, sourceRect, destRect, (Vector2) { 0, 0 }, 0.0f, WHITE);
                }
            }
        }

        DrawCircle((int)(ballPosition.x * TILE_SIZE + TILE_SIZE / 2), (int)(ballPosition.y * TILE_SIZE + TILE_SIZE / 2), TILE_SIZE / 2, RED);
        DrawRectangle((int)(enemy1Position.x * TILE_SIZE), (int)(enemy1Position.y * TILE_SIZE), TILE_SIZE, TILE_SIZE, BLACK);
        DrawRectangle((int)(enemy2Position.x * TILE_SIZE), (int)(enemy2Position.y * TILE_SIZE), TILE_SIZE, TILE_SIZE, BLACK);

        if (gameOver) {
            DrawText("Game Over!", screenWidth / 2 - MeasureText("Game Over!", 40) / 2, screenHeight / 2 - 20, 40, RED);
        }

        EndDrawing();
    }

    for (int i = 0; i < rows; i++) free(map[i]);
    free(map);

    UnloadTexture(tile);
    CloseWindow();
    return 0;
}
