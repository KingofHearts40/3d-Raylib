#include "main_function_entry.h"
#include "global_constants.h"
#include "world_edit.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


typedef enum GAME_MODE{
    EDIT_MODE,
    PLAY_TEST_MODE
}GAME_MODE;

GAME_MODE active_mode = EDIT_MODE;

typedef enum edit_mode_states{
    IDLE,
    MOVE_OBJECT
}edit_mode_states;

edit_mode_states edit_mode = IDLE;

int empty_main(){
    
    // Initialization
    //--------------------------------------------------------------------------------------
    int screen_width = 1280;
    int screen_height = 720;
    InitWindow(screen_width, screen_height, "Raylib Edit Mode");
    
    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        //Game logic here
        
        //Drawing logic here
        BeginDrawing();
        ClearBackground(WHITE);
        // BeginMode3D(active_camera);
        //EndMode3D();
        EndDrawing();

    }

    //unload from GPU

    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

int edit_mode_main(){

    GetMonitorWidth(0);
    GetMonitorHeight(0);

    // Initialization
    //--------------------------------------------------------------------------------------
    int screen_width = 1280;
    int screen_height = 720;
    InitWindow(screen_width, screen_height, "Raylib Edit Mode");
    
    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------
    custom_cam3d world_camera = Init3dCamera();
    Vector3 initial_cam_pos = {0.0f, 0.0f, 5.0f};
    setCameraPosition(&world_camera, initial_cam_pos);

    int test_anim_counter = 0;
    int animation_number = 2;

    // Main game loop
    while (!WindowShouldClose() &&active_mode == EDIT_MODE)        // Detect window close button or ESC key
    {
        //Game logic here
        storeGLBasGameObject();

        if(IsKeyPressed(KEY_P)){
            active_mode = PLAY_TEST_MODE;
        }
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            selectGameObject(world_camera.cam3D);
        }

        if(IsKeyDown(KEY_SPACE)){
            playActiveObjectAnim(&test_anim_counter, &animation_number);
        }

        if(IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) && !IsKeyDown(KEY_LEFT_SHIFT)){
            rotateCameraAroundCurrentTarget(&world_camera);
        }

        if(IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) && IsKeyDown(KEY_LEFT_SHIFT)){
            MoveCameraPos(&world_camera);
        }

        if (IsKeyPressed(KEY_G)){
            edit_mode = MOVE_OBJECT;
        }
        if(edit_mode == MOVE_OBJECT){
            MouseMoveSelectedGameObj(&world_camera);
            
            if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                edit_mode = IDLE;
            }
        }

        if(IsKeyPressed(KEY_TAB)){
            //testing incrementing the animation
            animation_number++;
        }

        //Drawing logic here
        BeginDrawing();
        ClearBackground(WHITE);
        BeginMode3D(world_camera.cam3D);

        drawGameObjects();

        DrawSphere(WorldCenter, 0.01, RED);

        EndMode3D();

        DrawText(TextFormat("Active Animation: %s", activeAnimationName(&animation_number)), 10, 10, 10, RED);

        EndDrawing();
    }

    //unload from GPU
    unloadGameObjects();

    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

