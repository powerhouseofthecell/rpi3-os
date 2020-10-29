#include "kernel/lfb.hh"

// the global font to use for this system
psf_t* font = (psf_t*)&_binary_font_psf_start;

unsigned int width, height, pitch;
unsigned char *lfb;

/**
 * Initialize the linear frame buffer
 * Set screen resolution to 1024x768
 */
void lfb_init()
{
    mbox[0] = 35 * 4;
    mbox[1] = MBOX_REQUEST;

    mbox[2] = 0x48003;  //set phy wh
    mbox[3] = 8;
    mbox[4] = 8;
    mbox[5] = 1024;         //FrameBufferInfo.width
    mbox[6] = 768;          //FrameBufferInfo.height

    mbox[7] = 0x48004;  //set virt wh
    mbox[8] = 8;
    mbox[9] = 8;
    mbox[10] = 1024;        //FrameBufferInfo.virtual_width
    mbox[11] = 768;         //FrameBufferInfo.virtual_height

    mbox[12] = 0x48009; //set virt offset
    mbox[13] = 8;
    mbox[14] = 8;
    mbox[15] = 0;           //FrameBufferInfo.x_offset
    mbox[16] = 0;           //FrameBufferInfo.y.offset

    mbox[17] = 0x48005; //set depth
    mbox[18] = 4;
    mbox[19] = 4;
    mbox[20] = 32;          //FrameBufferInfo.depth

    mbox[21] = 0x48006; //set pixel order
    mbox[22] = 4;
    mbox[23] = 4;
    mbox[24] = 1;           //RGB, not BGR preferably

    mbox[25] = 0x40001; //get framebuffer, gets alignment on request
    mbox[26] = 8;
    mbox[27] = 8;
    mbox[28] = 4096;        //FrameBufferInfo.pointer
    mbox[29] = 0;           //FrameBufferInfo.size

    mbox[30] = 0x40008; //get pitch
    mbox[31] = 4;
    mbox[32] = 4;
    mbox[33] = 0;           //FrameBufferInfo.pitch

    mbox[34] = MBOX_TAG_LAST;

    if(mbox_call(MBOX_CH_PROP) && mbox[20]==32 && mbox[28]!=0) {
        width   = mbox[5];
        height  = mbox[6];
        pitch   = mbox[33];

        mbox[28] &= 0x3FFFFFFF;
        lfb = (unsigned char*)((unsigned long) mbox[28]);
    } else {
        uart_puts("Unable to set screen resolution to 1024x768x32\n");
    }
}

/**
 * Display a string using fixed size PSF
 */
void lfb_print(int x, int y, const char *s) {
    int color = 0xFFFFFF;
    int rev_color = 0x00;

    // draw next character if it's not zero
    while(*s) {
        // get the offset of the glyph. Need to adjust this to support unicode table
        unsigned char *glyph = (unsigned char*)&_binary_font_psf_start +
         font->headersize + (*((unsigned char*)s)<font->numglyph?*s:0)*font->bytesperglyph;
        // calculate the offset on screen
        int offs = (y * pitch) + (x * 4);
        // variables
        unsigned int i, j, line, mask, bytesperline=(font->width+7)/8;
        // handle carrige return
        if (*s == '\r') {
            x = 0;
        } else if (*s == '\n') {
            x = 0; y += font->height;
        } else {
            // display a character
            for(j = 0; j < font->height; ++j){
                // display one row
                line = offs;
                mask = 1 << (font->width - 1);

                for(i = 0; i < font->width; ++i){
                    // if bit set, we use white color, otherwise black
                    *((unsigned int*)(lfb + line))= ((int)*glyph) & mask ? color : rev_color;
                    mask >>= 1;
                    line += 4;
                }
                // adjust to next line
                glyph+=bytesperline;
                offs+=pitch;
            }
            x += (font->width+1);
        }
        // next character
        s++;
    }
}

// puts a single character to the lfb
void lfb_putc(int x, int y, char c, int color, int rev_color) {
    // get the offset of the glyph. Need to adjust this to support unicode table
    unsigned char *glyph = (unsigned char*) &_binary_font_psf_start +
        font->headersize + (((unsigned char)c) < font->numglyph ? c : 0) * font->bytesperglyph;

    // calculate the offset on screen
    int offs = (y * pitch) + (x * 4);

    // variables
    unsigned int i, j, line, mask, bytesperline = (font->width + 7) / 8;
    
    // handle carrige return and newline
    if (c == '\r') {
        x = 0;
    } else if (c == '\n') {
        x = 0; y += font->height;
    } else {
        // display a character
        for (j = 0; j < font->height; ++j) {
            // display one row
            line = offs;
            mask = 1 << (font->width - 1);

            for (i = 0; i < font->width; ++i) {
                // if bit set, we use white color, otherwise black
                *((unsigned int*)(lfb + line)) = ((int)*glyph) & mask ? color : rev_color;
                mask >>= 1;
                line += 4;
            }
            // adjust to next line
            glyph += bytesperline;
            offs += pitch;
        }
        x += (font->width + 1);
    }
}