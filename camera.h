#ifndef CAMERA_H
#define CAMERA_H

//todo, Function rotateCameraAroundCurrentTarget remove static from the pitch and yaw,
// but store them in a struct somehow so that they aren't starting at 0
//tbh, needs an actual struct for whole custom camera so it can store (so far)
//pitch, yaw, clamp value for x and y axis, maybe variables for zoom, etc

#include "raylib.h"
#include "raymath.h"

#define LOOK_SENSITIVITY 0.01f
#define CAMERA_DISTANCE_INIT 5.0f

typedef struct custom_cam3d {
    Camera3D cam3D;
    float pitch;
    float yaw;
    float clamp_y;
    float clamp_x;
    float sensitivity;
}custom_cam3d;

//initializes a camera with target world origin, z position -5 to the world origin,
//up is the y direction, fovy = 45, and projection is perspective
custom_cam3d Init3dCamera(){

    custom_cam3d camera;
    Vector3 world_center = {0,0,0};
    
    camera.cam3D.position = world_center;
    camera.cam3D.target = world_center;
    camera.cam3D.position.z - CAMERA_DISTANCE_INIT;
    camera.cam3D.up = (Vector3){0,1,0};
    camera.cam3D.fovy = 45;
    camera.cam3D.projection = CAMERA_PERSPECTIVE;
    camera.pitch = 0.0f;
    camera.yaw = 0.0f;
    camera.sensitivity = LOOK_SENSITIVITY;

    float initial_clamp_y = 45.0f * DEG2RAD;
    float initial_clamp_x = 360.0f * DEG2RAD;

    camera.clamp_x = initial_clamp_x;
    camera.clamp_y = initial_clamp_y;

    return camera;
}

void setCameraTarget(custom_cam3d *camera, Vector3 target){
    camera->cam3D.target = target;
    camera->cam3D.position.z = target.z - CAMERA_DISTANCE_INIT;    
}

void setCameraPosition(custom_cam3d * camera, Vector3 pos){
    camera->cam3D.position = pos;
}

void setCameraFovy(custom_cam3d *camera, float fovy){
    camera->cam3D.fovy = fovy;
}

//uses mouse movement to rotate the camera
void rotateCameraAroundCurrentTarget(custom_cam3d * camera){

    Vector2 mouseDelta = GetMouseDelta();

    float distance = Vector3Distance(camera->cam3D.position, camera->cam3D.target);

    camera->yaw += mouseDelta.x * camera->sensitivity;
    camera->pitch += mouseDelta.y * camera->sensitivity;

    if (camera->pitch > camera->clamp_y) camera->pitch = camera->clamp_y;
    if (camera->pitch < -camera->clamp_y) camera->pitch = -camera->clamp_y;

    if (camera->yaw > camera->clamp_x) camera->yaw = camera->clamp_x;
    if (camera->yaw < -camera->clamp_x) camera->yaw = -camera->clamp_x;

    camera->cam3D.position.x = camera->cam3D.target.x + distance * sin(camera->yaw) * cos(camera->pitch);
    camera->cam3D.position.y = camera->cam3D.target.y + distance * sin(camera->pitch);
    camera->cam3D.position.z = camera->cam3D.target.z + distance * cos(camera->yaw) * cos(camera->pitch);
}

void zoomCamera(custom_cam3d * camera, float zoom){
    Vector3 forward =  Vector3Subtract(camera->cam3D.position, camera->cam3D.target);
    float distance = Vector3Length(forward);

    distance += zoom;
    distance = Clamp(distance, 2.0f, 10.0f);

    Vector3 new_camera_pos = Vector3Add(camera->cam3D.target, Vector3Scale(Vector3Normalize(forward), distance));
    camera->cam3D.position = new_camera_pos;
}

void resetCamera(custom_cam3d * camera, Vector3 target){
    camera->cam3D.position = (Vector3){target.x, target.y, target.z - CAMERA_DISTANCE_INIT};
    camera->pitch = 0.0f;
    camera->yaw = 0.0f; 
    camera->cam3D.target = target;
}

#endif