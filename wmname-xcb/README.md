# wmname-xcb 

This is a small utility that prints the title of the focused window to `stdout`.
Alternatively, if `-r` is given, it can print the name of the window manager.

Note: wmname-xcb only works with window managers that support `_NET_WM_NAME` and 
`_NET_SUPPORTING_WM_CHECK`. For other window managers, nothing will be printed.

## Dependencies

- libxcb 

## Building

- Make sure `gcc` and all dependencies are installed
- Run the included `build` script

## Usage

    wmname-xcb [OPTIONS...]

Options:

- `-b` buffer size (max string length) for the window title; defaults to 2048
- `-r` print the window manager's name instead

