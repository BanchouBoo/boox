#include <string.h>
#include <stdio.h>

#include "parsing.h"
#include "../xcb/selection.h"

char *format_output(char *src)
{
    static char buffer[4096];

    int offset = 0;
    while (*src) {
        char c = src[0];
        if (c == '%') {
            char nc = src[1];
            switch (nc) {
                case 'w': {
                    if (selection_mode == MODE_SELECT) {
                        offset += sprintf(buffer+offset, "%d", selection.w);
                        src++;
                    } else {
                        offset += sprintf(buffer+offset, "%d", point_selection.window);
                        src++;
                    }
                } break;
                case 'h': {
                    if (selection_mode == MODE_SELECT) {
                        offset += sprintf(buffer+offset, "%d", selection.h);
                        src++;
                    } else {
                        strncpy(buffer+offset, src, 1);
                        offset++;
                    }
                } break;
                case 'x': {
                    if (selection_mode == MODE_SELECT) {
                        offset += sprintf(buffer+offset, "%d", selection.x);
                        src++;
                    } else {
                        offset += sprintf(buffer+offset, "%d", point_selection.point.x);
                        src++;
                    }
                } break;
                case 'y': {
                    if (selection_mode == MODE_SELECT) {
                        offset += sprintf(buffer+offset, "%d", selection.y);
                        src++;
                    } else {
                        offset += sprintf(buffer+offset, "%d", point_selection.point.y);
                        src++;
                    }
                } break;
                default: {
                    strncpy(buffer+offset, src, 1);
                    offset++;
                } break;
            }
        } else if (c == '\\') {
            char nc = src[1];
            switch (nc) {
                case 'n': {
                    offset += sprintf(buffer+offset, "\n");
                    src++;
                } break;
                case 't': {
                    offset += sprintf(buffer+offset, "\t");
                    src++;
                } break;
                default: {
                    strncpy(buffer+offset, src, 1);
                    offset++;
                } break;
            }
        } else {
            strncpy(buffer+offset, src, 1);
            offset++;
        }
        src++;
    }
    return buffer;
}

void formatted_print(char *src)
{
    puts(format_output(src));
}
