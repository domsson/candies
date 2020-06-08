# temp-sensors

This is a small utility that prints current system tempreatures to `stdout`.
It is intended to be used with status bars like `Lemonbar` or their wrappers, 
like [`succade`](https://github.com/domsson/succade).

I'm using it to print CPU temperature, which is the common use-case, but the 
tool should also be able to query other temperature sensors (_chips_), if 
available. See usage. `temp-sensors` is generally pretty similar to the 
well-known `sensors`, but tailored for status bars.

## Concept 

`temp-sensors` uses `libsensors` to check for available temperature readings. 
For this, it iterates available _chips_ and their _features_, looking for 
the chip and feature names given via `-c` and `-f` respectively. Temperature 
values will be read for all features that match the given feature name, using 
`strstr()` (substring search). The temperature values found this way will be 
added up and, at the end, divided by the number of temperature values found. 
Only the first chip that matches the given name will be taken into account. 
If a reading was obtained, it is printed to `stdout`, including a newline,
otherwise `temp-sensors` does nothing.

## Dependencies

- `libsensors`

## Building

- Make sure `libsensors-dev` and `gcc` are installed
- Run the included `build` script

## Usage

    temp-sensors [OPTIONS...] -c CHIP -f FEATURE

- `-e` use imperial units instead of metric
- `-h` print usage information, then exit
- `-i INTERVAL` seconds between checking for a change in value
- `-l` list all available chips and their features, then exit
- `-m` keep running and print when there is a visible change in value
- `-p PREVISION` number of decimal digits to include in the output
- `-s` print a space between the value and unit
- `-t THRESHOLD` required change in value in order to print again
- `-u` add the temperature unit to the output

Note that `-c` and `-f` currently use `strstr()` internally, which means the given string will be searched for in the actual chip/feature name. In other words, `-c core` will match `coretemp`, as `core` is a substring of `coretemp`. Also note that the string comparison is case sensitive, so `core` is not the same as `Core`.

