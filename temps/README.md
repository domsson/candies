# temps

This is a small utility that will try to print current system tempreatures to `stdout`.  
It is intended to be used with status bars like `Lemonbar` and/or their wrappers, like [`succade`](https://github.com/domsson/succade).

I'm using it to print CPU temperature, which is the common use-case, but I believe the tool 
would also be able to query other temperature sensors (_chips_), if available. See usage.  
`temps` is generally pretty similar to the well-known `sensors`, but tailored for status bars.

## Concept 

`temps` uses `libsensors` to check for available temperature readings. 
For this, it iterates available _chips_ and their _features_, looking for the  
chip and feature name given via `-c` and `-f` respectively. All features that 
match (using `strstr()`) the given names will be read for their temperature,  
those values added up and, at the end, divided by the number of features used.  
Only the first chip that matches the given name will be taken into account.  
If a reading could be obtained, it is printed to `stdout` (including a newline), 
otherwise `temps` does nothing.

## Dependencies

- `libsensors`

## Building

- Make sure `libsensors-dev` and `gcc` are installed
- Run the included `build` script

## Usage

    temps [-h] [-l] [-m] [-p NUM] [-u] [-v] -c CHIP -f FEATURE

- `-h` print usage information, then exit
- `-l` list all available chips and their features, then exit
- `-m` monitor mode (keep running, print temperature as it changes) -- NOT YET IMPLEMENTED
- `-p` precision: number of decimals to include in the output
- `-u` add the Celcius unit (`" °C"`) to the output
- `-v` enable verbose mode (prints additional information to `stderr`)

Note that `-c` and `-f` currently use `strstr()` internally, which means the given string will be searched for in the actual chip/feature name. In other words, `-c core` will match `coretemp`, as `core` is a substring of `coretemp`.

## To do

- Add `-m` option (keep running, print whenever temperature change detected)
