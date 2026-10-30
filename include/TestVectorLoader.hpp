// TestVectorLoader.hpp
#ifndef TEST_VECTOR_LOADER_HPP
#define TEST_VECTOR_LOADER_HPP

#include <iostream>
#include <vector>
#include <cstdint>
#include <string>
#include <array>
#include <algorithm>
#include <memory>
#include <chrono>
#include <stdexcept>

#include "Definitions.hpp"
#include "hex_loader.h"

using std::cerr;
using std::cout;
using std::endl;
using std::string;

using std::vector;

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

struct TestHeaderDetailed;
struct TestVector;

// ============================================================================
// CONSTANTS AND TYPE ALIASES
// ============================================================================

using HeaderDescriptionFramerPCSTX =
	std::array<std::pair<uint32_t, std::string>, 12>;
using HeaderDescriptionSubsystems =
	std::array<std::pair<uint32_t, std::string>, 6>;

using RawDataBuffer = std::vector<uint8_t>;

// ============================================================================
// DATA STRUCTURES
// ============================================================================

struct TestHeaderDetailed {
	// Basic identification
	uint32_t signature;	 // Subsystem ID
	uint32_t module_id;	 // Module ID

	// Memory layout
	uint32_t total_data_length;	  // Total data length
	uint32_t input_base_addr;	  // Base address of input test vectors
	uint32_t input_vector_size;	  // Size of one input vector in bytes
	uint32_t output_base_addr;	  // Base address of output test vectors
	uint32_t output_vector_size;  // Size of one output vector in bytes
	uint32_t reserved;			  // Reserved

	// Signal configuration
	uint8_t num_input_signals;			  // Number of input signals
	uint8_t num_output_signals;			  // Number of output signals
	std::vector<uint16_t> input_widths;	  // Input signal widths in bits
	std::vector<uint16_t> output_widths;  // Output signal widths in bits

	// Validation flag
	bool valid = false;

	// Constructor
	TestHeaderDetailed();

	// Utility methods
	uint32_t total_input_bits() const noexcept;
	uint32_t total_output_bits() const noexcept;
	uint32_t total_input_bytes() const noexcept;
	uint32_t total_output_bytes() const noexcept;
	bool validate_sizes() const;
	void print_summary() const;

	// TODO: Add support for signal names
	// std::vector<std::string> input_names;
	// std::vector<std::string> output_names;
};

// ============================================================================
// TEST VECTOR STRUCTURE
// ============================================================================

struct TestVector {
	// ========================================================================
	// IDENTIFICATION
	// ========================================================================
	uint32_t vector_id = 0;
	uint32_t vector_number = 0;
	bool valid = false;

	// ========================================================================
	// RAW DATA STORAGE
	// ========================================================================
	// Main data - use ONLY ONE of the options below:

	// OPTION 1: Scalars only (recommended if 1-bit signals are packed
	// into bytes)
	std::vector<uint8_t> input_scalars;	  // Input scalar signals
	std::vector<uint8_t> output_scalars;  // Output scalar signals

	// OPTION 3: Raw data only
	std::vector<uint8_t> input_data;   // Input data (raw or processed)
	std::vector<uint8_t> output_data;  // Output data (raw or processed)

	// ========================================================================
	// PROCESSED DATA (uncomment if needed)
	// ========================================================================
	// For structured data (320/128/112/8 bits):

	// ========================================================================
	// CONSTRUCTORS
	// ========================================================================

	// Default constructor
	TestVector() = default;

	// Initializing constructor
	TestVector(uint32_t id, uint32_t number)
		: vector_id(id), vector_number(number), valid(false) {}

	// Move constructor
	TestVector(TestVector &&other) noexcept
		: vector_id(other.vector_id), vector_number(other.vector_number),
			valid(other.valid), input_scalars(std::move(other.input_scalars)),
			output_scalars(std::move(other.output_scalars)),
			input_data(std::move(other.input_data)),
			output_data(std::move(other.output_data)) {
		other.vector_id = 0;
		other.vector_number = 0;
		other.valid = false;
	}

