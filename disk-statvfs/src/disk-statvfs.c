#include <stdio.h>            // fprintf
#include <stdlib.h>           // NULL, EXIT_*
#include <unistd.h>           // getopt() et al.
#include <ctype.h>            // tolower()
#include <string.h>           // strstr()
#include <math.h>             // pow(), fabs()
#include <sys/statvfs.h>      // statvfs()

#define CANDIES_API static
#include "candies.h"

#define DEFAULT_PATH        "/"
#define DEFAULT_UNIT        "%"
#define DEFAULT_INTERVAL     10
#define DEFAULT_THRESHOLD    1.0
#define DEFAULT_GRANULARITY "g"
#define DEFAULT_FORMAT      "%u"

#define OUTPUT_SIZE 128
#define RESULT_SIZE 16

typedef unsigned long ulong;
typedef unsigned char byte;

struct info
{
	ulong total_abs;    // disk space total
	ulong free_abs;     // disk space free
	ulong avail_abs;    // disk space available to unpriviledged user
	ulong used_abs;     // disk space used
	double total_rel;
	double free_rel;
	double avail_rel;
	double used_rel;
};

typedef struct info info_s;

struct opts
{
	byte help : 1;      // show help and exit
	byte monitor : 1;   // keep running and printing
	byte unit : 1;      // print a unit character
	byte space : 1;     // print a space between value and unit
	byte available : 1; // print available, not free disk space 
	byte binary : 1;    // binary instead of decimal units (MiB vs MB etc)
	int interval;       // interval, in seconds, to check disk space
	int precision;      // number of decimals in output
	double threshold;   // minimum change in value to print again
	char *path;         // path of a file on the desired disk/mount
	char *format;
	char granularity;   // unit granularity (m = mega, g = giga, etc) 

	ulong unit_size;    // will be set by program
	char *unit_abbr;    // will be set by program
};

typedef struct opts opts_s;

struct context
{
	info_s* info;
	opts_s* opts;
	char buffer[RESULT_SIZE];
};

typedef struct context ctx_s;

static void
fetch_opts(opts_s *opts, int argc, char **argv)
{
	opterr = 0;
	int o;
	while ((o = getopt(argc, argv, "abd:f:g:hi:mp:st:u")) != -1)
	{
		switch (o)
		{
			case 'a':
				opts->available = 1;
				break;
			case 'b':
				opts->binary = 1;
				break;
			case 'd':
				opts->path = optarg;
				break;
			case 'f':
				opts->format = optarg;
				break;
			case 'g':
				opts->granularity = tolower(optarg[0]);
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
			case 'p':
				opts->precision = atoi(optarg);
				break;
			case 's':
				opts->space = 1;
				break;
			case 't':
				opts->threshold = atof(optarg);
				break;
			case 'u':
				opts->unit = 1;
				break;
		}
	}
}

static void
help(char *invocation, FILE* stream)
{
	fprintf(stream, "Usage:\n");
     	fprintf(stream, "\t%s [OPTION...]\n", invocation);
	fprintf(stream, "\n");
	fprintf(stream, "Options:\n");
	fprintf(stream, "\t-a Calculate space available to unprivilidged users, not free space.\n");
	fprintf(stream, "\t-d Path of any file/dir on the filesystem in question.\n");
	fprintf(stream, "\t-h Print this help text and exit.\n");
	fprintf(stream, "\t-i Seconds between checking for a change in value; default is 1.\n");
	fprintf(stream, "\t-m Keep running and print when there is a notable change in value.\n"); 
	fprintf(stream, "\t-p Number of decimal digits in the output; default is 0.\n");
	fprintf(stream, "\t-s Print a space between value and unit.\n");
	fprintf(stream, "\t-t Requried change in value in order to print again; default is 1.\n");
	fprintf(stream, "\t-u Print the appropriate unit after the value.\n");
}

static int
fetch_info(info_s* info, opts_s* opts)
{
	struct statvfs stat = { 0 };
	if (statvfs(opts->path, &stat) == -1)
	{
		return -1;
	}

	// usually f_frsize == f_bsize 
	info->total_abs = stat.f_bsize * stat.f_blocks;
	info->avail_abs = stat.f_bsize * stat.f_bavail;
	info->free_abs  = stat.f_bsize * stat.f_bfree;
	info->used_abs  = info->total_abs - info->free_abs;

	info->total_rel = 100.0;
	info->avail_rel = ((double) info->avail_abs / (double) info->total_abs) * 100.0;
	info->free_rel  = ((double) info->free_abs  / (double) info->total_abs) * 100.0;
	info->used_rel  = ((double) info->used_abs  / (double) info->total_abs) * 100.0;

	return 0;
}

