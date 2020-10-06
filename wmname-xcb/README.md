# wmname-xcb 

This is a small utility that prints the title of the focused window to `stdout`.

## Dependencies

- libxcb 

## Building

- Make sure `gcc` and all dependencies are installed
- Run the included `build` script

## Usage

    wmname-xcb [OPTIONS...]

Options:

- `b` buffer size (max string length) for the window title; defaults to 2048

