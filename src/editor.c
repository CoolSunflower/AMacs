#include "editor.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>

#define LINE_INIT_CAPACITY 1024
#define EDITOR_INIT_CAPACITY 128

static void line_grow(Line* line, size_t n){
    // n is the required new space
    // increase capacity as long as needed to fit n

    size_t newCapacity = line->capacity;
    assert(newCapacity >= line->size); // to check proper initialisation

    while (newCapacity - line->size < n){ // check free space versus required space
        if(newCapacity == 0){
            newCapacity = LINE_INIT_CAPACITY;
        }else{
            newCapacity *= 2;
        }
    }

    if(newCapacity != line->capacity){
        line->chars = realloc(line->chars, newCapacity);
        line->capacity = newCapacity;
    }
}

void line_insert_text_before_cursor(Line* line, const char *text, size_t *col){
    if (*col > line->size){
        *col = line->size;
    }

    size_t textSize = strlen(text);
    line_grow(line, textSize);

    // moving the existing content 
    memmove(line->chars + *col + textSize, 
            line->chars + *col, 
            line->size - *col);

    // prepend text
    memcpy(line->chars + *col, text, textSize);    
    line->size += textSize;
    *col += textSize;
}

void line_backspace(Line* line, size_t *col){
    if (*col > line->size){
        *col = line->size;
    }

    if(*col > 0 && line->size > 0){
        memmove(line->chars + *col - 1, 
                line->chars + *col, 
                line->size - *col);
        line->size--;
        *col = *col - 1;
    }
}

void line_delete(Line* line, size_t *col){
    if (*col > line->size){
        *col = line->size;
    }

    if(*col < line->size && line->size > 0){
        memmove(line->chars + *col, 
                line->chars + *col + 1, 
                line->size - *col);
        line->size--;
    }
}

static void editor_grow(Editor* editor, size_t n){
    // n is the required new space
    // increase capacity as long as needed to fit n

    size_t newCapacity = editor->capacity;
    assert(newCapacity >= editor->size); // to check proper initialisation

    while (newCapacity - editor->size < n){ // check free space versus required space
        if(newCapacity == 0){
            newCapacity = EDITOR_INIT_CAPACITY;
        }else{
            newCapacity *= 2;
        }
    }

    if(newCapacity != editor->capacity){
        editor->lines = realloc(editor->lines, newCapacity * sizeof(Line));
        editor->capacity = newCapacity;
    }
}

void editor_push_new_line(Editor* editor){
    editor_grow(editor, 1);
    memset(&editor->lines[editor->size], 0, sizeof(editor->lines[0]));
    editor->size += 1;
}

void editor_insert_text_before_cursor(Editor* editor, const char *text){
    if(editor->cursor_row >= editor->size){
        if(editor->size > 0){
            editor->cursor_row = editor->size - 1;
        }else{ // editor->size == 0
            editor_push_new_line(editor);
        }
    }

    line_insert_text_before_cursor(&editor->lines[editor->cursor_row], text, &editor->cursor_col);
}

void editor_backspace(Editor* editor){
    if(editor->cursor_row >= editor->size){
        if(editor->size > 0){
            editor->cursor_row = editor->size - 1;
        }else{ // editor->size == 0
            editor_push_new_line(editor);
        }
    }

    line_backspace(&editor->lines[editor->cursor_row], &editor->cursor_col);
}

void editor_delete(Editor* editor){
    if(editor->cursor_row >= editor->size){
        if(editor->size > 0){
            editor->cursor_row = editor->size - 1;
        }else{ // editor->size == 0
            editor_push_new_line(editor);
        }
    }

    line_delete(&editor->lines[editor->cursor_row], &editor->cursor_col);
}

void editor_insert_new_line(Editor* editor){
    if (editor->cursor_row > editor->size){
        editor->cursor_row = editor->size;
    }

    editor_grow(editor, 1);

    // moving the existing content 
    const size_t line_size = sizeof(editor->lines[0]);
    memmove(editor->lines + editor->cursor_row + 1, 
            editor->lines + editor->cursor_row, 
            (editor->size - editor->cursor_row)*line_size);
    memset(&editor->lines[editor->cursor_row + 1], 0, line_size);
    editor->cursor_row += 1;
    editor->cursor_col = 0;
    editor->size += 1;
}

const char *editor_char_under_cursor(const Editor* editor){
    if (editor->cursor_row < editor->size){
        if(editor->cursor_col < editor->lines[editor->cursor_row].size){
            return &editor->lines[editor->cursor_row].chars[editor->cursor_col];
        }
    }

    return NULL;
}

void editor_save_to_file(const Editor* editor, const char* filePath){
    FILE* f = fopen(filePath, "w");
    if (f == NULL){
        fprintf(stdout, "ERROR: Could not open file %s: %s\n", filePath, strerror(errno));
        exit(1);
    }

    for(size_t row = 0; row < editor->size; ++row){
        fwrite(editor->lines[row].chars, 1, editor->lines[row].size, f);
        fputc('\n', f);
    }

    fclose(f);
}