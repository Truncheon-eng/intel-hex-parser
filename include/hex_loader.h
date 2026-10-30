// hex_loader.h - VERSION 1.4 - ADDED ERROR-HANDLING METHODS
#ifndef HEX_LOADER_H
#define HEX_LOADER_H

#include <vector>
#include <cstdint>
#include <string>
#include <iostream>
#include <iomanip>
#include <memory>
#include <fstream>

#include "intelhexclass.h"
#include "hex_parser_logger.h"


// Default endianness (set via -D in the Makefile)
#ifndef HEX_LOADER_DEFAULT_ENDIANNESS
#define HEX_LOADER_DEFAULT_ENDIANNESS little_endian
#endif

// Convert the little_endian/big_endian token into a numeric code.
// A direct comparison '#if HEX_LOADER_DEFAULT_ENDIANNESS == little_endian'
// is always true (both names are non-macros yielding 0 == 0) — use token
// pasting only
#define HEX_ENDIANNESS_LITTLE 1
#define HEX_ENDIANNESS_BIG 2
#define HEX_ENDIANNESS_CODE_little_endian HEX_ENDIANNESS_LITTLE
#define HEX_ENDIANNESS_CODE_big_endian HEX_ENDIANNESS_BIG
#define HEX_ENDIANNESS_CAT_(a, b) a##b
#define HEX_ENDIANNESS_CAT(a, b) HEX_ENDIANNESS_CAT_(a, b)
#define HEX_ENDIANNESS \
	HEX_ENDIANNESS_CAT(HEX_ENDIANNESS_CODE_, HEX_LOADER_DEFAULT_ENDIANNESS)
#if (HEX_ENDIANNESS != HEX_ENDIANNESS_LITTLE) && \
	(HEX_ENDIANNESS != HEX_ENDIANNESS_BIG)
#error "HEX_LOADER_DEFAULT_ENDIANNESS must be little_endian or big_endian"
#endif

// Disable checksum verification
#ifdef HEX_LOADER_DISABLE_CHECKSUM
#define HEX_LOADER_CHECKSUM_ENABLED false
#else
#define HEX_LOADER_CHECKSUM_ENABLED true
#endif

// Parser verbose mode
#ifdef HEX_LOADER_VERBOSE
#define HEX_LOADER_VERBOSE_MODE true
#else
#define HEX_LOADER_VERBOSE_MODE false
#endif

// Automatic error checking on load
#ifdef HEX_LOADER_AUTO_CHECK_ERRORS
#define HEX_LOADER_ERROR_CHECKING true
#else
#define HEX_LOADER_ERROR_CHECKING false
#endif

using std::cout;
using std::endl;
using std::vector;

// ============================================================================

class HexLoader : public intelhex {
private:
	std::unique_ptr<HexParserLogger> logger_;

public:
	HexLoader();
	~HexLoader() = default;

	// Basic byte read
	uint8_t read_byte(uint32_t address, uint8_t default_value = 0xFF);

	// 16-bit versions
	// 16-bit read in little-endian format
	uint16_t read_halfword_le(uint32_t address, uint8_t default_value = 0xFF);

	// 16-bit read in big-endian format
	uint16_t read_halfword_be(uint32_t address, uint8_t default_value = 0xFF);

	// Halfword read with default endianness
	uint16_t read_halfword(uint32_t address, uint8_t default_value = 0xFF);

	// 32-bit versions
	// 32-bit read in little-endian format
	uint32_t read_word_le(uint32_t address, uint8_t default_value = 0xFF);

	// 32-bit read in big-endian format
	uint32_t read_word_be(uint32_t address, uint8_t default_value = 0xFF);

	// Word read with default endianness
	uint32_t read_word(uint32_t address, uint8_t default_value = 0xFF);

	// 64-bit versions
	// 64-bit read in little-endian format
	uint64_t read_dword_le(uint32_t address, uint8_t default_value = 0xFF);

	// 64-bit read in big-endian format
	uint64_t read_dword_be(uint32_t address, uint8_t default_value = 0xFF);

	// Double word read with default endianness
	uint64_t read_dword(uint32_t address, uint8_t default_value = 0xFF);

	// Block operations
	vector<uint8_t> read_block(uint32_t start_address, uint32_t size,
								uint8_t default_value = 0xFF);

	// Utilities
	bool has_data(uint32_t address);
	void print_info();

	// ===== NEW ERROR-HANDLING METHODS =====
	// === Error-handling methods via the logger ===

	// Logger access
	HexParserLogger &logger() { return *logger_; }
	const HexParserLogger &logger() const { return *logger_; }

	// Quick checks
	bool has_parser_errors() const { return logger_->has_errors(); }
	bool has_parser_warnings() const { return logger_->has_warnings(); }
	bool has_parser_messages() const { return logger_->has_messages(); }

	// Collect messages from the parser
	void collect_parser_messages() { logger_->collect_messages(); }
	void collect_and_clear_parser_messages() { logger_->collect_and_clear(); }

	// Message output
	void print_parser_errors(bool colored = true, bool clear_after = false) {
		logger_->print_errors(colored, clear_after);
	}

	void print_parser_warnings(bool colored = true, bool clear_after = false) {
		logger_->print_warnings(colored, clear_after);
	}

	void print_all_parser_messages(bool colored = true,
									bool clear_after = false) {
		logger_->print_all(colored, clear_after);
	}

	void print_parser_summary() { logger_->print_summary(); }

	// Status
	HexParserLogger::ParserStatus get_parser_status() const {
		return logger_->get_status();
	}

	std::string get_parser_status_string() const {
		return logger_->get_status_string();
	}

	// File loading with automatic error collection
	bool load_file(const std::string &filename, bool collect_messages = true);
};

#endif	// HEX_LOADER_H
