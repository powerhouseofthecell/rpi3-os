#include "common/console.hh"
#include "common/stdlib.hh"

Console::Console(uint64_t addr) {
    console_addr = addr;
}

void Console::write_pixel(uint32_t x, uint32_t y, const uint8_t* pixel) {
    uint8_t* location = (uint8_t*) ((char*) console_addr) + y * bytesPerRow + x * bytesPerPixel;
    memcpy(location, (void*) pixel, bytesPerPixel);
}

void Console::clear() {
    // for (int y = 0; y < 480; ++y) {
    //     for (int x = 0; x < 640; ++x) {
    //         write_pixel(x, y, BLACK);
    //     }
    // }

    // char_xpos = 0;
    // char_ypos = 0;
}

void Console::putc(int charX, int charY, char c) {
    // // don't attempt to print beyond the console
    // if (charX >= (640 / CHAR_WIDTH) || charY >= (480 / CHAR_HEIGHT)) {
    //     return;
    // }

    // switch (c) {
    //     case '\n':
    //         char_xpos = 0;
    //         ++char_ypos;
    //         break;

    //     case '\r':
    //         char_xpos = 0;
    //         break;

    //     default:
    //         uint8_t w, h;
    //         uint8_t mask;

    //         // get the bitmap for this character
    //         const uint8_t* bmp = font(c);

    //         // print the character pixel by pixel
    //         for (w = 0; w < CHAR_WIDTH; ++w) {
    //             for (h = 0; h < CHAR_HEIGHT; ++h) {
    //                 mask = 1 << (w);
    //                 if (bmp[h] & mask) {
    //                     write_pixel(charX * CHAR_WIDTH + w, charY * CHAR_HEIGHT + h, text_color);
    //                 } else {
    //                     write_pixel(charX * CHAR_WIDTH + w, charY * CHAR_HEIGHT + h, reverse_video);
    //                 }
    //             }
    //         }

    //         char_xpos = charX + 1;
    //         char_ypos = charY;
    // }
    
    // if (char_xpos >= 640 / CHAR_WIDTH) {
    //     ++char_ypos;
    //     char_xpos = 0;
    // }

    // if (char_ypos >= 480 / CHAR_HEIGHT) {
    //     char_ypos = 0;
    // }
}
void Console::putc(char c) {
    // // don't attempt to print beyond the console
    // if (char_xpos >= (640 / CHAR_WIDTH) || char_ypos >= (480 / CHAR_HEIGHT)) {
    //     return;
    // }

    // switch (c) {
    //     case '\n':
    //         char_xpos = 0;
    //         ++char_ypos;
    //         break;

    //     case '\r':
    //         char_xpos = 0;
    //         break;

    //     default:
    //         uint8_t w, h;
    //         uint8_t mask;

    //         // get the bitmap for this character
    //         const uint8_t* bmp = font(c);

    //         // print the character pixel by pixel
    //         for (w = 0; w < CHAR_WIDTH; ++w) {
    //             for (h = 0; h < CHAR_HEIGHT; ++h) {
    //                 mask = 1 << (w);
    //                 if (bmp[h] & mask) {
    //                     write_pixel(char_xpos * CHAR_WIDTH + w, char_ypos * CHAR_HEIGHT + h, text_color);
    //                 } else {
    //                     write_pixel(char_xpos * CHAR_WIDTH + w, char_ypos * CHAR_HEIGHT + h, reverse_video);
    //                 }
    //             }
    //         }

    //         ++char_xpos;
    // }
    
    // if (char_xpos >= 640 / CHAR_WIDTH) {
    //     ++char_ypos;
    //     char_xpos = 0;
    // }

    // if (char_ypos >= 480 / CHAR_HEIGHT) {
    //     char_ypos = 0;
    // }
}

void Console::puts(int charX, int charY, const char* str) {
    for (int i = 0; str[i] != '\0'; ++i) {
        putc(charX + i, charY, str[i]);
    }
}
void Console::puts(const char* str) {
    for (int i = 0; str[i] != '\0'; ++i) {
        putc(str[i]);
    }
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
