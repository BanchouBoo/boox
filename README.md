# boox

## details
`boox` is a screen region and point/window selection tool with formattable output similar to [crud](https://github.com/ix/crud), [hacksaw](https://github.com/neXromancers/hacksaw), or [slop](https://github.com/naelstrof/slop). Like these other tools, `boox` draws the selection using a window.

You can click on a window without dragging to select the geometry of that window, and the selection border will snap to windows you hover over. You can also right click to cancel the selection.

Modifier keys change how selection works in various ways:
- SHIFT: Keeps the selection size constant and instead moves the whole selection region around with your mouse
- CONTROL: Resize from the center of the selection instead of the top left
- ALT: Lock the selection to a specific axis (whichever you move in first), stacks with other modifiers

If you run with `-p` you will select a point/window.

## configuration
Region selection output format can be configured with the `-f` flag or with the `BOOX_SELECTION_FORMAT` environment variable, to format output, use a string with `%x`, `%y`, `%w`, and `%h` to fill in the selection values. For example, to get the same output as crud you'd run `boox -f 'W=%w\nH=%h\nX=%x\nY=%y\nG=%wx%h+%x+%y'`. The default output is `%wx%h+%x+%y`.

For point selection format you can still use `-f` or you can use the `BOOX_POINT_FORMAT` environment variable. For the format string, you can use `%x`, `%y`, and `%w` where `%w` is the ID of the clicked window.

Border size can be configured with the `-b` flag or the `BOOX_BORDER_SIZE` environment variable, e.g. `boox -b 4`

Border color can be configured with the `-c` flag or the `BOOX_BORDER_COLOR` environment variable, the value should be a hex color value **without** the `#`, e.g. `boox -c ff0000`

All options can also have their default values changed in `src/config.h`

Example usage video: https://streamable.com/53n3fo

## todo (maybe)
- Restructure the code to better support adding different modes of selection
- Allow printing the window ID from regular selection, either the window ID of the window you selected if you don't manually drag a selection, or a delimited list of window IDs for each window that falls within the selection, maybe both depending on context or a command flag
- Aspect ratio selection mode
- Grid selection mode, split the screen into a grid of rows and columns and snap the selection to grid cells, with optional padding between cells and at the edges of the screen
- Fixed size selection mode, so you can do things like move a window with boox without resizing it
