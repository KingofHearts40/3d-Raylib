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
    Vector3 cube_pos = {0.0f,0.5f, 0.0f};

    Camera custom_cam = Init3dCamera();
    setCameraTarget(&custom_cam, cube_pos);


    // Define the camera to look into our 3d world
    Camera camera = { 0 };
    //camera.position = (Vector3){ 0.0f, 1.0f, 1.0f}; // Camera position0

    //test code for camera position following player
    camera.position.x = cube_pos.x;
    camera.position.y = cube_pos.y + 2;
    camera.position.z = cube_pos.z + 5;

    camera.target = cube_pos;      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;            // Camera projection type

    Ray ray = { 0 };       
    float angle = (float)(PI/360);             // Picking line ray

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
            ray = GetScreenToWorldRay(GetMousePosition(), camera);
            ray.position.y +=0.01f;
        }

        if(IsMouseButtonDown(MOUSE_BUTTON_RIGHT)){
            camera.position.x += 0.1f;
        }

        if (IsKeyDown(KEY_W)){
            cube_pos.z += -0.1f;
            camera.position.z = cube_pos.z + 5.0f;
            camera.target = cube_pos;
        }

        if (IsKeyDown(KEY_S)){
            cube_pos.z += +0.1f;
            camera.position.z = cube_pos.z + 5.0f;
            camera.target = cube_pos;
        }

        if(IsKeyDown(KEY_A)){
            cube_pos.x -= 0.1f;
            camera.target = cube_pos;
        }

        if(IsKeyDown(KEY_D)){
            cube_pos.x += 0.1f;
            camera.target = cube_pos;
        }

        if(IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_RIGHT)){
            //angle += 0.1f/PI;
            //we are going to try and rotate around the y axis of the cube
            //behind the player is angle 0, and we will limit (for now) to rotation to 90 deg
            float x = camera.position.x;
            float z = camera.position.z;
            camera.position.x = x * cosf(angle) + z * sinf(angle);
            camera.position.y = camera.position.y;
            camera.position.z = -x * sinf(angle) + z * cos(angle);

        }

        if(IsKeyDown(KEY_UP)){
            //we are going to try and rotate around the x axis of the cube
            //behind the player is angle 0, and we will limit (for now) to rotation to 90 deg
            float y = camera.position.y;
            float z = camera.position.z;
            camera.position.y = y * cosf(angle) - z * sinf(angle);
            camera.position.z = y * sinf(angle) + z * cos(angle);
        }

        rotateCameraAroundCurrentTarget(&custom_cam);
    
        BeginDrawing();
        ClearBackground(WHITE);
        BeginMode3D(custom_cam);

        DrawRay(ray, MAROON);
        DrawGrid(10, 1.0F);
        DrawModel(floor, (Vector3){0,0,0}, 1, WHITE);
        DrawModel(robot, (Vector3){0,1,0}, 0.5, WHITE);
        DrawModelEx(robot, cube_pos, (Vector3){0,1,0}, 180.0f,(Vector3){0.5f, 0.5f, 0.5f}, WHITE);
        EndMode3D();

        DrawFPS(10, 10);
        DrawText(TextFormat("x pos: %f\n, y pos: %f\n, z pos: %f", camera.position.x, camera.position.y, camera.position.z),10, 30, 10, BLACK);
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