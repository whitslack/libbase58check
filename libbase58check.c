#include "base58check.h"

#include <assert.h>
#include <gmp.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>

#if GMP_LIMB_BITS < 32 || GMP_LIMB_BITS > 64
# error "unsupported limb size"
#endif

#define MP_NLIMBS(n) (((n) + sizeof(mp_limb_t) - 1) / sizeof(mp_limb_t))

#define _likely(...) __builtin_expect(!!(__VA_ARGS__), 1)
#define _unlikely(...) __builtin_expect(!!(__VA_ARGS__), 0)


static inline size_t encoded_size_upper_bound(size_t n) {
	// 1430893/1047768 approximates log(256)/log(58) with error +9.950928969715278e-12
#if SIZE_MAX >= UINT64_MAX
	if (_unlikely(n > /* ((1 << 64) - (1047768 - 1)) / 1430893 = */UINT64_C(12891770435461)))
#elif SIZE_MAX >= UINT32_MAX
	if (_unlikely(n > /* ((1 << 32) - (1047768 - 1)) / 1430893 = */3000))
		// 9886/7239 approximates log(256)/log(58) with error +2.7786490885972626e-9
		if (_likely(n <= /* ((1 << 32) - (7239 - 1)) / 9886 = */434448))
			return (n * 9886 + (7239 - 1)) / 7239;
		// 41/30 approximates log(256)/log(58) with error +1.0084293569057046e-3
		else if (_likely(n <= /* ((1 << 32) - (30 - 1)) / 41 = */104755299))
			return (n * 41 + (30 - 1)) / 30;
		// 2/1 approximates log(256)/log(58) with error +6.34341762690239e-1
		else if (_likely(n <= UINT32_C(2147483648)))
			return n * 2;
		else
#endif
		return SIZE_MAX;
	return (n * 1430893 + (1047768 - 1)) / 1047768;
}

static inline size_t decoded_size_upper_bound(size_t n) {
	// 49549/67667 approximates log(58)/log(256) with error +4.992450897134404e-12
#if SIZE_MAX >= UINT64_MAX
	if (_unlikely(n > /* ((1 << 64) - (67667 - 1)) / 49549 = */UINT64_C(372292964009555)))
#elif SIZE_MAX >= UINT32_MAX
	if (_unlikely(n > /* ((1 << 32) - (67667 - 1)) / 49549 = */86679))
		// 361/493 approximates log(58)/log(256) with error +3.896907227907143e-6
		if (_likely(n <= /* ((1 << 32) - (493 - 1)) / 361 = */11897414))
			return (n * 361 + (493 - 1)) / 493;
		else
#endif
		return n;
	return (n * 49549 + (67667 - 1)) / 67667;
}

static void bytes_to_limbs(mp_limb_t *restrict limbs, const uint8_t *restrict bytes, size_t n) {
	if (n == 0) {
		return;
	}
	while (n > sizeof(mp_limb_t)) {
		mp_limb_t limb = 0;
#if GMP_LIMB_BITS >= 64
		limb |= (mp_limb_t) (bytes[n - 8]) << 56;
		limb |= (mp_limb_t) (bytes[n - 7]) << 48;
		limb |= (mp_limb_t) (bytes[n - 6]) << 40;
		limb |= (mp_limb_t) (bytes[n - 5]) << 32;
#endif
#if GMP_LIMB_BITS >= 32
		limb |= (mp_limb_t) (bytes[n - 4]) << 24;
		limb |= (mp_limb_t) (bytes[n - 3]) << 16;
#endif
		limb |= (mp_limb_t) (bytes[n - 2]) << 8;
		limb |= (mp_limb_t) (bytes[n - 1]);
		*limbs++ = limb;
		n -= sizeof(mp_limb_t);
	}
	mp_limb_t limb = 0;
	switch (n) {
#if GMP_LIMB_BITS >= 64
		case 8:
			limb |= (mp_limb_t) (bytes[n - 8]) << 56;
		case 7:
			limb |= (mp_limb_t) (bytes[n - 7]) << 48;
		case 6:
			limb |= (mp_limb_t) (bytes[n - 6]) << 40;
		case 5:
			limb |= (mp_limb_t) (bytes[n - 5]) << 32;
#endif
#if GMP_LIMB_BITS >= 32
		case 4:
			limb |= (mp_limb_t) (bytes[n - 4]) << 24;
		case 3:
			limb |= (mp_limb_t) (bytes[n - 3]) << 16;
#endif
		case 2:
			limb |= (mp_limb_t) (bytes[n - 2]) << 8;
		case 1:
			limb |= (mp_limb_t) (bytes[n - 1]);
	}
	*limbs = limb;
}

