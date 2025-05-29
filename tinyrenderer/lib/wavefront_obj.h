#ifndef WAVEFRONT_OBJ_H
#define WAVEFRONT_OBJ_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    float x;
    float y;
    float z;
} Vertex3D;

typedef union {
    uint32_t v[3];
    struct { uint32_t v0, v1, v2; };
} Face32;

typedef struct {
    Vertex3D *vertexData;
    size_t vertexCapacity;
    size_t vertexSize;

    Face32 *faceData;
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

void OBJ_Model_add_face(OBJ_Model *model, Face32 face)
{
    if (model->faceCapacity == model->faceSize) {
        model->faceCapacity += 256;
        model->faceData = realloc(model->faceData, model->faceCapacity * sizeof(Face32));
    }
    model->faceData[model->faceSize++] = face;
}

void OBJ_Model_init(OBJ_Model *model)
{
    model->vertexCapacity = 1024;
    model->vertexData = malloc(model->vertexCapacity * sizeof(Vertex3D));
    model->vertexSize = 0;

    model->faceCapacity = 256;
    model->faceData = malloc(model->faceCapacity * sizeof(Face32));
    model->faceSize = 0;
}

typedef struct {
    uint32_t v[3];
    struct { Vertex3D v0, v1, v2; };
} Triangle;

typedef struct
{
    Triangle *tris;
    size_t trisSize;
} Mesh;


Mesh OBJ_Model_mesh(OBJ_Model *model)
{
    Mesh mesh;

    mesh.trisSize = model->faceSize;
    mesh.tris = malloc(mesh.trisSize * sizeof(Triangle));

    for (int i = 0; i < model->faceSize; i++) {
        Face32 face = model->faceData[i];

        Vertex3D v0 = model->vertexData[face.v0];
        Vertex3D v1 = model->vertexData[face.v1];
        Vertex3D v2 = model->vertexData[face.v2];

        mesh.tris[i] = (Triangle) {
            .v0 = model->vertexData[face.v0],
            .v1 = model->vertexData[face.v1],
            .v2 = model->vertexData[face.v2]
        };
    }

    return mesh;
}

int OBJ_Model_parse(const char *filename, OBJ_Model *model)
{
    FILE *fd;
    char buffer[256];
    char *token;
    int vIdx;

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
            uint32_t indices[256]; // Temporary storage for face indices
            int index_count = 0;

            token = strtok(buffer, " \t\n\r");
            token = strtok(NULL, " \t\n\r"); // Skip 'f'

            // Parse all vertex indices in the face
            while (token && index_count < 256) {
                int v;
                sscanf(token, "%d", &v);
                indices[index_count++] = v - 1; // Convert to zero-based
                token = strtok(NULL, " \t\n\r");
            }

            // Triangulate polygon (n-gon to n-2 triangles)
            for (int i = 1; i < index_count - 1; i++) {
                Face32 face = {
                    .v0 = indices[0],
                    .v1 = indices[i],
                    .v2 = indices[i + 1]
                };
                OBJ_Model_add_face(model, face);

                printf("Parsed %d->%d->%d,\n", face.v0, face.v1, face.v2);
            }
        }
    }
}

#endif
