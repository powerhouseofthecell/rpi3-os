#include "common/stdlib.hh"

extern "C" {

// memcpy, memmove, memset, memcmp, memchr, strlen, strnlen, strcpy, strcmp,
// strncmp, strchr, strtoul, strtol
//    We must provide our own implementations.

void* memcpy(void* dst, const void* src, size_t n) {
    const char* s = (const char*) src;
    for (char* d = (char*) dst; n > 0; --n, ++s, ++d) {
        *d = *s;
    }
    return dst;
}

void* memmove(void* dst, const void* src, size_t n) {
    const char* s = (const char*) src;
    char* d = (char*) dst;
    if (s < d && s + n > d) {
        s += n, d += n;
        while (n-- > 0) {
            *--d = *--s;
        }
    } else {
        while (n-- > 0) {
            *d++ = *s++;
        }
    }
    return dst;
}

void* memset(void* v, int c, size_t n) {
    for (char* p = (char*) v; n > 0; ++p, --n) {
        *p = c;
    }
    return v;
}

int memcmp(const void* a, const void* b, size_t n) {
    const uint8_t* sa = reinterpret_cast<const uint8_t*>(a);
    const uint8_t* sb = reinterpret_cast<const uint8_t*>(b);
    for (; n > 0; ++sa, ++sb, --n) {
        if (*sa != *sb) {
            return (*sa > *sb) - (*sa < *sb);
        }
    }
    return 0;
}

void* memchr(const void* s, int c, size_t n) {
    const unsigned char* ss;
    for (ss = (const unsigned char*) s; n != 0; ++ss, --n) {
        if (*ss == (unsigned char) c) {
            return (void*) ss;
        }
    }
    return nullptr;
}

size_t strlen(const char* s) {
    size_t n;
    for (n = 0; *s != '\0'; ++s) {
        ++n;
    }
    return n;
}

size_t strnlen(const char* s, size_t maxlen) {
    size_t n;
    for (n = 0; n != maxlen && *s != '\0'; ++s) {
        ++n;
    }
    return n;
}

char* strcpy(char* dst, const char* src) {
    char* d = dst;
    do {
        *d++ = *src++;
    } while (d[-1]);
    return dst;
}

int strcmp(const char* a, const char* b) {
    while (true) {
        unsigned char ac = *a, bc = *b;
        if (ac == 0 || bc == 0 || ac != bc) {
            return (ac > bc) - (ac < bc);
        }
        ++a, ++b;
    }
}

int strncmp(const char* a, const char* b, size_t n) {
    while (true) {
        unsigned char ac = n ? *a : 0, bc = n ? *b : 0;
        if (ac == 0 || bc == 0 || ac != bc) {
            return (ac > bc) - (ac < bc);
        }
        ++a, ++b, --n;
    }
}

int strcasecmp(const char* a, const char* b) {
    while (true) {
        unsigned char ac = tolower((unsigned char) *a);
        unsigned char bc = tolower((unsigned char) *b);
        if (ac == 0 || bc == 0 || ac != bc) {
            return (ac > bc) - (ac < bc);
        }
        ++a, ++b;
    }
}

int strncasecmp(const char* a, const char* b, size_t n) {
    while (true) {
        unsigned char ac = n ? tolower((unsigned char) *a) : 0;
        unsigned char bc = n ? tolower((unsigned char) *b) : 0;
        if (ac == 0 || bc == 0 || ac != bc) {
            return (ac > bc) - (ac < bc);
        }
        ++a, ++b, --n;
    }
}

char* strchr(const char* s, int c) {
    while (*s && *s != (char) c) {
        ++s;
    }
    if (*s == (char) c) {
        return (char*) s;
    } else {
        return nullptr;
    }
}

unsigned long strtoul(const char* s, char** endptr, int base) {
    while (isspace(*s)) {
        ++s;
    }
    bool negative = *s == '-';
    s += negative || *s == '+';
    if (base == 0) {
        if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
            base = 16;
            s += 2;
        } else if (s[0] == '0') {
            base = 8;
        } else {
            base = 10;
        }
    } else if (base == 16 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        s += 2;
    }
    unsigned long x = 0;
    bool overflow = false;
    while (true) {
        unsigned digit;
        if (*s >= '0' && *s < '0' + base) {
            digit = *s - '0';
        } else if (base > 10 && *s >= 'a' && *s < 'a' + base - 10) {
            digit = *s - 'a' + 10;
        } else if (base > 10 && *s >= 'A' && *s < 'A' + base - 10) {
            digit = *s - 'A' + 10;
        } else {
            break;
        }
        if (x > (-1UL - digit) / base) {
            overflow = true;
        } else {
            x = x * base + digit;
        }
        ++s;
    }
    if (endptr) {
        *endptr = const_cast<char*>(s);
    }
    if (overflow) {
        x = -1UL;
    }
    if (negative) {
        x = -x;
    }
    return x;
}

