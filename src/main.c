#include "raylib.h"
#include "raymath.h"
#include "camera.h"
#include "player.h"

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

    Model floor = LoadModel(MODELFOLDER "floor.glb");
    Model cube = LoadModel(MODELFOLDER "cube.glb");
 
    BoundingBox robot_bbox = GetMeshBoundingBox(p.p_model.meshes[0]);
    float scale = 0.5f;
    robot_bbox.min = Vector3Scale(robot_bbox.min, scale);
    robot_bbox.max = Vector3Scale(robot_bbox.max, scale);
    robot_bbox.min.y += 1.75f;
    robot_bbox.max.y += 1.75f;

    custom_cam3d custom_cam = Init3dCamera();
    setCameraTarget(&custom_cam, p.p_position);

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

        if(IsKeyDown(KEY_Z)){
            rotatePlayerModelY(&p, 1.0f);
        }

        if(IsKeyDown(KEY_X)){
            resetPlayerRotation(&p);
        }

        movePlayer(&p);
        playerCamera3rdPersonControls(&p);

        BeginDrawing();
        ClearBackground(WHITE);
        BeginMode3D(p.p_camera_3rd_person.cam3D);

        DrawRay(ray, MAROON);
        DrawModel(floor, (Vector3){0,0,0}, 1, WHITE);
        DrawModelEx(p.p_model, p.p_position, (Vector3){0,1,0}, 180.0f,(Vector3){0.5f, 0.5f, 0.5f}, WHITE);
        DrawBoundingBox(robot_bbox, RED);
        EndMode3D();

        DrawFPS(10, 10);
        DrawText(TextFormat("x pos: %f\n, y pos: %f\n, z pos: %f", 
            p.p_camera_3rd_person.cam3D.position.x, p.p_camera_3rd_person.cam3D.position.y, 
            p.p_camera_3rd_person.cam3D.position.z),10, 30, 10, BLACK);
        EndDrawing();
        

    }

    //unload from GPU
    UnloadModel(floor);
    UnloadModel(cube);
    DestroyPlayer(&p);
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}