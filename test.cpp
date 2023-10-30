#include "base58check.h"

#include <cassert>
#include <initializer_list>


template <typename T, size_t N>
static void test_encode(const T (&data)[N], const char expect[]) {
	auto actual = base58check::encode(reinterpret_cast<const base58check::byte *>(data), N, 0);
	assert(actual == expect);
}

static void test_round_trip(const char str[], size_t decoded_size) {
	auto bytes = base58check::decode(str, std::strlen(str), 0);
	assert(bytes.size() == decoded_size);
	auto out = base58check::encode(bytes.data(), bytes.size(), 0);
	assert(out == str);
}

static void test_hash_mismatch(const char str[]) {
	try {
		base58check::decode(str, 0);
	}
	catch (const std::invalid_argument &) {
		return;
	}
	throw std::logic_error("should have thrown");
}

int main() {
	test_encode("Hello world!", "gTazoqFi2U9CKLR6yjgYY8h");

	test_round_trip("3QJmnh", 0);
	test_round_trip("1111111111111111111114oLvT2", 21);
	test_round_trip("1BitcoinEaterAddressDontSendf59kuE", 21);

	test_hash_mismatch("1BitcoinEaterAddressDontSendf59kuF");

	return 0;
}
