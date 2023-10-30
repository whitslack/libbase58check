# libbase58check
**Library for Base58Check encoding and decoding**

This simple C library has two primary functions, **`base58check_encode`** and **`base58check_decode`**. Please refer to the auto-generated [Doxygen documentation](https://whitslack.github.io/libbase58check/base58check_8h.html) for usage details.

There are also overloaded C++ convenience functions if you are writing in C++.

### Usage examples

Here's an example of generating a 256-bit private key and encoding it as Base58Check in Bitcoin's Wallet Import Format (WIF):
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
	std::ranges::copy(std::as_bytes(std::span(key)), bytes.begin() + 1);
	bytes[33] = std::byte { 0x01 };

	// Base58Check encode the wrapped key and print it out
	std::cout << base58check::encode(bytes) << std::endl;
	return 0;
}
```

And here's an example of decoding a Base58Check encoding from standard input:
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

### Building

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
