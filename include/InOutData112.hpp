#pragma once

#ifndef IN_OUT_DATA_112_HPP
#define IN_OUT_DATA_112_HPP

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


struct InOutData112 {
	// Size constants
	static constexpr int TOTAL_BITS = 112;
	static constexpr int NUM_BYTES = 14;  // 112 / 8
	static constexpr int NUM_WORDS =
		4;	// 4 x 32 bits = 128 bits (with padding)
	static constexpr int FULL_WORDS = 3;					// 112 / 32 = 3.5
	static constexpr int REMAINING_BITS = 16;				// 112 % 32 = 16
	static constexpr uint32_t MASK_LAST_WORD = 0x0000FFFF;	// Mask for 16 bits

	union {
		// Access as an array of 32-bit words
		uint32_t words[NUM_WORDS];

		// Access as a byte array
		uint8_t bytes[NUM_BYTES];

		// Structured access to words
		struct {
			uint32_t w0;
			uint32_t w1;
			uint32_t w2;
			uint32_t w3;  // Only the least significant 16 bits are used
		};
	};

	// -------------------------------------------------
	// Constructors
	// -------------------------------------------------

	enum class InitOrder {
		LITTLE_ENDIAN_IN,  // data already little-endian (default)
		BIG_ENDIAN_IN	   // data is big-endian, needs conversion
	};

	InOutData112();

	// Constructor from an array of 32-bit words
	explicit InOutData112(const uint32_t data[4]);

	// Constructor from four 32-bit words
	InOutData112(uint32_t word0, uint32_t word1, uint32_t word2,
					uint32_t word3);

	// Constructor from a byte array with explicit byte order
	explicit InOutData112(const uint8_t *data, size_t len = NUM_BYTES,
							InitOrder order = InitOrder::LITTLE_ENDIAN_IN);

	// Constructor from a byte vector with explicit byte order
	explicit InOutData112(const std::vector<uint8_t> &data,
							InitOrder order = InitOrder::LITTLE_ENDIAN_IN);

	// Copy constructor
	InOutData112(const InOutData112 &other);
	InOutData112(InOutData112 &&other) noexcept;

	// -------------------------------------------------
	// Assignment operators
	// -------------------------------------------------
	InOutData112 &operator=(const InOutData112 &other);
	InOutData112 &operator=(InOutData112 &&other) noexcept;

	// -------------------------------------------------
	// Basic methods
	// -------------------------------------------------
	void clear();

	// Byte access
	uint8_t get_byte(int index) const;
	void set_byte(int index, uint8_t value);

	// Word access (little-endian semantics)
	uint32_t get_word(int index) const;
	void set_word(int index, uint32_t value);

	// Word access with explicit byte order
	uint32_t get_word_little_endian(int index) const;
	uint32_t get_word_big_endian(int index) const;
	void set_word_little_endian(int index, uint32_t value);
	void set_word_big_endian(int index, uint32_t value);

	// Set all words
	void set_all_words(uint32_t word0, uint32_t word1, uint32_t word2,
						uint32_t word3);

	// -------------------------------------------------
	// Set-from-byte-array methods
	// -------------------------------------------------
	// Generic method (little-endian by default)
	void set_from_bytes(const uint8_t *data, size_t len, size_t offset = 0);

	// Explicit byte-order methods
	void set_from_bytes_little_endian(const uint8_t *data, size_t len,
										size_t offset = 0);
	void set_from_bytes_big_endian(const uint8_t *data, size_t len,
									size_t offset = 0);

	// -------------------------------------------------
	// Vector methods
	// -------------------------------------------------
	// Generic (little-endian by default)
	void from_vector(const std::vector<uint8_t> &vec, size_t start_idx = 0);
	std::vector<uint8_t> to_vector() const;

	// Explicit byte order
	void from_vector_little_endian(const std::vector<uint8_t> &vec,
									size_t start_idx = 0);
	void from_vector_big_endian(const std::vector<uint8_t> &vec,
								size_t start_idx = 0);
	std::vector<uint8_t> to_vector_little_endian() const;
	std::vector<uint8_t> to_vector_big_endian() const;

	// -------------------------------------------------
	// Bitwise operations
	// -------------------------------------------------
	bool get_bit(int bit_index) const;
	void set_bit(int bit_index, bool value);

	// -------------------------------------------------
	// Width truncation utilities
	// -------------------------------------------------
	void apply_mask();
	bool is_masked_correctly() const;

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
	void print_words_and_bytes(const char *name = "") const;
	void print_detailed(const char *name = "") const;
	void print_raw_memory(const char *name = "") const;

	std::string to_hex_string() const;
	std::string to_hex_string(OutputFormat format) const;
	std::string to_binary_string() const;
	std::string to_binary_string(OutputFormat format) const;
	std::string to_raw_string(bool as_hex = true) const;

	void print_hex(const char *name = "") const;
	void print_hex(OutputFormat format, const char *name) const;
	void print_binary(const char *name = "") const;
	void print_binary(OutputFormat format, const char *name) const;

	// -------------------------------------------------
	// Conversion to arrays
	// -------------------------------------------------
	std::array<uint32_t, 4> to_array() const;
	void from_array(const std::array<uint32_t, 4> &arr);

	// -------------------------------------------------
	// Operators
	// -------------------------------------------------
	uint32_t operator[](int index) const;
	uint32_t &operator[](int index);

	InOutData112 operator&(const InOutData112 &other) const;
	InOutData112 operator|(const InOutData112 &other) const;
	InOutData112 operator^(const InOutData112 &other) const;
	InOutData112 operator~() const;

	InOutData112 &operator&=(const InOutData112 &other);
	InOutData112 &operator|=(const InOutData112 &other);
	InOutData112 &operator^=(const InOutData112 &other);

	bool operator==(const InOutData112 &other) const;
	bool operator!=(const InOutData112 &other) const;

	// Ordering operators
	bool operator<(const InOutData112 &other) const;
	bool operator>(const InOutData112 &other) const;
	bool operator<=(const InOutData112 &other) const;
	bool operator>=(const InOutData112 &other) const;

	// -------------------------------------------------
	// Static factory methods
	// -------------------------------------------------
	static InOutData112 from_hex_string(const std::string &hex_str);

	static InOutData112 from_words(uint32_t w0, uint32_t w1, uint32_t w2,
									uint32_t w3);

	static InOutData112 from_words_little_endian(uint32_t w0, uint32_t w1,
													uint32_t w2, uint32_t w3);

	static InOutData112 from_words_big_endian(uint32_t w0, uint32_t w1,
												uint32_t w2, uint32_t w3);

	static InOutData112 from_bytes(
		const uint8_t *bytes, size_t len = NUM_BYTES,
		InitOrder order = InitOrder::LITTLE_ENDIAN_IN);
	static InOutData112 from_bytes_little_endian(const uint8_t *bytes,
													size_t len = NUM_BYTES);
	static InOutData112 from_bytes_big_endian(const uint8_t *bytes,
												size_t len = NUM_BYTES);

	// -------------------------------------------------
	// Byte order utilities
	// -------------------------------------------------
	static uint32_t swap_endian(uint32_t value);
	static uint32_t bytes_to_word_big_endian(const uint8_t *bytes);
	static uint32_t bytes_to_word_little_endian(const uint8_t *bytes);

	// -------------------------------------------------
	// Verilator compatibility
	// -------------------------------------------------

private:
	// Helper methods
	static void convert_big_to_little_endian(uint8_t *dest, const uint8_t *src,
												size_t len);

	// Helper method for applying the mask
	void apply_mask_internal();
};

#endif	// IN_OUT_DATA_112_HPP
