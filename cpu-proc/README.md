# cpu-proc 

This is a small utility that tries to print the current CPU usage to `stdout`.
It is intended to be used with status bars like `Lemonbar` and/or their 
wrappers, like [`succade`](https://github.com/domsson/succade).

## Concept 

The tool reads the CPU statistics from `/proc/stat` two times, with a small
wait in between, then calculates the current CPU usage from it, as outlined in 
an [article on rosettacode](https://rosettacode.org/wiki/Linux_CPU_utilization).

## Dependencies

None.

## Building

- Make sure `gcc` are installed
- Run the included `build` script

## Usage

    cpu-proc [-h] [-i INTERVAL] [-m] [-p NUM] [-s] [-u]

- `-h` print usage information, then exit
- `-i` time between reads from `/proc/stat` and therefore between prints, in seconds
- `-m` keep running and printing every second (or every INTERVAL seconds)
- `-p` precision: number of decimals to include in the output
- `-s` print a space between the value and unit
- `-u` add the percentage sign (`" %"`) to the output

