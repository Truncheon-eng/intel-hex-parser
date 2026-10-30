// InOutData128.hpp
#ifndef IN_OUT_DATA_128_HPP
#define IN_OUT_DATA_128_HPP

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


struct InOutData128 {
	static constexpr int width = 128;
	static constexpr int num_words = 4;
	static constexpr int num_bytes = 16;

	union {
		uint32_t words[4];
		uint8_t bytes[16];

		// Structured word access
		struct {
			uint32_t w0;  // least significant 32 bits (bytes 0-3)
			uint32_t w1;  // bytes 4-7
			uint32_t w2;  // bytes 8-11
			uint32_t w3;  // most significant 32 bits (bytes 12-15)
		};

		// 64-bit access
		struct {
			uint64_t qword0;  // bytes 0-7
			uint64_t qword1;  // bytes 8-15
		} qwords;
	};

	enum class InitOrder {
		LITTLE_ENDIAN_IN,  // data already little-endian (default)
		BIG_ENDIAN_IN	   // data is big-endian, needs conversion
	};

	// -------------------------------------------------
	// Constructors
	// -------------------------------------------------
	InOutData128();
	explicit InOutData128(const uint32_t data[4]);
	InOutData128(uint32_t word0, uint32_t word1, uint32_t word2,
					uint32_t word3);
	explicit InOutData128(const uint8_t *data, size_t len = num_bytes);
	explicit InOutData128(const std::vector<uint8_t> &data);

	// Copy constructor
	InOutData128(const InOutData128 &other);

	// Move constructor
	InOutData128(InOutData128 &&other) noexcept;

	// -------------------------------------------------
	// Assignment operators
	// -------------------------------------------------
	InOutData128 &operator=(const InOutData128 &other);
	InOutData128 &operator=(InOutData128 &&other) noexcept;

	// -------------------------------------------------
	// Clear and access methods
	// -------------------------------------------------
	void clear();
	uint8_t get_byte(int index) const;
	void set_byte(int index, uint8_t value);

	// Direct copy from a byte array (little-endian)
	void set_from_bytes(const uint8_t *data, size_t len, size_t offset = 0);

	// For handling different byte orders
	void set_from_bytes_big_endian(const uint8_t *data, size_t len,
									size_t offset = 0);
	void set_from_bytes_little_endian(const uint8_t *data, size_t len,
										size_t offset = 0);

	static uint32_t bytes_to_word_big_endian(const uint8_t *bytes);
	static uint32_t bytes_to_word_little_endian(const uint8_t *bytes);

	// -------------------------------------------------
	// Vector methods
	// -------------------------------------------------
	// Deprecated !
	InOutData128 &from_vector(const std::vector<uint8_t> &vec,
								size_t start_idx = 0,
								InitOrder order = InitOrder::LITTLE_ENDIAN_IN);
	std::vector<uint8_t> to_vector() const;

	// Conversion to vector with explicit byte order
	std::vector<uint8_t> to_vector_little_endian() const;
	std::vector<uint8_t> to_vector_big_endian() const;

	// Vector methods with explicit byte order
	void from_vector_little_endian(const std::vector<uint8_t> &vec,
									size_t start_idx = 0);
	void from_vector_big_endian(const std::vector<uint8_t> &vec,
								size_t start_idx = 0);

	// -------------------------------------------------
	// Bitwise operations
	// -------------------------------------------------
	bool get_bit(int bit_index) const;
	void set_bit(int bit_index, bool value);

	enum class OutputFormat {
		HEX_BIG_ENDIAN,
		HEX_LITTLE_ENDIAN,
		BINARY_BIG_ENDIAN,
		BINARY_LITTLE_ENDIAN
	};

	// Static variable initialization
	OutputFormat default_output_format = OutputFormat::HEX_BIG_ENDIAN;

	// -------------------------------------------------
	// Output methods
	// -------------------------------------------------
	void print(const char *name = "") const;
	void print_bytes(const char *name = "") const;
	void print_words_and_bytes(const char *name = "") const;
	void print_hex(const char *name = "") const;
	void print_hex(OutputFormat format, const char *name) const;
	void print_binary(const char *name = "") const;
	void print_detailed(const char *name) const;

