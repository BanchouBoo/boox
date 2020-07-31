# boox

`boox` is a screen region selection tool with formattable output similar to [crud](https://github.com/ix/crud), [hacksaw](https://github.com/neXromancers/hacksaw), or [slop](https://github.com/naelstrof/slop). Like these other tools, `boox` draws the selection using a window.

You can click on a window without dragging to select the geometry of that window, and the selection border will snap to windows you hover over. You can also right click to cancel the selection.

To format output, the first argument should be a string containing `%x`, `%y`, `%w`, and `%h` to fill in the selection values. For example, to get the same output as crud you'd run `boox 'W=%w\nH=%h\nX=%x\nY=%y\nG=%wx%h+%x+%y'`. The default output is `%wx%h+%x+%y` and may be changed in `src/config.h` along with the border color and border size.

Example usage video: https://imgur.com/a/x5FcLbQ

# thanks

[crud's](https://github.com/ix/crud) source code for teaching me how to use the shape api

[Sweets](https://github.com/Sweets) for helping with learning C and xcb stuff
