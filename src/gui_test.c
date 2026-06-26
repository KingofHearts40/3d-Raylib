#include "gui_test.h"
#include "raygui.h"
#include "stdio.h"
#include <stdlib.h>

typedef enum UI_element_type{
    BACKGROUND_UI_ELEM
}UI_element_type;

typedef struct background_UI{
    Rectangle bounds;
    Color color;
}background_UI;

typedef struct data_holder{
    void * pointer;
    UI_element_type type;
    size_t capacity;
    size_t current_element;
}data_holder;


size_t data_element_type_size(data_holder *d){
    switch (d->type)
    {
    case BACKGROUND_UI_ELEM:
        return(sizeof(background_UI));
        break;
    
    default:
        break;
    }
}   

void * allocate_data_pointer_memory(data_holder *d){
    //get the size of the element stored in the void *
    size_t bytes_needed = data_element_type_size(d);
    void * temp_ptr = NULL;

    if(d->capacity == 0){
        temp_ptr = malloc(sizeof(bytes_needed) * 4);
        if(temp_ptr == NULL){
            FILE *fptr = fopen("../save_data/log_file.txt", "w");
            if(fptr) fprintf(fptr, "Failed to malloc memory for UI");
            fclose(fptr);
            return NULL;
        }
        else {
            d->capacity = 4;
        }
    }

    else if(d->current_element == d->capacity){
        temp_ptr = realloc(d->pointer, bytes_needed * d->capacity * 2);
        if(temp_ptr == NULL){
            FILE *fptr = fopen("../save_data/log_file.txt", "w");
            if(fptr) fprintf(fptr, "Failed to realloc memory for UI");
            fclose(fptr);
            return NULL;
        }
        else{
            d->pointer = temp_ptr;
            d->capacity *= 2;
        }

    }

    return temp_ptr;
}

void deallocate_all_pointer_elements(data_holder *d){
    for(int i = 0; i < d->current_element; i++){
        free(d[i].pointer);
    }
}

void create_new_background_ui(data_holder * bg_ui){
    if(bg_ui->type != BACKGROUND_UI_ELEM){
        FILE *fptr = fopen("../save_data/log_file.txt", "w");
        fprintf(fptr, "Attempted to create background, pointer is either empty or wrong type" );
        fclose(fptr);
        return;
    }
    
    bg_ui->pointer = allocate_data_pointer_memory(bg_ui);

    background_UI ui = {.bounds.height = 100, .bounds.width = 100, .bounds.x = 100, 
        .bounds.y = 100, .color = BLACK};
    ((background_UI*)(bg_ui->pointer))[bg_ui->current_element] = ui;
    bg_ui->current_element++;
}

int gui_main(){
    int layout_width = 1280;
    int layout_height = 720;
    int control_width = 200;
    int control_height = 720;

    data_holder UI_elements = {.pointer = NULL, .capacity = 0, .current_element = 0, 
    .type = BACKGROUND_UI_ELEM};


    Rectangle layout = {.width = layout_width, .height = layout_height, .x = 0, .y = 0};
    Rectangle control_panel = {.width = control_width, .height = control_height, .x = layout_width, .y = 0};

    int total_window_width = layout_width + control_width;

    InitWindow(total_window_width, layout_height, "GUI TEST");

    Rectangle button_rec = {.height = 100, .width = 100, .x = 100, .y = 100};
    Rectangle message_rec = {.height = 100, .width = 100, .x = 300, .y = 300};
    
    bool show_message_box = false;

    while(!WindowShouldClose()){
        //game logic
        create_new_background_ui(&UI_elements);


        if(IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && CheckCollisionPointRec(GetMousePosition(), button_rec)){
            button_rec.x += GetMouseDelta().x;
            button_rec.y += GetMouseDelta().y;
        }


        //draw logic
        BeginDrawing();
        ClearBackground(LIGHTGRAY);
        DrawRectangleRec(layout, YELLOW);
        DrawRectangleRec(control_panel, RED);
        if(GuiButton(button_rec, "test_button")){
            show_message_box = true;            
        }
        
        if(show_message_box) GuiMessageBox(message_rec, "TEST", "Message", "b");

        EndDrawing();
    }

    deallocate_all_pointer_elements(&UI_elements);
    //unloading code

    return 0;
}