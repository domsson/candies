# mc-server 

This is a small utility that prints Minecraft server information to `stdout`.
It can print any of the follwing information for servers running 1.4 or newer:

- Server version
- Server MOTD (message of the day)
- Players currently online
- Player slots available

## Dependencies

- None

## Building

- Make sure `gcc` and all dependencies are installed
- Run the included `build` script

## Usage

    mc-server [OPTIONS...] server_ip

Options:

- `h` Print help text and exit
- `i SECS` Server query interval in seconds (default: 10).
- `m` Keep running and print after every server query.
- `o MODE` Output mode (0 through 4, see below).
- `p PORT` Server port (default is 25565).
- `s` Add spaces before and after the slash (mode 0 only)

Output modes:

- `0`: `<players> / <slots>`
- `1`: `<players>`
- `2`: `<slots>`
- `3`: `<version>`
- `4`: `<message of the day>`

## To do 

- Only print if the received information has changed from the previous query
