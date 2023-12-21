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
#include "syms.h"

#define _COSMO_SOURCE
#include "libc/dlopen/dlfcn.h"

#define ENET_IMPLEMENTATION
#include "net/net_common.h"

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

static void *try_find_raylib_lib(void) {
    char *candidates[] = {
        "libraylib.so",
        "raylib.dll"
    };
    void *lib = NULL;
    for(size_t i = 0; i < (sizeof(candidates) / sizeof(*candidates)); ++i) {
        if ((lib = cosmo_dlopen(candidates[i], RTLD_LAZY))) return lib;
    }

    for(size_t i = 0; i < (sizeof(candidates) / sizeof(*candidates)); ++i) {
        printf("\"%s\" ", candidates[i]);
    }
    printf("\n");

    return NULL;
}

static struct raylib_syms *try_get_raylib_syms(void) {
    void *raylib = try_find_raylib_lib();
    if(!raylib) return NULL;
    struct raylib_syms *syms = calloc(1, sizeof(*syms));
    *syms = (struct raylib_syms) {
        .lib = raylib,
        .GetRandomValue = cosmo_dlsym(raylib, "GetRandomValue"),
        .DrawPlane = cosmo_dlsym(raylib, "DrawPlane"),
        .DisableCursor = cosmo_dlsym(raylib, "DisableCursor"),
        .EndMode3D = cosmo_dlsym(raylib, "EndMode3D"),
        .BeginMode3D = cosmo_dlsym(raylib, "BeginMode3D"),
        .DrawCapsule = cosmo_dlsym(raylib, "DrawCapsule"),
        .DrawCapsuleWires = cosmo_dlsym(raylib, "DrawCapsuleWires"),
        .TextFormat = cosmo_dlsym(raylib, "TextFormat"),
        .DrawRectangle = cosmo_dlsym(raylib, "DrawRectangle"),
        .DrawRectangleLines = cosmo_dlsym(raylib, "DrawRectangleLines"),
        .Fade = cosmo_dlsym(raylib, "Fade"),
        .GetMouseDelta = cosmo_dlsym(raylib, "GetMouseDelta"),
        .GetTime = cosmo_dlsym(raylib, "GetTime"),
        .GetFrameTime = cosmo_dlsym(raylib, "GetFrameTime"),
        .InitWindow = cosmo_dlsym(raylib, "InitWindow"),
        .CloseWindow = cosmo_dlsym(raylib, "CloseWindow"),
        .WindowShouldClose = cosmo_dlsym(raylib, "WindowShouldClose"),
        .ClearBackground = cosmo_dlsym(raylib, "ClearBackground"),
        .BeginDrawing = cosmo_dlsym(raylib, "BeginDrawing"),
        .EndDrawing = cosmo_dlsym(raylib, "EndDrawing"),
        .SetTargetFPS = cosmo_dlsym(raylib, "SetTargetFPS"),
        .IsKeyPressed = cosmo_dlsym(raylib, "IsKeyPressed"),
        .DrawText = cosmo_dlsym(raylib, "DrawText"),
        .InitAudioDevice = cosmo_dlsym(raylib, "InitAudioDevice"),
        .CloseAudioDevice = cosmo_dlsym(raylib, "CloseAudioDevice"),
        .LoadMusicStream = cosmo_dlsym(raylib, "LoadMusicStream"),
        .UnloadMusicStream = cosmo_dlsym(raylib, "UnloadMusicStream"),
        .PlayMusicStream = cosmo_dlsym(raylib, "PlayMusicStream"),
        .StopMusicStream = cosmo_dlsym(raylib, "StopMusicStream"),
        .UpdateMusicStream = cosmo_dlsym(raylib, "UpdateMusicStream")
    };

    for (size_t i = 0; i < (sizeof(struct raylib_syms) / sizeof(void *)); ++i) {
		if (!((void **)syms)[i]) {
			printf("raylib_syms[%zu] is NULL, check for typos\n", i);
			free(syms);
			return NULL;
		}
	}
	return syms;
}