	// Move assignment operator
	TestVector &operator=(TestVector &&other) noexcept {
		if (this != &other) {
			vector_id = other.vector_id;
			vector_number = other.vector_number;
			valid = other.valid;
			input_scalars = std::move(other.input_scalars);
			output_scalars = std::move(other.output_scalars);
			input_data = std::move(other.input_data);
			output_data = std::move(other.output_data);

			other.vector_id = 0;
			other.vector_number = 0;
			other.valid = false;
		}
		return *this;
	}

	// Copying disabled (data may be large)
	TestVector(const TestVector &) = delete;
	TestVector &operator=(const TestVector &) = delete;

	// ========================================================================
	// METHODS
	// ========================================================================

	// Clear data
	void clear() {
		input_scalars.clear();
		output_scalars.clear();

		input_data.clear();	 // Input data (raw or processed)
		output_data.clear();
		vector_id = 0;
		vector_number = 0;
		valid = false;
	}

	// Check data presence
	bool has_input_data() const {
		return !input_data.empty() || !input_scalars.empty();
	}
	bool has_output_data() const {
		return !output_data.empty() || !output_scalars.empty();
	}
	bool has_data() const { return has_input_data() && has_output_data(); }

	// Get data sizes
	size_t input_size() const {
		return input_data.size() + input_scalars.size();
	}

	size_t output_size() const {
		return output_data.size() + output_scalars.size();
	}

	size_t total_size() const {
		return input_scalars.size() + output_scalars.size();
	}

	// Vector comparison
	bool operator==(const TestVector &other) const {
		return vector_id == other.vector_id &&
				vector_number == other.vector_number &&
				input_scalars == other.input_scalars &&
				output_scalars == other.output_scalars;
	}

	// Print information
	void print_summary() const {
		std::cout << "=== Test Vector Summary ===" << std::endl;
		std::cout << "ID: " << vector_id << std::endl;
		std::cout << "Number: " << vector_number << std::endl;
		std::cout << "Valid: " << (valid ? "YES" : "NO") << std::endl;
		std::cout << "Input scalars: " << input_scalars.size() << " bytes"
					<< std::endl;
		std::cout << "Output scalars: " << output_scalars.size() << " bytes"
					<< std::endl;

		if (has_input_data() && input_scalars.size() <= 16) {
			std::cout << "First 4 input bytes: ";
			for (size_t i = 0; i < std::min(size_t(4), input_scalars.size());
					i++) {
				std::cout << "0x" << std::hex << std::setw(2)
							<< std::setfill('0')
							<< static_cast<int>(input_scalars[i]) << " ";
			}
			std::cout << std::dec << std::endl;
		}
	}

	// Detailed output (for debugging)
	void print_detailed() const {
		print_summary();

		if (has_input_data()) {
			std::cout << "\nInput data hex dump:" << std::endl;
			print_hex_dump(input_scalars, "IN");
		}

		if (has_output_data()) {
			std::cout << "\nOutput data hex dump:" << std::endl;
			print_hex_dump(output_scalars, "OUT");
		}
	}

private:
	// Helper method for hex dump
	static void print_hex_dump(const std::vector<uint8_t> &data,
								const std::string &prefix) {
		const size_t bytes_per_line = 16;
		const std::ios::fmtflags saved_flags = std::cout.flags();
		const std::streambuf::char_type saved_fill = std::cout.fill();

		for (size_t i = 0; i < data.size(); i += bytes_per_line) {
			std::cout << prefix << " 0x" << std::hex << std::setw(4)
						<< std::setfill('0') << i << ": ";

			// HEX bytes
			for (size_t j = 0; j < bytes_per_line; j++) {
				if (i + j < data.size()) {
					std::cout << std::hex << std::setw(2) << std::setfill('0')
								<< static_cast<int>(data[i + j]) << " ";
				} else {
					std::cout << "   ";
				}
			}

			std::cout << " ";

			// ASCII representation
			for (size_t j = 0; j < bytes_per_line; j++) {
				if (i + j < data.size()) {
					uint8_t byte = data[i + j];
					if (byte >= 32 && byte <= 126) {
						std::cout << static_cast<char>(byte);
					} else {
						std::cout << ".";
					}
				}
			}
			std::cout << std::endl;
		}

		std::cout.flags(saved_flags);
		std::cout.fill(saved_fill);
	}
};

// ============================================================================
// INTERNAL STATE MANAGEMENT
// ============================================================================

struct internalVariable {
	// Memory addresses
	uint32_t input_addr_read_offset;   // Input vector read address
	uint32_t output_addr_read_offset;  // Output vector read address

