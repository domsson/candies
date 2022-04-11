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

- `-F FILE`: file to query for CPU info; default is `/proc/stat`
- `-h`: print usage information, then exit
- `-i INTERVAL`: seconds between reads from `/proc/stat`; default is `1`
- `-m`: keep running and printing
- `-p PRECISION`: number of decimals to include in the output
- `-s`: print a space between the value and unit
- `-t THRESHOLD`: prequired change in value in order to print again; default is `1`
- `-u`: add the percentage sign (`" %"`) to the output

