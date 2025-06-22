#include "editor.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include "./lib/sv.h"

#define LINE_INIT_CAPACITY 1024
#define EDITOR_INIT_CAPACITY 128

static void editor_create_first_new_line(Editor* editor);

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

void line_append_text(Line* line, const char* text){
    line_append_text_sized(line, text, strlen(text));
}

void line_append_text_sized(Line* line, const char* text, size_t text_size){
    size_t col = line->size;
    line_insert_text_before_cursor_sized(line, text, &col, text_size);
}

void line_insert_text_before_cursor_sized(Line* line, const char* text, size_t* col, size_t text_size){
    if (*col > line->size){
        *col = line->size;
    }

    line_grow(line, text_size);

    // moving the existing content 
    memmove(line->chars + *col + text_size, 
            line->chars + *col, 
            line->size - *col);

    // prepend text
    memcpy(line->chars + *col, text, text_size);    
    line->size += text_size;
    *col += text_size;
}

void line_insert_text_before_cursor(Line* line, const char *text, size_t *col){
    line_insert_text_before_cursor_sized(line, text, col, strlen(text));
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

// static void editor_push_new_line(Editor* editor){
// }

static void editor_create_first_new_line(Editor* editor){
    if(editor->cursor_row >= editor->size){
        if(editor->size > 0){
            editor->cursor_row = editor->size - 1;
        }else{ // editor->size == 0
            editor_grow(editor, 1);
            memset(&editor->lines[editor->size], 0, sizeof(editor->lines[0]));
            editor->size += 1;
        }
    }
}

void editor_insert_text_before_cursor(Editor* editor, const char *text){
    editor_create_first_new_line(editor);
    line_insert_text_before_cursor(&editor->lines[editor->cursor_row], text, &editor->cursor_col);
}

void editor_backspace(Editor* editor){
    editor_create_first_new_line(editor);
    line_backspace(&editor->lines[editor->cursor_row], &editor->cursor_col);
}

void editor_delete(Editor* editor){
    editor_create_first_new_line(editor);
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

void editor_load_from_file(Editor* editor, const char* filePath){
    assert(editor->lines == NULL && "You can only load files into an empty editor");
    editor_create_first_new_line(editor);

    FILE* f = fopen(filePath, "r");
    if(f == NULL){
        fprintf(stderr, "Error: could not open file %s: %s", filePath, strerror(errno));
    }

    static char chunk[1024 * 640];

    while(!feof(f)){
        size_t n = fread(chunk, 1, sizeof(chunk), f);

        String_View chunk_sv = {
            .data = chunk,
            .count = n
        };

        while(chunk_sv.count > 0){
            String_View chunk_line = {0};
            Line *line = &editor->lines[editor->size - 1];
            if(sv_try_chop_by_delim(&chunk_sv, '\n', &chunk_line)){
                line_append_text_sized(line, chunk_line.data, chunk_line.count);
                editor_insert_new_line(editor);
            }else {
                line_append_text_sized(line, chunk_line.data, chunk_line.count);
                chunk_sv = SV_NULL;
            }
        }
    }

    editor->cursor_row = 0;

    fclose(f);
}