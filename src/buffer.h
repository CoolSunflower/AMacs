#ifndef BUFFER_H_
#define BUFFER_H_

#include <stdio.h>

typedef struct {
    char *chars;
    size_t size;
    size_t capacity;
} Line;

void line_insert_text_before_cursor(Line* line, const char *text, size_t col);
void line_backspace(Line* line, size_t col);
void line_delete(Line* line, size_t col);

#endif // BUFFER_H_