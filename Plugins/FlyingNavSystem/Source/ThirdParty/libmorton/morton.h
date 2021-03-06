#pragma once

// This file will always contain inline functions which point to the fastest Morton encoding/decoding implementation
// IF you just want to use the fastest method to encode/decode morton codes, include this header.

// If you want to experiment with alternative methods (which might be slower, all depending on hardware / your data set)
// check the individual headers below.

#include "morton3D.h"

namespace libmorton {
	// Functions under this are stubs which will always point to fastest implementation at the moment
	//-----------------------------------------------------------------------------------------------

	// ENCODING
	inline uint_fast32_t morton3D_32_encode(const uint_fast16_t x, const uint_fast16_t y, const uint_fast16_t z) {
		return m3D_e_sLUT<uint_fast32_t, uint_fast16_t>(x, y, z);
	}
	inline uint_fast64_t morton3D_64_encode(const uint_fast32_t x, const uint_fast32_t y, const uint_fast32_t z) {
		return m3D_e_sLUT<uint_fast64_t, uint_fast32_t>(x, y, z);
	}

	// DECODING
	inline void morton3D_32_decode(const uint_fast32_t morton, uint_fast16_t& x, uint_fast16_t& y, uint_fast16_t& z) {
		m3D_d_sLUT<uint_fast32_t, uint_fast16_t>(morton, x, y, z);
	}
	inline void morton3D_64_decode(const uint_fast64_t morton, uint_fast32_t& x, uint_fast32_t& y, uint_fast32_t& z) {
		m3D_d_sLUT<uint_fast64_t, uint_fast32_t>(morton, x, y, z);
	}
}