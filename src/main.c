/*******************************************************************************************
*
*   raylib [core] example - 3d camera first person
*
*   Example originally created with raylib 1.3, last time updated with raylib 1.3
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2015-2023 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include "rcamera.h"

#define MAX_COLUMNS 20

BoundingBox beanCollide = {0};

void updateBeanCollide(Camera* camera, int cameraMode) {
    if(cameraMode == CAMERA_FIRST_PERSON) {
        beanCollide = (BoundingBox){
                        (Vector3){camera->position.x - 0.7f, camera->position.y - 1.7f, camera->position.z - 0.7f},
                        (Vector3){camera->position.x + 0.7f, camera->position.y + 0.9f, camera->position.z + 0.7f}};
    } else if(cameraMode == CAMERA_THIRD_PERSON) {
        beanCollide = (BoundingBox){
                        (Vector3){camera->target.x - 0.7f, camera->target.y - 1.7f, camera->target.z - 0.7f},
                        (Vector3){camera->target.x + 0.7f, camera->target.y + 0.9f, camera->target.z + 0.7f}};
    }
}

bool collisionDetect() {
    if(CheckCollisionBoxSphere(beanCollide, (Vector3){-1.0f, 0.0f, -2.0f}, 1.0f)) {
        return true;
    } else {
        return false;
    }
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "bean game proto - happy bday yumbsy");

    // Define the camera to look into our 3d world (position, target, up vector)
    Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, 1.7f, 4.0f };    // Camera position
    camera.target = (Vector3){ 0.0f, 1.7f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 60.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

    int cameraMode = CAMERA_FIRST_PERSON;

    // Generates some random columns
    float heights[MAX_COLUMNS] = { 0 };
    Vector3 positions[MAX_COLUMNS] = { 0 };
    Color colors[MAX_COLUMNS] = { 0 };

    for (int i = 0; i < MAX_COLUMNS; i++)
    {
        heights[i] = (float)GetRandomValue(1, 12);
        positions[i] = (Vector3){ (float)GetRandomValue(-15, 15), heights[i]/2.0f, (float)GetRandomValue(-15, 15) };
        colors[i] = (Color){ GetRandomValue(20, 255), GetRandomValue(10, 55), 30, 255 };
    }

    Color beanColor = (Color){ GetRandomValue(0, 255), GetRandomValue(0, 255), GetRandomValue(0, 255), GetRandomValue(0, 255) };

    DisableCursor();                    // Limit cursor to relative movement inside the window

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        /*
        // Update
        //----------------------------------------------------------------------------------
        // Switch camera mode
        if (IsKeyPressed(KEY_ONE))
        {
            cameraMode = CAMERA_FREE;
            camera.up = (Vector3){ 0.0f, 1.0f, 0.0f }; // Reset roll
        }
        */

        if (IsKeyPressed(KEY_ONE))
        {
            if(cameraMode != CAMERA_FIRST_PERSON) {
                cameraMode = CAMERA_FIRST_PERSON;
                camera.up = (Vector3){ 0.0f, 1.0f, 0.0f }; // Reset roll
                camera.target.y = 1.7f;
                camera.position = camera.target;
                camera.target.x = camera.target.x + 4.0f; // TODO: make this better somehow
                updateBeanCollide(&camera, cameraMode);
            }
        }

        if (IsKeyPressed(KEY_TWO))
        {
            if(cameraMode != CAMERA_THIRD_PERSON) {
                cameraMode = CAMERA_THIRD_PERSON;
                camera.up = (Vector3){ 0.0f, 1.0f, 0.0f }; // Reset roll
                camera.position.y = 1.7f;
                camera.target = camera.position;
                camera.position.x = camera.position.x - 4.0f; // TODO: make this better somehow
                updateBeanCollide(&camera, cameraMode);
            }
        }

        updateBeanCollide(&camera, cameraMode);
        
        // Time for camera caluclations
        Vector3 ogCPos = camera.position;
        //Vector3 ogCTar = camera.target;
        UpdateCamera(&camera, cameraMode);
        // That easy?

        if(collisionDetect()) {
            //camera.target = ogCTar;
            camera.position = ogCPos;
        }

        updateBeanCollide(&camera, cameraMode);
        // It was not that easy.

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                DrawPlane((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector2){ 32.0f, 32.0f }, LIGHTGRAY); // Draw ground
                /*
                DrawCube((Vector3){ -16.0f, 2.5f, 0.0f }, 1.0f, 5.0f, 32.0f, BLUE);     // Draw a blue wall
                DrawCube((Vector3){ 16.0f, 2.5f, 0.0f }, 1.0f, 5.0f, 32.0f, LIME);      // Draw a green wall
                DrawCube((Vector3){ 0.0f, 2.5f, 16.0f }, 32.0f, 5.0f, 1.0f, GOLD);      // Draw a yellow wall
                */

               // DrawLine3D(camera.position, camera.target, YELLOW);

               /* This guy is just a test
               DrawCapsule((Vector3){-3.0f, 2.2f, -3.0f}, (Vector3){-3.0f, 1.0f, -3.0f}, 0.7f, 8, 8, beanColor);
               DrawCapsuleWires((Vector3){-3.0f, 2.2f, -3.0f}, (Vector3){-3.0f, 1.0f, -3.0f}, 0.7f, 8, 8, GREEN);
               */

                DrawSphere((Vector3){-1.0f, 0.0f, -2.0f}, 1.0f, GREEN);
                //DrawSphereWires((Vector3){1.0f, 0.0f, 2.0f}, 2.0f, 16, 16, LIME);
              
                // Draw some cubes around
                for (int i = 0; i < MAX_COLUMNS; i++)
                {
                    DrawCube(positions[i], 2.0f, heights[i], 2.0f, colors[i]);
                    DrawCubeWires(positions[i], 2.0f, heights[i], 2.0f, MAROON);
                }

                // Draw point at target
                if (cameraMode == CAMERA_FIRST_PERSON) {
                    DrawCube(camera.target, 0.5f, 0.5f, 0.5f, YELLOW);
                    DrawCubeWires(camera.target, 0.5f, 0.5f, 0.5f, DARKPURPLE);
                }

                // Draw bean
                if (cameraMode == CAMERA_THIRD_PERSON)
                {
                    DrawCapsule((Vector3){camera.target.x, camera.target.y + 0.2f, camera.target.z}, (Vector3){camera.target.x, camera.target.y - 1.0f, camera.target.z}, 0.7f, 8, 8, beanColor);
                    DrawCapsuleWires((Vector3){camera.target.x, camera.target.y + 0.2f, camera.target.z}, (Vector3){camera.target.x, camera.target.y - 1.0f, camera.target.z}, 0.7f, 8, 8, BLACK);
                    DrawBoundingBox(beanCollide, VIOLET);
                }

            EndMode3D();

            // Draw info boxes
            DrawRectangle(5, 5, 330, 70, Fade(RED, 0.5f));
            DrawRectangleLines(5, 5, 330, 70, BLUE);

            DrawText("Player controls:", 15, 15, 10, BLACK);
            DrawText("- Move keys: W, A, S, D, Space, Left-Ctrl", 15, 30, 10, BLACK);
            DrawText("- Look around: arrow keys or mouse", 15, 45, 10, BLACK);
            DrawText("- Camera mode keys: 1, 2", 15, 60, 10, BLACK);

            DrawRectangle(600, 5, 195, 70, Fade(SKYBLUE, 0.5f));
            DrawRectangleLines(600, 5, 195, 70, BLUE);

            DrawText("Camera status:", 610, 15, 10, BLACK);
            DrawText(TextFormat("- Position: (%06.3f, %06.3f, %06.3f)", camera.position.x, camera.position.y, camera.position.z), 610, 30, 10, BLACK);
            DrawText(TextFormat("- Target: (%06.3f, %06.3f, %06.3f)", camera.target.x, camera.target.y, camera.target.z), 610, 45, 10, BLACK);
            DrawText(TextFormat("- Up: (%06.3f, %06.3f, %06.3f)", camera.up.x, camera.up.y, camera.up.z), 610, 60, 10, BLACK);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}