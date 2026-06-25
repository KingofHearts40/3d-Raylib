#include "main_function_entry.h"
#include "raylib.h"
#include "raymath.h"
#include "camera.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "level_editor.h"

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

typedef struct World_Env_Obj{
    int obj_id;
    Vector3 pos;
    BoundingBox bbox;
    Color bbox_color;
}World_Env_Obj;

//array to hold model path
char *model_path[500];
Object object_array[500];
int loaded_models = 0;
Object *selected_thumbnail = NULL;
World_Env_Obj world_env_obj_arr[500];
World_Env_Obj * selected_world_env_obj = NULL;
int total_world_obj = 0;

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

int saveModelSourcesArray(){
    FILE *save_model_source_file = fopen("../save_data/save_model_data.bin", "w");
    if(save_model_source_file == NULL){
        printf("Failed to open file");
        return -1;
    }

    fprintf(save_model_source_file, "%d\n", loaded_models);
    
    for(int i = 0; i < loaded_models; i++){
        fprintf(save_model_source_file, "%s\n", model_path[i]);
    }

    fclose(save_model_source_file);
    return 0;
}

int loadModelSourceArray(){
    FILE *save_model_source_file = fopen("../save_data/save_model_data.bin", "r");
    if(save_model_source_file == NULL){
        printf("Unable to find save data");
        return -1;
    }

    fscanf(save_model_source_file, "%d\n", &loaded_models);
    
    int buffer_size;
    char buffer[buffer_size];
    for(int i = 0; i < loaded_models; i++){
        if(fgets(buffer, sizeof(buffer), save_model_source_file)){
            buffer[strcspn(buffer, "\n")] = '\0';
            model_path[i] = (char*)malloc(sizeof(char) * buffer_size);
            //successful malloc check
            if(model_path[i] != NULL){
                strncpy(model_path[i], buffer, buffer_size-1);
                model_path[i][buffer_size] = '\0';
            }
            //if it fails
            else if(!model_path[i]){
                printf("failed to allocate model_path data");
                return -1;
            }           
        }
    }

    fclose(save_model_source_file);
    return 0;    
}

void loadObjectArray(){
    for(int i = 0; i < loaded_models; i++){
        storeObjectDataInArray(model_path[i], i);
    }
}

int save3DWorldData(){
    FILE *world3D = fopen("../save_data/world3d.sav", "w");

    if(!world3D) return -1; //failed to get a file to write to

    fprintf(world3D, "%d\n", total_world_obj);

    World_Env_Obj *save_obj = &world_env_obj_arr[0];

    for(int i = 0; i < total_world_obj; i++){
        fprintf(world3D, 
            "obj_id: %d, "
            "pos: {%f, %f, %f} "
            "Box Color: {%hhu, %hhu, %hhu, %hhu} "
            "Box Max Vec3{%f, %f, %f} "
            "Box Min Vec3{%f, %f, %f}\n",             
            save_obj->obj_id, 
            save_obj->pos.x, save_obj->pos.y, save_obj->pos.z,
            save_obj->bbox_color.r, save_obj->bbox_color.g, save_obj->bbox_color.b, save_obj->bbox_color.a,
            save_obj->bbox.max.x, save_obj->bbox.max.y, save_obj->bbox.max.z,
            save_obj->bbox.min.x, save_obj->bbox.min.y, save_obj->bbox.min.z          
        );

        save_obj++;
    }

    fclose(world3D);
    return 0;
}

