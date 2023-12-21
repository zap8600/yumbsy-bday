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

#include <stdio.h>

#include "rlh/raylib.h"
#include "rlh/rcamera.h"
#include "rlh/raymath.h"
#include "objects.h"
#include "player.h"
#include "net/net_client.h"

#define MAX_COLUMNS 10

int LocalPlayerId = -1;

Bean beans[MAX_PLAYERS] = { 0 };

// the enet address we are connected to
ENetAddress address = { 0 };

// the server object we are connecting to
ENetPeer* server = { 0 };

// the client peer we are using
ENetHost* client = { 0 };

// how long in seconds since the last time we sent an update
double LastInputSend = -100;

// how long to wait between updates (20 update ticks a second)
double InputUpdateInterval = 1.0f / 20.0f;

double LastNow = 0;

LocalBean bean = { 0 };

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "bean game proto - happy xmas yumbsy");

    // Define the camera to look into our 3d world (position, target, up vector)
    //bean.transform.translation = (Vector3){ 0.0f, 1.7f, 4.0f };    // Camera position
    //bean.target = (Vector3){ 0.0f, 1.7f, 0.0f };      // Camera looking at point
    bean.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    
    bean.camera.fovy = 60.0f;                                // Camera field-of-view Y
    bean.camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type
    bean.cameraMode = CAMERA_FIRST_PERSON;
    UpdateCameraWithBean(&bean, sym);

    // Generates some random columns
    float heights[MAX_COLUMNS] = { 0 };
    Vector3 positions[MAX_COLUMNS] = { 0 };
    Color colors[MAX_COLUMNS] = { 0 };

    for (int i = 0; i < MAX_COLUMNS; i++)
    {
        heights[i] = (float)(GetRandomValue(1, 12));
        positions[i] = (Vector3){ (float)(GetRandomValue(-15, 15)), heights[i]/2.0f, (float)(GetRandomValue(-15, 15)) };
        colors[i] = (Color){ (GetRandomValue(20, 255)), (GetRandomValue(10, 55)), 30, 255 };
    }

    bean.beanColor = (Color){ (GetRandomValue(0, 255)), (GetRandomValue(0, 255)), (GetRandomValue(0, 255)), (GetRandomValue(0, 255)) };

    // sphere time
    /*
    Sphere thatSphere = { 0 };
    thatSphere.position = (Vector3){-1.0f, 1.0f, -2.0f};
    thatSphere.radius = 1.0f;
    thatSphere.color = GREEN;
    thatSphere.sphereCollide = (BoundingBox){
        Vector3SubtractValue(thatSphere.position, thatSphere.radius),
        Vector3AddValue(thatSphere.position, thatSphere.radius)
    };
    */

    DisableCursor();                    // Limit cursor to relative movement inside the window

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second

    bool connected = false;
    bool client = false;
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!(WindowShouldClose()))        // Detect window close button or ESC key
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

        if ((IsKeyPressed(KEY_ONE)))
        {
            if(bean.cameraMode != CAMERA_FIRST_PERSON) {
                bean.cameraMode = CAMERA_FIRST_PERSON;
                bean.up = (Vector3){ 0.0f, 1.0f, 0.0f }; // Reset roll
                UpdateCameraWithBean(&bean, sym);
                //updateBeanCollide(&camera, cameraMode);
            }
        }

        if ((IsKeyPressed(KEY_TWO)))
        {
            if(bean.cameraMode != CAMERA_THIRD_PERSON) {
                bean.cameraMode = CAMERA_THIRD_PERSON;
                bean.up = (Vector3){ 0.0f, 1.0f, 0.0f }; // Reset roll
                UpdateCameraWithBean(&bean, sym);
                //updateBeanCollide(&camera, cameraMode);
            }
        }

        if ((IsKeyPressed(KEY_THREE)))
        {
            // setup client
            if(!client) {
                client = true;
                Connect("127.0.0.1");
            }
        }

        //UpdateLocalBean(&bean);
        
        if (Connected()) {
            connected = true;
            UpdateLocalBean(&bean, sym);
        } else if (connected) {
            // they hate us sadge
            Connect("127.0.0.1");
            connected = false;
        }
        Update(GetTime(), GetFrameTime(), &bean);

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(bean.camera);

                DrawPlane((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector2){ 32.0f, 32.0f }, LIGHTGRAY); // Draw ground

                if (!Connected()) {
                    //DrawText("Connecting", 15, 75, 10, BLACK);
                } else {
                    //DrawText(TextFormat("Player %d", GetLocalPlayerId()), 15, 75, 10, BLACK);
                    printf("Bean %d position: x=%f, y=%f, z=%f\n", LocalPlayerId, bean.transform.translation.x, bean.transform.translation.y, bean.transform.translation.z);

                    for (int i = 0; i < MAX_PLAYERS; i++) {
                        if(i != LocalPlayerId) {
                            Vector3 pos = { 0 };
                            uint8_t r;
                            uint8_t g;
                            uint8_t b;
                            uint8_t a;
                            if(GetPlayerPos(i, &pos) && GetPlayerR(i, &r) && GetPlayerG(i, &g) && GetPlayerB(i, &b), GetPlayerA(i, &a)) {
                                DrawCapsule(
                                    (Vector3){pos.x, pos.y + 0.2f, pos.z},
                                    (Vector3){pos.x, pos.y - 1.0f, pos.z},
                                    0.7f, 8, 8, (Color){ r, g, b, a }
                                );
                                DrawCapsuleWires(
                                    (Vector3){pos.x, pos.y + 0.2f, pos.z},
                                    (Vector3){pos.x, pos.y - 1.0f, pos.z},
                                    0.7f, 8, 8, BLACK // an L color tbh
                                );
                            }
                        }
                    }
                }
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

                //DrawSphere(thatSphere.position, thatSphere.radius, thatSphere.color);
                //DrawBoundingBox(thatSphere.sphereCollide, VIOLET);

                /*
                // Draw some cubes around
                for (int i = 0; i < MAX_COLUMNS; i++)
                {
                    //DrawCube(positions[i], 2.0f, heights[i], 2.0f, colors[i]);
                    //DrawCubeWires(positions[i], 2.0f, heights[i], 2.0f, MAROON);
                }
                */

                // Draw point at target
                //DrawCube(bean.target, 0.5f, 0.5f, 0.5f, YELLOW);
                //DrawCubeWires(bean.target, 0.5f, 0.5f, 0.5f, DARKPURPLE);

                // Draw bean
                if (bean.cameraMode == CAMERA_THIRD_PERSON)
                {
                    DrawCapsule(bean.topCap, bean.botCap, 0.7f, 8, 8, bean.beanColor);
                    DrawCapsuleWires(bean.topCap, bean.botCap, 0.7f, 8, 8, BLACK);
                    //DrawBoundingBox(beanCollide, VIOLET);
                }

            EndMode3D();

            // Draw info boxes
            DrawRectangle(5, 5, 330, 85, RED);
            DrawRectangleLines(5, 5, 330, 85, BLUE);

            DrawText("Player controls:", 15, 15, 10, BLACK);
            DrawText("- Move keys: W, A, S, D, Space, Left-Ctrl", 15, 30, 10, BLACK);
            DrawText("- Look around: arrow keys or mouse", 15, 45, 10, BLACK);
            DrawText("- Camera mode keys: 1, 2", 15, 60, 10, BLACK);
            DrawText("- Connect to network: 3", 15, 75, 10, BLACK);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    Disconnect();
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

void UpdatePlayer(Vector3 pos, Vector3 tar) {
    bean.transform.translation = pos;
    bean.target = tar;
}
