#include "buffer.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define LINE_INIT_CAPACITY 1024

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

void line_insert_text_before_cursor(Line* line, const char *text, size_t col){
    size_t textSize = strlen(text);
    line_grow(line, textSize);

    // moving the existing content 
    memmove(line->chars + col + textSize, 
            line->chars + col, 
            line->size - col);

    // prepend text
    memcpy(line->chars + col, text, textSize);    
    line->size += textSize;
}

void line_backspace(Line* line, size_t col){
    if(col > 0 && line->size > 0){
        memmove(line->chars + col - 1, 
                line->chars + col, 
                line->size - col);
        line->size--;
    }
}

void line_delete(Line* line, size_t col){
    if(col < line->size && line->size > 0){
        memmove(line->chars + col, 
                line->chars + col + 1, 
                line->size - col);
        line->size--;
    }
}

