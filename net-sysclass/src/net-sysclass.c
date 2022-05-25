#include <stdio.h>            // fprintf(), getline(), fopen(), ...
#include <stdlib.h>           // NULL, EXIT_* 
#include <unistd.h>           // getopt() et al., access()
#include <string.h>           // strtok()
#include <ctype.h>            // tolower()
#include <math.h>             // pow(), fabs()

#define CANDIES_API static
#include "candies.h"

#define PROGRAM_NAME "net-proc"
#define PROGRAM_URL  "https://github.com/domsson/candies/net-sysclass"

#define PROGRAM_VER_MAJOR 0
#define PROGRAM_VER_MINOR 2
#define PROGRAM_VER_PATCH 0

#define DEFAULT_INTERVAL     1
#define DEFAULT_THRESHOLD    1
#define DEFAULT_GRANULARITY "k"
#define DEFAULT_NIC_MBPS     100 // max iface speed in Mbits (100 Mbit = 0.1 Gbit)
#define DEFAULT_FORMAT      "%c" 

#define STATS_FILE_FORMAT "/sys/class/net/%s/statistics/%s"
#define STATS_FILE_RX     "rx_bytes"
#define STATS_FILE_TX     "tx_bytes"
#define STATS_FILE_BUFLEN  64

#define OUTPUT_SIZE 128
#define RESULT_SIZE 16

//	/sys/class/net/<iface>/statistics/rx_bytes
//	/sys/class/net/<iface>/statistics/tx_bytes

typedef unsigned long ulong;
typedef unsigned char byte;

struct info
{
	ulong rx_abs; // received (down)
	ulong tx_abs; // transferred (up)
	ulong cx_abs; // combined (up & down)
	double rx_rel;
	double tx_rel;
	double cx_rel;
};

typedef struct info info_s;

struct options
{
	byte monitor : 1;    // keep running and printing
	byte continuous : 1; // keep printing, regardles of threshold
	byte unit : 1;	     // also print the % unit
	byte space : 1;      // space between val and unit
	byte help : 1;       // show help and exit
	byte version : 1;    // show version info and exit
	int interval;        // print every `interval` seconds
	int precision;       // decimal places in output
	int nic_mbps;        // network interface card max speed in Mbps
	char granularity;    // unit granularity (m = mega, g = giga, etc)
	char *iface;         // network interface to query
	char *format;        // format string
	ulong max_speed;     // max iface throughput in bytes

	// these will be set by the program
	char rx_file[STATS_FILE_BUFLEN]; 
	char tx_file[STATS_FILE_BUFLEN];
	ulong unit_size;
	char *unit_abbr;
};

typedef struct options opts_s;

struct context
{
	info_s *info;
	opts_s *opts;
	char buffer[RESULT_SIZE];
	char output_prev[OUTPUT_SIZE];
	char output_curr[OUTPUT_SIZE];
};

typedef struct context ctx_s;

