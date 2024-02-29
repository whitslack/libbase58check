/// @file

#include <stddef.h>

#ifdef __cplusplus
# define restrict __restrict
extern "C" {
#endif

/**
 * @brief Returns the recommended size of a buffer to hold the Base58Check
 * encoding of the specified input data.
 * @param[in] in A pointer to the input data needing to be encoded. Must not be
 * @c NULL.
 * @param n_in The number of bytes of input data at @p in.
 * @param n_pad The minimum number of excess bytes to include in the returned
 * estimate.
 * @return An upper-bound estimate of the size of the Base58Check encoding of
 * the specified input data, or @c SIZE_MAX upon overflow.
 */
size_t base58check_encode_buffer_size(const unsigned char *in, size_t n_in, size_t n_pad)
	__attribute__ ((__access__ (read_only, 1), __nonnull__, __nothrow__, __pure__));

/**
 * @brief Returns the recommended size of a buffer to hold the decoding of the
 * specified Base58Check encoding.
 * @param[in] in A pointer to the Base58Check encoding needing to be decoded.
 * Must not be @c NULL.
 * @param n_in The size of the Base58Check encoding at @p in, not including any
 * possible terminator.
 * @param n_pad The minimum number of excess bytes to include in the returned
 * estimate.
 * @return An upper-bound estimate of the size of the decoding of the specified
 * Base58Check encoding, or @c SIZE_MAX upon overflow.
 */
size_t base58check_decode_buffer_size(const char *in, size_t n_in, size_t n_pad)
	__attribute__ ((__access__ (read_only, 1), __nonnull__, __nothrow__, __pure__));

/**
 * @brief Encodes data in Base58Check format.
 * @param[in,out] out
 * @parblock
 * A pointer to the address of a buffer into which the encoding is to be
 * written. Must not be @c NULL.
 *
 * If @c *out is @c NULL upon entry, then this function will allocate a
 * suitably sized buffer by calling base58check_malloc() and will set @c *out
 * to the address of that buffer. It is the caller's responsibility to free the
 * allocated buffer by passing its address to base58check_free().
 *
 * If @c *out is not @c NULL upon entry, then this function will write the
 * encoding to the buffer at address @c *out, which is of size given by
 * @c *n_out.
 * @endparblock
 * @param[in,out] n_out
 * @parblock
 * If @c *out is not @c NULL upon entry, a pointer to the size of the buffer at
 * address @c *out, or else a pointer to the minimum number of excess bytes of
 * buffer space to allocate beyond the end of the encoding. Must not be
 * @c NULL.
 *
 * Upon return, @c *n_out will be set to the size of the encoding, including
 * any @p n_hdr bytes but not including any excess.
 * @endparblock
 * @param[in] in A pointer to the input data to be encoded. Must not be
 * @c NULL.
 * @param n_in The number of bytes of input data at @p in.
 * @param n_hdr The number of bytes to skip at @c *out before writing the
 * encoding.
 * @return 0 if the encoding was successful, or a negative number upon error,
 * which may be because @p n_in was too large, @c *n_out was too small or too
 * large, @p n_hdr was too large, or there was a failure to allocate memory.
 */
int base58check_encode(char **restrict out, size_t *n_out, const unsigned char *restrict in, size_t n_in, size_t n_hdr)
	__attribute__ ((__access__ (read_write, 1), __access__ (read_write, 2), __access__ (read_only, 3), __nonnull__, __nothrow__));

/**
 * @brief Decodes data from Base58Check format.
 * @param[in,out] out
 * @parblock
 * A pointer to the address of a buffer into which the decoded data are to be
 * written. Must not be @c NULL.
 *
 * If @c *out is @c NULL upon entry, then this function will allocate a
 * suitably sized buffer by calling base58check_malloc() and will set @c *out
 * to the address of that buffer. It is the caller's responsibility to free the
 * allocated buffer by passing its address to base58check_free().
 *
 * If @c *out is not @c NULL upon entry, then this function will write the
 * decoded data to the buffer at address @c *out, which is of size given by
 * @c *n_out.
 * @endparblock
 * @param[in,out] n_out
 * @parblock
 * If @c *out is not @c NULL upon entry, a pointer to the size of the buffer at
 * address @c *out, or else a pointer to the minimum number of excess bytes of
 * buffer space to allocate beyond the end of the decoded data. Must not be
 * @c NULL.
 *
 * Upon return, @c *n_out will be set to the size of the decoded data,
 * including any @p n_hdr bytes but not including any excess.
 * @endparblock
 * @param[in] in A pointer to the Base58Check encoding to be decoded. Must not
 * be @c NULL.
 * @param n_in The size of the Base58Check encoding at @p in, not including any
 * possible terminator.
 * @param n_hdr The number of bytes to skip at @c *out before writing the
 * decoded data.
 * @return 0 if the decoding was successful, or a negative number upon error,
 * which may be because @p n_in was too large, @c *n_out was too small or too
 * large, @p n_hdr was too large, the encoding at @p in contained an illegal
 * character, there was a checksum mismatch, or there was a failure to allocate
 * memory.
 */
int base58check_decode(unsigned char **restrict out, size_t *n_out, const char *restrict in, size_t n_in, size_t n_hdr)
	__attribute__ ((__access__ (read_write, 1), __access__ (read_write, 2), __access__ (read_only, 3), __nonnull__, __nothrow__));


/**
 * @brief Frees memory allocated by base58check_malloc().
 * @note This is a weak symbol in libbase58check that may be overridden at
 * (dynamic) link time. The default implementation forwards to @c free.
 * @param ptr A pointer to the memory allocation to be freed. Must have been
 * previously returned by base58check_malloc() and must not be @c NULL.
 */
void base58check_free(void *ptr)
	__attribute__ ((__nonnull__, __nothrow__));

/**
 * @brief Allocates memory.
 * @note This is a weak symbol in libbase58check that may be overridden at
 * (dynamic) link time. The default implementation forwards to @c malloc.
 * @param size The size of the requested allocation.
 * @return A pointer to the allocated memory, or @c NULL if a memory allocation
 * of the requested size could not be provided. If not @c NULL, this pointer
 * must be passed to base58check_free() to free the allocation.
 */
void * base58check_malloc(size_t size)
	__attribute__ ((__malloc__, __malloc__ (base58check_free, 1)));

#ifdef __cplusplus
} // extern "C"
# undef restrict


