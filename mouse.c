#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/input.h>

int main()
{
    int mouse_fd = open("/dev/input/event15", O_RDONLY | O_NONBLOCK);
    if (mouse_fd < 0) {
        fprintf(stderr, "Failed to open mouse handler");
        return -1;
    }

    struct input_event ev;

    for(;;) {
        if (read(mouse_fd, &ev, sizeof(ev)) == sizeof(ev)) {
            if (ev.type == EV_ABS) {
                printf("time:%d type: %x, code:%d, value: %d\n", ev.time, ev.type, ev.code, ev.value);

                if (ev.code == ABS_X) {
                    printf("X value: %d\n", ev.value);
                }

                if (ev.code == ABS_MT_POSITION_X) {
                    printf("MT X value: %d\n", ev.value);
                }
            }
        }
        //printf("x:%d y:%d\n", event.time);
    }
}