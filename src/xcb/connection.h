#ifndef CONNECTION_H
#define CONNECTION_H

#include <xcb/xcb.h>

extern xcb_connection_t *xcb_connection;
extern xcb_screen_t *xcb_screen;

int xcb_initialize(int wait_until_cursor_grabbable, char *constraining_window_tag);
void xcb_finalize(void);
void flush(void);

#endif
