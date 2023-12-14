#include "raylib/raylib.h"

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

// all of my pride and joy
void UpdateCameraWithBean(LocalBean* bean);
Vector3 GetBeanForward(LocalBean* bean);
Vector3 GetBeanUp(LocalBean* bean);
Vector3 GetBeanRight(LocalBean* bean);
void BeanYaw(LocalBean* bean, float angle, bool rotateAroundTarget);
void BeanPitch(LocalBean* bean, float angle, bool lockView, bool rotateAroundTarget, bool rotateUp);
void UpdateLocalBean(LocalBean* bean);