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

void addViewPortMemory(viewport_data **v){
    char *log_file = "../save_data/log_file.txt";
    if (*v == NULL){
        *v = malloc(sizeof(viewport_data) * 4);
            if (*v == NULL){
            logError(log_file,"viewport malloc failed\n");  
            }
            else viewport_capacity= 4;
    }

    else if(viewport_length == viewport_capacity && viewport_length > 0){
        void * temp_ptr = realloc(*v, sizeof(viewport_data) * viewport_capacity * 2);
        if(temp_ptr){
            *v = temp_ptr;
            viewport_capacity *= 2;
        }
        else{
            logError(log_file, "realloc for view_port failed");
        }
    }
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

void updateViewportsPositions(Vector2 delta){
    for(int i = 0; i < viewport_length; i++){
        if(CheckCollisionPointRec(GetMousePosition(), viewports[i].bounds)){
            if(!CheckCollisionPointRec(GetMousePosition(), viewports[i].resize_toggle)){
                viewports[i].bounds.x += delta.x;
                viewports[i].bounds.y += delta.y;
                viewports[i].resize_toggle.x += GetMouseDelta().x;
                viewports[i].resize_toggle.y += GetMouseDelta().y;    
            break; //if two overlap, this prevents moving them at the same time
            }
            else{
                viewports[i].bounds.width += GetMouseDelta().x;
                viewports[i].bounds.height += GetMouseDelta().y;
                viewports[i].resize_toggle.x += GetMouseDelta().x;
                viewports[i].resize_toggle.y += GetMouseDelta().y;                
            }
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