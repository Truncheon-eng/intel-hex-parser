// TestVectorLoader.cpp
#include "TestVectorLoader.hpp"
#include <iomanip>
#include <sstream>
#include <cstdint>
#include <climits>
#include <numeric>

using namespace std;

#define TEST_VECTOR_DEBUG

// ============================================================================
// TestHeaderDetailed IMPLEMENTATION
// ============================================================================

// TestHeaderDetailed implementation
TestHeaderDetailed::TestHeaderDetailed()
	: signature(0), module_id(0), total_data_length(0), input_base_addr(0),
		input_vector_size(0), output_base_addr(0), output_vector_size(0),
		reserved(0), num_input_signals(0), num_output_signals(0), valid(false) {}


constexpr uint32_t bits_to_bytes(uint32_t bits) noexcept {
	return (bits + CHAR_BIT - 1) / CHAR_BIT;
}

uint32_t TestHeaderDetailed::total_input_bits() const noexcept {
	return std::accumulate(input_widths.cbegin(), input_widths.cend(), 0u);
}

uint32_t TestHeaderDetailed::total_input_bytes() const noexcept {
	return std::transform_reduce(input_widths.cbegin(), input_widths.cend(), 0u,
									std::plus<>(), [](uint32_t width) {
										return (width + 7) / CHAR_BIT;
									});
}

uint32_t TestHeaderDetailed::total_output_bits() const noexcept {
	return std::accumulate(output_widths.cbegin(), output_widths.cend(), 0u);
}

uint32_t TestHeaderDetailed::total_output_bytes() const noexcept {
	return std::transform_reduce(output_widths.cbegin(), output_widths.cend(),
									0u, std::plus<>(), [](uint32_t width) {
										return (width + 7) / CHAR_BIT;
									});
}

bool TestHeaderDetailed::validate_sizes() const {
	return total_input_bytes() == input_vector_size &&
			total_output_bytes() == output_vector_size;
}

void TestHeaderDetailed::print_summary() const {
	cout << "=== Header Summary ===" << endl;
	cout << "Module: 0x" << std::hex << module_id << std::dec << endl;
	cout << "Inputs: " << (int)num_input_signals << " signals, "
			<< input_vector_size << " bytes" << endl;
	cout << "Outputs: " << (int)num_output_signals << " signals, "
			<< output_vector_size << " bytes" << endl;
	cout << "Valid: " << (valid ? "YES" : "NO") << endl;
}


// bool is_valid() const {
// 		return valid &&
// 			   signature == BEGIN_HEADER_WORD &&
// 			   num_input_signals > 0 &&
// 			   num_output_signals > 0 &&
// 			   input_vector_size > 0 &&
// 			   output_vector_size > 0 &&
// 			   input_widths.size() == num_input_signals &&
// 			   output_widths.size() == num_output_signals;
// 	}

// ============================================================================
// TestVector IMPLEMENTATION
// ============================================================================

// TestVector::TestVector() : vector_id(0), vector_number(0) {}

// ============================================================================
// TestVectorLoader PRIVATE HELPERS
// ============================================================================

void TestVectorLoader::read_signal_widths(uint32_t &addr, uint8_t count,
											const std::string &signal_type,
											std::vector<uint16_t> &widths) {
	for (uint8_t i = 0; i < count; i++) {
		uint16_t width = hex_loader.read_halfword_be(addr);
		widths.push_back(width);
#ifdef TEST_VECTOR_DEBUG
		cout << "DEBUG: " << signal_type << " width[" << (int)i
				<< "] = " << width << endl;
#endif
		addr += 2;
	}
}

auto TestVectorLoader::find_in_header_array(uint32_t key) {
	return std::find_if(headerDescriptionFramerTXModules.begin(),
						headerDescriptionFramerTXModules.end(),
						[key](const auto &pair) {
							return pair.first == key;
						});
}

std::string TestVectorLoader::to_hex_string(uint32_t value) const {
	std::stringstream ss;
	ss << std::hex << std::uppercase << value;
	return ss.str();
}

// ============================================================================
// HEADER PARSING - REFACTORED
// ============================================================================

void TestVectorLoader::parse_header() {
	log_info("=== START PARSING HEADER ===");

	try {
		// 1. Parse basic header fields
		parse_basic_header_fields();

		// 2. Validate basic structure
		if (!validate_basic_header()) {
			log_error("Basic header validation failed");
			return;
		}

		// 3. Parse signal information
		parse_signal_information();

		// 4. Validate signal data
		if (!validate_signal_data()) {
			log_error("Signal data validation failed");
			return;
		}

		// 5. Final integrity check
		perform_final_validation();

		// Calculate the total test vector count
		if (calculate_total_test_vector_count()) {
			log_info("Calculated total vectors: " +
						std::to_string(sysVar.total_vector_count));
		}

		// 6. Debug output
		if (is_debug_enabled()) {
			debug_print_header_memory(Vector_header);
		}

		set_header_valid();
		log_info("=== HEADER PARSING COMPLETED SUCCESSFULLY ===");

	} catch (const std::exception &e) {
		log_error("Error in parse_header: " + std::string(e.what()));
		set_header_corrupt();
	}
}

void TestVectorLoader::parse_basic_header_fields() {
	// Read all basic fields in one method
	Vector_header.signature = read_and_log_field(
		"Signature", HEADER_BASE_ADDR + HEADER_SIGNATURE_OFFSET);

	Vector_header.module_id = read_and_log_field(
		"Module ID", HEADER_BASE_ADDR + HEADER_MODULE_ID_OFFSET);

	Vector_header.total_data_length =
		read_and_log_field("Total data length",
							HEADER_BASE_ADDR + HEADER_TOTAL_DATA_LENGTH_OFFSET);

	Vector_header.input_base_addr = read_and_log_field(
		"Input base address", HEADER_BASE_ADDR + HEADER_INPUT_BASE_ADDR_OFFSET);

	Vector_header.input_vector_size =
		read_and_log_field("Input vector size",
							HEADER_BASE_ADDR + HEADER_INPUT_VECTOR_SIZE_OFFSET);

	Vector_header.output_base_addr =
		read_and_log_field("Output base address",
							HEADER_BASE_ADDR + HEADER_OUTPUT_BASE_ADDR_OFFSET);

	Vector_header.output_vector_size =
		read_and_log_field("Output vector size",
							HEADER_BASE_ADDR + HEADER_OUTPUT_VECTOR_SIZE_OFFSET);

	Vector_header.reserved = read_and_log_field(
		"Reserved", HEADER_BASE_ADDR + HEADER_RESERVED_OFFSET);
}

