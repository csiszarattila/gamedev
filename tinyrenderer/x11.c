#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#include <X11/Xft/Xft.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <locale.h>

typedef struct {
    Display *display;
    Window window;
    XftDraw *xft_draw;
    XftColor *color;
    XftFont *font;
} StatusBar;

void StatusBar_DrawCurrentTime(StatusBar *bar)
{
    char out_buffer[255] = "HelLoOOO:)";
    char with_format[255];
    time_t current_time;
    struct tm* timeinfo;

    strcpy(with_format, "%H:%M:%S");

    time(&current_time);
    timeinfo = localtime(&current_time);

    strftime(out_buffer, sizeof(out_buffer), with_format, timeinfo);

    XClearWindow(bar->display, bar->window);

    int y = bar->font->ascent;
    XftDrawString8(bar->xft_draw, bar->color, bar->font, 100, 50, (FcChar8*) out_buffer, strlen(out_buffer));
}

StatusBar* StatusBar_Create(Display *display, Window window)
{
	XftDraw *draw;
    XftColor *color;
    XftFont *font;
	Colormap colorMap;
	Visual *visual;
    const char *fontName = "Liberation Mono-16:style=Bold";
    StatusBar *bar;

    int screen = XDefaultScreen(display);

    visual = XDefaultVisual(display, screen);
    colorMap = XDefaultColormap(display, screen);
	
    draw = XftDrawCreate(display, window, visual, colorMap);

    XRenderColor render_color = { .red=0x0000, .green=0x0000, .blue=0x0000, .alpha=0xffff };
    color = malloc(sizeof(XftColor));
    XftColorAllocValue(display, visual, colorMap, &render_color, color);

    font = XftFontOpenName(display, screen, fontName);

    bar = malloc(sizeof(StatusBar));
    
    bar->display = display;
    bar->window = window;
    bar->xft_draw = draw;
    bar->color = color;
    bar->font = font;

    return bar;
}

int main() {

    int screenWidth = 3840;
    int screenHeight = 2160;

    setlocale(LC_ALL, "");

    Display *display = XOpenDisplay(NULL);
    if (display == NULL) {
        fprintf(stderr, "Cannot open display\n");
        return 1;
    }

    int screen_num = DefaultScreen(display);
    Window root = RootWindow(display, screen_num);

    // Create window
    Window win = XCreateSimpleWindow(display, root, 0, 0, screenWidth, screenHeight, 2,
                                       BlackPixel(display, screen_num),
                                       WhitePixel(display, screen_num));
    
    // Select input events
    XSelectInput(display, win, ExposureMask | KeyPressMask | StructureNotifyMask);

    // Cursor
    Cursor cursor = XCreateFontCursor(display, XC_cross);
    XStoreName(display, win, "X11 test window");
    // Map (show) window
    XMapRaised(display, win);
    XSetInputFocus(display, win, RevertToParent, CurrentTime);
    XDefineCursor(display, win, cursor);
    XFlush(display);

    StatusBar *statusBar;
    
    statusBar = StatusBar_Create(display, win);

    // Event loop
    XEvent ev;
    while (1) {
        XNextEvent(display, &ev);
	
    	switch (ev.type) {
            case KeyPress:
    		    KeySym key = XLookupKeysym(&ev.xkey, 0);
    		    
                if (key == XK_Escape) {
    		    	XDestroyWindow(display, win);
    			    XCloseDisplay(display);
    			    return 0;
    		    }
    
    		    if (key == XK_c) {
    			    Window win2 = XCreateSimpleWindow(
                            display, 
                            root, 
                            200, 200, 
                            400, 200, 
                            2,
    					    BlackPixel(display, screen_num),
    					    WhitePixel(display, screen_num));
    			    XMapRaised(display, win2);
    			    XFlush(display);
    		    }
                break;
    
    	    case Expose:
    		    StatusBar_DrawCurrentTime(statusBar);
                XFlush(display);
                break;
    
    	    case MapNotify:
    	       XClearWindow(display, win);
       	       XFlush(display);
    	       break;	   
    	}
    }

    // Cleanup
    XDestroyWindow(display, win);
    XCloseDisplay(display);
    return 0;
}
