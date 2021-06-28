# mem-proc

This is a small utility that prints memory (RAM) information to `stdout`.
It is intended to be used with status bars like `Lemonbar` and/or their 
wrappers, like [`succade`](https://github.com/domsson/succade).

## Concept 

The tool gets information about the amount of total and available memory from 
`/proc/meminfo`, then calculates the current memory usage from it. For this, it 
looks at the values `MemTotal`, `MemAvailable` and `MemFree`. Based on these, 
the tool will also calculate _used_ and _bound_ memory (see below for details).

## Terminology

The difference between _available_ and _free_ memory is that the former gives
a more intuitive value. That is because _available_ memory takes into account 
memory that is currently used by Linux, but can and will be freed as soon as 
any application needs it. In summary:

 - **Availble**: memory theoretically available (via freeing)
 - **Free**: memory practically and immediately available (without freeing)
 - **Used**: total minus available memory (memory in use by applications)
 - **Bound**: total minus free memory (memory in use by applications and Linux)

## Dependencies

None (other than standard libraries and `gcc`).

## Building

- Run the included `build` script

## Usage

    mem-sysinfo [OPTIONS...]

- `-f FORMAT` format string for the output (see below); default is `%u`
- `-F FILE` file to query for memory info; default is `/proc/meminfo`
- `-h` print usage information, then exit
- `-i INTERVAL` seconds between checking for a change in value; default is `1`
- `-m` keep running and print when there is a visible change in value
- `-p PRECISION` number of decimal digits to include in the output; default is `0`
- `-s` print a space between the value and unit
- `-t THRESHOLD` required change in value in order to print again; default is `1`
- `-u` add the unit (`%` or `GiB` respectively) to the output

### Format specifiers

- `%T` total memory in GiB
- `%A` available memory in GiB
- `%F` free memory in GiB
- `%U` used memory in GiB
- `%B` bound memory in GiB
- `%t` total memory in percent
- `%a` available memory in percent
- `%f` free memory in percent
- `%u` used memory in percent
- `%b` bound memory in percent