static void limbs_to_bytes(uint8_t *restrict bytes, const mp_limb_t *restrict limbs, size_t n) {
	if (n == 0) {
		return;
	}
	while (n > sizeof(mp_limb_t)) {
		mp_limb_t limb = *limbs++;
#if GMP_LIMB_BITS >= 64
		bytes[n - 8] = (uint8_t) (limb >> 56);
		bytes[n - 7] = (uint8_t) (limb >> 48);
		bytes[n - 6] = (uint8_t) (limb >> 40);
		bytes[n - 5] = (uint8_t) (limb >> 32);
#endif
#if GMP_LIMB_BITS >= 32
		bytes[n - 4] = (uint8_t) (limb >> 24);
		bytes[n - 3] = (uint8_t) (limb >> 16);
#endif
		bytes[n - 2] = (uint8_t) (limb >> 8);
		bytes[n - 1] = (uint8_t) (limb);
		n -= sizeof(mp_limb_t);
	}
	mp_limb_t limb = *limbs;
	switch (n) {
#if GMP_LIMB_BITS >= 64
		case 8:
			bytes[n - 8] = (uint8_t) (limb >> 56);
		case 7:
			bytes[n - 7] = (uint8_t) (limb >> 48);
		case 6:
			bytes[n - 6] = (uint8_t) (limb >> 40);
		case 5:
			bytes[n - 5] = (uint8_t) (limb >> 32);
#endif
#if GMP_LIMB_BITS >= 32
		case 4:
			bytes[n - 4] = (uint8_t) (limb >> 24);
		case 3:
			bytes[n - 3] = (uint8_t) (limb >> 16);
#endif
		case 2:
			bytes[n - 2] = (uint8_t) (limb >> 8);
		case 1:
			bytes[n - 1] = (uint8_t) (limb);
	}
}

static size_t encode_limbs(char *restrict out, size_t n_out, mp_limb_t *restrict limbs, mp_size_t n_limbs) {
	static const char encode[58] = {
		'1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G',
		'H', 'J', 'K', 'L', 'M', 'N', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y',
		'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'm', 'n', 'o', 'p',
		'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'
	};
	for (;;) {
		if (!n_limbs)
			return 0;
		if (limbs[n_limbs - 1])
			break;
		--n_limbs;
	}
	char *p = out + n_out;
	while (limbs[n_limbs - 1] || --n_limbs) {
#if GMP_LIMB_BITS == 64
		mp_limb_t limb = mpn_divrem_1(limbs, 0, limbs, n_limbs, UINT64_C(430804206899405824) /* 58**10 */);
#elif GMP_LIMB_BITS == 32
		mp_limb_t limb = mpn_divrem_1(limbs, 0, limbs, n_limbs, 656356768 /* 58**5 */);
#endif
		if (n_limbs == 1 && *limbs == 0) {
			while (limb) {
				*--p = encode[limb % 58], limb /= 58;
			}
			break;
		}
		size_t n_digits = GMP_LIMB_BITS * 10 / 64;
		do {
			*--p = encode[limb % 58], limb /= 58;
		} while (--n_digits);
	}
	size_t n_ret = out + n_out - p;
	assert(n_ret <= n_out);
	if (_unlikely(p != out)) {
		memmove(out, p, n_ret);
	}
	return n_ret;
}