uint32_t TestVectorLoader::read_and_log_field(const std::string &field_name,
												uint32_t address) {
	uint32_t value = hex_loader.read_word_be(address);
#ifdef TEST_VECTOR_DEBUG
	cout << field_name << " at 0x" << std::hex << address << ": 0x" << value
			<< std::dec << endl;
#endif
	return value;
}

bool TestVectorLoader::validate_basic_header() {
	log_info("=== VALIDATING BASIC HEADER ===");

	bool validation_passed = true;
	std::stringstream ss;  //

	// 1. Check signature (must be a subsystem)
	bool signature_valid = false;
	string subsystem_name;

	for (const auto &pair : headerDescriptionAubSubsystems) {
		if (pair.first == Vector_header.signature) {
			signature_valid = true;
			subsystem_name = pair.second;
			break;
		}
	}

	if (!signature_valid) {
		log_failure_icon("Invalid subsystem signature!");

		// Formatted hex output
		ss.str("");
		ss.clear();
		ss << "Found: 0x" << std::hex << std::uppercase << std::setw(8)
			<< std::setfill('0') << Vector_header.signature << std::dec;
		log_failure_icon(ss.str(), false);

		// Show the list of expected subsystems
		log_warning("Expected SUBSYSTEMS:");
		for (const auto &pair : headerDescriptionAubSubsystems) {
			ss.str("");
			ss.clear();
			ss << "  • 0x" << std::hex << std::uppercase << std::setw(8)
				<< std::setfill('0') << pair.first << " - " << pair.second
				<< std::dec;
			log_info(ss.str(), false);
		}
		validation_passed = false;
	} else {
		log_success_icon("Subsystem signature recognized: " + subsystem_name);
	}

	// 2. Check module_id (must be a module)
	bool module_valid = false;
	string module_name;

	for (const auto &pair : headerDescriptionFramerTXModules) {
		if (pair.first == Vector_header.module_id) {
			module_valid = true;
			module_name = pair.second;
			break;
		}
	}

	if (!module_valid) {
		log_failure_icon("Unknown module ID!");

		ss.str("");
		ss.clear();
		ss << "Found: 0x" << std::hex << std::uppercase << std::setw(8)
			<< std::setfill('0') << Vector_header.module_id << std::dec;
		log_failure_icon(ss.str(), false);

		// Show the list of expected modules
		log_warning_icon("Expected MODULES:");
		for (const auto &pair : headerDescriptionFramerTXModules) {
			ss.str("");
			ss.clear();
			ss << "  • 0x" << std::hex << std::uppercase << std::setw(8)
				<< std::setfill('0') << pair.first << " - " << pair.second
				<< std::dec;
			log_info_icon(ss.str(), false);
		}
		validation_passed = false;
	} else {
		log_success_icon("Module ID recognized: " + module_name);
	}

	// 3. If both are valid, show the full configuration
	if (signature_valid && module_valid) {
		log_info_icon("Test configuration: " + subsystem_name + " -> " +
						module_name);
	}

	// 4. Check vector sizes
	if (Vector_header.input_vector_size == 0) {
		log_failure_icon("Zero input vector size");
		validation_passed = false;
	} else {
		log_success_icon("Input vector size: " +
							std::to_string(Vector_header.input_vector_size) +
							" bytes");
	}

	if (Vector_header.output_vector_size == 0) {
		log_failure_icon("Zero output vector size");
		validation_passed = false;
	} else {
		log_success_icon("Output vector size: " +
							std::to_string(Vector_header.output_vector_size) +
							" bytes");
	}

	// 5. Check total data length
	if (Vector_header.total_data_length == 0) {
		log_warning_icon("Zero total data length");
	} else {
		// Verify there is enough data for at least one vector
		uint32_t min_required =
			Vector_header.input_vector_size + Vector_header.output_vector_size;

		if (Vector_header.total_data_length < min_required) {
			log_failure_icon("Total data length too small!");

			ss.str("");
			ss.clear();
			ss << "Minimum required for one vector: " << min_required
				<< " bytes, Available: " << Vector_header.total_data_length
				<< " bytes";
			log_error(ss.str(), false);

			validation_passed = false;
		} else {
			// Calculate the vector count
			uint32_t vector_pair_size = Vector_header.input_vector_size +
										Vector_header.output_vector_size;
			uint32_t total_vectors =
				Vector_header.total_data_length / vector_pair_size;
			uint32_t remainder =
				Vector_header.total_data_length % vector_pair_size;

			ss.str("");
			ss.clear();
			ss << "Estimated vectors: " << total_vectors << " ("
				<< Vector_header.total_data_length << " bytes total)";
			log_success_icon(ss.str());

			if (remainder != 0) {
				ss.str("");
				ss.clear();
				ss << "Data length not multiple of vector size. Remainder: "
					<< remainder << " bytes";
				log_warning_icon(ss.str());
			}
		}
	}

	// 6. Check base addresses
	if (Vector_header.input_base_addr == 0) {
		log_warning_icon("Zero input base address");
	}

	if (Vector_header.output_base_addr == 0) {
		log_warning_icon("Zero output base address");
	}

	// 7. Check the reserved field
	if (Vector_header.reserved != HEADER_RESERVED_MAGIC) {
		ss.str("");
		ss.clear();
		ss << "Reserved field is non-zero: 0x" << std::hex
			<< Vector_header.reserved << std::dec;
		log_info_icon(ss.str(), true);
	}

	// 8. Summary
	if (validation_passed) {
		log_success_icon("=== BASIC HEADER VALIDATION PASSED ===");

		// Summary information (only in DEBUG or INFO mode)
		if (is_debug_enabled() || is_info_enabled()) {
			log_info("=== TEST CONFIGURATION SUMMARY ===", true);
			log_info("Subsystem:      " + subsystem_name, false);
			log_info("Module:         " + module_name, false);
			log_info("Input size:     " +
							std::to_string(Vector_header.input_vector_size) +
							" bytes per vector",
						false);
			log_info("Output size:    " +
							std::to_string(Vector_header.output_vector_size) +
							" bytes per vector",
						false);
			log_info("Total data:     " +
							std::to_string(Vector_header.total_data_length) +
							" bytes",
						false);

			// Additional calculations
			if (Vector_header.input_vector_size > 0 &&
				Vector_header.output_vector_size > 0) {
				uint32_t vector_pair_size = Vector_header.input_vector_size +
											Vector_header.output_vector_size;
				if (vector_pair_size > 0) {
					uint32_t total_vectors =
						Vector_header.total_data_length / vector_pair_size;
					log_info(
						"Estimated:      " + std::to_string(total_vectors) +
							" test vectors",
						false);
				}
			}
			log_info("==================================", true);
		}

		return true;
	} else {
		log_failure_icon("\n=== BASIC HEADER VALIDATION FAILED ===");
		return false;
	}
}

