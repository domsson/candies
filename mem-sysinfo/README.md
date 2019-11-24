# mem-sysinfo 

This is a small utility that tries to print current memory usage to `stdout`.
It is intended to be used with status bars like `Lemonbar` and/or their 
wrappers, like [`succade`](https://github.com/domsson/succade).

## Concept 

The tool gets information about the amount of total and free memory via 
`sysinfo()`. This means that this tool is Linux-specific (not portable).

## Dependencies

None.

## Building

- Make sure `gcc` are installed
- Run the included `build` script

## Usage

    mem-sysinfo [-h] [-i INTERVAL] [-m] [-p NUM] [-s] [-u]

- `-h` print usage information, then exit
- `-i` time between reads from `/proc/stat` and therefore between prints, in seconds
- `-m` keep running and printing every second (or every INTERVAL seconds)
- `-p` precision: number of decimals to include in the output
- `-s` print a space between the value and unit
- `-u` add the percentage sign (`" %"`) to the output

