#include "player.h"

const float friction = 10.0f;
const float Acceleration = 5.0f;
const float max_speed = 5.0f;

const Vector3 VectorUp = {0.0f, 1.0f, 0.0f};
const Vector3 VectorDown = {0.0f, -1.0f, 0.0f};
const Vector3 VectorLeft = {1.0f, 0.0f, 0.0f};
const Vector3 VectorRight = {-1.0f, 0.0f, 0.0f};
const Vector3 VectorForward = {0.0f, 0.0f, -1.0f};
const Vector3 VectorBack = {0.0f, 0.0f, 1.0f};

player initPlayer(const char * model_path, Vector3 position){
    player p;

    p.p_model = LoadModel(model_path);
    if(p.p_model.meshCount < 1){
        printf("Error loading player model from path: %s\n", model_path);
        exit(1);
    }
    p.p_position = position;
    p.p_angleY = 0.0f;
    p.p_velocity = (Vector3){0.0f, 0.0f, 0.0f};
    p.look_dir = VectorForward; //starting look direction is -z
    //getMeshCenter must be called after the p_position is set, will crash if called before
    getMeshCenter(&p);

    p.p_camera_3rd_person = Init3dCamera();
    setCameraTarget(&p.p_camera_3rd_person, p.p_position);
    setCameraPosition(&p.p_camera_3rd_person, 
        (Vector3){p.p_position.x, p.p_position.y, p.p_position.z + CAMERA_DISTANCE_INIT});

    return p;
}

void getMeshCenter(player *p){
    p->bounding_box = GetMeshBoundingBox(p->p_model.meshes[0]);
    //adjust boundingbox for spawn position
    p->bounding_box.max = Vector3Add(p->bounding_box.max, p->p_position);
    p->bounding_box.min = Vector3Add(p->bounding_box.min, p->p_position);

    p->mesh_center = Vector3Scale(Vector3Add(p->bounding_box.min, p->bounding_box.max), 0.5f);
    //p->mesh_center = Vector3Add(center, p->p_position);
}

void playerLookDir(player *p, Vector3 v){
    float angle = atan2f(v.x, v.z); //whether needs to be negated, depends on Draw function used
    p->p_model.transform = MatrixRotateY(angle);
    p->look_dir = Vector3Normalize(v);
}

//test function to move player using Vector Math
void movePlayerVectors(player *p){
    //get dt to normalize to frame rate
    float dt = GetFrameTime();   

    //z axis
    if(IsKeyDown(KEY_W)){
        p->p_velocity.z -= dt * Acceleration / friction;
        if(fabs(p->p_velocity.z) > max_speed) p->p_velocity.z = -max_speed;
        playerLookDir(p, p->p_velocity);         
    }

    else if(IsKeyDown(KEY_S)){
        p->p_velocity.z += dt * Acceleration / friction;
        if(fabs(p->p_velocity.z) > max_speed) p->p_velocity.z = max_speed; 
        playerLookDir(p, p->p_velocity);
    }

    else{
        p->p_velocity.z /= (1.0f + friction * dt);
        if(fabs(p->p_velocity.z) < 0.01f) p->p_velocity.z = 0.0f;
    }

    //x axis
    if(IsKeyDown(KEY_A)){
        p->p_velocity.x -= dt * Acceleration / friction;  
        if(fabs(p->p_velocity.x) > max_speed) p->p_velocity.x = -max_speed;
        playerLookDir(p, p->p_velocity);       
    }

    else if(IsKeyDown(KEY_D)){
        p->p_velocity.x += dt * Acceleration / friction;
        if(fabs(p->p_velocity.x) > max_speed) p->p_velocity.x = max_speed; 
        playerLookDir(p, p->p_velocity);   
    }

    else{
        p->p_velocity.x /= (1.0f + friction * dt);
        if(fabs(p->p_velocity.x) < 0.01f) p->p_velocity.x = 0.0f;
    }

    p->p_position = Vector3Add(p->p_position, p->p_velocity);
    p->p_camera_3rd_person.cam3D.position = Vector3Add(p->p_camera_3rd_person.cam3D.position, p->p_velocity);
    p->p_camera_3rd_person.cam3D.target = p->p_position;
}

void playerCamera3rdPersonControls(player *p){

    if(IsMouseButtonDown(MOUSE_BUTTON_RIGHT)){
        rotateCameraAroundCurrentTarget(&p->p_camera_3rd_person);
    }

    if(IsKeyPressed(KEY_R)){
        //reset all the camera parameters, and put it behind the player
        Vector3 reset_cam_3rd_per_pos = p->p_position;
        reset_cam_3rd_per_pos.z += CAMERA_DISTANCE_INIT;
        resetCamera(&p->p_camera_3rd_person, reset_cam_3rd_per_pos);
    }

    if(GetMouseWheelMove()){
        float zoom = GetMouseWheelMove() * 0.4;
        zoomCamera(&p->p_camera_3rd_person, zoom);
    }
}

void resetPlayerRotation(player *p){
    p->p_angleY = 0.0f;
    p->p_model.transform = MatrixIdentity();
    p->look_dir = (Vector3){0.0f, 0.0f, -1.0f};
}

void LockCamera(player *p){

    if(IsKeyDown(KEY_KP_1)){
        p->p_camera_3rd_person.cam3D.position = p->p_position;
        p->p_camera_3rd_person.cam3D.position.z += 10;    
    }

    if(IsKeyDown(KEY_KP_3)){
        p->p_camera_3rd_person.cam3D.position = p->p_position;
        p->p_camera_3rd_person.cam3D.position.x += 10;    
    }

    if(IsKeyDown(KEY_KP_7)){
        p->p_camera_3rd_person.cam3D.position = p->p_position;
        p->p_camera_3rd_person.cam3D.position.y += 10;
        p->p_camera_3rd_person.cam3D.position.z += 0;
        p->p_camera_3rd_person.cam3D.target = p->p_position;    
    }


}

void DestroyPlayer(player *p){
    UnloadModel(p->p_model);
}
