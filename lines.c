#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define LINES_C

typedef struct {
    char* content;
} Line;

typedef struct {
    Line* lines;
    int len;
    int max_capacity;
}LineList;

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

LineList* new_line_list() {
    LineList* line_list = calloc(1, sizeof(LineList));

    line_list -> len = 0; 
    line_list -> lines = calloc(5, sizeof(Line)); 
    line_list -> max_capacity = 5; 
    
    return line_list;
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
    strcpy(line -> content, content);
    strtrim(line->content);
    return line;
}
void increase_line_list_capacity(LineList* line_list){
    int new_size = line_list -> len + 5;
    Line* temp_line = realloc(line_list -> lines, sizeof(Line) * new_size);
    if (temp_line == NULL) {
        puts("Failed to reallocate memory for lines.");
        exit(EXIT_FAILURE);
    }
    line_list -> lines = temp_line;
    line_list -> max_capacity = new_size;
}

void insert_into_line_list(LineList* line_list, Line* line, int insert_position) {
    if(insert_position < 0 || insert_position > line_list->len){
        puts("Could not insert into position, index out of bounds");
        exit(-1);        
    }
    if (line_list->len == line_list -> max_capacity) {
        increase_line_list_capacity(line_list);
    }
    for (int i = line_list -> len; i > insert_position; i--) {
        line_list->lines[i] = line_list->lines[i - 1];
    }

    line_list->lines[insert_position] = *line;
    line_list->len++;

}

void append_to_line_list(LineList* line_list, Line* line) {
    line_list -> lines[line_list -> len] = *line;
    line_list -> len++;
    if (line_list -> len == line_list -> max_capacity) {
        increase_line_list_capacity(line_list);
    }
}

void remove_from_line_list(LineList* line_list, int line_index){
    if (line_list -> len == 0) {
        puts("Error removing line. line list is empty!");        
        return;
    } else if (line_index >= line_list -> len || line_index < 0){
        puts("Error removing line. line index is out of bounds!");        
        return;
    }

    free(line_list->lines[line_index].content);
    for(int i = line_index; i < line_list -> len - 1; i++) {
        line_list -> lines[i] = line_list->lines[i + 1];
    }
    line_list-> lines[line_list -> len - 1].content = NULL; // not necessary, but let's keep it safe
    line_list -> len--;
}

void delete_line_char_at(Line* line, int char_index) {
    int line_len = strlen(line -> content);
    if(char_index >= line_len) {
        return;
    }
    for(int i = char_index; i < line_len - 1; i++) {
        line -> content[i] = line -> content[i + 1];
    }
    line->content[line_len - 1] = '\0';
}

void save_lines_to_file(LineList* line_list, char* filename){
    FILE* file = fopen(filename, "w");
    // puts("Saving line to file...");
    for(int i = 0; i < line_list -> len; i++) {
        fprintf(file, line_list -> lines[i].content);
        fprintf(file, "\n");
    }
    // puts("line saved to file!");
    fclose(file);
}
