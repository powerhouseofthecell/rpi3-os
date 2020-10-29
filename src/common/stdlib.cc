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
    static char intbuf[32];
    uint32_t j = 0, isneg = 0, i;

    if (num == 0) {
        intbuf[0] = '0';
        intbuf[1] = '\0';
        return intbuf;
    }

    if (base == 10 && num < 0) {
        isneg = 1;
        num = -num;
    }

    i = (uint32_t) num;

    while (i != 0) {
       intbuf[j++] = (i % base) < 10 ? '0' + (i % base) : 'a' + (i % base) - 10;
       i = i / base;
    }

    if (isneg)
        intbuf[j++] = '-';

    if (base == 16) {
        intbuf[j++] = 'x';
        intbuf[j++] = '0';
    } else if(base == 8) {
        intbuf[j++] = '0';
    } else if(base == 2) {
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
