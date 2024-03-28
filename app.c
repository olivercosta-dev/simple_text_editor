#include "lines.c"
#include "terminal.c"
#include <stddef.h>
#include <stdlib.h>

#define MAX_LINE_SIZE 256
#define INPUT_BUFFER_SIZE 10
#define TRUE 1
#define FALSE 0

typedef enum{
    UP,
    DOWN,
    LEFT,
    RIGHT
} Direction;


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



//? This is questionable - The whole appmode thing
typedef enum {
    EDIT,
    MOVE
}AppMode;
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
    configure_terminal();
    App* app = malloc(sizeof(App));
    app -> line_list = line_list;
    app -> cursor = new_cursor();
    app -> input_buf[0] = '\0';
    app -> mode = MOVE;
    return app;
} 

// TODO (oliver): Make the arguments the app or something
void print_line(Line* line, int with_cursor, int cursor_col){
    printf("%s\n", line -> content);
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
    free(line -> content);
    line->content = modified_line_content;
}

// TODO (oliver): Cross-line compability without accessing "bad" memory
void app_move_cursor_up(App* app) {
    if(app -> cursor -> row == 0)
        return;
    app->cursor->row--;
}
void app_move_cursor_down(App* app) {
    if(app -> cursor -> row == app -> line_list -> len - 1)
        return;
    app -> cursor->row++;
}
void app_move_cursor_left(App* app) {
    if(app -> cursor -> col == 0)
        return;
    app->cursor->col--;
}
void app_move_cursor_right(App* app) {
    int current_row = app -> cursor -> row;
    char* current_line = app -> line_list -> lines[current_row].content;
    int current_line_len = strlen(current_line);

    if(app -> cursor -> col == current_line_len)
        return;
    app -> cursor -> row++;
}


// #define UP "\033[A"
// #define DOWN "\033[B"
// #define RIGHT "\033[C"
// #define LEFT "\033[D"


Direction convert_to_direction(char buf[]){
    if(strcmp(buf, "\033[A") == 0){
        return UP;
    }
    else if(strcmp(buf, "\033[B") == 0){
        return DOWN;
    }
    else if(strcmp(buf, "\033[D") == 0){
        return LEFT;
    }
    else if(strcmp(buf, "\033[C") == 0){
        return RIGHT;
    }
    puts("Couldn't convert to direction!");
    exit(-1);
}
void app_cursor_up(App* app) {
    app->cursor->row--;
}
void app_cursor_down(App* app) {
    app->cursor->row++;
}
void app_cursor_left(App* app) {
    app->cursor->col--;
}
void app_cursor_right(App* app) {
    app->cursor->col++;
}
void handle_move(App* app, Direction dir) {
    if(dir == UP){
        terminal_cursor_up();
        app_cursor_up(app);
    }
    else if(dir == DOWN){
        terminal_cursor_down();
        app_cursor_down(app);
    }
    else if(dir == LEFT){
        terminal_cursor_left();
        app_cursor_left(app);
    }
    else if(dir == RIGHT){
        terminal_cursor_right();
        app_cursor_right(app);
    }
}

void rerender_line(App* app) {
    // TODO(oliver): rerender it from left or right depending on cursor position compared to the line's length
    clear_line_from_cursor_right();
    int cursor_col = app -> cursor -> col;
    int cursor_row = app -> cursor -> row;
    printf("%s",  app -> line_list -> lines[cursor_row].content + cursor_col);
    printf("\033[%d;%dH", cursor_row + 1 ,cursor_col + 1);
    app -> cursor -> col++;
    terminal_cursor_right();
    fflush(stdout);
}

void handle_edit(App* app, char buf[]) {
    int cursor_col = app -> cursor -> col;
    int cursor_row = app -> cursor -> row;
    
    // This presumes only ASCII characters
    add_char_to_line_at(&app -> line_list -> lines[cursor_row], cursor_col, buf[0]);
    
    rerender_line(app);
}

void handle_input(App* app, char buf[])
{
    int len = strlen(buf);
    if(len == 3) {
        Direction dir = convert_to_direction(buf);
        handle_move(app, dir);
    } else {
        handle_edit(app, buf); // backspace doesnt work! - need to rerender it manually
    }
}

