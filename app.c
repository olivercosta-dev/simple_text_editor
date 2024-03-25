#ifndef LINES_C
#include "lines.c"
#endif
#include <stddef.h>
#include <stdlib.h>

#define MAX_LINE_SIZE 256
#define INPUT_BUFFER_SIZE 10
#define TRUE 1
#define FALSE 0

typedef struct {
    int row;
    int col;
} Cursor;

Cursor* new_cursor() {
    Cursor* cursor = malloc(sizeof(Cursor));
    if(cursor == NULL){
        puts("Failed to allocate memory for cursor.");
        exit(-1);
    }
    cursor -> row = 0;
    cursor -> col = 0;
    return cursor;
}

typedef enum {
    EDIT,
    MOVE
} AppMode;

typedef struct {
    LineList* line_list;
    Cursor* cursor;
    char input_buf[10];
    AppMode mode;
} App;

// TODO check malloc failiures everywhere
App* new_app(LineList* line_list) {
    if(line_list == NULL){
        line_list = new_line_list();
    }
    App* app = malloc(sizeof(App));
    app -> line_list = line_list;
    app -> cursor = new_cursor();
    app -> input_buf[0] = '\0';
    app -> mode = MOVE;
    return app;
} 

// TODO (oliver): Make the arguments the app or something
void print_line(Line* line, int with_cursor, int cursor_col){
    if(with_cursor == TRUE) {
        printf("--->");
        for(int i = 0; i < strlen(line -> content); i++) {
            if(i == cursor_col) {
                printf("|%c", line -> content[i]);
            }
            else {
                printf("%c", line -> content[i]);
            }
        }
        printf("<---\n");
    }
    else {
        printf("%s\n", line -> content);
    }
}

void print_line_list(App* app) {
    for (int i = 0; i < app -> line_list -> len; i++) {
        if(i == app-> cursor -> row) {
            print_line(&app -> line_list -> lines[i], TRUE, app -> cursor -> col);
        } else {
            print_line(&app -> line_list -> lines[i], FALSE, app -> cursor -> col);
        }
    }
}

char* strtrim(char* str) {
    char* end;

    while (isspace((unsigned char)*str)) str++;

    if (*str == 0) {
        return str;
    }

    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    *(end + 1) = '\0';

    return str;
}

Line* new_line(char* content) {
    Line* line = malloc(sizeof(Line));
    if (line == NULL) {
        puts("Failed to allocated memory for line!");
        exit(EXIT_FAILURE);
    }

    line -> content = malloc(strlen(content) + 1);
    if (line->content == NULL) {
        free(line);
        puts("Failed to allocated memory for line content!");
        exit(EXIT_FAILURE);
        return NULL;
    }
    int content_length = strlen(content);
    strcpy(line -> content, content);
    strtrim(line->content);
    return line;
}

LineList* new_line_list_from_file(char* filename) {
    FILE* file = fopen(filename, "r");
    char buf[MAX_LINE_SIZE];
    LineList* line_list = new_line_list();

    while (fgets(buf, sizeof(buf), file)){
        add_to_line_list(line_list, new_line(buf));
    }
    fclose(file);
    return line_list;
}

void modify_line_content_ascii(Line* line, int char_index, char new_char){
    int line_len = strlen(line -> content);
    if(line_len == 0 || char_index >=  line_len || char_index < 0) {
        puts("Unable to modify line at that position: Invalid index!");
        //TODO (oliver): maybe don't exit.
        exit(-1);
    }
    line -> content[char_index] = new_char;
}

void add_char_to_line_at(Line* line, int char_index, char new_char)
{
    int line_len = strlen(line -> content);
    char* modified_line_content = calloc((line_len + 2), sizeof(char));
    for(int i = line_len; i > char_index; i--) {
        modified_line_content[i] = line -> content[i - 1];
    }
    modified_line_content[char_index] = new_char;
    for(int i = char_index - 1; i >= 0; i--) {
        modified_line_content[i] = line -> content[i];
    }
    modified_line_content[line_len + 1] = '\0';
    free(line->content);
    line->content = modified_line_content;
}

// TODO (oliver): Cross-line compability without accessing "bad" memory
void move_cursor_up(App* app) {
    if(app -> cursor -> row == 0)
        return;
    app->cursor->row--;
}
void move_cursor_down(App* app) {
    if(app -> cursor -> row == app -> line_list -> len - 1)
        return;
    app -> cursor->row++;
}
void move_cursor_left(App* app) {
    if(app -> cursor -> col == 0)
        return;
    app->cursor->col--;
}
void move_cursor_right(App* app) {
    int current_row = app -> cursor -> row;
    char* current_line = app -> line_list -> lines[current_row].content;
    int current_line_len = strlen(current_line);

    if(app -> cursor -> col == current_line_len)
        return;
    app -> cursor -> row++;
}

// void move_cursor(App* app, char input_char) {
//     switch (input_char)
//     {
//         case UP:
//         {
//             move_cursor_up(app);
//             break;
//         }
//         case DOWN:
//         {
//             move_cursor_down(app);
//             break;
//         }
//         case LEFT:
//         {
//             move_cursor_left(app);
//             break;
//         }
//         case RIGHT:
//         {
//             move_cursor_right(app);
//             break;
//         }
//         default:
//             break;
//     }
// }

void handle_input(App* app, char input_char)
{
    if(input_char == 72){
        puts("ARROW WAS PRESSED!");
        exit(-1);
    }
   switch (app -> mode)
   {
        case MOVE:
        {
            // move_cursor(app, input_char);
            break;
        }
        case EDIT:
        {

        }
        default:
            break;
   }
}