static mp_size_t decode_limbs(mp_limb_t *restrict limbs, const char *in, size_t n_in) {
	static const int8_t decode['z' - '1' + 1] = {
		 0,  1,  2,  3,  4,  5,  6,  7,  8, -1, -1, -1, -1, -1, -1, -1,
		 9, 10, 11, 12, 13, 14, 15, 16, -1, 17, 18, 19, 20, 21, -1, 22,
		23, 24, 25, 26, 27, 28, 29, 30, 31, 32, -1, -1, -1, -1, -1, -1,
		33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, -1, 44, 45, 46, 47,
		48, 49, 50, 51, 52, 53, 54, 55, 56, 57
	};
	for (;;) {
		if (!n_in)
			return 0;
		if (*in != '1')
			break;
		++in, --n_in;
	}
	mp_size_t n_limbs = 1;
	*limbs = 0;
	while (n_in) {
#if GMP_LIMB_BITS == 64
		static const mp_limb_t powers[] = {
			58, 3364, 195112, 11316496, 656356768, UINT64_C(38068692544),
			UINT64_C(2207984167552), UINT64_C(128063081718016),
			UINT64_C(7427658739644928), UINT64_C(430804206899405824)
		};
#elif GMP_LIMB_BITS == 32
		static const mp_limb_t powers[] = {
			58, 3364, 195112, 11316496, 656356768
		};
#endif
		size_t n_digits = sizeof powers / sizeof *powers;
		if (n_digits > n_in)
			n_digits = n_in;
		n_in -= n_digits;
		mp_limb_t carry = mpn_mul_1(limbs, limbs, n_limbs, powers[n_digits - 1]);
		if (carry)
			limbs[n_limbs++] = carry;
		mp_limb_t limb = 0;
		do {
			unsigned digit = (uint8_t) (*in++) - '1';
			if (digit > 'z' - '1' || (signed) (digit = decode[digit]) < 0)
				return -1;
			limb = limb * 58 + digit;
		} while (--n_digits);
		if (mpn_add_1(limbs, limbs, n_limbs, limb))
			limbs[n_limbs++] = 1;
	}
	return n_limbs;
}


size_t base58check_encode_buffer_size(const unsigned char in[], size_t n_in, size_t n_pad) {
	size_t n_leading_zeros = 0;
	while (n_in && *in == 0)
		++n_leading_zeros, ++in, --n_in;
	size_t n_out;
	if (__builtin_uaddl_overflow(n_in, 4, &n_in) ||
			__builtin_uaddl_overflow(encoded_size_upper_bound(n_in), n_leading_zeros, &n_out) ||
			__builtin_uaddl_overflow(n_out, n_pad, &n_out))
		return SIZE_MAX;
	return n_out;
}

size_t base58check_decode_buffer_size(const char in[], size_t n_in, size_t n_pad) {
	size_t n_leading_zeros = 0;
	while (n_in && *in == '1')
		++n_leading_zeros, ++in, --n_in;
	size_t n_out;
	if (__builtin_uaddl_overflow(decoded_size_upper_bound(n_in), n_leading_zeros, &n_out) ||
			__builtin_uaddl_overflow(n_out, n_pad, &n_out))
		return SIZE_MAX;
	return n_out;
}

