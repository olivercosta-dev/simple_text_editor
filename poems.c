
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <ctype.h>

#ifdef _WIN32
#include <conio.h> // For _getch on Windows
#else
#include <termios.h> // For termios on Unix-like systems
#include <unistd.h>
#endif

void enableRawMode() {
#ifndef _WIN32
    struct termios raw;
    tcgetattr(STDIN_FILENO, &raw);
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
#endif
}

void disableRawMode() {
#ifndef _WIN32
    struct termios raw;
    tcgetattr(STDIN_FILENO, &raw);
    raw.c_lflag |= ECHO | ICANON;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
#endif
}

int getch_cross_platform() {
#ifdef _WIN32
    return _getch();
#else
    return getchar();
#endif
}

#define MAX_LINE_SIZE 256

#define UP 'W'
#define DOWN 'S'
#define LEFT 'A'
#define RIGHT 'D'

#define TRUE 1
#define FALSE 0

// WM = Writing Mode
#define WM_EDIT 0
#define WM_MOVE 1

#define WM_KEY_SWITCH '\t'

#define KEY_DELETE_ROW '.'

typedef struct {
    char* content;
} Poem;


typedef struct {
    Poem* poems;
    int len;
    int max_capacity;
}PoemList;

typedef struct {
    int row;
    int col;
} Cursor;

typedef struct {
    PoemList* poem_list;
    Cursor cursor;
    char input_buf[10];
    int writing_mode;
} App;

void clear(){
    #if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
        system("clear");
    #endif

    #if defined(_WIN32) || defined(_WIN64)
        system("cls");
    #endif
}


PoemList* new_poem_list() {
    PoemList* poem_list = calloc(1, sizeof(PoemList));

    poem_list -> len = 0; 
    poem_list -> poems = calloc(5, sizeof(Poem)); 
    poem_list -> max_capacity = 5; 
    
    return poem_list;
}

void add_to_poem_list(PoemList* poem_list, Poem* poem) {
    poem_list -> poems[poem_list -> len] = *poem;
    poem_list -> len++;
    // printf("Poem added:\n%s\nLength increased to: %d\n",poem_list->poems[poem_list -> len - 1].content ,poem_list -> len);
    if (poem_list -> len == poem_list -> max_capacity) {
        int new_size = poem_list -> len + 5;
        Poem* temp_poem = realloc(poem_list -> poems, sizeof(Poem) * new_size);
        if (temp_poem == NULL) {
            puts("Failed to reallocate memory for poems.");
            exit(EXIT_FAILURE);
        }
        poem_list -> poems = temp_poem;
        poem_list -> max_capacity = new_size;
        // printf("Max cap successfully increased to: %d\n", poem_list -> max_capacity);
    }
}

void remove_from_poem_list(PoemList* poem_list, int poem_index){
    if (poem_list -> len == 0) {
        puts("Error removing poem. Poem list is empty!");        
        return;
    } else if (poem_index >= poem_list -> len || poem_index < 0){
        puts("Error removing poem. Poem index is out of bounds!");        
        return;
    }

    free(poem_list->poems[poem_index].content);
    for(int i = poem_index; i < poem_list -> len - 1; i++) {
        poem_list -> poems[i] = poem_list->poems[i + 1];
    }
    poem_list-> poems[poem_list -> len - 1].content = NULL; // not necessary, but let's keep it safe
    poem_list -> len--;
    // printf("Poem removed, current length: %d\n", poem_list -> len);
}

void print_poem(Poem* poem, int with_cursor, int cursor_col){
    if(with_cursor == TRUE) {
        printf("--->");
        for(int i = 0; i < strlen(poem -> content); i++) {
            if(i == cursor_col) {
                printf("|%c", poem -> content[i]);
            }
            else {
                printf("%c", poem -> content[i]);
            }
        }
        printf("<---\n");
    }
    else {
        printf("%s\n", poem -> content);
    }
}

