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

If you're wondering when you should ever use `mem-sysinfo` over `mem-proc`:

- If you want the usage calculated based on _free_, not _available_ memory
- `mem-sysinfo` is unaffected by possible future changes to `/proc/meminfo`
- `mem-sysinfo` seems to be a little faster than `mem-proc`

## Dependencies

None.

## Building

- Make sure `gcc` is installed
- Run the included `build` script

## Usage

    mem-sysinfo [OPTIONS...]

- `-h` print usage information, then exit
- `-i INTERVAL` seconds between checking for a change in value; default is `1`
- `-m` keep running and print when there is a visible change in value 
- `-p PRECISION` number of decimal digits to include in the output
- `-s` print a space between the value and unit
- `-t THRESHOLD` required change in value in order to print again; default is `1`
- `-u` add the percentage sign (`" %"`) to the output

