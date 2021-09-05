#include <xcb/xcb.h>
#include <xcb/xcb_cursor.h>
#include <stdlib.h>
#include <string.h>

#include "connection.h"
#include "selection.h"

xcb_connection_t *xcb_connection;
xcb_screen_t *xcb_screen;

int xcb_initialize(int wait_until_cursor_grabbable, char *constraining_window_tag)
{
    int values[4];
    int mask;

    if (xcb_connection_has_error(xcb_connection = xcb_connect(NULL, NULL)))
        return 0;

    xcb_screen = xcb_setup_roots_iterator(xcb_get_setup(xcb_connection)).data;

    xcb_cursor_context_t *ctx;
    xcb_cursor_t cursor = XCB_NONE;
    if (xcb_cursor_context_new(xcb_connection, xcb_screen, &ctx) >= 0) {
        cursor = xcb_cursor_load_cursor(ctx, "crosshair");
    }

    xcb_grab_pointer_cookie_t grab_pointer_cookie;
    xcb_grab_pointer_reply_t *pointer_reply;

    if (strcmp(constraining_window_tag, "root") == 0) {
        constraining_window = xcb_screen->root;
    } else if (strcmp(constraining_window_tag, "current") == 0) {
        xcb_intern_atom_cookie_t focused_atom_cookie = xcb_intern_atom(xcb_connection, 1,
            18, "_NET_ACTIVE_WINDOW");
        xcb_intern_atom_reply_t *focused_atom = xcb_intern_atom_reply(xcb_connection, focused_atom_cookie, NULL);
        if (focused_atom) {
            xcb_get_property_cookie_t property_cookie = xcb_get_property(xcb_connection, 0,
                xcb_screen->root, focused_atom->atom, XCB_ATOM_WINDOW, 0, 1);
            xcb_get_property_reply_t *property = xcb_get_property_reply(xcb_connection, property_cookie, NULL);
            if (property)
                constraining_window = *(xcb_window_t*)xcb_get_property_value(property);
        }
    } else {
        constraining_window = (xcb_window_t)strtol(constraining_window_tag, NULL, 10);
    }
    constraining_window = constraining_window ? constraining_window : xcb_screen->root;

    do {
        grab_pointer_cookie = xcb_grab_pointer(xcb_connection, 0,
            xcb_screen->root, XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE |
            XCB_EVENT_MASK_POINTER_MOTION, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC,
            xcb_screen->root, cursor, XCB_CURRENT_TIME);

        pointer_reply = xcb_grab_pointer_reply(xcb_connection,
            grab_pointer_cookie, NULL);

    } while (wait_until_cursor_grabbable && pointer_reply->status != XCB_GRAB_STATUS_SUCCESS);

    if (pointer_reply && pointer_reply->status == XCB_GRAB_STATUS_ALREADY_GRABBED)
        return 0;

    // grab alt key (64)
    xcb_grab_key(xcb_connection, 0, xcb_screen->root, XCB_MOD_MASK_ANY, 64, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);

    xcb_free_cursor(xcb_connection, cursor);

    values[0] = XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY;
    mask = XCB_CW_EVENT_MASK;
    xcb_change_window_attributes_checked(xcb_connection, xcb_screen->root, mask, values);

    selection_window_initialize();

    flush();

    return 1;
}

void xcb_finalize(void)
{
    if (xcb_connection) {
        xcb_kill_client(xcb_connection, selection_window);
        xcb_disconnect(xcb_connection);
    }
}

void flush()
{
    xcb_flush(xcb_connection);
}
