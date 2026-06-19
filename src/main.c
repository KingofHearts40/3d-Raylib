#include "raylib.h"
#include "raymath.h"
#include "camera.h"
#include "player.h"
#include "world_edit.h"

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

    player p = initPlayer(MODELFOLDER "robot.glb", (Vector3){0.0f, 1.75f, 0.0f});

    custom_cam3d custom_cam = Init3dCamera();
    setCameraTarget(&custom_cam, p.p_position);

    createTileMap();

    Ray r;

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        //movePlayer(&p);
        movePlayerVectors(&p);
        playerCamera3rdPersonControls(&p);
        LockCamera(&p);

        //gameobject code testing
        storeGLBasGameObject();
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            selectGameObject(p.p_camera_3rd_person.cam3D);
        }
        testMoveSelectedGameObj();

        r = getRayPerpendicularToActiveObject(&p.p_camera_3rd_person.cam3D);


        BeginDrawing();
        ClearBackground(WHITE);
        BeginMode3D(p.p_camera_3rd_person.cam3D);

        
        drawGameObjects();

        DrawRay(r, RED);
        

        EndMode3D();



        DrawFPS(10, 10);
        DrawText(TextFormat("x pos: %f\n, y pos: %f\n, z pos: %f", 
            p.p_camera_3rd_person.cam3D.position.x, p.p_camera_3rd_person.cam3D.position.y, 
            p.p_camera_3rd_person.cam3D.position.z),10, 30, 10, BLACK);
        EndDrawing();

    }

    //unload from GPU
    DestroyPlayer(&p);
    unloadGameObjects();
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}