// InOutData128.cpp
#include <algorithm>
#include "InOutData128.hpp"

// -------------------------------------------------
// Constructors
// -------------------------------------------------
InOutData128::InOutData128() { clear(); }

InOutData128::InOutData128(const uint32_t data[4]) {
	for (int i = 0; i < num_words; i++) {
		words[i] = data[i];
	}
}

InOutData128::InOutData128(uint32_t word0, uint32_t word1, uint32_t word2,
							uint32_t word3)
	: w0(word0), w1(word1), w2(word2), w3(word3) {}

InOutData128::InOutData128(const uint8_t *data, size_t len) {
	set_from_bytes(data, len);
}

InOutData128::InOutData128(const std::vector<uint8_t> &data) {
	from_vector(data);
}

// Copy constructor
InOutData128::InOutData128(const InOutData128 &other) {
	std::memcpy(bytes, other.bytes, num_bytes);
}

// Move constructor
InOutData128::InOutData128(InOutData128 &&other) noexcept {
	std::memcpy(bytes, other.bytes, num_bytes);
	other.clear();
}

// -------------------------------------------------
// Assignment operators
// -------------------------------------------------
InOutData128 &InOutData128::operator=(const InOutData128 &other) {
	if (this != &other) {
		std::memcpy(bytes, other.bytes, num_bytes);
	}
	return *this;
}

InOutData128 &InOutData128::operator=(InOutData128 &&other) noexcept {
	if (this != &other) {
		std::memcpy(bytes, other.bytes, num_bytes);
		other.clear();
	}
	return *this;
}

// -------------------------------------------------
// Clear and access methods
// -------------------------------------------------
void InOutData128::clear() { std::memset(bytes, 0, num_bytes); }

uint8_t InOutData128::get_byte(int index) const {
	if (index < 0 || index >= num_bytes) {
		return 0;
	}
	return bytes[index];
}

void InOutData128::set_byte(int index, uint8_t value) {
	if (index < 0 || index >= num_bytes) {
		cerr << RED << CROSS_MARK << __FUNCTION__ << " index error !\t"
				<< NORMAL << endl;
		return;
	};
	bytes[index] = value;
}

void InOutData128::set_from_bytes(const uint8_t *data, size_t len,
									size_t offset) {
	if (!data || len == 0) {
		cerr << RED << CROSS_MARK << __FUNCTION__ << " index error !\t"
				<< NORMAL << endl;
		return;
	}
	if (offset >= static_cast<size_t>(num_bytes)) {
		cerr << RED << CROSS_MARK << __FUNCTION__ << " index error !\t"
				<< NORMAL << endl;
		return;
	}
	size_t copy_len = std::min(len, static_cast<size_t>(num_bytes) - offset);
	if (copy_len > 0) {
		std::memcpy(bytes + offset, data, copy_len);
	}
}

void InOutData128::set_from_bytes_big_endian(const uint8_t *data, size_t len,
												size_t offset) {
	if (!data || len == 0) {
		cerr << RED << CROSS_MARK << __FUNCTION__ << " index error !\t"
				<< NORMAL << endl;
		return;
	}

	if (offset >= static_cast<size_t>(num_bytes)) {
		cerr << RED << CROSS_MARK << __FUNCTION__ << " index error !\t"
				<< NORMAL << endl;
		return;
	}
	size_t copy_len = std::min(len, static_cast<size_t>(num_bytes) - offset);
	if (copy_len > 0) {
		convert_big_to_little_endian(bytes + offset, data, copy_len);
	}
}

void InOutData128::convert_big_to_little_endian(uint8_t *dest,
												const uint8_t *src,
												size_t len) {
	for (size_t i = 0; i < len; i++) {
		dest[i] = src[len - 1 - i];
	}
}

void InOutData128::set_from_bytes_little_endian(const uint8_t *data, size_t len,
												size_t offset) {
	set_from_bytes(data, len, offset);
}

// -------------------------------------------------
// Vector handling
// -------------------------------------------------
InOutData128 &InOutData128::from_vector(const std::vector<uint8_t> &vec,
										size_t start_idx, InitOrder order) {
	if (order == InitOrder::LITTLE_ENDIAN_IN) {
		from_vector_little_endian(vec, start_idx);
	} else {
		from_vector_big_endian(vec, start_idx);
	}
	return *this;  // Return *this for method chaining
}

