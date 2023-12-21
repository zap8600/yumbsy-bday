#include <stdbool.h>
#include "rlh/raylib.h"

typedef struct raylib_syms {
    void *lib;
    int (*GetRandomValue)(int min, int max);
    void (*DrawPlane)(Vector3 centerPos, Vector2 size, Color color);
    void (*DisableCursor)(void);
    void (*EndMode3D)(void);
    void (*BeginMode3D)(Camera3D camera);
    void (*DrawCapsule)(Vector3 startPos, Vector3 endPos, float radius, int slices, int rings, Color color);
    void (*DrawCapsuleWires)(Vector3 startPos, Vector3 endPos, float radius, int slices, int rings, Color color);
    const char *(*TextFormat)(const char *text, ...);
    void (*DrawRectangle)(int posX, int posY, int width, int height, Color color);
    void (*DrawRectangleLines)(int posX, int posY, int width, int height, Color color);
    Color (*Fade)(Color color, float alpha);
    Vector2 (*GetMouseDelta)(void);
    double (*GetTime)(void);
    float (*GetFrameTime)(void);
    void (*InitWindow)(int width, int height, const char *title);
    void (*CloseWindow)(void);
    bool (*WindowShouldClose)(void);
    void (*ClearBackground)(Color color);                          // Set background color (framebuffer clear color)
    void (*BeginDrawing)(void);                                    // Setup canvas (framebuffer) to start drawing
    void (*EndDrawing)(void);
    void (*SetTargetFPS)(int fps);
    bool (*IsKeyPressed)(int key);
    void (*DrawText)(const char *text, int posX, int posY, int fontSize, Color color);
    void (*InitAudioDevice)(void);
    void (*CloseAudioDevice)(void);
    Music (*LoadMusicStream)(const char *fileName);
    void (*UnloadMusicStream)(Music music);
    void (*PlayMusicStream)(Music music);
    void (*StopMusicStream)(Music music);
    void (*UpdateMusicStream)(Music music);
} raylib_syms;