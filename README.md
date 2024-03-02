# libbase58check
**Library for Base58Check encoding and decoding**

This simple C library has two primary functions, **`base58check_encode`** and **`base58check_decode`**. Please refer to the auto-generated [Doxygen documentation](https://whitslack.github.io/libbase58check/base58check_8h.html) for usage details.

The library comes with a command-line utility for Base58Check encoding/decoding from `stdin` to `stdout`.

There are also overloaded C++ convenience functions if you are writing in C++.

## Usage examples

### Encoding
Here's an example of generating a 256-bit private key and encoding it as Base58Check in Bitcoin's Wallet Import Format (WIF).

#### Shell
```bash
$ { printf '\x80' ; head -c32 /dev/urandom ; printf '\x01' ; } | base58check
KxmBZwPrX3u2dukBawC9u88BKAXuRRHmPwuLzLo8skAgBQoGnC6c
```

#### C++
```cpp
#include <base58check.h>

#include <algorithm>
#include <array>
#include <iostream>
#include <random>
#include <ranges>

int main() {
	// fill a 32-byte key from std::random_device
	using random_t = std::random_device::result_type;
	std::array<random_t, 32 / sizeof(random_t)> key;
	std::random_device random;
	std::ranges::generate(key, std::reference_wrapper(random));

	// wrap the key between a 0x80 type byte and a 0x01 flag byte
	std::array<std::byte, 34> bytes;
	bytes[0] = std::byte { 0x80 };
	std::ranges::copy(as_bytes(std::span(key)), bytes.begin() + 1);
	bytes[33] = std::byte { 0x01 };

	// Base58Check encode the wrapped key and print it out
	std::cout << base58check::encode(bytes) << std::endl;
	return 0;
}
```

#### C
```c
#include <base58check.h>

#include <err.h>
#include <stdint.h>
#include <stdio.h>
#include <sysexits.h>
#include <sys/random.h>

int main() {
	struct {
		uint8_t type;
		uint8_t key[32];
		uint8_t flag;
	} buf;
	int ret = 0;

	// fill a 32-byte key using getrandom()
	if (getrandom(buf.key, sizeof buf.key, 0) < sizeof buf.key)
		err(EX_OSERR, "getrandom");

	// wrap the key between a 0x80 type byte and a 0x01 flag byte
	buf.type = 0x80, buf.flag = 0x01;

	// Base58Check encode the wrapped key
	char *out = NULL;
	size_t n_out = 1; // 1 excess char for null terminator
	if (base58check_encode(&out, &n_out, (unsigned char *) &buf, sizeof buf, 0) < 0)
		errx(EX_UNAVAILABLE, "base58check_encode failed");
	out[n_out] = '\0';

	// print it out
	if (puts(out) < 0 || fflush(stdout) < 0)
		err(EX_IOERR, "stdout");

	// be tidy
	base58check_free(out);
	return EX_OK;
}
```

### Decoding
And here's an example of decoding a Base58Check encoding from standard input.

#### Shell
```bash
$ echo 1BitcoinEaterAddressDontSendf59kuE | base58check -d | hexdump -C
00000000  00 75 9d 66 77 09 1e 97  3b 9e 9d 99 f1 9c 68 fb  |.u.fw...;.....h.|
00000010  f4 3e 3f 05 f9                                    |.>?..|
00000015
```

#### C++
```cpp
#include <base58check.h>

#include <iostream>
#include <sstream>

int main() {
	// read all input from cin to EOF
	std::ostringstream ss;
	ss << std::cin.rdbuf();

	// chop off any trailing newline character
	auto str = std::move(ss).str();
	if (str.ends_with('\n'))
		str.pop_back();

	// decode the Base58Check and output the raw bytes
	auto bytes = base58check::decode(str);
	std::cout.write(reinterpret_cast<const char *>(bytes.data()),
			bytes.size()) << std::flush;
	return 0;
}
```

#### C
```c
#include <base58check.h>

#include <err.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>

int main() {
	// read a line from stdin
	char *in = NULL;
	ssize_t n_in = 0;
	if ((n_in = getline(&in, (size_t *) &n_in, stdin)) < 0 && ferror(stdin))
		err(EX_IOERR, "stdin");

	// chop off any trailing newline character
	if (n_in && in[n_in - 1] == '\n')
		in[--n_in] = '\0';

	// decode the Base58Check
	unsigned char *out = NULL;
	size_t n_out = 0;
	if (base58check_decode(&out, &n_out, in, n_in, 0) < 0)
		errx(EX_DATAERR, "base58check_decode failed");

	// output the raw bytes
	if (fwrite(out, 1, n_out, stdout) < n_out || fflush(stdout) < 0)
		err(EX_IOERR, "stdout");

	// be tidy
	base58check_free(out);
	free(in);
	return EX_OK;
}
```

## Building

1. Install the prerequisites — most/all of which you probably already have:

	* [Autoconf](https://www.gnu.org/software/autoconf/)
	* [Autoconf Archive](https://www.gnu.org/software/autoconf-archive/)
	* [Automake](https://www.gnu.org/software/automake/)
	* [GMP](https://gmplib.org/)
	* [Libtool](https://www.gnu.org/software/libtool/)
	* [OpenSSL](https://www.openssl.org/)
	* [pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config/)

1. Then it's just your standard Autotools madness:

	```
	$ autoreconf -i
	$ ./configure
	$ make
	$ sudo make install
	```