	// Current vector tracking
	uint32_t current_input_vec_num;	  // Current input vector number
	uint32_t current_output_vec_num;  // Current output vector number
	uint32_t num_vec;				  // Current test vector

	// Raw data buffers
	RawDataBuffer raw_input_data;	// Raw input data
	RawDataBuffer raw_output_data;	// Raw output data

	// Total test vectors

	uint32_t total_vector_count;
	bool total_vector_count_is_valid;

	const uint32_t PREVIEW_BYTES = 64;

	// Default constructor
	internalVariable() {
		input_addr_read_offset = 0;
		output_addr_read_offset = 0;
		current_input_vec_num = 0;
		current_output_vec_num = 0;
		num_vec = 0;
		raw_input_data.clear();
		raw_output_data.clear();

		total_vector_count = 0x00;
		total_vector_count_is_valid = false;
	};
};

// ============================================================================
// MAIN CLASS
// ============================================================================

class TestVectorLoader {
private:
	// Dependencies
	HexLoader &hex_loader;

	// Configuration
	// Module identifiers
	const HeaderDescriptionFramerPCSTX headerDescriptionFramerTXModules = {
		{{FULL, "FULL"},
			{OTU4_CONVERTER, "OTU4"},
			{DESCRAMBLER, "DSCR"},
			{FEC_EXTRACTOR, "FXTR"},
			{AIS_FRAME_GENERATOR_TX, "AFGT"},
			{FRAMER_STAT, "STAT"},
			{HEADER_PROCESSOR, "HEPR"},
			{HEADERS_MANAGER_TX, "HMTX"},
			{OTUC1_PADDER, "OTPD"},
			{SCRAMBLER, "SCRM"},
			{OUTPUT_BUFFER, "OBFR"},
			{FRAMER_PCS_TX, "FRTX"}}};

	// Subsystem identifiers
	const HeaderDescriptionSubsystems headerDescriptionAubSubsystems = {
		{{FRAMER_PCS_TX, "FRTX"},
			{FRAMER_PCS_RX, "FRPR"},
			{FRAMER_PCS, "FRPC"},
			{DSP_RX, "DSRX"},
			{DSP_TX, "DSTX"},
			{TEST_VECTOR_TEST, "TEST"}}};

	// State
	TestHeaderDetailed Vector_header;
	internalVariable sysVar;
	bool header_is_valid_ = false;

public:
	// ========================================================================
	// CONSTRUCTION AND BASIC OPERATIONS
	// ========================================================================

	explicit TestVectorLoader(HexLoader &loader);

	// Disable copying
	TestVectorLoader(const TestVectorLoader &) = delete;
	TestVectorLoader &operator=(const TestVectorLoader &) = delete;

	// ========================================================================
	// MAIN LOADING INTERFACE
	// ========================================================================

	void parse_header();
	void parse_basic_header_fields();
	bool validate_basic_header();
	void parse_signal_information();
	bool validate_signal_data();
	void perform_final_validation();

	// ========================================================================
	// QUERY METHODS
	// ========================================================================

	bool headerIsCorrect() const { return header_is_valid_; }

	TestHeaderDetailed get_header_description() const { return Vector_header; }

	// ========================================================================
	// PRINTING AND DEBUGGING METHODS
	// ========================================================================

	void print_header_info(const TestHeaderDetailed &header,
							bool detailed) const;
	void print_test_vector(const TestVector &tv, bool detailed = false,
							const TestHeaderDetailed *header = nullptr) const;
	void print_raw_data_analysis() const;

	// ========================================================================
	// UTILITY METHODS
	// ========================================================================

	std::string get_header_field_description(
		uint32_t offset, const TestHeaderDetailed &header) const;
	auto find_in_header_array(uint32_t key);

	// Method to calculate the total test vector count
	bool calculate_total_test_vector_count();

	bool set_total_vector_count(uint32_t total_vec_cnt) {
		if (total_vec_cnt == 0) {
			sysVar.total_vector_count_is_valid = false;
			return false;
		}
		this->sysVar.total_vector_count = total_vec_cnt;
		this->sysVar.total_vector_count_is_valid = true;
		return true;
	}

