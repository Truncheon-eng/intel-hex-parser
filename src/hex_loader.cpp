#include "hex_loader.h"

// Constructor
HexLoader::HexLoader()
	: intelhex(), logger_(std::make_unique<HexParserLogger>(this)) {
	// TODO: Implement checksum-disable support
	// if (!HEX_LOADER_CHECKSUM_ENABLED) {
	//     checksumVerificationOff();
	// }
	if (HEX_LOADER_VERBOSE_MODE) {
		verboseOn();
	}
}

// Basic byte read
uint8_t HexLoader::read_byte(uint32_t address, uint8_t default_value) {
	unsigned char data;
	if (getData(&data, address)) {
		return static_cast<uint8_t>(data);
	}
	return default_value;
}

// 16-bit read in little-endian format
uint16_t HexLoader::read_halfword_le(uint32_t address, uint8_t default_value) {
	uint16_t result = 0;
	for (int i = 0; i < 2; i++) {
		uint8_t byte = read_byte(address + i, default_value);
		result |= (static_cast<uint16_t>(byte) << (8 * i));
	}
	return result;
}

// 16-bit read in big-endian format
uint16_t HexLoader::read_halfword_be(uint32_t address, uint8_t default_value) {
	uint16_t result = 0;
	for (int i = 0; i < 2; i++) {
		uint8_t byte = read_byte(address + i, default_value);
		result |= (static_cast<uint16_t>(byte) << (8 * (1 - i)));
	}
	return result;
}

// Halfword read with default endianness
uint16_t HexLoader::read_halfword(uint32_t address, uint8_t default_value) {
#if HEX_ENDIANNESS == HEX_ENDIANNESS_LITTLE
	return read_halfword_le(address, default_value);
#else
	return read_halfword_be(address, default_value);
#endif
}

// 32-bit read in little-endian format
uint32_t HexLoader::read_word_le(uint32_t address, uint8_t default_value) {
	uint32_t result = 0;
	for (int i = 0; i < 4; i++) {
		uint8_t byte = read_byte(address + i, default_value);
		result |= (static_cast<uint32_t>(byte) << (8 * i));
	}
	return result;
}

// 32-bit read in big-endian format
uint32_t HexLoader::read_word_be(uint32_t address, uint8_t default_value) {
	uint32_t result = 0;
	for (int i = 0; i < 4; i++) {
		uint8_t byte = read_byte(address + i, default_value);
		result |= (static_cast<uint32_t>(byte) << (8 * (3 - i)));
	}
	return result;
}

// Word read with default endianness
uint32_t HexLoader::read_word(uint32_t address, uint8_t default_value) {
#if HEX_ENDIANNESS == HEX_ENDIANNESS_LITTLE
	return read_word_le(address, default_value);
#else
	return read_word_be(address, default_value);
#endif
}

// 64-bit read in little-endian format
uint64_t HexLoader::read_dword_le(uint32_t address, uint8_t default_value) {
	uint64_t result = 0;
	for (int i = 0; i < 8; i++) {
		uint8_t byte = read_byte(address + i, default_value);
		result |= (static_cast<uint64_t>(byte) << (8 * i));
	}
	return result;
}

// 64-bit read in big-endian format
uint64_t HexLoader::read_dword_be(uint32_t address, uint8_t default_value) {
	uint64_t result = 0;
	for (int i = 0; i < 8; i++) {
		uint8_t byte = read_byte(address + i, default_value);
		result |= (static_cast<uint64_t>(byte) << (8 * (7 - i)));
	}
	return result;
}

// Double word read with default endianness
uint64_t HexLoader::read_dword(uint32_t address, uint8_t default_value) {
#if HEX_ENDIANNESS == HEX_ENDIANNESS_LITTLE
	return read_dword_le(address, default_value);
#else
	return read_dword_be(address, default_value);
#endif
}

// Block operations
vector<uint8_t> HexLoader::read_block(uint32_t start_address, uint32_t size,
										uint8_t default_value) {
	vector<uint8_t> block;
	block.reserve(size);

	for (uint32_t i = 0; i < size; i++) {
		block.push_back(read_byte(start_address + i, default_value));
	}
	return block;
}

// Utilities
bool HexLoader::has_data(uint32_t address) {
	unsigned char dummy;
	return getData(&dummy, address);
}

void HexLoader::print_info() {
	cout << "=== HEX Loader Information ===" << endl;
	cout << "Total data bytes: " << size() << endl;

#if HEX_ENDIANNESS == HEX_ENDIANNESS_LITTLE
	cout << "Default endianness: little-endian" << endl;
#else
	cout << "Default endianness: big-endian" << endl;
#endif

	cout << "Checksum verification: "
			<< (HEX_LOADER_CHECKSUM_ENABLED ? "enabled" : "disabled") << endl;
	cout << "Verbose mode: "
			<< (HEX_LOADER_VERBOSE_MODE ? "enabled" : "disabled") << endl;

	begin();
	unsigned long start_addr = currentAddress();
	end();
	unsigned long end_addr = currentAddress();

	cout << "Address range: 0x" << std::hex << start_addr << " - 0x" << end_addr
			<< std::dec << endl;

	unsigned long eip;
	if (getStartLinearAddress(&eip)) {
		cout << "Start address (EIP): 0x" << std::hex << eip << std::dec
				<< endl;
	}

	cout << "=============================" << endl;
}

// File loading
bool HexLoader::load_file(const std::string &filename, bool collect_messages) {
	std::ifstream file(filename);
	if (!file.is_open()) {
		logger_->add_error("Cannot open file: " + filename);
		return false;
	}

	try {
		// Load data
		file >> *this;
		file.close();

		// Collect messages from the parser
		if (collect_messages) {
			logger_->collect_messages();
		}

		return !logger_->has_errors() && getNoErrors() == 0;

	} catch (const std::exception &e) {
		logger_->add_error("Exception during loading: " +
							std::string(e.what()));
		return false;
	}
}
