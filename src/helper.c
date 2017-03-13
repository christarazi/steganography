#include "../include/helper.h"

void print_usage(char const *n)
{
	fprintf(stderr,
		"Usage: %s [-h] [-m <METHOD>] [-d -c <COUNT> | -e <MSG>] <BMP>\n"
		"\n"
		"Options:\n"
		" -h               Print this help.\n"
		" -m <METHOD>      Method to use for steganography.\n"
		"                  <METHOD> can be 'lsb' or 'simple'.\n"
		"                  'lsb' is least significant bit (beter at hiding).\n"
		"                  'simple' just replaces the pixels with <MSG>.\n"
		" -d               Decode and print message found in <BMP>.\n"
		" -c <COUNT>       Number of bytes to print in <BMP> when decoding "
		"(-d).\n"
		" -e <MSG>         Encode message in <BMP>.\n"
		, n);
}

void clean_exit(FILE *fp, struct RGB *rgbs, int const code)
{
	if (fp)
		fclose(fp);
	if (rgbs)
		free(rgbs);
	exit(code);
}

void parse_args(int const argc, char * const *argv, struct Args * const args)
{
	char *cflagstr;  /* Used for cflag error checking */
	char *cflagendr; /* Used for cflag error checking */
	char gtp;

	while ((gtp = getopt(argc, argv, "hm:dc:e:")) != -1) {
		switch (gtp) {
		case 'h':
			print_usage(argv[0]);
			clean_exit(NULL, NULL, EXIT_SUCCESS);
		case 'm':
			args->mflag = true;
			args->mmet = optarg;
			if ((strncmp(args->mmet, "lsb", 3) != 0) &&
			    (strncmp(args->mmet, "simple", 6) != 0)) {
				fprintf(stderr,
					"Option -%c only accepts '%s' or '%s'\n",
					'm', "lsb", "simple");
				clean_exit(NULL, NULL, EXIT_FAILURE);
			}
			break;
		case 'd':
			args->dflag = true;
			break;
		case 'c':
			args->cflag = true;
			cflagstr = optarg;
			args->ccount = strtoull(cflagstr, &cflagendr, 10);
			if ((errno == ERANGE &&
			     (args->ccount == ULLONG_MAX ||
			      args->ccount == 0)) || (errno != 0 &&
						      args->ccount == 0)) {
				perror("strtoull");
				clean_exit(NULL, NULL, EXIT_FAILURE);
			}
			if (cflagendr == cflagstr) {
				fprintf(stderr,
					"Option -%c requires a number\n", 'c');
				clean_exit(NULL, NULL, EXIT_FAILURE);
			}
			break;
		case 'e':
			args->eflag = true;
			args->emsg = optarg;
			args->emsglen = strlen(args->emsg);
			break;
		case '?':
			if (optopt == 'm' || optopt == 'e' || optopt == 'c')
				fprintf(stderr,
					"Option -%c requires an argument\n",
					optopt);
			else if (isprint(optopt))
				fprintf(stderr, "Unknown option '-%c'\n",
					optopt);
			else
				fprintf(stderr,
					"Unknown option character '\\x%x'\n",
					optopt);
			clean_exit(NULL, NULL, EXIT_FAILURE);
		default:
			print_usage(argv[0]);
			clean_exit(NULL, NULL, EXIT_FAILURE);
		}
	}

	if (argc < 6 || (args->dflag && argc != 7)) {
		print_usage(argv[0]);
		clean_exit(NULL, NULL, EXIT_FAILURE);
	}

	/* |optind| holds the index of the first non-option argument in |argv| */
	args->bmpfname = argv[optind];
	/* printf("[DEBUG] optind: %d\n", optind); */
	/* printf("[DEBUG] fname: %s\n", args->bmpfname); */

	if (!(args->mflag)) {
		fprintf(stderr, "Error: option -%c is required", 'm');
		clean_exit(NULL, NULL, EXIT_FAILURE);
	}

	if (!(args->dflag || args->eflag)) {
		fprintf(stderr,
			"Error: one of the two options (-%c, -%c) are "
			"required \n", 'd', 'e');
		clean_exit(NULL, NULL, EXIT_FAILURE);
	}

	if (args->dflag && args->eflag) {
		fprintf(stderr,
			"Error: only one of the two options (-%c, -%c) are "
			"allowed at the same time\n", 'd', 'e');
		clean_exit(NULL, NULL, EXIT_FAILURE);
	}

	if (args->eflag && args->emsglen == 0) {
		fprintf(stderr, "Error: message is empty\n");
		clean_exit(NULL, NULL, EXIT_FAILURE);
	}

	if (args->cflag && !(args->dflag)) {
		fprintf(stderr,
			"Error: only option -%c allowed with -%c\n", 'd', 'c');
		clean_exit(NULL, NULL, EXIT_FAILURE);
	}
}