	uint32_t get_total_vector_count(void) {
		if (sysVar.total_vector_count == 0 ||
			sysVar.total_vector_count_is_valid == false) {
			throw std::runtime_error("total_vector_count is not valid !\t");
		}
		return sysVar.total_vector_count;
	}

	void preview_data_block(uint32_t address, uint32_t size,
							const string &prefix);

	void print_vector_summary(uint32_t vector_num, uint32_t input_addr,
								uint32_t output_addr);

	// ========================================================================
	// VECTOR READING METHODS (UNIFIED)
	// ========================================================================

	// Main reading methods
	bool read_vector(uint32_t vector_num, TestVector &result);
	bool read_vector_by_address(uint32_t input_addr, uint32_t output_addr,
								TestVector &result);

	// Batch reading
	bool read_vector_range(uint32_t start, uint32_t end,
							std::vector<TestVector> &results,
							bool verbose_mode = false);

	bool read_all_vectors(std::vector<TestVector> &results);

	// Preview methods (view only)
	void preview_vector(uint32_t vector_num, bool show_summary = true);

	void preview_vector_by_address(uint32_t input_addr, uint32_t output_addr,
									bool show_summary = true);

	// Fast reading (raw data only)

	// Read a data block
	std::vector<uint8_t> read_data_block(uint32_t address, uint32_t size);

	bool read_data_block(uint32_t address, uint32_t size,
							std::vector<uint8_t> &result);

	// Helper methods

	// ========================================================================
	// DEPRECATED
	// ========================================================================

	// bool read_raw_vector_data_extern(uint32_t vec_num,
	//								 TestVector& result);

	// bool read_single_vector(uint32_t vector_num,
	// 						   TestVector& result);

	// void preview_vector_data(uint32_t input_addr,
	// 						 uint32_t output_addr,
	//						 bool show_summary = false);

	// void parse_single_vector(uint32_t vector_num,
	//						 bool show_summary = false);
	//  parse_single_vector -> preview_vector

	// Logging flags
	enum LogFlag {
		LOG_FLAG_NONE = 0,
		LOG_FLAG_ERROR = 1 << 0,	// 1
		LOG_FLAG_WARNING = 1 << 1,	// 2
		LOG_FLAG_INFO = 1 << 2,		// 4
		LOG_FLAG_DEBUG = 1 << 3,	// 8
		LOG_FLAG_ALL =
			LOG_FLAG_ERROR | LOG_FLAG_WARNING | LOG_FLAG_INFO | LOG_FLAG_DEBUG
	};

	// Preset levels for convenience
	enum LogLevel {
		LOG_LEVEL_NONE = LOG_FLAG_NONE,
		LOG_LEVEL_ERROR = LOG_FLAG_ERROR,
		LOG_LEVEL_WARNING = LOG_FLAG_ERROR | LOG_FLAG_WARNING,
		LOG_LEVEL_INFO = LOG_FLAG_ERROR | LOG_FLAG_WARNING | LOG_FLAG_INFO,
		LOG_LEVEL_DEBUG = LOG_FLAG_ALL,
		LOG_LEVEL_ALL = LOG_FLAG_ALL
	};

	// Methods
	void set_log_level(int mask) { log_mask_ = mask; }

	void set_log_level(LogLevel level) { log_mask_ = level; }

	// Quick settings
	void enable_debug_logging() { log_mask_ |= LOG_FLAG_DEBUG; }

	void disable_debug_logging() { log_mask_ &= ~LOG_FLAG_DEBUG; }

	void enable_info_logging() { log_mask_ |= LOG_FLAG_INFO; }

	void disable_info_logging() { log_mask_ &= ~LOG_FLAG_INFO; }

	void enable_all_logging() { log_mask_ = LOG_FLAG_ALL; }

	void disable_all_logging() { log_mask_ = LOG_FLAG_NONE; }

	// Check whether a given level is enabled
	bool is_debug_enabled() const { return log_mask_ & LOG_FLAG_DEBUG; }

	bool is_info_enabled() const { return log_mask_ & LOG_FLAG_INFO; }

	bool is_warning_enabled() const { return log_mask_ & LOG_FLAG_WARNING; }

	bool is_error_enabled() const { return log_mask_ & LOG_FLAG_ERROR; }

private:
	// ========================================================================
	// PRIVATE HELPER METHODS
	// ========================================================================

