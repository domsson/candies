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

- `-a` print space available to unprivilegded users, instead of overall free space
- `-d` a file or folder on the mount to be checked (defaults to `/`)
- `-h` print usage information, then exit
- `-i INTERVAL` seconds between checking for a change in value; default is `10`
- `-m` keep running and print when there is a visible change in value 
- `-p PRECISION` number of decimal digits to include in the output
- `-s` print a space between the value and unit
- `-t THRESHOLD` required change in value in order to print again; default is `1`
- `-u` add the percentage sign (`" %"`) to the output

