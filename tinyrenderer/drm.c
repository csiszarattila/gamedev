#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include <drm.h>
#include <drm_mode.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

#define DRM_DEVICE "/dev/dri/card1"

typedef struct {
    uint32_t *data;
    int width;
    int height;
} FrameBuffer;

typedef struct {
    uint8_t R;
    uint8_t G;
    uint8_t B;
} FrameBufferPixel;

void setPixel(FrameBuffer *buffer, int x, int y, FrameBufferPixel color)
{
    buffer->data[width * y + x] = (color.R << 16) | (color.G << 8) | color.B;
}

void drawLine(int x0, int y0, int x1, int y1, uint32_t *fb_data, int width, TGAPixel color)
{
    // https://zingl.github.io/bresenham.html
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;  // error value

    for (;;) {
        
        fb_data[width*y0 + x0] = (color.R << 16) | (color.G << 8) | color.B;

        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

int main() {
    int fd;
    drmModeRes *res;
    drmModeConnector *connector = NULL;
    drmModeEncoder *encoder = NULL;
    drmModeCrtc *original_crtc = NULL;
    uint32_t fb_id = 0;
    struct drm_mode_create_dumb create_dumb = {0};
    struct drm_mode_map_dumb map_dumb = {0};
    void *fb_data = NULL;

 // 1. Open DRM device
    fd = open(DRM_DEVICE, O_RDWR);
    if (fd < 0) {
        perror("Failed to open DRM device");
        return 1;
    }

    // 2. Get DRM resources
    res = drmModeGetResources(fd);
    if (!res) {
        perror("Failed to get DRM resources");
        close(fd);
        return 1;
    }

    // 3. Find first connected connector
    for (int i = 0; i < res->count_connectors; i++) {
        connector = drmModeGetConnector(fd, res->connectors[i]);
        if (connector->connection == DRM_MODE_CONNECTED) {
            break;
        }
        drmModeFreeConnector(connector);
        connector = NULL;
    }
    if (!connector) {
        fprintf(stderr, "No connected connectors found\n");
        drmModeFreeResources(res);
        close(fd);
        return 1;
    }

    // 4. Get encoder and original CRTC
    encoder = drmModeGetEncoder(fd, connector->encoder_id);
    if (!encoder || !encoder->crtc_id) {
        fprintf(stderr, "Failed to get encoder or CRTC\n");
        goto cleanup;
    }

    original_crtc = drmModeGetCrtc(fd, encoder->crtc_id);
    if (!original_crtc) {
        perror("Failed to get original CRTC");
        goto cleanup;
    }

    // 5. Create dumb buffer
    create_dumb.width = connector->modes[0].hdisplay;
    create_dumb.height = connector->modes[0].vdisplay;
    create_dumb.bpp = 32;
    if (drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &create_dumb) < 0) {
        perror("Failed to create dumb buffer");
        goto cleanup;
    }

    // 6. Create framebuffer
    if (drmModeAddFB(fd, create_dumb.width, create_dumb.height, 
                    24, 32, create_dumb.pitch, create_dumb.handle, &fb_id) < 0) {
        perror("Failed to add framebuffer");
        goto cleanup;
    }

    // 7. Map framebuffer
    map_dumb.handle = create_dumb.handle;
    if (drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &map_dumb) < 0) {
        perror("Failed to map dumb buffer");
        goto cleanup;
    }
    fb_data = mmap(0, create_dumb.size, PROT_WRITE, 
                  MAP_SHARED, fd, map_dumb.offset);
    if (fb_data == MAP_FAILED) {
        perror("Failed to mmap framebuffer");
        goto cleanup;
    }

    // 8. Draw pattern (simple gradient)
    // for (int y = 0; y < create_dumb.height; y++) {
    //     for (int x = 0; x < create_dumb.width; x++) {
    //         uint8_t r = x * 255 / create_dumb.width;
    //         uint8_t g = y * 255 / create_dumb.height;
    //        // fb_data[y * create_dumb.width + x] = (r << 16) | (g << 8) | 0xFF;
    //     }
    // }

    // 9. Set new CRTC
    if (drmModeSetCrtc(fd, original_crtc->crtc_id, fb_id, 
                      0, 0, &connector->connector_id, 1, &connector->modes[0]) < 0) {
        perror("Failed to set CRTC");
        goto cleanup;
    }
    
    #define COLOR_WHITE 0x00FFFFFF  // XRGB8888 format

    for (int x = 0; x < create_dumb.width; x += 10) {
    // Draw vertical line
    for (int y = 0; y < create_dumb.height; y++) {
        uint32_t *row = (uint32_t*)((char*)fb_data + y * create_dumb.pitch);
        for (int dx = 0; dx < 10; dx++) {
            if (x + dx < create_dumb.width) {
                row[x + dx] = COLOR_WHITE;
            }
        }
    }

    // Flush and notify
    // msync(fb_data, create_dumb.size, MS_SYNC);
    // drmModeClip clip = {.x1 = x, .y1 = 0, .x2 = x+10, .y2 = create_dumb.height};
    // drmModeDirtyFB(fd, fb_id, &clip, 1);
    drmModeDirtyFB(fd, fb_id, NULL, 0);

    sleep(1);
}

    // Wait for user input before exiting
    printf("Press Enter to exit...\n");
    //getchar();

cleanup:
    // 10. Restore original CRTC state
    if (original_crtc) {
        drmModeSetCrtc(fd, original_crtc->crtc_id, original_crtc->buffer_id,
                      original_crtc->x, original_crtc->y,
                      &connector->connector_id, 1, &original_crtc->mode);
        drmModeFreeCrtc(original_crtc);
    }

    // Cleanup resources
    if (fb_data) munmap(fb_data, create_dumb.size);
    if (fb_id) drmModeRmFB(fd, fb_id);
    struct drm_mode_destroy_dumb destroy_dumb = {.handle = create_dumb.handle};
    drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy_dumb);
    if (encoder) drmModeFreeEncoder(encoder);
    if (connector) drmModeFreeConnector(connector);
    if (res) drmModeFreeResources(res);
    close(fd);
    return 0;
}
