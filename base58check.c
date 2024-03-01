#define _GNU_SOURCE

#include "base58check.h"

#include <err.h>
#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <sysexits.h>


static void print_usage() {
	fprintf(stderr, "usage: %s [-d] [-h]\n\n"
		"Reads data from stdin, encodes it in Base58Check, and writes the encoding to\n"
		"stdout. Specify -d to decode instead. Specify -h to use hex data input/output.\n",
		program_invocation_short_name);
}

static int gethex() {
	static const int8_t DECODE['f' + 1 - '0'] = {
		 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,
		-1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, 10, 11, 12, 13, 14, 15
	};
	int hi, lo;
	if ((hi = getchar()) < 0)
		return hi;
	if ((hi -= '0') >= 0 && hi <= 'f' - '0' && (hi = DECODE[hi]) >= 0) {
		if ((lo = getchar()) >= 0 && (lo -= '0') >= 0 && lo <= 'f' - '0' && (lo = DECODE[lo]) >= 0)
			return hi << 4 | lo;
	}
	else if (hi == '\n' - '0')
		return EOF;
	errx(EX_DATAERR, "invalid hex on stdin");
}

static ssize_t fwrite_hex(const unsigned char in[], size_t n_in, FILE *out) {
	static const char ENCODE[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
	};
	for (size_t i = 0; i < n_in; ++i) {
		char x[2] = { ENCODE[in[i] >> 4], ENCODE[in[i] & 0xF] };
		if (fwrite(x, 1, 2, out) < 2)
			return -1;
	}
	return putc('\n', out) < 0 ? -1 : (ssize_t) n_in;
}

int main(int argc, char *argv[]) {
	static const struct option longopts[] = {
		{ .name = "decode", .has_arg = no_argument, .val = 'd' },
		{ .name = "hex", .has_arg = no_argument, .val = 'h' },
		{ .name = "help", .has_arg = no_argument, .val = 1 },
		{ .name = "version", .has_arg = no_argument, .val = 2 },
		{ }
	};
	bool decode = false, hex = false;
	for (int opt; (opt = getopt_long(argc, argv, "dh", longopts, NULL)) >= 0;) {
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
			case 'h':
				hex = true;
				break;
			default:
				print_usage();
				return EX_USAGE;
		}
	}
	if (optind != argc)
		return print_usage(), EX_USAGE;

	unsigned char in[4096];
	size_t n_in = 0;
	if (decode || hex) {
		for (int c;;) {
			if (decode ? (c = getchar()) < 0 || c == '\n' : (c = gethex()) < 0) {
				if (ferror(stdin))
					err(EX_IOERR, "error reading from stdin");
				break;
			}
			if (n_in == sizeof in)
				errx(EX_DATAERR, "input is too long");
			in[n_in++] = (unsigned char) c;
		}
	}
	else {
		n_in = fread(in, 1, sizeof in, stdin);
		if (ferror(stdin))
			err(EX_IOERR, "error reading from stdin");
		if (!feof(stdin) && getchar() >= 0)
			errx(EX_DATAERR, "input is too long");
	}

	unsigned char *out = NULL;
	size_t n_out = 0;
	if (decode) {
		if (base58check_decode(&out, &n_out, (char *) in, n_in, 0))
			errx(EX_DATAERR, "input was not a valid Base58Check encoding");
	}
	else {
		n_out = 1;
		if (base58check_encode((char **) &out, &n_out, in, n_in, 0))
			errx(EX_SOFTWARE, "internal error");
		out[n_out++] = '\n';
	}

	if (hex && decode ?
			fwrite_hex((const unsigned char *) out, n_out, stdout) < (ssize_t) n_out :
			fwrite(out, 1, n_out, stdout) < n_out)
		err(EX_IOERR, "error writing to stdout");

	return EX_OK;
}
