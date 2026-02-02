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
#include <signal.h>
#include <time.h>
#include <sys/time.h>

#include "font8x8_basic.h"   // ← the correct, complete font file

/* Button enumeration (inspired by libmmenu conventions)
 * In C, enums auto-increment: BUTTON_UP = 0, BUTTON_DOWN = 1, etc.
 * No need to explicitly assign each value unless you want specific numbers.
 */
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

/* Map evdev KEY codes to logical buttons
 * TrimUI Model S button-to-evdev mappings:
 *   DPAD: UP=103, DOWN=108, LEFT=105, RIGHT=106
 *   Action buttons: X=42 (North), Y=56 (West), A=57 (East), B=29 (South)
 *   Shoulders: L=15, R=14
 *   Menu: SELECT=97, START=28, MENU=1
 * These codes are discovered by reading /dev/input/eventX with evdev
 */
Button evdev_to_button(int code) {
    switch (code) {
    case 103:           return BUTTON_UP;       // UP = 103
    case 108:           return BUTTON_DOWN;     // DOWN = 108
    case 105:           return BUTTON_LEFT;     // LEFT = 105
    case 106:           return BUTTON_RIGHT;    // RIGHT = 106
    case 42:            return BUTTON_X;        // NORTH = 42
    case 56:            return BUTTON_Y;        // WEST = 56
    case 57:            return BUTTON_A;        // EAST = 57
    case 29:            return BUTTON_B;        // SOUTH = 29
    case 15:            return BUTTON_L;        // L = 15
    case 14:            return BUTTON_R;        // R = 14
    case 28:            return BUTTON_START;    // START = 28
    case 97:            return BUTTON_SELECT;   // SELECT = 97
    case 1:             return BUTTON_MENU;     // MENU = 1
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

/* Play a simple beep on button press */
void beep_on_button(void) {
    // Non-blocking beep: fork a child process so event loop continues immediately
    // This makes button presses feel instant instead of waiting for sound to finish
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: play beep and exit
        int dsp = open("/dev/dsp", O_WRONLY | O_NONBLOCK);
        if (dsp < 0) exit(0);
        
        // Brief 50ms beep at 8kHz
        unsigned char beep[400];
        for (int i = 0; i < 400; i++) {
            beep[i] = (i % 2 == 0) ? 255 : 0;
        }
        
        ssize_t written = write(dsp, beep, sizeof(beep));
        (void)written;
        close(dsp);
        exit(0);
    }
    // Parent continues immediately - no blocking!
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
    // Redirect stdio to prevent launcher interference
    FILE *f_stdin = freopen("/dev/null", "r", stdin);
    FILE *f_stdout = freopen("/dev/null", "w", stdout);
    FILE *f_stderr = freopen("/dev/null", "w", stderr);
    (void)f_stdin; (void)f_stdout; (void)f_stderr;
    
    // Ignore signals that might be sent by the launcher
    signal(SIGTERM, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGCONT, SIG_IGN);
    
    // Try to take control of the console
    int console = open("/dev/console", O_RDWR);
    if (console >= 0) {
        // Successfully opened console
        (void)console; // Keep it open
    }
    
    // Small delay to ensure proper initialization
    usleep(100000); // 100ms
    
    // Set high priority to ensure we stay in foreground
    int prio = nice(-20);
    (void)prio; // Ignore return value
    
    // Detach from parent process
    setsid();
    
    // Open framebuffer device
    // Try with O_EXCL first to get exclusive access, fall back to shared mode
    int fb = open("/dev/fb0", O_RDWR | O_EXCL);
    if (fb < 0) {
        // Try without O_EXCL if that failed
        fb = open("/dev/fb0", O_RDWR);
        if (fb < 0) {
            return 1;
        }
    }
    
    // Suspend the launcher so we can take over the screen and input
    // (Kill -STOP pauses the process, -CONT resumes it)
    int ret_kill1 = system("killall -STOP gmenunx 2>/dev/null");
    int ret_kill2 = system("killall -STOP gmenu2x 2>/dev/null");
    int ret_kill3 = system("killall -STOP MainUI 2>/dev/null");
    (void)ret_kill1; (void)ret_kill2; (void)ret_kill3;

    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;

    ioctl(fb, FBIOGET_VSCREENINFO, &vinfo);
    ioctl(fb, FBIOGET_FSCREENINFO, &finfo);

    // Map framebuffer into memory
    // RGB565 format: 2 bytes per pixel (RRRRRGGGGGGBBBBB)
    uint8_t *fbp = mmap(NULL, finfo.smem_len,
                        PROT_READ | PROT_WRITE,
                        MAP_SHARED, fb, 0);

    int width  = vinfo.xres;        // 320
    int height = vinfo.yres;        // 240
    int stride = finfo.line_length; // 640 bytes (320 pixels * 2 bytes)

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
    int last_btn_code = -1;
    Button last_btn = BUTTON_UNKNOWN;
    int last_btn_state = 0;  // 0 = released, 1 = pressed
    int prev_btn_code = -1;  // Track previous button to detect state changes
    int prev_btn_state = 0;  // Track previous press/release state
    Button prev_btn = BUTTON_UNKNOWN;
    int need_redraw = 1;  // Flag to only redraw when state changes (instant response)
    
    // Exit instruction (bottom)
    const char *exit_msg = "Press MENU to exit";
    int exit_x = (width - 16 * strlen(exit_msg)) / 2;
    int exit_y = height - 32;
    
    // Draw initial state
    fill_rect(fbp, stride, button_x, button_y, button_w, button_h, bg);
    draw_text_2x(fbp, stride, button_x, button_y, button_info, white);
    draw_text_2x(fbp, stride, exit_x, exit_y, exit_msg, white);

    // Prepare input devices (try event0-event15, only open ones that exist)
    // Linux input subsystem exposes button presses via /dev/input/eventN
    // Multiple devices may report the same button, so we monitor all of them
    // O_NONBLOCK ensures read() returns immediately if no data available
    #define MAX_EV 16
    int evfds[MAX_EV];
    int num_evs = 0;  // Track how many event devices we actually opened
    for (int i = 0; i < MAX_EV; i++) evfds[i] = -1;
    for (int i = 0; i < MAX_EV; i++) {
        char path[64];
        snprintf(path, sizeof(path), "/dev/input/event%d", i);
        int fd = open(path, O_RDONLY | O_NONBLOCK);
        if (fd >= 0) {
            evfds[i] = fd;
            num_evs++;
        }
    }

    // Use poll() to efficiently wait for input on multiple devices
    // Much better than spinning in a loop checking each device
    struct pollfd pfds[MAX_EV];
    int active_fds = 0;
    for (int i = 0; i < MAX_EV; i++) {
        if (evfds[i] >= 0) {
            pfds[active_fds].fd = evfds[i];
            pfds[active_fds].events = POLLIN;
            active_fds++;
        }
    }

    int timeout_ms = 1; // 1ms timeout for ultra-responsive display updates (instant feel)
    int running = 1;
    while (running) {
        if (active_fds == 0) {
            // No input devices found: wait indefinitely for input
            usleep(100 * 1000);
            continue;
        }
        int ret = poll(pfds, active_fds, timeout_ms);
        for (int i = 0; i < active_fds; i++) {
            if (pfds[i].revents & POLLIN) {
                struct input_event ev;
                // Read multiple events in a loop to not miss any
                while (read(pfds[i].fd, &ev, sizeof(ev)) == (ssize_t)sizeof(ev)) {
                    if (ev.type == EV_KEY) {
                        Button btn = evdev_to_button(ev.code);
                        
                        // Ignore unknown buttons
                        if (btn == BUTTON_UNKNOWN) continue;
                        
                        // Only redraw on meaningful state changes
                        int state_changed = 0;
                        
                        if (ev.value == 1) {
                            // Button press - always show it
                            if (btn != last_btn || ev.code != last_btn_code || last_btn_state != 1) {
                                state_changed = 1;
                            }
                        } else if (ev.value == 0) {
                            // Button release - only clear if it's the button we're displaying
                            if (btn == last_btn && last_btn_state == 1) {
                                state_changed = 1;
                            }
                        }
                        
                        if (state_changed) {
                            last_btn = btn;
                            last_btn_code = ev.code;
                            last_btn_state = ev.value;
                            prev_btn = btn;
                            prev_btn_code = ev.code;
                            prev_btn_state = ev.value;
                            need_redraw = 1;
                        }
                        
                        // Beep on button press
                        if (ev.value == 1) {
                            beep_on_button();
                        }
                        // Exit on MENU button press
                        if (btn == BUTTON_MENU && ev.value == 1) {
                            running = 0;
                            break;
                        }
                    }
                }
            }
        }
        
        // Update display based on actual button state, not elapsed time
        // Show button when pressed (state=1), clear when released (state=0)
        if (need_redraw) {
            if (last_btn != BUTTON_UNKNOWN && last_btn_state == 1) {
                // Button is currently pressed
                snprintf(button_info, sizeof(button_info), "%s (code:%d)", button_name(last_btn), last_btn_code);
            } else {
                // Button was released or no button pressed yet
                snprintf(button_info, sizeof(button_info), "No button pressed");
            }
            
            fill_rect(fbp, stride, button_x, button_y, button_w, button_h, bg);
            draw_text_2x(fbp, stride, button_x, button_y, button_info, white);
            need_redraw = 0;
        }
    }

    // Close input fds
    for (int i = 0; i < MAX_EV; i++) if (evfds[i] >= 0) close(evfds[i]);

    // Resume launcher processes before exiting
    int ret_resume1 = system("killall -CONT gmenunx 2>/dev/null");
    int ret_resume2 = system("killall -CONT gmenu2x 2>/dev/null");
    int ret_resume3 = system("killall -CONT MainUI 2>/dev/null");
    (void)ret_resume1; (void)ret_resume2; (void)ret_resume3;

    munmap(fbp, finfo.smem_len);
    close(fb);

    return 0;
}
