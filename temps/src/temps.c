#include <stdio.h>            // fprintf
#include <stdlib.h>           // NULL, EXIT_*
#include <unistd.h>           // getopt() et al.
#include <string.h>           // strstr()
#include <sensors/sensors.h>

#define UNIT_METRIC   "°C" 
#define UNIT_IMPERIAL "°F"

struct config 
{
	int list : 1;           // list chips and features
	int unit : 1;           // also print the °C unit
	int space : 1;          // space between val and unit
	int imperial : 1;       // use fahrenheit (imperial)
	int precision;          // decimal places in output
	char *unit_str;         // holds the actual unit string
	char *chip;	        // chip prefix to look for
	char *feat;	        // feature label to look for
};

double to_fahrenheit(double celcius)
{
	return (celcius * 1.8) + 32;
}

/**
 * Goes through the list of chips and looks for the one with the given prefix.
 * Returns the specified chip if found, otherwise NULL.
 */
sensors_chip_name const *find_chip(const char *prefix)
{
	sensors_chip_name const *cm = NULL; // matched chip
	int c = 0;

	// Iterate over the chips
	while(cm = sensors_get_detected_chips(NULL, &c))
	{
		// Check if this is the chip the user is interested in
		if (prefix && strstr(cm->prefix, prefix) != NULL)
		{
			return cm;
		}
	}
	return NULL;
}

/**
 * Tries to read the actual temperature value from the specified feature
 * of the specified chip. Writes the temperature vlue to ret. Returns 0 on
 * success, -1 on error. 
 */
int get_temp_value(const sensors_chip_name *chip, const sensors_feature *feat, double *ret)
{
	// Loop over the subfeatures
	sensors_subfeature const *sc; // current subfeature
	int s = 0;
	double val = 0.0;

	while(sc = sensors_get_all_subfeatures(chip, feat, &s))
	{
		// Not the tempreature reading? Not interested!
		if (sc->type != SENSORS_SUBFEATURE_TEMP_INPUT)
		{
			continue;
		}

		// Early return if value can't be read
		if ((sc->flags & SENSORS_MODE_R) == 0)
		{
			return -1;
		}

		int rc = sensors_get_value(chip, sc->number, ret);

		// Early return if value couldn't be read
		if (rc != 0)
		{
			return -1;
		}

		return 0;
	}
}

/**
 * List all temperature features of the provided chip, including their readings.
 * Temperature readings will be formatted according to precision and unit.
 */
void list_features(sensors_chip_name const *cm, struct config *cfg)
{
	// Iterate over the features
	sensors_feature const *fc = NULL; // current feature
	int f = 0;

	while (fc = sensors_get_features(cm, &f))
	{
		// Not a temperature? Not interested!
		if (fc->type != SENSORS_FEATURE_TEMP)
		{
			continue;
		}

		// Get the feature's label
		char *fl = sensors_get_label(cm, fc);
		if (fl == NULL)
		{
			// Error getting the label
			continue;
		}

		// Get the feature's value and print
		double temp;
		if (get_temp_value(cm, fc, &temp) == 0)
		{
			fprintf(stdout, " '-[F%d] %s (%.*f%s%s)\n", f, fl, 
				cfg->precision, 
				cfg->imperial ? to_fahrenheit(temp) : temp,
				cfg->space && cfg->unit ? " " : "",
				cfg->unit_str);
		}
		else
		{
			fprintf(stdout, " '-[F%d] %s (n/a)", f, fl); 
		}

		free(fl);
		fl = NULL;
	}
}

/**
 * Prints all detected chips and their features to stdout.
 */
void list_chips_and_features(struct config *cfg)
{
	// Iterate over the chips
	sensors_chip_name const *cc = NULL; // current chip
	int c = 0;

	while(cc = sensors_get_detected_chips(NULL, &c))
	{
		fprintf(stdout, "[C%d] %s (from %s)\n", c, cc->prefix, cc->path);
		list_features(cc, cfg);
	}
}

/**
 * Prints the provided temperature value to stdout.
 */