int load3DWorldData(){
    FILE *world3D = fopen("../save_data/world3d.sav", "r");
    if(!world3D) return -1;

    fscanf(world3D, "%d\n", &total_world_obj);

    World_Env_Obj *save_obj = &world_env_obj_arr[0];

    for(int i = 0; i<total_world_obj; i++){
        fscanf(world3D,
            "obj_id: %d, "
            "pos: {%f, %f, %f} "
            "Box Color: {%hhu, %hhu, %hhu, %hhu} "
            "Box Max Vec3{%f, %f, %f} "
            "Box Min Vec3{%f, %f, %f}\n",             
            &save_obj->obj_id, 
            &save_obj->pos.x, &save_obj->pos.y, &save_obj->pos.z,
            &save_obj->bbox_color.r, &save_obj->bbox_color.g, &save_obj->bbox_color.b, &save_obj->bbox_color.a,
            &save_obj->bbox.max.x, &save_obj->bbox.max.y, &save_obj->bbox.max.z,
            &save_obj->bbox.min.x, &save_obj->bbox.min.y, &save_obj->bbox.min.z             
         );

         save_obj++; //pointer math, move to next array pos
    }

    fclose(world3D);
    return 0;
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
    DrawCube(WorldCenter, 0.1f, 0.1f, 0.1f, RED);
    draw3DViewportMeshes();
    
    if(getMouseCollisionRec3Dscreen(screen_height)){
        drawPreviewMesh3D(camera, screen_height);
    }

    EndMode3D();
    EndTextureMode();

    Rectangle srcRec = { 0.0f, 0.0f, (float)view_port.texture.width, -(float)view_port.texture.height};
    Rectangle destRec = { 0.0f, 0.0f, (float)screen_width, (float)screen_height};
    DrawTexturePro(view_port.texture, srcRec, destRec, (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
}

//draw a preview of the selecte thumbnail mesh on the 3d Viewport
void drawPreviewMesh3D(custom_cam3d *c, int height_3d_viewport){
    if(selected_thumbnail == NULL) return; //can only draw Preview if a thumbnail is selected

    Vector3 position = convertMousePos3DSpace(c, height_3d_viewport);

    DrawModel(selected_thumbnail->model, position, 1, (Color){255, 255, 255, 100});
}

//place 3d object in the viewport
void placeMesh3DSpace(custom_cam3d *c, int viewport_h){
    Vector3 pos = convertMousePos3DSpace(c, viewport_h);
    createWorldEnvObj(pos);
}

void createWorldEnvObj(Vector3 pos){
    if(!selected_thumbnail) return;

    world_env_obj_arr[total_world_obj].obj_id = selected_thumbnail->id;
    world_env_obj_arr[total_world_obj].pos = pos;
    world_env_obj_arr[total_world_obj].bbox = GetModelBoundingBox(selected_thumbnail->model);
    world_env_obj_arr[total_world_obj].bbox_color = BLACK;
    updateWorldMeshBBox(&world_env_obj_arr[total_world_obj]);
    total_world_obj++;   
}

//draw the 3d objects placed in the viewport
void draw3DViewportMeshes(){
    for(int i = 0; i < total_world_obj; i++){
        int index = world_env_obj_arr[i].obj_id;
        Vector3 pos = world_env_obj_arr[i].pos;

        DrawModel(object_array[index].model, pos, 1.0f, WHITE);
        drawViewportMeshBBox(&world_env_obj_arr[i]);
    }
}

void drawViewportMeshBBox(World_Env_Obj * w){
    DrawBoundingBox(w->bbox, w->bbox_color);
}

void updateWorldMeshBBox(World_Env_Obj *w){
    w->bbox.max = Vector3Add(w->bbox.max, w->pos);
    w->bbox.min = Vector3Add(w->bbox.min, w->pos);
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

//gets the mouse direction in 3D space, might delete, not sure what use atm
//coould be useful for first person camera work
Vector3 getMouse3dDirection(custom_cam3d *c){
    Ray r = GetMouseRay(GetMousePosition(), c->cam3D);
    return(r.direction);
}

//converts camera position, target, and the mouse position to a 3D vector, needs viewport size
//to correct for the fact that the whole screen isn't the viewport
Vector3 convertMousePos3DSpace(custom_cam3d *c, int height_3d_viewport){
    Vector2 current_mouse_pos = GetMousePosition();
    float distance = Vector3Distance(c->cam3D.position, c->cam3D.target);
    float screen_height = height_3d_viewport;
    float screen_width = GetScreenWidth();
 
    float world_height_at_distance = 2.0f * distance * tanf(c->cam3D.fovy*DEG2RAD/2.0f);
    float pixel_to_world_scale = world_height_at_distance / screen_height;

    //since we are moving relative to the camera target, we can use that as reference
    // //top left is 0, 0 bottom right is Screenwidth, ScreenHeight
    Vector2 center_screen = {.x = screen_width/2, .y = screen_height/2}; // {this is where target of cam is}

    //pixels on screen * conversion factor to 3D world distance
    float x_offcenter = (current_mouse_pos.x - center_screen.x) * pixel_to_world_scale;
    float y_offcenter = (current_mouse_pos.y - center_screen.y) * pixel_to_world_scale;

    Vector3 x_pos_from_cam_target = Vector3Scale(c->x_axis, x_offcenter);
    Vector3 y_pos_from_cam_target = Vector3Scale(c->cam3D.up, y_offcenter);

    Vector3 mouse_pos_3d = Vector3Add(c->cam3D.target, x_pos_from_cam_target);
    mouse_pos_3d = Vector3Subtract(mouse_pos_3d, y_pos_from_cam_target);

    return mouse_pos_3d;
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

void deselectThumbnail(){
    if(selected_thumbnail) {
        selected_thumbnail->thumbnail_border_color = LIGHTGRAY;
        selected_thumbnail = NULL; 
    }

}

//select the closest viewport model
void pickViewportModel(custom_cam3d * c){
//if thumbnail is selected, we are placing objects in the 3D view port so don't select one to move  
    if (selected_thumbnail) return;
    //if there is an active world_obj and this function is called we unselect it and look for new
    if(selected_world_env_obj){
        deselectViewportModel();
    }

    Ray mouse_ray = GetMouseRay(GetMousePosition(), c->cam3D);

    for(int i = 0; i < total_world_obj; i++){
        RayCollision collision = GetRayCollisionBox(mouse_ray, world_env_obj_arr[i].bbox);
        //to avoid accidently dereferencing NULL ptr
        if(collision.hit && selected_world_env_obj == NULL){
            selected_world_env_obj = &world_env_obj_arr[i];
            selected_world_env_obj->bbox_color = RED;
        }

        else if(collision.hit && selected_world_env_obj){
            //check which object is closer, if current object closer, select it
            if(collision.distance < Vector3Distance(mouse_ray.position, selected_world_env_obj->pos)){
                deselectViewportModel();
                selected_world_env_obj = &world_env_obj_arr[i];
                selected_world_env_obj->bbox_color = RED;   
            }
        }
    }
}

void deselectViewportModel(){
    if(selected_world_env_obj){
        selected_world_env_obj->bbox_color = BLACK;
        selected_world_env_obj = NULL;
    }    
}

//moves the model around in 3d space with the mouse movement when active
void moveSelectedViewportModel(custom_cam3d *c, float viewport_3d_h){
    if(!selected_world_env_obj) return;
    if(selected_thumbnail) deselectViewportModel();
    float delta_x = GetMouseDelta().x, delta_y = GetMouseDelta().y;
    float distance = Vector3Distance(c->cam3D.position, selected_world_env_obj->pos);
    float screen_height = viewport_3d_h;

    //convert screen pixel to world space
    float world_height_at_distance = 2.0f * distance * tanf(c->cam3D.fovy*DEG2RAD/2.0f);
    float pixel_to_world_scale = world_height_at_distance / screen_height;
    
    delta_x *= pixel_to_world_scale;
    delta_y *= pixel_to_world_scale;

    Vector3 change_pos_x = Vector3Scale(c->x_axis, delta_x);
    Vector3 change_pos_y = Vector3Scale(c->cam3D.up, delta_y);

    Vector3 new_pos = Vector3Add(selected_world_env_obj->pos, change_pos_x);
    new_pos = Vector3Subtract(new_pos, change_pos_y);

    updateWorldObjectPos3D(selected_world_env_obj, new_pos);
}

//updates the position and the bounding box position for the world object
void  updateWorldObjectPos3D(World_Env_Obj *w, Vector3 new_pos){
    Vector3 last_pos = w->pos;
    
    //update player position to the provided Vec3
    w->pos = new_pos;

    //get the change in player position an add it to the boundingBox
    Vector3 delta_pos = Vector3Subtract(new_pos, last_pos);
    w->bbox.max = Vector3Add(w->bbox.max, delta_pos);
    w->bbox.min = Vector3Add(w->bbox.min, delta_pos);
}

//UI controls for keyboard and mouse
void UI2DControls(int screen_height_ui){
    if(!getMouseCollisionRec2DScreen(screen_height_ui)) return;

    if(GetMouseWheelMove()){
            scrollThumbNails(GetMouseWheelMove());
        }

    pickThumbNail(screen_height_ui);
}

//Viewport controls for the 3D display
void ViewPort3DControls(int screen_height_3d, custom_cam3d * world_cam){
    if (!getMouseCollisionRec3Dscreen(screen_height_3d)) return;   

    if(IsMouseButtonDown(MOUSE_MIDDLE_BUTTON) && !IsKeyDown(KEY_LEFT_SHIFT)){
            rotateCameraAroundCurrentTarget(world_cam);
        }

    if(IsMouseButtonDown(MOUSE_MIDDLE_BUTTON) && IsKeyDown(KEY_LEFT_SHIFT)){
            MoveCameraPos(world_cam);
        }
    
    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
       placeMesh3DSpace(world_cam, screen_height_3d);
       pickViewportModel(world_cam);  
    }

    if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
        moveSelectedViewportModel(world_cam, screen_height_3d);
    }

    if(GetMouseWheelMove()){
        float delta = GetMouseWheelMove();
        zoomCamera(world_cam, delta);
    }

    if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)){
        deselectThumbnail();
    }

    if(IsKeyPressed(KEY_S)){
        saveModelSourcesArray();
        save3DWorldData();
    }

    if(IsKeyPressed(KEY_L)){
        loadModelSourceArray();
        loadObjectArray();//need to reload object array with the model source or bad things happen
        //since both arrays use the loaded_model variable currently
        load3DWorldData();
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

int level_editor_main(){
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
    setCameraZoomParam(&world_cam, 0.2f, 100.0f);
    
    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        //Game logic here
        getDroppedGLBFilePath();

        UI2DControls(screen_height_ui);
        ViewPort3DControls(screen_height_3d, &world_cam);

        //Drawing logic here
        BeginDrawing();
        ClearBackground(WHITE);
        
        draw3DViewPort(view_port_3d, &world_cam, screen_width, screen_height_3d);
        draw2dUIForThumbnails(screen_height_ui);
        drawThumbNails();
        DrawText(TextFormat("Camera x: %f y: %f z: %f", world_cam.cam3D.position.x, world_cam.cam3D.position.y, world_cam.cam3D.position.z), 10, 10, 10, RED);
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

