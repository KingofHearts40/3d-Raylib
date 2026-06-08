#include "camera.h"

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

//set camera target to a Vec3 pos, and then moves the camera behind the Z position of the target
void setCameraTarget(custom_cam3d *camera, Vector3 target){
    camera->cam3D.target = target;
    camera->cam3D.position = target;
    camera->cam3D.position.z = target.z - CAMERA_DISTANCE_INIT;    
}

//changes the camera position to supplied Vec3
void setCameraPosition(custom_cam3d * camera, Vector3 pos){
    camera->cam3D.position = pos;
}

//function to change fovy
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

//allows camera to zoom, Clamp values defined in this function
void zoomCamera(custom_cam3d * camera, float zoom){
    Vector3 forward =  Vector3Subtract(camera->cam3D.position, camera->cam3D.target);
    float distance = Vector3Length(forward);

    distance += zoom;
    distance = Clamp(distance, 2.0f, 10.0f);

    Vector3 new_camera_pos = Vector3Add(camera->cam3D.target, Vector3Scale(Vector3Normalize(forward), distance));
    camera->cam3D.position = new_camera_pos;
}

//resets the camera to be behind the target's z position, resets pitch and yaw
void resetCamera(custom_cam3d * camera, Vector3 target){
    camera->cam3D.position = (Vector3){target.x, target.y, target.z - CAMERA_DISTANCE_INIT};
    camera->pitch = 0.0f;
    camera->yaw = 0.0f; 
    camera->cam3D.target = target;
}
