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
}viewport_data;

viewport_data * viewports = NULL;
int viewport_memory = 0;
int current_viewport_slot = 0;

void logError(char * file, char * error_message){
    FILE * fptr = fopen(file, "w");
    if(fptr){
        fprintf(fptr, error_message);
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
            else viewport_memory= 4;
    }

    else if(current_viewport_slot == viewport_memory && current_viewport_slot > 0){
        void * temp_ptr = realloc(*v, sizeof(viewport_data) * viewport_memory * 2);
        if(temp_ptr){
            *v = temp_ptr;
            viewport_memory *= 2;
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
            logError(log_file, "realloc for view_port failed");
            return 2;
        }
    }
}

void deallocatePointer(void **ptr, int *capacity, int *length){
    free(*ptr);
    *ptr = NULL;
    *capacity = 0;
    *length = 0;
}

void createNewViewPort(){
    //addViewPortMemory(&viewports);
    genericMemoryAlloc((void**)&viewports, sizeof(viewport_data), &viewport_memory, current_viewport_slot);

    viewports[current_viewport_slot].bounds = (Rectangle){.height = 100, .width = 100, .x = 500, .y = 100};
    viewports[current_viewport_slot].color = BLACK;
    current_viewport_slot++;
}

void drawViewPorts(){
    for(int i = 0; i < current_viewport_slot; i++){
        DrawRectangleRec(viewports[i].bounds, viewports[i].color);
    }
}

void updateViewportsPositions(Vector2 delta){
    for(int i = 0; i < current_viewport_slot; i++){
        if(CheckCollisionPointRec(GetMousePosition(), viewports[i].bounds)){
            viewports[i].bounds.x += delta.x;
            viewports[i].bounds.y += delta.y;
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
    
    bool show_message_box = false;

    while(!WindowShouldClose()){
        //game logic

        if(IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && CheckCollisionPointRec(GetMousePosition(), button_rec)){
            button_rec.x += GetMouseDelta().x;
            button_rec.y += GetMouseDelta().y;
        }

        if(IsMouseButtonDown(MOUSE_RIGHT_BUTTON)){
            updateViewportsPositions(GetMouseDelta());               
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

        EndDrawing();
    }


    //unloading code
    
    deallocatePointer((void**)&viewports, &viewport_memory, &current_viewport_slot);

    return 0;
}