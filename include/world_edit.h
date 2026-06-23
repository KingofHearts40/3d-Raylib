#ifndef WORLD_EDIT_H
#define WORLD_EDIT_H

#include "raylib.h"
#include "raymath.h"
#include "camera.h"
#include <stdio.h>

typedef struct gameObject gameObject;

// typedef struct gameObject{
//     int model_num;
//     BoundingBox bbox;
//     Vector3 pos;
//     Color bbox_color;
//     bool bbox_draw;
//     int anim_count;
//     Model model;
//     ModelAnimation *animations;
// }gameObject;

struct floor_tile {
    Vector3 pos;
    Model model;
    Color color;
    BoundingBox bounding_box;
};


//raycast related functions

Ray getRayBetweenPoints(Vector3 start, Vector3 end);
Ray cameraToMouseRay(Camera3D c);
bool isPositionInRange(Ray r, Vector3 target, float distance);
Ray getPerpendicularRay(Camera3D *c, gameObject *g);
Ray getRayPerpendicularToActiveObject(Camera3D *c);

//gameObject related functions

gameObject initGameObject(Model *m);
void updateGameObjectPos(gameObject *g, Vector3 new_pos);
void MouseMoveSelectedGameObj(custom_cam3d *c);

//animation related functions
void playActiveObjectAnim(int *current_frame, int *animation_num);
char * activeAnimationName(int * anim_num);

//select and deselect objects

void selectGameObject(Camera3D c);
void deselectGameObject();

//import functions

void storeGLBasGameObject();
//drawing and GPU functions

void drawGameObjects();
void unloadGameObjects();

//original testing functiuons for proof of concept
void createTileMap();
void drawTileMap();
int getClosestTileIndexCollision(Ray *ray);
void change_tile_color_if_clicked(Ray *ray);
void highlightCurrentTile(Ray *ray);
void selectTile(Ray *ray);
void moveSelectedTile();
void drawFloorGridMap();
void storeDropGLB();
void displaySavedModels();


#endif