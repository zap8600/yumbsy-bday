#include "rlh/raylib.h"

Vector3 BeanVector3Add(Vector3 v1, Vector3 v2);
Vector3 BeanVector3AddValue(Vector3 v, float add);
Vector3 BeanVector3Subtract(Vector3 v1, Vector3 v2);
Vector3 BeanVector3SubtractValue(Vector3 v, float sub);
Vector3 BeanVector3Scale(Vector3 v, float scalar);
Vector3 BeanVector3CrossProduct(Vector3 v1, Vector3 v2);
float BeanVector3Angle(Vector3 v1, Vector3 v2);
Vector3 BeanVector3Negate(Vector3 v);
Vector3 BeanVector3Normalize(Vector3 v);
Vector3 BeanVector3RotateByAxisAngle(Vector3 v, Vector3 axis, float angle);