	std::string to_hex_string(
		OutputFormat format = OutputFormat::HEX_LITTLE_ENDIAN)
		const;							   // big-endian representation
	std::string to_hex_string_le() const;  // little-endian representation
	std::string to_binary_string(
		OutputFormat format = OutputFormat::HEX_LITTLE_ENDIAN) const;
	std::string to_binary_string_le() const;

	std::string to_raw_string(bool as_hex = true) const;
	void print_raw_memory(const char *name) const;
	void print_test_vector_format() const;

	// -------------------------------------------------
	// Access operators
	// -------------------------------------------------
	uint32_t operator[](int index) const;
	uint32_t &operator[](int index);

	// -------------------------------------------------
	// Convenience methods
	// -------------------------------------------------
	void set_word(int index, uint32_t value);
	uint32_t get_word(int index) const;
	void set_fields(uint32_t word0, uint32_t word1, uint32_t word2,
					uint32_t word3);

	// Explicit little-endian order methods
	uint32_t get_word_big_endian(int index) const;
	void set_word_big_endian(int index, uint32_t value);

	// Big-endian order methods (for completeness)
	uint32_t get_word_little_endian(int index) const;
	void set_word_little_endian(int index, uint32_t value);

	// -------------------------------------------------
	// Conversion
	// -------------------------------------------------
	std::array<uint32_t, 4> to_array() const;
	void from_array(const std::array<uint32_t, 4> &arr);
	std::array<uint32_t, 4> to_array_big_endian() const;
	void from_array_big_endian(const std::array<uint32_t, 4> &arr);

	// -------------------------------------------------
	// Binary operators
	// -------------------------------------------------
	InOutData128 operator&(const InOutData128 &other) const;
	InOutData128 operator|(const InOutData128 &other) const;
	InOutData128 operator^(const InOutData128 &other) const;
	InOutData128 operator~() const;

	InOutData128 &operator&=(const InOutData128 &other);
	InOutData128 &operator|=(const InOutData128 &other);
	InOutData128 &operator^=(const InOutData128 &other);

	// -------------------------------------------------
	// Comparison
	// -------------------------------------------------
	bool operator==(const InOutData128 &other) const;
	bool operator!=(const InOutData128 &other) const;

	// -------------------------------------------------
	// Static creation methods
	// -------------------------------------------------
	static InOutData128 from_hex_string(
		const std::string &hex_str);  // hex as big-endian
	static InOutData128 from_hex_string_le(
		const std::string &hex_str);  // hex as little-endian
	static InOutData128 from_words(uint32_t w0, uint32_t w1, uint32_t w2,
									uint32_t w3);

	// Static factory methods with explicit byte order
	static InOutData128 from_words_little_endian(uint32_t w0, uint32_t w1,
													uint32_t w2, uint32_t w3);

	static InOutData128 from_words_big_endian(uint32_t w0, uint32_t w1,
												uint32_t w2, uint32_t w3);

	static InOutData128 from_bytes(const uint8_t *bytes, size_t len = 16);
	static InOutData128 from_bytes_big_endian(const uint8_t *bytes,
												size_t len = 16);

	// Static factory methods from vectors
	static InOutData128 create_from_vector(
		const std::vector<uint8_t> &vec, size_t start_idx = 0,
		InitOrder order = InitOrder::LITTLE_ENDIAN_IN);

	static InOutData128 from_vector_le(const std::vector<uint8_t> &vec,
										size_t start_idx = 0);

	static InOutData128 from_vector_be(const std::vector<uint8_t> &vec,
										size_t start_idx = 0);

	// -------------------------------------------------
	// Byte order utilities
	// -------------------------------------------------
	static uint32_t swap_endian(uint32_t value);
	static uint64_t swap_endian(uint64_t value);

	// -------------------------------------------------
	// Verilator compatibility
	// -------------------------------------------------

private:
	// Helper method for converting big-endian to little-endian
	static void convert_big_to_little_endian(uint8_t *dest, const uint8_t *src,
												size_t len);
	bool check_vector_bounds(const std::vector<uint8_t> &vec, size_t start_idx,
								const char *method_name) const;
};

#endif	// IN_OUT_DATA_128_HPP


// Constructors can be used instead of the static methods:
// InOutData128 data1(vec_le.data(), vec_le.size());  // little-endian by
// default
// Or
// InOutData128 data2;
// data2.from_vector_little_endian(vec_le);
