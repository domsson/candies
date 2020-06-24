#include <stdio.h>            // fprintf
#include <stdlib.h>           // NULL, EXIT_*
#include <unistd.h>           // getopt(), sleep()
#include <time.h>

#define DEFAULT_FORMAT   "%F %T"
#define DEFAULT_INTERVAL 1
#define RESULT_LENGTH    64


/**
 * Prints usage information.
 */
void help(char *invocation)
{
	fprintf(stderr, "Usage:\n");
     	fprintf(stderr, "\t%s [OPTION...] [-f FORMAT] [-i INTERVAL] [-o OFFSET]\n", invocation);
	fprintf(stderr, "\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "\t-h Print this help text and exit.\n");
	fprintf(stderr, "\t-g Use UTC/GMT instead of local time.\n");
	fprintf(stderr, "\t-m Keep running and print every second (or every INTERVAL seconds).\n");
}

void print_datetime(const char *format, int len, int utc, int offset)
{
	time_t now    = time(NULL) + (offset * 60 * 60);
	struct tm *tm = utc ? gmtime(&now) : localtime(&now);
	
	char *result = malloc(sizeof(char) * len);
	strftime(result, len, format, tm);
	fprintf(stdout, "%s\n", result);
	free(result);
}

/**
 * Sources:
 */
int main(int argc, char **argv)
{
	char *format = NULL;	// format string for use with strftime()
	int   offset = 0;	// timezone offset, in hours, from UTC
	int  use_utc = 0;	// use UTC instead of local time?
	int  monitor = 0;       // keep running and printing?
	int interval = 0;	// seconds between printing the datetime

	// Get arguments, if any
	opterr = 0;
	int o;
	while ((o = getopt(argc, argv, "gmf:i:o:h")) != -1)
	{
		switch (o)
		{
			case 'g':
				use_utc = 1;
				break;
			case 'm':
				monitor = 1;
				break;
			case 'f':
				format = optarg;
				break;
			case 'i':
				interval = atoi(optarg);
				break;
			case 'o':
				offset = atoi(optarg);
				break;
			case 'h':
				help(argv[0]);
				return EXIT_SUCCESS;
		}
	}

	// Set defaults for `format` and `interval` if they weren't given
	if (format == NULL)
	{
		format = DEFAULT_FORMAT;
	}

	if (interval == 0)
	{
		interval = DEFAULT_INTERVAL;
	}

	// Make sure stdout is line buffered
	setlinebuf(stdout);
	
	// Run endless loop, if `monitor` option given
	if (monitor)
	{
		while(monitor)
		{
			print_datetime(format, RESULT_LENGTH, use_utc, offset);
			sleep(interval);
		}
	}
	// Otherwise just print once
	else
	{
		print_datetime(format, RESULT_LENGTH, use_utc, offset);
	}
	
	return EXIT_SUCCESS;
}

