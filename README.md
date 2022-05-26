# candies

A collection of lightweight programs and scripts that can print some 
(system) information to `stdout`. Examples for such information would be 
CPU usage, CPU temperature, volume level and battery level and similar.

These can then be used to gather and/ or pipe information to a status bar, 
notifications, fetch scripts, monitoring tools and many more.

Check out the subdirectories for more information on each individual _candy_.

## Concept

For the sake of consistency, all candies should follow these design principles:

- Lightweight program/ script that returns as soon as possible
- Set `stdout` to line buffering before printing the result
- On success, print the result and a line break to `stdout`
- On error, print nothing, just return with `EXIT_FAILURE`
- By default, just print the raw result without label or units
- Only print other output when requested by user via arguments

## Monitoring

Monitoring refers to a candy's capability to keep running and printing its 
result over and over again, but only if the output has changed. 

An example would be a CPU temperature program that prints the current 
temperature once when initially run, then prints it again whenever the 
temperature has changed.

## Common/ reserved command line flags

Where applicable, the following command line arguments should be available 
for all candies and should hence be reserved for these purposes, if possible:

 - `-h`: print help/ usage information and exit
 - `-V`: print version info and exit
 - `-u`: add the appropriate unit (`%`, `°C`, etc) to the output
 - `-s`: add a space between the value and the unit
 - `-p PRECISION`: number of decimals to include in the output
 - `-f FORMAT`: format string for the output, using custom format specifiers
 - `-m`: monitoring (keep running and printing upon change of value)
 - `-k`: keep printing even if the output hasn't changed (only in combination with `-m`)
 - `-i INTERVAL`: interval, in seconds, between probing/ printing of data (only in combination with `-m`)

## License

This is all public domain, go bonkers, have fun. Or don't, I'm not your boss.
