#include "raylib.h"
#include "raymath.h"
#include "camera.h"
#include "player.h"
#include "world_edit.h"
#include "global_constants.h"

enum edit_mode_states{
    IDLE,
    MOVE_OBJECT
};


//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    GetMonitorWidth(0);
    GetMonitorHeight(0);
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - 3d picking");
    int new_screen_width = GetMonitorWidth(0) * 0.33f;
    int new_screen_height = GetMonitorHeight(0) * 0.5f;
    SetWindowSize(new_screen_width, new_screen_height);

    #define MODELFOLDER "../model3d/"

    Model cube = LoadModel(MODELFOLDER "cube.glb");


    custom_cam3d world_camera = Init3dCamera();
    setCameraPosition(&world_camera, (Vector3){0.0f, 0.0f, 5.0f});

    enum edit_mode_states edit_mode = IDLE;

    
    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        //movePlayer(&p);
        // movePlayerVectors(&p);
        // playerCamera3rdPersonControls(&p);
        // LockCamera(&p);

        //gameobject code testing
        storeGLBasGameObject();
        
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            selectGameObject(world_camera.cam3D);
        }
        
        Vector3 camera_up = world_camera.cam3D.up;
        Vector3 camera_right = world_camera.x_axis;

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

        BeginDrawing();
        ClearBackground(WHITE);
        BeginMode3D(world_camera.cam3D);

        
        drawGameObjects();    
        DrawModel(cube, WorldCenter, 1, WHITE);  

        EndMode3D();

        DrawFPS(10, 10);
        DrawText(TextFormat("x pos: %f\n, y pos: %f\n, z pos: %f", 
            world_camera.cam3D.position.x, world_camera.cam3D.position.y, 
            world_camera.cam3D.position.z),10, 30, 10, BLACK);

        DrawText(TextFormat("Camera Up x pos: %f\n, y pos: %f\n, z pos: %f", camera_up.x, 
            camera_up.y, camera_up.z), 10, 70, 10, BLACK);
        EndDrawing();

    }

    //unload from GPU
    unloadGameObjects();
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}