	// Header parsing
	void read_signal_widths(uint32_t &addr, uint8_t count,
							const std::string &signal_type,
							std::vector<uint16_t> &widths);

	// Debug utilities
	void debug_print_header_memory(const TestHeaderDetailed &header);
	void debug_print_enhanced_signal_analysis(const TestHeaderDetailed &header);
	void print_hex_dump(const RawDataBuffer &data,
						const std::string &prefix) const;

	// String conversion
	std::string to_hex_string(uint32_t value) const;
	uint32_t read_and_log_field(const std::string &field_name,
								uint32_t address);

	void set_header_valid() { this->header_is_valid_ = true; }

	void set_header_corrupt() { this->header_is_valid_ = false; }

	// Helper methods
	void validate_read_operation(uint32_t vector_num) const;
	bool read_vector_internal(uint32_t input_addr, uint32_t output_addr,
								TestVector &result,
								uint32_t vector_num = UINT32_MAX);

	uint32_t calculate_vector_number(uint32_t input_addr,
										uint32_t output_addr) const;

	void print_preview_summary(uint32_t input_addr, uint32_t output_addr);

	// Debug level control method

	int log_mask_ = LOG_LEVEL_WARNING;	// Default: errors and warnings

	void log_debug(const std::string &message, bool color = false) {
		if (log_mask_ & LOG_FLAG_DEBUG) {
			cout << (color ? GREEN : NORMAL) << "[DEBUG] " << message << NORMAL
					<< endl;
		}
	}

	void log_info(const std::string &message, bool color = false) {
		if (log_mask_ & LOG_FLAG_INFO) {
			cout << (color ? CYAN : NORMAL) << "[INFO] " << message << NORMAL
					<< endl;
		}
	}

	void log_warning(const std::string &message, bool color = false) {
		if (log_mask_ & LOG_FLAG_WARNING) {
			cerr << (color ? YELLOW : NORMAL) << "[WARNING] " << message
					<< NORMAL << endl;
		}
	}

	void log_error(const std::string &message, bool color = false) {
		if (log_mask_ & LOG_FLAG_ERROR) {
			cerr << (color ? RED : NORMAL) << "[ERROR] " << message << NORMAL
					<< endl;
		}
	}

	// Optional special methods for success/failure
	void log_success_icon(const std::string &message, bool color = true) {
		if (log_mask_ & LOG_FLAG_INFO) {
			cout << (color ? GREEN : NORMAL) << CHECK_MARK << message << NORMAL
					<< endl;
		}
	}

	void log_failure_icon(const std::string &message, bool color = true) {
		if (log_mask_ & LOG_FLAG_ERROR) {
			cerr << (color ? RED : NORMAL) << CROSS_MARK << message << NORMAL
					<< endl;
		}
	}

	void log_warning_icon(const std::string &message, bool color = true) {
		if (log_mask_ & LOG_FLAG_WARNING) {
			cerr << (color ? YELLOW : NORMAL) << WARNING_MARK << message
					<< NORMAL << endl;
		}
	}

	void log_info_icon(const std::string &message, bool color = true) {
		if (log_mask_ & LOG_FLAG_INFO) {
			cout << (color ? BLUE : NORMAL) << INFO_MARK << message << NORMAL
					<< endl;
		}
	}

// Convenience macros (optional)
#define LOG_DEBUG(msg)              \
	if (log_mask_ & LOG_FLAG_DEBUG) \
	cout << "[DEBUG] " << msg << endl
#define LOG_INFO(msg)              \
	if (log_mask_ & LOG_FLAG_INFO) \
	cout << BLUE << "[INFO] " << msg << NORMAL << endl
#define LOG_WARNING(msg)              \
	if (log_mask_ & LOG_FLAG_WARNING) \
	cerr << YELLOW << "[WARNING] " << msg << NORMAL << endl
#define LOG_ERROR(msg)              \
	if (log_mask_ & LOG_FLAG_ERROR) \
	cerr << RED << "[ERROR] " << msg << NORMAL << endl
};

// ============================================================================
// INLINE IMPLEMENTATIONS (for simple methods)
// ============================================================================

inline TestVectorLoader::TestVectorLoader(HexLoader &loader)
	: hex_loader(loader) {
	// internal_state_ is initialized with default values
}

#endif	// TEST_VECTOR_LOADER_HPP
