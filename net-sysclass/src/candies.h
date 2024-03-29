#ifndef CANDIES_H
#define CANDIES_H

#ifndef CANDIES_API
#define CANDIES_API
#endif

#include <stddef.h>     // NULL
#include <string.h>     // strlen()

#define KILO_MULT 1000U
#define MEGA_MULT KILO_MULT * KILO_MULT
#define GIGA_MULT MEGA_MULT * KILO_MULT
#define TERA_MULT GIGA_MULT * KILO_MULT
#define PETA_MULT TERA_MULT * KILO_MULT

#define KIBI_MULT 1024U
#define MEBI_MULT KIBI_MULT * KIBI_MULT
#define GIBI_MULT MEBI_MULT * KIBI_MULT
#define TEBI_MULT GIBI_MULT * KIBI_MULT
#define PEBI_MULT TEBI_MULT * KIBI_MULT

#define KILOBYTE_ABBR "kB"
#define MEGABYTE_ABBR "MB"
#define GIGABYTE_ABBR "GB"
#define TERABYTE_ABBR "TB"
#define PETABYTE_ABBR "PB"

#define KIBIBYTE_ABBR "KiB"
#define MEBIBYTE_ABBR "MiB"
#define GIBIBYTE_ABBR "GiB"
#define TEBIBYTE_ABBR "TiB"
#define PEBIBYTE_ABBR "PiB"

#define KILOBIT_ABBR "kbit"
#define MEGABIT_ABBR "Mbit"
#define GIGABIT_ABBR "Gbit"
#define TERABIT_ABBR "Tbit"
#define PETABIT_ABBR "Pbit"

#define KIBIBIT_ABBR "Kibit"
#define MEBIBIT_ABBR "Mibit"
#define GIBIBIT_ABBR "Gibit"
#define TEBIBIT_ABBR "Tibit"
#define PEBIBIT_ABBR "Pibit"

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
candy_unit_info(char granularity, unsigned char bits, unsigned char binary, unsigned long* size, char** abbr)
{
	switch (granularity)
	{
		case 'k':
			*size = binary ? KIBI_MULT : KILO_MULT;
			*abbr = binary ? (bits ? KIBIBIT_ABBR : KIBIBYTE_ABBR) : (bits ? KILOBIT_ABBR : KILOBYTE_ABBR);
			break;
		case 'm':
			*size = binary ? MEBI_MULT : MEGA_MULT;
			*abbr = binary ? (bits ? MEBIBIT_ABBR : MEBIBYTE_ABBR) : (bits ? MEGABIT_ABBR : MEGABYTE_ABBR);
			break;
		case 'g':
			*size = binary ? GIBI_MULT : GIGA_MULT;
			*abbr = binary ? (bits ? GIBIBIT_ABBR : GIBIBYTE_ABBR) : (bits ? GIGABIT_ABBR : GIGABYTE_ABBR);
			break;
		case 't':
			*size = binary ? TEBI_MULT : TERA_MULT;
			*abbr = binary ? (bits ? TEBIBIT_ABBR : TEBIBYTE_ABBR) : (bits ? TERABIT_ABBR : TERABYTE_ABBR);
			break;
		case 'p':
			*size = binary ? PEBI_MULT : PETA_MULT;
			*abbr = binary ? (bits ? PEBIBIT_ABBR : PEBIBYTE_ABBR) : (bits ? PETABIT_ABBR : PETABYTE_ABBR);
			break;
	}
}

#endif
