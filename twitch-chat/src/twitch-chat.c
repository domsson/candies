#include <stdio.h>      // NULL, fprintf(), perror(), setlinebuf()
#include <string.h>     // strcmp(), strcasestr()
#include <stdlib.h>     // NULL, EXIT_FAILURE, EXIT_SUCCESS
#include <unistd.h>     // getopt() et al.
#include "libtwirc.h"

#define DEFAULT_HOST "irc.chat.twitch.tv"
#define DEFAULT_PORT "6667"
#define MAX_MSG_LEN   2048

enum usertype
{
	USER_ALL,
	USER_VIP,
	USER_SUB,
	USER_MOD
};

typedef enum usertype usertype_e;

struct metadata
{
	char      *channel;           // Channel to join
	char      *filter;            // String to look for
	int        help : 1;          // Print help and exit
	int        maxlen;            // Maximum message length to print
	usertype_e usertype;          // ALL < VIP < SUB < MOD
	int        bits;              // Only print messages with this many bits
	int        badges : 1;        // Print 'badges'
	int        omit_names : 1;    // Only print messages, no user names
	int        displaynames : 1;  // Favor display over user names
};

typedef struct metadata metadata_s;

/*
 * Returns 0 if str is NULL or empty, otherwise 1.
 */
int empty(char const *str)
{
	return str == NULL || str[0] == '\0';
}

/*
 * Called once we're authenticated. This is where we can join channels etc.
 */
void handle_welcome(twirc_state_t *s, twirc_event_t *evt)
{
	metadata_s *meta = twirc_get_context(s);

	// Let's join the channel of interest
	twirc_cmd_join(s, meta->channel);
}

/*
 * Given the "badges" tag, determines if the origin has moderator permissions.
 * This means the origin (the user who caused the event) is either a moderator
 * or the broadcaster of the channel the event occurred in.
 * Returns 1 for mods, 0 for non-mods, -1 on error.
 */
int is_mod(char const *badges)
{
	return badges && (strstr(badges, "moderator") || strstr(badges, "broadcaster"));
}

/*
 * Given the "badges" tag, determines if the origin is a subscriber.
 * Returns 1 for subs, 0 for non-subs, -1 on error.
 */
int is_sub(char const *badges)
{
	return badges && strstr(badges, "subscriber");
}

int is_vip(char const *badges)
{
	return badges && strstr(badges, "vip");
}

int has_bits(twirc_event_t *evt)
{
	twirc_tag_t *bits = twirc_get_tag(evt->tags, "bits");
	return bits && bits->value ? atoi(bits->value) : 0;
}

void print_msg(char const *badge, char const *nick, char const *msg, int action, int maxlen)
{
	size_t nick_len = TWIRC_NICK_SIZE + 4;
	char nick_buf[TWIRC_NICK_SIZE + 4] = { 0 };

	size_t len = nick_len + strlen(msg) + 1;
	char  *buf = malloc(len);

	if (!empty(nick))
	{
		//                              .-- badge 
		//                              | .-- nick
		//                              | | .-- ": "
		//                              | | | 
		snprintf(nick_buf, nick_len , "%s%s%s", badge, nick, action ? " " : ": ");
	}

	//                  .-- nick string
	//                  | .-- message
	//                  | |
	snprintf(buf, len, "%s%s", nick_buf, msg);

	fprintf(stdout, "%.*s\n", maxlen == 0 ? MAX_MSG_LEN : maxlen, buf);
	free(buf);
}

