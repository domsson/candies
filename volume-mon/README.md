# volume-mon

Simple tool that monitors changes of the default PulseAudio sink.  
Whenever a change is registered (usually a change of volume or mute state), 
prints `"change detected\n"` to `stdout`.

## Dependencies

- `libpulse` (`libpulse-dev` in Debian)

## To do

- Add option to specify the notification text that will be printed
- Add option to specify the sink to be monitored by name, index or both
- Add option to list the detected sinks, maybe
