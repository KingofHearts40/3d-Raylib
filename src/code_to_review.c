#include "main_function_entry.h"
#include "global_constants.h"
#include "raylib.h"
#include "raymath.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define SLOT_WIDTH 100.0f
#define SLOT_HEIGHT 100.0f
#define SLOT_SPACING 110.0f

// Structural Data Layouts
typedef struct {
    Model model;
    RenderTexture2D thumbnail; 
    Rectangle bounds;    
    Vector2 homePos;     
    bool isBeingDragged;
    float scaleFactor;
    Vector3 centerOffset;
} DraggableItem;

typedef struct {
    int inventoryIndex;     
    Vector3 worldPosition;  
} PlacedInstance;

// System Global Variables for State Management
DraggableItem* inventory = NULL;
int inventoryCount = 0;
int inventoryCapacity = 0;

PlacedInstance* placedObjects = NULL;
int placedCount = 0;
int placedCapacity = 0;

DraggableItem* activeDragItem = NULL;
int activeDragIndex = -1;
bool dragPreviewValid = false;
Vector3 dragPreviewWorldPos = { 0.0f, 0.0f, 0.0f };


//given a model path, loads it, gets the max dimension to scale it, draws the thumbnail
//stores the data in the inventory pointer/array
void HandleAssetRegistration(const char* path, Camera3D thumbCamera, float uiTopY) {
    //if no items, initializes capacity to 4, otherwise doubles capacity
    if (inventoryCount >= inventoryCapacity) {
        inventoryCapacity = (inventoryCapacity == 0) ? 4 : inventoryCapacity * 2;
        inventory = realloc(inventory, inventoryCapacity * sizeof(DraggableItem));
    }

    Model model = LoadModel(path);

    // Compute normalization bounds
    BoundingBox box = GetModelBoundingBox(model);
    float width = box.max.x - box.min.x;
    float height = box.max.y - box.min.y;
    float depth = box.max.z - box.min.z;
    //this next ternary gets the max dimension
    float maxDim = (width > height) ? ((width > depth) ? width : depth) : ((height > depth) ? height : depth);
    if (maxDim == 0.0f) maxDim = 1.0f;
    //scale factor needed to scale the max dimension to 1 (is the inverse of the max dimmension)
    float scaleFactor = 1.0f / maxDim;
    Vector3 center = { box.min.x + width/2.0f, box.min.y + height/2.0f, box.min.z + depth/2.0f };
    //need this offset to feed into DrawModelEX to move the model to the center of the camera view
    Vector3 centerOffset = { -center.x * scaleFactor, -center.y * scaleFactor + 0.5f, -center.z * scaleFactor };

    // creates a 2D thumbnail of the model, has a baked in left offset of 20 pixels, and 10 pixels
    //between each thumbnail
    RenderTexture2D thumbTarget = LoadRenderTexture((int)SLOT_WIDTH, (int)SLOT_HEIGHT);
    BeginTextureMode(thumbTarget);
        ClearBackground(BLANK); 
        BeginMode3D(thumbCamera);
            DrawModelEx(model, centerOffset, (Vector3){0.0f, 1.0f, 0.0f}, 0.0f, (Vector3){scaleFactor, scaleFactor, scaleFactor}, WHITE);
        EndMode3D();
    EndTextureMode();

    float slotX = 20.0f + (inventoryCount * SLOT_SPACING);
    float slotY = uiTopY + 25.0f;

    inventory[inventoryCount] = (DraggableItem){
        .model = model,
        .thumbnail = thumbTarget,
        .bounds = (Rectangle){ slotX, slotY, SLOT_WIDTH, SLOT_HEIGHT },
        .homePos = (Vector2){ slotX, slotY },
        .isBeingDragged = false,
        .scaleFactor = scaleFactor,
        .centerOffset = centerOffset
    };
    inventoryCount++;
}

