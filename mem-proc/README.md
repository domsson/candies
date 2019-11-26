# mem-proc

This is a small utility that tries to print current memory usage to `stdout`.
It is intended to be used with status bars like `Lemonbar` and/or their 
wrappers, like [`succade`](https://github.com/domsson/succade).

## Concept 

The tool gets information about the amount of total and available memory from 
`/proc/meminfo`, then calculates the current memory usage from it. For this, it 
looks at the values `MemTotal` and `MemAvailable`, although `MemFree` can be 
used instead via a command-line argument, if so desired. The difference is that 
`MemAvailable` gives a more intuitive memory usage, as it accounts for memory 
that is currently used by Linux, but can and will be freed as soon as any 
application needs it. Using `MemFree`, on the other hand, the tool will show a 
higher memory usage, as that _used_ but _available_ memory will be treated as 
if unavailable.

## Dependencies

None.

## Building

- Make sure `gcc` is installed
- Run the included `build` script

## Usage

    mem-sysinfo [OPTIONS...]

- `-f FILE` file to query for memory info; default is `/proc/meminfo`
- `-g` show gross usage, using `MemFree` instead of `MemAvailable`
- `-h` print usage information, then exit
- `-i INTERVAL` seconds between checking for a change in value; default is `1`
- `-m` keep running and print when there is a visible change in value
- `-p PRECISION` number of decimal digits to include in the output; default is `0`
- `-s` print a space between the value and unit
- `-t THRESHOLD` required change in value in order to print again; default is `1`
- `-u` add the percentage sign (`" %"`) to the output

