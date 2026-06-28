#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include "world_edit.h"
#include "global_constants.h"

typedef struct gameObject{
    int model_num;
    BoundingBox bbox;
    Vector3 pos;
    Color bbox_color;
    bool bbox_draw;
    int anim_count;
    Model model;
    ModelAnimation *animations;
}gameObject;


Model model_list[400];
int current_model = 0;
int max_models = 400;

#define MAXGAMEOBJECTS 400
const int max_game_obj = MAXGAMEOBJECTS;
gameObject game_object_storage[MAXGAMEOBJECTS];
int current_num_game_obj = 0;
gameObject *active_game_object = NULL;

#undef MAXGAMEOBJECTS

//code needed for the floor tile experiment
struct floor_tile floor_test[400];
int active_tile = -1;

//first attempt at serializing my game_objects
void write_game_obj_file(){

}

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

//initializes game object to be in center of the world, get a boundingbox for it
//intializes color of the bbox to be black and by default draws the bbox
gameObject initGameObject(Model *m){
    gameObject g;
    g.model = *m;
    g.pos =(Vector3){0,0,0};
    g.bbox = GetModelBoundingBox(g.model);
    g.bbox_color = BLACK;
    g.bbox_draw = true;

    return g;
}

//get a ray perpendicular to ray from camera to gameObjects
Ray getPerpendicularRay(Camera3D *c, gameObject *g){
    Ray r = getRayBetweenPoints(c->position, g->pos);

    Vector3 perpendicularDir = Vector3CrossProduct(r.direction, VectorUp);
    perpendicularDir = Vector3Normalize(perpendicularDir);

    Ray perpRay;
    perpRay.position = g->pos;
    perpRay.direction = perpendicularDir;

    return perpRay;
}

//get Ray perpendicular from the camera to the selected object
Ray getRayPerpendicularToActiveObject(Camera3D *c){
    if(active_game_object){
        Ray r = getPerpendicularRay(c, active_game_object);
        return r;
    }
}

//updates gameObjects model and bbox positions to provided Vector3
void updateGameObjectPos(gameObject *g, Vector3 new_pos){
    Vector3 last_pos = g->pos;
    
    //update player position to the provided Vec3
    g->pos = new_pos;

    //get the change in player position an add it to the boundingBox
    Vector3 delta_pos = Vector3Subtract(new_pos, last_pos);
    g->bbox.max = Vector3Add(g->bbox.max, delta_pos);
    g->bbox.min = Vector3Add(g->bbox.min, delta_pos);
}

//draws all the objects in the game_object_storage array and their boundingboxes
void drawGameObjects(){
    for(int i = 0; i < current_num_game_obj; i++){
        DrawModel(game_object_storage[i].model, game_object_storage[i].pos, 1.0f, WHITE);

        if(game_object_storage[i].bbox_draw){
            DrawBoundingBox(game_object_storage[i].bbox, game_object_storage[i].bbox_color);
        }
    }
}

void unloadGameObjects(){
    for(int i = 0; i < current_num_game_obj; i++){
        UnloadModel(game_object_storage[i].model);
    }    
}

void MouseMoveSelectedGameObj(custom_cam3d *c){
    if(!active_game_object) return;

    float screenHeight = GetScreenHeight();

    float distance = Vector3Distance(c->cam3D.position, active_game_object->pos);

    //math to convert pixels on screen to world space at the disance the active ob is from camera
    float world_height_at_distance = 2.0f * distance * tanf(c->cam3D.fovy*DEG2RAD/2.0f);
    float pixel_to_world_scale = world_height_at_distance / screenHeight;

    Vector2 delta_mouse = GetMouseDelta();
    float delta_x = delta_mouse.x * pixel_to_world_scale;
    float delta_y = delta_mouse.y * pixel_to_world_scale;

    //change in float mouse position conversion to a delta Vector3
    Vector3 change_pos_x = Vector3Scale(c->x_axis, delta_x);
    Vector3 change_pos_y = Vector3Scale(c->cam3D.up, delta_y);

    Vector3 new_pos;

    new_pos = Vector3Add(active_game_object->pos, change_pos_x);
    new_pos = Vector3Subtract(new_pos, change_pos_y);
    updateGameObjectPos(active_game_object, new_pos);
}


//select the closest gameObject to the camera
void selectGameObject(Camera3D c){
    //basic flow: create a ray from camera to mouseclick and Raycollision
    //deselect previous gameobject since the mousebutton was clicked
    //check for a hit with all objects in the game_object_storage
    //if hit and nothing previously selected store in the active_game_object ptr
    //otherwise pick the closest object to the camera
    //change bbox color to red to make it clear which object is selected

    Ray r = cameraToMouseRay(c);
    RayCollision rcol;

    deselectGameObject();

    for(int i = 0; i <current_num_game_obj; i++){
        //check for collision between ray and object
        rcol = GetRayCollisionBox(r, game_object_storage[i].bbox);

        if(rcol.hit){
            //if pointer is NULL we store this object in the active_game_object pointer
            //otherwise we need to compare if current object is closer than what the pointer
            //already has
            if(active_game_object == NULL) active_game_object = &game_object_storage[i];

            else{
                if(rcol.distance < Vector3Distance(r.position, active_game_object->pos)){
                    active_game_object->bbox_color = BLACK;
                    active_game_object = &game_object_storage[i];
                }
            }
        }
    }

    if(active_game_object){
        active_game_object->bbox_color = RED;
        }
}

//deselects the previously active game object
void deselectGameObject(){
    if(active_game_object){
        active_game_object->bbox_color = BLACK; //default color
        active_game_object = NULL;
    }
}

//gets a dropped GLB and stores it in the array for gameobjects
//currently set to only work with 1 file at a time
void storeGLBasGameObject(){
    if(IsFileDropped()){
        FilePathList droppedFile = LoadDroppedFiles();
        
        //exit function if not the right file type
        if(!IsFileExtension(droppedFile.paths[0], ".glb")){
            printf("dropped file %s is not a .glb", droppedFile.paths[0]);
            UnloadDroppedFiles(droppedFile);            
            return;
        }

        //if the array is maxed out, avoids array overflow
        if(current_num_game_obj >= max_game_obj){
            printf("No storage room left for new models");
            return;
        }

        if(droppedFile.count == 1){
            Model m = LoadModel(droppedFile.paths[0]);
            gameObject g = initGameObject(&m);
            g.animations = LoadModelAnimations(droppedFile.paths[0], &g.anim_count);

            
            game_object_storage[current_num_game_obj] = g;
            current_num_game_obj++;
        }

        UnloadDroppedFiles(droppedFile);
    }
}

void playActiveObjectAnim(int *current_frame, int *animation_num){
    if (active_game_object == NULL || active_game_object->anim_count == 0) return;

    if ((*animation_num) >= active_game_object->anim_count) (*animation_num) = 0;
    int anim = *animation_num;
    int max_frame_count = active_game_object->animations[anim].keyframeCount;
    
    UpdateModelAnimation(active_game_object->model, active_game_object->animations[anim], *current_frame);
    (*current_frame)++;

    if ((*current_frame) >= max_frame_count) (*current_frame) = 0; //reset the indx if reached final anim
}

char * activeAnimationName(int * anim_num){
    if(!active_game_object) return "NULL";
    if ((*anim_num) >= active_game_object->anim_count) (*anim_num) = 0;

    char * name = active_game_object->animations[*anim_num].name;

    return name;
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



