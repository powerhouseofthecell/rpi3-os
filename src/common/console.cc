#include "common/console.hh"
#include "common/stdlib.hh"

Console::Console(uint64_t addr) {
    console_addr = addr;
}

// x and y are pixel locations
void Console::write_pixel(uint32_t x, uint32_t y, uint32_t pixel) {
    unsigned int* location = (unsigned int*) (console_addr + y * ((fbInfo.depth / 8) * fbInfo.width) + x * (fbInfo.depth / 8));
    
    // set the location to the pixel
    *location = pixel;
}

// clear the console by setting all pixels to black
void Console::clear() {
    for (int y = 0; y < fbInfo.height; ++y) {
        for (int x = 0; x < fbInfo.width; ++x) {
            write_pixel(x, y, 0x00);
        }
    }

    char_xpos = 0;
    char_ypos = 0;
}

// put a single character (c) to the console at character position (charX, charY)
void Console::putc(uint32_t charX, uint32_t charY, char c, int textColor, int reverseColor) {
    // don't attempt to print beyond the console
    if (charX >= (fbInfo.width / font->width) || charY >= (fbInfo.height / font->height)) {
        return;
    }

    switch (c) {
        case '\n':
            char_xpos = 0;
            ++char_ypos;
            break;

        case '\r':
            char_xpos = 0;
            break;

        default:
            // get the bitmap for this character
            unsigned char* glyph = (unsigned char*) &_binary_font_psf_start +
                font->headersize + (((unsigned char)c) < font->numglyph ? c : 0) * font->bytesperglyph;

            uint8_t w, h;

            // the mask determines which bit in the bitmap we're examining
            uint8_t mask;

            // print the character pixel by pixel
            for (w = 0; w < font->width; ++w) {
                for (h = 0; h < font->height; ++h) {
                    mask = 1 << (w);
                    if (glyph[h] & mask) {
                        write_pixel(charX * font->width + (font->width - w), charY * font->height + h, textColor);
                    } else {
                        write_pixel(charX * font->width + (font->width - w), charY * font->height + h, reverseColor);
                    }
                }
            }

            char_xpos = charX + 1;
            char_ypos = charY;
    }
    
    if (char_xpos >= fbInfo.width / font->width) {
        ++char_ypos;
        char_xpos = 0;
    }

    if (char_ypos >= fbInfo.height / font->height) {
        char_ypos = 0;
    }
}
void Console::putc(char c) {
    putc(char_xpos, char_ypos, c, WHITE, BLACK);
}

void Console::puts(uint32_t charX, uint32_t charY, const char* str) {
    for (int i = 0; str[i] != '\0'; ++i) {
        putc(charX + i, charY, str[i], WHITE, BLACK);
    }
}
void Console::puts(const char* str) {
    puts(char_xpos, char_ypos, str);
}

void Console::printf(const char * fmt, ...) {
    va_list args;
    va_start(args, fmt);

    for (; *fmt != '\0'; fmt++) {
        if (*fmt == '%') {
            switch (*(++fmt)) {
                case '%':
                    putc('%');
                    break;
                case 'i':
                case 'd':
                    puts(itoa(va_arg(args, int), 10));
                    break;
                case 'x':
                    puts(itoa(va_arg(args, int), 16));
                    break;
                case 's':
                    puts(va_arg(args, char *));
                    break;
            }
        } else {
            putc(*fmt);
        }
    }

    va_end(args);
}