void Connect(const char* serverAddress);
void Update(double now, float deltaT, LocalBean* bean);
void Disconnect();
bool Connected();
int GetLocalPlayerId();
bool GetPlayerPos(int id, Vector3* pos);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    struct raylib_syms *sym = try_get_raylib_syms();
    if(!sym) return -1;
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    sym->InitWindow(screenWidth, screenHeight, "bean game proto - happy xmas yumbsy");

    LocalBean bean = { 0 };

    // Define the camera to look into our 3d world (position, target, up vector)
    //bean.transform.translation = (Vector3){ 0.0f, 1.7f, 4.0f };    // Camera position
    //bean.target = (Vector3){ 0.0f, 1.7f, 0.0f };      // Camera looking at point
    bean.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    
    bean.camera.fovy = 60.0f;                                // Camera field-of-view Y
    bean.camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type
    bean.cameraMode = CAMERA_FIRST_PERSON;
    UpdateCameraWithBean(&bean);

    // Generates some random columns
    float heights[MAX_COLUMNS] = { 0 };
    Vector3 positions[MAX_COLUMNS] = { 0 };
    Color colors[MAX_COLUMNS] = { 0 };

    for (int i = 0; i < MAX_COLUMNS; i++)
    {
        heights[i] = (float)(sym->GetRandomValue(1, 12));
        positions[i] = (Vector3){ (float)(sym->GetRandomValue(-15, 15)), heights[i]/2.0f, (float)(sym->GetRandomValue(-15, 15)) };
        colors[i] = (Color){ (sym->GetRandomValue(20, 255)), (sym->GetRandomValue(10, 55)), 30, 255 };
    }

    bean.beanColor = (Color){ (sym->GetRandomValue(0, 255)), (sym->GetRandomValue(0, 255)), (sym->GetRandomValue(0, 255)), (sym->GetRandomValue(0, 255)) };

    // sphere time
    Sphere thatSphere = { 0 };
    thatSphere.position = (Vector3){-1.0f, 1.0f, -2.0f};
    thatSphere.radius = 1.0f;
    thatSphere.color = GREEN;
    thatSphere.sphereCollide = (BoundingBox){
        Vector3SubtractValue(thatSphere.position, thatSphere.radius),
        Vector3AddValue(thatSphere.position, thatSphere.radius)
    };

    sym->DisableCursor();                    // Limit cursor to relative movement inside the window

    sym->SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second

    bool connected = false;
    bool client = false;
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!(sym->WindowShouldClose()))        // Detect window close button or ESC key
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

        if ((sym->IsKeyPressed(KEY_ONE)))
        {
            if(bean.cameraMode != CAMERA_FIRST_PERSON) {
                bean.cameraMode = CAMERA_FIRST_PERSON;
                bean.up = (Vector3){ 0.0f, 1.0f, 0.0f }; // Reset roll
                UpdateCameraWithBean(&bean);
                //updateBeanCollide(&camera, cameraMode);
            }
        }

        if ((sym->IsKeyPressed(KEY_TWO)))
        {
            if(bean.cameraMode != CAMERA_THIRD_PERSON) {
                bean.cameraMode = CAMERA_THIRD_PERSON;
                bean.up = (Vector3){ 0.0f, 1.0f, 0.0f }; // Reset roll
                UpdateCameraWithBean(&bean);
                //updateBeanCollide(&camera, cameraMode);
            }
        }

        if ((sym->IsKeyPressed(KEY_THREE)))
        {
            // setup client
            if(!client) {
                client = true;
                Connect(argv[1]);
            }
        }

        //UpdateLocalBean(&bean);
        
        if (Connected()) {
            connected = true;
            UpdateLocalBean(&bean, sym);
        } else if (connected) {
            // they hate us sadge
            Connect(argv[1]);
            connected = false;
        }
        Update(sym->GetTime(), sym->GetFrameTime(), &bean);

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        sym->BeginDrawing();

            sym->ClearBackground(RAYWHITE);

            sym->BeginMode3D(bean.camera);

                sym->DrawPlane((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector2){ 32.0f, 32.0f }, LIGHTGRAY); // Draw ground

                if (!Connected()) {
                    //sym->DrawText("Connecting", 15, 75, 10, BLACK);
                } else {
                    //sym->DrawText(TextFormat("Player %d", GetLocalPlayerId()), 15, 75, 10, BLACK);

                    for (int i = 0; i < MAX_PLAYERS; i++) {
                        if(i != LocalPlayerId) {
                            Vector3 pos = { 0 };
                            if(GetPlayerPos(i, &pos)) {
                                sym->DrawCapsule(
                                    (Vector3){pos.x, pos.y + 0.2f, pos.z},
                                    (Vector3){pos.x, pos.y - 1.0f, pos.z},
                                    0.7f, 8, 8, beans[i].beanColor
                                );
                                sym->DrawCapsuleWires(
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
                    sym->DrawCapsule(bean.topCap, bean.botCap, 0.7f, 8, 8, bean.beanColor);
                    sym->DrawCapsuleWires(bean.topCap, bean.botCap, 0.7f, 8, 8, BLACK);
                    //DrawBoundingBox(beanCollide, VIOLET);
                }

            sym->EndMode3D();

            // Draw info boxes
            sym->DrawRectangle(5, 5, 330, 85, sym->Fade(RED, 0.5f));
            sym->DrawRectangleLines(5, 5, 330, 85, BLUE);

            sym->DrawText("Player controls:", 15, 15, 10, BLACK);
            sym->DrawText("- Move keys: W, A, S, D, Space, Left-Ctrl", 15, 30, 10, BLACK);
            sym->DrawText("- Look around: arrow keys or mouse", 15, 45, 10, BLACK);
            sym->DrawText("- Camera mode keys: 1, 2", 15, 60, 10, BLACK);
            sym->DrawText("- Connect to network: 3", 15, 75, 10, BLACK);

        sym->EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    Disconnect();
    sym->CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

// Connect to a server
void Connect(const char* serverAddress)
{
	// startup the network library
	enet_initialize();

	// create a client that we will use to connect to the server
	client = enet_host_create(NULL, 1, 1, 0, 0);

	// set the address and port we will connect to
	enet_address_set_host(&address, serverAddress);
	address.port = 4545;

	// start the connection process. Will be finished as part of our update
	server = enet_host_connect(client, &address, 1, 0);
}

Vector3 ReadPosition(ENetPacket* packet, size_t* offset)
{
	Vector3 pos = { 0 };
	pos.x = ReadFloat(packet, offset);
	pos.y = ReadFloat(packet, offset);
    pos.z = ReadFloat(packet, offset);

	return pos;
}

Color ReadColor(ENetPacket* packet, size_t* offset) {
    Color color = { 0 };
    color.r = ReadByte(packet, offset);
    color.g = ReadByte(packet, offset);
    color.b = ReadByte(packet, offset);
    color.a = ReadByte(packet, offset);

    return color;
}

// A new remote player was added to our local simulation
void HandleAddPlayer(ENetPacket* packet, size_t* offset)
{
	// find out who the server is talking about
	int remotePlayer = ReadByte(packet, offset);
	if (remotePlayer >= MAX_PLAYERS || remotePlayer == LocalPlayerId)
		return;

	// set them as active and update the location
	printf("Bean %d added\n", remotePlayer);
	beans[remotePlayer].position = ReadPosition(packet, offset);
    beans[remotePlayer].beanColor = ReadColor(packet, offset);
    beans[remotePlayer].active = true;
	beans[remotePlayer].updateTime = LastNow;
	printf("Bean %d position: x=%f, y=%f, z=%f\n", remotePlayer, beans[remotePlayer].position.x, beans[remotePlayer].position.y, beans[remotePlayer].position.z);

	// In a more robust game, this message would have more info about the new player, such as what sprite or model to use, player name, or other data a client would need
	// this is where static data about the player would be sent, and any initial state needed to setup the local simulation
}

// A remote player has left the game and needs to be removed from the local simulation
void HandleRemovePlayer(ENetPacket* packet, size_t* offset)
{
	// find out who the server is talking about
	int remotePlayer = ReadByte(packet, offset);
	if (remotePlayer >= MAX_PLAYERS || remotePlayer == LocalPlayerId)
		return;

	// remove the player from the simulation. No other data is needed except the player id
	printf("Bean %d removed\n", remotePlayer); // they may be black
	beans[remotePlayer].active = false;
}

// The server has a new position for a player in our local simulation
void HandleUpdatePlayer(ENetPacket* packet, size_t* offset)
{
	// find out who the server is talking about
	int remotePlayer = ReadByte(packet, offset);
	if (remotePlayer >= MAX_PLAYERS || remotePlayer == LocalPlayerId || !beans[remotePlayer].active)
		return;

	// update the last known position and movement
	//printf("Bean %d update\n", remotePlayer);
	beans[remotePlayer].position = ReadPosition(packet, offset);
    beans[remotePlayer].beanColor = ReadColor(packet, offset);
	beans[remotePlayer].updateTime = LastNow;

	// in a more robust game this message would have a tick ID for what time this information was valid, and extra info about
	// what the input state was so the local simulation could do prediction and smooth out the motion
}

// process one frame of updates
void Update(double now, float deltaT, LocalBean* bean)
{
	LastNow = now;
	// if we are not connected to anything yet, we can't do anything, so bail out early
	if (server == NULL)
		return;

	// Check if we have been accepted, and if so, check the clock to see if it is time for us to send the updated position for the local player
	// we do this so that we don't spam the server with updates 60 times a second and waste bandwidth
	// in a real game we'd send our normalized movement vector or input keys along with what the current tick index was
	// this way the server can know how long it's been since the last update and can do interpolation to know were we are between updates.
	if (LocalPlayerId >= 0 && now - LastInputSend > InputUpdateInterval)
	{
		// Pack up a buffer with the data we want to send
		uint8_t buffer[17] = { 0 }; // 10 bytes for a 1 byte command number and two bytes for each X and Y value
		buffer[0] = (uint8_t)UpdateInput;   // this tells the server what kind of data to expect in this packet
		*(float*)(buffer + 1) = (float)beans[LocalPlayerId].position.x;
		*(float*)(buffer + 5) = (float)beans[LocalPlayerId].position.y;
		*(float*)(buffer + 9) = (float)beans[LocalPlayerId].position.z;
        *(uint8_t*)(buffer + 13) = (uint8_t)beans[LocalPlayerId].beanColor.r;
        *(uint8_t*)(buffer + 14) = (uint8_t)beans[LocalPlayerId].beanColor.g;
        *(uint8_t*)(buffer + 15) = (uint8_t)beans[LocalPlayerId].beanColor.b;
        *(uint8_t*)(buffer + 16) = (uint8_t)beans[LocalPlayerId].beanColor.a;

        // copy this data into a packet provided by enet (TODO : add pack functions that write directly to the packet to avoid the copy)
		ENetPacket* packet = enet_packet_create(buffer, 17, ENET_PACKET_FLAG_RELIABLE);

		// send the packet to the server
		enet_peer_send(server, 0, packet);

		// NOTE enet_host_service will handle releasing send packets when the network system has finally sent them,
		// you don't have to destroy them

		// mark that now was the last time we sent an update
		LastInputSend = now;
    }

    // read one event from enet and process it
	ENetEvent Event = { 0 };

    if (enet_host_service(client, &Event, 0) > 0)
	{
		// see what kind of event it is
		switch (Event.type)
		{
			// the server sent us some data, we should process it
			case ENET_EVENT_TYPE_RECEIVE:
			{
				// we know that all valid packets have a size >= 1, so if we get this, something is bad and we ignore it.
				if (Event.packet->dataLength < 1)
					break;

				// keep an offset of what data we have read so far
				size_t offset = 0;

				// read off the command that the server wants us to do
				NetworkCommands command = (NetworkCommands)ReadByte(Event.packet, &offset);

                // if the server has not accepted us yet, we are limited in what packets we can receive
				if (LocalPlayerId == -1)
				{
					if (command == AcceptPlayer)    // this is the only thing we can do in this state, so ignore anything else
					{
						// See who the server says we are
						LocalPlayerId = ReadByte(Event.packet, &offset);
						printf("Local ID = %d\n", LocalPlayerId);

						// Make sure that it makes sense
						if (LocalPlayerId < 0 || LocalPlayerId > MAX_PLAYERS)
						{
							LocalPlayerId = -1;
							break;
						}

						// Force the next frame to do an update by pretending it's been a very long time since our last update
						LastInputSend = -InputUpdateInterval;

						// We are active
						beans[LocalPlayerId].active = true;

						// Set our player at some location on the field.
						// optimally we would do a much more robust connection negotiation where we tell the server what our name is, what we look like
						// and then the server tells us where we are
						// But for this simple test, everyone starts at the same place on the field
						bean->transform.translation = (Vector3){ 0.0f, 1.7f, 4.0f };    // Camera position
                        bean->target = (Vector3){ 0.0f, 1.7f, 0.0f };      // Camera looking at point
                        UpdatePlayerList(bean);
					}
				}
                else // we have been accepted, so process play messages from the server
				{
					// see what the server wants us to do
					switch (command)
					{
						case AddPlayer:
							HandleAddPlayer(Event.packet, &offset);
							break;

						case RemovePlayer:
							HandleRemovePlayer(Event.packet, &offset);
							break;

						case UpdatePlayer:
							HandleUpdatePlayer(Event.packet, &offset);
							break;
					}
				}
				// tell enet that it can recycle the packet data
				enet_packet_destroy(Event.packet);
				break;
			}

            // we were disconnected, we have a sad
			case ENET_EVENT_TYPE_DISCONNECT:
				server = NULL;
				LocalPlayerId = -1;
				break;
		}
	}
}

// force a disconnect by shutting down enet
void Disconnect()
{
	// close our connection to the server
	if (server != NULL)
		enet_peer_disconnect(server, 0);

	// close our client
	if (client != NULL)
		enet_host_destroy(client);

	client = NULL;
	server = NULL;

	// clean up enet
	enet_deinitialize();
}

// true if we are connected and have been accepted
bool Connected()
{
	return server != NULL && LocalPlayerId >= 0;
}

int GetLocalPlayerId()
{
	return LocalPlayerId;
}

// get the info for a particular player
bool GetPlayerPos(int id, Vector3* pos)
{
	// make sure the player is valid and active
	if (id < 0 || id >= MAX_PLAYERS || !beans[id].active)
		return false;

	*pos = beans[id].position;
	return true;
}

void UpdatePlayerList(LocalBean* bean) {
    beans[LocalPlayerId].position = bean->transform.translation;
    beans[LocalPlayerId].beanColor = bean->beanColor;
}
