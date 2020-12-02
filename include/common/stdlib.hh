#ifndef STDLIB_HH
#define STDLIB_HH
#include "common/types.hh"
#include "common/uart.hh"
#include "common/lfb.hh"

extern "C" {
void* memcpy(void* dst, const void* src, size_t n);
void* memmove(void* dst, const void* src, size_t n);
void* memset(void* s, int c, size_t n);
int memcmp(const void* a, const void* b, size_t n);
void* memchr(const void* s, int c, size_t n);
size_t strlen(const char* s);
size_t strnlen(const char* s, size_t maxlen);
char* strcpy(char* dst, const char* src);
int strcmp(const char* a, const char* b);
int strncmp(const char* a, const char* b, size_t maxlen);
int strcasecmp(const char* a, const char* b);
int strncasecmp(const char* a, const char* b, size_t maxlen);
char* strchr(const char* s, int c);
long strtol(const char* s, char** endptr = nullptr, int base = 0);
unsigned long strtoul(const char* s, char** endptr = nullptr, int base = 0);
ssize_t snprintf(char* s, size_t size, const char* format, ...);
ssize_t vsnprintf(char* s, size_t size, const char* format, va_list val);
inline bool isspace(int c);
inline bool isdigit(int c);
inline bool islower(int c);
inline bool isupper(int c);
inline bool isalpha(int c);
inline bool isalnum(int c);
inline int tolower(int c);
inline int toupper(int c);
}

char* itoa(int num, int base);
int atoi(char * num);

#define RAND_MAX 0x7FFFFFFF
int rand();
void srand(unsigned seed);
int rand(int min, int max);

// single character manipulations/properties
inline bool isspace(int c) {
    return (c >= '\t' && c <= '\r') || c == ' ';
}
inline bool isdigit(int c) {
    return (unsigned(c) - unsigned('0')) < 10;
}
inline bool islower(int c) {
    return (unsigned(c) - unsigned('a')) < 26;
}
inline bool isupper(int c) {
    return (unsigned(c) - unsigned('A')) < 26;
}
inline bool isalpha(int c) {
    return ((unsigned(c) | 0x20) - unsigned('a')) < 26;
}
inline bool isalnum(int c) {
    return isalpha(c) || isdigit(c);
}

inline int tolower(int c) {
    return isupper(c) ? c + 'a' - 'A' : c;
}
inline int toupper(int c) {
    return islower(c) ? c + 'A' - 'a' : c;
}

template <typename T>
inline constexpr T round_down(T x, unsigned multiple) {
    return x - (x % multiple);
}
template <typename T>
inline constexpr T round_up(T x, unsigned multiple) {
    return round_down(x + multiple - 1, multiple);
}

// console color stuff
static const unsigned int WHITE = 0xffffff;
static const unsigned int BLACK = 0x000000;
static const unsigned int BLUE =  0xff0000;
static const unsigned int RED =   0x0000ff;
static const unsigned int GREEN = 0x00ff00;

static const unsigned int COLORS[4] = {WHITE, BLUE, RED, GREEN};

// console special characters
#define CONSOLE_SQUARE          128
#define CONSOLE_SQUARE_OUTLINE  129

// position of the console cursor
extern uint32_t char_xpos;
extern uint32_t char_ypos;

// writes the provided pixel to the console @ (x, y)
void write_pixel(uint32_t x, uint32_t y, uint32_t pixel);

// clear the console
void clear_console();

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

// assert(x)
//    If !x, then print a message and spin forever
#define assert(x) \
    if (!(x)) \
        _assertion_failure(__FILE__, __LINE__)
inline void _assertion_failure(const char* file, int line) {
    uart_puts(file);
    uart_puts(":");
    uart_puts(itoa(line, 10));
    uart_puts(": Assertion failure\n");
    
    while (true);
}

// syscalls!
#define SYSCALL_GETPID          1
#define SYSCALL_YIELD           2
#define SYSCALL_PANIC           3
#define SYSCALL_PAGE_ALLOC      4
#define SYSCALL_FORK            5
#define SYSCALL_EXIT            6
#define SYSCALL_PAGE_FREE       7

__always_inline uint64_t make_syscall(uint16_t syscallno) {
    asm volatile("svc %0" : : "i" (syscallno));

    uint64_t ret_val;
    asm volatile("mov %0, x0" : "=r" (ret_val));

    return ret_val;
}

__always_inline uint64_t make_syscall(uint16_t syscallno, uint64_t arg1) {
    asm volatile("mov x0, %0" :: "r" (arg1));
    asm volatile("svc %0" : : "i" (syscallno));

    uint64_t ret_val;
    asm volatile("mov %0, x0" : "=r" (ret_val));

    return ret_val;
}

// return the pid for the current process
inline pid_t sys_getpid() {
    return make_syscall(SYSCALL_GETPID);
}

// yield and allow another process to run
inline void sys_yield() {
    make_syscall(SYSCALL_YIELD);
}

// allocate a page of memory and give it to the process
inline uintptr_t sys_page_alloc() {
    return make_syscall(SYSCALL_PAGE_ALLOC);
}

inline void sys_page_free(void* ptr) {
    make_syscall(SYSCALL_PAGE_FREE, (uint64_t) ptr);
}

#endif