#include "../include/args.h"

void print_usage(char const *n)
{
	fprintf(stderr,
		"Usage: %s [-h] [-m <METHOD>] [-t <TYPE>] [-d | -e <VAL>] <BMP>\n\n"
		"Options:\n"
		" -h               Print this help.\n\n"
		" -m <METHOD>      Method to use for steganography.\n"
		"                  <METHOD> can be 'lsb' or 'simple'.\n"
		"                  'lsb' is least significant bit (beter at hiding).\n"
		"                  'simple' just replaces the pixels outright.\n\n"
		" -t <TYPE>        Type of steganography to perform.\n"
		"                  <TYPE> can be 'message' or 'file'.\n"
		"                  'message' is for hiding messages.\n"
		"                  'file' is for hiding files (or images) within <BMP>.\n\n"
		" -d               Decode [message | file] found in <BMP>.\n\n"
		" -e <VAL>         <VAL> can be a message or a file name.\n"
		"                  When <TYPE> is 'message', <VAL> is encoded in <BMP>.\n"
		"                  When <TYPE> is 'file', <VAL> is the file to hide in <BMP>.\n"
		, n);
}

void parse_args(int const argc, char * const *argv, struct Args * const args)
{
	char gtp;

	while ((gtp = getopt(argc, argv, "hm:t:de:")) != -1) {
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
		case 't':
			args->tflag = true;
			args->ttyp = optarg;
			if ((strncmp(args->ttyp, "message", 7) != 0) &&
			    (strncmp(args->ttyp, "file", 4) != 0)) {
				fprintf(stderr,
					"Option -%c only accepts '%s' or '%s'\n",
					't', "message", "file");
				clean_exit(NULL, NULL, EXIT_FAILURE);
			}
			break;
		case 'd':
			args->dflag = true;
			break;
		case 'e':
			args->eflag = true;
			args->eval = optarg;
			args->evallen = strlen(args->eval);
			break;
		case '?':
			if (optopt == 'm' || optopt == 'e')
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

	if ((args->dflag && argc != 7) || (args->eflag && argc != 8)) {
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

	if (!(args->tflag)) {
		fprintf(stderr, "Error: option -%c is required", 't');
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

	if (args->eflag && args->evallen == 0) {
		fprintf(stderr, "Error: value to option -%c is empty\n", 'e');
		clean_exit(NULL, NULL, EXIT_FAILURE);
	}

	if (args->eflag && args->evallen > SUPPORTED_MAX_MSG_LEN) {
		fprintf(stderr, "Error: max length for option -%c is %d\n",
			'e', SUPPORTED_MAX_MSG_LEN);
		clean_exit(NULL, NULL, EXIT_FAILURE);
	}
}

