#ifndef PLAYER_H
#define PLAYER_H

#include <stdio.h>
#include <stdlib.h>

#include "raylib.h"
#include "camera.h"

typedef struct player {
    Model p_model;
    Vector3 p_position;
    BoundingBox bounding_box;
    Vector3 mesh_center;
    custom_cam3d p_camera_3rd_person;
    float p_angleY;
    Vector3 p_velocity;
    Vector3 look_dir;
    bool is_grounded;
} player;

player initPlayer(const char * model_path, Vector3 position);
void getMeshCenter(player *p);
void playerLookDir(player *p, Vector3 v);
void movePlayerVectors(player *p);
void playerCamera3rdPersonControls(player *p);
void resetPlayerRotation(player *p);
void LockCamera(player *p);
void DestroyPlayer(player *p);

#endif