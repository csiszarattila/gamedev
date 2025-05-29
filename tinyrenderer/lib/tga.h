#ifndef TGA_H
#define TGA_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

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

#pragma pack(push, 1)
typedef struct {
    uint8_t B;
    uint8_t G;
    uint8_t R;
} TGAPixel;
#pragma pack(pop)

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

#endif