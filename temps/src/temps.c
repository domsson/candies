#include <stdio.h>            // fprintf
#include <stdlib.h>           // NULL, EXIT_*
#include <unistd.h>           // getopt() et al.
#include <string.h>           // strstr()
#include <float.h>            // DBL_MAX
#include <sensors/sensors.h>

// This will not work on a Raspberry Pi, but if we find a way to check
// if we are on a Pi, we could go another route for it:
// https://www.raspberrypi.org/forums/viewtopic.php?t=208548

sensors_chip_name const *find_chip(const char *prefix)
{
	// Iterate over the chips
	sensors_chip_name const *cc = NULL; // current chip
	sensors_chip_name const *cm = NULL; // matched chip
	int c = 0;

	while(cc = sensors_get_detected_chips(NULL, &c))
	{
		// Remember the first chip we find
		if (cm == NULL)
		{
			cm = cc;
		}

		// Check if this is the chip the user is interested in
		if (prefix && strstr(cc->prefix, prefix) != NULL)
		{
			cm = cc;
			break;
		}
	}
	return cm;
}

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

void list_chips()
{
	// Iterate over the chips
	sensors_chip_name const *cc = NULL; // current chip
	int c = 0;

	while(cc = sensors_get_detected_chips(NULL, &c))
	{
		fprintf(stdout, "%s\n", cc->prefix);
	}
}

void help(char *invocation)
{
	fprintf(stderr, "Usage:   %s [-v] [-c <chip_prefix>] [-f <feature_label>]\n", invocation);
	fprintf(stderr, "Example: %s -c coretemp -f Package\n", invocation);
}

// Sources:
// - manpage for libsensors
// - User Mat on Stack overflow: https://stackoverflow.com/a/8565176
int main(int argc, char **argv)
{
	int list = 0;
	char *chip = NULL;
	char *feat = NULL;
	int verbatim = 0;

	// Get arguments, if any
	opterr = 0;
	int o;
	while ((o = getopt (argc, argv, "c:f:vlh")) != -1)
	{
		switch (o)
		{
			case 'c':
				chip = optarg;
				break;
			case 'f':
				feat = optarg;
				break;
			case 'v':
				verbatim = 1;
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
		list_chips();
		return EXIT_SUCCESS;
	}

	// Find the chip 'chip' or just the first one we encounter
	sensors_chip_name const *cm = find_chip(chip);
	
	if (verbatim)
	{
		fprintf(stderr, "Chip source: %s from %s\n", cm->prefix, cm->path);
	}

	// Abort if we didn't find any chips
	if (cm == NULL)
	{
		return EXIT_FAILURE;
	}
	
	// https://www.kernel.org/doc/Documentation/hwmon/coretemp
	// For Intel, Feature labels will be either "Core 0", "Core 1" etc
	// or either "Package id n" (n = some number) or "Physical id n"
	
	// According to some SO post, "package" temp is the temperature of the
	// CPU socket itself, which is usually higher than that of the cores 
	// because the socket, in contrast to the CPU, is not directly cooled.
	// Hence, the average core temperatures are probably more relevant 
	// than the package temperature.

	// If we find the user-specified feature, we use that. If not, we sum
	// up all the Core temps and return the average. If we can't find any,
	// we use the package temperature, if we managed to find that. In case
	// we can't find any of the above, we use the temperature reading of 
	// the first feature we came across. If we coulnd't find any of the 
	// above, we will finally give up and EXIT_FAILURE without printing.
	
	// Iterate over the features
	sensors_feature const *fc = NULL; // current feature
	int f = 0;

	double feature_temp = DBL_MAX; // temperature of user-specified feature
	double package_temp = DBL_MAX; // temperature of package, if found
	double core_temps   = 0.0;     // avg. temp of all cores, if any found
	size_t num_cores    = 0;       // number of core temps found

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
		
		// Check if this is the feature the user is interested in
		if (feat && strstr(fl, feat) != NULL)
		{
			get_temp_value(cm, fc, &feature_temp);
		}

		// Check if this is the package temperature (intel)
		if (strstr(fl, "Package") != NULL || strstr(fl, "Physical") != NULL)
		{
			get_temp_value(cm, fc, &package_temp);
		}

		// Check if this is a core temperature (intel)
		if (strstr(fl, "Core") != NULL)
		{
			double ct = 0.0;
			get_temp_value(cm, fc, &ct);
			core_temps += ct;
			++num_cores;
		}

		free(fl);
		fl = NULL;
	}

	// We found the requested temperature, print that!
	if (feature_temp != DBL_MAX)
	{
		if (verbatim)
		{
			fprintf(stderr, "Temp source: specified feature\n");
		}
		fprintf(stdout, "%.0f\n", feature_temp);
	}

	// We were able to gather core temps, print their average!
	else if (num_cores > 0)
	{
		if (verbatim)
		{
			fprintf(stderr, "Temp source: core average\n");
		}
		fprintf(stdout, "%.0f\n", core_temps / num_cores);
	}

	// We were able to find a package temperature, print that!
	else if (package_temp != DBL_MAX)
	{
		if (verbatim)
		{
			fprintf(stderr, "Temp source: package\n");
		}
		fprintf(stdout, "%.0f\n", package_temp);
	}

	// Cleanup
	sensors_cleanup();
	return EXIT_SUCCESS;
}
