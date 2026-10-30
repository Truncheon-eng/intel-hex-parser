#include <algorithm>
#include "InOutData112.hpp"

// Static variable initialization
InOutData112::OutputFormat InOutData112::default_output_format =
	InOutData112::OutputFormat::HEX_BIG_ENDIAN;

// -------------------------------------------------
// Constructors
// -------------------------------------------------
InOutData112::InOutData112() { clear(); }

InOutData112::InOutData112(const uint32_t data[4]) {
	for (int i = 0; i < 3; i++) {
		words[i] = data[i];
	}
	w3 = data[3] & MASK_LAST_WORD;
}

InOutData112::InOutData112(uint32_t word0, uint32_t word1, uint32_t word2,
							uint32_t word3)
	: w0(word0), w1(word1), w2(word2), w3(word3 & MASK_LAST_WORD) {}

InOutData112::InOutData112(const uint8_t *data, size_t len, InitOrder order) {
	if (order == InitOrder::LITTLE_ENDIAN_IN) {
		set_from_bytes_little_endian(data, len);
	} else {
		set_from_bytes_big_endian(data, len);
	}
	apply_mask_internal();
}

InOutData112::InOutData112(const std::vector<uint8_t> &data, InitOrder order) {
	if (order == InitOrder::LITTLE_ENDIAN_IN) {
		from_vector_little_endian(data);
	} else {
		from_vector_big_endian(data);
	}
	apply_mask_internal();
}

InOutData112::InOutData112(const InOutData112 &other) {
	std::memcpy(bytes, other.bytes, NUM_BYTES);
	apply_mask_internal();
}

InOutData112::InOutData112(InOutData112 &&other) noexcept {
	std::memcpy(bytes, other.bytes, NUM_BYTES);
	apply_mask_internal();
	other.clear();
}

// -------------------------------------------------
// Assignment operators
// -------------------------------------------------
InOutData112 &InOutData112::operator=(const InOutData112 &other) {
	if (this != &other) {
		std::memcpy(bytes, other.bytes, NUM_BYTES);
		apply_mask_internal();
	}
	return *this;
}

InOutData112 &InOutData112::operator=(InOutData112 &&other) noexcept {
	if (this != &other) {
		std::memcpy(bytes, other.bytes, NUM_BYTES);
		apply_mask_internal();
		other.clear();
	}
	return *this;
}

// -------------------------------------------------
// Basic methods
// -------------------------------------------------
void InOutData112::clear() { std::memset(words, 0, sizeof(words)); }

void InOutData112::apply_mask_internal() { w3 &= MASK_LAST_WORD; }

void InOutData112::apply_mask() { apply_mask_internal(); }

bool InOutData112::is_masked_correctly() const {
	return (w3 & ~MASK_LAST_WORD) == 0;
}

uint8_t InOutData112::get_byte(int index) const {
	if (index < 0 || index >= NUM_BYTES) {
		cerr << RED << CROSS_MARK << __FUNCTION__
				<< " byte index error! Index=" << index << ", valid range: 0-"
				<< (NUM_BYTES - 1) << NORMAL << endl;
		return 0;
	}
	return bytes[index];
}

void InOutData112::set_byte(int index, uint8_t value) {
	if (index < 0 || index >= NUM_BYTES) {
		cerr << RED << CROSS_MARK << __FUNCTION__
				<< " byte index error! Index=" << index << ", valid range: 0-"
				<< (NUM_BYTES - 1) << NORMAL << endl;
		return;
	}
	bytes[index] = value;

	// Apply the mask if byte 12 or 13 changed
	if (index >= 12) {
		apply_mask_internal();
	}
}

uint32_t InOutData112::get_word(int index) const {
	if (index < 0 || index >= NUM_WORDS) {
		cerr << RED << CROSS_MARK << __FUNCTION__
				<< " word index error! Index=" << index << ", valid range: 0-"
				<< (NUM_WORDS - 1) << NORMAL << endl;
		return 0;
	}

	if (index == 3) {
		return w3 & MASK_LAST_WORD;
	}
	return words[index];
}

