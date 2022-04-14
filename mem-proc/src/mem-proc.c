#include <stdio.h>            // fprintf
#include <stdlib.h>           // NULL, EXIT_*
#include <unistd.h>           // getopt() et al.
#include <string.h>           // strstr(), strtok()
#include <ctype.h>            // tolower()

#define CANDIES_API static
#include "candies.h"

#define PROGRAM_NAME "mem-proc"
#define PROGRAM_URL  "https://github.com/domsson/candies/mem-proc"

#define PROGRAM_VER_MAJOR 1
#define PROGRAM_VER_MINOR 0
#define PROGRAM_VER_PATCH 0

#define DEFAULT_INTERVAL     1
#define DEFAULT_PROCFILE    "/proc/meminfo"
#define DEFAULT_GRANULARITY "g"
#define DEFAULT_FORMAT      "%b"

#define STR_MEM_TOTAL "MemTotal"
#define STR_MEM_FREE  "MemFree"
#define STR_MEM_AVAIL "MemAvailable"

#define OUTPUT_SIZE 128
#define RESULT_SIZE 16

typedef unsigned long ulong;
typedef unsigned char byte;

// Free  = entirely unused, completely free for use right now
// Avail = includes reserved memory that will be freed if needed
// Bound = reserved by other applications, can't be used at all (total - free)
// Used  = in use, but part of it could be freed if needed (total - avail)
struct info
{
	ulong total_abs;
	ulong free_abs; 
	ulong avail_abs;
	ulong bound_abs;
	ulong used_abs;
	double total_rel;
	double free_rel;
	double avail_rel;
	double bound_rel;
	double used_rel;
};

typedef struct info info_s;

struct options
{
	byte help : 1;
	byte version : 1;
	byte monitor : 1;      // keep running and printing
	byte space : 1;
	byte binary : 1;       // use power-of-two instead of decimal units
	byte unit : 1;
	byte continuous : 1;   // keep printing even if value didn't change
	int interval;          // run main loop every `interval` seconds
	int precision;         // number of decimal places in output
	double threshold;      // minimum change in value to issue a print 
	char *format;
	char *file;            // file to read memory stats from
	char granularity;      // unit granulairty (m = mega, g = giga, etc)

	ulong unit_size;       // will be set by program
	char *unit_abbr;       // will be set by program
};

typedef struct options opts_s;

struct context
{
	info_s* info;
	opts_s* opts;
	char buffer[RESULT_SIZE];
	char output_prev[OUTPUT_SIZE];
	char output_curr[OUTPUT_SIZE];
};

typedef struct context ctx_s;

/**
 * Process command line options into the provided options struct.
 */
static void
fetch_opts(opts_s *opts, int argc, char **argv)
{
	opterr = 0;
	int o;
	while ((o = getopt(argc, argv, "bhmg:i:kp:f:F:suV")) != -1)
	{
		switch(o)
		{
			case 'b':
				opts->binary = 1;
				break;
			case 'f':
				opts->format = optarg;
				break;
			case 'F':
				opts->file = optarg;
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
			case 'k':
				opts->continuous = 1;
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
			case 'u':
				opts->unit = 1;
				break;
			case 'V':
				opts->version = 1;
				break;
		}
	}
}

/**
 * Prints usage information.
 */
