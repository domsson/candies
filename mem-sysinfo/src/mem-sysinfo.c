#include <stdio.h>            // fprintf
#include <stdlib.h>           // NULL, EXIT_*
#include <unistd.h>           // getopt() et al.
#include <string.h>           // strstr()
#include <sys/sysinfo.h>      // sysinfo()

#define DEFAULT_UNIT    "%"
#define DEFAULT_INTERVAL 1

void print_usage(double usage, int precision, const char *unit, int space)
{
	fprintf(stdout, "%.*lf%s%s\n", precision, usage,
			space && strlen(unit) ? " " : "", unit);
}

int get_mem_info(unsigned long *total, unsigned long *free)
{
	struct sysinfo info = { 0 };
	if (sysinfo(&info) == -1)
	{
		return -1;
	}

	*total = info.totalram;
	*free  = info.freeram;
	return 0;
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
	// TODO: implement threshold option, so that we only print
	//       again once the memory usage has changed by the given 
	//       percentage, instead of using a set interval
	//
	// TODO: implement the option to show absolute values instead 
	//       of a percentage
	//
	// TODO: maybe implement the option to show FREE memory instead
	//       of USED memory; not sure about this one
	
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

	// If no interval given, use the default
	if (interval == 0)
	{
		interval = DEFAULT_INTERVAL;
	}

	// If we only run once, we set the interval to 0 
	if (monitor == 0)
	{
		interval = 0;
	}

	// Disable stdout buffering
	setbuf(stdout, NULL);

	unsigned long total = 0;
	unsigned long free  = 0;
	double usage = 0;

	do
	{
		get_mem_info(&total, &free);
		usage = (1 - ((double) free / (double) total)) * 100;
		print_usage(usage, precision, unit ? DEFAULT_UNIT : "", space);
		sleep(interval);
	}
	while (monitor);

	return EXIT_SUCCESS;
}

