#include <stdio.h>            // fprintf(), getline(), fopen(), ...
#include <stdlib.h>           // NULL, EXIT_* 
#include <unistd.h>           // getopt() et al.
#include <string.h>           // strtok()
#include <math.h>             // pow(), fabs()

#define DEFAULT_UNIT      "%"
#define DEFAULT_INTERVAL   1
#define DEFAULT_THRESHOLD  1
#define DEFAULT_PROCFILE  "/proc/stat"

typedef unsigned long ulong;

/*
 * Opens and reads the first line from the given file, which is assumed to have 
 * the format of /proc/stat and returns the total CPU time, plus the idle time
 * in `total` and `idle`. These times are total times accumulated since system 
 * boot; you would want to take at least one more measurement, then calculate 
 * the difference between them to get meaningful information regarding current 
 * CPU usage. Returns 0 on success, -1 on error (couldn't open file for read).
 */
int read_cpu_stats(const char *file, ulong *total, ulong *idle)
{
	FILE *fp = fopen(file, "r");
	if (fp == NULL)
	{
		return -1;
	}

	char *buf = NULL;
	size_t len = 0;
	if (getline(&buf, &len, fp) == -1)
	{
		return -1;
	}

	char *token;
	int i;
	for (i = 0; (token = strtok(i == 0 ? buf : NULL, " ")) != NULL; ++i)
	{
		if (i == 0)
		{
			continue;
		}

		if (i == 4)
		{
			// TODO add error handling
			*idle = strtoul(token, NULL, 0);
		}

		// TODO add error handling
		*total += strtoul(token, NULL, 0);
	}

	free(buf);
	fclose(fp);
	return 0;
}

/*
 * Turns delta times for total and idle CPU time into a percentage value that
 * represents the CPU usage for the timespan indirectly given by the delta that 
 * was used to calculate the given values.
 */
double calc_usage(ulong delta_total, ulong delta_idle)
{
	return (1 - ((double) delta_idle / (double) delta_total)) * 100;
}

/*
 * Calculates an approximation of the current CPU usage by reading CPU time 
 * statistics from the provided proc stat file up to two times. If `total` 
 * and `idle` CPU times from a previous read are provided, the file will only 
 * be read once. If both values are 0, the file will be read twice. Between the 
 * two reads (or before the single read), a wait() of `interval` seconds will 
 * be performed. The length of the interval influences the significance of the 
 * returned CPU usage: shorter times make for a more 'current' usage, but will 
 * reduce the validity of the value and vice versa. 
 */
double determine_usage(const char *file, int interval, ulong *total, ulong *idle)
{
	if (*total == 0 && *idle == 0)
	{
		// TODO: add error handling (this might return -1)
		read_cpu_stats(file, total, idle);
	}

	sleep(interval);
	
	ulong new_total = 0;
	ulong new_idle  = 0;
	
	// TODO: add error handling (this might return -1)
	read_cpu_stats(file, &new_total, &new_idle);
	
	ulong delta_total = new_total - *total;
	ulong delta_idle  = new_idle  - *idle;

	*total = new_total;
	*idle  = new_idle;

	return calc_usage(delta_total, delta_idle); 
}

void print_usage(double usage, int precision, const char *unit, int space)
{
	fprintf(stdout, "%.*lf%s%s\n", precision, usage,
			space && strlen(unit) ? " " : "", unit);
}

/**
 * Prints usage information.
 */
void help(char *invocation)
{
	fprintf(stderr, "Usage:\n");
     	fprintf(stderr, "\t%s [OPTIONS...]\n", invocation);
	fprintf(stderr, "\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "\t-f File to query for CPU info; default is '/proc/stat'.\n");
	fprintf(stderr, "\t-h Print this help text and exit.\n");
	fprintf(stderr, "\t-i Seconds between checking for a change in value; default is 1.\n");
	fprintf(stderr, "\t-m Keep running and print when there is a notable change in value.\n"); 
	fprintf(stderr, "\t-p Number of decimal digits in the output; default is 0.\n");
	fprintf(stderr, "\t-s Print a space between value and unit..\n");
	fprintf(stderr, "\t-t Required change in value in order to print again; default is 1.\n");
	fprintf(stderr, "\t-u Print the appropriate unit after the value.\n");
}

int main(int argc, char **argv)
{
	int monitor = 0;        // keep running and printing
	int interval = 0;       // print every `interval` seconds
	int unit = 0;		// also print the % unit
	int space = 0;          // space between val and unit
	int precision = 0;      // decimal places in output
	double threshold = -1;  // minimum change in value required to print
	char *file = NULL;      // file to read CPU stats from

	// Get arguments, if any
	opterr = 0;
	int o;
	while ((o = getopt(argc, argv, "f:hi:mp:st:u")) != -1)
	{
		switch (o)
		{
			case 'm':
				monitor = 1;
				break;
			case 'f':
				file = optarg;
				break;
			case 'i':
				interval = atoi(optarg);
				break;
			case 'p':
				precision = atoi(optarg);
				break;
			case 'u':
				unit = 1;
				break;
			case 's':
				space = 1;
				break;
			case 't':
				threshold = atof(optarg);
				break;
			case 'h':
				help(argv[0]);
				return EXIT_SUCCESS;
		}
	}

	if (file == NULL)
	{
		file = DEFAULT_PROCFILE;
	}

	if (threshold == -1)
	{
		threshold = DEFAULT_THRESHOLD / pow(10.0, (double) precision);
	}

	if (interval == 0)
	{
		// We need some interval, as we need to take two measurements
		interval = DEFAULT_INTERVAL;
	}

	// make sure stdout is line buffered 
	setlinebuf(stdout);

	// Prepare string we'll need multiple times
	const char *str_unit = unit ? DEFAULT_UNIT : "";

	// Loop variables
	ulong total   = 0;
	ulong idle    = 0;
	double usage_prev  = -1.0; // makes sure that we print the first time
	double usage_curr  =  0.0;
	double usage_delta =  0.0;

	do
	{
		// Calculate usage (this will do the sleep internally)
		usage_curr  = determine_usage(file, interval, &total, &idle);
		usage_delta = fabs(usage_curr - usage_prev);

		// Check if the value changed enough for us to print
		if (usage_prev < 0 || usage_delta >= threshold)
		{
			// Print
			print_usage(usage_curr, precision, str_unit, space);

			// Update values
			usage_prev = usage_curr;
		}
		
		// Sleep, maybe (if interval > 0)
		sleep(interval);
	}
	while (monitor);

	return EXIT_SUCCESS;
}

