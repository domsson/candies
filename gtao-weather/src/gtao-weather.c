#include <stdio.h>      // NULL, fprintf(), perror(), setlinebuf()
#include <stdlib.h>     // NULL, EXIT_FAILURE, EXIT_SUCCESS
#include <unistd.h>     // getopt(), sleep()
#include <math.h>       // fmod()
#include <time.h>       // time()

#define CANDIES_API static
#include "candies.h"

//
// DEFINES
//

#define DEFAULT_BUFFER 256
#define DEFAULT_FORMAT "%t | %w"
#define DEFAULT_INTERVAL 1

//
// STRUCTS AND ENUMS
// 

typedef unsigned char byte;

struct info
{
	char time[6];      // "23:45"
	char weather[14];  // "Mostly cloudy"
};

typedef struct info info_s;

struct options
{
	byte   help : 1;        // Print help and exit
	byte   monitor : 1;     // Keep running, print new info when avail
	int    interval;        // Query server every `interval` seconds
	int    buffer;          // Buffer size for the printed string
	char  *format;
};

typedef struct options opts_s;

enum weather_state
{
	WEATHER_CLEAR,         // 0
	WEATHER_MOSTLY_CLEAR,  // 1
	WEATHER_PARTLY_CLOUDY, // 2
	WEATHER_CLOUDY,        // 3
	WEATHER_MOSTLY_CLOUDY, // 4
	WEATHER_MIST,          // 5
	WEATHER_HAZE,          // 6
	WEATHER_FOG,           // 7
	WEATHER_DRIZZLE,       // 8
	WEATHER_RAIN           // 9
};

typedef enum weather_state weather_state_e;

struct gta_weather
{
	int             period;
	weather_state_e state;
};

typedef struct gta_weather gta_weather_s;

//
// GLOBALS
//

char *weather_string[] = {
	"Clear",          // 0
	"Mostly clear",   // 1
	"Partly cloudy",  // 2
	"Cloudy",         // 3
	"Mostly cloudy",  // 4
	"Mist",           // 5
	"Haze",           // 6
	"Fog",            // 7
	"Drizzle",        // 8
	"Rain"            // 9
};

#define NUM_WEATHER_STATES 55
gta_weather_s weather_states[] = {
	{   0, WEATHER_PARTLY_CLOUDY },
	{   4, WEATHER_MIST },
	{   7, WEATHER_MOSTLY_CLOUDY },
	{  11, WEATHER_CLEAR },
	{  14, WEATHER_MIST },
	{  16, WEATHER_CLEAR },
	{  28, WEATHER_MIST },
	{  31, WEATHER_CLEAR },
	{  41, WEATHER_HAZE },
	{  45, WEATHER_PARTLY_CLOUDY },
	{  52, WEATHER_MIST },
	{  55, WEATHER_CLOUDY },
	{  62, WEATHER_FOG },
	{  66, WEATHER_CLOUDY },
	{  72, WEATHER_PARTLY_CLOUDY },
	{  78, WEATHER_FOG },
	{  82, WEATHER_CLOUDY },
	{  92, WEATHER_MOSTLY_CLEAR },
	{ 104, WEATHER_PARTLY_CLOUDY },
	{ 105, WEATHER_DRIZZLE },
	{ 108, WEATHER_PARTLY_CLOUDY },
	{ 125, WEATHER_MIST },
	{ 128, WEATHER_PARTLY_CLOUDY },
	{ 131, WEATHER_RAIN },
	{ 134, WEATHER_DRIZZLE },
	{ 137, WEATHER_CLOUDY },
	{ 148, WEATHER_MIST },
	{ 151, WEATHER_MOSTLY_CLOUDY },
	{ 155, WEATHER_FOG },
	{ 159, WEATHER_CLEAR },
	{ 176, WEATHER_MOSTLY_CLEAR },
	{ 196, WEATHER_FOG },
	{ 201, WEATHER_PARTLY_CLOUDY },
	{ 220, WEATHER_MIST },
	{ 222, WEATHER_MOSTLY_CLEAR },
	{ 244, WEATHER_MIST },
	{ 246, WEATHER_MOSTLY_CLEAR },
	{ 247, WEATHER_RAIN },
	{ 250, WEATHER_DRIZZLE },
	{ 252, WEATHER_PARTLY_CLOUDY },
	{ 268, WEATHER_MIST },
	{ 270, WEATHER_PARTLY_CLOUDY },
	{ 272, WEATHER_CLOUDY },
	{ 277, WEATHER_PARTLY_CLOUDY },
	{ 292, WEATHER_MIST },
	{ 295, WEATHER_PARTLY_CLOUDY },
	{ 300, WEATHER_MOSTLY_CLOUDY },
	{ 306, WEATHER_PARTLY_CLOUDY },
	{ 318, WEATHER_MOSTLY_CLOUDY },
	{ 330, WEATHER_PARTLY_CLOUDY },
	{ 337, WEATHER_CLEAR },
	{ 367, WEATHER_PARTLY_CLOUDY },
	{ 369, WEATHER_RAIN },
	{ 376, WEATHER_DRIZZLE },
	{ 377, WEATHER_PARTLY_CLOUDY }
};

