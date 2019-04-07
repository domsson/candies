#include <stdio.h>            // fprintf
#include <stdlib.h>           // NULL, EXIT_*
#include <unistd.h>           // getopt() et al.
#include <string.h>           // strstr()
#include <sensors/sensors.h>

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
void list_features(sensors_chip_name const *cm, int precision, int unit)
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
			fprintf(stdout, " '-[%d] %s (%.*f%s)\n", f, fl, 
					precision, temp, unit ? " °C" : "");
		}
		else
		{
			fprintf(stdout, " '-[%d] %s (n/a)", f, fl); 
		}

		free(fl);
		fl = NULL;
	}
}

/**
 * Prints all detected chips and their features to stdout.
 */
void list_chips_and_features(int precision, int unit)
{
	// Iterate over the chips
	sensors_chip_name const *cc = NULL; // current chip
	int c = 0;

	while(cc = sensors_get_detected_chips(NULL, &c))
	{
		fprintf(stdout, "[%d] %s\n", c, cc->prefix);
		list_features(cc, precision, unit);
	}
}

/**
 * Prints the provided temperature value to stdout.
 */
void print_temp(double temp, int precision, int unit, int space)
{
	fprintf(stdout, "%.*f%s%s\n", precision, temp, 
			space && unit ? " " : "",
			unit ? "°C" : "");
}

/**
 * Prints usage information.
 */
void help(char *invocation)
{
	fprintf(stderr, "Usage:\n");
     	fprintf(stderr, "\t%s [OPTION...] -c CHIP -f FEATURE\n", invocation);
	fprintf(stderr, "\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "\t-h Print this help text and exit.\n");
	fprintf(stderr, "\t-l List all chips and features, then exit.\n");
	fprintf(stderr, "\t-u Include the temperature unit in the output.\n");
	fprintf(stderr, "\t-s Print a space between value and unit.\n");
	fprintf(stderr, "\t-p<int> Number of decimal places in the output.\n");
	fprintf(stderr, "\t-v Print additional information, similar to -l.\n");
}

/**
 * Sources:
 * - manpage for libsensors
 * - User Mat on Stack overflow: https://stackoverflow.com/a/8565176
 */
int main(int argc, char **argv)
{
	int list = 0;		// list chips and features
	int unit = 0;		// also print the °C unit
	int space = 0;          // space between val and unit
	int precision = 0;      // decimal places in output
	int verbose = 0;	// print additional info
	char *chip = NULL;	// chip prefix to look for
	char *feat = NULL;	// feature label to look for

	// Get arguments, if any
	opterr = 0;
	int o;
	while ((o = getopt(argc, argv, "c:f:p:usvlh")) != -1)
	{
		switch (o)
		{
			case 'c':
				chip = optarg;
				break;
			case 'f':
				feat = optarg;
				break;
			case 'p':
				precision = atoi(optarg);
				break;
			case 'u':
				unit = 1;
				break;
			case 's':
				space = 1;
				break;
			case 'v':
				verbose = 1;
				break;
			case 'l':
				list = 1;
				break;
			case 'h':
				help(argv[0]);
				return EXIT_SUCCESS;
		}
	}

	// Init sensors library
	if (sensors_init(NULL) != 0)
	{
		// Error initializing sensors
		return EXIT_FAILURE;
	}

	// List chips and exit (if that's what we're supposed to do)
	if (list)
	{
		list_chips_and_features(precision, unit);
		sensors_cleanup();
		return EXIT_SUCCESS;
	}

	// Chip and/or feature not given? Print help and exit
	if (chip == NULL || feat == NULL)
	{
		help(argv[0]);
		sensors_cleanup();
		return EXIT_SUCCESS;
	}

	// Find the chip that matches the provided chip name
	sensors_chip_name const *cm = find_chip(chip);

	// Abort if we didn't find any matching chips
	if (cm == NULL)
	{
		sensors_cleanup();
		return EXIT_FAILURE;
	}
	
	if (verbose)
	{
		fprintf(stderr, "Chip: %s from %s\n", cm->prefix, cm->path);
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
		if (feat && strstr(fl, feat) != NULL)
		{
			// If so, we get the temperature and add it
			double curr = 0.0;
			int ret = get_temp_value(cm, fc, &curr);

			if (get_temp_value(cm, fc, &curr) != 0)
			{
				continue;
			}

			temp += curr;
			++count;
				
			if (verbose)
			{
				fprintf(stderr, " '- Feature: %s (%.*f%s)\n", 
							fl, precision, curr, 
							unit ? " °C" : "");
			}
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
	print_temp(temp / count, precision, unit, space);

	// Cleanup
	sensors_cleanup();
	return EXIT_SUCCESS;
}