void InOutData112::set_word(int index, uint32_t value) {
	if (index < 0 || index >= NUM_WORDS) {
		cerr << RED << CROSS_MARK << __FUNCTION__
				<< " word index error! Index=" << index << ", valid range: 0-"
				<< (NUM_WORDS - 1) << NORMAL << endl;
		return;
	}

	if (index == 3) {
		w3 = value & MASK_LAST_WORD;
	} else {
		words[index] = value;
	}
}

uint32_t InOutData112::get_word_little_endian(int index) const {
	return get_word(index);
}

uint32_t InOutData112::get_word_big_endian(int index) const {
	if (index >= 0 && index < NUM_WORDS) {
		if (index == 3) {
			return swap_endian(w3 & MASK_LAST_WORD);
		}
		return swap_endian(words[index]);
	}
	return 0;
}

void InOutData112::set_word_little_endian(int index, uint32_t value) {
	set_word(index, value);
}

void InOutData112::set_word_big_endian(int index, uint32_t value) {
	if (index >= 0 && index < NUM_WORDS) {
		if (index == 3) {
			w3 = swap_endian(value) & MASK_LAST_WORD;
		} else {
			words[index] = swap_endian(value);
		}
	}
}

void InOutData112::set_all_words(uint32_t word0, uint32_t word1, uint32_t word2,
									uint32_t word3) {
	w0 = word0;
	w1 = word1;
	w2 = word2;
	w3 = word3 & MASK_LAST_WORD;
}

// -------------------------------------------------
// Set-from-byte-array methods
// -------------------------------------------------
void InOutData112::set_from_bytes(const uint8_t *data, size_t len,
									size_t offset) {
	set_from_bytes_little_endian(data, len, offset);
}

void InOutData112::set_from_bytes_little_endian(const uint8_t *data, size_t len,
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

		// Apply the mask if the last 2 bytes were touched
		if (offset + copy_len > 12) {
			apply_mask_internal();
		}
	}
}

void InOutData112::set_from_bytes_big_endian(const uint8_t *data, size_t len,
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

		// Apply the mask if the last 2 bytes were touched
		if (offset + copy_len > 12) {
			apply_mask_internal();
		}
	}
}

// -------------------------------------------------
// Vector methods
// -------------------------------------------------
void InOutData112::from_vector(const std::vector<uint8_t> &vec,
								size_t start_idx) {
	from_vector_little_endian(vec, start_idx);
}

void InOutData112::from_vector_little_endian(const std::vector<uint8_t> &vec,
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

	apply_mask_internal();
}

void InOutData112::from_vector_big_endian(const std::vector<uint8_t> &vec,
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

	apply_mask_internal();
}

std::vector<uint8_t> InOutData112::to_vector() const {
	return to_vector_little_endian();
}

std::vector<uint8_t> InOutData112::to_vector_little_endian() const {
	return std::vector<uint8_t>(bytes, bytes + NUM_BYTES);
}

std::vector<uint8_t> InOutData112::to_vector_big_endian() const {
	std::vector<uint8_t> result(NUM_BYTES);
	for (size_t i = 0; i < NUM_BYTES; i++) {
		result[NUM_BYTES - 1 - i] = bytes[i];
	}
	return result;
}

// -------------------------------------------------
// Bitwise operations
// -------------------------------------------------
bool InOutData112::get_bit(int bit_index) const {
	if (bit_index < 0 || bit_index >= TOTAL_BITS) {
		cerr << RED << CROSS_MARK << __FUNCTION__
				<< " bit index error! Index=" << bit_index << ", valid range: 0-"
				<< (TOTAL_BITS - 1) << NORMAL << endl;
		return false;
	}

	int byte_idx = bit_index / 8;
	int bit_in_byte = bit_index % 8;
	return (bytes[byte_idx] >> bit_in_byte) & 0x01;
}