void print_temp(double temp, int precision, const char *unit, int space)
{
	fprintf(stdout, "%.*f%s%s\n", precision, temp, 
			space && strlen(unit) ? " " : "", unit);
}

/**
 * Prints usage information.
 */
void help(char *invocation)
{
	fprintf(stderr, "Usage:\n");
     	fprintf(stderr, "\t%s [OPTION...] [-p PRECISION] -c CHIP -f FEATURE\n", invocation);
	fprintf(stderr, "\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "\t-h Print this help text and exit.\n");
	fprintf(stderr, "\t-i Use Fahrenheit instead of Celcius.\n");
	fprintf(stderr, "\t-l List all chips and features, then exit.\n");
	fprintf(stderr, "\t-u Include the temperature unit in the output.\n");
	fprintf(stderr, "\t-s Print a space between value and unit.\n");
}

/**
 * Sources:
 * - manpage for libsensors
 * - User Mat on Stack overflow: https://stackoverflow.com/a/8565176
 */
int main(int argc, char **argv)
{
	struct config cfg = { 0 };

	// Get arguments, if any
	opterr = 0;
	int o;
	while ((o = getopt(argc, argv, "c:f:p:iusvlh")) != -1)
	{
		switch (o)
		{
			case 'c':
				cfg.chip = optarg;
				break;
			case 'f':
				cfg.feat = optarg;
				break;
			case 'i':
				cfg.imperial = 1;
				break;
			case 'p':
				cfg.precision = atoi(optarg);
				break;
			case 'u':
				cfg.unit = 1;
				break;
			case 's':
				cfg.space = 1;
				break;
			case 'l':
				cfg.list = 1;
				break;
			case 'h':
				help(argv[0]);
				return EXIT_SUCCESS;
		}
	}

	// Set the unit string to imperial (F) or metric (C)
	cfg.unit_str = cfg.unit ? (cfg.imperial ? UNIT_IMPERIAL : UNIT_METRIC) : "";

	// Init sensors library
	if (sensors_init(NULL) != 0)
	{
		// Error initializing sensors
		return EXIT_FAILURE;
	}

	// List chips and exit (if that's what we're supposed to do)
	if (cfg.list)
	{
		list_chips_and_features(&cfg);
		sensors_cleanup();
		return EXIT_SUCCESS;
	}

	// Chip and/or feature not given? Print help and exit
	if (cfg.chip == NULL || cfg.feat == NULL)
	{
		help(argv[0]);
		sensors_cleanup();
		return EXIT_SUCCESS;
	}

	// Find the chip that matches the provided chip name
	sensors_chip_name const *cm = find_chip(cfg.chip);

	// Abort if we didn't find any matching chips
	if (cm == NULL)
	{
		sensors_cleanup();
		return EXIT_FAILURE;
	}
	
	// Iterate over the features
	sensors_feature const *fc = NULL; // current feature
	int f = 0;

	double temp = 0.0;
	size_t count = 0;

	while (fc = sensors_get_features(cm, &f))
	{
		// Not a temperature? Not interested!
		if (fc->type != SENSORS_FEATURE_TEMP)
		{
			continue;
		}

		// Get the feature's label
		char *fl = sensors_get_label(cm, fc);
		if (fl == NULL)
		{
			// Error getting the label
			continue;
		}

		// Check if this is a feature the user is interested in
		if (cfg.feat && strstr(fl, cfg.feat) != NULL)
		{
			// If so, we get the temperature and add it
			double curr = 0.0;

			if (get_temp_value(cm, fc, &curr) != 0)
			{
				continue;
			}

			temp += curr;
			++count;
		}

		free(fl);
		fl = NULL;
	}

	// We didn't manage to find a temp value
	if (count <= 0)
	{
		sensors_cleanup();
		return EXIT_FAILURE;
	}

	// Print the summed temperature value divided by the number of values
	setbuf(stdout, NULL);
	double final = cfg.imperial ? to_fahrenheit(temp / count) : temp / count;
	print_temp(final, cfg.precision, cfg.unit_str, cfg.space);

	// Cleanup
	sensors_cleanup();
	return EXIT_SUCCESS;
}