// 2. RAYCASTING UTILITY: Helper function to convert raw screen mouse to 3D grid point
bool TryGetGridIntersection(Vector2 mousePos, Camera3D camera, float uiTopY, Vector3* outIntersection) {
    if (mousePos.y >= uiTopY) return false;

    Ray ray = GetMouseRay(mousePos, camera);
    if (ray.direction.y != 0.0f) {
        float t = -ray.position.y / ray.direction.y;
        if (t > 0.0f) {
            outIntersection->x = ray.position.x + ray.direction.x * t;
            outIntersection->y = 0.0f;
            outIntersection->z = ray.position.z + ray.direction.z * t;
            return true;
        }
    }
    return false;
}

// 3. STATE UTILITY: Handles mouse interactions, drag calculations, and drops
void UpdateInteractionState(Vector2 mousePos, Camera3D mainCamera, float uiTopY) {
    // Start dragging an item
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && activeDragItem == NULL) {
        for (int i = 0; i < inventoryCount; i++) {
            if (CheckCollisionPointRec(mousePos, inventory[i].bounds)) {
                inventory[i].isBeingDragged = true;
                activeDragItem = &inventory[i];
                activeDragIndex = i;
                break;
            }
        }
    }

    // Actively dragging an itemCamera3D c
    if (activeDragItem != NULL && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        activeDragItem->bounds.x = mousePos.x - (SLOT_WIDTH / 2.0f);
        activeDragItem->bounds.y = mousePos.y - (SLOT_HEIGHT / 2.0f);

        // Update real-time ghost preview coordinates
        dragPreviewValid = TryGetGridIntersection(mousePos, mainCamera, uiTopY, &dragPreviewWorldPos);
    } 
    // Drop execution
    else if (activeDragItem != NULL && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        if (dragPreviewValid) {
            if (placedCount >= placedCapacity) {
                placedCapacity = (placedCapacity == 0) ? 8 : placedCapacity * 2;
                placedObjects = realloc(placedObjects, placedCapacity * sizeof(PlacedInstance));
            }

            placedObjects[placedCount] = (PlacedInstance){
                .inventoryIndex = activeDragIndex,
                .worldPosition = dragPreviewWorldPos
            };
            placedCount++;
        }

        // Return the inventory icon cleanly back to its dock position
        activeDragItem->bounds.x = activeDragItem->homePos.x;
        activeDragItem->bounds.y = activeDragItem->homePos.y;
        activeDragItem->isBeingDragged = false;
        activeDragItem = NULL;
        activeDragIndex = -1;
        dragPreviewValid = false;
    }
}