static void
print_info(ctx_s* ctx)
{
	char output[OUTPUT_SIZE] = { 0 };
	candy_format(ctx->opts->format, output, OUTPUT_SIZE, candy_format_cb, ctx);
	fprintf(stdout, "%s\n", output);
}

static void
format_rel_value(char *buf, size_t len, double val, opts_s* opts)
{
	snprintf(buf, len, "%.*lf%s%s",
			opts->precision,
			val,
			opts->space && opts->unit ? " " : "",
			opts->unit ? "%" : ""
	);
}

static void
format_abs_value(char *buf, size_t len, double val, opts_s* opts)
{
	snprintf(buf, len, "%.*lf%s%s",
			opts->precision,
			val / opts->unit_size,
			opts->space && opts->unit ? " " : "",
			opts->unit ? opts->unit_abbr : ""
	);
}

static char*
candy_format_cb(char c, void* context)
{
	ctx_s* ctx = (ctx_s*) context;

	switch(c)
	{
		case 't':	// disk space total, percent
			format_rel_value(ctx->buffer, RESULT_SIZE,
					ctx->info->total_rel, ctx->opts);
			return ctx->buffer;
		case 'f':       // disk space free, percent
			format_rel_value(ctx->buffer, RESULT_SIZE,
					ctx->info->free_rel, ctx->opts);
			return ctx->buffer;
		case 'u':       // disk space used, percent
			format_rel_value(ctx->buffer, RESULT_SIZE,
					ctx->info->used_rel, ctx->opts);
			return ctx->buffer;
		case 'a':	// disk space avail. to unpriv. users, percent
			format_rel_value(ctx->buffer, RESULT_SIZE,
					ctx->info->avail_rel, ctx->opts);
			return ctx->buffer;
		case 'T':       // disk space total, absolute 
			format_abs_value(ctx->buffer, RESULT_SIZE,
					ctx->info->total_abs, ctx->opts);
			return ctx->buffer;
		case 'F':       // disk space free, absolute
			format_abs_value(ctx->buffer, RESULT_SIZE,
					ctx->info->free_abs, ctx->opts);
			return ctx->buffer;
		case 'U':       // disk space used, absolute
			format_abs_value(ctx->buffer, RESULT_SIZE,
					ctx->info->used_abs, ctx->opts);
			return ctx->buffer;
		case 'A':	// disk space avail. to unpriv. users, absolute
			format_abs_value(ctx->buffer, RESULT_SIZE,
					ctx->info->avail_abs, ctx->opts);
			return ctx->buffer;
		default:
			return NULL;
	}
}

int
main(int argc, char **argv)
{
	opts_s opts = { 0 };
	fetch_opts(&opts, argc, argv);

	// show help and exit
	if (opts.help)
	{
		help(argv[0], stdout);
		return EXIT_SUCCESS;
	}

	if (opts.path == NULL)
	{
		opts.path = DEFAULT_PATH;
	}

	if (opts.format == NULL)
	{
		opts.format = DEFAULT_FORMAT;
	}

	if (opts.granularity == 0)
	{
		opts.granularity = *DEFAULT_GRANULARITY;
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

	// important data structures for processing
	info_s info = { 0 };
	ctx_s ctx = { .info = &info, .opts = &opts };

	// set additional members of the opts struct based on the granularity
	candy_unit_info(opts.granularity, opts.binary, &opts.unit_size, &opts.unit_abbr);

	// loop variables
	double usage_prev  = -1.0; // last usage value we printed (!)
	double usage_curr  =  0.0; // current usage value
	double usage_delta =  0.0; // difference to last printed value

	do
	{
		if (fetch_info(&info, &opts) == -1)
		{
			return EXIT_FAILURE;
		}
		
		// calculate current disk usage, as well as the difference
		usage_curr  = info.free_rel;
		usage_delta = fabs(usage_curr - usage_prev);

		if (usage_delta >= opts.threshold)
		{
			// print
			print_info(&ctx);

			// update values for next iteration
			usage_prev = usage_curr;
		}
		
		// sleep, maybe (if interval > 0)
		sleep(opts.interval);
	}
	while (opts.monitor);

	return EXIT_SUCCESS;
}

