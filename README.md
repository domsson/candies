# candies

A collection of simple programs and scripts that can print some (system) information to `stdout`.   
Examples for such information would be CPU usage, CPU temperature, volume level and battery level.

These can then be used to gather and/or pipe information to a status bar like Lemonbar.  
Personally, I use these for [succade](https://github.com/domsson/succade), a Lemonbar wrapper I wrote.

Check out the subdirectories for more information on each individual _candy_.

## Concept

For the sake of consistency, all _candies_ should follow these design principles:

- Simple program/script that returns as soon as possible
- If possible, compiled native binary or shell script
- Set `stdout` to line buffering before printing the result
- On success, print the result and a line break to `stdout`
- On error, print nothing, just return with `EXIT_FAILURE`
- By default, omit units (`%`, `°C`, etc) in the output
- Enable printing of units, if applicable, with a `-u` argument
- Omit the space between value and unit unless `-s` is given
- Only print other output when requested by user via arguments
- Candies that can monitor should enable this feature with `-m` (see below)

## Monitoring

Monitoring refers to a candy's capability to keep running and printing its result over and over again (ideally only when the output has changed). If this option is present, it should be made available with the `-m` command line switch.

An example would be a CPU temperature program that prints the current temperature once when initially run, then prints it again whenever the temperature changes. Each print should be terminated with a line break (`\n`).

Monitoring candies might want to optionally offer these two command line arguments:

- `-t` to set the value-change threshold required to print again
- `-i` to set the internal update interval (seconds between checking values)

## License

This is all public domain, go bonkers, have fun. Or don't, I'm not your boss.
