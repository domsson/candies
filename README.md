# candies

A collection of simple programs and scripts that can print some system information to `stdout`.   
Examples for such information would be CPU usage, CPU temperature, volume level and battery level.

These can then be used to gather and/or pipe information to a status bar like Lemonbar.   
Personally, I use these for [`succade`](https://github.com/domsson/succade), a Lemonbar wrapper I wrote.

Check out the subdirectories for more information on each individual _block_ (aka _candy_).

# Concept

For the sake of consistency, all _candies_ should follow these design principles:

- Simple program/script that returns as soon as possible
- Disable `stdout` buffering before printing the result
- On success, print the result and a line break to `stdout`
- On error, print nothing, just return with `EXIT_FAILURE`
- By default, omit units (`%`, `°C`, etc) in the output
- Enable printing of units, if applicable, with a `-u` argument
- Only print other output when requested by user via arguments

# License

This is all public domain, go bonkers, have fun.
