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
- `-m` keep running and print when there is a visible change in the output 
- `-p PRECISION` number of decimal digits to include in the output
- `-s` print a space between the value and unit
- `-u` add the percentage sign (`" %"`) to the output

### Format specifiers

- `%T` disk space total, absolute
- `%F` disk space full, absolute
- `%U` disk space used, absolute
- `%A` disk space available to unprivileged users, absolute
- `%t` disk space total, percent
- `%f` disk space full, percent
- `%u` disk space used, percent
- `%a` disk space available to unprivileged users, percent

