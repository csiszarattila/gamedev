#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "lib/tga.h"
#include "lib/wavefront_obj.h"

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

void drawTriangle(Vertex3D v0, Vertex3D v1, Vertex3D v2, TGAImage *image, TGAPixel color)
{
    drawLine(v0.x, v0.y, v1.x, v1.y, image, color);
    drawLine(v1.x, v1.y, v2.x, v2.y, image, color);
    drawLine(v2.x, v2.y, v0.x, v0.y, image, color);
}

int projectX(float x, int width)
{
    int min_x, max_x;
    float u;

    min_x = -1;
    max_x = 1;

    u = (x - min_x) / (max_x - min_x);

    return u * (width-1);
}

int projectY(float y, int height)
{
    int min_y, max_y;
    float v;

    min_y = -1;
    max_y = 1;

    v = (y - min_y) / (max_y - min_y);

    return (1-v) * (height-1);
}

int main()
{
    int imgWidth = 800;
    int imgHeight = 800;

    OBJ_Model model;

    OBJ_Model_init(&model);
 
    OBJ_Model_parse("model/cube.obj", &model);

    for (int i = 0; i < model.vertexSize; i++) {
        printf("x:%.9f, y:%.9f \n", model.vertexData[i].x, model.vertexData[i].y);
    }

    for (int i = 0; i < model.faceSize; i++) {
        printf("%d->%d->%d \n", model.faceData[i].v0, model.faceData[i].v1, model.faceData[i].v2);
    }

    TGAImage image = tgaCreateImage(imgWidth, imgHeight);

    Mesh mesh = OBJ_Model_mesh(&model);
    for (int i = 0; i < mesh.trisSize; i++) {
        Triangle triangle = mesh.tris[i];

        Vertex3D v0 = triangle.v0;
        Vertex3D v1 = triangle.v1;
        Vertex3D v2 = triangle.v2;
        
        v0.x = projectX(v0.x, imgWidth);
        v0.y = projectY(v0.y, imgWidth);
        v1.x = projectX(v1.x, imgWidth);
        v1.y = projectY(v1.y, imgWidth);
        v2.x = projectX(v2.x, imgWidth);
        v2.y = projectY(v2.y, imgWidth);

        drawTriangle(v0, v1, v2, &image, red);
    }

    tgaSaveImage(&image, "sample.tga");

    return 0;
}

