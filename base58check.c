#define _GNU_SOURCE

#include "base58check.h"

#include <err.h>
#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <sysexits.h>
#include <unistd.h>


static void print_usage() {
	fprintf(stderr, "usage: %s [-d]\n\n"
		"Reads data from stdin, encodes it in Base58Check, and writes the encoding to\n"
		"stdout. Specify -d to decode instead.\n",
		program_invocation_short_name);
}

int main(int argc, char *argv[]) {
	static const struct option longopts[] = {
		{ .name = "decode", .has_arg = no_argument, .val = 'd' },
		{ .name = "help", .has_arg = no_argument, .val = 1 },
		{ .name = "version", .has_arg = no_argument, .val = 2 },
		{ }
	};
	bool decode = false;
	for (int opt; (opt = getopt_long(argc, argv, "d", longopts, NULL)) >= 0;) {
		switch (opt) {
			case 1:
				print_usage();
				return EX_OK;
			case 2:
				printf("base58check %s\n", VERSION);
				return EX_OK;
			case 'd':
				decode = true;
				break;
			default:
				print_usage();
				return EX_USAGE;
		}
	}

	unsigned char in[4096];
	size_t n_in = 0;

	for (size_t r; n_in < sizeof in && (r = fread(in + n_in, 1, sizeof in - n_in, stdin)) > 0; n_in += r);
	if (ferror(stdin) || n_in == sizeof in)
		return EX_IOERR;

	unsigned char *out = NULL;
	size_t n_out = 0;
	if (decode) {
		if (n_in && in[n_in - 1] == '\n')
			in[--n_in] = '\0';
		if (base58check_decode(&out, &n_out, (char *) in, n_in, 0))
			errx(EX_DATAERR, "input was not a valid Base58Check encoding");
	}
	else {
		n_out = 1;
		if (base58check_encode((char **) &out, &n_out, in, n_in, 0))
			return EX_UNAVAILABLE;
		out[n_out++] = '\n';
	}

	if (fwrite(out, 1, n_out, stdout) < n_out)
		return EX_IOERR;

	return EX_OK;
}
