#include <stdio.h>      // NULL, fprintf(), perror(), setlinebuf()
#include <string.h>     // strcmp(), strcasestr()
#include <stdlib.h>     // NULL, EXIT_FAILURE, EXIT_SUCCESS
#include <unistd.h>     // getopt() et al.
#include <error.h>
#include <arpa/inet.h>

#define TCPSOCK_IMPLEMENTATION
#include "tcpsock.h"

#define DEFAULT_PORT "25565"

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

struct options
{
	char  *host;
	char  *port;
	byte   help : 1;        // Print help and exit
	byte   monitor : 1;
	byte   space : 1;
	mode_e mode;
	int interval;
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
	fprintf(stdout, "\t%s [OPTIONS...] -a [IP]\n", invocation);
	fprintf(stdout, "\n");
	fprintf(stdout, "Options:\n");
	fprintf(stdout, "\t-h\tPrint this help text and exit.\n");
	fprintf(stdout, "\t-i SECS\tQuery server every SECS seconds. (NOT IMPLEMENTED)\n");
	fprintf(stdout, "\t-m\tKeep running, printing new info when available. (NOT IMPLEMENTED)\n");
	fprintf(stdout, "\t-o\tOutput format mode (0..4).\n");
	fprintf(stdout, "\t-p\tServer port (default is 25565).\n");
	fprintf(stdout, "\t-s\tAdd space before and after the slash (mode 0 only).\n");
	fprintf(stdout, "\n");
	fprintf(stdout, "Format modes:\n");
	fprintf(stdout, "\t0\t<players> / <slots>\n");
	fprintf(stdout, "\t1\t<players>\n");
	fprintf(stdout, "\t2\t<slots>\n");
	fprintf(stdout, "\t3\t<version>\n");
	fprintf(stdout, "\t4\t<message of the day>\n");
	fprintf(stdout, "\n");
}

/*
 * Here be tempeh burgers!
 */
int main(int argc, char **argv)
{
	// Set stdout to line buffered
	setlinebuf(stdout);
	
	// Get an options struct	
	opts_s opts = { 0 };
	
	// Process command line options
	opterr = 0;
	int o;
	while ((o = getopt(argc, argv, "a:hi:mo:p:s")) != -1)
	{
		switch(o)
		{
			case 'a':
				opts.host = optarg;
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
			case 'o':
				opts.mode = atoi(optarg);
				break;
			case 'p':
				opts.port = optarg;
				break;
			case 's':
				opts.space = 1;
				break;
		}
	}

	if (opts.help)
	{
		help(argv[0]);
		return EXIT_SUCCESS;
	}
	
	// DO THE THING

	int sock = tcpsock_create(TCPSOCK_IPV4);

	if (sock == -1)
	{
		return EXIT_FAILURE;
	}

	if (opts.port == NULL)
	{
		opts.port = DEFAULT_PORT;
	}

	if (tcpsock_connect(sock, TCPSOCK_IPV4, opts.host, opts.port) == -1)
	{
		return EXIT_FAILURE;
	}

	char msg[] = { 0xFE, 0x01 }; 
	int sent = tcpsock_send(sock, msg, 2);
	if (sent == -1)
	{
		return EXIT_FAILURE;
	}

	char buf[128] = { 0 };
	int recv = tcpsock_receive(sock, buf, 128);

	// https://wiki.vg/Server_List_Ping#Server_to_client

	//char protocol[4] = { 0 };
	char version[8]  = { 0 };
	char motd[64]    = { 0 };
	char players[4]  = { 0 };
	char slots[4]    = { 0 };

	int i = 10;
	int j = 0;

	for (j = 0; i < recv; i += 2)
	{
		if (buf[i] == '\0')
		{
			i += 2;
			break;
		}

		//protocol[p++] = buf[i];
		j++;
	}

	for (j = 0; i < recv; i += 2)
	{
		if (buf[i] == '\0')
		{
			i += 2;
			break;
		}

		version[j++] = buf[i];
	}

	for (j = 0; i < recv; i += 2)
	{
		if (buf[i] == '\0')
		{
			i += 2;
			break;
		}

		motd[j++] = buf[i];
	}

	for (j = 0; i < recv; i += 2)
	{
		if (buf[i] == '\0')
		{
			i += 2;
			break;
		}

		players[j++] = buf[i];
	}

	for (j = 0; i < recv; i += 2)
	{
		if (buf[i] == '\0')
		{
			i += 2;
			break;
		}

		slots[j++] = buf[i];
	}

	char space = opts.space ? ' ' : 0;

	switch(opts.mode)
        {
		case MODE_PLAYERS_AND_SLOTS:
			fprintf(stdout, "%s%c/%c%s\n", players, space, space, slots);
			break;
		case MODE_PLAYERS:
			fprintf(stdout, "%s\n", players);
			break;
		case MODE_SLOTS:
			fprintf(stdout, "%s\n", slots);
			break;
		case MODE_VERSION:
			fprintf(stdout, "%s\n", version);
			break;
		case MODE_MOTD:
			fprintf(stdout, "%s\n", motd);
			break;
	}

	return EXIT_SUCCESS;
}

