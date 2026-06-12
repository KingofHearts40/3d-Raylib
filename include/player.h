#ifndef PLAYER_H
#define PLAYER_H

#include <stdio.h>
#include <stdlib.h>

#include "raylib.h"
#include "camera.h"

KeyboardKey foward_key_1 = KEY_W;
KeyboardKey backward_key_1 = KEY_S;
KeyboardKey left_key_1 = KEY_A;
KeyboardKey right_key_1 = KEY_D;

const float friction = 10.0f;
const float Acceleration = 5.0f;
const float max_speed = 10.0f;

typedef struct player {
    Model p_model;
    Vector3 p_position;
    custom_cam3d p_camera_3rd_person;
    float p_angleY;
    Vector3 p_velocity;

} player;

player initPlayer(const char* model_path, Vector3 position){
    player p;
    p.p_model = LoadModel(model_path);
    if(p.p_model.meshCount < 1){
        printf("Error loading player model from path: %s\n", model_path);
        exit(1);
    }
    p.p_position = position;
    p.p_angleY = 0.0f;
    p.p_velocity = (Vector3){0.0f, 0.0f, 0.0f};

    p.p_camera_3rd_person = Init3dCamera();
    setCameraTarget(&p.p_camera_3rd_person, p.p_position);
    setCameraPosition(&p.p_camera_3rd_person, 
        (Vector3){p.p_position.x, p.p_position.y, p.p_position.z + CAMERA_DISTANCE_INIT});

    return p;
}

void movePlayer(player *p){

    float dt = GetFrameTime();
    
    if (IsKeyDown(foward_key_1)){
        p->p_velocity.z += -Acceleration * dt / friction;
        if(p->p_velocity.z < -max_speed){
            p->p_velocity.z = -max_speed;
        }
    }

    else if (IsKeyDown(backward_key_1)){
        p->p_velocity.z += Acceleration * dt / friction;
        if(p->p_velocity.z > max_speed){
            p->p_velocity.z = max_speed;
        }
    }

    else{
        p->p_velocity.z /= (1.0f + friction * dt);

        //set velocity.z to 0 if it's close enough
        if(fabs(p->p_velocity.z) < 0.01f){
            p->p_velocity.z = 0.0f;
        }
    }

    if(IsKeyDown(left_key_1)){
        p->p_velocity.x = -Acceleration * dt / friction;
    }

    else if(IsKeyDown(right_key_1)){
        p->p_velocity.x = Acceleration * dt / friction;
    }
    
    else{
        p->p_velocity.x = 0.0f;
    }

        p->p_position = Vector3Add(p->p_position, p->p_velocity);
        p->p_camera_3rd_person.cam3D.position =Vector3Add(p->p_camera_3rd_person.cam3D.position, p->p_velocity);
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

void rotatePlayerModelY(player *p, float angle){
    p->p_angleY += angle * DEG2RAD;
    p->p_model.transform = MatrixRotateY(p->p_angleY);
}

void resetPlayerRotation(player *p){
    p->p_angleY = 0.0f;
    p->p_model.transform = MatrixIdentity();
}

void DestroyPlayer(player *p){
    UnloadModel(p->p_model);
}

#endif