#include "rlh/raylib.h"
#include "syms.h"
#include "net/net_constants.h"

// the player id of this client
//int LocalPlayerId = -1;

// this struct wont be used until networking is added
typedef struct Bean {
    Vector3 position; // player position
    Color beanColor; // player color
    bool active; // are they awake
    double updateTime; // time of last update
} Bean;

typedef struct LocalBean {
    Transform transform; // player position, rotation, and scale
    Vector3 target; // player target
    Vector3 up; // player up? used for rolling i think
    BoundingBox beanCollide;
    Color beanColor; // player color
    Vector3 topCap; // start cap for capsule
    Vector3 botCap; // end cap for capsule
    Camera camera; // camera
    int cameraMode; // camera mode
} LocalBean;

//Bean beans[MAX_PLAYERS] = { 0 };

// all of my pride and joy
void UpdateCameraWithBean(LocalBean* bean, struct raylib_syms *sym);
Vector3 GetBeanForward(LocalBean* bean, struct raylib_syms *sym);
Vector3 GetBeanUp(LocalBean* bean, struct raylib_syms *sym);
Vector3 GetBeanRight(LocalBean* bean, struct raylib_syms *sym);
void BeanYaw(LocalBean* bean, float angle, bool rotateAroundTarget, struct raylib_syms *sym);
void BeanPitch(LocalBean* bean, float angle, bool lockView, bool rotateAroundTarget, bool rotateUp, struct raylib_syms *sym);
void UpdateLocalBean(LocalBean* bean, struct raylib_syms *sym);

// defined in main.c
void UpdatePlayerList(LocalBean* bean);