#include <stdio.h>      // NULL, fprintf(), perror(), setlinebuf()
#include <stdlib.h>     // NULL, EXIT_FAILURE, EXIT_SUCCESS
#include <unistd.h>     // getopt(), sleep()

#define TCPSOCK_IMPLEMENTATION
#define TCPSOCK_API static
#include "tcpsock.h"

#define CANDIES_API static
#include "candies.h"

#define DEFAULT_PORT "25565"
#define DEFAULT_INTERVAL 10
#define BUFFER_SIZE 128
#define DEFAULT_FORMAT "%p/%s"

typedef unsigned char byte;

struct info
{
	char protocol[4]; // max is "127"
	char version[9];  // "xx.xx.xx"
	char motd[60];    // max length for MOTD is 59
	char players[5];  // up to "9999"
	char slots[5];    // up to "9999"
};

typedef struct info info_s;

struct options
{
	char  *host;
	char  *port;
	byte   help : 1;        // Print help and exit
	byte   monitor : 1;     // Keep running, print new info when avail
	int    interval;        // Query server every `interval` seconds
	char  *format;
};

typedef struct options opts_s;

/*
 * Prints usage information.
 */
static void
help(char *invocation)
{
	fprintf(stdout, "Usage:\n");
	fprintf(stdout, "\t%s [OPTIONS...] server_ip\n", invocation);
	fprintf(stdout, "\n");
	fprintf(stdout, "Options:\n");
	fprintf(stdout, "\t-f FORMAT\tFormat string, see below.\n");
	fprintf(stdout, "\t-h\tPrint this help text and exit.\n");
	fprintf(stdout, "\t-i SECS\tQuery server every SECS seconds.\n");
	fprintf(stdout, "\t-m\tKeep running, printing new info when available.\n");
	fprintf(stdout, "\t-p PORT\tServer port (default is 25565).\n");
	fprintf(stdout, "\n");
	fprintf(stdout, "Format specifiers:\n");
	fprintf(stdout, "\t%%%%\ta literal %%\n");
	fprintf(stdout, "\t%%p\tnumber of players currently online\n");
	fprintf(stdout, "\t%%s\tnumber of player slots available\n");
	fprintf(stdout, "\t%%v\tserver version\n");
	fprintf(stdout, "\t%%m\tmessage of the day\n");
	fprintf(stdout, "\n");
}

static void
fetch_opts(opts_s *opts, int argc, char **argv)
{
	// Process command line options
	opterr = 0;
	int o;
	while ((o = getopt(argc, argv, "f:hi:mo:p:")) != -1)
	{
		switch(o)
		{
			case 'f':
				opts->format = optarg;
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
				opts->port = optarg;
				break;
		}
	}

	if (optind < argc)
	{
		opts->host = argv[optind];
	}
}

static int
query_info(opts_s *opts, char *buf, int len)
{
	int sock = tcpsock_create(TCPSOCK_IPV4);

	// socket creation failed, bye bye
	if (sock == -1)
	{
		return -1;
	}

	// socket connection failed, bye bye
	if (tcpsock_connect(sock, TCPSOCK_IPV4, opts->host, opts->port) == -1)
	{
		return -1;
	}

	// send ping request to server
	char msg[] = { 0xFE, 0x01 }; 
	int sent = tcpsock_send(sock, msg, 2);

	// sending failed, bye bye
	if (sent == -1)
	{
		return -1;
	}

	int recv = tcpsock_receive(sock, buf, len);
	tcpsock_close(sock);
	return recv;
}

// https://wiki.vg/Server_List_Ping#Server_to_client
// Look, it ain't pretty, but it gets the job done.
static void
extract_info(info_s *info, char *buf, int len)
{
	int i = 10;
	int j = 0;

	for (j = 0; i < len; i += 2)
	{
		if (buf[i] == '\0')
		{
			i += 2;
			break;
		}

		info->protocol[j++] = buf[i];
	}

	for (j = 0; i < len; i += 2)
	{
		if (buf[i] == '\0')
		{
			i += 2;
			break;
		}

		info->version[j++] = buf[i];
	}

	for (j = 0; i < len; i += 2)
	{
		if (buf[i] == '\0')
		{
			i += 2;
			break;
		}

		info->motd[j++] = buf[i];
	}

	for (j = 0; i < len; i += 2)
	{
		if (buf[i] == '\0')
		{
			i += 2;
			break;
		}

		info->players[j++] = buf[i];
	}

	for (j = 0; i < len; i += 2)
	{
		if (buf[i] == '\0')
		{
			i += 2;
			break;
		}

		info->slots[j++] = buf[i];
	}
}

static char*
candy_format_cb(char c, void* ctx)
{
	info_s* info = (info_s*) ctx;

	switch (c)
	{
		case 'p':
			return info->players;
		case 's':
			return info->slots;
		case 'v':
			return info->version;
		case 'm':
			return info->motd;
		default:
			return NULL;
	}
}

static void
print_info(info_s *info, opts_s *opts)
{
	char result[BUFFER_SIZE] = { 0 };
	candy_format(opts->format, result, BUFFER_SIZE, candy_format_cb, info);	
	fprintf(stdout, "%s\n", result);
}

/*
 * Here be tempeh burgers!
 */
int
main(int argc, char **argv)
{
	// set stdout to line buffered
	setlinebuf(stdout);
	
	// parse command line arguments
	opts_s opts = { 0 };
	fetch_opts(&opts, argc, argv);
	
	// print help and exit
	if (opts.help)
	{
		help(argv[0]);
		return EXIT_SUCCESS;
	}

	if (opts.format == NULL)
	{
		opts.format = DEFAULT_FORMAT;
	}

	// if no interval given, use default
	if (opts.interval == 0)
	{
		opts.interval = DEFAULT_INTERVAL;
	}

	// if monitoring is off, set interval (back) to 0
	if (opts.monitor == 0)
	{
		opts.interval = 0;
	}

	// no host given, bye bye
	if (opts.host == NULL)
	{
		return EXIT_FAILURE;
	}
	
	// no port given, use default
	if (opts.port == NULL)
	{
		opts.port = DEFAULT_PORT;
	}

	// do the thing (possibly more than just once)
	char buf[BUFFER_SIZE];
	for (int run = 1; run; run = opts.monitor)
	{
		// query server for data
		int recv = query_info(&opts, buf, BUFFER_SIZE);
	
		// extract info from received data
		info_s info = { 0 };
		extract_info(&info, buf, recv);

		// print the requested info
		print_info(&info, &opts);

		// maybe sleep, maybe not
		sleep(opts.interval);
	}

	// done, bye
	return EXIT_SUCCESS;
}

