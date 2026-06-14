#include "player.h"

const float friction = 10.0f;
const float Acceleration = 5.0f;
const float max_speed = 5.0f;
const float jumpAccel = 10.0f;
const float gravity = 5.0f;

const Vector3 VectorUp = {0.0f, 1.0f, 0.0f};
const Vector3 VectorDown = {0.0f, -1.0f, 0.0f};
const Vector3 VectorLeft = {1.0f, 0.0f, 0.0f};
const Vector3 VectorRight = {-1.0f, 0.0f, 0.0f};
const Vector3 VectorForward = {0.0f, 0.0f, -1.0f};
const Vector3 VectorBack = {0.0f, 0.0f, 1.0f};

typedef struct {
    Vector3 start; // Start point of the capsule line
    Vector3 end;   // End point of the capsule line
    float radius;  // Radius of the capsule
} Capsule;

//initialize player stats
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

//get the center and bounding box of the mesh
void getMeshCenter(player *p){
    p->bounding_box = GetMeshBoundingBox(p->p_model.meshes[0]);
    //adjust boundingbox for spawn position
    p->bounding_box.max = Vector3Add(p->bounding_box.max, p->p_position);
    p->bounding_box.min = Vector3Add(p->bounding_box.min, p->p_position);

    p->mesh_center = Vector3Scale(Vector3Add(p->bounding_box.min, p->bounding_box.max), 0.5f);
    //p->mesh_center = Vector3Add(center, p->p_position);
}

//code to get the boundingbox to move with the player
void moveBoundBoxWithPlayer(player *p, Vector3 delta_pos){
    p->bounding_box.max = Vector3Add(p->bounding_box.max, delta_pos);
    p->bounding_box.min = Vector3Add(p->bounding_box.min, delta_pos);
}

//I need a function that moves the player, camera, boundingbox, etc
void changePlayerPosition(player *p, Vector3 delta){
    float distance = Vector3Distance(p->p_camera_3rd_person.cam3D.position, p->p_position);

    p->p_position = Vector3Add(p->p_position, delta);    
    moveBoundBoxWithPlayer(p, delta);

    p->p_camera_3rd_person.cam3D.target = p->p_position;
    p->p_camera_3rd_person.cam3D.position = Vector3Add(p->p_camera_3rd_person.cam3D.position, delta);
}

//code to make the mesh player turn towards movement direction
void playerLookDir(player *p, Vector3 v){
    float angle = atan2f(v.x, v.z); //whether needs to be negated, depends on Draw function used
    p->p_model.transform = MatrixRotateY(angle);
    p->look_dir = Vector3Normalize(v);
}

//movement code
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

    //attempt to code jumping and fall
    //currently allows player to fall through the floor
    if(IsKeyPressed(KEY_SPACE) && p->is_grounded){
        p->p_velocity.y += jumpAccel / gravity;   
        p->is_grounded = false;     
    }

    else if(p->bounding_box.min.y <= 0 && !p->is_grounded) {
        p->p_velocity.y = 0;
        float delta_below_floor = 0.0f - p->bounding_box.min.y;
        changePlayerPosition(p, (Vector3){0, delta_below_floor, 0});
        p->is_grounded = true;
    }

    else if(p->is_grounded == false){
        p->p_velocity.y -= dt * gravity;
    }

    changePlayerPosition(p, p->p_velocity);
    
}

//rotate camera around the player controls
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

//reset player orientation facing the -Z position
void resetPlayerRotation(player *p){
    p->p_angleY = 0.0f;
    p->p_model.transform = MatrixIdentity();
    p->look_dir = (Vector3){0.0f, 0.0f, -1.0f};
}

//use number pad to rotate around the player, likely only use is for edit mode style
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

//unload model and items related to the player from GPU
void DestroyPlayer(player *p){
    UnloadModel(p->p_model);
}
