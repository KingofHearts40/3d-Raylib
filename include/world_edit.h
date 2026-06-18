#ifndef WORLD_EDIT_H
#define WORLD_EDIT_H

#include "raylib.h"
#include "raymath.h"
#include <stdio.h>

struct floor_tile {
    Vector3 pos;
    Model model;
    Color color;
    BoundingBox bounding_box;
};

Ray getRayBetweenPoints(Vector3 start, Vector3 end);
Ray cameraToMouseRay(Camera3D c);
bool isPositionInRange(Ray r, Vector3 target, float distance);
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