//
// FUNCTIONS
//

/*
 * Prints usage information.
 */
static void
help(char *invocation)
{
	fprintf(stdout, "Usage:\n");
	fprintf(stdout, "\t%s [OPTIONS...]\n", invocation);
	fprintf(stdout, "\n");
	fprintf(stdout, "Options:\n");
	fprintf(stdout, "\t-b INT\tBuffer size for the output string; default is 256\n");
	fprintf(stdout, "\t-f FORMAT\tFormat string, see below.\n");
	fprintf(stdout, "\t-h\tPrint this help text and exit.\n");
	fprintf(stdout, "\t-i SECS\tPrint every SECS seconds (only in conjunction with -m).\n");
	fprintf(stdout, "\t-m\tKeep running, printing new info when available.\n");
	fprintf(stdout, "\n");
	fprintf(stdout, "Format specifiers:\n");
	fprintf(stdout, "\t%%%%\ta literal %%\n");
	fprintf(stdout, "\t%%t\tcurrent time in GTA online\n");
	fprintf(stdout, "\t%%w\tcurrent weather in GTA online\n");
	fprintf(stdout, "\n");
}

static void
fetch_opts(opts_s *opts, int argc, char **argv)
{
	// Process command line options
	opterr = 0;
	int o;
	while ((o = getopt(argc, argv, "b:f:hi:m")) != -1)
	{
		switch(o)
		{
			case 'b':
				opts->buffer = atoi(optarg);
				break;
			case 'f':
				opts->format = optarg;
				break;
			case 'h':
				opts->help = 1;
				break;
			case 'i':
				opts->interval = atoi(optarg);
				break;
			case 'm':
				opts->monitor = 1;
				break;
		}
	}
}

static char*
candy_format_cb(char c, void* ctx)
{
	info_s* info = (info_s*) ctx;

	switch (c)
	{
		case 't':
			return info->time;
		case 'w':
			return info->weather;
		default:
			return NULL;
	}
}

static void
print_info(info_s *info, opts_s *opts)
{
	char *result = malloc(opts->buffer);
	candy_format(opts->format, result, opts->buffer, candy_format_cb, info);	
	fprintf(stdout, "%s\n", result);
	free(result);
}

double
gta_weather_period(unsigned long long s)
{
	double gta_hours_total = s / 120.0;
	return fmod(gta_hours_total, 384.0);
}

void
gta_weather_string(unsigned long long s, char *buf, size_t len)
{
	double weather_period = gta_weather_period(s);

	char *ws = NULL;
	gta_weather_s *w_prev = NULL;
	gta_weather_s *w_this = NULL;
        gta_weather_s *w_last = &weather_states[NUM_WEATHER_STATES-1];	

	for (int i = 0; i < NUM_WEATHER_STATES; ++i)
	{
		w_prev = &weather_states[i==0 ? NUM_WEATHER_STATES-1 : i-1];
		w_this = &weather_states[i]; 
		if (w_this->period > weather_period)
		{
			ws = weather_string[w_prev->state];
			break;
		}
	}
	ws = weather_string[w_last->state];
	snprintf(buf, len, "%s", ws);
}

void
gta_time_string(unsigned long long s, char *buf, size_t len)
{
	double gta_hours_total = s / 120.0;
	double gta_hours_day   = fmod(gta_hours_total, 24.0);
	int h = (int)   gta_hours_day;
	int m = (int) ((gta_hours_day - h) * 60.0);

	snprintf(buf, len, "%02d:%02d", h, m);
}

/*
 * Sources / credits:
 *   https://gtaweather.herokuapp.com/weather
 *   https://github.com/adam10603/GTAWeather/blob/master/src/gtaweather.js
 */
int
main(int argc, char **argv)
{
	// set stdout to line buffered
	setlinebuf(stdout);
	
	// parse command line arguments
	opts_s opts = { 0 };
	fetch_opts(&opts, argc, argv);
	
	// print help and exit
	if (opts.help)
	{
		help(argv[0]);
		return EXIT_SUCCESS;
	}

	// if no buffer size given, use default
	if (opts.buffer == 0)
	{
		opts.buffer = DEFAULT_BUFFER;
	}

	// if no format given, use default
	if (opts.format == NULL)
	{
		opts.format = DEFAULT_FORMAT;
	}

	// if no interval given, use default
	if (opts.interval == 0)
	{
		opts.interval = DEFAULT_INTERVAL;
	}

	// if monitoring is off, set interval (back) to 0
	if (opts.monitor == 0)
	{
		opts.interval = 0;
	}

	// do the thing (possibly more than just once)
	info_s info = { 0 };
	for (int run = 1; run; run = opts.monitor)
	{
		// get current time as seconds since UNIX epoch (UTC)
		time_t timestamp = time(NULL);

		// fetch the required info
		gta_time_string(timestamp, info.time, opts.buffer);
		gta_weather_string(timestamp, info.weather, opts.buffer);

		// print the requested info
		print_info(&info, &opts);

		// maybe sleep, maybe not
		sleep(opts.interval);
	}

	// done, bye
	return EXIT_SUCCESS;
}

