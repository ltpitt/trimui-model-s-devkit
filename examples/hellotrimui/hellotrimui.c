#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <string.h>

#include "font8x8_basic.h"   // ← the correct, complete font file

/* Draw a single character scaled 2× in RGB565 */
void draw_char_2x(uint8_t *fbp, int stride, int x, int y, char c, uint16_t color) {
    if (c < 0 || c > 127) return;

    const unsigned char *glyph = font8x8_basic[(int)c];

    for (int row = 0; row < 8; row++) {
        uint8_t bits = glyph[row];

        for (int col = 0; col < 8; col++) {
            if (bits & (1 << col)) {
                for (int dy = 0; dy < 2; dy++) {
                    uint16_t *dst = (uint16_t *)(fbp + (y + row*2 + dy) * stride
                                                 + (x + col*2) * 2);
                    dst[0] = color;
                    dst[1] = color;
                }
            }
        }
    }
}

/* Draw a string scaled 2× */
void draw_text_2x(uint8_t *fbp, int stride, int x, int y,
                  const char *text, uint16_t color) {
    while (*text) {
        draw_char_2x(fbp, stride, x, y, *text, color);
        x += 16; // 8px * 2 scale
        text++;
    }
}

int main() {
    int fb = open("/dev/fb0", O_RDWR);
    if (fb < 0) {
        perror("open");
        return 1;
    }

    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;

    ioctl(fb, FBIOGET_VSCREENINFO, &vinfo);
    ioctl(fb, FBIOGET_FSCREENINFO, &finfo);

    uint8_t *fbp = mmap(NULL, finfo.smem_len,
                        PROT_READ | PROT_WRITE,
                        MAP_SHARED, fb, 0);

    int width  = vinfo.xres;        // 320
    int height = vinfo.yres;        // 240
    int stride = finfo.line_length; // 640 bytes

    // Colors
    uint16_t bg    = (0 << 11) | (0 << 5) | 31;  // dark blue
    uint16_t white = 0xFFFF;

    // Fill background
    for (int y = 0; y < height; y++) {
        uint16_t *row = (uint16_t *)(fbp + y * stride);
        for (int x = 0; x < width; x++) {
            row[x] = bg;
        }
    }

    const char *msg = "Hello Trimui";

    int text_width_px  = 16 * strlen(msg); // 8px * 2 scale
    int text_height_px = 16;

    int start_x = (width  - text_width_px)  / 2;
    int start_y = (height - text_height_px) / 2;

    draw_text_2x(fbp, stride, start_x, start_y, msg, white);

    sleep(5);

    munmap(fbp, finfo.smem_len);
    close(fb);

    return 0;
}
