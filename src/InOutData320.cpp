// InOutData320.cpp
#include <algorithm>
#include "InOutData320.hpp"

// Static variable initialization
InOutData320::OutputFormat InOutData320::default_output_format =
	InOutData320::OutputFormat::HEX_BIG_ENDIAN;

void InOutData320::set_default_output_format(OutputFormat format) {
	default_output_format = format;
}

// -------------------------------------------------
// Constructors
// -------------------------------------------------
InOutData320::InOutData320() { clear(); }

InOutData320::InOutData320(const uint32_t data[10]) {
	for (int i = 0; i < 10; i++) {
		words[i] = data[i];
	}
}

InOutData320::InOutData320(uint32_t word0, uint32_t word1, uint32_t word2,
							uint32_t word3, uint32_t word4, uint32_t word5,
							uint32_t word6, uint32_t word7, uint32_t word8,
							uint32_t word9)
	: w0(word0), w1(word1), w2(word2), w3(word3), w4(word4), w5(word5),
		w6(word6), w7(word7), w8(word8), w9(word9) {}

// Deprecated!
// InOutData320::InOutData320(const uint8_t* data,
// 						   size_t len) {
// 	set_from_bytes_little_endian(data, len);
// }

// Deprecated!
// InOutData320::InOutData320(const std::vector<uint8_t>& data) {
// 	from_vector_little_endian(data);
// }

// Updated constructor with byte-order selection
InOutData320::InOutData320(const uint8_t *data, size_t len, InitOrder order) {
	if (order == InitOrder::LITTLE_ENDIAN_IN) {
		set_from_bytes_little_endian(data, len);
	} else {
		set_from_bytes_big_endian(data, len);
	}
}

InOutData320::InOutData320(const std::vector<uint8_t> &data, InitOrder order) {
	if (order == InitOrder::LITTLE_ENDIAN_IN) {
		from_vector_little_endian(data);
	} else {
		from_vector_big_endian(data);
	}
}

InOutData320::InOutData320(const InOutData320 &other) {
	std::memcpy(bytes, other.bytes, NUM_BYTES);
}

InOutData320::InOutData320(InOutData320 &&other) noexcept {
	std::memcpy(bytes, other.bytes, NUM_BYTES);
	other.clear();
}

// -------------------------------------------------
// Assignment operators
// -------------------------------------------------
InOutData320 &InOutData320::operator=(const InOutData320 &other) {
	if (this != &other) {
		std::memcpy(bytes, other.bytes, NUM_BYTES);
	}
	return *this;
}

InOutData320 &InOutData320::operator=(InOutData320 &&other) noexcept {
	if (this != &other) {
		std::memcpy(bytes, other.bytes, NUM_BYTES);
		other.clear();
	}
	return *this;
}

// -------------------------------------------------
// Clear and access methods
// -------------------------------------------------

// -------------------------------------------------
// Basic methods
// -------------------------------------------------
void InOutData320::clear() { std::memset(bytes, 0, NUM_BYTES); }

uint8_t InOutData320::get_byte(int index) const {
	if (index < 0 || index >= NUM_BYTES) {
		cerr << RED << CROSS_MARK << __FUNCTION__
				<< " byte index error! Index=" << index << ", valid range: 0-"
				<< (NUM_BYTES - 1) << NORMAL << endl;
		return 0;
	}
	return bytes[index];
}

void InOutData320::set_byte(int index, uint8_t value) {
	if (index < 0 || index >= NUM_BYTES) {
		cerr << RED << CROSS_MARK << __FUNCTION__
				<< " byte index error! Index=" << index << ", valid range: 0-"
				<< (NUM_BYTES - 1) << NORMAL << endl;
		return;
	}
	bytes[index] = value;
}

uint32_t InOutData320::get_word(int index) const {
	if (index < 0 || index >= NUM_WORDS) {
		cerr << RED << CROSS_MARK << __FUNCTION__
				<< " word index error! Index=" << index << ", valid range: 0-"
				<< (NUM_WORDS - 1) << NORMAL << endl;
		return 0;
	}
	return words[index];
}

