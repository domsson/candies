# datetime 

This is a small utility that prints the current date and/or time to `stdout`. 
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

- `FORMAT` defines the output string, see the manpage for `date` or `strftime`
- `INTERVAL` (seconds) specifies how often to print to `stdout`
- `OFFSET` (hours) will be added to the time obtained from the system
