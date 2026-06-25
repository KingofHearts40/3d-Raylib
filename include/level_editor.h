#ifndef LEVEL_EDITOR_H
#define LEVEL_EDITOR_H

#include "raylib.h"

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

typedef struct Object Object;
typedef struct World_Env_Obj World_Env_Obj;

void getDroppedGLBFilePath();
int saveModelSourcesArray();
int loadModelSourceArray();
void loadObjectArray();
int save3DWorldData();
int load3DWorldData();
void storeObjectDataInArray(char * model_file, int id);
void createThumbNailData(Object *o);
void drawThumbNails();
void scrollThumbNails(float delta);
void draw3DViewPort(RenderTexture2D view_port, custom_cam3d *camera, int screen_width, int screen_height);
void drawPreviewMesh3D(custom_cam3d *c, int height_3d_viewport);
void draw3DViewportMeshes();
void placeMesh3DSpace(custom_cam3d *c, int viewport_h);
void createWorldEnvObj(Vector3 pos);
void drawViewportMeshBBox(World_Env_Obj * w);
void updateWorldMeshBBox(World_Env_Obj *w);
void printFilePathToScreen();
void draw2dUIForThumbnails(int screen_height_ui);
bool getMouseCollisionRec2DScreen(int screen_height_ui);
bool getMouseCollisionRec3Dscreen(int screen_height_3d);
Vector3 getMouse3dDirection(custom_cam3d *c);
Vector3 convertMousePos3DSpace(custom_cam3d *c, int height_3d_viewport);
void pickThumbNail(int screen_height_ui);
void deselectThumbnail();
void pickViewportModel(custom_cam3d * c, int screen_height_3d);
void deselectViewportModel();
void moveSelectedViewportModel(custom_cam3d *c, float viewport_3d_h);
void updateWorldObjectPos3D(World_Env_Obj *w, Vector3 new_pos);
void UI2DControls(int screen_height_ui);
void ViewPort3DControls(int screen_height_3d, custom_cam3d * world_cam);
void freeModelPathArray();
void unloadObjectModelTextures(Object * o);
void freeObjectData();

int level_editor_main();

#endif