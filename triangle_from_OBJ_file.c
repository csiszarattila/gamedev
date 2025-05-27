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
    float z;
} Vertex3D;

typedef struct {
    uint8_t v0, v1, v2;
} Face8;

typedef struct {
    Vertex3D *vertexData;
    size_t vertexCapacity;
    size_t vertexSize;

    Face8 *faceData;
    size_t faceCapacity;
    size_t faceSize;
} OBJ_Model;

void OBJ_Model_add_vertex(OBJ_Model *model, Vertex3D vertex)
{
    if (model->vertexCapacity == model->vertexSize) {
        model->vertexCapacity += 1024;
        model->vertexData = realloc(model->vertexData, model->vertexCapacity * sizeof(Vertex3D));
    }
    model->vertexData[model->vertexSize++] = vertex;
}

void OBJ_Model_add_face(OBJ_Model *model, Face8 face)
{
    if (model->faceCapacity == model->faceSize) {
        model->faceCapacity += 256;
        model->faceData = realloc(model->faceData, model->faceCapacity * sizeof(Face8));
    }
    model->faceData[model->faceSize++] = face;
}

void OBJ_Model_init(OBJ_Model *model)
{
    model->vertexCapacity = 1024;
    model->vertexData = malloc(model->vertexCapacity * sizeof(Vertex3D));
    model->vertexSize = 0;

    model->faceCapacity = 256;
    model->faceData = malloc(model->faceCapacity * sizeof(Face8));
    model->faceSize = 0;
}

int OBJ_Model_parse(const char *filename, OBJ_Model *model)
{
    FILE *fd;
    char buffer[256];

    fd = fopen(filename, "r");
   
    if (!fd) {
        perror("Failed to open file");
        return -1;
    }

    while (fgets(buffer, sizeof(buffer), fd) != NULL)
    {    
        if (buffer[0] == 'v' && buffer[1] == ' ') {
            Vertex3D vertex;

            if (sscanf(buffer, "v %f %f", &vertex.x, &vertex.y)) {
                printf("Parsed x:%.9f y:%.9f\n", vertex.x, vertex.y);
                OBJ_Model_add_vertex(model, vertex);
            } else {
                fprintf(stderr, "Failed to parse vertex: %s", buffer);
            }
        }

        if (buffer[0] == 'f' && buffer[1] == ' ') {
            Face8 face;

            if (sscanf(buffer, "f %d/%d/%d", &face.v0, &face.v1, &face.v2)) {
                face.v0--; face.v1--; face.v2--; // OBJ indices start at 1
                OBJ_Model_add_face(model, face);

                printf("Parsed %d->%d->%d,\n",face.v0, face.v1, face.v2);
            } else {
                fprintf(stderr, "Failed to parse face: %s", buffer);
            }
        }
    }
}

int main()
{
    int imgWidth = 600;
    int imgHeight = 600;

    OBJ_Model triangle;

    OBJ_Model_init(&triangle);
 
    OBJ_Model_parse("triangle.obj", &triangle);

    for (int i = 0; i < triangle.vertexSize; i++) {
        printf("x:%.9f, y:%.9f \n", triangle.vertexData[i].x, triangle.vertexData[i].y);
    }

    for (int i = 0; i < triangle.faceSize; i++) {
        printf("%d->%d->%d \n", triangle.faceData[i].v0, triangle.faceData[i].v1, triangle.faceData[i].v2);
    }

    TGAImage image = tgaCreateImage(imgWidth, imgHeight);

    for (int i = 0; i < triangle.faceSize; i++) {
        Face8 face = triangle.faceData[i];

        drawLine(
            triangle.vertexData[face.v0].x * imgWidth, 
            triangle.vertexData[face.v0].y * imgHeight, 
            triangle.vertexData[face.v1].x * imgWidth, 
            triangle.vertexData[face.v1].y * imgHeight, 
            &image, 
            red
        );

        drawLine(
            triangle.vertexData[face.v1].x * imgWidth, 
            triangle.vertexData[face.v1].y * imgHeight, 
            triangle.vertexData[face.v2].x * imgWidth, 
            triangle.vertexData[face.v2].y * imgHeight, 
            &image, 
            red
        );

        drawLine(
            triangle.vertexData[face.v2].x * imgWidth, 
            triangle.vertexData[face.v2].y * imgHeight, 
            triangle.vertexData[face.v0].x * imgWidth, 
            triangle.vertexData[face.v0].y * imgHeight, 
            &image, 
            red
        );
    }

    tgaSaveImage(&image, "sample.tga");

    return 0;
}

