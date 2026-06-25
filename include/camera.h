#ifndef CAMERA_H
#define CAMERA_H

//todo, Function rotateCameraAroundCurrentTarget remove static from the pitch and yaw,
// but store them in a struct somehow so that they aren't starting at 0
//tbh, needs an actual struct for whole custom camera so it can store (so far)
//pitch, yaw, clamp value for x and y axis, maybe variables for zoom, etc

#include "raylib.h"
#include "raymath.h"
#include "global_constants.h"

#define LOOK_SENSITIVITY 0.01f
#define CAMERA_DISTANCE_INIT 5.0f

typedef struct custom_cam3d {
    Camera3D cam3D;
    float pitch;
    float yaw;
    float clamp_y;
    float clamp_x;
    float sensitivity;
    float min_zoom;
    float max_zoom;
    Vector3 x_axis;
}custom_cam3d;


custom_cam3d Init3dCamera();
void setCameraTarget(custom_cam3d *camera, Vector3 target);
void setCameraPosition(custom_cam3d * camera, Vector3 pos);
void setCameraFovy(custom_cam3d *camera, float fovy);
void setCameraZoomParam(custom_cam3d *c, float min_zoom, float max_zoom);
void rotateCameraAroundCurrentTarget(custom_cam3d * camera);
void MoveCameraPos(custom_cam3d *c);
Vector3 getCameraDirectionX(custom_cam3d *c);
void zoomCamera(custom_cam3d *camera, float zoom);
void resetCamera(custom_cam3d * camera, Vector3 target);

#endif