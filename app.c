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

void print_line(Line* line) {
    if(line->content == NULL){
        puts("Line content is null.");
        return;
    }
    printf("%s\n", line -> content);
}

void print_line_list(App* app) {
    for (int i = 0; i < app -> line_list -> len; i++) {
        print_line(&app -> line_list -> lines[i]);
    }
}




// TODO (oliver): put this in the lines.c file
LineList* new_line_list_from_file(char* filename) {
    FILE* file = fopen(filename, "r");
    char buf[MAX_LINE_SIZE];
    LineList* line_list = new_line_list();

    while (fgets(buf, sizeof(buf), file)){
        append_to_line_list(line_list, new_line(buf));
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

// todo add this to lines.c
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


void app_go_to_cursor(App* app){
    int cursor_col = app -> cursor -> col;
    int cursor_row = app -> cursor -> row;
    printf("\033[%d;%dH", cursor_row + 1 ,cursor_col + 1);
    fflush(stdout);
}
void app_cursor_up(App* app) {
  if(app -> cursor -> row != 0) {
    app->cursor->row--;
    int col = app->cursor->col;
    int line_len = strlen(app->line_list->lines[app->cursor->row].content);
    app->cursor->col = col > line_len ? line_len : col;
    app_go_to_cursor(app);
  }
}

void app_cursor_left(App* app) {
    if(app -> cursor -> col != 0) {
        app->cursor->col--;
        terminal_cursor_left();
    }
}

void app_cursor_right(App* app) {
    int current_row = app -> cursor -> row;
    char* current_line = app -> line_list -> lines[current_row].content;
    int current_line_len = strlen(current_line);

    if(app -> cursor -> col == current_line_len ){
        return;
    }
    app -> cursor -> col++;
    terminal_cursor_right();
}

void app_cursor_down(App* app) {
    if(app -> cursor -> row == app -> line_list -> len - 1){
        return;
    }
    app -> cursor -> row++;
    int col = app->cursor->col;
    int line_len = strlen(app->line_list->lines[app->cursor->row].content);
    app->cursor->col = col > line_len ? line_len : col;
    app_go_to_cursor(app);
}
// TODO(oliver): Cursor can still go out of bounds, when going up/down w/ different sized lines
void handle_move(App* app, Direction dir) {
    if(dir == UP){
        app_cursor_up(app);
    }
    else if(dir == DOWN){
        app_cursor_down(app);
    }
    else if(dir == LEFT){
        app_cursor_left(app);
    }
    else if(dir == RIGHT){
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
}
void rerender_lines_cursor_down(App* app) {
    int cursor_col = app -> cursor -> col;
    int cursor_row = app -> cursor -> row;
    clear_lines_from_cursor_down();
    clear_line();
    printf("\033[0E");
    for(int i = app -> cursor -> row + 1; i < app -> line_list->len; i++) {
        print_line(&app->line_list->lines[i]);
    }
  
    app -> cursor -> col = 0;
    printf("\033[%d;1H", cursor_row + 1);
    fflush(stdout);
    // printf("\003");
    
}
void handle_edit(App* app, char buf[]) {

    int cursor_col = app -> cursor -> col;
    int cursor_row = app -> cursor -> row;
    // if it is a backspace
    if(0x7F == buf[0]){
        delete_line_char_at(&app -> line_list -> lines[cursor_row], cursor_col);
        if(strcmp(app -> line_list -> lines[cursor_row].content, "") == 1) {
            remove_from_line_list(app -> line_list, cursor_row);
        }
        rerender_line(app);
        app_cursor_left(app);
    } else if('\n' == buf[0]){ // if it is an Enter
        Line* line = new_line("");
        insert_into_line_list(app->line_list, line, cursor_row);
        rerender_lines_cursor_down(app);
    } else {
        // This presumes only ASCII characters
        add_char_to_line_at(&app -> line_list -> lines[cursor_row], cursor_col, buf[0]);
        rerender_line(app);
        app_cursor_right(app);
    }
    fflush(stdout);
}
void handle_save(App* app){
    save_lines_to_file(app->line_list, "file.txt");
}

void handle_input(App* app, char buf[])
{
    int len = strlen(buf);
    if(len == 3) {
        Direction dir = convert_to_direction(buf);
        handle_move(app, dir);
    } else if(buf[0] == 23) { // if ctrl+w was pressed
        handle_save(app);
    } else {
        handle_edit(app, buf);
    }
}