void print_poem_list(PoemList* poem_list, Cursor cursor) {
    for (int i = 0; i < poem_list -> len; i++) {
        if(i == cursor.row) {
            print_poem(&poem_list -> poems[i], TRUE, cursor.col);
        } else {
            print_poem(&poem_list -> poems[i], FALSE, cursor.col);
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

Poem* new_poem(char* content) {
    Poem* poem = malloc(sizeof(Poem));
    if (poem == NULL) {
        puts("Failed to allocated memory for poem!");
        exit(EXIT_FAILURE);
    }

    poem -> content = malloc(strlen(content) + 1);
    if (poem->content == NULL) {
        free(poem);
        puts("Failed to allocated memory for poem content!");
        exit(EXIT_FAILURE);
        return NULL;
    }
    int content_length = strlen(content);
    strcpy(poem -> content, content);
    strtrim(poem->content);
    return poem;
}

PoemList* new_poem_list_from_file(char* filename) {
    FILE* file = fopen(filename, "r");
    char buf[MAX_LINE_SIZE];
    PoemList* poem_list = new_poem_list();

    while (fgets(buf, sizeof(buf), file)){
        add_to_poem_list(poem_list, new_poem(buf));
    }
    fclose(file);
    return poem_list;
}

void save_poems_to_file(PoemList* poem_list, char* filename){
    FILE* file = fopen(filename, "w");
    puts("Saving poem to file...");
    for(int i = 0; i < poem_list -> len; i++) {
        fprintf(file, poem_list -> poems[i].content);
        fprintf(file, "\n");
    }
    puts("Poem saved to file!");
    fclose(file);
}

void modify_poem_content_ascii(Poem* poem, int char_index, char new_char){
    int poem_len = strlen(poem -> content);
    if(poem_len == 0 || char_index >=  poem_len || char_index < 0) {
        puts("Unable to modify poem at that position: Invalid index!");
        //TODO (oliver): maybe don't exit.
        exit(-1);
    }
    poem -> content[char_index] = new_char;
}


void delete_poem_char_at(Poem* poem, int char_index){
    int poem_len = strlen(poem -> content);
    if(char_index >= poem_len) {
        return;
    }
    for(int i = char_index; i < poem_len - 1; i++) {
        poem -> content[i] = poem -> content[i + 1];
    }
    poem->content[poem_len - 1] = '\0';
}

void add_char_to_poem_at(Poem* poem, int char_index, char new_char)
{
    int poem_len = strlen(poem -> content);
    char* modified_poem_content = calloc((poem_len + 2), sizeof(char));
    for(int i = poem_len; i > char_index; i--) {
        modified_poem_content[i] = poem -> content[i - 1];
    }
    modified_poem_content[char_index] = new_char;
    for(int i = char_index - 1; i >= 0; i--) {
        modified_poem_content[i] = poem -> content[i];
    }
    modified_poem_content[poem_len + 1] = '\0';
    free(poem->content);
    poem->content = modified_poem_content;
}

void handle_wm_edit(char input_char, Cursor* cursor, PoemList* poem_list){
    switch (input_char)
    {
        case '.':
            cursor->writing_mode = WM_MOVE;
            break;
        case '\b':
            delete_poem_char_at(&(poem_list->poems[cursor->row]), cursor->col);
            break;
        default:
            add_char_to_poem_at(&(poem_list->poems[cursor->row]), cursor->col, input_char);
            cursor -> col++;
            break;
    }
}
void handle_wm_move(char input_char, Cursor* cursor, PoemList* poem_list){
    switch (toupper(input_char))
    {
            case UP:
            {
                if(cursor -> row > 0)
                    cursor -> row--;
                break;
            }
            case DOWN:
            {
                if(cursor->row < poem_list -> len)
                    cursor->row++;
                break;
            }
            case LEFT:
            {
                if(cursor->col > 0)
                    cursor->col--;
                break;
            }
            case RIGHT:
            {
                if(cursor->col < strlen(poem_list->poems->content))
                    cursor->col++;
                break;
            }
            case '.':
                cursor->writing_mode = WM_EDIT;
                break;
            case '\b':
                remove_from_poem_list(poem_list, cursor->row);
                //TODO handle out of bounds
                break;;
            default:
                break;
    }
}
void handle_input(char input_char, Cursor* cursor, PoemList* poem_list)
{
    switch (cursor -> writing_mode)
        {
            case WM_MOVE:
            {
                handle_wm_move(input_char, cursor, poem_list);
                break;
            }
            case WM_EDIT:
            {
                handle_wm_edit(input_char, cursor, poem_list);
                break;
            }
            default:
                break;
        }
}

App* new_app(PoemList* poem_list) {
    if(poem_list == NULL){
        poem_list = new_poem_list();
    }
} 
Cursor* new_cursor() {
    Cursor* cursor = malloc(sizeof(Cursor));
    cursor -> row = 0;
    cursor -> col = 0;
}
int main(int argc, char* argv[])
{
    //? setlocale(LC_ALL, "hu_HU.UTF-8"); // csak hogy magyar is lehessen :) - de így nem jó mert ascii kell csak!

    if(argc != 2) {
        puts("Incorrect arguments supplied.\nUse: poems filename.txt");
        return 1;
    }

    char* file_name = argv[1];
    PoemList* poem_list = new_poem_list_from_file(file_name);
    App app = new_app(poem_list);
    Cursor cursor;
    cursor.col = 0;
    cursor.row = 0;
    cursor.writing_mode = WM_MOVE;
    char c;
    int current_row = 0;
    while(c = getch_cross_platform()){
        clear();
        handle_input(c, &cursor, poem_list);
        // TODO (oliver): WRITE MODE WITH TAB
        print_poem_list(poem_list, cursor);
        printf("In mode: %d", cursor.writing_mode);
    }
    return 0;
}