#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>
#if __cplusplus >= 201103L
# include <type_traits>
# if __cplusplus >= 201703L
#  include <string_view>
#  if __cplusplus >= 202002L
#   include <span>
#  endif
# endif
#endif

namespace base58check {

#if __cplusplus >= 201703L
using std::byte;
#else
enum byte : unsigned char { };
#endif

static inline std::string
__attribute__ ((__pure__))
encode(const byte in[], size_t n_in, size_t n_hdr = 0) {
	std::string ret;
	ret.resize(::base58check_encode_buffer_size(reinterpret_cast<const unsigned char *>(in), n_in, n_hdr));
	char *out = &ret.front();
	size_t n_out = ret.size();
	if (::base58check_encode(&out, &n_out, reinterpret_cast<const unsigned char *>(in), n_in, n_hdr) < 0)
		throw std::length_error("Base58Check encoding is too large");
	ret.resize(n_out);
	return ret;
}

template <size_t N_in>
static inline std::string
__attribute__ ((__pure__))
encode(const byte (&in)[N_in], size_t n_hdr = 0) {
	return ::base58check::encode(in, N_in, n_hdr);
}

static inline std::vector<byte>
__attribute__ ((__pure__))
decode(const char in[], size_t n_in, size_t n_hdr = 0) {
	std::vector<byte> ret;
	ret.resize(::base58check_decode_buffer_size(in, n_in, n_hdr));
	unsigned char *out = reinterpret_cast<unsigned char *>(ret.data());
	size_t n_out = ret.size();
	if (::base58check_decode(&out, &n_out, in, n_in, n_hdr) < 0)
		throw std::invalid_argument("not a valid Base58Check encoding");
	ret.resize(n_out);
	return ret;
}

#if __cplusplus >= 201103L

#if __cpp_concepts >= 201907L
template <typename T> requires std::is_trivially_copyable_v<T>
#else
template <typename T, typename std::enable_if<std::is_trivially_copyable<T>::value, bool>::type = true>
#endif
static inline std::string
__attribute__ ((__pure__))
encode(const T in[], size_t n_in, size_t n_hdr = 0) {
	return ::base58check::encode(reinterpret_cast<const byte *>(in), n_in * sizeof(T), n_hdr);
}

#if __cpp_concepts >= 201907L
template <typename T> requires std::is_trivially_copyable_v<T>
#else
template <typename T, typename std::enable_if<std::is_trivially_copyable<T>::value, bool>::type = true>
#endif
static inline std::string
__attribute__ ((__pure__))
encode(const T &in, size_t n_hdr = 0) {
	return ::base58check::encode(&in, 1, n_hdr);
}

#if __cplusplus >= 201703L

static inline std::vector<byte>
__attribute__ ((__pure__))
decode(std::string_view in, size_t n_hdr = 0) {
	return ::base58check::decode(in.data(), in.size(), n_hdr);
}

#if __cplusplus >= 202002L

#if __cpp_concepts >= 201907L
template <typename T> requires std::is_trivially_copyable_v<T>
#else
template <typename T, typename std::enable_if<std::is_trivially_copyable<T>::value, bool>::type = true>
#endif
static inline std::string
__attribute__ ((__pure__))
encode(std::span<const T> in, size_t n_hdr = 0) {
	return ::base58check::encode(in.as_bytes().data(), in.size_bytes(), n_hdr);
}

#endif // __cplusplus >= 202002L

#endif // __cplusplus >= 201703L

#endif // __cplusplus >= 201103L

} // namespace base58check

#endif // defined(__cplusplus)