// ============================================================================
// DEBUG AND PRINT METHODS
// ============================================================================

void TestVectorLoader::print_header_info(const TestHeaderDetailed &header,
											bool detailed) const {
	cout << "=== Test Header Information ===" << endl;

	// Basic information
	cout << "Signature: 0x\t" << std::hex << header.signature << std::dec
			<< endl;
	cout << "Module ID: 0x\t" << std::hex << header.module_id << std::dec
			<< endl;
	cout << "Total data length:\t" << header.total_data_length << " bytes"
			<< endl;
	cout << "Input base address: 0x\t" << std::hex << header.input_base_addr
			<< std::dec << endl;
	cout << "Input vector size:\t" << header.input_vector_size << " bytes"
			<< endl;
	cout << "Output base address: 0x\t" << std::hex << header.output_base_addr
			<< std::dec << endl;
	cout << "Output vector size:\t" << header.output_vector_size << " bytes"
			<< endl;

	if (detailed) {
		cout << "\n=== Signal Information ===" << endl;
		cout << "Input signals: " << (int)header.num_input_signals << endl;
		cout << "Output signals: " << (int)header.num_output_signals << endl;

		uint32_t total_input_bits = header.total_input_bits();
		uint32_t total_output_bits = header.total_output_bits();

		cout << "\n=== Bit Counts ===" << endl;
		cout << "Total input bits: " << total_input_bits << " ("
				<< (total_input_bits + 7) / 8 << " bytes)" << endl;
		cout << "Total output bits: " << total_output_bits << " ("
				<< (total_output_bits + 7) / 8 << " bytes)" << endl;

		cout << "\n=== Individual Signal Widths ===" << endl;
		cout << "Input signals:" << endl;
		for (size_t i = 0; i < header.input_widths.size(); i++) {
			cout << "  Signal " << (i + 1) << ": " << header.input_widths[i]
					<< " bits (" << (header.input_widths[i] + 7) / 8 << " bytes)"
					<< endl;
		}

		cout << "Output signals:" << endl;
		for (size_t i = 0; i < header.output_widths.size(); i++) {
			cout << "  Signal " << (i + 1) << ": " << header.output_widths[i]
					<< " bits (" << (header.output_widths[i] + 7) / 8 << " bytes)"
					<< endl;
		}

		cout << "\n=== Size Validation ===" << endl;
		if (header.validate_sizes()) {
			cout << GREEN << CHECK_MARK << "Header size validation: PASSED"
					<< endl;
		} else {
			cout << RED << CROSS_MARK << "Header size validation: FAILED"
					<< endl;
			cout << "  Calculated input bytes: " << ((total_input_bits + 7) / 8)
					<< " vs header: " << header.input_vector_size << endl;
			cout << "  Calculated output bytes: "
					<< ((total_output_bits + 7) / 8)
					<< " vs header: " << header.output_vector_size << endl;
		}
	} else {
		// Compact output for non-detailed mode
		cout << "Input signals: " << (int)header.num_input_signals << endl;
		cout << "Output signals: " << (int)header.num_output_signals << endl;
		cout << "Total input bits: " << header.total_input_bits() << endl;
		cout << "Total output bits: " << header.total_output_bits() << endl;

		cout << "Input widths: ";
		for (size_t i = 0; i < header.input_widths.size(); i++) {
			cout << header.input_widths[i];
			if (i < header.input_widths.size() - 1) {
				cout << ", ";
			}
		}
		cout << " bits" << endl;

		cout << "Output widths: ";
		for (size_t i = 0; i < header.output_widths.size(); i++) {
			cout << header.output_widths[i];
			if (i < header.output_widths.size() - 1) {
				cout << ", ";
			}
		}
		cout << " bits" << endl;
	}

	cout << "=============================" << endl;
}

void TestVectorLoader::debug_print_header_memory(
	const TestHeaderDetailed &header) {
#ifdef TEST_VECTOR_DEBUG
	cout << "DEBUG: Checking header memory..." << endl;

	uint32_t addr = 0;
	uint32_t data = 0;

	// Read the fixed part of the header (before signals)
	for (uint32_t i = 0; i < HEADER_SIGNAL_WIDTHS_OFFSET; i += 4) {
		addr = HEADER_BASE_ADDR + i;
		data = hex_loader.read_word_be(addr);

		cout << "  0x" << std::hex << addr << ": 0x" << data << std::dec;
		cout << " ← " << get_header_field_description(i, header);

		// Special handling for SIGNALS_INFO (2 bytes)
		if (i == HEADER_SIGNALS_INFO_OFFSET) {
			uint16_t signals_info = hex_loader.read_halfword_be(addr);
			cout << " (0x" << std::hex << signals_info << std::dec << ")";
		}

		cout << endl;
	}

	// Signal area - read 2 bytes at a time
	uint32_t signal_addr = HEADER_BASE_ADDR + HEADER_SIGNAL_WIDTHS_OFFSET;

	cout << "Input and Output signals:" << endl;

	// Input signals
	for (uint8_t i = 0; i < header.num_input_signals; i++) {
		uint16_t width = hex_loader.read_halfword_be(signal_addr);
		cout << "  0x" << std::hex << signal_addr << ": 0x" << width << std::dec
				<< " ← INPUT_WIDTH[" << (int)i << "] (" << width << "b)" << endl;
		//<< std::dec << " ← " << get_header_field_description(signal_addr -
		// HEADER_BASE_ADDR, header) << endl;
		signal_addr += 2;
	}

	// Output signals
	for (uint8_t i = 0; i < header.num_output_signals; i++) {
		uint16_t width = hex_loader.read_halfword_be(signal_addr);
		cout << "  0x" << std::hex << signal_addr << ": 0x" << width << std::dec
				<< " ← OUTPUT_WIDTH[" << (int)i << "] (" << width << "b)" << endl;
		//<< std::dec << " ← " << get_header_field_description(signal_addr -
		// HEADER_BASE_ADDR, header) << endl;
		signal_addr += 2;
	}
#endif
}

