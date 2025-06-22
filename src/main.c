#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<assert.h>

#include<windows.h>
#include <SDL2/SDL.h>
#define STB_IMAGE_IMPLEMENTATION
#include "../dependencies/stb/stb_image.h"

#include "la.h"
#include "editor.h"

#define FONT_WIDTH 128
#define FONT_HEIGHT 64
#define FONT_ROWS 7
#define FONT_COLS 18
float FONT_CHAR_WIDTH = (float)FONT_WIDTH / (float)FONT_COLS;
float FONT_CHAR_HEIGHT = (float) FONT_HEIGHT / (float) FONT_ROWS;
#define FONT_SCALE 5.0f

void scc(int code){
    if (code < 0){
        fprintf(stderr, "SDL ERROR: %s\n", SDL_GetError());
        exit(1);
    }
}

void *scp(void *ptr){
    if(ptr == NULL){
        fprintf(stderr, "SDL ERROR: %s\n", SDL_GetError());
        exit(1);
    }
    return ptr;
}

// int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
//     (void) hInstance;
//     (void) hPrevInstance;
//     (void) lpCmdLine;
//     (void) nShowCmd;
//     return main(0, NULL);
// }

SDL_Surface * surfaceFromFile(const char *filePath){
    int width, height, n;
    unsigned char *pixel = stbi_load(filePath, &width, &height, &n, STBI_rgb_alpha);
    if (pixel==NULL){
        fprintf(stderr, "ERROR: could not load file %s: %s\n", filePath, stbi_failure_reason());
        exit(1);
    }

    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
        const Uint32 rmask = 0xff000000;
        const Uint32 gmask = 0x00ff0000;
        const Uint32 bmask = 0x0000ff00;
        const Uint32 amask = 0x000000ff;
    #else 
        const Uint32 rmask = 0x000000ff;
        const Uint32 gmask = 0x0000ff00;
        const Uint32 bmask = 0x00ff0000;
        const Uint32 amask = 0xff000000;
    #endif

    const int depth = 32;
    const int pitch = 4*width;

    return scp(SDL_CreateRGBSurfaceFrom((void*)pixel, width, height, depth, pitch, rmask, gmask, bmask, amask));
} 

#define ASCII_DISPLAY_LOW 32
#define ASCII_DISPLAY_HIGH 126

typedef struct {
    SDL_Texture *spritesheet;
    SDL_Rect glyphTable[ASCII_DISPLAY_HIGH - ASCII_DISPLAY_LOW + 1];
} Font;

Font loadFontFromFile(const char* filePath, SDL_Renderer *renderer){
    Font font = {0};
    SDL_Surface *fontSurface = scp(surfaceFromFile(filePath));
    // after getting the surface you need to set the surface color key i.e. define the background color so that you can get transparent glyphs
    scc(SDL_SetColorKey(fontSurface, SDL_TRUE, 0xFF000000));
    font.spritesheet = scp(SDL_CreateTextureFromSurface(renderer, fontSurface));
    SDL_FreeSurface(fontSurface);

    for(size_t ascii = ASCII_DISPLAY_LOW; ascii <= ASCII_DISPLAY_HIGH; ascii++){
        const size_t index = ascii - ASCII_DISPLAY_LOW;
        const size_t row = index/FONT_COLS;
        const size_t col = index%FONT_COLS;
        SDL_Rect temp = {
            .x = col * FONT_CHAR_WIDTH,
            .y = row * FONT_CHAR_HEIGHT,
            .w = FONT_CHAR_WIDTH,
            .h = FONT_CHAR_HEIGHT
        };
        font.glyphTable[index] = temp;
    }

    return font;
}

void setTextureColor(Font *font, Uint32 color){
    scc(SDL_SetTextureColorMod(font->spritesheet, (color >> (8*0)) & 0xff,(color >> (8*1)) & 0xff,(color >> (8*2)) & 0xff));
    scc(SDL_SetTextureAlphaMod(font->spritesheet, (color >> (8*3))&0xff));
}

void renderChar(SDL_Renderer *renderer, Font *font, char c, Vec2f pos, float scale){
    assert(c >= ASCII_DISPLAY_LOW);
    assert(c <= ASCII_DISPLAY_HIGH);
    const SDL_Rect dest = {
        .x = (int) floorf(pos.x),
        .y = (int) floorf(pos.y),
        .w = (int) floorf(scale * FONT_CHAR_WIDTH),
        .h = (int) floorf(scale * FONT_CHAR_HEIGHT)
    };
    scc(SDL_RenderCopy(renderer, font->spritesheet, &font->glyphTable[c - ASCII_DISPLAY_LOW], &dest));
}

