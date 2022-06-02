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

    mem-proc [OPTIONS...]

- `-b` use binary instead of decimal units (MiB vs MB, etc)
- `-f FORMAT` format string for the output (see below); default is `%b`
- `-F FILE` file to query for memory info; default is `/proc/meminfo`
- `-g GRANULARITY` value granularity (`k` for KB, `m` for MB, etc)
- `-h` print usage information, then exit
- `-i INTERVAL` seconds between reading memory usage; default is `1`
- `-k` keep printing even if the ouput hasn't changed (only in combination with `-m`)
- `-m` keep running and print when there is a change in output
- `-p PRECISION` number of decimal digits to include in the output; default is `0`
- `-s` print a space between the value and unit
- `-u` add the unit (`%` or `GiB` respectively) to the output
- `-V` print version information and exit

### Format specifiers

- `%T` and `%t`: total memory, absolute and percent
- `%A` and `%a`: available memory, absolute and percent
- `%F` and `%f`: free memory, absolute and percent
- `%U` and `%u`: used memory, absolute and percent
- `%B` and `%b`: bound memory, absolute and percent

## Examples

Print the bound memory in percent, with two decimal digits and space-separated unit:

    $ ./bin/mem-proc -us -p 2
    23.11 %
   
Print the available memory in MiB, with space-separated unit but no decimal digits:

    $ ./bin/mem-proc -us -f "%A" -g m -b
    13280 MiB

Continuously print the used memory, in GB, with three decimals and space-separated unit:

    $ ./bin/mem-proc -mus -f "%U" -p 3
    3.988 GB
    3.803 GB
    3.976 GB
    4.002 GB

