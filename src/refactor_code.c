#include "main_function_entry.h"
#include "raylib.h"
#include "raymath.h"
#include "camera.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define THUMB_SLOT_WIDTH 100
#define THUMB_SLOT_HEIGHT 100
#define THUMB_SLOT_SPACING 110

typedef struct Object{
    int id;
    int animation_num;
    Model model;
    ModelAnimation *animations;
    RenderTexture2D thumbnail;
    Rectangle bounds;
    Vector2 home_pos;
    float scale_fac;
    Vector3 center_offset;
    Color thumbnail_border_color;
}Object;

//array to hold model path
char *model_path[500];
Object object_array[500];
int loaded_models = 0;
Object *selected_thumbnail = NULL;

//function declarations:
void getDroppedGLBFilePath();
void storeObjectDataInArray(char * model_file, int id);
void createThumbNailData(Object *o);
void drawThumbNails();
void scrollThumbNails(float delta);
void draw3DViewPort(RenderTexture2D view_port, custom_cam3d *camera, int screen_width, int screen_height);
void printFilePathToScreen();
void draw2dUIForThumbnails(int screen_height_ui);
bool getMouseCollisionRec2DScreen(int screen_height_ui);
bool getMouseCollisionRec3Dscreen(int screen_height_3d);
void pickThumbNail(int screen_height_ui);
void freeModelPathArray();
void freeObjectData();

//actual functions

//stores the GLBFilePath into the model_path array
void getDroppedGLBFilePath(){

    FilePathList files;
    if(IsFileDropped()){
        files = LoadDroppedFiles();
    }

    for (int i = 0; i < files.count; i++) {

        if(!IsFileDropped()) return;

        if(!IsFileExtension(files.paths[i], ".glb")){
            printf("Not a GLB file");
            continue;
        };

        bool isDuplicate = false;

        // Check against existing files in your own array/list
        for (int j = 0; j < loaded_models; j++) {
            if (strcmp(files.paths[i], model_path[j]) == 0) {
                printf("Duplicate found");
                isDuplicate = true;
                break;
            }
        }

        if (!isDuplicate) {
            model_path[loaded_models] = malloc(512);
            strcpy(model_path[loaded_models], files.paths[i]);
            storeObjectDataInArray(model_path[loaded_models], loaded_models);
            loaded_models++;
        }
    }

    UnloadDroppedFiles(files);
}

//stores model_data into object_array
void storeObjectDataInArray(char * model_file, int id){
    Object o;
    o.id = id;
    o.model = LoadModel(model_file);
    o.animations = LoadModelAnimations(model_file, &o.animation_num);

    if(o.animations == NULL){
        o.animation_num = 0; //needed to avoid wrap-around bug where this is assigned 32767        
    }

    createThumbNailData(&o);
    object_array[id] = o;
}

//function to create a thumbnail of the model
void createThumbNailData(Object *o){
    BoundingBox model_box = GetModelBoundingBox(o->model);
    float width = model_box.max.x - model_box.min.x;
    float height = model_box.max.y - model_box.min.y;
    float depth = model_box.max.z - model_box.min.z;
    //find the largest dimension, first check width vs height, then result against depth
    float w_v_h = (width > height) ? width : height;
    float largest_dim = (w_v_h > depth) ? w_v_h : depth;
    if (largest_dim == 0.0f) largest_dim = 1.0f; 
    
    //create the scale and center offset to position the model center of the thumbnaile based on
    //how DrawModelEx works
    float scale_correction = 1.0f/largest_dim;
    Vector3 center = {model_box.min.x + width/2.0f, model_box.min.y + height/2.0f, 
        model_box.min.z + depth / 2.0f};
    Vector3 center_offset = {-center.x *scale_correction, -center.y *scale_correction + 0.5, 
        -center.z *scale_correction};

    //2D off screen texture and camera setup to draw to
    RenderTexture2D thumbTarget = LoadRenderTexture((int)THUMB_SLOT_WIDTH, (int)THUMB_SLOT_HEIGHT);
    Camera3D thumbCamera;
    thumbCamera.position = (Vector3){0.0f, 0.5f, 2.0f };
    thumbCamera.target = (Vector3){0.0f, 0.5f, 0.0f };
    thumbCamera.up = (Vector3){0.0f, 1.0f, 0.0f};
    thumbCamera.fovy = 45.0f;
    thumbCamera.projection = CAMERA_PERSPECTIVE;

    //Render it to the RenderTexture2D
    BeginTextureMode(thumbTarget);
        ClearBackground(BLANK); 
        BeginMode3D(thumbCamera);
            DrawModelEx(o->model, center_offset, (Vector3){0.0f, 1.0f, 0.0f}, 0.0f, (Vector3){scale_correction, scale_correction, scale_correction}, WHITE);
        EndMode3D();
    EndTextureMode();

    //store the needed data to draw later, need the spacing macro to add 10 pixels between
    float slotX = 20 + THUMB_SLOT_SPACING * o->id;
    float slotY = 640 + THUMB_SLOT_HEIGHT;
    if(o->id != 0){
        //this code is so that if you scroll the thumbnails, can still place the next one correctly
        int last_id = o->id - 1;
        slotX = object_array[last_id].home_pos.x + THUMB_SLOT_SPACING;        
    }
    //store the rendered thumbnail and position
    o->thumbnail = thumbTarget;
    o->bounds = (Rectangle) {slotX, slotY, THUMB_SLOT_WIDTH, THUMB_SLOT_HEIGHT};
    o->center_offset = center_offset;
    o->home_pos = (Vector2){slotX, slotY};
    o->scale_fac = scale_correction;
    o->thumbnail_border_color = LIGHTGRAY;
}

