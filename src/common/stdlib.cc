#include "common/stdlib.hh"

int strlen(const char* str) {
    int len = 0;

    for (; str[len] != '\0'; ++len);

    return len;
}