void handle_message(twirc_state_t *s, twirc_event_t *evt)
{
	struct metadata *meta = twirc_get_context(s);

	if (meta->filter && strstr(evt->message, meta->filter) == NULL)
	{
		return;
	}

	char const *badges = twirc_get_tag_value(evt->tags, "badges");
	char const *dname  = twirc_get_tag_value(evt->tags, "display-name");

	int bits = has_bits(evt);
	if (bits < meta->bits)
	{
		return;
	}

	// Prepare nickname string
	char nick[TWIRC_NICK_SIZE] = { 0 };
	if (meta->omit_names == 0)
	{
		snprintf(nick, TWIRC_NICK_SIZE, "%s", meta->displaynames && !empty(dname) ? dname : evt->origin);
	}
	
	// Prepare badges string
	char badge[2] = { 0 };
	if (meta->badges)
	{
		if (is_vip(badges) || is_sub(badges))
		{
			snprintf(badge, 2, "%s", "+");
		}
		if (is_mod(badges))
		{
			snprintf(badge, 2, "%s", "@");
		}
	}

	print_msg(badge, nick, evt->message, evt->ctcp ? 1 : 0, meta->maxlen);
}

/**
 * Prints usage information.
 */
void help(char *invocation)
{
	fprintf(stdout, "Usage:\n");
	fprintf(stdout, "\t%s [OPTIONS...] #channel\n", invocation);
	fprintf(stdout, "\tNote: the channel should start with '#' and be all lower-case.\n");
	fprintf(stdout, "\n");
	fprintf(stdout, "Options:\n");
	fprintf(stdout, "\t-b\tMark VIPs and subs width '+', mods '@'.\n");
	fprintf(stdout, "\t-d\t Use display names instead of user names where available.\n");
	fprintf(stdout, "\t-f STR\t Only print messages containing this exact string (case sensitive).\n");
	fprintf(stdout, "\t-g INT\t Only print cheer messages with at least this many bits.\n");
	fprintf(stdout, "\t-h\t Print this help text and exit.\n");
	fprintf(stdout, "\t-n INT\t Maximum message length.\n");
	fprintf(stdout, "\t-o\t Omit user names, only print chat messages.\n");
	fprintf(stdout, "\t-p INT\t Required user permission (0: all, 1: VIP, 2: sub, 3: mod).\n");
	fprintf(stdout, "\n");
}

/*
 * Main - this is where we make things happen!
 */
int main(int argc, char **argv)
{
	// Set stdout to line buffered
	setlinebuf(stdout);
	
	// Get a metadata struct	
	struct metadata m = { 0 };
	
	// Process command line options
	opterr = 0;
	int o;
	while ((o = getopt(argc, argv, "b:df:g:hn:op:")) != -1)
	{
		switch(o)
		{
			case 'b':
				m.badges = 1;
				break;
			case 'd':
				m.displaynames = 1;
				break;
			case 'f':
				m.filter = optarg;
				break;
			case 'g':
				m.bits = atoi(optarg);
				break;
			case 'h':
				m.help = 1;
				break;
			case 'n':
				m.maxlen = atoi(optarg);
				break;
			case 'o':
				m.omit_names = 1;
				break;			
			case 'p':
				m.usertype = atoi(optarg);
				break;
		}
	}

	if (optind < argc)
	{
		m.channel = argv[optind];
	}

	if (m.help)
	{
		help(argv[0]);
		return EXIT_SUCCESS;
	}

	// Abort if no channel name was given	
	if (m.channel == NULL)
	{
		fprintf(stderr, "lol");
		return EXIT_FAILURE;
	}

	// Create libtwirc state instance
	twirc_state_t *s = twirc_init();

	if (s == NULL)
	{
		return EXIT_FAILURE;
	}
	
	// Save the metadata in the state
	twirc_set_context(s, &m);

	// We get the callback struct from the libtwirc state
	twirc_callbacks_t *cbs = twirc_get_callbacks(s);

	// We assign our handlers to the events we are interested int
	cbs->welcome    = handle_welcome;
	cbs->action     = handle_message;
	cbs->privmsg    = handle_message;
	
	// Connect to the IRC server
	if (twirc_connect_anon(s, DEFAULT_HOST, DEFAULT_PORT) != 0)
	{
		return EXIT_FAILURE;
	}

	// Loop until shit goes south, then die
	twirc_loop(s);
	twirc_kill(s);
	return EXIT_SUCCESS;
}

