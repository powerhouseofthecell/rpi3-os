#ifndef LFB_HH
#define LFB_HH
#include "common/types.hh"
#include "kernel/uart.hh"
#include "kernel/mbox.hh"

/* PC Screen Font as used by Linux Console */
typedef struct {
    unsigned int magic;
    unsigned int version;
    unsigned int headersize;
    unsigned int flags;
    unsigned int numglyph;
    unsigned int bytesperglyph;
    unsigned int height;
    unsigned int width;
    unsigned char glyphs;
} __attribute__((packed)) psf_t;
extern volatile unsigned char _binary_font_psf_start;

/* Scalable Screen Font (https://gitlab.com/bztsrc/scalable-font2) */
typedef struct {
    unsigned char  magic[4];
    unsigned int   size;
    unsigned char  type;
    unsigned char  features;
    unsigned char  width;
    unsigned char  height;
    unsigned char  baseline;
    unsigned char  underline;
    unsigned short fragments_offs;
    unsigned int   characters_offs;
    unsigned int   ligature_offs;
    unsigned int   kerning_offs;
    unsigned int   cmap_offs;
} __attribute__((packed)) sfn_t;
extern volatile unsigned char _binary_font_sfn_start;

// contains information about the framebuffer
// there should be a single global one of these
struct framebufferInfo {
    // address of the framebuffer
    uint64_t addr;

    // height of the console in pixels
    int height;

    // width of the console in pixels
    int width;

    // how many bits per pixel
    int depth;
    int pitch;
};

// define some lfb globals
extern psf_t* font;

extern framebufferInfo fbInfo;

void lfb_init();

void lfb_print(int x, int y, const char *s);

void lfb_putc(int x, int y, char c, int color, int rev_color);

#endif