//draw the thumbnails
void drawThumbNails(){
    for(int i = 0; i < loaded_models; i++){
        DrawRectangleLines(object_array[i].home_pos.x, object_array[i].home_pos.y, THUMB_SLOT_WIDTH, THUMB_SLOT_HEIGHT, BLACK);
        DrawRectangleRec(object_array[i].bounds, LIGHTGRAY);
        Rectangle thumbSrc = {0.0f, 0.0f, (float)object_array[i].thumbnail.texture.width, -(float)object_array[i].thumbnail.texture.height };
        DrawTexturePro(object_array[i].thumbnail.texture, thumbSrc, object_array[i].bounds, (Vector2){0,0}, 0.0f, WHITE);
        DrawRectangleLinesEx(object_array[i].bounds, 1, object_array[i].thumbnail_border_color);
    }
}

//code the move the thumbnails left and right based on delta value
void scrollThumbNails(float delta){
    if((object_array[0].home_pos.x + delta) < 20){
        if(delta < 0) return;
        if (delta > 0) ; //we want the scroll to work in this instance, do nothing
    }
    if (object_array[loaded_models - 1].home_pos.x < (GetScreenWidth() - 70) && delta < 0) return;

    for (int i = 0; i < loaded_models; i++){
        object_array[i].home_pos.x += delta * 100; 
        object_array[i].bounds.x += delta * 100;       
    }
}

