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

- `f FORMAT` Format string, see below
- `h` Print help text and exit
- `i SECS` Server query interval in seconds (default: 10)
- `m` Keep running and print after every server query
- `p PORT` Server port (default is 25565)

Output modes:

- `%%`: a literal `%`
- `%p`: players online
- `%s`: slots available
- `%v`: server version
- `%m`: message of the day

## To do 

- Only print if the received information has changed from the previous query
