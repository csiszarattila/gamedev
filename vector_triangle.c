#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#pragma pack(push, 1)
typedef struct {
    uint8_t  idLength;           // Length of optional identification sequence
    uint8_t  colorMapType;       // Is a palette present? (1=yes)
    uint8_t  imageType;          // Image data type (0=none, 1=indexed, 2=rgb, 3=grey, +8=rle packed)
    uint16_t firstPaletteEntry;  // First palette index, if present
    uint16_t numPaletteEntries;  // Number of palette entries, if present
    uint8_t  paletteBits;        // Number of bits per palette entry
    uint16_t xOrigin;            // X coordinate of lower left of image
    uint16_t yOrigin;            // Y coordinate of lower left of image
    uint16_t width;              // Image width in pixels
    uint16_t height;             // Image height in pixels
    uint8_t  depth;              // Bits per pixel
    uint8_t  descriptor;         // Image attribute flags
} TGAHeader;
#pragma pack(pop)

typedef struct {
    uint8_t B;
    uint8_t G;
    uint8_t R;
} TGAPixel;

typedef struct {
    TGAHeader header;
    TGAPixel *pixels;
} TGAImage;

const TGAPixel white = { .B = 255, .G = 255, .R = 255 };
const TGAPixel red = { .R = 255 };
const TGAPixel blue = { .B = 255 };
const TGAPixel green = { .G = 255 };


TGAImage tgaCreateImage(int width, int height)
{
    TGAImage image = {
        .header = {
            .idLength = 0,
            .colorMapType = 0,
            .imageType = 2,          // Uncompressed true color
            .firstPaletteEntry = 0,
            .numPaletteEntries = 0,
            .paletteBits = 0,
            .xOrigin = 0,
            .yOrigin = 0,   
            .width = width,
            .height = height,
            .depth = 24,              // 8 bits per pixel (grayscale)
            .descriptor = 0x20       // Upper-left origin
        }
    };

    image.pixels = calloc(width*height, sizeof(TGAPixel));

    return image;
}

void tgaSetPixel(TGAImage *image, int x, int y, TGAPixel pixel)
{
    if (!image || !image->pixels) return;
    if (x < 0 || x >= image->header.width) return;
    if (y < 0 || y >= image->header.height) return;

    image->pixels[image->header.width*y + x] = pixel;
}

void tgaSaveImage(TGAImage *image, char *path)
{
    FILE *imageFile = fopen(path, "wb");

    fwrite(&image->header, sizeof(TGAHeader), 1, imageFile);

    fwrite(image->pixels, sizeof(TGAPixel), image->header.width * image->header.height, imageFile);

    fclose(imageFile);
}

void drawLine(int x0, int y0, int x1, int y1, TGAImage *image, TGAPixel color)
{
    // https://zingl.github.io/bresenham.html
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;  // error value

    for (;;) {
        tgaSetPixel(image, x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

typedef struct {
    float x;
    float y;
} vertex2d;

int main()
{
    int imgWidth = 600;
    int imgHeight = 600;

    TGAImage image = tgaCreateImage(imgWidth, imgHeight);

    vertex2d triangle[3] = {
        { .x = 0.5, .y = 0 },
        { .x = 1, .y = 0.5 },
        { .x = 0, .y = 0.5 },
    };

    int faces[1][3] = {
        { 1,2,3 }
    };

    for (int i = 0; i < 3; i++) {
        vertex2d start = triangle[faces[0][i] - 1];
        vertex2d end = triangle[faces[0][(i+1)%3] - 1];

        printf("x:%f y:%f \n", start.x, start.y);

        drawLine(
            start.x * imgWidth, 
            start.y * imgHeight, 
            end.x * imgWidth, 
            end.y * imgHeight, 
            &image, 
            white
        );
    }

    tgaSaveImage(&image, "sample.tga");

    return 0;
}
