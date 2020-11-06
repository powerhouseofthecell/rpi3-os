#include "common/console.hh"
#include "common/stdlib.hh"

// initialize the console
Console::Console(uint64_t addr) {
    console_addr = addr;
}

// allow setting text
void Console::setTextColor(int color) {
    text_color = color;
}

// allow setting the reverse video color
void Console::setReverseColor(int color) {
    reverse_video = color;
}

// x and y are pixel locations
void Console::write_pixel(uint32_t x, uint32_t y, uint32_t pixel) {
    unsigned int* location = (unsigned int*) (console_addr + y * ((fbInfo.depth / 8) * fbInfo.width) + x * (fbInfo.depth / 8));
    
    // set the location to the pixel
    *location = pixel;
}

// clear the console by setting all pixels to reverse_video
void Console::clear() {
    for (int y = 0; y < fbInfo.height; ++y) {
        for (int x = 0; x < fbInfo.width; ++x) {
            write_pixel(x, y, reverse_video);
        }
    }

    char_xpos = 0;
    char_ypos = 0;
}

// put a single character (c) to the console at character position (charX, charY)
void Console::putc(uint32_t charX, uint32_t charY, char c, unsigned int textColor, unsigned int reverseColor) {
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
        
        case '\t':
            char_xpos = round_up(char_xpos, 4);
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
    putc(char_xpos, char_ypos, c, text_color, reverse_video);
}
void Console::putc(char c, unsigned int color) {
    putc(char_xpos, char_ypos, c, color, reverse_video);
}
void Console::putc(char c, unsigned int color, unsigned int rev) {
    putc(char_xpos, char_ypos, c, color, rev);
}

// allows printing whole strings to the console
void Console::puts(uint32_t charX, uint32_t charY, const char* str, unsigned int color, unsigned int rev) {
    for (int i = 0; str[i] != '\0'; ++i) {
        putc(charX + i, charY, str[i], color, rev);
    }
}
void Console::puts(const char* str) {
    puts(char_xpos, char_ypos, str, text_color, reverse_video);
}
void Console::puts(const char* str, unsigned int color) {
    puts(char_xpos, char_ypos, str, color, reverse_video);
}
void Console::puts(const char* str, unsigned int color, unsigned int rev) {
    puts(char_xpos, char_ypos, str, color, rev);
}

// this is also borrowed from WeensyOS
static char* fill_numbuf(char* numbuf_end, unsigned long val, int base) {
    static const char upper_digits[] = "0123456789ABCDEF";
    static const char lower_digits[] = "0123456789abcdef";

    const char* digits = upper_digits;
    if (base < 0) {
        digits = lower_digits;
        base = -base;
    }

    *--numbuf_end = '\0';
    do {
        *--numbuf_end = digits[val % base];
        val /= base;
    } while (val != 0);
    return numbuf_end;
}

#define FLAG_ALT                (1<<0)
#define FLAG_ZERO               (1<<1)
#define FLAG_LEFTJUSTIFY        (1<<2)
#define FLAG_SPACEPOSITIVE      (1<<3)
#define FLAG_PLUSPOSITIVE       (1<<4)
static const char flag_chars[] = "#0- +";

#define FLAG_NUMERIC            (1<<5)
#define FLAG_SIGNED             (1<<6)
#define FLAG_NEGATIVE           (1<<7)
#define FLAG_ALT2               (1<<8)

