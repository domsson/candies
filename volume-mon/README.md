# volume-mon

Simple tool that monitors changes of the default PulseAudio sink.  
Prints `"change detected\n"` to `stdout` when a change is detected.  
Such changes could be volume level or mute state. 

## Dependencies

- `libpulse` (`libpulse-dev` in Debian)

## To do

- Add option to specify the notification text that will be printed
- Add option to specify the sink to be monitored by name, index or both
- Add option to list the detected sinks, maybe
