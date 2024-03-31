static struct termios config, original_config;


void reset_terminal() {
    printf("\e[m"); // Reset the colors
    printf("\e[?25h"); // Show the cursor
    fflush(stdout); // Just to make sure everything gets applied
    tcsetattr(STDIN_FILENO, TCSANOW, &original_config); // Restore original settings
}

void configure_terminal() {
    tcgetattr(STDIN_FILENO, &original_config); // Save the original for later
    tcgetattr(STDIN_FILENO, &config);
    
    config.c_lflag &= ~(ICANON | ECHO); // Turn canonical mode & echo off 
    tcsetattr(STDIN_FILENO, TCSANOW, &config); // Set the terminal attributes
    // printf("\e[?25l]"); // Hide the cursor via an ANSI Escape Code

    atexit(reset_terminal); // When the program terminates it should reset the terminal to the original state
}

char arrow_to_str(char buf[]) {
    return buf[strlen(buf) - 1]; 
}

void clear_screen() {
    printf("\033[2J"); // Clear the screen
    // printf("\033\033[H"); // Move cursor to the back "HOME"
}

void clear_line_from_cursor_right() {
    printf("\033[0K");
    fflush(stdout);
}

void clear_line_from_cursor_left() {
    printf("\033[1K");
    fflush(stdout);
}

void clear_line() {
    printf("\033[2K");
}

void clear_lines_from_cursor_down(){
    printf("\033[0J");
    fflush(stdout);
}
void clear_lines_from_cursor_up(){
    printf("\033[1J");
    fflush(stdout);
}
// ANSI Escape Codes for Arrow keys.
// All of these are 3 bytes long.

#define CURSOR_UP_CMD "\033[A"
#define CURSOR_DOWN_CMD "\033[B"
#define CURSOR_RIGHT_CMD "\033[C"
#define CURSOR_LEFT_CMD "\033[D"

void terminal_cursor_up() {
    printf("%s", CURSOR_UP_CMD);
    fflush(stdout);
}
void terminal_cursor_down() {
    printf("%s", CURSOR_DOWN_CMD);
    fflush(stdout);
}
void terminal_cursor_left() {
    printf("%s", CURSOR_LEFT_CMD);
    fflush(stdout);

}
void terminal_cursor_right() {
    printf("%s", CURSOR_RIGHT_CMD);
    fflush(stdout);
}


void write_at_cursor(char buf[]){
    printf("%s", buf);
    fflush(stdout);
}