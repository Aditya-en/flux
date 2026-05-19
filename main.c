#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define INITIAL_SIZE 10

// --- STRUCTS ---

typedef struct {
    char *dynamic_buffer;
    int current_size;
    int gap;
    int cursor;
} Line;

typedef struct {
    Line *lines; // Dynamic array of lines
    int num_lines;
    int active_line_index; // Tracks which line the cursor is currently on
} Text;

// --- RAW MODE ---

struct termios original_termios;

void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &original_termios);
    struct termios raw = original_termios;
    raw.c_lflag &= ~(ECHO | ICANON);  // no echo, read char-by-char
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
}

// --- INITIALIZATION ---

void init_line(Line *line) {
    line->current_size = INITIAL_SIZE;
    line->gap = INITIAL_SIZE;
    line->cursor = 0;
    line->dynamic_buffer = (char *)malloc(INITIAL_SIZE);
    for(int i = 0; i < INITIAL_SIZE; i++) {
        line->dynamic_buffer[i] = 0;
    }
}

void init_text(Text *text, int starting_lines) {
    text->num_lines = starting_lines;
    text->active_line_index = 0;
    text->lines = (Line *)malloc(sizeof(Line) * starting_lines);
    for(int i = 0; i < starting_lines; i++) {
        init_line(&text->lines[i]);
    }
}


// --- LINE OPERATIONS ---

void resize_buffer(Line *line) {
    int old_size = line->current_size;
    line->current_size = line->current_size * 2;
    char *new_buffer = (char *)malloc(line->current_size);

    for(int i = 0; i < line->current_size; i++) {
        new_buffer[i] = 0; 
    }

    for(int i = 0; i < line->cursor; i++) {
        new_buffer[i] = line->dynamic_buffer[i];
    }

    int right_text_length = old_size - line->cursor;
    int new_right_start = line->current_size - right_text_length;

    for(int i = 0; i < right_text_length; i++) {
        // Fixed: Used line->cursor instead of global cursor
        new_buffer[new_right_start + i] = line->dynamic_buffer[line->cursor + i]; 
    }

    line->gap = line->current_size - old_size;

    free(line->dynamic_buffer);
    line->dynamic_buffer = new_buffer;
}

void write_char(char c, Line *line){
    if (line->gap == 0) {
        resize_buffer(line); // Fixed: Removed the '&'
    }
    line->dynamic_buffer[line->cursor] = c;
    line->cursor++;
    line->gap--;
}

void erase_char(Line *line) {
    if (line->cursor > 0) {
        line->dynamic_buffer[line->cursor-1] = 0;
        line->cursor--;
        line->gap++;
    }
}

void move_left(Line *line){
    if (line->cursor > 0) {
        line->dynamic_buffer[line->cursor + line->gap - 1] = line->dynamic_buffer[line->cursor - 1];
        line->dynamic_buffer[line->cursor - 1] = 0; 
        line->cursor--;
    }
}

void move_right(Line *line){
    if (line->cursor < line->current_size - line->gap){
        line->dynamic_buffer[line->cursor] = line->dynamic_buffer[line->cursor + line->gap];
        line->dynamic_buffer[line->cursor + line->gap] = 0; 
        line->cursor++;
    }
}


// --- VERTICAL NAVIGATION ---

void move_up(Text *text){
    if (text->active_line_index > 0) {
        Line *current_line = &text->lines[text->active_line_index];
        int desired_col = current_line->cursor; // Remember where we are horizontally

        text->active_line_index--; // Move up
        
        Line *target_line = &text->lines[text->active_line_index];
        
        // Prevent moving past the end of the line above us if it's shorter
        int max_col = target_line->current_size - target_line->gap;
        int target_col = MIN(desired_col, max_col);

        // Shift the gap in the new line so the cursor aligns properly
        while (target_line->cursor < target_col) {
            move_right(target_line);
        }
        while (target_line->cursor > target_col) {
            move_left(target_line);
        }
    }
}

void move_down(Text *text){
    if (text->active_line_index < text->num_lines - 1) {
        Line *current_line = &text->lines[text->active_line_index];
        int desired_col = current_line->cursor;

        text->active_line_index++; // Move down
        
        Line *target_line = &text->lines[text->active_line_index];
        
        int max_col = target_line->current_size - target_line->gap;
        int target_col = MIN(desired_col, max_col);

        while (target_line->cursor < target_col) {
            move_right(target_line);
        }
        while (target_line->cursor > target_col) {
            move_left(target_line);
        }
    }
}

// --- RENDER ---

void render(Text *text) {
    // Clear screen, move cursor to top-left
    printf("\033[2J\033[H");

    for (int l = 0; l < text->num_lines; l++) {
        Line *line = &text->lines[l];
        int content_len = line->current_size - line->gap;

        // Print chars before the gap
        for (int i = 0; i < line->cursor; i++)
            putchar(line->dynamic_buffer[i]);

        // Print chars after the gap
        int right_start = line->cursor + line->gap;
        for (int i = right_start; i < line->current_size; i++)
            putchar(line->dynamic_buffer[i]);

        putchar('\n');
    }

    // Move terminal cursor to match editor cursor position
    Line *al = &text->lines[text->active_line_index];
    printf("\033[%d;%dH", text->active_line_index + 1, al->cursor + 1);
    fflush(stdout);
}
// --- INPUT LOOP ---

void run(Text *text) {
    enable_raw_mode();

    while (1) {
        render(text);

        char c;
        read(STDIN_FILENO, &c, 1);

        if (c == 17) break;  // Ctrl-Q to quit

        if (c == 127) {      // Backspace
            erase_char(&text->lines[text->active_line_index]);
        } else if (c == '\033') {
            // Escape sequence — arrow keys send ESC [ A/B/C/D
            char seq[2];
            read(STDIN_FILENO, &seq[0], 1);
            read(STDIN_FILENO, &seq[1], 1);
            if (seq[0] == '[') {
                if      (seq[1] == 'A') move_up(text);
                else if (seq[1] == 'B') move_down(text);
                else if (seq[1] == 'C') move_right(&text->lines[text->active_line_index]);
                else if (seq[1] == 'D') move_left(&text->lines[text->active_line_index]);
            }
        } else if (c >= 32 && c < 127) {  // Printable ASCII
            write_char(c, &text->lines[text->active_line_index]);
        }
    }

    disable_raw_mode();
    printf("\033[2J\033[H");  // Clear on exit
}
// --- DEBUGGING ---

void print_text(Text *text) {
    printf("\n--- TEXT DOCUMENT ---\n");
    for(int l = 0; l < text->num_lines; l++) {
        Line *line = &text->lines[l];
        
        // Print an arrow next to the active line
        if (l == text->active_line_index) printf("-> ");
        else printf("   ");
        
        printf("Line %d [ ", l);
        for(int i = 0; i < line->current_size; i++){
            if (i >= line->cursor && i < line->cursor + line->gap) {
                printf("_ ");
            } else {
                if (line->dynamic_buffer[i] == 0) printf(". "); 
                else printf("%c ", line->dynamic_buffer[i]);
            }
        }
        printf("]\n");
    }
    printf("---------------------\n");
}


// --- MAIN TEST ---
int main() {
    Text editor;
    init_text(&editor, 10);

    run(&editor);

    for (int i = 0; i < editor.num_lines; i++) free(editor.lines[i].dynamic_buffer);
    free(editor.lines);
    return 0;
}