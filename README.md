# candies

A collection of simple programs and scripts that can print some system information to `stdout`.   
Examples for such information would be CPU usage, CPU temperature, volume level and battery level.

These can then be used to gather and/or pipe information to a status bar like Lemonbar.   
Personally, I use these for [`succade`](https://github.com/domsson/succade), a Lemonbar wrapper I wrote.

Check out the subdirectories for more information on each individual _block_ (aka _candy_).

## Concept

For the sake of consistency, all _candies_ should follow these design principles:

- Simple program/script that returns as soon as possible
- If possible, compiled native binary or shell script
- Disable `stdout` buffering before printing the result
- On success, print the result and a line break to `stdout`
- On error, print nothing, just return with `EXIT_FAILURE`
- By default, omit units (`%`, `°C`, etc) in the output
- Enable printing of units, if applicable, with a `-u` argument
- Only print other output when requested by user via arguments
- Candies that can monitor should enable this feature with `-m` (see below)

## Candies with monitoring capabilities

_Live blocks_, or monitoring candies, refers to the option to have the program keep running and print their result over and over again, whenever they detect a change in the value. If a block offers this option, it should be made available with the `-m` command line switch.

An example would be a CPU temperature program that prints the current temperature once when initially run, then prints it again whenever the temperature changes. Each print should be terminated with a line break (`\n`).

## License

This is all public domain, go bonkers, have fun. Or don't, I'm not your boss.
