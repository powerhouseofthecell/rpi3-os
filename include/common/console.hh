#ifndef CONSOLE_HH
#define CONSOLE_HH

#include "kernel/lfb.hh"
#include "common/types.hh"

// define the dimensions of each character
#define CHAR_WIDTH  8
#define CHAR_HEIGHT 8

// define some useful colors
static const int WHITE = 0xffffff;
static const int BLACK = 0x000000;
static const int BLUE =  0x0000ff;
static const int RED =   0xff0000;
static const int GREEN = 0x00ff00;

class Console {
public:
    uint64_t console_addr = 0x00;

    // the font to use
    psf_t* font = (psf_t*) &_binary_font_psf_start;

    // set default printing colors
    const int text_color = WHITE;
    const int reverse_video = BLACK;

    int char_xpos = 0;
    int char_ypos = 0;

    int bytesPerPixel = 3;
    int bytesPerRow = 1920;

    // initialize a Console object (with the provided console address)
    Console(uint64_t addr);

    // writes the provided pixel to the console @ (x, y)
    void write_pixel(uint32_t x, uint32_t y, const uint8_t* pixel);

    // clear the console
    void clear();

    // put a single character to the console
    // at position (charX, charY) (this is the char position, not the pixel position)
    void putc(int charX, int charY, char c);
    void putc(char c);

    // places a null-terminated string starting at (charX, charY) (position by characters)
    void puts(int charX, int charY, const char* str);
    void puts(const char* str);

    // allows limited printing functionality (prints to the console)
    void printf(const char * fmt, ...);
};

#endif
