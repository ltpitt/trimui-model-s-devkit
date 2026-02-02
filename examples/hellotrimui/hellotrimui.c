#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <string.h>
#include <linux/input.h>
#include <poll.h>
#include <errno.h>

#include "font8x8_basic.h"   // ← the correct, complete font file

/* Button enumeration (inspired by libmmenu conventions) */
typedef enum {
    BUTTON_UP = 0,
    BUTTON_DOWN,
    BUTTON_LEFT,
    BUTTON_RIGHT,
    BUTTON_A,
    BUTTON_B,
    BUTTON_X,
    BUTTON_Y,
    BUTTON_L,
    BUTTON_R,
    BUTTON_START,
    BUTTON_SELECT,
    BUTTON_MENU,
    BUTTON_UNKNOWN,
} Button;

/* Map evdev KEY codes to logical buttons */
Button evdev_to_button(int code) {
    switch (code) {
    case KEY_UP:        return BUTTON_UP;
    case KEY_DOWN:      return BUTTON_DOWN;
    case KEY_LEFT:      return BUTTON_LEFT;
    case KEY_RIGHT:     return BUTTON_RIGHT;
    case KEY_ENTER:     return BUTTON_A;        // commonly mapped
    case KEY_BACKSPACE: return BUTTON_B;        // commonly mapped
    case KEY_SPACE:     return BUTTON_X;        // fallback
    case KEY_MENU:      return BUTTON_MENU;
    case KEY_SELECT:    return BUTTON_SELECT;
    case KEY_VOLUMEUP:  return BUTTON_R;
    case KEY_VOLUMEDOWN:return BUTTON_L;
    default:            return BUTTON_UNKNOWN;
    }
}

/* Get friendly button name */
const char* button_name(Button btn) {
    switch (btn) {
    case BUTTON_UP:      return "UP";
    case BUTTON_DOWN:    return "DOWN";
    case BUTTON_LEFT:    return "LEFT";
    case BUTTON_RIGHT:   return "RIGHT";
    case BUTTON_A:       return "A";
    case BUTTON_B:       return "B";
    case BUTTON_X:       return "X";
    case BUTTON_Y:       return "Y";
    case BUTTON_L:       return "L";
    case BUTTON_R:       return "R";
    case BUTTON_START:   return "START";
    case BUTTON_SELECT:  return "SELECT";
    case BUTTON_MENU:    return "MENU";
    default:             return "???";
    }
}

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

/* Clear rectangle (fill with color) */
void fill_rect(uint8_t *fbp, int stride, int x, int y, int w, int h, uint16_t color) {
    for (int yy = y; yy < y + h; yy++) {
        if (yy < 0) continue;
        uint16_t *row = (uint16_t *)(fbp + yy * stride);
        for (int xx = x; xx < x + w; xx++) {
            if (xx < 0) continue;
            row[xx] = color;
        }
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

    // Layout: title at top, button display in center, exit instruction at bottom
    
    // Title (top)
    int title_x = (width  - 16 * strlen(msg)) / 2;
    int title_y = 16;
    draw_text_2x(fbp, stride, title_x, title_y, msg, white);

    // Button display area (center, large and prominent)
    int button_x = 8;
    int button_y = 100;
    int button_w = width - 16;
    int button_h = 48;

    char button_info[128] = "No button pressed";
    
    // Exit instruction (bottom)
    const char *exit_msg = "Press MENU to exit";
    int exit_x = (width - 16 * strlen(exit_msg)) / 2;
    int exit_y = height - 32;
    
    // Draw initial state
    fill_rect(fbp, stride, button_x, button_y, button_w, button_h, bg);
    draw_text_2x(fbp, stride, button_x, button_y, button_info, white);
    draw_text_2x(fbp, stride, exit_x, exit_y, exit_msg, white);

    // Prepare input devices (try common event nodes)
    #define MAX_EV 4
    int evfds[MAX_EV];
    for (int i = 0; i < MAX_EV; i++) evfds[i] = -1;
    for (int i = 0; i < MAX_EV; i++) {
        char path[64];
        snprintf(path, sizeof(path), "/dev/input/event%d", i);
        int fd = open(path, O_RDONLY | O_NONBLOCK);
        if (fd >= 0) {
            evfds[i] = fd;
        }
    }

    // Display area for button press info (below "Hello Trimui")
    int info_x = 8;
    int info_y = start_y + 48;
    int info_w = width - 16;
    int info_h = 32;

    char info[128] = "Press buttons...";
    draw_text_2x(fbp, stride, info_x, info_y, info, white);

    // Poll loop: exit on MENU button press or after 30 seconds
    struct pollfd pfds[MAX_EV];
    int active_fds = 0;
    for (int i = 0; i < MAX_EV; i++) {
        if (evfds[i] >= 0) {
            pfds[active_fds].fd = evfds[i];
            pfds[active_fds].events = POLLIN;
            active_fds++;
        }
    }

    int timeout_ms = 100; // poll timeout
    time_t start = time(NULL);
    int running = 1;
    while (running) {
        if (active_fds == 0) {
            // No input devices found: just wait a short time then exit
            usleep(200 * 1000);
            if (time(NULL) - start > 5) break;
            continue;
        }

        int ret = poll(pfds, active_fds, timeout_ms);
        if (ret > 0) {
            for (int i = 0; i < active_fds; i++) {
                if (pfds[i].revents & POLLIN) {
                    struct input_event ev;
                    ssize_t rd = read(pfds[i].fd, &ev, sizeof(ev));
                    if (rd == (ssize_t)sizeof(ev)) {
                        if (ev.type == EV_KEY) {
                            Button btn = evdev_to_button(ev.code);
                            const char *action = (ev.value == 1) ? "pressed" : "released";
                            
                            // Update button display in center
                            snprintf(button_info, sizeof(button_info), "Button: %s %s", button_name(btn), action);
                            fill_rect(fbp, stride, button_x, button_y, button_w, button_h, bg);
                            draw_text_2x(fbp, stride, button_x, button_y, button_info, white);
                            
                            // Exit on MENU button press
                            if (btn == BUTTON_MENU && ev.value == 1) {
                                running = 0;
                                break;
                            }
                        } else if (ev.type == EV_ABS || ev.type == EV_REL) {
                            snprintf(button_info, sizeof(button_info), "Axis: type=%d code=%d value=%d", ev.type, ev.code, ev.value);
                            fill_rect(fbp, stride, button_x, button_y, button_w, button_h, bg);
                            draw_text_2x(fbp, stride, button_x, button_y, button_info, white);
                        }
                    }
                }
            }
        }

        // Timeout exit after 30s
        if (time(NULL) - start > 30) break;
    }

    // Close input fds
    for (int i = 0; i < MAX_EV; i++) if (evfds[i] >= 0) close(evfds[i]);

    munmap(fbp, finfo.smem_len);
    close(fb);

    return 0;
}