void InOutData112::set_bit(int bit_index, bool value) {
	if (bit_index < 0 || bit_index >= TOTAL_BITS) {
		cerr << RED << CROSS_MARK << __FUNCTION__
				<< " bit index error! Index=" << bit_index << ", valid range: 0-"
				<< (TOTAL_BITS - 1) << NORMAL << endl;
		return;
	}

	int byte_idx = bit_index / 8;
	int bit_in_byte = bit_index % 8;

	if (value) {
		bytes[byte_idx] |= (1u << bit_in_byte);
	} else {
		bytes[byte_idx] &= ~(1u << bit_in_byte);
	}

	// Apply the mask if the bit lies in the last 16 bits
	if (bit_index >= 96) {
		apply_mask_internal();
	}
}

// -------------------------------------------------
// Output methods
// -------------------------------------------------
void InOutData112::set_default_output_format(OutputFormat format) {
	default_output_format = format;
}

void InOutData112::print(const char *name) const {
	std::cout << name
				<< " InOutData112 (112 bits, LITTLE-ENDIAN):" << std::endl;

	std::cout << "  Memory words: { ";
	for (int i = 0; i < NUM_WORDS; i++) {
		std::cout << "0x" << std::hex << std::setw(8) << std::setfill('0')
					<< get_word(i);
		if (i < 3) {
			std::cout << ", ";
		}
	}
	std::cout << " }" << std::dec << std::endl;
}

void InOutData112::print_bytes(const char *name) const {
	std::cout << name << " Bytes (little-endian):" << std::endl;
	std::cout << "  ";
	for (int i = 0; i < NUM_BYTES; i++) {
		std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0')
					<< static_cast<int>(bytes[i]);
		if (i < NUM_BYTES - 1) {
			std::cout << " ";
			if ((i + 1) % 8 == 0) {
				std::cout << std::endl << "  ";
			}
		}
	}
	std::cout << std::dec << std::endl;
}

void InOutData112::print_words_and_bytes(const char *name) const {
	std::cout << name << " Words and bytes:" << std::endl;
	for (int i = 0; i < NUM_WORDS; i++) {
		std::cout << "  w" << i << " = 0x" << std::hex << std::setw(8)
					<< std::setfill('0') << get_word(i) << std::dec;

		if (i < 3) {
			std::cout << "  bytes[" << (i * 4) << "-" << (i * 4 + 3) << "] = ";
			for (int j = 0; j < 4; j++) {
				std::cout << "0x" << std::hex << std::setw(2)
							<< std::setfill('0')
							<< static_cast<int>(bytes[i * 4 + j]);
				if (j < 3) {
					std::cout << " ";
				}
			}
		} else {
			std::cout << "  bytes[12-13] = 0x" << std::hex << std::setw(2)
						<< std::setfill('0') << static_cast<int>(bytes[12])
						<< " 0x" << std::setw(2) << static_cast<int>(bytes[13]);
		}
		std::cout << std::dec << std::endl;
	}
}

void InOutData112::print_detailed(const char *name) const {
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

void InOutData112::print_raw_memory(const char *name) const {
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
		if ((i + 1) % 8 == 0 && i < NUM_BYTES - 1) {
			std::cout << std::endl << "                     ";
		}
	}
	std::cout << std::dec << std::endl;
}

