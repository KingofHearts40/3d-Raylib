#include "raylib.h"
#include "raymath.h"
#include "camera.h"

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - 3d picking");

    Model floor = LoadModel("model3d/floor.glb");
    Model cube = LoadModel("model3d/cube.glb");
    Model robot = LoadModel("model3d/robot.glb");
    Vector3 cube_pos = {0.0f, 1.75f, 0.0f};

    custom_cam3d custom_cam = Init3dCamera();
    setCameraTarget(&custom_cam, cube_pos);

    Ray ray = { 0 };       
    float angle = (float)(PI/360);             // Picking line ray

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
            ray = GetScreenToWorldRay(GetMousePosition(), custom_cam.cam3D);
            ray.position.y +=0.01f;
        }

        if (IsKeyDown(KEY_W)){
            cube_pos.z += -0.1f;
            custom_cam.cam3D.position.z += -0.1f;
            custom_cam.cam3D.target = cube_pos;
        }

        if (IsKeyDown(KEY_S)){
            cube_pos.z += +0.1f;
            custom_cam.cam3D.position.z += +0.1f;
            custom_cam.cam3D.target = cube_pos;
        }

        if(IsKeyDown(KEY_A)){
            cube_pos.x -= 0.1f;
            custom_cam.cam3D.position.x -= 0.1f;
            custom_cam.cam3D.target = cube_pos;
        }

        if(IsKeyDown(KEY_D)){
            cube_pos.x += 0.1f;
            custom_cam.cam3D.position.x += 0.1f;       
            custom_cam.cam3D.target = cube_pos;
        }

        if(IsKeyDown(KEY_R)){
            resetCamera(&custom_cam, cube_pos);
        }

        if(IsMouseButtonDown(MOUSE_BUTTON_RIGHT)){
            rotateCameraAroundCurrentTarget(&custom_cam);
        }

        if(GetMouseWheelMove()){
            float zoom = GetMouseWheelMove() * 0.4;
            zoomCamera(&custom_cam, zoom);

        }

        BeginDrawing();
        ClearBackground(WHITE);
        BeginMode3D(custom_cam.cam3D);

        DrawRay(ray, MAROON);
        DrawGrid(10, 1.0F);
        DrawModel(floor, (Vector3){0,0,0}, 1, WHITE);
        DrawModelEx(robot, cube_pos, (Vector3){0,1,0}, 180.0f,(Vector3){0.5f, 0.5f, 0.5f}, WHITE);
        EndMode3D();

        DrawFPS(10, 10);
        DrawText(TextFormat("x pos: %f\n, y pos: %f\n, z pos: %f", 
            custom_cam.cam3D.position.x, custom_cam.cam3D.position.y, 
            custom_cam.cam3D.position.z),10, 30, 10, BLACK);
        EndDrawing();
        

    }

    //unload from GPU
    UnloadModel(floor);
    UnloadModel(cube);
    UnloadModel(robot);
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}