// -------------------------------------------------
// Vector methods (explicit byte order)
// -------------------------------------------------

void InOutData128::from_vector_big_endian(const std::vector<uint8_t> &vec,
											size_t start_idx) {
	check_vector_bounds(vec, start_idx, "from_vector_big_endian");

	// Convert big-endian to little-endian
	for (size_t i = 0; i < num_bytes; i++) {
		size_t vec_idx = start_idx + i;
		size_t mem_idx = num_bytes - 1 - i;
		bytes[mem_idx] = vec[vec_idx];
	}
}

void InOutData128::from_vector_little_endian(const std::vector<uint8_t> &vec,
												size_t start_idx) {
	check_vector_bounds(vec, start_idx, "from_vector_little_endian");

	// Direct copy
	std::memcpy(bytes, vec.data() + start_idx, num_bytes);
}

std::vector<uint8_t> InOutData128::to_vector() const {
	return to_vector_little_endian();
}

std::vector<uint8_t> InOutData128::to_vector_little_endian() const {
	return std::vector<uint8_t>(bytes, bytes + num_bytes);
}

std::vector<uint8_t> InOutData128::to_vector_big_endian() const {
	std::vector<uint8_t> result(num_bytes);
	for (size_t i = 0; i < num_bytes; i++) {
		result[num_bytes - 1 - i] = bytes[i];
	}
	return result;
}

// -------------------------------------------------
// Bitwise operations
// -------------------------------------------------
bool InOutData128::get_bit(int bit_index) const {
	if (bit_index < 0 || bit_index >= width) {
		cerr << RED << CROSS_MARK << __FUNCTION__ << " index error !\t"
				<< NORMAL << endl;
		return false;
	}

	int byte_idx = bit_index / 8;
	int bit_in_byte = bit_index % 8;
	return (bytes[byte_idx] >> bit_in_byte) & 0x01;
}

void InOutData128::set_bit(int bit_index, bool value) {
	if (bit_index < 0 || bit_index >= width) {
		cerr << RED << CROSS_MARK << __FUNCTION__ << " index error !\t"
				<< NORMAL << endl;
		return;
	}

	int byte_idx = bit_index / 8;
	int bit_in_byte = bit_index % 8;

	if (value) {
		bytes[byte_idx] |= (1u << bit_in_byte);
	} else {
		bytes[byte_idx] &= ~(1u << bit_in_byte);
	}
}

// -------------------------------------------------
// Output methods
// -------------------------------------------------
void InOutData128::print(const char *name) const {
	std::cout << name
				<< " InOutData128 (128 bits, LITTLE-ENDIAN):" << std::endl;

	// Little-endian representation (as stored in memory)
	std::cout << "  Memory words (word0 = bytes[0..3]): { ";
	for (int i = 0; i < num_words; i++) {
		std::cout << "0x" << std::hex << std::setw(8) << std::setfill('0')
					<< words[i];
		if (i < 3) {
			std::cout << ", ";
		}
	}
	std::cout << " }" << std::dec << std::endl;

	// Big-endian representation for comparison
	std::cout << "  Logical words (word0 = most significant 32 bits): { ";
	for (int i = 3; i >= 0; i--) {
		std::cout << "0x" << std::hex << std::setw(8) << std::setfill('0')
					<< words[i];
		if (i > 0) {
			std::cout << ", ";
		}
	}
	std::cout << " }" << std::dec << std::endl;
}

void InOutData128::print_bytes(const char *name) const {
	std::cout << name << " Bytes (little-endian):" << std::endl;
	for (int i = 0; i < 16; i++) {
		std::cout << "  [" << std::setw(2) << i << "] = 0x" << std::hex
					<< std::setw(2) << std::setfill('0')
					<< static_cast<int>(bytes[i]) << std::dec;
		if ((i + 1) % 4 == 0) {
			std::cout << "  (word " << i / 4 << ")";
		}
		std::cout << std::endl;
	}
}