void InOutData320::set_word(int index, uint32_t value) {
	if (index < 0 || index >= NUM_WORDS) {
		cerr << RED << CROSS_MARK << __FUNCTION__
				<< " word index error! Index=" << index << ", valid range: 0-"
				<< (NUM_WORDS - 1) << NORMAL << endl;
		return;
	}
	words[index] = value;
}

// -------------------------------------------------
// Word access methods with explicit byte order
// -------------------------------------------------
uint32_t InOutData320::get_word_little_endian(int index) const {
	if (index < 0 || index >= NUM_WORDS) {
		cerr << RED << CROSS_MARK << __FUNCTION__ << " index error !\t"
				<< NORMAL << endl;
		return 0;
	}
	return words[index];
}

uint32_t InOutData320::get_word_big_endian(int index) const {
	if (index < 0 || index >= NUM_WORDS) {
		cerr << RED << CROSS_MARK << __FUNCTION__ << " index error !\t"
				<< NORMAL << endl;
		return 0;
	}
	return swap_endian(words[index]);
}

void InOutData320::set_word_little_endian(int index, uint32_t value) {
	if (index >= 0 && index < NUM_WORDS) {
		words[index] = value;
	}
}

void InOutData320::set_word_big_endian(int index, uint32_t value) {
	if (index >= 0 && index < NUM_WORDS) {
		words[index] = swap_endian(value);
	}
}

// -------------------------------------------------
// Set-from-byte-array methods
// -------------------------------------------------
void InOutData320::set_from_bytes(const uint8_t *data, size_t len,
									size_t offset) {
	// Little-endian by default
	set_from_bytes_little_endian(data, len, offset);
}

void InOutData320::set_from_bytes_little_endian(const uint8_t *data, size_t len,
												size_t offset) {
	if (!data || len == 0) {
		return;
	}
	if (offset >= static_cast<size_t>(NUM_BYTES)) {
		return;
	}

	size_t copy_len = std::min(len, static_cast<size_t>(NUM_BYTES) - offset);
	if (copy_len > 0) {
		std::memcpy(bytes + offset, data, copy_len);
	}
}

void InOutData320::set_from_bytes_big_endian(const uint8_t *data, size_t len,
												size_t offset) {
	if (!data || len == 0) {
		return;
	}
	if (offset >= static_cast<size_t>(NUM_BYTES)) {
		return;
	}

	size_t copy_len = std::min(len, static_cast<size_t>(NUM_BYTES) - offset);
	if (copy_len > 0) {
		convert_big_to_little_endian(bytes + offset, data, copy_len);
	}
}

// -------------------------------------------------
// Vector methods
// -------------------------------------------------
// TODO: Add default unload mode

void InOutData320::from_vector(const std::vector<uint8_t> &vec,
								size_t start_idx) {
	from_vector_little_endian(vec, start_idx);
}

void InOutData320::from_vector_little_endian(const std::vector<uint8_t> &vec,
												size_t start_idx) {
	if (vec.empty() || start_idx >= vec.size()) {
		clear();
		return;
	}

	size_t bytes_to_copy =
		std::min(static_cast<size_t>(NUM_BYTES), vec.size() - start_idx);

	std::memcpy(bytes, vec.data() + start_idx, bytes_to_copy);

	if (bytes_to_copy < NUM_BYTES) {
		std::memset(bytes + bytes_to_copy, 0, NUM_BYTES - bytes_to_copy);
	}
}

void InOutData320::from_vector_big_endian(const std::vector<uint8_t> &vec,
											size_t start_idx) {
	if (vec.empty() || start_idx >= vec.size()) {
		clear();
		return;
	}

	size_t bytes_to_copy =
		std::min(static_cast<size_t>(NUM_BYTES), vec.size() - start_idx);

	// Convert big-endian to little-endian
	for (size_t i = 0; i < bytes_to_copy; i++) {
		bytes[bytes_to_copy - 1 - i] = vec[start_idx + i];
	}

	if (bytes_to_copy < NUM_BYTES) {
		std::memset(bytes + bytes_to_copy, 0, NUM_BYTES - bytes_to_copy);
	}
}

