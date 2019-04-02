# temps

This is a small utility that will try to print current system tempreatures to `stdout`.  
It is intended to be used with status bars like `Lemonbar` and/or their wrappers, like [`succade`](https://github.com/domsson/succade).

I'm using it to print CPU temperature, which is the common use-case, but I believe the tool 
would also be able to query other temperature sensors (_chips_), if available. See usage.

## Concept 

`temps` uses `libsensors` to check for available temperature readings. 
For this, it iterates available _chips_ and their _features_.
If no chip prefix is given, the first one found will be used.
If no feature label is given, it will look for "Core" or "Package" features. 
This should accomodate for most intel CPUs. I don't currently have an AMD system available. 
In case you have one, I'd appreciate if you'd let me know what we should look for on those.
If a reading could be obtained, it is printed to `stdout` (including a newline), 
otherwise `temps` does nothing.

## Dependencies

- `libsensors`

## Building

- Make sure `libsensors-dev` and `gcc` are installed
- Run the included `build` script

## Usage

    temps [-l] [-v] [-c <chip_prefix>] [-f <feature_label>]

- `-l` list all available chips that can be queried, then exit
- `-v` enables verbose mode that prints additional information to `stderr`
- `-c` lets you specify part of the chip prefix from which to gather the temperatures, for example `coretemp`
- `-f` lets you specify part of the feature label to get the temperature from, for example `Package`

Note that `-c` and `-f` currently use `strstr()` internally, which means the given string will be searched for in the actual chip/feature name. In other words, `-c core` will match `coretemp`, as `core` is a substring of `coretemp`.

## To do

- Add `-u` option (if given, also print the temperature unit)
- Add `-m` option (keep running, print whenever temperature change detected)