void InOutData128::print_words_and_bytes(const char *name) const {
	std::cout << name << " Words and bytes:" << std::endl;
	for (int i = 0; i < num_words; i++) {
		std::cout << "  Word[" << i << "] = 0x" << std::hex << std::setw(8)
					<< std::setfill('0') << words[i] << std::dec << "  bytes "
					<< (i * 4) << "-" << (i * 4 + 3) << ": ";
		for (int j = 0; j < num_words; j++) {
			std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0')
						<< static_cast<int>(bytes[i * 4 + j]);
			if (j < 3) {
				std::cout << " ";
			}
		}
		std::cout << std::dec << std::endl;
	}
}

// New
std::string InOutData128::to_raw_string(bool as_hex) const {
	std::stringstream ss;
	if (as_hex) {
		ss << std::hex << std::setfill('0');
		for (int i = 0; i < num_bytes; i++) {
			ss << std::setw(2) << static_cast<int>(bytes[i]);
			if (i < num_bytes - 1) {
				ss << " ";
			}
		}
	} else {
		for (int i = 0; i < num_bytes; i++) {
			ss << std::bitset<8>(bytes[i]);
			if (i < num_bytes - 1) {
				ss << " ";
			}
		}
	}
	return ss.str();
}

void InOutData128::print_raw_memory(const char *name) const {
	std::cout << name;
	if (strlen(name) > 0) {
		std::cout << ": ";
	}

	std::cout << "Raw bytes in memory: ";
	for (int i = 0; i < num_bytes; i++) {
		std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0')
					<< static_cast<int>(bytes[i]);
		if (i < num_bytes - 1) {
			std::cout << " ";
		}
	}
	std::cout << std::dec << std::endl;
}

// Or for test vectors:
void InOutData128::print_test_vector_format() const {
	std::cout << "Test vector format: ";
	for (int i = 0; i < num_bytes; i++) {
		std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0')
					<< static_cast<int>(bytes[i]);
		if (i < num_bytes - 1) {
			std::cout << ", ";
			if ((i + 1) % 8 == 0) {
				std::cout << std::endl << "                    ";
			}
		}
	}
	std::cout << std::dec << std::endl;
}

void InOutData128::print_hex(const char *name) const {
	std::cout << name;
	if (strlen(name) > 0) {
		std::cout << " ";
	}
	std::cout << "Hex (default): " << to_hex_string() << std::endl;
}

void InOutData128::print_hex(OutputFormat format, const char *name) const {
	std::cout << name;
	if (strlen(name) > 0) {
		std::cout << " ";
	}

	switch (format) {
		case OutputFormat::HEX_BIG_ENDIAN:
			std::cout << "Hex (BE): ";
			break;
		case OutputFormat::HEX_LITTLE_ENDIAN:
			std::cout << "Hex (LE): ";
			break;
		default:
			std::cout << "Hex: ";
			break;
	}

	std::cout << to_hex_string(format) << std::endl;
}

void InOutData128::print_binary(const char *name) const {
	std::cout << name << " Binary (BE): " << to_binary_string() << std::endl;
	std::cout << name << " Binary (LE): " << to_binary_string_le() << std::endl;
}

void InOutData128::print_detailed(const char *name) const {
	std::cout << "\n=== " << name << " ===" << std::endl;

	// 1. Bytes in memory
	std::cout << "Memory layout (little-endian):" << std::endl;
	std::cout << "  Bytes: ";
	for (int i = 0; i < num_bytes; i++) {
		std::cout << "[" << std::setw(2) << i << "]=0x" << std::hex
					<< std::setw(2) << std::setfill('0')
					<< static_cast<int>(bytes[i]) << std::dec;
		if (i < num_bytes - 1) {
			std::cout << " ";
		}
		if ((i + 1) % 4 == 0 && i < num_bytes - 1) {
			std::cout << std::endl << "         ";
		}
	}
	std::cout << std::endl;

	// 2. 32-bit words in memory
	std::cout << "32-bit words in memory:" << std::endl;
	for (int i = 0; i < num_words; i++) {
		std::cout << "  word[" << i << "] = 0x" << std::hex << std::setw(8)
					<< std::setfill('0') << get_word_little_endian(i) << std::dec
					<< "  (BE: 0x" << std::hex << std::setw(8)
					<< get_word_big_endian(i) << ")" << std::dec << std::endl;
	}

	// 3. Various string representations
	std::cout << "\nString representations:" << std::endl;
	std::cout << "  Hex BE:      "
				<< to_hex_string(OutputFormat::HEX_BIG_ENDIAN) << std::endl;
	std::cout << "  Hex LE:      "
				<< to_hex_string(OutputFormat::HEX_LITTLE_ENDIAN) << std::endl;
	std::cout << "  Binary BE:   "
				<< to_binary_string(OutputFormat::BINARY_BIG_ENDIAN) << std::endl;
	std::cout << "  Binary LE:   "
				<< to_binary_string(OutputFormat::BINARY_LITTLE_ENDIAN)
				<< std::endl;
}

