#ifndef EDITOR_H_
#define EDITOR_H_

#include <stdio.h>

typedef struct {
    char *chars;
    size_t size;
    size_t capacity;
} Line;

void line_insert_text_before_cursor(Line* line, const char *text, size_t col);
void line_backspace(Line* line, size_t col);
void line_delete(Line* line, size_t col);

typedef struct {
    size_t capacity;
    size_t size;
    Line* lines;

    size_t cursor_row;
    size_t cursor_col;
} Editor;

#endif // EDITOR_H_