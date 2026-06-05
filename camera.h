#ifndef CAMERA_H
#define CAMERA_H

//todo, Function rotateCameraAroundCurrentTarget remove static from the pitch and yaw,
// but store them in a struct somehow so that they aren't starting at 0
//tbh, needs an actual struct for whole custom camera so it can store (so far)
//pitch, yaw, clamp value for x and y axis, maybe variables for zoom, etc

#include "raylib.h"
#include "raymath.h"

//initializes a camera with target world origin, z position -5 to the world origin,
//up is the y direction, fovy = 45, and projection is perspective
Camera3D Init3dCamera(){

    Camera3D camera;
    Vector3 world_center = {0,0,0};
    
    camera.position = world_center;
    camera.target = world_center;
    camera.position.z - 5;
    camera.up = (Vector3){0,1,0};
    camera.fovy = 45;
    camera.projection = CAMERA_PERSPECTIVE;

    return camera;
}

void setCameraTarget(Camera3D *camera, Vector3 target){
    camera->target = target;
    camera->position.z = target.z - 10;    
}

void setCameraPosition(Camera3D * camera, Vector3 pos){
    camera->position = pos;
}

void setCameraFovy(Camera3D *camera, float fovy){
    camera->fovy = fovy;
}

//uses mouse movement to rotate the camera
void rotateCameraAroundCurrentTarget(Camera3D * camera){

    Vector2 mouseDelta = GetMouseDelta();

    float distance = Vector3Distance(camera->position, camera->target);
    static float pitch = 0.0f; //use to rotate around X axis
    static float yaw = 0.0f;   //use to rotate around Y axis
    float sensitivity = 0.01f; //mouse movement speed

    yaw += mouseDelta.x * sensitivity;
    pitch += mouseDelta.y *sensitivity;

    float clamp_y = 60.0f * DEG2RAD;

    if (pitch > clamp_y) pitch = clamp_y;
    if (pitch < -clamp_y) pitch = -clamp_y;

    camera->position.x = camera->target.x + distance * cos(yaw) * cos(pitch);
    camera->position.y = camera->target.y + distance * sin(pitch);
    camera->position.z = camera->target.z + distance * sin(yaw) * cos(pitch);
}

#endif