// -------------------------------------------------
// String representations
// -------------------------------------------------
std::string InOutData128::to_hex_string(OutputFormat format) const {
	std::stringstream ss;

	switch (format) {
		case OutputFormat::HEX_BIG_ENDIAN:
			ss << "0x";
			for (int i = num_bytes - 1; i >= 0; i--) {
				ss << std::hex << std::setw(2) << std::setfill('0')
					<< static_cast<int>(bytes[i]);
			}
			break;

		case OutputFormat::HEX_LITTLE_ENDIAN:
			// Little-endian: least significant byte first (as in memory)
			ss << "0x";
			for (int i = 0; i < num_bytes; i++) {
				ss << std::hex << std::setw(2) << std::setfill('0')
					<< static_cast<int>(bytes[i]);
			}
			break;

		default:
			// Big-endian by default
			ss << "0x";
			for (int i = num_bytes - 1; i >= 0; i--) {
				ss << std::hex << std::setw(2) << std::setfill('0')
					<< static_cast<int>(bytes[i]);
			}
			break;
	}

	return ss.str();
}

std::string InOutData128::to_hex_string_le() const {
	std::stringstream ss;
	ss << "0x";
	// Little-endian representation
	for (int i = 0; i < num_bytes; i++) {
		ss << std::hex << std::setw(2) << std::setfill('0')
			<< static_cast<int>(bytes[i]);
	}
	return ss.str();
}

std::string InOutData128::to_binary_string(OutputFormat format) const {
	std::stringstream ss;

	switch (format) {
		case OutputFormat::BINARY_BIG_ENDIAN:
			// Big-endian: from most to least significant byte
			for (int i = num_bytes - 1; i >= 0; i--) {
				ss << std::bitset<8>(bytes[i]);
				if (i > 0) {
					ss << " ";
				}
			}
			break;

		case OutputFormat::BINARY_LITTLE_ENDIAN:
			// Little-endian: from least to most significant byte
			for (int i = 0; i < num_bytes; i++) {
				ss << std::bitset<8>(bytes[i]);
				if (i < num_bytes - 1) {
					ss << " ";
				}
			}
			break;

		default:
			// Big-endian by default
			for (int i = num_bytes - 1; i >= 0; i--) {
				ss << std::bitset<8>(bytes[i]);
				if (i > 0) {
					ss << " ";
				}
			}
			break;
	}

	return ss.str();
}

std::string InOutData128::to_binary_string_le() const {
	std::stringstream ss;
	// Little-endian representation
	for (int i = 0; i < num_bytes; i++) {
		ss << std::bitset<8>(bytes[i]);
		if (i < num_bytes - 1) {
			ss << " ";
		}
	}
	return ss.str();
}

// -------------------------------------------------
// Access operators
// -------------------------------------------------
uint32_t InOutData128::operator[](int index) const {
	if (index >= 0 && index < num_words) {
		return words[index];
	}
	cerr << RED << CROSS_MARK << __FUNCTION__ << " index error !\t" << NORMAL
			<< endl;
	return 0;
}

uint32_t &InOutData128::operator[](int index) {
	if (index >= 0 && index < num_words) {
		return words[index];
	}
	throw std::out_of_range("InOutData128::operator[]: index " +
							std::to_string(index) + " out of range 0-" +
							std::to_string(num_words - 1));
}

