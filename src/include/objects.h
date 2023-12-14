#include "raylib/raylib.h"

typedef struct Sphere {
    Vector3 position;
    float radius;
    Color color;
    BoundingBox sphereCollide;
} Sphere;