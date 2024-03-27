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
}

void clear_line_from_cursor_left() {
    printf("\033[1K");
}

void clear_line() {
    printf("\033[2K");
}