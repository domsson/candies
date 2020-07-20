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
	size_t i = 0;
	size_t b = 0;
	size_t n = 0;
	char *add = NULL;

	for (n = strlen(format); i < n && b < (len-1); ++i)
	{
		if (format[i] == '%' && i+1 < n) 
		{
			if (format[i+1] == '%')
			{
				buf[b++] = format[++i];
				continue;
			}
			if ((add = cb(format[i+1], ctx)))
			{
				while (*add && b < (len-1))
				{
					buf[b++] = *add++;
				}
				++i;
				continue;
			}
		}
	
		buf[b++] = format[i];
	}

	buf[b] = '\0';
	return buf;
}

#endif
