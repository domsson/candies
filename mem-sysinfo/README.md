# mem-sysinfo 

This is a small utility that tries to print current memory usage to `stdout`.
It is intended to be used with status bars like `Lemonbar` and/or their 
wrappers, like [`succade`](https://github.com/domsson/succade).

## Concept 

The tool gets information about the amount of total and free memory via 
`sysinfo()` and calculates the current memory usage from it. Using `sysinfo()` 
means that this tool is Linux-specific (not portable). Also, note that the 
_free_ memory reported by `sysinfo()` will not give an accurate value for how 
much memory is available to start new applications: Linux uses a certain amount 
of unused memory for caches/buffers. These can (and will) be freed and made 
available if an application needs more memory than is currently ununsed. The 
amount of memory theoretically _available_ for applications is not provided by 
`sysinfo()`. In other words, while this tool accurately shows the current 
memory _usage_, this value is not what most users would intuitively expect as 
such. To get the memory usage based on the _available_ memory, use the 
`mem-proc` tool instead. 

## Dependencies

None.

## Building

- Make sure `gcc` are installed
- Run the included `build` script

## Usage

    mem-sysinfo [-h] [-i INTERVAL] [-m] [-p NUM] [-s] [-u]

- `-h` print usage information, then exit
- `-i` time between memory checks/prints, in seconds
- `-m` keep running and printing every second (or every INTERVAL seconds)
- `-p` precision: number of decimals to include in the output
- `-s` print a space between the value and unit
- `-u` add the percentage sign (`" %"`) to the output

