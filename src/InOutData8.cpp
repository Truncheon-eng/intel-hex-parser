// InOutData8.cpp
#include <algorithm>
#include "InOutData8.hpp"

// Static variable initialization
InOutData8::OutputFormat InOutData8::default_output_format =
	InOutData8::OutputFormat::HEX_BIG_ENDIAN;

// -------------------------------------------------
// Constructors
// -------------------------------------------------
InOutData8::InOutData8() { clear(); }

InOutData8::InOutData8(const uint8_t data[1]) {
	if (data) {
		bytes[0] = data[0];
	} else {
		clear();
	}
}

InOutData8::InOutData8(uint8_t value) : packed(value) {}

InOutData8::InOutData8(const std::vector<uint8_t> &data, size_t start_idx) {
	if (data.empty() || start_idx >= data.size()) {
		clear();
		return;
	}
	bytes[0] = data[start_idx];
}

InOutData8::InOutData8(const InOutData8 &other) { bytes[0] = other.bytes[0]; }

InOutData8::InOutData8(InOutData8 &&other) noexcept {
	bytes[0] = other.bytes[0];
	other.clear();
}

// -------------------------------------------------
// Assignment operators
// -------------------------------------------------
InOutData8 &InOutData8::operator=(const InOutData8 &other) {
	if (this != &other) {
		bytes[0] = other.bytes[0];
	}
	return *this;
}

InOutData8 &InOutData8::operator=(InOutData8 &&other) noexcept {
	if (this != &other) {
		bytes[0] = other.bytes[0];
		other.clear();
	}
	return *this;
}

// -------------------------------------------------
// Basic methods
// -------------------------------------------------
void InOutData8::clear() { packed = 0; }

uint8_t InOutData8::get_byte() const { return bytes[0]; }

void InOutData8::set_byte(uint8_t value) { bytes[0] = value; }

uint8_t InOutData8::get_packed() const { return packed; }

void InOutData8::set_packed(uint8_t value) { packed = value; }

// -------------------------------------------------
// Set-from-byte-array methods
// -------------------------------------------------
void InOutData8::set_from_bytes(const uint8_t *data, size_t len,
								size_t offset) {
	if (!data || len == 0) {
		return;
	}
	if (offset >= NUM_BYTES) {
		return;
	}

	size_t copy_len = std::min(len, static_cast<size_t>(NUM_BYTES) - offset);
	if (copy_len > 0) {
		bytes[offset] = data[0];
	}
}

// -------------------------------------------------
// Vector methods
// -------------------------------------------------
std::vector<uint8_t> InOutData8::to_vector() const {
	return std::vector<uint8_t>{bytes[0]};
}
// Non-static method implementation
void InOutData8::from_vector(const std::vector<uint8_t> &vec,
								size_t start_idx) {
	if (vec.empty() || start_idx >= vec.size()) {
		clear();
		return;
	}
	bytes[0] = vec[start_idx];
}

// -------------------------------------------------
// Bitwise operations
// -------------------------------------------------
bool InOutData8::get_bit(int bit_index) const {
	if (bit_index < 0 || bit_index >= WIDTH) {
		cerr << "InOutData8::get_bit: Invalid bit index " << bit_index << endl;
		return false;
	}
	return (packed >> bit_index) & 0x01;
}

void InOutData8::set_bit(int bit_index, bool value) {
	if (bit_index < 0 || bit_index >= WIDTH) {
		cerr << "InOutData8::set_bit: Invalid bit index " << bit_index << endl;
		return;
	}

	if (value) {
		packed |= (1u << bit_index);
	} else {
		packed &= ~(1u << bit_index);
	}
}

// -------------------------------------------------
// Output methods
// -------------------------------------------------
void InOutData8::set_default_output_format(OutputFormat format) {
	default_output_format = format;
}