// -------------------------------------------------
// Convenience methods
// -------------------------------------------------
void InOutData128::set_word(int index, uint32_t value) {
	if (index >= 0 && index < num_words) {
		words[index] = value;
	} else {
		cerr << RED << CROSS_MARK << __FUNCTION__ << " index error !\t"
				<< NORMAL << endl;
	}
}

uint32_t InOutData128::get_word(int index) const {
	if (index >= 0 && index < num_words) {
		return words[index];
	}
	cerr << RED << CROSS_MARK << __FUNCTION__ << " index error !\t" << NORMAL
			<< endl;
	return 0;
}

void InOutData128::set_fields(uint32_t word0, uint32_t word1, uint32_t word2,
								uint32_t word3) {
	w0 = word0;
	w1 = word1;
	w2 = word2;
	w3 = word3;
}

// -------------------------------------------------
// Explicit byte-order methods
// -------------------------------------------------

uint32_t InOutData128::get_word_big_endian(int index) const {
	if (index < 0 || index >= num_words) {
		cerr << RED << CROSS_MARK << __FUNCTION__ << " index error !\t"
				<< NORMAL << endl;
		return 0;
	}
	return swap_endian(words[index]);
}

void InOutData128::set_word_big_endian(int index, uint32_t value) {
	if (index >= 0 && index < num_words) {
		words[index] = swap_endian(value);
	} else {
		cerr << RED << CROSS_MARK << __FUNCTION__ << " index error !\t"
				<< NORMAL << endl;
	}
}

uint32_t InOutData128::get_word_little_endian(int index) const {
	if (index < 0 || index >= num_words) {
		cerr << "get_word_little_endian: Index error !" << endl;
		return 0;
	}
	return words[index];
}

void InOutData128::set_word_little_endian(int index, uint32_t value) {
	if (index >= 0 && index < num_words) {
		words[index] = value;
	} else {
		cerr << RED << CROSS_MARK << __FUNCTION__ << " index error !\t"
				<< NORMAL << endl;
	}
}