static void
help(char *invocation, FILE* stream)
{
	fprintf(stream, "Usage:\n");
     	fprintf(stream, "\t%s [OPTION...]\n", invocation);
	fprintf(stream, "\n");
	fprintf(stream, "Options:\n");
	fprintf(stream, "\t-b Use binary instead of decimal units\n");
	fprintf(stream, "\t-f FORMAT Format string, see below; default is '%%b'\n");
	fprintf(stream, "\t-F FILE File to query for memory info; default is '/proc/meminfo'\n");
	fprintf(stream, "\t-g GRANULARITY Value granularity (k, m, g, t, p); default is 'g'\n");
	fprintf(stream, "\t-h Print this help text and exit\n");
	fprintf(stream, "\t-i INTERVAL Seconds between reading memory usage; default is 1\n");
	fprintf(stream, "\t-k Keep printing even if the output hasn't changed\n");
	fprintf(stream, "\t-m Keep running and print when there is a change in output\n"); 
	fprintf(stream, "\t-p Number of decimal digits in the output; default is 0\n");
	fprintf(stream, "\t-s Print a space between value and unit\n");
	fprintf(stream, "\t-u Print the appropriate unit after the value\n");
	fprintf(stream, "\t-V Print version information and exit\n");
	fprintf(stream, "\n");
	fprintf(stream, "Format specifiers:\n");
	fprintf(stream, "\t%%T and %%t: Total memory (absolute and percent)\n");
	fprintf(stream, "\t%%F and %%f: Free memory (absolute and percent)\n");
	fprintf(stream, "\t%%A and %%a: Available memory (absolute and percent)\n");
	fprintf(stream, "\t%%B and %%b: Bound memory (absolute and percent)\n");
	fprintf(stream, "\t%%U and %%u: Used memory (absolute and percen)\n");
}

/*
 * Print version information.
 */
static void
version(FILE *stream)
{
	fprintf(stream, "%s %d.%d.%d\n%s\n", PROGRAM_NAME,
			PROGRAM_VER_MAJOR, PROGRAM_VER_MINOR, PROGRAM_VER_PATCH,
			PROGRAM_URL);
}

/**
 * Given a line from `/proc/meminfo` (or a file of the same format), extracts 
 * the memory value and stores it in `val`. Returns 0 on success, -1 on error.
 */
static int
extract_value(char *buf, ulong *val)
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
 * format) line by line, looking for values of interest. If found, it tries to
 * extract the memory values from those lines and places them into `info`.
 * Returns 0 if all values were extracted successfully, otherwise -1.
 */
static int
fetch_info(info_s* info, opts_s* opts)
{
	FILE *fp = fopen(opts->file, "r");
	if (fp == NULL)
	{
		return -1;
	}

	// Read the file line by line
	char *buf = NULL;
	size_t len = 0;
	while (getline(&buf, &len, fp) != -1)
	{
		// Check if the line starts with `str_total`
		if (strncmp(STR_MEM_TOTAL, buf, strlen(STR_MEM_TOTAL)) == 0)
		{
			extract_value(buf, &info->total_abs);
		}
		// Check if the line starts with `str_free`
		else if (strncmp(STR_MEM_FREE, buf, strlen(STR_MEM_FREE)) == 0)
		{
			extract_value(buf, &info->free_abs);
		}
		// Check if the line starts with `str_avail`
		else if (strncmp(STR_MEM_AVAIL, buf, strlen(STR_MEM_AVAIL)) == 0)
		{
			extract_value(buf, &info->avail_abs);
		}
		// We've found everything we were looking for, end the loop 
		if (info->total_abs && info->free_abs && info->avail_abs)
		{
			break;
		}
	}

	free(buf);
	fclose(fp);

	// Indicate error if we couldn't get the bare minimum info
	if (info->total_abs == 0)
	{
		return -1;
	}

	// If MemAvailable wasn't present (older kernels), we use MemFree instead
	if (info->avail_abs == 0)
	{
		info->avail_abs = info->free_abs;
	}

	
	// Calculate all other values that we derive from those from the file
	info->total_rel = 100;
	info->free_rel  = ((double) info->free_abs  / (double) info->total_abs) * 100;
	info->avail_rel = ((double) info->avail_abs / (double) info->total_abs) * 100;

	info->bound_abs = info->total_abs - info->free_abs;
	info->bound_rel = ((double) info->bound_abs / (double) info->total_abs) * 100;

	info->used_abs  = info->total_abs - info->avail_abs;
	info->used_rel  = ((double) info->used_abs / (double) info->total_abs) * 100;

	return 0; 
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
		(val * 1024.0) / opts->unit_size, // KiB to bytes to user-selected unit
		opts->space && opts->unit ? " " : "",
	       	opts->unit ? opts->unit_abbr : ""
	);
}

