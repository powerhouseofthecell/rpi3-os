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
    unsigned char* glyphs;
} __attribute__((packed)) psf_t;
extern volatile unsigned char _binary_font_psf_start;

// define some lfb globals
extern psf_t* font;

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

    uint32_t xpos;
    uint32_t ypos;
};
extern framebufferInfo fbInfo;

void lfb_init();

#endif