void InOutData8::print(const char *name) const {
	cout << name << " InOutData8 (8 bits):" << endl;
	cout << "  Value: 0x" << std::hex << std::setw(2) << std::setfill('0')
			<< static_cast<int>(packed) << std::dec << endl;
	cout << "  Binary: " << to_binary_string() << endl;
}

void InOutData8::print_bytes(const char *name) const {
	cout << name << " Bytes:" << endl;
	cout << "  [0] = 0x" << std::hex << std::setw(2) << std::setfill('0')
			<< static_cast<int>(bytes[0]) << std::dec << endl;
}

void InOutData8::print_detailed(const char *name) const {
	cout << "\n=== " << name << " ===" << endl;

	cout << "Value: 0x" << std::hex << std::setw(2) << std::setfill('0')
			<< static_cast<int>(packed) << std::dec << endl;

	cout << "Binary: " << to_binary_string() << endl;

	cout << "Bit breakdown:" << endl;
	for (int i = 7; i >= 0; i--) {
		cout << "  bit[" << i << "] = " << get_bit(i);
		if (i == 4) {
			cout << "  (nibble boundary)";
		}
		cout << endl;
	}
}

void InOutData8::print_raw_memory(const char *name) const {
	if (strlen(name) > 0) {
		cout << name << ": ";
	}
	cout << "Raw byte: 0x" << std::hex << std::setw(2) << std::setfill('0')
			<< static_cast<int>(bytes[0]) << std::dec << endl;
}

// -------------------------------------------------
// String representations
// -------------------------------------------------
std::string InOutData8::to_hex_string(OutputFormat /*format*/) const {
	std::stringstream ss;
	ss << "0x" << std::hex << std::setw(2) << std::setfill('0')
		<< static_cast<int>(packed);
	return ss.str();
}

std::string InOutData8::to_hex_string() const {
	return to_hex_string(default_output_format);
}

std::string InOutData8::to_binary_string(OutputFormat format) const {
	std::stringstream ss;
	switch (format) {
		case OutputFormat::BINARY_BIG_ENDIAN:
		case OutputFormat::BINARY_LITTLE_ENDIAN:
			// For 8 bits the byte order does not matter
			ss << std::bitset<8>(packed);
			break;
		default:
			ss << std::bitset<8>(packed);
			break;
	}
	return ss.str();
}

std::string InOutData8::to_binary_string() const {
	return to_binary_string(default_output_format);
}

std::string InOutData8::to_raw_string(bool as_hex) const {
	std::stringstream ss;
	if (as_hex) {
		ss << "0x" << std::hex << std::setw(2) << std::setfill('0')
			<< static_cast<int>(bytes[0]);
	} else {
		ss << std::bitset<8>(bytes[0]);
	}
	return ss.str();
}

std::string InOutData8::to_bin_string() const { return to_binary_string(); }

void InOutData8::print_hex(const char *name) const {
	if (strlen(name) > 0) {
		cout << name << " ";
	}
	cout << "Hex: " << to_hex_string() << endl;
}

void InOutData8::print_hex(OutputFormat format, const char *name) const {
	if (strlen(name) > 0) {
		cout << name << " ";
	}
	cout << "Hex: " << to_hex_string(format) << endl;
}

void InOutData8::print_binary(const char *name) const {
	if (strlen(name) > 0) {
		cout << name << " ";
	}
	cout << "Binary: " << to_binary_string() << endl;
}

void InOutData8::print_binary(OutputFormat format, const char *name) const {
	if (strlen(name) > 0) {
		cout << name << " ";
	}
	cout << "Binary: " << to_binary_string(format) << endl;
}

// -------------------------------------------------
// Conversion to arrays
// -------------------------------------------------
std::array<uint8_t, 1> InOutData8::to_array() const {
	return std::array<uint8_t, 1>{bytes[0]};
}

void InOutData8::from_array(const std::array<uint8_t, 1> &arr) {
	bytes[0] = arr[0];
}

// -------------------------------------------------
// Operators
// -------------------------------------------------
uint8_t InOutData8::operator[](int index) const {
	if (index == 0) {
		return bytes[0];
	}
	cerr << "InOutData8::operator[] const: Invalid index " << index << endl;
	return 0;
}