// limited format string printing to the console! (borrowed from WeensyOS)
void Console::printf(const char* format, ...) {
    va_list val;
    va_start(val, format);

    unsigned int color = text_color;

#define NUMBUFSIZ 24
    char numbuf[NUMBUFSIZ];

    for (; *format; ++format) {
        if (*format != '%') {
            putc(*format);
            continue;
        }

        // process flags
        int flags = 0;
        for (++format; *format; ++format) {
            const char* flagc = strchr(flag_chars, *format);
            if (flagc) {
                flags |= 1 << (flagc - flag_chars);
            } else {
                break;
            }
        }

        // process width
        int width = -1;
        if (*format >= '1' && *format <= '9') {
            for (width = 0; *format >= '0' && *format <= '9'; ) {
                width = 10 * width + *format++ - '0';
            }
        } else if (*format == '*') {
            width = va_arg(val, int);
            ++format;
        }

        // process precision
        int precision = -1;
        if (*format == '.') {
            ++format;
            if (*format >= '0' && *format <= '9') {
                for (precision = 0; *format >= '0' && *format <= '9'; ) {
                    precision = 10 * precision + *format++ - '0';
                }
            } else if (*format == '*') {
                precision = va_arg(val, int);
                ++format;
            }
            if (precision < 0) {
                precision = 0;
            }
        }

        // process length
        int length = 0;
        switch (*format) {
        case 'l':
        case 't': // ptrdiff_t
        case 'z': // size_t, ssize_t
            length = 1;
            ++format;
            break;
        case 'h':
            ++format;
            break;
        }

        // process main conversion character
        int base = 10;
        unsigned long num = 0;
        const char* data = "";

        switch (*format) {
        case 'd':
        case 'i': {
            long x = length ? va_arg(val, long) : va_arg(val, int);
            int negative = x < 0 ? FLAG_NEGATIVE : 0;
            num = negative ? -x : x;
            flags |= FLAG_NUMERIC | FLAG_SIGNED | negative;
            break;
        }
        case 'u':
        format_unsigned:
            num = length ? va_arg(val, unsigned long) : va_arg(val, unsigned);
            flags |= FLAG_NUMERIC;
            break;
        case 'x':
            base = -16;
            goto format_unsigned;
        case 'X':
            base = 16;
            goto format_unsigned;
        case 'p':
            num = (uintptr_t) va_arg(val, void*);
            base = -16;
            flags |= FLAG_ALT | FLAG_ALT2 | FLAG_NUMERIC;
            break;
        case 's':
            data = va_arg(val, char*);
            break;
        case 'C':
            color = va_arg(val, int);
            continue;
        case 'c':
            data = numbuf;
            numbuf[0] = va_arg(val, int);
            numbuf[1] = '\0';
            break;
        default:
            data = numbuf;
            numbuf[0] = (*format ? *format : '%');
            numbuf[1] = '\0';
            if (!*format) {
                format--;
            }
            break;
        }

        if (flags & FLAG_NUMERIC) {
            data = fill_numbuf(numbuf + NUMBUFSIZ, num, base);
        }

        const char* prefix = "";
        if ((flags & FLAG_NUMERIC) && (flags & FLAG_SIGNED)) {
            if (flags & FLAG_NEGATIVE) {
                prefix = "-";
            } else if (flags & FLAG_PLUSPOSITIVE) {
                prefix = "+";
            } else if (flags & FLAG_SPACEPOSITIVE) {
                prefix = " ";
            }
        } else if ((flags & FLAG_NUMERIC) && (flags & FLAG_ALT)
                   && (base == 16 || base == -16)
                   && (num || (flags & FLAG_ALT2))) {
            prefix = (base == -16 ? "0x" : "0X");
        }

        int datalen;
        if (precision >= 0 && !(flags & FLAG_NUMERIC)) {
            datalen = strnlen(data, precision);
        } else {
            datalen = strlen(data);
        }

        int zeros;
        if ((flags & FLAG_NUMERIC)
            && precision >= 0) {
            zeros = precision > datalen ? precision - datalen : 0;
        } else if ((flags & FLAG_NUMERIC)
                   && (flags & FLAG_ZERO)
                   && !(flags & FLAG_LEFTJUSTIFY)
                   && datalen + (int) strlen(prefix) < width) {
            zeros = width - datalen - strlen(prefix);
        } else {
            zeros = 0;
        }

        width -= datalen + zeros + strlen(prefix);
        for (; !(flags & FLAG_LEFTJUSTIFY) && width > 0; --width) {
            putc(' ', color);
        }
        for (; *prefix; ++prefix) {
            putc(*prefix, color);
        }
        for (; zeros > 0; --zeros) {
            putc('0', color);
        }
        for (; datalen > 0; ++data, --datalen) {
            putc(*data, color);
        }
        for (; width > 0; --width) {
            putc(' ', color);
        }
    }

    va_end(val);
}