// Detailed raw data analysis
void TestVectorLoader::print_raw_data_analysis() const {
	std::cout << "\n=== RAW DATA ANALYSIS ===" << std::endl;

	// Analyze input data
	std::cout << "INPUT DATA (" << std::dec << sysVar.raw_input_data.size()
				<< " bytes):" << std::endl;
	print_hex_dump(sysVar.raw_input_data, "IN");
	// print_data_statistics(sysVar.raw_input_data, "Input");

	// Analyze output data
	std::cout << "\nOUTPUT DATA (" << std::dec << sysVar.raw_output_data.size()
				<< " bytes):" << std::endl;
	print_hex_dump(sysVar.raw_output_data, "OUT");
	// print_data_statistics(sysVar.raw_output_data, "Output");

	// Size comparison
	std::cout << "\nSIZE COMPARISON:" << std::endl;
	std::cout << "Input vector size in header: "
				<< Vector_header.input_vector_size
				<< " bytes, actual: " << sysVar.raw_input_data.size() << " bytes"
				<< std::endl;
	std::cout << "Output vector size in header: "
				<< Vector_header.output_vector_size
				<< " bytes, actual: " << sysVar.raw_output_data.size() << " bytes"
				<< std::endl;

	if (Vector_header.input_vector_size != sysVar.raw_input_data.size()) {
		std::cout << "WARNING: Input size mismatch!" << std::endl;
	}
	if (Vector_header.output_vector_size != sysVar.raw_output_data.size()) {
		std::cout << "WARNING: Output size mismatch!" << std::endl;
	}
}

