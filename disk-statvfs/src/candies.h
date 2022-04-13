#ifndef CANDIES_H
#define CANDIES_H

#ifndef CANDIES_API
#define CANDIES_API
#endif

#include <stddef.h>     // NULL
#include <string.h>     // strlen()

#define KIBIBYTE_SIZE 1024L
#define MEBIBYTE_SIZE KIBIBYTE_SIZE * KIBIBYTE_SIZE
#define GIBIBYTE_SIZE MEBIBYTE_SIZE * KIBIBYTE_SIZE
#define TEBIBYTE_SIZE GIBIBYTE_SIZE * KIBIBYTE_SIZE
#define PEBIBYTE_SIZE TEBIBYTE_SIZE * KIBIBYTE_SIZE

#define KILOBYTE_SIZE 1000L
#define MEGABYTE_SIZE KILOBYTE_SIZE * KILOBYTE_SIZE
#define GIGABYTE_SIZE MEGABYTE_SIZE * KILOBYTE_SIZE
#define TERABYTE_SIZE GIGABYTE_SIZE * KILOBYTE_SIZE
#define PETABYTE_SIZE TERABYTE_SIZE * KILOBYTE_SIZE

#define KIBIBYTE_ABBR "KiB"
#define MEBIBYTE_ABBR "MiB"
#define GIBIBYTE_ABBR "GiB"
#define TEBIBYTE_ABBR "TiB"
#define PEBIBYTE_ABBR "PiB"

#define KILOBYTE_ABBR "KB"
#define MEGABYTE_ABBR "MB"
#define GIGABYTE_ABBR "GB"
#define TERABYTE_ABBR "TB"
#define PETABYTE_ABBR "PB"

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

CANDIES_API void
candy_unit_info(char granularity, unsigned char binary, unsigned long* size, char** abbr)
{
	switch(granularity)
	{
		case 'k':
			*size = binary ? KIBIBYTE_SIZE : KILOBYTE_SIZE;
			*abbr = binary ? KIBIBYTE_ABBR : KILOBYTE_ABBR;
			break;
		case 'm':
			*size = binary ? MEBIBYTE_SIZE : MEGABYTE_SIZE;
			*abbr = binary ? MEBIBYTE_ABBR : MEGABYTE_ABBR;
			break;
		case 'g':
			*size = binary ? GIBIBYTE_SIZE : GIGABYTE_SIZE;
			*abbr = binary ? GIBIBYTE_ABBR : GIGABYTE_ABBR;
			break;
		case 't':
			*size = binary ? TEBIBYTE_SIZE : TERABYTE_SIZE;
			*abbr = binary ? TEBIBYTE_ABBR : TERABYTE_ABBR;
			break;
		case 'p':
			*size = binary ? PEBIBYTE_SIZE : PETABYTE_SIZE;
			*abbr = binary ? PEBIBYTE_ABBR : PETABYTE_ABBR;
			break;
	}
}


#endif
