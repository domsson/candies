#include <stdio.h>            // fprintf
#include <stdlib.h>           // NULL, EXIT_*
#include <unistd.h>           // getopt() et al.
#include <string.h>           // strstr()
#include <math.h>             // pow(), fabs()
#include <sys/statvfs.h>      // statvfs()

#define DEFAULT_PATH      "/"
#define DEFAULT_UNIT      "%"
#define DEFAULT_INTERVAL   10
#define DEFAULT_THRESHOLD  1.0

typedef unsigned char byte;

struct opts
{
	byte help : 1;         // show help and exit
	byte monitor : 1;      // keep running and printing
	byte unit : 1;         // print a unit character
	byte space : 1;        // print a space between value and unit
	byte available : 1;    // print available, not free disk space 
	int interval;          // interval, in seconds, to check disk space
	float precision;       // number of decimals in output
	float threshold;       // minimum change in value to print again
	char *path;            // path of a file on the desired disk/mount
};

typedef struct opts opts_s;

int get_info(const char *path, unsigned long *total, unsigned long *free, unsigned long *avail)
{
	struct statvfs stat = { 0 };
	if (statvfs(path, &stat) == -1)
	{
		return -1;
	}

	*total = stat.f_blocks * stat.f_frsize;
	*avail = stat.f_bsize * stat.f_bavail;
	*free = stat.f_bsize * stat.f_bfree;

	return 0;
}

void print_info(double usage, int precision, const char *unit, int space)
{
	fprintf(stdout, "%.*lf%s%s\n", precision, usage,
			space && strlen(unit) ? " " : "", unit);
}

void help(char *invocation)
{
	fprintf(stdout, "Usage:\n");
     	fprintf(stdout, "\t%s [OPTION...]\n", invocation);
	fprintf(stdout, "\n");
	fprintf(stdout, "Options:\n");
	fprintf(stdout, "\t-a Calculate space available to unprivilidged users, not free space.\n");
	fprintf(stdout, "\t-d Path of any file/dir on the filesystem in question.\n");
	fprintf(stdout, "\t-h Print this help text and exit.\n");
	fprintf(stdout, "\t-i Seconds between checking for a change in value; default is 1.\n");
	fprintf(stdout, "\t-m Keep running and print when there is a notable change in value.\n"); 
	fprintf(stdout, "\t-p Number of decimal digits in the output; default is 0.\n");
	fprintf(stdout, "\t-s Print a space between value and unit.\n");
	fprintf(stdout, "\t-t Requried change in value in order to print again; default is 1.\n");
	fprintf(stdout, "\t-u Print the appropriate unit after the value.\n");
}

int main(int argc, char **argv)
{
	opts_s opts  = { 0 };
	
	opterr = 0;
	int o;
	while ((o = getopt(argc, argv, "ad:hi:mp:st:u")) != -1)
	{
		switch (o)
		{
			case 'a':
				opts.available = 1;
				break;
			case 'd':
				opts.path = optarg;
				break;
			case 'h':
				opts.help = 1;
				break;
			case 'i':
				opts.interval = atoi(optarg);
				break;
			case 'm':
				opts.monitor = 1;
				break;
			case 'p':
				opts.precision = atoi(optarg);
				break;
			case 's':
				opts.space = 1;
				break;
			case 't':
				opts.threshold = atof(optarg);
				break;
			case 'u':
				opts.unit = 1;
				break;
		}
	}

	// show help and exit
	if (opts.help)
	{
		help(argv[0]);
		return EXIT_SUCCESS;
	}

	// if no threshold given, determine it based on precision
	if (opts.threshold == -1)
	{
		opts.threshold = DEFAULT_THRESHOLD / pow(10.0, (double) opts.precision);
	}

	// if no interval given, use the default
	if (opts.interval == 0)
	{
		opts.interval = DEFAULT_INTERVAL;
	}

	// set interval to 0 if we don't monitor (run only once)
	if (opts.monitor == 0)
	{
		opts.interval = 0;
	}
	
	// ensure stdout is line buffered
	setlinebuf(stdout);

	// loop variables
	unsigned long total = 0;
	unsigned long free  = 0;
	unsigned long avail = 0;
	double usage_prev  = -1.0; // last usage value we printed (!)
	double usage_curr  =  0.0; // current usage value
	double usage_delta =  0.0; // difference to last printed value

	do
	{
		if (get_info(opts.path ? opts.path : DEFAULT_PATH, &total, &free, &avail) == -1)
		{
			return EXIT_FAILURE;
		}
		
		// calculate current disk usage, as well as the difference
		usage_curr  = (1 - ((double) (opts.available ? avail : free) / (double) total)) * 100;
		usage_delta = fabs(usage_curr - usage_prev);

		if (usage_delta >= opts.threshold)
		{
			// print
			print_info(usage_curr, opts.precision, opts.unit ? DEFAULT_UNIT : "", opts.space);

			// update values for next iteration
			usage_prev = usage_curr;
		}
		
		// sleep, maybe (if interval > 0)
		sleep(opts.interval);
	}
	while (opts.monitor);

	return EXIT_SUCCESS;
}

