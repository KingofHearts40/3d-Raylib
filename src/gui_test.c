#include "gui_test.h"
#include "raygui.h"
#include "stdio.h"
#include <stdlib.h>

typedef enum UI_element_type{
    BACKGROUND_UI_ELEM
}UI_element_type;

typedef struct viewport_data{
    Rectangle bounds;
    Color color;
    Rectangle resize_toggle;
    bool resizing;
    bool dragging;
}viewport_data;

viewport_data * viewports = NULL;
int viewport_capacity = 0;
int viewport_length = 0;

void logError(char * file, char * error_message){
    FILE * fptr = fopen(file, "a");
    if(fptr){
        fprintf(fptr, "%s", error_message);
    }
    else{
        printf("could not open file %s", file);
    }
    fclose(fptr);
}

//this can be used to allocate memory for any ptr that has a tracking 
//variable for max allocation and current pos
//ptr needs to be cast: (void**)&myptr, use sizeof for second variable
//don't forget & for 3rd variable
int genericMemoryAlloc(void **ptr, size_t size_of, int *capacity, int length){
    char *log_file = "../save_data/log_file.txt";
    if (*ptr == NULL){
        *ptr = malloc(size_of * 4);
            if (*ptr == NULL){
            logError(log_file,"malloc failed to provide memory\n");
            return 1;  
            }
            else{
                *capacity = 4;
                return 0;
            }
    }

    else if(length == (*capacity) && length > 0){
        void * temp_ptr = realloc(*ptr, size_of * (*capacity) * 2);
        if(temp_ptr){
            *ptr = temp_ptr;
            *capacity *= 2;
            return 0;
        }
        else{
            logError(log_file, "realloc failed to resize requested data\n");
            return 2;
        }
    }

    return 0;
}

void deallocatePointer(void **ptr, int *capacity, int *length){
    free(*ptr);
    *ptr = NULL;
    *capacity = 0;
    *length = 0;
}

void createNewViewPort(){
    //addViewPortMemory(&viewports);
    int success = genericMemoryAlloc((void**)&viewports, sizeof(viewport_data), &viewport_capacity, viewport_length);

    if (success != 0) return;

    viewports[viewport_length].bounds = (Rectangle){.height = 100, .width = 100, .x = 500, .y = 100};
    viewports[viewport_length].color = BLACK;
    viewports[viewport_length].resizing = false;

    viewports[viewport_length].resize_toggle = (Rectangle){.height = 20, .width = 20, 
        .x = viewports[viewport_length].bounds.x +viewports[viewport_length].bounds.width - 20, 
        .y = viewports[viewport_length].bounds.y +viewports[viewport_length].bounds.height - 20};

    viewport_length++;
}

void drawViewPorts(){
    for(int i = 0; i < viewport_length; i++){
        DrawRectangleRec(viewports[i].bounds, viewports[i].color);
        DrawRectangleRec(viewports[i].resize_toggle, ORANGE);
    }
}

void toggleViewportDragResize(){
    bool collided_rec = false;
    for(int i = 0; i < viewport_length; i++){
        if(collided_rec == true){
            //hit a different rec, just want to set this one to false
            viewports[i].resizing = false;
            viewports[i].dragging = false;
            continue; //go to next viewport to set it false            
        }
        //first check if I hit the viewport, then check for the toggle
        if(CheckCollisionPointRec(GetMousePosition(), viewports[i].bounds)){
            if(CheckCollisionPointRec(GetMousePosition(), viewports[i].resize_toggle)){
                viewports[i].resizing = true;
                collided_rec = true;
            }
            else{//didn't hit the toggle, but still hit the viewport, can drag
                viewports[i].resizing = false;
                viewports[i].dragging = true;
                collided_rec = true;                
            }
        }
        else{//didn't hit this viewPort, don't want to do anything
            viewports[i].resizing = false;
            viewports[i].dragging = false;
        }        
    }    
}

void updateViewportsPositions(Vector2 delta){
    float min_width = 30.0f, min_height = 30.0f;

    for(int i = 0; i < viewport_length; i++){
        if(viewports[i].resizing == true){
            //first make sure we don't get below the min width/height
            if(viewports[i].bounds.width + delta.x <= min_width){
                continue;
            }
            if(viewports[i].bounds.height + delta.y <= min_height){
                continue;
            }            
            viewports[i].bounds.width += delta.x;
            viewports[i].bounds.height += delta.y;
            viewports[i].resize_toggle.x += delta.x;
            viewports[i].resize_toggle.y += delta.y;             
        }
        else if(viewports[i].dragging == true){
                viewports[i].bounds.x += delta.x;
                viewports[i].bounds.y += delta.y;
                viewports[i].resize_toggle.x += delta.x;
                viewports[i].resize_toggle.y += delta.y;    
            break; //if two overlap, this prevents moving them at the same time
        }
    }
}

int gui_main(){
    int layout_width = 1280;
    int layout_height = 720;
    int control_width = 200;
    int control_height = 720;

    Rectangle layout = {.width = layout_width, .height = layout_height, .x = 0, .y = 0};
    Rectangle control_panel = {.width = control_width, .height = control_height, .x = layout_width, .y = 0};

    int total_window_width = layout_width + control_width;

    InitWindow(total_window_width, layout_height, "GUI TEST");

    Rectangle button_rec = {.height = 100, .width = 100, .x = 100, .y = 100};
    Rectangle message_rec = {.height = 100, .width = 100, .x = 300, .y = 300};

    Rectangle right_click_menu = {.height = 20, .width = 100, .x = 0, .y = 0};
    const char* menu_items = "Copy;Paste;Delete";
    int active_item = -1;
    bool show_right_click_menu = false;
    
    bool show_message_box = false;

    SetTargetFPS(60);
    while(!WindowShouldClose()){
        //game logic

        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), button_rec)){
            button_rec.x += GetMouseDelta().x;
            button_rec.y += GetMouseDelta().y;
        }

        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            toggleViewportDragResize();
        }

        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
            updateViewportsPositions(GetMouseDelta());               
        }

        if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)){
            Vector2 mousePos = GetMousePosition();
            right_click_menu.x = mousePos.x;
            right_click_menu.y = mousePos.y;
            show_right_click_menu = true;
        }
           


        //draw logic
        BeginDrawing();
        ClearBackground(LIGHTGRAY);
        DrawRectangleRec(layout, YELLOW);
        DrawRectangleRec(control_panel, RED);
        if(GuiButton(button_rec, "test_button")){
            show_message_box = true; 
            createNewViewPort();           
        }

        drawViewPorts();
        
        if(show_message_box) GuiMessageBox(message_rec, "TEST", "Number of new rectangles:", "b");
        if (show_right_click_menu){
            int toggle_show = GuiDropdownBox(right_click_menu, menu_items, &active_item, show_right_click_menu);
            //GuiDropdownMenuBox returns 0 until left mouse button is clicked
            if (toggle_show){
                show_right_click_menu = false;
            } 
        }

        EndDrawing();
    }


    //unloading code
    
    deallocatePointer((void**)&viewports, &viewport_capacity, &viewport_length);

    return 0;
}