std::vector<uint8_t> InOutData320::to_vector() const {
	return to_vector_little_endian();
}

std::vector<uint8_t> InOutData320::to_vector_little_endian() const {
	return std::vector<uint8_t>(bytes, bytes + NUM_BYTES);
}

std::vector<uint8_t> InOutData320::to_vector_big_endian() const {
	std::vector<uint8_t> result(NUM_BYTES);
	for (size_t i = 0; i < NUM_BYTES; i++) {
		result[NUM_BYTES - 1 - i] = bytes[i];
	}
	return result;
}

// -------------------------------------------------
// Bitwise operations
// -------------------------------------------------
bool InOutData320::get_bit(int bit_index) const {
	if (bit_index < 0 || bit_index >= WIDTH) {
		cerr << RED << CROSS_MARK << __FUNCTION__
				<< " bit index error! Index=" << bit_index << ", valid range: 0-"
				<< (WIDTH - 1) << NORMAL << endl;
		return false;
	}

	int byte_idx = bit_index / 8;
	int bit_in_byte = bit_index % 8;
	return (bytes[byte_idx] >> bit_in_byte) & 0x01;
}

void InOutData320::set_bit(int bit_index, bool value) {
	if (bit_index < 0 || bit_index >= WIDTH) {
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
// Output methods (same as before, with added formats)
// -------------------------------------------------
void InOutData320::print(const char *name) const {
	std::cout << name
				<< " InOutData320 (320 bits, LITTLE-ENDIAN):" << std::endl;

	std::cout << "  Memory words: { ";
	for (int i = 0; i < NUM_WORDS; i++) {
		std::cout << "0x" << std::hex << std::setw(8) << std::setfill('0')
					<< words[i];
		if (i < 9) {
			std::cout << ", ";
		}
		if ((i + 1) % 4 == 0 && i < 9) {
			std::cout << std::endl << "                ";
		}
	}
	std::cout << " }" << std::dec << std::endl;
}

void InOutData320::print_bytes(const char *name) const {
	std::cout << name << " Bytes (little-endian):" << std::endl;
	std::cout << "  ";
	for (int i = 0; i < NUM_BYTES; i++) {
		std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0')
					<< static_cast<int>(bytes[i]);
		if (i < NUM_BYTES - 1) {
			std::cout << " ";
			if ((i + 1) % 16 == 0) {
				std::cout << std::endl << "  ";
			} else if ((i + 1) % 8 == 0) {
				std::cout << "  ";
			}
		}
	}
	std::cout << std::dec << std::endl;
}

void InOutData320::print_words_and_bytes(const char *name) const {
	std::cout << name << " Words and bytes:" << std::endl;
	for (int i = 0; i < NUM_WORDS; i++) {
		std::cout << "  w" << i << " = 0x" << std::hex << std::setw(8)
					<< std::setfill('0') << words[i] << std::dec << "  bytes["
					<< (i * 4) << "-" << (i * 4 + 3) << "] = ";
		for (int j = 0; j < 4; j++) {
			std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0')
						<< static_cast<int>(bytes[i * 4 + j]);
			if (j < 3) {
				std::cout << " ";
			}
		}
		std::cout << std::dec << std::endl;
	}
}

void InOutData320::print_detailed(const char *name) const {
	std::cout << "\n=== " << name << " ===" << std::endl;

	std::cout << "Memory layout (little-endian):" << std::endl;
	std::cout << "  Bytes: ";
	for (int i = 0; i < NUM_BYTES; i++) {
		std::cout << "[" << std::setw(2) << i << "]=0x" << std::hex
					<< std::setw(2) << std::setfill('0')
					<< static_cast<int>(bytes[i]) << std::dec;
		if (i < NUM_BYTES - 1) {
			std::cout << " ";
		}
		if ((i + 1) % 8 == 0 && i < NUM_BYTES - 1) {
			std::cout << std::endl << "         ";
		}
	}
	std::cout << std::endl;

	std::cout << "\n32-bit words in memory:" << std::endl;
	for (int i = 0; i < NUM_WORDS; i++) {
		std::cout << "  word[" << i << "] = 0x" << std::hex << std::setw(8)
					<< std::setfill('0') << get_word_little_endian(i) << std::dec
					<< "  (BE: 0x" << std::hex << std::setw(8)
					<< get_word_big_endian(i) << ")" << std::dec << std::endl;
	}

	std::cout << "\nString representations:" << std::endl;
	std::cout << "  Hex BE:      "
				<< to_hex_string(OutputFormat::HEX_BIG_ENDIAN) << std::endl;
	std::cout << "  Hex LE:      "
				<< to_hex_string(OutputFormat::HEX_LITTLE_ENDIAN) << std::endl;
}

void InOutData320::print_raw_memory(const char *name) const {
	std::cout << name;
	if (strlen(name) > 0) {
		std::cout << ": ";
	}

	std::cout << "Raw bytes in memory: ";
	for (int i = 0; i < NUM_BYTES; i++) {
		std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0')
					<< static_cast<int>(bytes[i]);
		if (i < NUM_BYTES - 1) {
			std::cout << " ";
		}
		if ((i + 1) % 16 == 0 && i < NUM_BYTES - 1) {
			std::cout << std::endl << "                     ";
		}
	}
	std::cout << std::dec << std::endl;
}

// -------------------------------------------------
// String representations
// -------------------------------------------------
std::string InOutData320::to_hex_string(OutputFormat format) const {
	std::stringstream ss;

	switch (format) {
		case OutputFormat::HEX_BIG_ENDIAN:
			ss << "0x";
			for (int i = NUM_BYTES - 1; i >= 0; i--) {
				ss << std::hex << std::setw(2) << std::setfill('0')
					<< static_cast<int>(bytes[i]);
			}
			break;

		case OutputFormat::HEX_LITTLE_ENDIAN:
			ss << "0x";
			for (int i = 0; i < NUM_BYTES; i++) {
				ss << std::hex << std::setw(2) << std::setfill('0')
					<< static_cast<int>(bytes[i]);
			}
			break;

		default:
			ss << "0x";
			for (int i = NUM_BYTES - 1; i >= 0; i--) {
				ss << std::hex << std::setw(2) << std::setfill('0')
					<< static_cast<int>(bytes[i]);
			}
			break;
	}

	return ss.str();
}

std::string InOutData320::to_hex_string() const {
	return to_hex_string(default_output_format);
}

std::string InOutData320::to_binary_string(OutputFormat format) const {
	std::stringstream ss;

	switch (format) {
		case OutputFormat::BINARY_BIG_ENDIAN:
			for (int i = NUM_BYTES - 1; i >= 0; i--) {
				ss << std::bitset<8>(bytes[i]);
				if (i > 0) {
					ss << " ";
				}
				if ((NUM_BYTES - i) % 4 == 0 && i > 0) {
					ss << std::endl;
				}
			}
			break;

		case OutputFormat::BINARY_LITTLE_ENDIAN:
			for (int i = 0; i < NUM_BYTES; i++) {
				ss << std::bitset<8>(bytes[i]);
				if (i < NUM_BYTES - 1) {
					ss << " ";
				}
				if ((i + 1) % 4 == 0 && i < NUM_BYTES - 1) {
					ss << std::endl;
				}
			}
			break;

		default:
			for (int i = NUM_BYTES - 1; i >= 0; i--) {
				ss << std::bitset<8>(bytes[i]);
				if (i > 0) {
					ss << " ";
				}
				if ((NUM_BYTES - i) % 4 == 0 && i > 0) {
					ss << std::endl;
				}
			}
			break;
	}

	return ss.str();
}

std::string InOutData320::to_binary_string() const {
	return to_binary_string(
		(default_output_format == OutputFormat::HEX_BIG_ENDIAN ||
			default_output_format == OutputFormat::HEX_LITTLE_ENDIAN)
			? OutputFormat::BINARY_BIG_ENDIAN
			: default_output_format);
}

std::string InOutData320::to_raw_string(bool as_hex) const {
	std::stringstream ss;
	if (as_hex) {
		ss << std::hex << std::setfill('0');
		for (int i = 0; i < NUM_BYTES; i++) {
			ss << std::setw(2) << static_cast<int>(bytes[i]);
			if (i < NUM_BYTES - 1) {
				ss << " ";
			}
			if ((i + 1) % 16 == 0 && i < NUM_BYTES - 1) {
				ss << std::endl;
			}
		}
	} else {
		for (int i = 0; i < NUM_BYTES; i++) {
			ss << std::bitset<8>(bytes[i]);
			if (i < NUM_BYTES - 1) {
				ss << " ";
			}
			if ((i + 1) % 4 == 0 && i < NUM_BYTES - 1) {
				ss << std::endl;
			}
		}
	}
	return ss.str();
}

// -------------------------------------------------
// Access operators
// -------------------------------------------------
uint32_t InOutData320::operator[](int index) const {
	if (index >= 0 && index < NUM_WORDS) {
		return words[index];
	}
	cerr << RED << CROSS_MARK << __FUNCTION__ << " index error !\t" << NORMAL
			<< endl;
	return 0;
}

uint32_t &InOutData320::operator[](int index) {
	if (index >= 0 && index < NUM_WORDS) {
		return words[index];
	}
	throw std::out_of_range("InOutData320::operator[]: index " +
							std::to_string(index) + " out of range 0-" +
							std::to_string(NUM_WORDS - 1));
}

// -------------------------------------------------
// Comparison
// -------------------------------------------------
bool InOutData320::operator==(const InOutData320 &other) const {
	return std::memcmp(bytes, other.bytes, NUM_BYTES) == 0;
}

bool InOutData320::operator!=(const InOutData320 &other) const {
	return !(*this == other);
}

// ... remaining methods (operators, comparison, static methods) ...

// -------------------------------------------------
// Static creation methods
// -------------------------------------------------
InOutData320 InOutData320::from_hex_string(const std::string &hex_str) {
	InOutData320 result;

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

	constexpr size_t HEX_CHARS_NEEDED = NUM_BYTES * 2;

	// Pad/trim the string
	if (clean_str.length() < HEX_CHARS_NEEDED) {
		clean_str =
			std::string(HEX_CHARS_NEEDED - clean_str.length(), '0') + clean_str;
	} else if (clean_str.length() > HEX_CHARS_NEEDED) {
		clean_str = clean_str.substr(clean_str.length() - HEX_CHARS_NEEDED);
	}

	// Hex string in big-endian format (first byte in the string = most
	// significant byte) Convert to little-endian for in-memory storage
	for (size_t i = 0; i < NUM_BYTES; i++) {
		size_t hex_pos = (NUM_BYTES - 1 - i) * 2;  // reversed!
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

InOutData320 InOutData320::from_hex_string_le(const std::string &hex_str) {
	InOutData320 result;

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

	constexpr size_t HEX_CHARS_NEEDED = NUM_BYTES * 2;

	// Pad/trim the string
	if (clean_str.length() < HEX_CHARS_NEEDED) {
		clean_str =
			std::string(HEX_CHARS_NEEDED - clean_str.length(), '0') + clean_str;
	} else if (clean_str.length() > HEX_CHARS_NEEDED) {
		clean_str = clean_str.substr(clean_str.length() - HEX_CHARS_NEEDED);
	}

	// Hex string is ALREADY little-endian (first byte in the string = first
	// byte in memory)
	for (size_t i = 0; i < NUM_BYTES; i++) {
		size_t hex_pos = i * 2;	 // not reversed!
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

InOutData320 InOutData320::from_words(uint32_t w0, uint32_t w1, uint32_t w2,
										uint32_t w3, uint32_t w4, uint32_t w5,
										uint32_t w6, uint32_t w7, uint32_t w8,
										uint32_t w9) {
	return InOutData320(w0, w1, w2, w3, w4, w5, w6, w7, w8, w9);
}

// ... remaining implementation mirrors `InOutData128` ...

// -------------------------------------------------
// Binary operators
// -------------------------------------------------
InOutData320 InOutData320::operator&(const InOutData320 &other) const {
	InOutData320 result;
	for (int i = 0; i < NUM_WORDS; i++) {
		result.words[i] = words[i] & other.words[i];
	}
	return result;
}

InOutData320 InOutData320::operator|(const InOutData320 &other) const {
	InOutData320 result;
	for (int i = 0; i < NUM_WORDS; i++) {
		result.words[i] = words[i] | other.words[i];
	}
	return result;
}

InOutData320 InOutData320::operator^(const InOutData320 &other) const {
	InOutData320 result;
	for (int i = 0; i < NUM_WORDS; i++) {
		result.words[i] = words[i] ^ other.words[i];
	}
	return result;
}

InOutData320 InOutData320::operator~() const {
	InOutData320 result;
	for (int i = 0; i < NUM_WORDS; i++) {
		result.words[i] = ~words[i];
	}
	return result;
}

InOutData320 &InOutData320::operator&=(const InOutData320 &other) {
	for (int i = 0; i < NUM_WORDS; i++) {
		words[i] &= other.words[i];
	}
	return *this;
}

InOutData320 &InOutData320::operator|=(const InOutData320 &other) {
	for (int i = 0; i < NUM_WORDS; i++) {
		words[i] |= other.words[i];
	}
	return *this;
}

InOutData320 &InOutData320::operator^=(const InOutData320 &other) {
	for (int i = 0; i < NUM_WORDS; i++) {
		words[i] ^= other.words[i];
	}
	return *this;
}

// Ordering operators
// For use in `std::map`, `std::set`, etc.
bool InOutData320::operator<(const InOutData320 &other) const {
	return std::memcmp(bytes, other.bytes, NUM_BYTES) < 0;
}

bool InOutData320::operator>(const InOutData320 &other) const {
	return std::memcmp(bytes, other.bytes, NUM_BYTES) > 0;
}

bool InOutData320::operator<=(const InOutData320 &other) const {
	return std::memcmp(bytes, other.bytes, NUM_BYTES) <= 0;
}

bool InOutData320::operator>=(const InOutData320 &other) const {
	return std::memcmp(bytes, other.bytes, NUM_BYTES) >= 0;
}

// -------------------------------------------------
// Static factory methods with explicit byte order
// -------------------------------------------------


// -------------------------------------------------
// Byte order utilities
// -------------------------------------------------
uint32_t InOutData320::swap_endian(uint32_t value) {
	return ((value & 0x000000FF) << 24) | ((value & 0x0000FF00) << 8) |
			((value & 0x00FF0000) >> 8) | ((value & 0xFF000000) >> 24);
}

uint64_t InOutData320::swap_endian(uint64_t value) {
	return ((value & 0x00000000000000FFULL) << 56) |
			((value & 0x000000000000FF00ULL) << 40) |
			((value & 0x0000000000FF0000ULL) << 24) |
			((value & 0x00000000FF000000ULL) << 8) |
			((value & 0x000000FF00000000ULL) >> 8) |
			((value & 0x0000FF0000000000ULL) >> 24) |
			((value & 0x00FF000000000000ULL) >> 40) |
			((value & 0xFF00000000000000ULL) >> 56);
}

uint32_t InOutData320::bytes_to_word_big_endian(const uint8_t *bytes) {
	return (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
}

uint32_t InOutData320::bytes_to_word_little_endian(const uint8_t *bytes) {
	return bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
}

void InOutData320::convert_big_to_little_endian(uint8_t *dest,
												const uint8_t *src,
												size_t len) {
	for (size_t i = 0; i < len; i++) {
		dest[i] = src[len - 1 - i];
	}
}

// -------------------------------------------------
// Static factory methods
// -------------------------------------------------
InOutData320 InOutData320::from_bytes(const uint8_t *bytes, size_t len,
										InitOrder order) {
	InOutData320 result;

	if (order == InitOrder::LITTLE_ENDIAN_IN) {
		result.set_from_bytes_little_endian(bytes, len);
	} else {
		result.set_from_bytes_big_endian(bytes, len);
	}

	return result;
}

// -------------------------------------------------
// Verilator compatibility
// -------------------------------------------------
//
// Byte conversions
//
InOutData320 InOutData320::from_bytes_little_endian(const uint8_t *bytes,
													size_t len) {
	InOutData320 result;
	result.set_from_bytes_little_endian(bytes, len);
	return result;
}

InOutData320 InOutData320::from_bytes_big_endian(const uint8_t *bytes,
													size_t len) {
	InOutData320 result;
	result.set_from_bytes_big_endian(bytes, len);
	return result;
}

// -------------------------------------------------
// Conversion to arrays
// -------------------------------------------------
std::array<uint32_t, 10> InOutData320::to_array() const {
	std::array<uint32_t, 10> arr;
	std::copy(words, words + NUM_WORDS, arr.begin());
	return arr;
}

void InOutData320::from_array(const std::array<uint32_t, 10> &arr) {
	std::copy(arr.begin(), arr.end(), words);
}

// -------------------------------------------------
// Parameterized output methods
// -------------------------------------------------
void InOutData320::print_hex(OutputFormat format, const char *name) const {
	std::cout << name;
	if (strlen(name) > 0) {
		std::cout << ": ";
	}
	std::cout << to_hex_string(format) << std::endl;
}

void InOutData320::print_binary(OutputFormat format, const char *name) const {
	std::cout << name;
	if (strlen(name) > 0) {
		std::cout << ": ";
	}
	std::cout << to_binary_string(format) << std::endl;
}

void InOutData320::print_hex(const char *name) const {
	print_hex(default_output_format, name);
}

void InOutData320::print_binary(const char *name) const {
	print_binary((default_output_format == OutputFormat::HEX_BIG_ENDIAN ||
					default_output_format == OutputFormat::HEX_LITTLE_ENDIAN)
						? OutputFormat::BINARY_BIG_ENDIAN
						: default_output_format,
					name);
}

// Factory methods for words with explicit byte order:
// In the implementation file (`InOutData320.cpp`):
InOutData320 InOutData320::from_words_little_endian(uint32_t w0, uint32_t w1,
													uint32_t w2, uint32_t w3,
													uint32_t w4, uint32_t w5,
													uint32_t w6, uint32_t w7,
													uint32_t w8, uint32_t w9) {
	return InOutData320(w0, w1, w2, w3, w4, w5, w6, w7, w8, w9);
}

InOutData320 InOutData320::from_words_big_endian(uint32_t w0, uint32_t w1,
													uint32_t w2, uint32_t w3,
													uint32_t w4, uint32_t w5,
													uint32_t w6, uint32_t w7,
													uint32_t w8, uint32_t w9) {
	InOutData320 result;
	result.words[0] = swap_endian(w0);
	result.words[1] = swap_endian(w1);
	result.words[2] = swap_endian(w2);
	result.words[3] = swap_endian(w3);
	result.words[4] = swap_endian(w4);
	result.words[5] = swap_endian(w5);
	result.words[6] = swap_endian(w6);
	result.words[7] = swap_endian(w7);
	result.words[8] = swap_endian(w8);
	result.words[9] = swap_endian(w9);
	return result;
}
