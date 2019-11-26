#include <stdio.h>            // fprintf
#include <stdlib.h>           // NULL, EXIT_*
#include <unistd.h>           // getopt() et al.
#include <string.h>           // strstr()
#include <math.h>             // pow(), fabs()
#include <sys/sysinfo.h>      // sysinfo()

#define DEFAULT_UNIT      "%"
#define DEFAULT_INTERVAL   1
#define DEFAULT_THRESHOLD  1.0

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
     	fprintf(stderr, "\t%s [OPTION...]\n", invocation);
	fprintf(stderr, "\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "\t-h Print this help text and exit.\n");
	fprintf(stderr, "\t-i Seconds between checking for a change in value; default is 1.\n");
	fprintf(stderr, "\t-m Keep running and print when there is a notable change in value.\n"); 
	fprintf(stderr, "\t-p Number of decimal digits in the output; default is 0.\n");
	fprintf(stderr, "\t-s Print a space between value and unit.\n");
	fprintf(stderr, "\t-t Requried change in value in order to print again; default is 1.\n");
	fprintf(stderr, "\t-u Print the appropriate unit after the value.\n");
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
	double threshold = -1;  // minimum change in value to issue a print

	// Get arguments, if any
	opterr = 0;
	int o;
	while ((o = getopt(argc, argv, "hi:mp:st:u")) != -1)
	{
		switch (o)
		{
			case 'h':
				help(argv[0]);
				return EXIT_SUCCESS;
			case 'i':
				interval = atoi(optarg);
				break;
			case 'm':
				monitor = 1;
				break;
			case 'p':
				precision = atoi(optarg);
				break;
			case 's':
				space = 1;
				break;
			case 't':
				threshold = atof(optarg);
				break;
			case 'u':
				unit = 1;
				break;
		}
	}

	// If no threshold given, determine it based on precision
	if (threshold == -1)
	{
		threshold = DEFAULT_THRESHOLD / pow(10.0, (double) precision);
	}

	// If no interval given, use the default
	if (interval == 0)
	{
		// Set interval to 0 if we don't monitor (run only once)
		interval = monitor ? DEFAULT_INTERVAL : 0;
	}

	// Disable stdout buffering
	setbuf(stdout, NULL);

	// Loop variables
	unsigned long total = 0;
	unsigned long free  = 0;
	double usage_prev  = -1.0; // last usage value we printed (!)
	double usage_curr  =  0.0; // current usage value
	double usage_delta =  0.0; // difference to last printed value

	do
	{
		if (get_mem_info(&total, &free) == -1)
		{
			return EXIT_FAILURE;
		}
		
		// Calculate the current usage, as well as the difference
		usage_curr  = (1 - ((double) free / (double) total)) * 100;
		usage_delta = fabs(usage_curr - usage_prev);

		if (usage_delta >= threshold)
		{
			// Print
			print_usage(usage_curr, precision, unit ? DEFAULT_UNIT : "", space);

			// Update values for next iteration
			usage_prev = usage_curr;
		}
		
		// Sleep, maybe (if interval > 0)
		sleep(interval);
	}
	while (monitor);

	return EXIT_SUCCESS;
}