uint32_t InOutData128::bytes_to_word_big_endian(const uint8_t *bytes) {
	return (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
}

uint32_t InOutData128::bytes_to_word_little_endian(const uint8_t *bytes) {
	return bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
}

// -------------------------------------------------
// Conversion to arrays
// -------------------------------------------------
std::array<uint32_t, 4> InOutData128::to_array() const {
	std::array<uint32_t, num_words> arr;
	for (int i = 0; i < num_words; i++) {
		arr[i] = words[i];
	}
	return arr;
}

void InOutData128::from_array(const std::array<uint32_t, num_words> &arr) {
	for (int i = 0; i < num_words; i++) {
		words[i] = arr[i];
	}
}

std::array<uint32_t, 4> InOutData128::to_array_big_endian() const {
	std::array<uint32_t, num_words> arr;
	for (int i = 0; i < num_words; i++) {
		arr[i] = get_word_big_endian(i);
	}
	return arr;
}

void InOutData128::from_array_big_endian(
	const std::array<uint32_t, num_words> &arr) {
	for (int i = 0; i < num_words; i++) {
		set_word_big_endian(i, arr[i]);
	}
}

// -------------------------------------------------
// Binary operators
// -------------------------------------------------
InOutData128 InOutData128::operator&(const InOutData128 &other) const {
	InOutData128 result;
	for (int i = 0; i < num_words; i++) {
		result.words[i] = words[i] & other.words[i];
	}
	return result;
}

InOutData128 InOutData128::operator|(const InOutData128 &other) const {
	InOutData128 result;
	for (int i = 0; i < num_words; i++) {
		result.words[i] = words[i] | other.words[i];
	}
	return result;
}

InOutData128 InOutData128::operator^(const InOutData128 &other) const {
	InOutData128 result;
	for (int i = 0; i < num_words; i++) {
		result.words[i] = words[i] ^ other.words[i];
	}
	return result;
}

InOutData128 InOutData128::operator~() const {
	InOutData128 result;
	for (int i = 0; i < num_words; i++) {
		result.words[i] = ~words[i];
	}
	return result;
}

InOutData128 &InOutData128::operator&=(const InOutData128 &other) {
	for (int i = 0; i < num_words; i++) {
		words[i] &= other.words[i];
	}
	return *this;
}

InOutData128 &InOutData128::operator|=(const InOutData128 &other) {
	for (int i = 0; i < num_words; i++) {
		words[i] |= other.words[i];
	}
	return *this;
}

InOutData128 &InOutData128::operator^=(const InOutData128 &other) {
	for (int i = 0; i < num_words; i++) {
		words[i] ^= other.words[i];
	}
	return *this;
}

// -------------------------------------------------
// Comparison
// -------------------------------------------------
bool InOutData128::operator==(const InOutData128 &other) const {
	return std::memcmp(bytes, other.bytes, num_bytes) == 0;
}

bool InOutData128::operator!=(const InOutData128 &other) const {
	return !(*this == other);
}

// -------------------------------------------------
// Static creation methods
// -------------------------------------------------
InOutData128 InOutData128::create_from_vector(const std::vector<uint8_t> &vec,
												size_t start_idx,
												InitOrder order) {
	InOutData128 result;

	// Check that the vector holds enough data
	if (vec.size() - start_idx < num_bytes) {
		throw std::out_of_range(
			"Not enough data in vector to create InOutData128");
	}

	if (order == InitOrder::LITTLE_ENDIAN_IN) {
		result.from_vector_little_endian(vec, start_idx);
	} else {
		result.from_vector_big_endian(vec, start_idx);
	}
	return result;
}

InOutData128 InOutData128::from_vector_le(const std::vector<uint8_t> &vec,
											size_t start_idx) {
	InOutData128 result;
	result.from_vector_little_endian(vec, start_idx);
	return result;
}

InOutData128 InOutData128::from_vector_be(const std::vector<uint8_t> &vec,
											size_t start_idx) {
	InOutData128 result;
	result.from_vector_big_endian(vec, start_idx);
	return result;
}

InOutData128 InOutData128::from_hex_string(const std::string &hex_str) {
	InOutData128 result;

	if (hex_str.empty()) {
		return result;
	}

	std::string clean_str = hex_str;

	// Strip the prefix
	if (hex_str.size() >= 2 && hex_str.compare(0, 2, "0x") == 0) {
		clean_str = hex_str.substr(2);
	} else if (hex_str.size() >= 2 && hex_str.compare(0, 2, "0X") == 0) {
		clean_str = hex_str.substr(2);
	}

	constexpr size_t NUM_BYTES = 16;
	constexpr size_t HEX_CHARS_NEEDED = NUM_BYTES * 2;

	// Pad/trim the string
	if (clean_str.length() < HEX_CHARS_NEEDED) {
		clean_str =
			std::string(HEX_CHARS_NEEDED - clean_str.length(), '0') + clean_str;
	} else if (clean_str.length() > HEX_CHARS_NEEDED) {
		clean_str = clean_str.substr(clean_str.length() - HEX_CHARS_NEEDED);
	}

	// Hex string: "00112233445566778899AABBCCDDEEFF"
	// We want: `word0` = FF EE DD CC (first 4 bytes in memory)
	// So: the last 8 hex string characters -> the first 4 bytes in memory

	// Fill the bytes in reverse order
	for (size_t i = 0; i < NUM_BYTES; i++) {
		// Take from the end of the hex string for the first memory bytes
		size_t hex_pos = (NUM_BYTES - 1 - i) * 2;
		std::string byte_str = clean_str.substr(hex_pos, 2);

		try {
			result.bytes[i] =
				static_cast<uint8_t>(std::stoul(byte_str, nullptr, 16));
		} catch (...) {
			result.bytes[i] = 0;
		}
	}

	return result;
}

// And the `for_hex_string_le` method (when hex is already little-endian)
InOutData128 InOutData128::from_hex_string_le(const std::string &hex_str) {
	InOutData128 result;

	if (hex_str.empty()) {
		return result;
	}

	std::string clean_str = hex_str;

	if (hex_str.size() >= 2 && hex_str.compare(0, 2, "0x") == 0) {
		clean_str = hex_str.substr(2);
	} else if (hex_str.size() >= 2 && hex_str.compare(0, 2, "0X") == 0) {
		clean_str = hex_str.substr(2);
	}

	constexpr size_t NUM_BYTES = 16;
	constexpr size_t HEX_CHARS_NEEDED = NUM_BYTES * 2;

	if (clean_str.length() < HEX_CHARS_NEEDED) {
		clean_str =
			std::string(HEX_CHARS_NEEDED - clean_str.length(), '0') + clean_str;
	} else if (clean_str.length() > HEX_CHARS_NEEDED) {
		clean_str = clean_str.substr(clean_str.length() - HEX_CHARS_NEEDED);
	}

	// Little-endian hex: first byte in the string = `byte[0]` in memory
	for (size_t i = 0; i < NUM_BYTES; i++) {
		size_t hex_pos = i * 2;
		std::string byte_str = clean_str.substr(hex_pos, 2);

		try {
			result.bytes[i] =
				static_cast<uint8_t>(std::stoul(byte_str, nullptr, 16));
		} catch (...) {
			result.bytes[i] = 0;
		}
	}

	return result;
}

InOutData128 InOutData128::from_words(uint32_t w0, uint32_t w1, uint32_t w2,
										uint32_t w3) {
	return InOutData128(w0, w1, w2, w3);
}

// -------------------------------------------------
// Static factory methods with explicit byte order
// -------------------------------------------------

InOutData128 InOutData128::from_words_little_endian(uint32_t w0, uint32_t w1,
													uint32_t w2, uint32_t w3) {
	InOutData128 result;
	result.set_word_little_endian(0, w0);
	result.set_word_little_endian(1, w1);
	result.set_word_little_endian(2, w2);
	result.set_word_little_endian(3, w3);
	return result;
}

InOutData128 InOutData128::from_words_big_endian(uint32_t w0, uint32_t w1,
													uint32_t w2, uint32_t w3) {
	InOutData128 result;
	result.set_word_big_endian(0, w0);
	result.set_word_big_endian(1, w1);
	result.set_word_big_endian(2, w2);
	result.set_word_big_endian(3, w3);
	return result;
}

InOutData128 InOutData128::from_bytes(const uint8_t *bytes, size_t len) {
	return InOutData128(bytes, len);
}

InOutData128 InOutData128::from_bytes_big_endian(const uint8_t *bytes,
													size_t len) {
	InOutData128 result;
	result.set_from_bytes_big_endian(bytes, len);
	return result;
}

// -------------------------------------------------
// Byte order utilities
// -------------------------------------------------
uint32_t InOutData128::swap_endian(uint32_t value) {
	return ((value & 0x000000FF) << 24) | ((value & 0x0000FF00) << 8) |
			((value & 0x00FF0000) >> 8) | ((value & 0xFF000000) >> 24);
}

uint64_t InOutData128::swap_endian(uint64_t value) {
	return ((value & 0x00000000000000FFULL) << 56) |
			((value & 0x000000000000FF00ULL) << 40) |
			((value & 0x0000000000FF0000ULL) << 24) |
			((value & 0x00000000FF000000ULL) << 8) |
			((value & 0x000000FF00000000ULL) >> 8) |
			((value & 0x0000FF0000000000ULL) >> 24) |
			((value & 0x00FF000000000000ULL) >> 40) |
			((value & 0xFF00000000000000ULL) >> 56);
}

// -------------------------------------------------
// For Verilator
// -------------------------------------------------

bool InOutData128::check_vector_bounds(const std::vector<uint8_t> &vec,
										size_t start_idx,
										const char *method_name) const {
	if (vec.empty()) {
		throw std::invalid_argument(std::string(method_name) +
									": Cannot initialize from empty vector");
	}

	if (start_idx >= vec.size()) {
		throw std::out_of_range(std::string(method_name) + ": start_idx (" +
								std::to_string(start_idx) +
								") exceeds vector size (" +
								std::to_string(vec.size()) + ")");
	}

	if (vec.size() - start_idx < static_cast<size_t>(num_bytes)) {
		throw std::length_error(
			std::string(method_name) + ": Vector has insufficient data (" +
			std::to_string(vec.size() - start_idx) + " bytes) to fill " +
			std::to_string(num_bytes) + " bytes");
	}
	return true;
}
