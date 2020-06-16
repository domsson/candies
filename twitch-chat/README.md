# twitch-chat 

This is a small utility that prints a Twitch channel's chat to `stdout`.
Via some simple filter options, only certain types of messages (cheers; 
messages by VIPs, subs or mods; messages containing a specific string) 
can be printed. Optionally, a maximum string length can be set.

This can be very useful if you want a notification show up everytime 
someone mentions or tags you (or says a specific word/phrase of interest) 
in a Twitch channel. 

## Dependencies

- [`libtwirc`](https://github.com/domsson/libtwirc)

## Building

- Make sure `gcc` and all dependencies are installed
- Run the included `build` script

## Usage

    twitch-chat [OPTIONS...] #channel

Note: the channel needs to start with a `#` and be all lower-case.

Options:

- `-b` Mark VIPs and subs with `+`, mods with `@`.
- `-d` Use display names instead of user names where available.
- `-f STR` Only print messages containing this exact string (case sensitive).
- `-g INT` Only print cheer messages with at least this many bits.
- `-h` Print help text and exit.
- `-n INT` Maximum message length.
- `-o` Omit user names, only print chat messages.
- `-p INT` Required user permission (0: all, 1: VIP, 2: sub, 3: mod).
