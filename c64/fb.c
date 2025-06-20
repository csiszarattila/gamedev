#include <c64.h>

#define BITMAP ((unsigned char *)0x2000)
#define SCREEN ((unsigned char *)0x0400)

const unsigned char font_A[8] = {
    0x3C, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00
};

typedef struct {
    int width;
    int height;
} screen;

void SetPixel(screen *screen, int x, int y, char val)
{
    int offset;

    offset = (y / 8) * screen->width + (y % 8) + (x / 8) * 8;
    SCREEN[offset] = val;
}

void main()
{
    int b;

    VIC.ctrl1 = 0x3B;      // Bitmap mode (BMM=1, ECM=0)
    VIC.addr = 0x18;       // Bitmap at $2000, screen at $0400

    for (b =0; b < 8000; b++) {
        BITMAP[b] = 0;
    }

    for (b =0; b < 1000; b++) {
        SCREEN[b] = 0x0F; // Set all screen characters to color 0x0F (white)
    }

    for (b=0; b < 8; b++) {
        BITMAP[b] = font_A[b];
    }
    
    // unsigned int bitmap = 0x2000;
    // *(unsigned char *)bitmap = 0x20;
    for(;;) {

    }
}