#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>

#include<windows.h>
#include<SDL.h>

#define STB_IMAGE_IMPLEMENTATION
#include "./src/stb/stb_image.h"

#include "./src/la/la.h"

#define FONT_WIDTH 128
#define FONT_HEIGHT 64
#define FONT_ROWS 7
#define FONT_COLS 18
#define FONT_CHAR_WIDTH (FONT_WIDTH / FONT_COLS)
#define FONT_CHAR_HEIGHT (FONT_HEIGHT / FONT_ROWS)
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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    (void) hInstance;
    (void) hPrevInstance;
    (void) lpCmdLine;
    (void) nShowCmd;
    return main(0, NULL);
}

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

char buffer[BUFFER_CAPACITY];
size_t bufferCursor = 0;
size_t bufferSize = 0;


void renderCursor(SDL_Renderer *renderer, Font *font){
    const Vec2f pos = vec2f(floorf(bufferCursor*FONT_CHAR_WIDTH*FONT_SCALE), 0.0);

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
    if (bufferCursor < bufferSize){
        renderChar(renderer, font, buffer[bufferCursor], pos, FONT_SCALE);
    }
}

void bufferInsertTextBeforeCursor(const char *text){
    size_t textSize = strlen(text);
    const size_t freeSpace = BUFFER_CAPACITY - bufferSize;
    if (textSize > freeSpace){
        textSize = freeSpace;
    }
    // moving the existing content 
    memmove(buffer + bufferCursor + textSize, buffer + bufferCursor, bufferSize-bufferCursor);
    memcpy(buffer + bufferCursor, text, textSize);
    bufferSize += textSize;
    bufferCursor += textSize;
}

void bufferBackspace(void){
    if(bufferCursor > 0 && bufferSize > 0){
        memmove(buffer + bufferCursor - 1, buffer + bufferCursor, bufferSize - bufferCursor);
        bufferSize--;
        bufferCursor--;
    }
}

void bufferDelete(void){
    if(bufferCursor > 0 && bufferSize > 0 && bufferCursor != bufferSize){
        memmove(buffer + bufferCursor, buffer + bufferCursor + 1, bufferSize - bufferCursor - 1);
        bufferSize--;
    }
}

int main(int argc, char *argv[]) {
    printf("Hello\n");
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
                        case SDLK_BACKSPACE : {
                            bufferBackspace();
                            break;
                        }
                        case SDLK_DELETE : {
                            bufferDelete();
                            break;
                        }
                        case SDLK_LEFT : {
                            if (bufferCursor > 0) {
                                bufferCursor--;
                            }
                            break;
                        }
                        case SDLK_RIGHT : {
                            if (bufferCursor < bufferSize) {
                                bufferCursor++;
                            }
                        }
                    }
                    break;
                }

                case SDL_TEXTINPUT : {
                    bufferInsertTextBeforeCursor(event.text.text);
                } break;
            }
        }

        scc(SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0));
        scc(SDL_RenderClear(renderer));

        renderTextSized(renderer, &font, buffer, vec2f(0.0, 0.0), 0xFFFFFFFF, FONT_SCALE, bufferSize);
        renderCursor(renderer, &font);

        SDL_RenderPresent(renderer);
    }

    SDL_Quit();
    return 0;
}