static char*
candy_format_cb(char c, void* context)
{
	ctx_s* ctx = (ctx_s*) context;

	switch (c)
	{
		case 't':
			format_rel_value(ctx->buffer, RESULT_SIZE,
					ctx->info->total_rel, ctx->opts);
			return ctx->buffer;
		case 'a':
			format_rel_value(ctx->buffer, RESULT_SIZE, 
					ctx->info->avail_rel, ctx->opts);
			return ctx->buffer;
		case 'f':
			format_rel_value(ctx->buffer, RESULT_SIZE, 
					ctx->info->free_rel, ctx->opts);
			return ctx->buffer;
		case 'b':
			format_rel_value(ctx->buffer, RESULT_SIZE, 
					ctx->info->bound_rel, ctx->opts);
			return ctx->buffer;
		case 'u':

			format_rel_value(ctx->buffer, RESULT_SIZE, 
					ctx->info->used_rel, ctx->opts);
			return ctx->buffer;
		case 'T':
			format_abs_value(ctx->buffer, RESULT_SIZE, 
					ctx->info->total_abs, ctx->opts);
			return ctx->buffer;
		case 'A':
			format_abs_value(ctx->buffer, RESULT_SIZE, 
					ctx->info->avail_abs, ctx->opts);
			return ctx->buffer;
		case 'F':
			format_abs_value(ctx->buffer, RESULT_SIZE, 
					ctx->info->free_abs, ctx->opts);
			return ctx->buffer;
		case 'B':
			format_abs_value(ctx->buffer, RESULT_SIZE, 
					ctx->info->bound_abs, ctx->opts);
			return ctx->buffer;
		case 'U':
			format_abs_value(ctx->buffer, RESULT_SIZE, 
					ctx->info->used_abs, ctx->opts);
			return ctx->buffer;

		default:
			return NULL;
	}
}

static void
format_info(ctx_s* ctx)
{
	candy_format(ctx->opts->format, ctx->output_curr, OUTPUT_SIZE, candy_format_cb, ctx);
}

int
main(int argc, char **argv)
{
	opts_s opts = { .threshold = -1 };
	fetch_opts(&opts, argc, argv);

	if (opts.help)
	{
		help(argv[0], stdout);
		return EXIT_SUCCESS;
	}

	if (opts.version)
	{
		version(stdout);
		return EXIT_SUCCESS;
	}

	// If no file given, use the default
	if (opts.file == NULL)
	{
		opts.file = DEFAULT_PROCFILE;
	}
	
	// If no interval given, use the default
	if (opts.interval == 0)
	{
		opts.interval = DEFAULT_INTERVAL;
	}

	// Reset interval to 0 if we don't monitor
	if (opts.monitor == 0)
	{
		opts.interval = 0;
	}

	// If no granularity given, use the default
	if (opts.granularity == 0)
	{
		opts.granularity = *DEFAULT_GRANULARITY;
	}

	// If not format given, use the default
	if (opts.format == NULL)
	{
		opts.format = DEFAULT_FORMAT;
	}

	// Make sure stdout is line buffered
	setlinebuf(stdout);

	// Data structures we'll need going forward 
	info_s info = { 0 };
	ctx_s ctx = { .info = &info, .opts = &opts };

	// Set additional options based on 'granularity'
	//set_unit(&info, &opts);
	candy_unit_info(opts.granularity, opts.binary, &opts.unit_size, &opts.unit_abbr);

	// do-while, because we need to run at least once either way
	do
	{
		// Empty the gathered information, if any
		info = (const info_s) { 0 };
		
		// Get the current memory usage
		if (fetch_info(&info, &opts) == -1)
		{
			return EXIT_FAILURE;
		}

		// Formulate the final output string based on opts.format
		format_info(&ctx);

		// Print
		if (opts.continuous || strcmp(ctx.output_prev, ctx.output_curr) != 0)
		{
			fprintf(stdout, "%s\n", ctx.output_curr);
		}
		
		// Update values for next iteration
		strcpy(ctx.output_prev, ctx.output_curr);

		// Sleep, maybe (if interval > 0)
		sleep(opts.interval);
	}
	while (opts.monitor);

	return EXIT_SUCCESS;
}

