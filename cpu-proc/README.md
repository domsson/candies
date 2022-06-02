# cpu-proc 

This is a small utility that tries to print the current CPU usage to `stdout`. For continous monitoring, there is no need to run the tool over and over again, just use `-m` instead.

## Concept 

The tool reads the CPU statistics from `/proc/stat` two times, with a small
wait in between, then calculates the current CPU usage from it, as outlined in 
an [article on rosettacode](https://rosettacode.org/wiki/Linux_CPU_utilization).

## Dependencies

 - `gcc` for compiling
 - `libc-dev` for standard libraries

## Building

- Make sure `gcc` is installed
- Run the included `build` script

## Usage

    cpu-proc [OPTION...]

- `-F FILE`: file to query for CPU info; default is `/proc/stat`
- `-h`: print usage information, then exit
- `-i INTERVAL`: seconds between reads from `/proc/stat`; default is `1`
- `-k`: keep printing, regardles of threshold
- `-m`: keep running and printing
- `-p PRECISION`: number of decimals to include in the output
- `-s`: print a space between the value and unit
- `-t THRESHOLD`: prequired change in value in order to print again; default is `1`
- `-u`: add the percentage sign (`" %"`) to the output
- `-V`: print version info and exit

## Examples

Print CPU usage with percent sign and two decimals of precision:

    $ ./cpu-proc -us -p 2
    3.12 %


Print CPU usage as whole number; print again when usage changed by 2% or more:

    $ ./cpu-proc -m -t 2
    3
    5
    1

