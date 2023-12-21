#include "player.h"
#include "rlh/raylib.h"
#include "rlh/rcamera.h"
#include "rlh/raymath.h"
#include "net/net_client.h"
#include <stdio.h>

void UpdateCameraWithBean(LocalBean* bean, struct raylib_syms *sym) {
    if(bean->cameraMode == CAMERA_FIRST_PERSON) {
        bean->camera.position = bean->transform.translation;
        bean->camera.target = bean->target;
        bean->camera.up = bean->up;
    } else { // assume third person for time
        Vector3 newCameraPosition = Vector3Negate(Vector3Subtract(bean->target, bean->transform.translation));
        bean->camera.position = Vector3Add(bean->transform.translation, newCameraPosition);
        bean->camera.target = bean->transform.translation;
        bean->camera.up = bean->up;
    }
}

Vector3 GetBeanForward(LocalBean* bean, struct raylib_syms *sym) {
    return Vector3Normalize(Vector3Subtract(bean->target, bean->transform.translation));
}

Vector3 GetBeanUp(LocalBean* bean, struct raylib_syms *sym)
{
    return Vector3Normalize(bean->up);
}

Vector3 GetBeanRight(LocalBean* bean, struct raylib_syms *sym)
{
    Vector3 forward = GetBeanForward(bean, sym);
    Vector3 up = GetBeanUp(bean, sym);

    return Vector3CrossProduct(forward, up);
}

void BeanMoveForward(LocalBean* bean, float distance, bool moveInWorldPlane, struct raylib_syms *sym) {
    Vector3 forward = GetBeanForward(bean, sym);

    if (moveInWorldPlane)
    {
        // Project vector onto world plane
        forward.y = 0;
        forward = Vector3Normalize(forward);
    }

    // Scale by distance
    forward = Vector3Scale(forward, distance);

    bean->transform.translation = Vector3Add(bean->transform.translation, forward);
    bean->target = Vector3Add(bean->target, forward);
    UpdateCameraWithBean(bean, sym);
}

void BeanMoveRight(LocalBean* bean, float distance, bool moveInWorldPlane, struct raylib_syms *sym)
{
    Vector3 right = GetBeanRight(bean, sym);

    if (moveInWorldPlane)
    {
        // Project vector onto world plane
        right.y = 0;
        right = Vector3Normalize(right);
    }

    // Scale by distance
    right = Vector3Scale(right, distance);

    // Move position and target
    bean->transform.translation = Vector3Add(bean->transform.translation, right);
    bean->target = Vector3Add(bean->target, right);
    UpdateCameraWithBean(bean, sym);
}

void BeanYaw(LocalBean* bean, float angle, bool rotateAroundTarget, struct raylib_syms *sym)
{
    // Rotation axis
    Vector3 up = GetBeanUp(bean, sym);

    // View vector
    Vector3 targetPosition = Vector3Subtract(bean->target, bean->transform.translation);

    // Rotate view vector around up axis
    targetPosition = Vector3RotateByAxisAngle(targetPosition, up, angle);

    bean->target = Vector3Add(bean->transform.translation, targetPosition);
    
    UpdateCameraWithBean(bean, sym);
}

void BeanPitch(LocalBean* bean, float angle, bool lockView, bool rotateAroundTarget, bool rotateUp, struct raylib_syms *sym)
{
    // Up direction
    Vector3 up = GetBeanUp(bean, sym);

    // View vector
    Vector3 targetPosition = Vector3Subtract(bean->target, bean->transform.translation);

    if (lockView)
    {
        // In these camera modes we clamp the Pitch angle
        // to allow only viewing straight up or down.

        // Clamp view up
        float maxAngleUp = Vector3Angle(up, targetPosition);
        maxAngleUp -= 0.001f; // avoid numerical errors
        if (angle > maxAngleUp) angle = maxAngleUp;

        // Clamp view down
        float maxAngleDown = Vector3Angle(Vector3Negate(up), targetPosition);
        maxAngleDown *= -1.0f; // downwards angle is negative
        maxAngleDown += 0.001f; // avoid numerical errors
        if (angle < maxAngleDown) angle = maxAngleDown;
    }

    // Rotation axis
    Vector3 right = GetBeanRight(bean, sym);

    // Rotate view vector around right axis
    targetPosition = Vector3RotateByAxisAngle(targetPosition, right, angle);
    
    bean->target = Vector3Add(bean->transform.translation, targetPosition);

    if (rotateUp)
    {
        // Rotate up direction around right axis
        bean->up = Vector3RotateByAxisAngle(bean->up, right, angle);
    }
    UpdateCameraWithBean(bean, sym);
}

#define BEAN_MOVE_SPEED 0.09f
#define CAMERA_MOUSE_SPEED 0.003f

void UpdateLocalBean(LocalBean* bean, struct raylib_syms *sym) {
    Vector2 mousePositionDelta = GetMouseDelta();

    bool moveInWorldPlane = ((bean->cameraMode == CAMERA_FIRST_PERSON) || (bean->cameraMode == CAMERA_THIRD_PERSON));
    bool rotateAroundTarget = ((bean->cameraMode == CAMERA_THIRD_PERSON) || (bean->cameraMode == CAMERA_ORBITAL));
    bool lockView = ((bean->cameraMode == CAMERA_FREE) || (bean->cameraMode == CAMERA_FIRST_PERSON) || (bean->cameraMode == CAMERA_THIRD_PERSON) || (bean->cameraMode == CAMERA_ORBITAL));
    bool rotateUp = false;

    BeanYaw(bean, -mousePositionDelta.x*CAMERA_MOUSE_SPEED, rotateAroundTarget, sym);
    BeanPitch(bean, -mousePositionDelta.y*CAMERA_MOUSE_SPEED, lockView, rotateAroundTarget, rotateUp, sym);

    if (IsKeyDown(KEY_W)) BeanMoveForward(bean, BEAN_MOVE_SPEED, moveInWorldPlane, sym);
    if (IsKeyDown(KEY_A)) BeanMoveRight(bean, -BEAN_MOVE_SPEED, moveInWorldPlane, sym);
    if (IsKeyDown(KEY_S)) BeanMoveForward(bean, -BEAN_MOVE_SPEED, moveInWorldPlane, sym);
    if (IsKeyDown(KEY_D)) BeanMoveRight(bean, BEAN_MOVE_SPEED, moveInWorldPlane, sym);

    bean->beanCollide = (BoundingBox){
                        (Vector3){bean->transform.translation.x - 0.7f, bean->transform.translation.y - 1.7f, bean->transform.translation.z - 0.7f},
                        (Vector3){bean->transform.translation.x + 0.7f, bean->transform.translation.y + 0.9f, bean->transform.translation.z + 0.7f}};
    bean->topCap = (Vector3){bean->transform.translation.x, bean->transform.translation.y + 0.2f, bean->transform.translation.z};
    bean->botCap = (Vector3){bean->transform.translation.x, bean->transform.translation.y - 1.0f, bean->transform.translation.z};

    // update the local player in the player list
    UpdatePlayerList(bean->transform.translation, bean->beanColor.r, bean->beanColor.g, bean->beanColor.b, bean->beanColor.a);
}