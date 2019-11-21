# datetime 

This is a small utility that prints the current date and/or time `stdout`. 
It is intended to be used with status bars like `Lemonbar` and/or their 
wrappers, like [`succade`](https://github.com/domsson/succade).

## Concept 

Similar to `date`, this tool can print the current date and time. The main 
differences to `date` are a different default format and the option to keep 
the tool running, having it print over and over again in a set interval.

## Dependencies

None.

## Building

- Make sure `gcc` is installed
- Run the included `build` script

## Usage

    datetime [-h] [-g] [-m] [-f FORMAT] [-i INTERVAL] [-o OFFSET]

- `-h` print usage information, then exit
- `-g` use UTC/GMT instead of local time
- `-m` keep running, printing every seconds (or every INTERVAL seconds)

For possible `FORMAT` strings, check the manpage for `date` or `strftime`.

`INTERVAL` can be used to specify every how many seconds the date/time string 
is to be printed to `stdout`. If you don't plan on displaying seconds in your 
output, it would be wise to set this to `60`, in order to not waste resources.

If `OFFSET` is given, it will be treated as hours and added to the time 
obtained from the system (local or UTC, depending on the options).
