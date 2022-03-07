# boox

https://user-images.githubusercontent.com/32691832/156974396-c0ae7ba5-d6bd-4d18-b450-ce3b30d61c4f.mp4

## details
`boox` is a screen region and point/window selection tool with formattable output similar to [crud](https://github.com/ix/crud), [hacksaw](https://github.com/neXromancers/hacksaw), or [slop](https://github.com/naelstrof/slop). Like these other tools, `boox` draws the selection using a window.

You can click on a window without dragging to select the geometry of that window, and the selection border will snap to windows you hover over. You can also right click to cancel the selection.

Modifier keys change how selection works in various ways:
- SHIFT: Keeps the selection size constant and instead moves the whole selection region around with your mouse
- CONTROL: Resize from the center of the selection instead of the top left
- ALT: Lock the selection to a specific axis (whichever you move in first), stacks with other modifiers

If you run with `-p` you will select a point/window.

## flags/configuration
Region selection output format can be configured with the `-f` flag or with the `BOOX_SELECTION_FORMAT` environment variable, to format output, use a string with `%x`, `%y`, `%w`, and `%h` to fill in the selection region values, you can also use `%i` for the window ID if the you selected a whole window by clicking it rather than dragging a selection, otherwise `%i` will just output 0. For example, to get the same output as crud you'd run `boox -f 'W=%w\nH=%h\nX=%x\nY=%y\nG=%wx%h+%x+%y'`. The default output is `%wx%h+%x+%y`.

For point selection format you can still use `-f` or you can use the `BOOX_POINT_FORMAT` environment variable. All the formatting options are the same, though `%w` and `%h` will always be 0. The default output is `%x %y`.

Border size can be configured with the `-b` flag or the `BOOX_BORDER_SIZE` environment variable, e.g. `boox -b 4`

Border color can be configured with the `-c` flag or the `BOOX_BORDER_COLOR` environment variable, the value should be a hex color value **without** the `#`, e.g. `boox -c ff0000`

Normally when the mouse pointer is already captured, boox will silently exit with an error, but you can use the `-w` flag to make it wait until it can capture the pointer

You can restrict the selection to a specific window with the `-r` flag. Valid values are `root` (default), `current` (the current active window as determined by `_NET_ACTIVE_WINDOW`), and a specific window ID. A pattern you may find useful is `boox -r $(boox -p -f '%i')` to first select what window you want the selection to be restricted to then start the actual selection

Some flags can also have their default values changed in `src/config.h`

## todo (maybe)
- Aspect ratio selection mode
- Grid selection mode, split the screen into a grid of rows and columns and snap the selection to grid cells, with optional padding between cells and at the edges of the screen
- Start a selection with the initial values being that of a window
- Keyboard control for selection, arrow keys/hjkl for moving the pointer, enter and/or space to start and finish the selection, alt to move by single pixel distances rather than jumping larger distances, shift and control work as they already do
- Abstract platform backend to make it easier to implement other backends in the future
- Wayland backend
