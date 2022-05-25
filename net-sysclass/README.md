# ney-sysclass 

This is a small utility that tries to print the current network usage to `stdout`. For continous monitoring, there is no need to run the tool over and over again, just use `-m` instead.

## Concept 

The tool reads the received and transmitted bytes from `/sys/class/net/<iface>` 
two times, with a small wait in between, then calculates the current network 
usage from the difference.

## Dependencies

None, apart from standard libraries and gcc for compiling.

## Building

- Make sure `gcc` is installed
- Run the included `build` script

## Usage

    net-sysclass -I <interface> [OPTIONS...]

- `-f FORMAT`: format string for the output, see below; default is `%c`
- `-g GRANULARITY`: data unit to use (`k` for kbit, `m` for Mbit, etc); default is `k`
- `-h`: print usage information, then exit
- `-i INTERVAL`: seconds between probing for network usage; default is `1`
- `-I INTERFACE`: (required) name of the network interface to monitor 
- `-k`: keep printing, regardles of whether or not the ouput has changed 
- `-m`: keep running and printing
- `-p PRECISION`: number of decimals to include in the output
- `-r RATE`: maximum throughput speed of the network interface in Mbps; default is `100`
- `-s`: print a space between the value and unit
- `-u`: add the appropriate unit to the output (`%`, `kbps`, etc)
- `-V`: print version info and exit

### Format specifiers

- `%r`: received bytes (aka download), relative to interface max throughput (%)
- `%t`: transmitted bytes (aka upload), relative to interface max throughput (%)
- `%c`: combined bytes (aka up & down), relative to interface max throughput (%)
- `%R`: received bytes (aka download), absolute
- `%T`: transmitted bytes (aka upload), absolute
- `%C`: combined bytes (aka up & down), absolute