// HEX data dump
void TestVectorLoader::print_hex_dump(const std::vector<uint8_t> &data,
										const std::string &prefix) const {
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


std::string TestVectorLoader::get_header_field_description(
	uint32_t offset, const TestHeaderDetailed &header) const {
	uint32_t signal_offset;
	uint32_t signal_index;
	uint32_t output_index;

	switch (offset) {
		case HEADER_SIGNATURE_OFFSET:
			return "_SIGNATURE (0x" + to_hex_string(header.signature) + ")";

		case HEADER_MODULE_ID_OFFSET:
			return "_MODULE_ID (0x" + to_hex_string(header.module_id) + ")";

		case HEADER_TOTAL_DATA_LENGTH_OFFSET:
			return "_TOTAL_DATA_LENGTH (" +
					std::to_string(header.total_data_length) + " bytes)";

		case HEADER_INPUT_BASE_ADDR_OFFSET:
			return "_INPUT_BASE_ADDR (0x" +
					to_hex_string(header.input_base_addr) + ")";

		case HEADER_INPUT_VECTOR_SIZE_OFFSET:
			return "_INPUT_VECTOR_SIZE (" +
					std::to_string(header.input_vector_size) + " bytes)";

		case HEADER_OUTPUT_BASE_ADDR_OFFSET:
			return "_OUTPUT_BASE_ADDR (0x" +
					to_hex_string(header.output_base_addr) + ")";

		case HEADER_OUTPUT_VECTOR_SIZE_OFFSET:
			return "_OUTPUT_VECTOR_SIZE (" +
					std::to_string(header.output_vector_size) + " bytes)";

		case HEADER_RESERVED_OFFSET:
			return "_RESERVED (0x" + to_hex_string(header.reserved) + ")";

		case HEADER_SIGNALS_INFO_OFFSET:
			return "_SIGNALS_INFO (in:" +
					std::to_string(header.num_input_signals) +
					" out:" + std::to_string(header.num_output_signals) + ")";

		default:
			if (offset >= HEADER_SIGNAL_WIDTHS_OFFSET) {
				signal_offset = offset - HEADER_SIGNAL_WIDTHS_OFFSET;

				// Check that this is a signal width offset (multiple of 2)
				if (signal_offset % 2 == 0) {
					signal_index = signal_offset / 2;

					if (signal_index < header.input_widths.size()) {
						return "_INPUT_WIDTH[" + std::to_string(signal_index) +
								"] (" +
								std::to_string(
									header.input_widths[signal_index]) +
								" bits)";
					} else {
						output_index =
							signal_index - header.input_widths.size();
						if (output_index < header.output_widths.size()) {
							return "_OUTPUT_WIDTH[" +
									std::to_string(output_index) + "] (" +
									std::to_string(
										header.output_widths[output_index]) +
									" bits)";
						}
					}
				}
			}
			return "UNKNOWN_OFFSET_0x" + to_hex_string(offset);
	}
}


void TestVectorLoader::parse_signal_information() {
	cout << "=== VALIDATING BASIC SIGNAL PART ===" << endl;

	// Read signals info
	uint16_t signals_info = hex_loader.read_halfword_be(
		HEADER_BASE_ADDR + HEADER_SIGNALS_INFO_OFFSET);
	Vector_header.num_input_signals = (signals_info >> 8) & 0xFF;
	Vector_header.num_output_signals = signals_info & 0xFF;

	cout << BLUE << INFO_MARK
			<< "Input signals: " << (int)Vector_header.num_input_signals << NORMAL
			<< endl;
	cout << BLUE << INFO_MARK
			<< "Output signals: " << (int)Vector_header.num_output_signals
			<< NORMAL << endl;

	// Read signal widths
	uint32_t signal_data_addr = HEADER_BASE_ADDR + HEADER_SIGNAL_WIDTHS_OFFSET;

	read_signal_widths(signal_data_addr, Vector_header.num_input_signals,
						"Input", Vector_header.input_widths);

	read_signal_widths(signal_data_addr, Vector_header.num_output_signals,
						"Output", Vector_header.output_widths);
}

bool TestVectorLoader::validate_signal_data() {
	// Validate signal counts
	if (Vector_header.num_input_signals == 0 ||
		Vector_header.num_input_signals > MAX_INPUT_SIGNALS) {
		cout << RED << CROSS_MARK << "ERROR: Invalid input signals count"
				<< endl;
		return false;
	}

	if (Vector_header.num_output_signals == 0 ||
		Vector_header.num_output_signals > MAX_OUTPUT_SIGNALS) {
		cout << RED << CROSS_MARK << "ERROR: Invalid output signals count"
				<< endl;
		return false;
	}

	// Validate signal widths
	for (size_t i = 0; i < Vector_header.input_widths.size(); i++) {
		if (Vector_header.input_widths[i] == 0) {
			cout << RED << CROSS_MARK << "ERROR: Zero width for input signal "
					<< i << endl;
			return false;
		}
	}

	for (size_t i = 0; i < Vector_header.output_widths.size(); i++) {
		if (Vector_header.output_widths[i] == 0) {
			cout << RED << CROSS_MARK << "ERROR: Zero width for output signal "
					<< i << endl;
			return false;
		}
	}

	cout << GREEN << CHECK_MARK << "Signal data validation passed" << NORMAL
			<< endl;
	return true;
}

void TestVectorLoader::perform_final_validation() {
	cout << "\n--- FINAL VALIDATION ---" << endl;

	// Helper function to compute offsets
	auto calculate_offsets = [](const std::vector<uint16_t> &widths)
		-> std::vector<std::pair<uint32_t, uint32_t>> {	 // pairs {offset,
															// size in bytes}
		std::vector<std::pair<uint32_t, uint32_t>> result;
		uint32_t offset{0}, size_bytes{0};

		for (uint32_t width : widths) {
			size_bytes = (width + 7) / CHAR_BIT;
			result.emplace_back(offset, size_bytes);
			offset += size_bytes;
		}

		return result;
	};

	// Compute offsets for input and output signals
	auto input_offsets = calculate_offsets(Vector_header.input_widths);
	auto output_offsets = calculate_offsets(Vector_header.output_widths);

	// Detailed info for each input signal
	cout << "\n=== INPUT SIGNALS ===" << endl;
	cout << "Index | Bits | Bytes | Offset (hex) | Offset (dec) | Range"
			<< endl;
	cout << "------|------|-------|--------------|--------------|--------------"
			"--"
			<< endl;

	uint32_t bits, bytes, offset, end_offset;

	for (size_t i = 0; i < Vector_header.input_widths.size(); ++i) {
		bits = Vector_header.input_widths[i];
		bytes = input_offsets[i].second;
		offset = input_offsets[i].first;
		end_offset = offset + bytes - 1;

		cout << setw(5) << i << " | " << setw(4) << bits << " | " << setw(5)
				<< bytes << " | " << "0x" << setw(10) << setfill('0') << hex
				<< offset << dec << setfill(' ') << " | " << setw(12) << offset
				<< " | "
				<< "[" << offset << ":" << end_offset << "]";

		if (bits > 0 && bits < 8) {
			cout << " (padded)";
		}
		cout << endl;
	}

	// Detailed info for each output signal
	cout << "\n=== OUTPUT SIGNALS ===" << endl;
	cout << "Index | Bits | Bytes | Offset (hex) | Offset (dec) | Range"
			<< endl;
	cout << "------|------|-------|--------------|--------------|--------------"
			"--"
			<< endl;

	for (size_t i = 0; i < Vector_header.output_widths.size(); ++i) {
		bits = Vector_header.output_widths[i];
		bytes = output_offsets[i].second;
		offset = output_offsets[i].first;
		end_offset = offset + bytes - 1;

		cout << setw(5) << i << " | " << setw(4) << bits << " | " << setw(5)
				<< bytes << " | " << "0x" << setw(10) << setfill('0') << hex
				<< offset << dec << setfill(' ') << " | " << setw(12) << offset
				<< " | "
				<< "[" << offset << ":" << end_offset << "]";

		if (bits > 0 && bits < CHAR_BIT) {
			cout << " (padded)";
		}
		cout << endl;
	}

	// Summary information
	uint32_t calc_input_bytes = Vector_header.total_input_bytes();
	uint32_t calc_output_bytes = Vector_header.total_output_bytes();

	cout << "\n=== SUMMARY ===" << endl;
	cout << "Total input size:  " << calc_input_bytes << " bytes (0x" << hex
			<< calc_input_bytes << dec << ")" << endl;
	cout << "Total output size: " << calc_output_bytes << " bytes (0x" << hex
			<< calc_output_bytes << dec << ")" << endl;

	// TODO: Add calculation accounting for the number of vectors
	//  cout << "\nTotal memory range:" << endl;
	//  cout << "  Input:  [0x" << hex << 0 << " - 0x" << (calc_input_bytes - 1)
	//  	 << "] (" << dec << calc_input_bytes << " bytes)" << endl;
	//  cout << "  Output: [0x" << hex << 0 << " - 0x" << (calc_output_bytes -
	//  1)
	//  	 << "] (" << dec << calc_output_bytes << " bytes)" << endl;

	uint32_t aligned_32bit;

	// For memory-handling convenience, also print aligned addresses
	cout << "\n=== ALIGNED ADDRESSES (for 32-bit access) ===" << endl;
	cout << "Input region:" << endl;
	for (size_t i = 0; i < Vector_header.input_widths.size(); ++i) {
		offset = input_offsets[i].first;
		aligned_32bit = (offset + 3) & ~3;	// Align to 4 bytes
		if (aligned_32bit != offset) {
			cout << "  Signal[" << i << "]: offset 0x" << hex << offset
					<< " -> aligned 0x" << aligned_32bit << " (+" << dec
					<< (aligned_32bit - offset) << " bytes padding)" << endl;
		}
	}

	// Validation
	cout << "\n=== VALIDATION ===" << endl;
	cout << "Expected input size:  " << Vector_header.input_vector_size
			<< " bytes" << endl;
	cout << "Calculated input size: " << calc_input_bytes << " bytes" << endl;
	cout << "Expected output size: " << Vector_header.output_vector_size
			<< " bytes" << endl;
	cout << "Calculated output size: " << calc_output_bytes << " bytes" << endl;

	bool input_match = (calc_input_bytes == Vector_header.input_vector_size);
	bool output_match = (calc_output_bytes == Vector_header.output_vector_size);

	if (!input_match || !output_match) {
		cout << RED << "\nERROR: Size mismatch detected!" << NORMAL << endl;

		if (!input_match) {
			cout << "Input: expected " << Vector_header.input_vector_size
					<< ", got " << calc_input_bytes << " (difference: "
					<< (int)calc_input_bytes - (int)Vector_header.input_vector_size
					<< ")" << endl;

			// Detailed analysis for input signals
			cout << "\nInput signals analysis:" << endl;
			for (size_t i = 0; i < Vector_header.input_widths.size(); ++i) {
				uint32_t bits = Vector_header.input_widths[i];
				if (bits < CHAR_BIT) {
					cout << "  Signal[" << i << "]: " << bits
							<< " bits -> 1 byte (padded)" << endl;
				}
			}
		}

		if (!output_match) {
			cout << "Output: expected " << Vector_header.output_vector_size
					<< ", got " << calc_output_bytes << " (difference: "
					<< (int)calc_output_bytes -
						(int)Vector_header.output_vector_size
					<< ")" << endl;

			// Detailed analysis for output signals
			cout << "\nOutput signals analysis:" << endl;
			for (size_t i = 0; i < Vector_header.output_widths.size(); ++i) {
				uint32_t bits = Vector_header.output_widths[i];
				if (bits < CHAR_BIT) {
					cout << "  Signal[" << i << "]: " << bits
							<< " bits -> 1 byte (padded)" << endl;
				}
			}
		}

		throw std::runtime_error("Size mismatch in validation");
	}

	cout << GREEN << CHECK_MARK << " Final validation passed" << NORMAL << endl;
#ifdef USING_CODE_GENERATION
	// Additionally: print offsets in a copy-paste-friendly format
	cout << "\n=== OFFSETS FOR CODE GENERATION ===" << endl;
	cout << "// Input signal offsets:" << endl;
	for (size_t i = 0; i < input_offsets.size(); ++i) {
		cout << "INPUT_" << i << "_OFFSET = 0x" << hex << input_offsets[i].first
				<< dec << ";" << endl;
	}

	cout << "\n// Output signal offsets:" << endl;
	for (size_t i = 0; i < output_offsets.size(); ++i) {
		cout << "OUTPUT_" << i << "_OFFSET = 0x" << hex
				<< output_offsets[i].first << dec << ";" << endl;
	}
#endif
}

// ============================================================================
// VECTOR PARSING - REFACTORED
// ============================================================================

void TestVectorLoader::preview_data_block(uint32_t address, uint32_t size,
											const string &prefix) {
	uint32_t bytes_to_read = std::min(sysVar.PREVIEW_BYTES, size);

	if (bytes_to_read == 0) {
		cout << YELLOW << "  No data (size = 0)" << NORMAL << endl;
		return;
	}

	try {
		vector<uint8_t> data = hex_loader.read_block(address, bytes_to_read);
		print_hex_dump(data, prefix);

		if (bytes_to_read < size) {
			cout << YELLOW << "  ... " << (size - bytes_to_read)
					<< " more bytes (preview limit: " << sysVar.PREVIEW_BYTES
					<< ")" << NORMAL << endl;
		}
	} catch (const exception &e) {
		cerr << RED << "  ERROR reading data: " << e.what() << NORMAL << endl;
	}
}

void TestVectorLoader::print_vector_summary(uint32_t vector_num,
											uint32_t input_addr,
											uint32_t output_addr) {
	cout << CYAN << "\n--- SUMMARY ---" << NORMAL << endl;
	cout << "Vector:         " << vector_num << endl;
	cout << "Input addr:     0x" << hex << input_addr << dec << endl;
	cout << "Output addr:    0x" << hex << output_addr << dec << endl;
	cout << "Input size:     " << Vector_header.input_vector_size << " bytes"
			<< endl;
	cout << "Output size:    " << Vector_header.output_vector_size << " bytes"
			<< endl;

	// Next vector
	if (vector_num + 1 < sysVar.total_vector_count) {
		cout << "Next input:     0x" << hex
				<< (input_addr + Vector_header.input_vector_size) << dec << endl;
		cout << "Next output:    0x" << hex
				<< (output_addr + Vector_header.output_vector_size) << dec << endl;
	}
}

// TODO: Consolidate or remove.
// Similar functionality exists

void TestVectorLoader::debug_print_enhanced_signal_analysis(
	const TestHeaderDetailed &header) {
#ifdef TEST_VECTOR_DEBUG
	// ENHANCED SIGNAL ANALYSIS
	cout << "DEBUG: ===== ENHANCED SIGNAL ANALYSIS =====" << endl;
	cout << "DEBUG: Signal widths memory layout:" << endl;
	cout << "DEBUG: Input signals: " << (int)header.num_input_signals << endl;
	cout << "DEBUG: Output signals: " << (int)header.num_output_signals << endl;

	uint32_t signal_area_start = HEADER_BASE_ADDR + HEADER_SIGNAL_WIDTHS_OFFSET;
	uint32_t total_signal_bytes =
		(header.num_input_signals + header.num_output_signals) * 2;
	uint32_t total_signal_words = (total_signal_bytes + 3) / 4;

	cout << "DEBUG: Signal area: 0x" << std::hex << signal_area_start << " - 0x"
			<< (signal_area_start + total_signal_bytes - 1) << " ("
			<< total_signal_bytes << " bytes, " << total_signal_words << " words)"
			<< std::dec << endl;

	for (uint32_t word_idx = 0; word_idx < total_signal_words; word_idx++) {
		uint32_t addr = signal_area_start + word_idx * 4;
		uint32_t data = hex_loader.read_word_be(addr);

		cout << "DEBUG: Word " << word_idx << " at 0x" << std::hex << addr
				<< ": 0x" << data << std::dec << " -> ";

		// Analyze 2 bytes at a time
		for (int byte_pair = 0; byte_pair < 2; byte_pair++) {
			uint32_t signal_offset = word_idx * 4 + byte_pair * 2;
			if (signal_offset < total_signal_bytes) {
				uint16_t width_value =
					(data >> (16 * (1 - byte_pair))) & 0xFFFF;

				if (signal_offset < header.num_input_signals * 2) {
					uint32_t signal_idx = signal_offset / 2;
					cout << "IN[" << signal_idx << "]=" << width_value << "b ";
				} else {
					uint32_t output_offset =
						signal_offset - header.num_input_signals * 2;
					uint32_t signal_idx = output_offset / 2;
					cout << "OUT[" << signal_idx << "]=" << width_value << "b ";
				}
			}
		}
		cout << endl;
	}

	cout << "DEBUG: ===== END ENHANCED ANALYSIS =====" << endl;
#endif
}

// Method to calculate the total test vector count
bool TestVectorLoader::calculate_total_test_vector_count() {
	// Validate the header
	if (Vector_header.total_data_length == 0) {
		cout << RED << CROSS_MARK << "ERROR: Invalid total data length field"
				<< endl;
		return false;
	}

	// Check vector sizes
	if (Vector_header.input_vector_size == 0 ||
		Vector_header.output_vector_size == 0) {
		cout << RED << CROSS_MARK << "ERROR: Zero vector size" << endl;
		return false;
	}

	// Total size of one vector pair (input + output)
	uint32_t vector_pair_size =
		Vector_header.input_vector_size + Vector_header.output_vector_size;

	// Check for division by zero (extra safety)
	if (vector_pair_size == 0) {
		cout << RED << CROSS_MARK << "ERROR: Combined vector size is zero"
				<< endl;
		return false;
	}

	// Calculate the number of complete vector pairs
	this->sysVar.total_vector_count =
		Vector_header.total_data_length / vector_pair_size;

	// Verify the data divides without remainder
	if (Vector_header.total_data_length % vector_pair_size != 0) {
		cout << " WARNING: Data length " << Vector_header.total_data_length
				<< " not divisible by vector pair size " << vector_pair_size
				<< " (remainder: "
				<< Vector_header.total_data_length % vector_pair_size << ")"
				<< endl;
		// Could round down or report an error
	}

#ifdef TEST_VECTOR_DEBUG
	cout << "DEBUG: Vector calculation:" << endl;
	cout << "  Total data: " << Vector_header.total_data_length << " bytes"
			<< endl;
	cout << "  Input vector: " << Vector_header.input_vector_size << " bytes"
			<< endl;
	cout << "  Output vector: " << Vector_header.output_vector_size << " bytes"
			<< endl;
	cout << "  Vector pair: " << vector_pair_size << " bytes" << endl;
	cout << "  Total vectors: " << this->sysVar.total_vector_count << endl;
#endif

	if (sysVar.total_vector_count > 0) {
		sysVar.total_vector_count_is_valid = true;
		return true;
	}
	return false;
}

// ============================================================================
// HELPER METHODS
// ============================================================================
void TestVectorLoader::validate_read_operation(uint32_t vector_num) const {
	if (!header_is_valid_) {
		throw std::runtime_error("Header not valid");
	}

	if (!sysVar.total_vector_count_is_valid) {
		throw std::runtime_error("Total vector count not calculated");
	}

	if (vector_num >= sysVar.total_vector_count) {
		std::string msg = "Vector number " + std::to_string(vector_num) +
							" out of range (0-" +
							std::to_string(sysVar.total_vector_count - 1) + ")";
		throw std::runtime_error(msg);
	}
}

uint32_t TestVectorLoader::calculate_vector_number(uint32_t input_addr,
													uint32_t output_addr) const {
	if (!header_is_valid_) {
		throw std::runtime_error("Header not valid");
	}

	// Try to determine from the input address
	if (Vector_header.input_vector_size > 0 &&
		input_addr >= Vector_header.input_base_addr) {
		uint32_t offset = input_addr - Vector_header.input_base_addr;
		if (offset % Vector_header.input_vector_size == 0) {
			return offset / Vector_header.input_vector_size;
		}
	}

	// Try to determine from the output address
	if (Vector_header.output_vector_size > 0 &&
		output_addr >= Vector_header.output_base_addr) {
		uint32_t offset = output_addr - Vector_header.output_base_addr;
		if (offset % Vector_header.output_vector_size == 0) {
			return offset / Vector_header.output_vector_size;
		}
	}

	throw std::runtime_error("Cannot determine vector number from addresses");
}

// ============================================================================
// MAIN READING METHODS
// ============================================================================

bool TestVectorLoader::read_vector(uint32_t vector_num, TestVector &result) {
	try {
		validate_read_operation(vector_num);

		// Calculate addresses
		uint32_t input_addr = Vector_header.input_base_addr +
								(vector_num * Vector_header.input_vector_size);
		uint32_t output_addr = Vector_header.output_base_addr +
								(vector_num * Vector_header.output_vector_size);

		return read_vector_internal(input_addr, output_addr, result,
									vector_num);

	} catch (const std::exception &e) {
		cerr << RED << "ERROR reading vector " << vector_num << ": " << e.what()
				<< NORMAL << endl;
		return false;
	}
}

bool TestVectorLoader::read_vector_internal(uint32_t input_addr,
											uint32_t output_addr,
											TestVector &result,
											uint32_t vector_num) {
	log_debug("Reading vector data", true);

	if (vector_num != UINT32_MAX) {
		log_debug("Vector: " + to_hex_string(vector_num));
	}

	log_debug("Input: 0x" + to_hex_string(input_addr) + " (" +
				std::to_string(Vector_header.input_vector_size) + " bytes)");
	log_debug("Output: 0x" + to_hex_string(output_addr) + " (" +
				std::to_string(Vector_header.output_vector_size) + " bytes)");

	// Clear the result
	result.clear();

	// Read input data
	try {
		// cout << "  Reading input data..." << endl;
		result.input_data =
			hex_loader.read_block(input_addr, Vector_header.input_vector_size);
		// cout << "  Input data size: " << result.input_data.size() << " bytes"
		// << endl;
	} catch (const std::exception &e) {
		cerr << RED << "ERROR reading input data: " << e.what() << NORMAL
				<< endl;
		return false;
	}

	// Read output data
	try {
		// cout << "  Reading output data..." << endl;
		result.output_data = hex_loader.read_block(
			output_addr, Vector_header.output_vector_size);
		// cout << "  Output data size: " << result.output_data.size() << "
		// bytes" << endl;
	} catch (const std::exception &e) {
		cerr << RED << "ERROR reading output data: " << e.what() << NORMAL
				<< endl;
		return false;
	}

	// Check sizes
	if (result.input_data.size() != Vector_header.input_vector_size) {
		cerr << YELLOW << "WARNING: Input size mismatch. Expected: "
				<< Vector_header.input_vector_size
				<< ", Got: " << result.input_data.size() << NORMAL << endl;
	}

	if (result.output_data.size() != Vector_header.output_vector_size) {
		cerr << YELLOW << "WARNING: Output size mismatch. Expected: "
				<< Vector_header.output_vector_size
				<< ", Got: " << result.output_data.size() << NORMAL << endl;
	}

	// Set metadata
	result.vector_number = vector_num;
	if (vector_num != UINT32_MAX) {
		result.vector_id = vector_num;	// Or generate a unique ID
	}
	result.valid = true;

	// Store into system variables for debugging
	sysVar.raw_input_data = result.input_data;
	sysVar.raw_output_data = result.output_data;
	sysVar.input_addr_read_offset = input_addr;
	sysVar.output_addr_read_offset = output_addr;

	log_debug("Vector read successfully", true);
	log_debug("Input: " + std::to_string(result.input_data.size()) + " bytes");
	log_debug("Output: " + std::to_string(result.output_data.size()) +
				" bytes");

	return true;
}

bool TestVectorLoader::read_vector_by_address(uint32_t input_addr,
												uint32_t output_addr,
												TestVector &result) {
	try {
		if (!header_is_valid_) {
			throw std::runtime_error("Header not valid");
		}

		// Try to determine the vector number
		uint32_t vector_num = UINT32_MAX;
		try {
			vector_num = calculate_vector_number(input_addr, output_addr);
		} catch (...) {
			// If it cannot be determined - that is fine, read by addresses
		}

		return read_vector_internal(input_addr, output_addr, result,
									vector_num);

	} catch (const std::exception &e) {
		cerr << RED << "ERROR reading vector by address: " << e.what() << NORMAL
				<< endl;
		return false;
	}
}

// ============================================================================
// BATCH READING
// ============================================================================

bool TestVectorLoader::read_vector_range(uint32_t start, uint32_t end,
											std::vector<TestVector> &results,
											bool verbose_mode) {
	if (start > end) {
		log_error("Invalid range: " + std::to_string(start) + " > " +
					std::to_string(end));
		return false;
	}

	try {
		validate_read_operation(start);
		if (end >= sysVar.total_vector_count) {
			throw std::runtime_error("End index out of range");
		}

		results.clear();
		results.reserve(end - start + 1);

		log_info("Reading vectors " + std::to_string(start) + " to " +
					std::to_string(end) + " (" + std::to_string(end - start + 1) +
					" vectors)");

		int last_percent_shown = -1;

		for (uint32_t i = start; i <= end; i++) {
			TestVector vec;
			if (!read_vector(i, vec)) {
				log_error("Failed to read vector " + std::to_string(i));
				return false;
			}
			results.push_back(std::move(vec));

			if (verbose_mode) {
				// Show progress
				int percent = ((i - start + 1) * 100) / (end - start + 1);

				// Show every 100 vectors or every percent (whichever is more
				// frequent)
				if ((i - start) % 100 == 0 || percent != last_percent_shown ||
					i == end) {
					last_percent_shown = percent;

					// Formatted progress output
					std::stringstream progress_msg;
					progress_msg << "Progress: " << std::setw(6)
									<< (i - start + 1) << "/" << std::setw(6)
									<< (end - start + 1) << " (" << std::setw(3)
									<< percent << "%)";

					log_info(progress_msg.str());
				}
			}
		}
		log_info("Read " + std::to_string(results.size()) + " vectors");

		return true;

	} catch (const std::exception &e) {
		log_error("Error reading vector range: " + std::string(e.what()));
		return false;
	}
}

bool TestVectorLoader::read_all_vectors(std::vector<TestVector> &results) {
	if (!sysVar.total_vector_count_is_valid || sysVar.total_vector_count == 0) {
		log_error("No vectors available or total count not calculated");
		return false;
	}

	log_info("Reading all " + std::to_string(sysVar.total_vector_count) +
				" vectors");
	return read_vector_range(0, sysVar.total_vector_count - 1, results);
}

// ============================================================================
// PREVIEW METHODS (NON-DESTRUCTIVE)
// ============================================================================

void TestVectorLoader::preview_vector(uint32_t vector_num, bool show_summary) {
	try {
		validate_read_operation(vector_num);

		uint32_t input_addr = Vector_header.input_base_addr +
								(vector_num * Vector_header.input_vector_size);
		uint32_t output_addr = Vector_header.output_base_addr +
								(vector_num * Vector_header.output_vector_size);

		preview_vector_by_address(input_addr, output_addr, show_summary);

	} catch (const std::exception &e) {
		cerr << RED << CROSS_MARK << "ERROR previewing vector: " << e.what()
				<< NORMAL << endl;
	}
}

void TestVectorLoader::print_preview_summary(uint32_t input_addr,
												uint32_t output_addr) {
	cout << CYAN << INFO_MARK << "\n--- PREVIEW SUMMARY ---" << NORMAL << endl;

	cout << "Input address:  0x" << hex << input_addr << dec << endl;
	cout << "Output address: 0x" << hex << output_addr << dec << endl;

	// Offsets from base addresses
	if (input_addr >= Vector_header.input_base_addr) {
		cout << "Input offset:    "
				<< (input_addr - Vector_header.input_base_addr) << " bytes"
				<< endl;
	}

	if (output_addr >= Vector_header.output_base_addr) {
		cout << "Output offset:   "
				<< (output_addr - Vector_header.output_base_addr) << " bytes"
				<< endl;
	}

	cout << "Input size:     " << Vector_header.input_vector_size << " bytes"
			<< endl;
	cout << "Output size:    " << Vector_header.output_vector_size << " bytes"
			<< endl;
	cout << "Preview limit:  " << sysVar.PREVIEW_BYTES << " bytes" << endl;

	// Percentage of data shown
	if (Vector_header.input_vector_size > 0) {
		uint32_t preview_in =
			std::min(sysVar.PREVIEW_BYTES, Vector_header.input_vector_size);
		double percent_in =
			(preview_in * 100.0) / Vector_header.input_vector_size;
		cout << "Input shown:     " << fixed << setprecision(1) << percent_in
				<< "%" << endl;
	}

	if (Vector_header.output_vector_size > 0) {
		uint32_t preview_out =
			std::min(sysVar.PREVIEW_BYTES, Vector_header.output_vector_size);
		double percent_out =
			(preview_out * 100.0) / Vector_header.output_vector_size;
		cout << "Output shown:    " << fixed << setprecision(1) << percent_out
				<< "%" << endl;
	}
}

void TestVectorLoader::preview_vector_by_address(uint32_t input_addr,
													uint32_t output_addr,
													bool show_summary) {
	cout << CYAN << "\n=== VECTOR PREVIEW ===" << NORMAL << endl;

	// Try to determine the vector number
	try {
		uint32_t vector_num = calculate_vector_number(input_addr, output_addr);
		cout << "Vector number: " << vector_num << endl;

		if (sysVar.total_vector_count_is_valid) {
			cout << "Position:       " << (vector_num + 1) << " of "
					<< sysVar.total_vector_count << endl;
		}
	} catch (...) {
		cout << YELLOW << WARNING_MARK
				<< "Note: Vector number unknown (reading by address)" << NORMAL
				<< endl;
	}

	// Preview input data
	cout << BLUE << INFO_MARK << "--- INPUT DATA ---" << NORMAL << endl;
	cout << "Address: 0x" << hex << input_addr << dec << endl;
	cout << "Size:    " << Vector_header.input_vector_size << " bytes" << endl;

	if (Vector_header.input_vector_size > 0) {
		preview_data_block(input_addr, Vector_header.input_vector_size, "IN");
	} else {
		cout << YELLOW << WARNING_MARK << "No input data (size = 0)" << NORMAL
				<< endl;
	}

	// Preview output data
	cout << MAGENTA << "\n--- OUTPUT DATA ---" << NORMAL << endl;
	cout << "Address: 0x" << hex << output_addr << dec << endl;
	cout << "Size:    " << Vector_header.output_vector_size << " bytes" << endl;

	if (Vector_header.output_vector_size > 0) {
		preview_data_block(output_addr, Vector_header.output_vector_size,
							"OUT");
	} else {
		cout << YELLOW << WARNING_MARK << "No output data (size = 0)" << NORMAL
				<< endl;
	}

	if (show_summary) {
		// TODO:Add implementation
		cout << BLUE << INFO_MARK << "show summary is:\t" << show_summary
				<< NORMAL << endl;
		// print_preview_summary(input_addr, output_addr);
	}

	cout << CYAN << "=== END PREVIEW ===" << NORMAL << endl;
}

// ============================================================================
// DATA BLOCK METHODS
// ============================================================================

std::vector<uint8_t> TestVectorLoader::read_data_block(uint32_t address,
														uint32_t size) {
	if (size == 0) {
		return {};
	}
	try {
		return hex_loader.read_block(address, size);
	} catch (const std::exception &e) {
		cerr << RED << "ERROR reading data block at 0x" << hex << address
				<< " (size: " << dec << size << "): " << e.what() << NORMAL
				<< endl;
		throw;
	}
}

bool TestVectorLoader::read_data_block(uint32_t address, uint32_t size,
										std::vector<uint8_t> &result) {
	try {
		result = read_data_block(address, size);
		return true;
	} catch (...) {
		return false;
	}
}
