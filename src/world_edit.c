#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include "world_edit.h"

struct floor_tile {
    Vector3 pos;
    Model model;
    Color color;
    BoundingBox bounding_box;
};

Model model_list[400];
int current_model = 0;
int max_models = 400;

//code needed for the floor tile experiment
struct floor_tile floor_test[400];
int active_tile = -1;

//allow a ray to be cast between any two points
Ray getRayBetweenPoints(Vector3 start, Vector3 end){
    Ray r;
    r.position = start;
    r.direction = Vector3Normalize(Vector3Subtract(end, start));

    return r;
}

//get a ray from provided camera to mouse click on screen
Ray cameraToMouseRay(Camera3D c){
    Vector2 mouse_pos = GetMousePosition();
    Ray r = GetMouseRay(mouse_pos, c);
    r.direction = Vector3Normalize(r.direction);

    return r;
}

//takes a Ray, position, and distance, and determine if the position is within the distance
//of the beginning position of the Ray
bool isPositionInRange(Ray r, Vector3 target, float distance){
   bool inRange = (distance > Vector3Distance(r.position, target)) ? true : false;
   return inRange;
}

//function to store a single drag and Dropped GLB file onto the program
//requires 3 variables exist, the array, a placeholder for current open spot
//and an int for max array positions
void storeDropGLB(){
    if(IsFileDropped()){
        FilePathList droppedFile = LoadDroppedFiles();
        if(droppedFile.count == 1 && IsFileExtension(droppedFile.paths[0], ".glb") && current_model < max_models){
            model_list[current_model] = LoadModel(droppedFile.paths[0]);
            current_model++;
        }

        UnloadDroppedFiles(droppedFile);
    }
}

//function that displays the models that were dragged and dropped
//to make useable need to plan to make a window pane for the selectable models
//and another window for the world to build. Then display models as thumbnail
void displaySavedModels(){
    if(current_model == 0) return; //no models in the array

    for(int i = 0; i < current_model; i++){
        DrawModel(model_list[i], (Vector3){-10 + i*2, 2, 0},1, WHITE);
    }
}

//code to create a floor TileMap
void createTileMap(){

    Mesh cube = GenMeshCube(1.0f, 1.0f, 1.0f);
    Model cube_model = LoadModelFromMesh(cube);

    for(int x = 0; x < 20; x++){
        for (int z = 0; z < 20; z++){
            Vector3 pos;
            pos.x = -10.0f + x;
            pos.y = 0;
            pos.z = -10.0f + z;
        
            int current_tile = z*20 + x;
            floor_test[current_tile].pos = pos;
            floor_test[current_tile].model = cube_model;   
            floor_test[current_tile].color = RED;

            floor_test[current_tile].bounding_box = GetModelBoundingBox(cube_model);
            floor_test[current_tile].bounding_box.min = Vector3Add(floor_test[current_tile].bounding_box.min, pos);
            floor_test[current_tile].bounding_box.max = Vector3Add(floor_test[current_tile].bounding_box.max, pos);
        }
    }
}

//code for drawing the TileMap
void drawTileMap(){
    for(int x = 0; x < 20; x++){
        for (int z = 0; z < 20; z++){
            int current_tile = z*20 + x;

            DrawModel(floor_test[current_tile].model, floor_test[current_tile].pos, 1, floor_test[current_tile].color);
            DrawBoundingBox(floor_test[current_tile].bounding_box, GREEN);
        }
    }
}

//code for picking the closestTile to raycast
int getClosestTileIndexCollision(Ray *ray){
    
    float closest_dist = INFINITY;
    int nearest_tile_index = -1;

    for(int x = 0; x < 20; x++){
        for (int z = 0; z < 20; z++){
            int current_tile = z*20 + x;

            RayCollision collision = GetRayCollisionBox(*ray, floor_test[current_tile].bounding_box);

            if(collision.hit && collision.distance < closest_dist){
                closest_dist = collision.distance;
                nearest_tile_index = current_tile;
            }
        }
    }

    return nearest_tile_index;
}

//changes color of the tile
void change_tile_color_if_clicked(Ray *ray){
    int nearest_tile_index = getClosestTileIndexCollision(ray);  

    if(nearest_tile_index >= 0){
        floor_test[nearest_tile_index].color = BLUE;
    }
}

//highlight active tile
void highlightCurrentTile(Ray *ray){

    float closest_dist = INFINITY;
    int nearest_tile_index = -1;

    for(int x = 0; x < 20; x++){
        for (int z = 0; z < 20; z++){
            int current_tile = z*20 + x;

            RayCollision collision = GetRayCollisionBox(*ray, floor_test[current_tile].bounding_box);

            if(collision.hit && collision.distance < closest_dist){
                closest_dist = collision.distance;
                nearest_tile_index = current_tile;
            }

            floor_test[current_tile].color = RED;
        }
    }

    if(nearest_tile_index > -1){
        floor_test[nearest_tile_index].color = BLACK;
    }
}

void selectTile(Ray *ray){
    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
        active_tile = getClosestTileIndexCollision(ray);
    }
}

void moveSelectedTile(){

    if (active_tile < 0) return;

    if(IsKeyPressed(KEY_UP)) floor_test[active_tile].pos.y+= 1;
    if(IsKeyPressed(KEY_DOWN)) floor_test[active_tile].pos.y-= 1;

    if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
       Vector2 delta = GetMouseDelta();
    }

    floor_test[active_tile].color = BLACK;
    floor_test[active_tile].bounding_box = GetModelBoundingBox(floor_test[active_tile].model);
    floor_test[active_tile].bounding_box.min = Vector3Add(floor_test[active_tile].bounding_box.min, floor_test[active_tile].pos);
    floor_test[active_tile].bounding_box.max = Vector3Add(floor_test[active_tile].bounding_box.max, floor_test[active_tile].pos);

}

void drawFloorGridMap(){
    for(int x = 0; x < 20; x++){
        for (int z = 0; z < 20; z++){
            Vector3 pos;
            pos.x = -10.0f + x;
            pos.y = 0;
            pos.z = -10.0f + z;

            DrawCubeWiresV(pos, (Vector3){1.0f, 0.1f, 1.0f}, RED);
        }            
    }
}



