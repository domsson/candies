#include <stdio.h>      // NULL, fprintf(), perror(), setlinebuf()
#include <string.h>     // strcmp(), strcasestr()
#include <stdlib.h>     // NULL, EXIT_FAILURE, EXIT_SUCCESS
#include <unistd.h>     // getopt(), sleep()

#define TCPSOCK_IMPLEMENTATION
#include "tcpsock.h"

#define DEFAULT_PORT "25565"
#define DEFAULT_INTERVAL 10
#define BUFFER_SIZE 128

typedef unsigned char byte;

enum mode
{
	MODE_PLAYERS_AND_SLOTS,
	MODE_PLAYERS,
	MODE_SLOTS,
	MODE_VERSION,
	MODE_MOTD
};

typedef enum mode mode_e;

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
	byte   space : 1;       // Add spaces before/after the slash (mode 0)
	mode_e mode;            // What to display?
	int interval;           // Query server every `interval` seconds
};

typedef struct options opts_s;

/*
 * Returns 0 if str is NULL or empty, otherwise 1.
 */
int empty(char const *str)
{
	return str == NULL || str[0] == '\0';
}

/*
 * Prints usage information.
 */
void help(char *invocation)
{
	fprintf(stdout, "Usage:\n");
	fprintf(stdout, "\t%s [OPTIONS...] server_ip\n", invocation);
	fprintf(stdout, "\n");
	fprintf(stdout, "Options:\n");
	fprintf(stdout, "\t-h\tPrint this help text and exit.\n");
	fprintf(stdout, "\t-i SECS\tQuery server every SECS seconds. (NOT IMPLEMENTED)\n");
	fprintf(stdout, "\t-m\tKeep running, printing new info when available. (NOT IMPLEMENTED)\n");
	fprintf(stdout, "\t-o MODE\tOutput mode (0 through 4, see below).\n");
	fprintf(stdout, "\t-p PORT\tServer port (default is 25565).\n");
	fprintf(stdout, "\t-s\tAdd spaces before and after the slash (mode 0 only).\n");
	fprintf(stdout, "\n");
	fprintf(stdout, "Output modes:\n");
	fprintf(stdout, "\t0\t<players> / <slots>\n");
	fprintf(stdout, "\t1\t<players>\n");
	fprintf(stdout, "\t2\t<slots>\n");
	fprintf(stdout, "\t3\t<version>\n");
	fprintf(stdout, "\t4\t<message of the day>\n");
	fprintf(stdout, "\n");
}

void fetch_opts(opts_s *opts, int argc, char **argv)
{
	// Process command line options
	opterr = 0;
	int o;
	while ((o = getopt(argc, argv, "hi:mo:p:s")) != -1)
	{
		switch(o)
		{
			case 'h':
				opts->help = 1;
				break;
			case 'i':
				opts->interval = atoi(optarg);
				break;
			case 'm':
				opts->monitor = 1;
				break;
			case 'o':
				opts->mode = atoi(optarg);
				break;
			case 'p':
				opts->port = optarg;
				break;
			case 's':
				opts->space = 1;
				break;
		}
	}

	if (optind < argc)
	{
		opts->host = argv[optind];
	}
}

int query_info(opts_s *opts, char *buf, int len)
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

	// receive answer from server, hopefully
	/*
	char buf[128] = { 0 };
	int recv = tcpsock_receive(sock, buf, 128);
	*/

	return tcpsock_receive(sock, buf, len);
	
	/*
	// no data received or error, bye bye
	if (recv <= 0)
	{
		return -1;
	}
	*/
}

// https://wiki.vg/Server_List_Ping#Server_to_client
// Look, it ain't pretty, but it gets the job done.
void extract_info(info_s *info, char *buf, int len)
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

void print_info(info_s *info, opts_s *opts)
{
	char space = opts->space ? ' ' : 0;

        if (opts->mode == MODE_PLAYERS_AND_SLOTS)
	{
		fprintf(stdout, "%s%c/%c%s\n", info->players, space, space, info->slots);
		return;
	}

	if (opts->mode == MODE_PLAYERS)
	{
		fprintf(stdout, "%s\n", info->players);
		return;
	}

	if (opts->mode == MODE_SLOTS)
	{
		fprintf(stdout, "%s\n", info->slots);
		return;
	}

	if (opts->mode == MODE_VERSION)
	{
		fprintf(stdout, "%s\n", info->version);
		return;
	}

	if (opts->mode == MODE_MOTD)
	{
		fprintf(stdout, "%s\n", info->motd);
		return;
	}
}

/*
 * Here be tempeh burgers!
 */
int main(int argc, char **argv)
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

	if (opts.interval == 0)
	{
		opts.interval = DEFAULT_INTERVAL;
	}

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

		sleep(opts.interval);
	}

	// done, bye
	return EXIT_SUCCESS;
}