std::string InOutData112::to_hex_string(OutputFormat format) const {
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

std::string InOutData112::to_hex_string() const {
	return to_hex_string(default_output_format);
}

std::string InOutData112::to_binary_string(OutputFormat format) const {
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

std::string InOutData112::to_binary_string() const {
	return to_binary_string(
		(default_output_format == OutputFormat::HEX_BIG_ENDIAN ||
			default_output_format == OutputFormat::HEX_LITTLE_ENDIAN)
			? OutputFormat::BINARY_BIG_ENDIAN
			: default_output_format);
}

std::string InOutData112::to_raw_string(bool as_hex) const {
	std::stringstream ss;
	if (as_hex) {
		ss << std::hex << std::setfill('0');
		for (int i = 0; i < NUM_BYTES; i++) {
			ss << std::setw(2) << static_cast<int>(bytes[i]);
			if (i < NUM_BYTES - 1) {
				ss << " ";
			}
			if ((i + 1) % 8 == 0 && i < NUM_BYTES - 1) {
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

void InOutData112::print_hex(OutputFormat format, const char *name) const {
	std::cout << name;
	if (strlen(name) > 0) {
		std::cout << ": ";
	}
	std::cout << to_hex_string(format) << std::endl;
}

void InOutData112::print_hex(const char *name) const {
	print_hex(default_output_format, name);
}

void InOutData112::print_binary(OutputFormat format, const char *name) const {
	std::cout << name;
	if (strlen(name) > 0) {
		std::cout << ": ";
	}
	std::cout << to_binary_string(format) << std::endl;
}

void InOutData112::print_binary(const char *name) const {
	print_binary((default_output_format == OutputFormat::HEX_BIG_ENDIAN ||
					default_output_format == OutputFormat::HEX_LITTLE_ENDIAN)
						? OutputFormat::BINARY_BIG_ENDIAN
						: default_output_format,
					name);
}

// -------------------------------------------------
// Conversion to arrays
// -------------------------------------------------
std::array<uint32_t, 4> InOutData112::to_array() const {
	std::array<uint32_t, 4> arr;
	arr[0] = w0;
	arr[1] = w1;
	arr[2] = w2;
	arr[3] = w3 & MASK_LAST_WORD;
	return arr;
}

void InOutData112::from_array(const std::array<uint32_t, 4> &arr) {
	w0 = arr[0];
	w1 = arr[1];
	w2 = arr[2];
	w3 = arr[3] & MASK_LAST_WORD;
}

// -------------------------------------------------
// Access operators
// -------------------------------------------------
uint32_t InOutData112::operator[](int index) const {
	if (index >= 0 && index < NUM_WORDS) {
		return get_word(index);
	}
	cerr << RED << CROSS_MARK << __FUNCTION__ << " index error !\t" << NORMAL
			<< endl;
	return 0;
}

uint32_t &InOutData112::operator[](int index) {
	if (index >= 0 && index < NUM_WORDS) {
		if (index == 3) {
			throw std::out_of_range(
				"InOutData112::operator[]: index 3 is masked (16-bit); "
				"use get_word(3)/set_word(3, v) instead");
		}
		return words[index];
	}
	throw std::out_of_range("InOutData112::operator[]: index " +
							std::to_string(index) + " out of range 0-" +
							std::to_string(NUM_WORDS - 1));
}

// -------------------------------------------------
// Binary operators
// -------------------------------------------------
InOutData112 InOutData112::operator&(const InOutData112 &other) const {
	InOutData112 result;
	for (int i = 0; i < 3; i++) {
		result.words[i] = words[i] & other.words[i];
	}
	result.w3 = (w3 & other.w3) & MASK_LAST_WORD;
	return result;
}

InOutData112 InOutData112::operator|(const InOutData112 &other) const {
	InOutData112 result;
	for (int i = 0; i < 3; i++) {
		result.words[i] = words[i] | other.words[i];
	}
	result.w3 = (w3 | other.w3) & MASK_LAST_WORD;
	return result;
}

InOutData112 InOutData112::operator^(const InOutData112 &other) const {
	InOutData112 result;
	for (int i = 0; i < 3; i++) {
		result.words[i] = words[i] ^ other.words[i];
	}
	result.w3 = (w3 ^ other.w3) & MASK_LAST_WORD;
	return result;
}

InOutData112 InOutData112::operator~() const {
	InOutData112 result;
	for (int i = 0; i < 3; i++) {
		result.words[i] = ~words[i];
	}
	result.w3 = (~w3) & MASK_LAST_WORD;
	return result;
}

InOutData112 &InOutData112::operator&=(const InOutData112 &other) {
	for (int i = 0; i < 3; i++) {
		words[i] &= other.words[i];
	}
	w3 &= other.w3;
	apply_mask_internal();
	return *this;
}

InOutData112 &InOutData112::operator|=(const InOutData112 &other) {
	for (int i = 0; i < 3; i++) {
		words[i] |= other.words[i];
	}
	w3 |= other.w3;
	apply_mask_internal();
	return *this;
}

InOutData112 &InOutData112::operator^=(const InOutData112 &other) {
	for (int i = 0; i < 3; i++) {
		words[i] ^= other.words[i];
	}
	w3 ^= other.w3;
	apply_mask_internal();
	return *this;
}

// -------------------------------------------------
// Comparison
// -------------------------------------------------
bool InOutData112::operator==(const InOutData112 &other) const {
	return std::memcmp(bytes, other.bytes, NUM_BYTES) == 0 &&
			(w3 & MASK_LAST_WORD) == (other.w3 & MASK_LAST_WORD);
}

bool InOutData112::operator!=(const InOutData112 &other) const {
	return !(*this == other);
}

// Ordering operators
bool InOutData112::operator<(const InOutData112 &other) const {
	return std::memcmp(bytes, other.bytes, NUM_BYTES) < 0;
}

bool InOutData112::operator>(const InOutData112 &other) const {
	return std::memcmp(bytes, other.bytes, NUM_BYTES) > 0;
}

bool InOutData112::operator<=(const InOutData112 &other) const {
	return std::memcmp(bytes, other.bytes, NUM_BYTES) <= 0;
}

bool InOutData112::operator>=(const InOutData112 &other) const {
	return std::memcmp(bytes, other.bytes, NUM_BYTES) >= 0;
}

// -------------------------------------------------
// Static factory methods
// -------------------------------------------------
InOutData112 InOutData112::from_hex_string(const std::string &hex_str) {
	InOutData112 result;

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

	result.apply_mask_internal();
	return result;
}

InOutData112 InOutData112::from_words(uint32_t w0, uint32_t w1, uint32_t w2,
										uint32_t w3) {
	return InOutData112(w0, w1, w2, w3);
}

InOutData112 InOutData112::from_words_little_endian(uint32_t w0, uint32_t w1,
													uint32_t w2, uint32_t w3) {
	return InOutData112(w0, w1, w2, w3);
}

InOutData112 InOutData112::from_words_big_endian(uint32_t w0, uint32_t w1,
													uint32_t w2, uint32_t w3) {
	InOutData112 result;
	result.w0 = swap_endian(w0);
	result.w1 = swap_endian(w1);
	result.w2 = swap_endian(w2);
	result.w3 = swap_endian(w3) & MASK_LAST_WORD;
	return result;
}

InOutData112 InOutData112::from_bytes(const uint8_t *bytes, size_t len,
										InitOrder order) {
	if (order == InitOrder::LITTLE_ENDIAN_IN) {
		return from_bytes_little_endian(bytes, len);
	} else {
		return from_bytes_big_endian(bytes, len);
	}
}

InOutData112 InOutData112::from_bytes_little_endian(const uint8_t *bytes,
													size_t len) {
	InOutData112 result;
	result.set_from_bytes_little_endian(bytes, len);
	return result;
}

InOutData112 InOutData112::from_bytes_big_endian(const uint8_t *bytes,
													size_t len) {
	InOutData112 result;
	result.set_from_bytes_big_endian(bytes, len);
	return result;
}

// -------------------------------------------------
// Byte order utilities
// -------------------------------------------------
uint32_t InOutData112::swap_endian(uint32_t value) {
	return ((value & 0x000000FF) << 24) | ((value & 0x0000FF00) << 8) |
			((value & 0x00FF0000) >> 8) | ((value & 0xFF000000) >> 24);
}

uint32_t InOutData112::bytes_to_word_big_endian(const uint8_t *bytes) {
	return (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
}

uint32_t InOutData112::bytes_to_word_little_endian(const uint8_t *bytes) {
	return bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
}

void InOutData112::convert_big_to_little_endian(uint8_t *dest,
												const uint8_t *src,
												size_t len) {
	for (size_t i = 0; i < len; i++) {
		dest[i] = src[len - 1 - i];
	}
}
