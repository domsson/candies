#include <stdio.h>            // fprintf
#include <stdlib.h>           // NULL, EXIT_*
#include <unistd.h>           // getopt() et al.
#include <string.h>           // strstr()
#include <math.h>             // pow(), fabs()
#include <float.h>            // DBL_MAX
#include <sys/types.h>        // opendir()
#include <dirent.h>           // opendir(), readdir()

#define UNIT_METRIC   "°C" 
#define UNIT_IMPERIAL "°F"

#define DEFAULT_INTERVAL  1
#define DEFAULT_THRESHOLD 1
#define DEFAULT_PATH      "/sys/class/hwmon"
#define DEFAULT_FILE      "hwmon"

struct config 
{
	int list : 1;           // list chips and features
	int monitor : 1;        // monitor mode (keep running)?
	int unit : 1;           // also print the °C unit
	int space : 1;          // space between val and unit
	int imperial: 1;        // use fahrenheit (imperial)
	int precision;          // decimal places in output
	int interval;           // seconds between temp checks
	double threshold;       // (not yet in use)
	char *path;             // path with hwmon entires
	char *file;             // beginning of relevant file names in path
	char *unit_str;         // unit string to use
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
 */

/**
 * Tries to read the actual temperature value from the specified feature
 * of the specified chip. Writes the temperature vlue to ret. Returns 0 on
 * success, -1 on error. 
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
 */

/**
 * List all temperature features of the provided chip, including their readings.
 * Temperature readings will be formatted according to precision and unit.
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
 */

/**
 * Prints all detected chips and their features to stdout.
 */
int list_chips_and_features(struct config *cfg)
{
	DIR *hwmon = opendir(cfg->path);
	
	if (hwmon == NULL)
	{
		return -1;
	}

	struct dirent *direntry = NULL;
	size_t c = 0;
	
	while ((direntry = readdir(hwmon)))
	{
		// TODO free() and fclose()/close() everything
		//      and make sure we also do it if we return -1

		// Skipping everything that doesn't start with "hwmon"
		// or whatever else is in the config's 'file' field 
		if (strncmp(direntry->d_name, cfg->file, strlen(cfg->file)) != 0)
		{
			continue;
		}
	
		size_t chip_path_len = strlen(cfg->path) + 1 + strlen(direntry->d_name) + 1;
		char *chip_path = malloc(sizeof(char) * chip_path_len);
		snprintf(chip_path, chip_path_len, "%s/%s", cfg->path, direntry->d_name);
	
		DIR *chip = opendir(chip_path);
		if (chip == NULL)
		{
			free(chip_path);
			return -1;
		}

		size_t name_path_len = strlen(chip_path) + 1 + strlen("name") + 1;
		char *name_path = malloc(sizeof(char) * name_path_len);
		snprintf(name_path, name_path_len, "%s/%s", chip_path, "name");
		free(chip_path);
		
		FILE *name_file = fopen(name_path, "r");
		free(name_path);
		if (name_file == NULL)
		{
			closedir(chip);
			return -1;
		}

		// Using fscanf() vs fgets() or getline() helps to skip all the 
		// potential whitespace at the end of a line in these files
		char *chip_name = NULL; 
		if (fscanf(name_file, "%ms", &chip_name) != 1)
		{
			fclose(name_file);
			free(chip_name);
			closedir(chip);
			return -1;
		}

		// Finally print the chip name
		fprintf(stdout, "[C%ld] %s (from %s)\n", c, chip_name, cfg->path);
		++c;

		fclose(name_file);
		free(chip_name);

		// TODO list features for this chip

		
		closedir(chip);
	}

	closedir(hwmon);
	return 0;
}

/*
double determine_temp(const char *chip, const char *feat, double *t, size_t *n)
{
	// Iterate over the features
	sensors_feature const *fc = NULL; // current feature
	int f = 0;

	*t = 0.0;
	*n = 0;

	double curr = 0.0;

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
			curr = 0.0;
			if (get_temp_value(cm, fc, &curr) != 0)
			{
				continue;
			}

			*t += curr;
			++(*n);
		}

		free(fl);
		fl = NULL;
	}

	return *t / *n;
}
*/


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
     	fprintf(stderr, "\t%s [OPTIONS...] -c CHIP -f FEATURE\n", invocation);
	fprintf(stderr, "\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "\t-e Use Fahrenheit instead of Celcius.\n");
	fprintf(stderr, "\t-h Print this help text and exit.\n");
	fprintf(stderr, "\t-i Seconds between checking for a change in value; default is 1.\n");
	fprintf(stderr, "\t-l List all chips and features, then exit.\n");
	fprintf(stderr, "\t-m Keep running and print when there is a notable change in value.\n");
	fprintf(stderr, "\t-p Number of decimal digits in the output; default is 0.\n");
	fprintf(stderr, "\t-s Print a space between value and unit.\n");
	fprintf(stderr, "\t-t Required change in value in order to print again; default is 1.\n");
	fprintf(stderr, "\t-u Include the temperature unit in the output.\n");
}

int main(int argc, char **argv)
{
	struct config cfg = { 0 };
	cfg.threshold = -1.0;

	// Get arguments, if any
	opterr = 0;
	int o;
	while ((o = getopt(argc, argv, "c:ef:hi:lmp:st:u")) != -1)
	{
		switch (o)
		{
			case 'c':
				cfg.chip = optarg;
				break;
			case 'e':
				cfg.imperial = 1;
				break;
			case 'f':
				cfg.feat = optarg;
				break;
			case 'h':
				help(argv[0]);
				return EXIT_SUCCESS;
			case 'i':
				cfg.interval = atoi(optarg);
				break;
			case 'l':
				cfg.list = 1;
				break;
			case 'm':
				cfg.monitor = 1;
				break;
			case 'p':
				cfg.precision = atoi(optarg);
				break;
			case 's':
				cfg.space = 1;
				break;
			case 't':
				cfg.threshold = atof(optarg);
				break;
			case 'u':
				cfg.unit = 1;
				break;
		}
	}
	
	// Prepare strings we'll need multiple times 
	cfg.unit_str = cfg.unit ? (cfg.imperial ? UNIT_IMPERIAL : UNIT_METRIC) : "";
	cfg.path = DEFAULT_PATH; // TODO
	cfg.file = DEFAULT_FILE; // TODO

	// List chips and exit (if that's what we're supposed to do)
	if (cfg.list)
	{
		list_chips_and_features(&cfg);
		return EXIT_SUCCESS;
	}

	// Chip and/or feature not given? Print help and exit
	if (cfg.chip == NULL || cfg.feat == NULL)
	{
		help(argv[0]);
		return EXIT_SUCCESS;
	}
	
	// Find the chip that matches the provided chip name
	//const char *chip = find_chip(cfg.chip);
	const char *chip = NULL; // TODO

	// Abort if we didn't find any matching chips
	if (chip == NULL)
	{
		return EXIT_FAILURE;
	}

	// If no threshold given, determine it based on the precision
	if (cfg.threshold == -1)
	{
		cfg.threshold = DEFAULT_THRESHOLD / pow(10.0, (double) cfg.precision);
	}

	// If no interval given, use the default
	if (cfg.interval == 0)
	{
		// Set interval to 0 if we don't monitor
		cfg.interval = DEFAULT_INTERVAL;
	}

	// Reset interval to 0 if we don't monitor anyway
	if (cfg.monitor == 0)
	{
		cfg.interval = 0;
	}
	
	// Make sure stdout is line buffered
	setlinebuf(stdout);

	// Loop vars
	size_t temp_num = 0;
	double temp_avg_prev  = -DBL_MAX;
	double temp_avg_curr  =  0.0;
	double temp_avg_delta =  0.0;

	do
	{
		// Get the current temperature (average of all found values)
		//temp_avg_curr = determine_temp(cfg.chip, cfg.feat, &temp_sum, &temp_num);
		temp_avg_curr = 0; // TODO
		
		// We didn't manage to find a temp value
		if (temp_num <= 0)
		{
			// k thx bye :(
			return EXIT_FAILURE;
		}

		// Convert to Fahrenheit, if the user asked us to
		if (cfg.imperial)
		{
			temp_avg_curr = to_fahrenheit(temp_avg_curr);
		}

		// Figure out the difference to the previously printed value
		temp_avg_delta = fabs(temp_avg_curr - temp_avg_prev);

		// Check if the difference is significant enough
		if (temp_avg_delta >= cfg.threshold)
		{
			// Print (and possibly convert to Fahrenheit first) 
			print_temp(temp_avg_curr, cfg.precision, cfg.unit_str, cfg.space);

			// Update values
			temp_avg_prev = temp_avg_curr;
		}

		// Sleep, maybe (if interval > 0)
		sleep(cfg.interval);
	}
	while (cfg.monitor);

	return EXIT_SUCCESS;
}