// 4. DRAWING UTILITY: Renders the 3D scene and updates the 2D overlays
void RenderPipeline(RenderTexture2D main3DTarget, Camera3D mainCamera, int view3DW, int view3DH, int view2DH, float uiTopY) {
    // Pass A: Draw 3D world into offscreen texture target
    BeginTextureMode(main3DTarget);
        ClearBackground(RAYWHITE);
        BeginMode3D(mainCamera);
            DrawGrid(20, 1.0f);
            
            // Render existing world instances at full native scale
            for (int i = 0; i < placedCount; i++) {
                int idx = placedObjects[i].inventoryIndex;
                DrawModel(inventory[idx].model, placedObjects[i].worldPosition, 1.0f, WHITE);
            }

            // Render active ghost hologram preview if applicable
            if (activeDragItem != NULL && dragPreviewValid) {
                Color ghostColor = (Color){ 0, 121, 241, 120 }; 
                DrawModel(inventory[activeDragIndex].model, dragPreviewWorldPos, 1.0f, ghostColor);
                DrawCircle3D(dragPreviewWorldPos, 0.5f, (Vector3){1,0,0}, 90.0f, (Color){ 0, 121, 241, 200 });
            }
        EndMode3D();
    EndTextureMode();

    // Pass B: Draw frame outputs directly to screen canvas
    BeginDrawing();
        ClearBackground(DARKGRAY);

        // Blit 3D Scene
        Rectangle srcRec = { 0.0f, 0.0f, (float)main3DTarget.texture.width, -(float)main3DTarget.texture.height };
        Rectangle destRec = { 0.0f, 0.0f, (float)view3DW, (float)view3DH };
        DrawTexturePro(main3DTarget.texture, srcRec, destRec, (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);

        // Blit 2D Layout Dock
        DrawRectangle(0, (int)uiTopY, view3DW, view2DH, LIGHTGRAY);
        DrawLine(0, (int)uiTopY, view3DW, (int)uiTopY, GRAY);
        DrawText("Drag thumbnails up into the 3D scene grid to spawn objects:", 20, (int)uiTopY + 5, 12, DARKGRAY);

        // Render standard dock slots
        for (int i = 0; i < inventoryCount; i++) {
            DrawRectangleLines(inventory[i].homePos.x, inventory[i].homePos.y, SLOT_WIDTH, SLOT_HEIGHT, GRAY);
            
            if (!inventory[i].isBeingDragged) {
                DrawRectangleRec(inventory[i].bounds, WHITE);
                Rectangle thumbSrc = { 0.0f, 0.0f, (float)inventory[i].thumbnail.texture.width, -(float)inventory[i].thumbnail.texture.height };
                DrawTexturePro(inventory[i].thumbnail.texture, thumbSrc, inventory[i].bounds, (Vector2){0,0}, 0.0f, WHITE);
                DrawRectangleLinesEx(inventory[i].bounds, 1, DARKGRAY);
            }
        }

        // Render top floating layer for item tracking
        if (activeDragItem != NULL) {
            DrawRectangleRec(activeDragItem->bounds, (Color){ 255, 255, 255, 200 }); 
            Rectangle thumbSrc = { 0.0f, 0.0f, (float)activeDragItem->thumbnail.texture.width, -(float)activeDragItem->thumbnail.texture.height };
            DrawTexturePro(activeDragItem->thumbnail.texture, thumbSrc, activeDragItem->bounds, (Vector2){0,0}, 0.0f, WHITE);
            DrawRectangleLinesEx(activeDragItem->bounds, 2, ORANGE);
        }
    EndDrawing();
}

// ==========================================
// SYSTEM ENTRY MAIN ROUTINE LOOP
// ==========================================
int test_viewport_main(void) {
    const int view3DWidth = 1280;
    const int view3DHeight = 720;
    const int view2DHeight = 140; 
    const int windowHeight = view3DHeight + view2DHeight;
    const float uiTopY = (float)view3DHeight;

    InitWindow(view3DWidth, windowHeight, "Raylib - Part 2: Main Application Flow");
    SetTargetFPS(60);

    Camera3D mainCamera = { .position = { 12.0f, 12.0f, 12.0f }, .target = { 0.0f, 0.0f, 0.0f }, .up = { 0.0f, 1.0f, 0.0f }, .fovy = 45.0f, .projection = CAMERA_PERSPECTIVE };
    Camera3D thumbCamera = { .position = { 0.0f, 0.5f, 2.0f }, .target = { 0.0f, 0.5f, 0.0f }, .up = { 0.0f, 1.0f, 0.0f }, .fovy = 45.0f, .projection = CAMERA_PERSPECTIVE };

    RenderTexture2D main3DTarget = LoadRenderTexture(view3DWidth, view3DHeight);

    while (!WindowShouldClose()) {
        if (IsFileDropped()) {
            FilePathList droppedFiles = LoadDroppedFiles();
            for (unsigned int i = 0; i < droppedFiles.count; i++) {
                if (IsFileExtension(droppedFiles.paths[i], ".glb")) {
                    HandleAssetRegistration(droppedFiles.paths[i], thumbCamera, uiTopY);
                }
            }
            UnloadDroppedFiles(droppedFiles);
        }

        Vector2 mousePos = GetMousePosition();
        
        UpdateInteractionState(mousePos, mainCamera, uiTopY);
        RenderPipeline(main3DTarget, mainCamera, view3DWidth, view3DHeight, view2DHeight, uiTopY);
    }

    // System Shutdown & Cleanup
    for (int i = 0; i < inventoryCount; i++) {
        UnloadModel(inventory[i].model);
        UnloadRenderTexture(inventory[i].thumbnail);
    }
    free(inventory);
    free(placedObjects);
    UnloadRenderTexture(main3DTarget);
    CloseWindow();

    return 0;
}