void renderTextSized(SDL_Renderer *renderer, Font *font, const char *text, Vec2f pos, Uint32 color, float scale, size_t textSize){
    Vec2f pen = pos;

    setTextureColor(font, color);

    for(size_t i = 0; i < textSize; i++){
        renderChar(renderer, font, text[i], pen, scale);
        pen.x += FONT_CHAR_WIDTH*scale;
    }
}

void renderText(SDL_Renderer *renderer, Font *font, const char *text, Vec2f pos, Uint32 color, float scale){
    renderTextSized(renderer, font, text, pos, color, scale, strlen(text));
}

#define BUFFER_CAPACITY 1024
#define UNHEX(color) \
    ((color) >> (8*0)) & 0xFF, \
    ((color) >> (8*1)) & 0xFF, \
    ((color) >> (8*2)) & 0xFF, \
    ((color) >> (8*3)) & 0xFF

Editor editor = {0};

// char buffer[BUFFER_CAPACITY];
// size_t bufferCursor = 0;
// size_t bufferSize = 0;


void renderCursor(SDL_Renderer *renderer, Font *font){
    const Vec2f pos = vec2f(floorf(editor.cursor_col*FONT_CHAR_WIDTH*FONT_SCALE), 
                            (float)editor.cursor_row*FONT_CHAR_HEIGHT*FONT_SCALE);

    const SDL_Rect rect = {
        .x = (int) floorf(pos.x),
        .y = (int) floorf(pos.y),
        .w = FONT_CHAR_WIDTH * FONT_SCALE,
        .h = FONT_CHAR_HEIGHT * FONT_SCALE
    };

    // draw the cursor
    scc(SDL_SetRenderDrawColor(renderer, UNHEX(0xFFFFFFFF)));
    scc(SDL_RenderFillRect(renderer, &rect));

    // render the char at that position
    setTextureColor(font, 0xFF000000);
    const char *c = editor_char_under_cursor(&editor);
    if(c){
        renderChar(renderer, font, *c, pos, FONT_SCALE);
    }
}

int main(int argc, char *argv[]) {
    scc(SDL_Init(SDL_INIT_VIDEO));

    SDL_Window *window = scp(SDL_CreateWindow("AMacs",20,20,800,600,SDL_WINDOW_RESIZABLE));
    SDL_Renderer *renderer = scp(SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED));

    // to enable drawing rectangles with transparency
    scc(SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND));

    Font font = loadFontFromFile("./assets/charmap-oldschool_white.png", renderer);

    bool quit = false;
    while (!quit){
        SDL_Event event = {0};

        while (SDL_PollEvent(&event)){
            switch (event.type){
                case SDL_QUIT : {
                    quit=true;
                } break;

                case SDL_KEYDOWN : {
                    switch (event.key.keysym.sym){
                        case SDLK_F8: {
                            editor_save_to_file(&editor, "output");
                            break;
                        }
                        case SDLK_BACKSPACE : {
                            editor_backspace(&editor);
                            break;
                        }
                        case SDLK_RETURN: {
                            editor_insert_new_line(&editor);
                            break;
                        }
                        case SDLK_DELETE : {
                            editor_delete(&editor);
                            break;
                        }
                        case SDLK_UP: {
                            if (editor.cursor_row > 0){
                                editor.cursor_row -= 1;
                            }
                            break;
                        }
                        case SDLK_DOWN: {
                            editor.cursor_row += 1;
                            break;
                        }
                        case SDLK_LEFT : {
                            if (editor.cursor_col > 0) {
                                editor.cursor_col--;
                            }
                            break;
                        }
                        case SDLK_RIGHT : {
                            editor.cursor_col++;
                        }
                    }
                    break;
                }

                case SDL_TEXTINPUT : {
                    editor_insert_text_before_cursor(&editor, event.text.text);
                } break;
            }
        }

        scc(SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0));
        scc(SDL_RenderClear(renderer));

        for(size_t row = 0; row < editor.size; ++row){
            const Line* line = &editor.lines[row];
            renderTextSized(renderer, &font, line->chars, vec2f(0.0, (float)row*FONT_CHAR_HEIGHT*FONT_SCALE), 0xFFFFFFFF, FONT_SCALE, line->size);
        }
        renderCursor(renderer, &font);

        SDL_RenderPresent(renderer);
    }

    SDL_Quit();
    return 0;
}
