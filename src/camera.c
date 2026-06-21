#include "camera.h"

//initializes a camera with target world origin, z position -5 to the world origin,
//up is the y direction, fovy = 45, and projection is perspective
custom_cam3d Init3dCamera(){

    custom_cam3d camera;
    Vector3 world_center = {0,0,0};
    
    camera.cam3D.position = world_center;
    camera.cam3D.target = world_center;
    camera.cam3D.up = VectorUp;
    camera.x_axis = VectorRight;
    camera.cam3D.fovy = 45;
    camera.cam3D.projection = CAMERA_PERSPECTIVE;
    camera.pitch = 0.0f;
    camera.yaw = 0.0f;
    camera.sensitivity = LOOK_SENSITIVITY;
    camera.max_zoom = 10.0f;

    float initial_clamp_y = 89.0f * DEG2RAD;
    float initial_clamp_x = 360.0f * DEG2RAD;

    camera.clamp_x = initial_clamp_x;
    camera.clamp_y = initial_clamp_y;

    return camera;
}

//set camera target to a Vec3 pos
void setCameraTarget(custom_cam3d *camera, Vector3 target){
    camera->cam3D.target = target;   
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

    camera->yaw += -mouseDelta.x * camera->sensitivity;
    camera->pitch += mouseDelta.y * camera->sensitivity;

    if (camera->pitch > camera->clamp_y) camera->pitch = camera->clamp_y;
    if (camera->pitch < -camera->clamp_y) camera->pitch = -camera->clamp_y;

    if (camera->yaw > camera->clamp_x) camera->yaw = camera->clamp_x;
    if (camera->yaw < -camera->clamp_x) camera->yaw = -camera->clamp_x;

    camera->cam3D.position.x = camera->cam3D.target.x + distance * sinf(camera->yaw) * cosf(camera->pitch);
    camera->cam3D.position.y = camera->cam3D.target.y + distance * sinf(camera->pitch);
    camera->cam3D.position.z = camera->cam3D.target.z + distance * cosf(camera->yaw) * cosf(camera->pitch);

    camera->cam3D.up.x = -sinf(camera->pitch) * sinf(camera->yaw);
    camera->cam3D.up.y = cosf(camera->pitch);
    camera->cam3D.up.z = -sinf(camera->pitch) * cosf(camera->yaw);

    camera->x_axis = getCameraDirectionX(camera);
}

//code to move camera around the world
void MoveCameraPos(custom_cam3d *c){
    Vector2 delta_mouse = GetMouseDelta();
    //play around with this constant later
    delta_mouse.y = delta_mouse.y  *GetFrameTime() / 3.0f;

    //scale the up position by delta_mouse.y and then add it to both the position
    //and target to move the camera while keeping it looking in the same direction
    Vector3 change_pos_y = Vector3Scale(c->cam3D.up, delta_mouse.y);
    c->cam3D.position = Vector3Add(c->cam3D.position,change_pos_y);
    c->cam3D.target = Vector3Add(c->cam3D.target, change_pos_y);

    Vector3 camera_x_axis_vector = getCameraDirectionX(c);
    c->x_axis = camera_x_axis_vector;
    float delta_x = -delta_mouse.x * GetFrameTime() / 3.0f; //negative needed to move correct direction
    
    Vector3 change_pos_x = Vector3Scale(camera_x_axis_vector, delta_x);
    c->cam3D.position = Vector3Add(c->cam3D.position, change_pos_x);
    c->cam3D.target = Vector3Add(c->cam3D.target, change_pos_x);
}

//get the vector direction coresponding to the screen's x axis
Vector3 getCameraDirectionX(custom_cam3d *c){
    Vector3 screen_direction_y = c->cam3D.up;
    screen_direction_y = Vector3Normalize(screen_direction_y);
    //get the z direction vector which is into the screen, this is not world space
    //remember that we are trying to convert 2D x and y movement to 3d space, so the "z"
    //direction is always into the screen, but depending on camera orientation
    //x y and z of "screen space" probably != to world space counterparts
    Vector3 screen_direction_z = Vector3Subtract(c->cam3D.position, c->cam3D.target);
    screen_direction_z = Vector3Normalize(screen_direction_z);

    Vector3 direction_x = Vector3CrossProduct(screen_direction_y, screen_direction_z);
    direction_x = Vector3Normalize(direction_x);
    
    return direction_x;
}

//allows camera to zoom, Clamp values defined in this function
void zoomCamera(custom_cam3d * camera, float zoom){
    Vector3 forward =  Vector3Subtract(camera->cam3D.position, camera->cam3D.target);
    float distance = Vector3Length(forward);

    distance += zoom;
    distance = Clamp(distance, 2.0f, camera->max_zoom);

    Vector3 new_camera_pos = Vector3Add(camera->cam3D.target, Vector3Scale(Vector3Normalize(forward), distance));
    camera->cam3D.position = new_camera_pos;
}

//resets the camera position to target, and resets pitch and yaw to 0
void resetCamera(custom_cam3d * camera, Vector3 target){
    setCameraPosition(camera, target);
    camera->pitch = 0.0f;
    camera->yaw = 0.0f; 
}
