# disk-statvfs 

This is a small utility that tries to print current disk usage to `stdout`.
It is intended to be used with status bars like `Lemonbar` and/or their 
wrappers, like [`succade`](https://github.com/domsson/succade).

## Concept 

The tool gets information about the amount of total and free disk space via 
`statvfs()` and calculates the current disk usage from it. 

## Dependencies

None.

## Building

- Make sure `gcc` is installed
- Run the included `build` script

## Usage

    disk-statvfs [OPTIONS...]

- `-b` use binary instead of decimal units (Gibibyte vs. Gigabyte, etc)
- `-d` a file or folder on the mount to be checked (defaults to `/`)
- `-f FORMAT` format string for the output (see below), default is `%u`
- `-g GRANULARITY` data unit size, `k` for KB, `m` for MB, etc; default is `g`
- `-h` print usage information, then exit
- `-i INTERVAL` seconds between checking for a change in value; default is `10`
- `-k` keep printing even if the output hasn't changed
- `-m` keep running and print when there is a visible change in the output 
- `-p PRECISION` number of decimal digits to include in the output
- `-s` print a space between the value and unit
- `-u` add the appropriate unit to the output
- `-V` print version information and exit

### Format specifiers

- `%T` and `%t`: disk space total (absolute and percent)
- `%F` and `%f`: disk space full (absolute and percent)
- `%U` and `%u`: disk space used (absolute and percent)
- `%A` and `%a`: disk space available to unprivileged users (absolute and percent)

