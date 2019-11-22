#include <stdio.h>            // fprintf
#include <stdlib.h>           // NULL, EXIT_*
#include <unistd.h>           // getopt() et al.
#include <string.h>           // strstr()
#include <time.h>             // nanosleep()

#define PROC_FILE "/proc/stat"
#define PROC_BUF  128

#define DEFAULT_INTERVAL 1

int read_cpu_stats(const char *file, char *buf, size_t len, long *total, long *idle)
{
	FILE *fp = fopen(file, "r");
	if (fp == NULL)
	{
		return 0;
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
	return 1;
}

double calc_usage(long total, long idle)
{
	return (1 - ((double) idle / (double) total)) * 100;
}

double determine_usage(int interval, size_t buf_len)
{
	char cpu_stats[buf_len];

	long total_1 = 0;
	long idle_1  = 0;

	long total_2 = 0;
	long idle_2  = 0;

	read_cpu_stats(PROC_FILE, cpu_stats, buf_len, &total_1, &idle_1);

	sleep(interval);
	
	read_cpu_stats(PROC_FILE, cpu_stats, buf_len, &total_2, &idle_2);

	return calc_usage(total_2 - total_1, idle_2 - idle_1); 
}

void print_usage(double usage, int precision, int unit, int space)
{
	fprintf(stdout, "%.*lf%s%s\n", precision, usage,
			space && unit ? " " : "",
			unit ? "%" : "");
}

/**
 * Prints usage information.
 */
void help(char *invocation)
{
	fprintf(stderr, "Usage:\n");
     	fprintf(stderr, "\t%s [OPTION...] -c CHIP -f FEATURE\n", invocation);
	fprintf(stderr, "\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "\t-h Print this help text and exit.\n");
}

int main(int argc, char **argv)
{
	int monitor = 0;        // keep running and printing
	int interval = 0;       // print every `interval` seconds
	int unit = 0;		// also print the % unit
	int space = 0;          // space between val and unit
	int precision = 0;      // decimal places in output

	// Get arguments, if any
	opterr = 0;
	int o;
	while ((o = getopt(argc, argv, "mi:p:ush")) != -1)
	{
		switch (o)
		{
			case 'm':
				monitor = 1;
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

	if (interval == 0)
	{
		interval = DEFAULT_INTERVAL;
	}

	if (monitor)
	{
		while(monitor)
		{
			// TODO: instead of fetching two measurements every time
			//       we run determine_usage(), we should be able to 
			//       pass in the previous results, so the function 
			//       has to only open and read from /proc/stat once!
			double usage = determine_usage(interval, PROC_BUF);
			print_usage(usage, precision, unit, space);
		}
	}
	else
	{
		double usage = determine_usage(interval, PROC_BUF);
		print_usage(usage, precision, unit, space);
	}

	return EXIT_SUCCESS;
}

