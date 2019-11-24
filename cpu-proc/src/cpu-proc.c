#include <stdio.h>            // fprintf
#include <stdlib.h>           // NULL, EXIT_*
#include <unistd.h>           // getopt() et al.
#include <string.h>           // strstr()

#define PROC_BUF 128

#define DEFAULT_UNIT    "%"
#define DEFAULT_INTERVAL 1
#define DEFAULT_PROCFILE "/proc/stat"

/*
 * Opens and reads the first line from the given file, which is assumed to have 
 * the format of /proc/stat and returns the total CPU time, plus the idle time
 * in `total` and `idle`. These times are total times accumulated since system 
 * boot; you would want to take at least one more measurement, then calculate 
 * the difference between them to get meaningful information regarding current 
 * CPU usage. Returns 0 on success, -1 on error (couldn't open file for read).
 */
int read_cpu_stats(const char *file, char *buf, size_t len, long *total, long *idle)
{
	FILE *fp = fopen(file, "r");
	if (fp == NULL)
	{
		return -1;
	}

	fgets(buf, len, fp);

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
			*idle = atoll(token);
		}

		*total += atoll(token);
	}

	fclose(fp);
	return 0;
}

/*
 * Turns delta times for total and idle CPU time into a percentage value that
 * represents the CPU usage for the timespan indirectly given by the delta that 
 * was used to calculate the given values.
 */
double calc_usage(long delta_total, long delta_idle)
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
double determine_usage(const char *file, size_t buf_len, int interval, long *total, long *idle)
{
	char cpu_stats[buf_len];

	long new_total = 0;
	long new_idle  = 0;

	if (*total == 0 && *idle == 0)
	{
		// TODO: add error handling (this might return -1)
		read_cpu_stats(file, cpu_stats, buf_len, total, idle);
	}

	sleep(interval);
	
	// TODO: add error handling (this might return -1)
	read_cpu_stats(file, cpu_stats, buf_len, &new_total, &new_idle);
	
	long delta_total = new_total - *total;
	long delta_idle  = new_idle  - *idle;

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
     	fprintf(stderr, "\t%s [OPTION...] [-f PROCFILE] [-i INTERVAL] [-p PRECISION]\n", invocation);
	fprintf(stderr, "\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "\t-h Print this help text and exit.\n");
	fprintf(stderr, "\t-m Keep running and printing every second (or INTERVAL seconds).\n"); 
	fprintf(stderr, "\t-u Print a percentage sign after the CPu usage value.\n");
	fprintf(stderr, "\t-s Print a space between CPU usage and percentage sign.\n");
}

int main(int argc, char **argv)
{
	int monitor = 0;        // keep running and printing
	int interval = 0;       // print every `interval` seconds
	int unit = 0;		// also print the % unit
	int space = 0;          // space between val and unit
	int precision = 0;      // decimal places in output
	char *file = NULL;      // file to read CPU stats from

	// Get arguments, if any
	opterr = 0;
	int o;
	while ((o = getopt(argc, argv, "mf:i:p:ush")) != -1)
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
			case 'h':
				help(argv[0]);
				return EXIT_SUCCESS;
		}
	}

	if (file == NULL)
	{
		file = DEFAULT_PROCFILE;
	}

	if (interval == 0)
	{
		interval = DEFAULT_INTERVAL;
	}
	
	// Disable stdout buffering
	setbuf(stdout, NULL);

	long total   = 0;
	long idle    = 0;
	double usage = 0;

	do
	{
		usage = determine_usage(file, PROC_BUF, interval, &total, &idle);
		print_usage(usage, precision, unit ? DEFAULT_UNIT : "", space);
	}
	while (monitor);

	return EXIT_SUCCESS;
}