uint8_t &InOutData8::operator[](int index) {
	if (index == 0) {
		return bytes[0];
	}
	throw std::out_of_range("InOutData8::operator[]: index " +
							std::to_string(index) + " out of range, valid: 0");
}

InOutData8 InOutData8::operator&(const InOutData8 &other) const {
	InOutData8 result;
	result.packed = packed & other.packed;
	return result;
}

InOutData8 InOutData8::operator|(const InOutData8 &other) const {
	InOutData8 result;
	result.packed = packed | other.packed;
	return result;
}

InOutData8 InOutData8::operator^(const InOutData8 &other) const {
	InOutData8 result;
	result.packed = packed ^ other.packed;
	return result;
}

InOutData8 InOutData8::operator~() const {
	InOutData8 result;
	result.packed = ~packed;
	return result;
}

InOutData8 &InOutData8::operator&=(const InOutData8 &other) {
	packed &= other.packed;
	return *this;
}

InOutData8 &InOutData8::operator|=(const InOutData8 &other) {
	packed |= other.packed;
	return *this;
}

InOutData8 &InOutData8::operator^=(const InOutData8 &other) {
	packed ^= other.packed;
	return *this;
}

bool InOutData8::operator==(const InOutData8 &other) const {
	return packed == other.packed;
}

bool InOutData8::operator!=(const InOutData8 &other) const {
	return !(*this == other);
}

// Cast and assignment operators
InOutData8::operator uint8_t() const { return packed; }

InOutData8 &InOutData8::operator=(uint8_t value) {
	packed = value;
	return *this;
}

// -------------------------------------------------
// Static factory methods
// -------------------------------------------------
InOutData8 InOutData8::from_hex_string(const std::string &hex_str) {
	InOutData8 result;

	if (hex_str.empty()) {
		return result;
	}

	std::string clean_str = hex_str;

	// Strip the 0x or 0X prefix
	if (hex_str.size() >= 2 && hex_str.compare(0, 2, "0x") == 0) {
		clean_str = hex_str.substr(2);
	} else if (hex_str.size() >= 2 && hex_str.compare(0, 2, "0X") == 0) {
		clean_str = hex_str.substr(2);
	}

	// Take the last 2 characters (least significant byte)
	if (clean_str.length() < 2) {
		clean_str = std::string(2 - clean_str.length(), '0') + clean_str;
	} else {
		clean_str = clean_str.substr(clean_str.length() - 2);
	}

	try {
		result.packed =
			static_cast<uint8_t>(std::stoul(clean_str, nullptr, 16));
	} catch (const std::exception &e) {
		cerr << "InOutData8::from_hex_string: Error parsing hex string: "
				<< e.what() << endl;
		result.packed = 0;
	}

	return result;
}

InOutData8 InOutData8::from_byte(uint8_t value) { return InOutData8(value); }

InOutData8 InOutData8::from_bytes(const uint8_t *bytes, size_t len) {
	InOutData8 result;
	if (bytes && len > 0) {
		result.bytes[0] = bytes[0];
	}
	return result;
}

// Kept for interface compatibility (meaningless for 8 bits)
InOutData8 InOutData8::from_bytes_little_endian(const uint8_t *bytes,
												size_t len) {
	return from_bytes(bytes, len);
}

InOutData8 InOutData8::from_bytes_big_endian(const uint8_t *bytes, size_t len) {
	return from_bytes(bytes, len);
}

InOutData8 InOutData8::create_from_vector(const std::vector<uint8_t> &vec,
											size_t start_idx) {
	InOutData8 result;
	result.from_vector(vec, start_idx);
	return result;
}

// -------------------------------------------------
// Backward-compatibility methods
// -------------------------------------------------
void InOutData8::print_bin(const char *name) const { print_binary(name); }

void InOutData8::set_field(uint8_t byte0) { b0 = byte0; }
