#ifndef CONSOLE_HH
#define CONSOLE_HH

#include "kernel/lfb.hh"
#include "common/types.hh"

// define some useful colors
static const unsigned int WHITE = 0xffffff;
static const unsigned int BLACK = 0x000000;
static const unsigned int BLUE =  0xff0000;
static const unsigned int RED =   0x0000ff;
static const unsigned int GREEN = 0x00ff00;

class Console {
public:
    uint64_t console_addr = 0x00;

    // the font to use
    psf_t* font = (psf_t*) &_binary_font_psf_start;

    // set default printing colors
    int text_color = WHITE;
    int reverse_video = BLACK;

    uint32_t char_xpos = 0;
    uint32_t char_ypos = 0;

    // initialize a Console object (with the provided console address)
    Console();
    Console(uint64_t addr);

    void setTextColor(int color);
    void setReverseColor(int color);

    // writes the provided pixel to the console @ (x, y)
    void write_pixel(uint32_t x, uint32_t y, uint32_t pixel);

    // clear the console
    void clear();

    // put a single character to the console
    // at position (charX, charY) (this is the char position, not the pixel position)
    void putc(uint32_t charX, uint32_t charY, char c, unsigned int textColor, unsigned int reverseColor);
    void putc(char c);
    void putc(char c, unsigned int color);
    void putc(char c, unsigned int color, unsigned int rev);

    // places a null-terminated string starting at (charX, charY) (position by characters)
    void puts(uint32_t charX, uint32_t charY, const char* str, unsigned int color, unsigned int rev);
    void puts(const char* str);
    void puts(const char* str, unsigned int color);
    void puts(const char* str, unsigned int color, unsigned int rev);

    // allows limited printing functionality (prints to the console)
    void printf(const char* format, ...);
};

#endif