void draw3DViewPort(RenderTexture2D view_port, custom_cam3d *camera, int screen_width, int screen_height){
    BeginTextureMode(view_port);
    ClearBackground(RAYWHITE);
    Camera3D world_camera = {.position = (Vector3){0.0f,0.0f, 5.0f}, .fovy = 90,
                .projection = CAMERA_PERSPECTIVE, .target = {0.0f, 0.0f, 0.0f}, .up ={0, 1, 0}};
    BeginMode3D(camera->cam3D);
    DrawCube(WorldCenter, 1, 1, 1, RED);
    EndMode3D();
    EndTextureMode();

    Rectangle srcRec = { 0.0f, 0.0f, (float)view_port.texture.width, -(float)view_port.texture.height};
    Rectangle destRec = { 0.0f, 0.0f, (float)screen_width, (float)screen_height};
    DrawTexturePro(view_port.texture, srcRec, destRec, (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
}

//debug purposes only
void printFilePathToScreen(){
    if(loaded_models == 0) return;

    for(int i = 0; i < loaded_models; i++){
        DrawText(model_path[i], 10, 10 * i, 10, RED);
    }
}

//draws the background for the bottom UI, screen_height_ui the total height of the 2D screen
void draw2dUIForThumbnails(int screen_height_ui){
    float total_height = GetScreenHeight();
    float total_width = GetScreenWidth();

    DrawRectangle(0, total_height-screen_height_ui, total_width, screen_height_ui, BLUE);
}

//function to check if the mouse is over the 2D UI, returns true if so
bool getMouseCollisionRec2DScreen(int screen_height_ui){
    Rectangle UI_2D_Rec;
    UI_2D_Rec.y = GetScreenHeight() - screen_height_ui;
    UI_2D_Rec.x = 0;
    UI_2D_Rec.width = GetScreenWidth();
    UI_2D_Rec.height = screen_height_ui;

    if(!IsCursorOnScreen()){
        return false;
    }
    Vector2 mouse_pos = GetMousePosition();

    if(CheckCollisionPointRec(mouse_pos, UI_2D_Rec)){
        return true;
    }

    else return false;
}

//function to check if mouse is over the 3D viewport
bool getMouseCollisionRec3Dscreen(int screen_height_3d){
    Rectangle UI_3D_Rec;
    UI_3D_Rec.y = 0;
    UI_3D_Rec.x = 0;
    UI_3D_Rec.width = GetScreenWidth();
    UI_3D_Rec.height = screen_height_3d;

    if(!IsCursorOnScreen()){
        return false;
    }
    Vector2 mouse_pos = GetMousePosition();

    if(CheckCollisionPointRec(mouse_pos, UI_3D_Rec)){
        return true;
    }

    else return false;
    
}

//code to select a thumbnail
void pickThumbNail(int screen_height_ui){
    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
        //if we don't hit the UI, ignore the mouse button click
        if(!getMouseCollisionRec2DScreen(screen_height_ui)) return;

        //click in the 2D UI, reset the selected thumbnail
        if(selected_thumbnail) selected_thumbnail->thumbnail_border_color = LIGHTGRAY;
        selected_thumbnail = NULL;

        //then check to see if a new one was selected (can be the same one that was just de-select)
        for(int i = 0; i < loaded_models; i++){
            if(CheckCollisionPointRec(GetMousePosition(), object_array[i].bounds)){
                object_array[i].thumbnail_border_color = RED;
                selected_thumbnail = &object_array[i];
                break; //mouse can only hit one possible thumbnail at a time
            }
        }
    }
}

//needed to unload all the malloc strings
void freeModelPathArray(){
    for(int i = 0; i < loaded_models; i++){
        free(model_path[i]);
    }
}

//unloads model data and model animations
void freeObjectData(){
    for(int i = 0; i< loaded_models; i++){
        UnloadModelAnimations(object_array[i].animations, object_array[i].animation_num);
        UnloadModel(object_array[i].model);
        UnloadRenderTexture(object_array[i].thumbnail);
    }
}

int refactor_main(){
// Initialization
    //--------------------------------------------------------------------------------------
    int screen_width = 1280;
    int screen_height_3d = 720;
    int screen_height_ui = 140;
    int screen_height = screen_height_3d + screen_height_ui;
    InitWindow(screen_width, screen_height, "Raylib Edit Mode");

    RenderTexture2D view_port_3d = LoadRenderTexture(screen_width, screen_height_3d);
    custom_cam3d world_cam = Init3dCamera();
    setCameraPosition(&world_cam, (Vector3){0,0,5});
    
    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        //Game logic here
        getDroppedGLBFilePath();

        if(GetMouseWheelMove()){
            scrollThumbNails(GetMouseWheelMove());
        }

        if(IsMouseButtonDown(MOUSE_MIDDLE_BUTTON) && !IsKeyDown(KEY_LEFT_SHIFT) && getMouseCollisionRec3Dscreen(screen_height_3d)){
            rotateCameraAroundCurrentTarget(&world_cam);
        }

        if(IsMouseButtonDown(MOUSE_MIDDLE_BUTTON) && IsKeyDown(KEY_LEFT_SHIFT) && getMouseCollisionRec3Dscreen(screen_height_3d)){
            MoveCameraPos(&world_cam);
        }

        pickThumbNail(screen_height_ui);

        
        //Drawing logic here
        BeginDrawing();
        ClearBackground(WHITE);
        draw3DViewPort(view_port_3d, &world_cam, screen_width, screen_height_3d);
        draw2dUIForThumbnails(screen_height_ui);
        drawThumbNails();
        DrawText(TextFormat("Camera x: %f y: %f z: %f", world_cam.cam3D.position.x, world_cam.cam3D.position.y, world_cam.cam3D.position.z), 10, 10, 10, RED);
        printFilePathToScreen();
        EndDrawing();

    }

    //unload from GPU and free other data
    freeModelPathArray();
    freeObjectData();
    UnloadRenderTexture(view_port_3d);

    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

