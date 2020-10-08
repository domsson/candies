# gtao-weather 

A small tool that prints GTA online's current time and weather to `stdout`.  
Heavily based on the code from the [GTAWeather](https://github.com/adam10603/GTAWeather) project, although [the GTA Weather Cam](https://gtaweather.herokuapp.com/weather) also came in handy.

## Dependencies

- None

## Building

- Make sure `gcc` and all dependencies are installed
- Run the included `build` script

## Usage

    gtao-weather [OPTIONS...]

Options:

- `b INT` Buffer size for the returned string (default: 256)
- `f FORMAT` Format string, see below
- `h` Print help text and exit
- `i SECS` Print every SECS seconds (in conjunction with `m`, default: 1)
- `m` Keep running and printing

Format specifiers:

- `%%`: a literal `%`
- `%t`: current GTA online time (e.g. "23:45")
- `%w`: current GTA online weather (e.g. "Mostly cloudy")

