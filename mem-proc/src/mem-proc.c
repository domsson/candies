#include <stdio.h>            // fprintf
#include <stdlib.h>           // NULL, EXIT_*
#include <unistd.h>           // getopt() et al.
#include <string.h>           // strstr(), strtok()

#define DEFAULT_UNIT    "%"
#define DEFAULT_INTERVAL 1
#define DEFAULT_PROCFILE "/proc/meminfo"

#define STR_MEM_TOTAL "MemTotal"
#define STR_MEM_FREE  "MemFree"
#define STR_MEM_AVAIL "MemAvailable"

typedef unsigned long ulong;

/**
 * Given a line from `/proc/meminfo` (or a file of the same format), extracts 
 * the memory value and stores it in `val`. Returns 0 on success, -1 on error.
 */
int extract_value(char *buf, ulong *val)
{
	// Example line from `/proc/meminfo`:
	// "MemTotal:        8199704 kB"
	// Hence, we split on whitespace and extract the second token
	
	char *token;
	int i;
	for (i = 0; (token = strtok(i == 0 ? buf : NULL, " ")) != NULL; ++i)
	{
		// Index 1 (second token) should be the memory value
		if (i == 1)
		{
			// TODO add error handling
			*val = strtoul(token, NULL, 0);
			return 0;
		}
	}

	return -1;
}

/**
 * Reads the given file (expected to be `/proc/meminfo` or a file of the same 
 * format) line by line, lookign for lines beginning with `str_total` and 
 * `str_avail` respectively. If found, it tries to extract the memory value 
 * from those lines and returns them in `total` and `avail`.
 * Returns 0 if both values were extracted successfully, otherwise -1.
 */
int read_mem_stats(const char *file, ulong *total, ulong *avail, 
		const char *str_total, const char *str_avail)
{
	FILE *fp = fopen(file, "r");
	if (fp == NULL)
	{
		return -1;
	}

	// Keep track of what we've found (or not)
	int found_total = 0;
	int found_avail = 0;

	// Read the file line by line
	char *buf = NULL;
	size_t len = 0;
	while (getline(&buf, &len, fp) != -1)
	{
		// Check if the line starts with `str_total`
		if (strncmp(str_total, buf, strlen(str_total)) == 0)
		{
			if (extract_value(buf, total) == 0)
			{
				found_total = 1;
			}
		}
		// Check if the line starts with `str_avail`
		else if (strncmp(str_avail, buf, strlen(str_avail)) == 0)
		{
			if (extract_value(buf, avail) == 0)
			{
				found_avail = 1;
			}
		}
		// We've found everything we were looking for, end the loop 
		if (found_total && found_avail)
		{
			break;
		}
	}

	free(buf);
	fclose(fp);

	return (found_total && found_avail) ? 0 : -1;
}

/**
 * Prints the given `usage` value to `stdout`, including `precision` decimal 
 * places in the output. If `unit` is not empty, it will be appended and, if 
 * `space` is `1`, a single space will be inserted before the `unit` string.
 */
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
	int space = 0;          // space between value and unit?
	int precision = 0;      // number of decimal places in output
	int gross = 0;          // use 'free' instead of 'available' memory
	char *file = NULL;      // file to read memory stats from

	// Get arguments, if any
	opterr = 0;
	int o;
	while ((o = getopt(argc, argv, "mf:i:p:gush")) != -1)
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
			case 'g':
				gross = 1;
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

	// If no file given, use the default
	if (file == NULL)
	{
		file = DEFAULT_PROCFILE;
	}

	// If no interval given, use the default
	if (interval == 0)
	{
		interval = DEFAULT_INTERVAL;
	}

	// We don't sleep() if we don't monitor
	if (monitor == 0)
	{
		interval = 0;
	}

	const char *str_total = STR_MEM_TOTAL;
	const char *str_avail = gross ? STR_MEM_FREE : STR_MEM_AVAIL;

	// Disable stdout buffering
	setbuf(stdout, NULL);

	ulong total = 0;
	ulong avail = 0;
	double usage = 0;

	do
	{
		// TODO: add error handling (this might return -1)
		read_mem_stats(file, &total, &avail, str_total, str_avail);
		usage = (1 - ((double) avail / (double) total)) * 100;

		print_usage(usage, precision, unit ? DEFAULT_UNIT : "", space);
		sleep(interval);
	}
	while (monitor);

	return EXIT_SUCCESS;
}

