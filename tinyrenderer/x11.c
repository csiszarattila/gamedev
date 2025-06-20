#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    Display *display = XOpenDisplay(NULL);
    if (display == NULL) {
        fprintf(stderr, "Cannot open display\n");
        return 1;
    }

    int screen_num = DefaultScreen(display);
    Window root = RootWindow(display, screen_num);

    // Create window
    Window win = XCreateSimpleWindow(display, root, 10, 10, 800, 600, 1,
                                       BlackPixel(display, screen_num),
                                       WhitePixel(display, screen_num));

    // Select input events
    XSelectInput(display, win, ExposureMask | KeyPressMask);

    // Map (show) window
    XMapRaised(display, win);

    // Event loop
    XEvent event;
    while (1) {
        XNextEvent(display, &event);
        if (event.type == KeyPress)
            break; // Exit on key press
        if (event.type == Expose) {
            // Draw or refresh window content here
        }
    }

    // Cleanup
    XDestroyWindow(display, win);
    XCloseDisplay(display);
    return 0;
}