static void
fetch_opts(opts_s *opts, int argc, char **argv)
{
	opterr = 0;
	int o;
	while ((o = getopt(argc, argv, "f:g:hi:I:kmp:r:suV")) != -1)
	{
		switch (o)
		{
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
			case 'I':
				opts->iface = optarg;
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
			case 'r':
				opts->nic_mbps = atoi(optarg);
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
     	fprintf(stream, "\t%s [OPTIONS...]\n", invocation);
	fprintf(stream, "\n");
	fprintf(stream, "Options:\n");
	fprintf(stream, "\t-f Output format string\n");
	fprintf(stream, "\t-h Print this help text and exit\n");
	fprintf(stream, "\t-i Seconds between checking for a change in value; default is 1\n");
	fprintf(stream, "\t-I Network interface of interest\n");
	fprintf(stream, "\t-k Keep printing, even if the values haven't changed\n");
	fprintf(stream, "\t-m Keep running and print when there is a notable change in value\n"); 
	fprintf(stream, "\t-p Number of decimal digits in the output; default is 0\n");
	fprintf(stream, "\t-r Speed rating of the network adapter in Mbit/s (100, 1000, ...)\n");
	fprintf(stream, "\t-s Print a space between value and unit\n");
	fprintf(stream, "\t-t Required change in value in order to print again; default is 1\n");
	fprintf(stream, "\t-u Print the appropriate unit after the value\n");
	fprintf(stream, "\t-V Print version information and exit\n");
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

static int
read_file_to_var(const char *file, ulong *value)
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

	*value = strtoul(buf, NULL, 0);
	free(buf);
	fclose(fp);
	return 0;
}

static int 
fetch_info(opts_s* opts, info_s* info, ulong* rx_prev, ulong* tx_prev)
{
	if (*rx_prev == 0 && *tx_prev == 0)
	{
		// TODO: add error handling (this might return -1)
		read_file_to_var(opts->rx_file, rx_prev);
		read_file_to_var(opts->tx_file, tx_prev);
	}

	sleep(opts->interval);
	
	ulong rx_curr = 0;
	ulong tx_curr = 0;
	
	// TODO: add error handling (this might return -1)
	read_file_to_var(opts->rx_file, &rx_curr);
	read_file_to_var(opts->tx_file, &tx_curr);
	
	ulong delta_rx = rx_curr - *rx_prev;
	ulong delta_tx = tx_curr - *tx_prev;

	*rx_prev = rx_curr;
	*tx_prev = tx_curr;

	// absolute values in bytes
	info->rx_abs = (ulong) (delta_rx / (float) opts->interval);
	info->tx_abs = (ulong) (delta_tx / (float) opts->interval);
	info->cx_abs = info->rx_abs + info->tx_abs;

	// bytes to Mbits
	double rx_abs_mbit = (info->rx_abs * 8.0) / 1000000.0;
	double tx_abs_mbit = (info->tx_abs * 8.0) / 1000000.0;

	// relative values in percent of NIC max throughput
	info->rx_rel = (rx_abs_mbit / (double) opts->nic_mbps) * 100.0;
	info->tx_rel = (tx_abs_mbit / (double) opts->nic_mbps) * 100.0;
	info->cx_rel = info->rx_rel + info->tx_rel; 
	
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
		(val * 8) / (double) opts->unit_size, // bytes to bits to user-selected unit
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
		case 'r': // rx (down), relative
			format_rel_value(ctx->buffer, RESULT_SIZE,
					ctx->info->rx_rel, ctx->opts);
			return ctx->buffer;
		case 't': // tx (up), relative
			format_rel_value(ctx->buffer, RESULT_SIZE,
					ctx->info->tx_rel, ctx->opts);
			return ctx->buffer;
		case 'c': // combined, relative
			format_rel_value(ctx->buffer, RESULT_SIZE,
					ctx->info->cx_rel, ctx->opts);
			return ctx->buffer;
		case 'R': // rx (down), absolute
			format_abs_value(ctx->buffer, RESULT_SIZE,
					ctx->info->rx_abs, ctx->opts);
			return ctx->buffer;
		case 'T': // tx (up), absolute
			format_abs_value(ctx->buffer, RESULT_SIZE,
					ctx->info->tx_abs, ctx->opts);
			return ctx->buffer;
		case 'C': // combined, absolute
			format_abs_value(ctx->buffer, RESULT_SIZE,
					ctx->info->cx_abs, ctx->opts);
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

	if (opts.iface == NULL)
	{
		return EXIT_FAILURE;
	}

	if (opts.threshold == -1)
	{
		opts.threshold = DEFAULT_THRESHOLD / pow(10.0, (double) opts.precision);
	}

	if (opts.interval == 0)
	{
		// We need some interval, as we need to take two measurements
		opts.interval = DEFAULT_INTERVAL;
	}

	if (opts.nic_mbps == 0)
	{
		opts.nic_mbps = DEFAULT_NIC_MBPS;
	}

	// if no granularity given, use the default
	if (opts.granularity == 0)
	{
		opts.granularity = *DEFAULT_GRANULARITY;
	}

	// if not format given, use the default
	if (opts.format == NULL)
	{
		opts.format = DEFAULT_FORMAT;
	}

	snprintf(opts.rx_file, STATS_FILE_BUFLEN, STATS_FILE_FORMAT, opts.iface, STATS_FILE_RX);
	snprintf(opts.tx_file, STATS_FILE_BUFLEN, STATS_FILE_FORMAT, opts.iface, STATS_FILE_TX);

	// check if file access is possible (reading)
	if (access(opts.rx_file, R_OK) != 0)
	{
		return EXIT_FAILURE;
	}
	
	// check if file access is possible (reading)
	if (access(opts.tx_file, R_OK) != 0)
	{
		return EXIT_FAILURE;
	}

	// make sure stdout is line buffered 
	setlinebuf(stdout);

	// data structures we'll need from here on out
	info_s info = { 0 };
	ctx_s ctx = { .info = &info, .opts = &opts };

	candy_unit_info(opts.granularity, 1, 0, &opts.unit_size, &opts.unit_abbr);

	// soymeat
	ulong rx = 0;
	ulong tx = 0;

	do
	{
		// zero out the gathered info from last iteration, if any
		info = (const info_s) { 0 };

		if (fetch_info(&opts, &info, &rx, &tx) == -1)
		{
			return EXIT_FAILURE;
		}

		format_info(&ctx);

		if (opts.continuous || strcmp(ctx.output_prev, ctx.output_curr) != 0)
		{
			fprintf(stdout, "%s\n", ctx.output_curr);
		}

		strcpy(ctx.output_prev, ctx.output_curr);
	}
	while (opts.monitor);

	return EXIT_SUCCESS;
}

