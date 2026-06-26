#include "raylib.h"
#include "level_editor.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "gui_test.h"

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    gui_main();
    //level_editor_main();

    return 0;
}