#ifndef CANDIES_H
#define CANDIES_H

#ifndef CANDIES_API
#define CANDIES_API
#endif

#include <stddef.h>     // NULL
#include <string.h>     // strlen()

CANDIES_API char*
candy_format_cb(char c, void* ctx);


CANDIES_API char*
candy_format(const char* format, char *buf, size_t len, char* (*cb)(char c, void* ctx), void *ctx)
{
	const char *curr;  // current char from format
	const char *next;  // next char from format

	size_t i = 0;      // index into buf
	char *ins = NULL;  // string to insert

	// iterate `format`, abort once we exhaust the output buffer
	for (; *format && i < (len-1); ++format)
	{
		curr = format;
		next = format+1;

		if (*curr == '%' && *next) 
		{
			if (*next == '%') // escaped %, copy it over and skip
			{
				buf[i++] = *format++;
				continue;
			}
			if ((ins = cb(*next, ctx))) // get string to insert
			{
				// copy string, again aborting once buffer full
				while (*ins && i < (len-1))
				{
					buf[i++] = *ins++;
				}
				++format;
				continue;
			}
		}
	
		// any other character, just copy over
		buf[i++] = *curr;
	}

	// null terminate
	buf[i] = '\0';
	return buf;
}

#endif
