<?php
namespace Base58Check;

function encode(string $str) {
	static $ffi = \FFI::cdef('
		size_t base58check_encode_buffer_size(const char *, size_t, size_t);
		int base58check_encode(char **, size_t *, const char *, size_t, size_t);
	', 'libbase58check.so.0');
	$n_out = $ffi->new('size_t');
	$n_out->cdata = $ffi->base58check_encode_buffer_size($str, strlen($str), 0);
	$out = $ffi->new("char[$n_out]");
	$out_ptr = \FFI::addr($out[0]);
	if ($ffi->base58check_encode(\FFI::addr($out_ptr), \FFI::addr($n_out), $str, strlen($str), 0) < 0)
		throw new \Exception('error in base58check_encode()');
	return \FFI::string($out, $n_out->cdata);
}

function decode(string $str) {
	static $ffi = \FFI::cdef('
		size_t base58check_decode_buffer_size(const char *, size_t, size_t);
		int base58check_decode(char **, size_t *, const char *, size_t, size_t);
	', 'libbase58check.so.0');
	$n_out = $ffi->new('size_t');
	$n_out->cdata = $ffi->base58check_decode_buffer_size($str, strlen($str), 0);
	$out = $ffi->new("char[$n_out]");
	$out_ptr = \FFI::addr($out[0]);
	if ($ffi->base58check_decode(\FFI::addr($out_ptr), \FFI::addr($n_out), $str, strlen($str), 0) < 0)
		throw new \Exception('error in base58check_decode()');
	return \FFI::string($out, $n_out->cdata);
}
