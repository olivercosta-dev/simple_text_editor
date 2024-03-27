#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <termios.h>
#include <unistd.h>

#include "app.c"



#define KEY_MODE_SWITCH '\t'




int main(int argc, char* argv[])
{
    if(argc != 2) {
        puts("Incorrect arguments supplied.\nUse: editor filename.txt");
        return 1;
    }
    char* file_name = argv[1];
    LineList* line_list = new_line_list_from_file(file_name);
    App* app = new_app(line_list);
    print_line_list(app);

    // TODO (oliver): WRITE MODE WITH TAB
    while(1) {
        // clear_screen();
        char buf[100];
        ssize_t bytes_read = read(STDIN_FILENO, buf, sizeof(buf) - 1); // Leave space for '\0'
        if (bytes_read > 0) {
            buf[bytes_read] = '\0'; // Null-terminate the string
        }
        handle_input(app, buf);
    }
    return 0;
}