long strtol(const char* s, char** endptr, int base) {
    while (isspace(*s)) {
        ++s;
    }
    bool negative = *s == '-';
    if (negative) {
        ++s;
    }
    unsigned long u = strtoul(s, endptr, base);
    unsigned long bound = (1UL << (8 * sizeof(unsigned long) - 1)) - !negative;
    if (u > bound) {
        u = bound;
    }
    if (negative) {
        u = -u;
    }
    return long(u);
}

} // extern "C"

// converts an integer to its string representation
char* itoa(int num, int base) {
    static char intbuf[64];
    uint64_t j = 0, isneg = 0, i;

    if (num == 0) {
        intbuf[0] = '0';
        intbuf[1] = '\0';
        return intbuf;
    }

    if (base == 10 && num < 0) {
        isneg = 1;
        num = -num;
    }

    i = (uint64_t) num;

    while (i != 0) {
       intbuf[j++] = (i % base) < 10 ? '0' + (i % base) : 'a' + (i % base) - 10;
       i = i / base;
    }

    if (isneg) {
        intbuf[j++] = '-';
    }

    if (base == 16) {
        intbuf[j++] = 'x';
        intbuf[j++] = '0';
    } else if (base == 8) {
        intbuf[j++] = '0';
    } else if (base == 2) {
        intbuf[j++] = 'b';
        intbuf[j++] = '0';
    }

    intbuf[j] = '\0';
    j--;
    i = 0;
    while (i < j) {
        isneg = intbuf[i];
        intbuf[i] = intbuf[j];
        intbuf[j] = isneg;
        i++;
        j--;
    }

    return intbuf;
}

// converts a string representation of a number to an integer
int atoi(char * num) {
    int res = 0, power = 0, digit, i;
    char* start = num;

    // Find the end
    while (*num >= '0' && *num <= '9') {
        num++;
    }

    num--;

    while (num != start) {
        digit = *num - '0';
        for (i = 0; i < power; i++) {
            digit *= 10;
        }
        res += digit;
        power++;
        num--;
    }

    return res;
}

// returns the current exception level
unsigned long getCurrentEL() {
    unsigned long el;

    // read the current level from system register
    asm volatile ("mrs %0, CurrentEL" : "=r" (el));

    return el >> 2;
}


// x and y are pixel locations
void write_pixel(uint32_t x, uint32_t y, uint32_t pixel) {
    unsigned int* location = (unsigned int*) (fbInfo.addr + y * ((fbInfo.depth / 8) * fbInfo.width) + x * (fbInfo.depth / 8));
    
    // set the location to the pixel
    *location = pixel;
}

// BEGIN: console functions
// clear the console by setting all pixels to reverse_video
void clear_console() {
    for (int y = 0; y < fbInfo.height; ++y) {
        for (int x = 0; x < fbInfo.width; ++x) {
            write_pixel(x, y, BLACK);
        }
    }

    fbInfo.xpos = 0;
    fbInfo.ypos = 0;
}

// returns true iff w, h is within a solid square
bool console_fx_square(uint8_t w, uint8_t h) {
    return (w >= 1 && w < 8) && (h >= 4 && h < 11);
}

// returns true iff w, h is on a square outline
bool console_fx_square_outline(uint8_t w, uint8_t h) {
    return (w == 1 || w == 7 || h == 4 || h == 10) 
        && console_fx_square(w, h);
}

