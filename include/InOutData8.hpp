// InOutData8.hpp
#ifndef IN_OUT_DATA_8_HPP
#define IN_OUT_DATA_8_HPP

#include <iostream>
#include <stdint.h>
#include <iomanip>
#include <array>
#include <cstring>
#include <vector>
#include <bitset>
#include <sstream>
#include <stdexcept>


#include "Definitions.hpp"

using std::cerr;
using std::cout;
using std::endl;

struct InOutData8 {
	static constexpr int WIDTH = 8;
	static constexpr int NUM_BYTES = 1;

	// Data is stored in little-endian format
	union {
		uint8_t bytes[1];

		struct {
			uint8_t b0;
		};

		uint8_t packed;	 // Single 8-bit value
	};

	enum class InitOrder {
		LITTLE_ENDIAN_IN,  // data already little-endian (default)
		BIG_ENDIAN_IN	   // data is big-endian, needs conversion
	};

	// -------------------------------------------------
	// Constructors
	// -------------------------------------------------
	InOutData8();
	explicit InOutData8(const uint8_t data[1]);
	explicit InOutData8(uint8_t value);
	explicit InOutData8(const std::vector<uint8_t> &data, size_t start_idx = 0);

	// Copy and move constructors
	InOutData8(const InOutData8 &other);
	InOutData8(InOutData8 &&other) noexcept;

	// -------------------------------------------------
	// Assignment operators
	// -------------------------------------------------
	InOutData8 &operator=(const InOutData8 &other);
	InOutData8 &operator=(InOutData8 &&other) noexcept;

	// -------------------------------------------------
	// Basic methods
	// -------------------------------------------------
	void clear();

	// Data access
	uint8_t get_byte() const;
	void set_byte(uint8_t value);

	uint8_t get_packed() const;
	void set_packed(uint8_t value);

	void set_field(uint8_t byte0);

	// -------------------------------------------------
	// Set-from-byte-array methods
	// -------------------------------------------------
	void set_from_bytes(const uint8_t *data, size_t len, size_t offset = 0);
	// void set_from_bytes_little_endian(const uint8_t* data,
	//								  size_t len,
	//								  size_t offset = 0);
	// void set_from_bytes_big_endian(const uint8_t* data,
	//							   size_t len,
	//							   size_t offset = 0);

	// -------------------------------------------------
	// Vector methods
	// -------------------------------------------------
	void from_vector(const std::vector<uint8_t> &vec, size_t start_idx = 0);
	// void from_vector_little_endian(const std::vector<uint8_t>& vec,
	//							   size_t start_idx = 0);
	// void from_vector_big_endian(const std::vector<uint8_t>& vec,
	//							size_t start_idx = 0);

	std::vector<uint8_t> to_vector() const;
	// std::vector<uint8_t> to_vector_little_endian() const;
	// std::vector<uint8_t> to_vector_big_endian() const;

	// -------------------------------------------------
	// Bitwise operations
	// -------------------------------------------------
	bool get_bit(int bit_index) const;
	void set_bit(int bit_index, bool value);

	// -------------------------------------------------
	// Output methods
	// -------------------------------------------------
	enum class OutputFormat {
		HEX_BIG_ENDIAN,
		HEX_LITTLE_ENDIAN,
		BINARY_BIG_ENDIAN,
		BINARY_LITTLE_ENDIAN
	};

	static OutputFormat default_output_format;
	static void set_default_output_format(OutputFormat format);

	void print(const char *name = "") const;
	void print_bytes(const char *name = "") const;
	void print_detailed(const char *name = "") const;
	void print_raw_memory(const char *name = "") const;

	std::string to_hex_string() const;
	std::string to_hex_string(OutputFormat format) const;
	std::string to_binary_string() const;
	std::string to_binary_string(OutputFormat format) const;
	std::string to_raw_string(bool as_hex = true) const;
	std::string to_bin_string() const;	// for backward compatibility

	void print_hex(const char *name = "") const;
	void print_hex(OutputFormat format, const char *name) const;
	void print_binary(const char *name = "") const;
	void print_binary(OutputFormat format, const char *name) const;
	void print_bin(const char *name = "") const;  // for backward compatibility

	// -------------------------------------------------
	// Conversion to arrays
	// -------------------------------------------------
	std::array<uint8_t, 1> to_array() const;
	void from_array(const std::array<uint8_t, 1> &arr);

	// -------------------------------------------------
	// Operators
	// -------------------------------------------------
	uint8_t operator[](int index) const;
	uint8_t &operator[](int index);

	InOutData8 operator&(const InOutData8 &other) const;
	InOutData8 operator|(const InOutData8 &other) const;
	InOutData8 operator^(const InOutData8 &other) const;
	InOutData8 operator~() const;

	InOutData8 &operator&=(const InOutData8 &other);
	InOutData8 &operator|=(const InOutData8 &other);
	InOutData8 &operator^=(const InOutData8 &other);

	bool operator==(const InOutData8 &other) const;
	bool operator!=(const InOutData8 &other) const;

	// Cast and assignment operators
	operator uint8_t() const;
	InOutData8 &operator=(uint8_t value);

	// -------------------------------------------------
	// Static factory methods
	// -------------------------------------------------
	static InOutData8 from_hex_string(const std::string &hex_str);
	// static InOutData8 from_hex_string_le(const std::string& hex_str);

	static InOutData8 from_byte(uint8_t value);
	static InOutData8 from_bytes(const uint8_t *bytes, size_t len = 1);
	static InOutData8 from_bytes_little_endian(const uint8_t *bytes,
												size_t len = 1);
	static InOutData8 from_bytes_big_endian(const uint8_t *bytes,
											size_t len = 1);
	static InOutData8 create_from_vector(const std::vector<uint8_t> &vec,
											size_t start_idx = 0);
	// static InOutData8 from_vector(const std::vector<uint8_t>& vec,
	//							  size_t start_idx = 0);
	// static InOutData8 from_vector_little_endian(const std::vector<uint8_t>&
	// vec, 											size_t start_idx = 0);
	// static InOutData8 from_vector_big_endian(const std::vector<uint8_t>& vec,
	// size_t start_idx
	//= 0);
};

static_assert(sizeof(InOutData8) == 1, "InOutData8 must occupy exactly 1 byte");

#endif	// IN_OUT_DATA_8_HPP
