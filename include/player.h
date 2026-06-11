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

typedef struct player {
    Model p_model;
    Vector3 p_position;
    custom_cam3d p_camera_3rd_person;

} player;

player InitPlayer(const char* model_path, Vector3 position){
    player p;
    p.p_model = LoadModel(model_path);
    if(p.p_model.meshCount < 1){
        printf("Error loading player model from path: %s\n", model_path);
        exit(1);
    }
    p.p_position = position;

    p.p_camera_3rd_person = Init3dCamera();
    setCameraTarget(&p.p_camera_3rd_person, p.p_position);
    setCameraPosition(&p.p_camera_3rd_person, 
        (Vector3){p.p_position.x, p.p_position.y, p.p_position.z + CAMERA_DISTANCE_INIT});

    return p;
}

void MovePlayer(player *p){
    
    if (IsKeyDown(foward_key_1)){
            p->p_position.z += -0.1f;
            p->p_camera_3rd_person.cam3D.position.z += -0.1f;
            p->p_camera_3rd_person.cam3D.target = p->p_position;
        }

        if (IsKeyDown(backward_key_1)){
            p->p_position.z += +0.1f;
            p->p_camera_3rd_person.cam3D.position.z += +0.1f;
            p->p_camera_3rd_person.cam3D.target = p->p_position;
        }

        if(IsKeyDown(left_key_1)){
            p->p_position.x -= 0.1f;
            p->p_camera_3rd_person.cam3D.position.x -= 0.1f;
            p->p_camera_3rd_person.cam3D.target = p->p_position;
        }

        if(IsKeyDown(right_key_1)){
            p->p_position.x += 0.1f;
            p->p_camera_3rd_person.cam3D.position.x += 0.1f;
            p->p_camera_3rd_person.cam3D.target = p->p_position;
        }
}

void PlayerCamera3rdPersonControls(player *p){

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

void DestroyPlayer(player *p){
    UnloadModel(p->p_model);
}

#endif