// put a single character (c) to the console at character position (charX, charY)
void putc(uint32_t charX, uint32_t charY, char c, unsigned int textColor, unsigned int reverseColor) {
    // don't attempt to print beyond the console
    if (charX >= (fbInfo.width / font->width) || charY >= (fbInfo.height / font->height)) {
        return;
    }

    switch (c) {
        case '\n':
            fbInfo.xpos = 0;
            ++fbInfo.ypos;
            break;

        case '\r':
            fbInfo.xpos = 0;
            break;
        
        case '\t':
            fbInfo.xpos = round_up(fbInfo.xpos, 8);
            break;
        
        // prints a square
        case CONSOLE_SQUARE:
            for (uint8_t w = 0; w < font->width; ++w) {
                for (uint8_t h = 0; h < font->height; ++h) {
                    if (console_fx_square(w, h)) {
                        write_pixel(charX * font->width + (font->width - w), charY * font->height + h, textColor);
                    } else {
                        write_pixel(charX * font->width + (font->width - w), charY * font->height + h, reverseColor);
                    }
                }
            }

            fbInfo.xpos = charX + 1;
            fbInfo.ypos = charY;

            break;
        
        // prints a square outline
        case CONSOLE_SQUARE_OUTLINE:
            for (uint8_t w = 0; w < font->width; ++w) {
                for (uint8_t h = 0; h < font->height; ++h) {
                    if (console_fx_square_outline(w, h)) {
                        write_pixel(charX * font->width + (font->width - w), charY * font->height + h, textColor);
                    } else {
                        write_pixel(charX * font->width + (font->width - w), charY * font->height + h, reverseColor);
                    }
                }
            }

            fbInfo.xpos = charX + 1;
            fbInfo.ypos = charY;

            break;

        default:
            // get the bitmap for this character
            uint32_t idx = (((unsigned char) c) < font->numglyph ? c : 0) * font->bytesperglyph;
            unsigned char* glyph = (unsigned char*) &font->glyphs + idx;

            uint8_t w, h;

            // the mask determines which bit in the bitmap we're examining
            uint8_t mask;

            // print the character pixel by pixel
            for (w = 0; w < font->width; ++w) {
                for (h = 0; h < font->height; ++h) {
                    mask = (1 << w);
                    if (glyph[h] & mask) {
                        write_pixel(charX * font->width + (font->width - w), charY * font->height + h, textColor);
                    } else {
                        write_pixel(charX * font->width + (font->width - w), charY * font->height + h, reverseColor);
                    }
                }
            }

            fbInfo.xpos = charX + 1;
            fbInfo.ypos = charY;
    }
    
    if (fbInfo.xpos >= fbInfo.width / font->width) {
        ++fbInfo.ypos;
        fbInfo.xpos = 0;
    }

    if (fbInfo.ypos >= fbInfo.height / font->height) {
        fbInfo.ypos = 0;
    }
}
void putc(char c) {
    putc(fbInfo.xpos, fbInfo.ypos, c, WHITE, BLACK);
}
void putc(char c, unsigned int color) {
    putc(fbInfo.xpos, fbInfo.ypos, c, color, BLACK);
}
void putc(char c, unsigned int color, unsigned int rev) {
    putc(fbInfo.xpos, fbInfo.ypos, c, color, rev);
}

// allows printing whole strings to the console
void puts(uint32_t charX, uint32_t charY, const char* str, unsigned int color, unsigned int rev) {
    for (uint32_t i = 0; str[i] != '\0'; ++i) {
        putc(charX + i, charY, str[i], color, rev);
    }
}
void puts(const char* str) {
    puts(fbInfo.xpos, fbInfo.ypos, str, WHITE, BLACK);
}
void puts(const char* str, unsigned int color) {
    puts(fbInfo.xpos, fbInfo.ypos, str, color, BLACK);
}
void puts(const char* str, unsigned int color, unsigned int rev) {
    puts(fbInfo.xpos, fbInfo.ypos, str, color, rev);
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

#define NUMBUFSIZ 24

// limited format string printing to the console! (borrowed from WeensyOS)
void printf(const char* format, ...) {
    va_list val;
    va_start(val, format);

    unsigned int color = WHITE;

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
// END: console functions
