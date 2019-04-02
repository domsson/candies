# temps block

This is a small utility that will try to print the current CPU tempreature to `stdout`.
It is intended to be used with status bars like `Lemonbar` and/or their wrappers, like `succade`.

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

    temps [-v] [-c <chip_prefix>] [-f <feature_label>]

- `-l` list all available chips that can be queried, then exit
- `-v` enables verbose mode that prints additional information to `stderr`
- `-c` lets you specify part of the chip prefix from which to gather the temperatures, for example `coretemp`
- `-f` lets you specify part of the feature label to get the temperature from, for example `Package`

