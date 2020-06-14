# volume-pulse

This is a small utility that will try to print the volume level of the _default_ PulseAudio sound device (sink).

## Dependencies

- `libpulse`

## Usage

- `-h` Print this help text and exit.
- `-m` Enables monitor mode, where the volume is printed every time it changes.
- `-p NUMBER` Number of decimal places in the output.
- `-s` Print a space between value and percent sign.
- `-u` Include the percent sign in the output.
- `-w STRING` Print this string instead of the volume level when the sink is muted.

## To do

- Add option to specify the sound device by name or number or both
- Add option to list all detected / available sound devices