int base58check_encode(char **restrict out, size_t *n_out, const unsigned char *restrict in, size_t n_in, size_t n_hdr) {
	unsigned char hash[32];
	SHA256(in, n_in, hash);
	SHA256(hash, sizeof hash, hash);

	size_t n_leading_zeros = 0;
	while (n_in && *in == 0)
		++n_leading_zeros, ++in, --n_in;

	// add 4 bytes for the hash fragment
	if (__builtin_uaddl_overflow(n_in, 4, &n_in))
		return -1;

	size_t n_need = encoded_size_upper_bound(n_in);
	if (__builtin_uaddl_overflow(n_need, n_hdr, &n_need) ||
			__builtin_uaddl_overflow(n_need, n_leading_zeros, &n_need))
		return -1;

	char *out_ = *out;
	size_t n_out_ = *n_out;
	if (!out_) {
		if (__builtin_uaddl_overflow(n_need, n_out_, &n_out_) ||
				!(out_ = base58check_malloc(n_out_)))
			return -1;
	}
	else if (n_out_ < n_need)
		return -1;

	memset(out_ + n_hdr, '1', n_leading_zeros);
	out_ += n_hdr += n_leading_zeros, n_out_ -= n_hdr;

	// use out_ as a temporary scratch space to append the hash fragment
	memcpy(out_, in, n_in - 4);
	memcpy(out_ + (n_in - 4), hash, 4);

	size_t n_limbs = MP_NLIMBS(n_in);
	mp_limb_t *limbs = base58check_malloc(n_limbs * sizeof(mp_limb_t));
	if (limbs) {
		bytes_to_limbs(limbs, (uint8_t *) out_, n_in);
		n_out_ = encode_limbs(out_, n_out_, limbs, n_limbs);
		base58check_free(limbs);

		*out = out_ - n_hdr;
		*n_out = n_out_ + n_hdr;
		return 0;
	}
	if (!*out)
		base58check_free(out_ - n_hdr);
	return -1;
}

int base58check_decode(unsigned char **restrict out, size_t *n_out, const char *restrict in, size_t n_in, size_t n_hdr) {
	size_t n_leading_zeros = 0;
	while (n_in && *in == '1')
		++n_leading_zeros, ++in, --n_in;

	size_t n_need = decoded_size_upper_bound(n_in);
	if (__builtin_uaddl_overflow(n_need, n_hdr, &n_need) ||
			__builtin_uaddl_overflow(n_need, n_leading_zeros, &n_need) ||
			n_need < 4 /* need at least 4 bytes for the hash fragment */)
		return -1;

	unsigned char *out_ = *out;
	size_t n_out_ = *n_out;
	if (!out_) {
		if (__builtin_uaddl_overflow(n_need, n_out_, &n_out_) ||
				!(out_ = base58check_malloc(n_out_)))
			return -1;
	}
	else if (n_out_ < n_need)
		return -1;

	memset(out_ + n_hdr, 0, n_leading_zeros);
	out_ += n_hdr += n_leading_zeros, n_out_ -= n_hdr;

	size_t n_limbs = MP_NLIMBS(n_out_);
	mp_limb_t *limbs = base58check_malloc(n_limbs * sizeof(mp_limb_t));
	if (limbs) {
		limbs[n_limbs - 1] = 0; // decode_limbs might not write the most significant limb
		if (decode_limbs(limbs, in, n_in) >= 0) {
			limbs_to_bytes(out_, limbs, n_out_);
			base58check_free(limbs);
			if (!*out_) {
				size_t chomp = 0;
				while (!out_[++chomp]);
				memmove(out_, out_ + chomp, n_out_ -= chomp);
			}
			out_ -= n_leading_zeros, n_out_ += n_leading_zeros, n_hdr -= n_leading_zeros;

			unsigned char hash[32];
			SHA256(out_, n_out_ -= 4, hash);
			SHA256(hash, sizeof hash, hash);
			if (!memcmp(hash, out_ + n_out_, 4)) {
				*out = out_ - n_hdr;
				*n_out = n_out_ + n_hdr;
				return 0;
			}
		}
		else
			base58check_free(limbs);
	}
	if (!*out)
		base58check_free(out_ - n_hdr);
	return -1;
}


void __attribute__ ((weak)) base58check_free(void *ptr) {
	free(ptr);
}

void * __attribute__ ((weak)) base58check_malloc(size_t size) {
	return malloc(size);
}
