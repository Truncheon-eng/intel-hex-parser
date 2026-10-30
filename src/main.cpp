#include <iostream>
#include <fstream>
#include <iomanip>
#include <chrono>

#include "hex_loader.h"
#include "TestVectorLoader.hpp"
#include "HexDumper.h"

using namespace std;

// Usage output function
void print_usage(const char *program_name) {
	cout << "Usage: " << program_name << " <hex_file> [options]" << endl;
	cout << "\nOptions:" << endl;
	cout << "  -v, --verbose     Enable verbose output" << endl;
	cout << "  -e, --errors      Show parser errors in detail" << endl;
	cout << "  -w, --warnings    Show parser warnings in detail" << endl;
	cout << "  -a, --all         Show all parser messages" << endl;
	cout << "  -s, --stats       Show only statistics" << endl;
	cout << "  -h, --help        Show this help message" << endl;
}

// Command-line argument handling function
struct ProgramOptions {
	string hex_file;
	bool verbose = false;
	bool show_errors = true;
	bool show_warnings = true;
	bool show_all_messages = false;
	bool show_only_stats = false;
};

ProgramOptions parse_arguments(int argc, char **argv) {
	ProgramOptions options;

	if (argc < 2) {
		print_usage(argv[0]);
		exit(1);
	}

	options.hex_file = argv[1];

	for (int i = 2; i < argc; i++) {
		string arg = argv[i];

		if (arg == "-v" || arg == "--verbose") {
			options.verbose = true;
		} else if (arg == "-e" || arg == "--errors") {
			options.show_errors = true;
			options.show_warnings = false;
		} else if (arg == "-w" || arg == "--warnings") {
			options.show_errors = false;
			options.show_warnings = true;
		} else if (arg == "-a" || arg == "--all") {
			options.show_all_messages = true;
		} else if (arg == "-s" || arg == "--stats") {
			options.show_only_stats = true;
			options.show_errors = false;
			options.show_warnings = false;
		} else if (arg == "-h" || arg == "--help") {
			print_usage(argv[0]);
			exit(0);
		} else {
			cerr << "Unknown option: " << arg << endl;
			print_usage(argv[0]);
			exit(1);
		}
	}

	return options;
}

int main(int argc, char **argv) {
	// Parse command-line arguments
	ProgramOptions options = parse_arguments(argc, argv);

	cout << "=== HEX FILE ANALYZER ===" << endl;
	cout << "File: " << options.hex_file << endl;
	cout << string(40, '=') << endl;

	// Temporarily unused during the error handler debugging phase
	// Parse test vectors using `TestVectorLoader`

	HexLoader hex_loader;

	cout << "Start Load file\t" << options.hex_file << endl;
	if (!hex_loader.load_file(options.hex_file, true)) {
		cerr << "Failed to load HEX file!" << endl;

		// Show load errors
		hex_loader.print_all_parser_messages(true, false);

		// Check whether data exists despite errors
		if (hex_loader.empty()) {
			return EXIT_FAILURE;
		} else {
			cout << "Warning: File loaded with errors, but contains some data."
					<< endl;
		}
	}

	cout << "\n=== Test Vector Analysis ===" << endl;
	TestVectorLoader vector_loader(hex_loader);
	vector_loader.set_log_level(TestVectorLoader::LOG_LEVEL_INFO);

	// Parse the header first for validation
	vector_loader.parse_header();

	if (!vector_loader.headerIsCorrect()) {
		cout << "Invalid test header" << endl;
		return EXIT_FAILURE;
	}

	uint32_t total_vec_cnt = 0;
	try {
		total_vec_cnt = vector_loader.get_total_vector_count();
	} catch (const std::exception &e) {
		cerr << RED << CROSS_MARK << e.what() << NORMAL << endl;
		return EXIT_FAILURE;
	}
	cout << "Total test vectors: " << total_vec_cnt << endl;

	constexpr uint32_t first_vec_count = 5;
	constexpr uint32_t last_vec_count = 5;

	if (first_vec_count > 0 && first_vec_count <= total_vec_cnt) {
		cout << "\nFirst " << first_vec_count << " vectors:" << endl;
		for (uint32_t i = 0; i < first_vec_count; i++) {
			vector_loader.preview_vector(i, false);
		}
	}

	if (last_vec_count > 0 && total_vec_cnt > last_vec_count) {
		cout << "\nLast " << last_vec_count << " vectors:" << endl;
		for (uint32_t i = total_vec_cnt - last_vec_count; i < total_vec_cnt;
				i++) {
			vector_loader.preview_vector(i, false);
		}
	}

	std::vector<TestVector> testReadAllVec;
	testReadAllVec.reserve(total_vec_cnt);

	cout << YELLOW << INFO_MARK << "Start read ALL vectors!\t" << NORMAL
			<< endl;
	bool success = vector_loader.read_all_vectors(testReadAllVec);

	if (success) {
		cout << GREEN << CHECK_MARK << "All Vector SUCCESSFUL read!" << NORMAL
				<< endl;

		// Statistics
		cout << "\n" << CYAN << "=== LOADING STATISTICS ===" << NORMAL << endl;
		cout << "Total vectors loaded: " << testReadAllVec.size() << endl;

		if (!testReadAllVec.empty()) {
			// Check the first vector to obtain sizes
			const auto &first_vec = testReadAllVec[0];
			cout << "Input vector size:  " << first_vec.input_data.size()
					<< " bytes" << endl;
			cout << "Output vector size: " << first_vec.output_data.size()
					<< " bytes" << endl;

			// Count the total data volume
			size_t total_input_bytes = 0;
			size_t total_output_bytes = 0;
			size_t vectors_with_input = 0;
			size_t vectors_with_output = 0;

			for (const auto &vec : testReadAllVec) {
				total_input_bytes += vec.input_data.size();
				total_output_bytes += vec.output_data.size();
				if (!vec.input_data.empty()) {
					vectors_with_input++;
				}
				if (!vec.output_data.empty()) {
					vectors_with_output++;
				}
			}

			cout << "Total input data:   " << total_input_bytes << " bytes"
					<< endl;
			cout << "Total output data:  " << total_output_bytes << " bytes"
					<< endl;
			cout << "Vectors with input: " << vectors_with_input << endl;
			cout << "Vectors with output:" << vectors_with_output << endl;

			// Show the first 5 vectors for verification
			cout << "\n"
					<< CYAN << "=== FIRST 5 INPUT VECTORS ===" << NORMAL << endl;

			for (uint32_t i = 0;
					i < min((uint32_t)5, (uint32_t)testReadAllVec.size()); i++) {
				const auto &vec = testReadAllVec[i];
				cout << "\nVector " << i << " (input, " << vec.input_data.size()
						<< " bytes):" << endl;

				if (!vec.input_data.empty()) {
					// Formatted HEX output
					for (uint32_t j = 0; j < vec.input_data.size(); j++) {
						if (j % 16 == 0 && j > 0) {
							cout << endl;
						}
						if (j % 16 == 0) {
							cout << "  " << hex << setw(4) << setfill('0') << j
									<< ": ";
						}
						cout << hex << setw(2) << setfill('0')
								<< (uint32_t)vec.input_data[j] << " ";
					}
					cout << dec << endl;

					// Brief vector information
					if (vec.input_data.size() >= 2) {
						uint16_t last_two_bytes =
							(vec.input_data[vec.input_data.size() - 2] << 8) |
							vec.input_data[vec.input_data.size() - 1];
						cout << "  Last 2 bytes: 0x" << hex << setw(4)
								<< setfill('0') << last_two_bytes << dec << endl;
					}
				} else {
					cout << RED << CROSS_MARK << "  Empty input data" << NORMAL
							<< endl;
				}
			}

			// Show output data of the first 5 vectors
			cout << "\n"
					<< CYAN << "=== FIRST 5 OUTPUT VECTORS ===" << NORMAL << endl;
			for (uint32_t i = 0;
					i < min((uint32_t)5, (uint32_t)testReadAllVec.size()); i++) {
				const auto &vec = testReadAllVec[i];
				cout << "\nVector " << i << " (output, "
						<< vec.output_data.size() << " bytes):" << endl;

				if (!vec.output_data.empty()) {
					// Formatted HEX output
					for (uint32_t j = 0;
							j <
							min((uint32_t)16, (uint32_t)vec.output_data.size());
							j++) {
						if (j % 16 == 0 && j > 0) {
							cout << endl;
						}
						if (j % 16 == 0) {
							cout << "  " << hex << setw(4) << setfill('0') << j
									<< ": ";
						}
						cout << hex << setw(2) << setfill('0')
								<< (uint32_t)vec.output_data[j] << " ";
					}

					if (vec.output_data.size() > 16) {
						cout << "\n  ... and " << (vec.output_data.size() - 16)
								<< " more bytes";
					}
					cout << dec << endl;
				} else {
					cout << YELLOW << "  No output data" << NORMAL << endl;
				}
			}

			// Pattern analysis in the first 10 vectors
			// cout << "\n" << CYAN << "=== PATTERN ANALYSIS (first 10 vectors)
			// ===" << NORMAL << endl; for (uint32_t i = 0; i <
			// min((uint32_t)10, (uint32_t)testReadAllVec.size()); i++) { const
			// auto& vec = testReadAllVec[i];

			// 	if (!vec.input_data.empty()) {
			// 		// Zero count
			// 		size_t zero_count = 0;
			// 		for (uint8_t byte : vec.input_data) {
			// 			if (byte == 0x00) zero_count++;
			// 		}

			// 		double zero_percent = (vec.input_data.size() > 0) ?
			// 			(zero_count * 100.0 / vec.input_data.size()) : 0;

			// 		cout << "Vector " << setw(2) << i << ": "
			// 			 << setw(6) << zero_count << "/" << setw(2) <<
			// vec.input_data.size()
			// 			 << " zeros (" << fixed << setprecision(1) << setw(5) <<
			// zero_percent << "%)";

			// 		// Check the last 2 bytes
			// 		if (vec.input_data.size() >= 2) {
			// 			uint8_t last_byte =
			// vec.input_data[vec.input_data.size()-1]; 			uint8_t
			// second_last_byte = vec.input_data[vec.input_data.size()-2];
			// cout << "  End: 0x" << hex << setw(2) << setfill('0')
			// 				 << (uint32_t)second_last_byte
			// 				 << setw(2) << setfill('0') << (uint32_t)last_byte
			// << dec;
			// 		}

			// 		cout << endl;
			// 	}
			// }

		} else {
			cout << RED << CROSS_MARK << "No vectors loaded (empty vector)"
					<< NORMAL << endl;
		}

	} else {
		cerr << RED << CROSS_MARK << "All Vector FAILURE read!" << NORMAL
				<< endl;
		return EXIT_FAILURE;
	}

	cout << "\n"
			<< GREEN << CHECK_MARK << "Vector loading completed successfully!"
			<< NORMAL << endl;


	// Print information about the loaded data
	cout << "\n=== Loaded Data Information ===" << endl;
	hex_loader.print_info();

	// Create `HexDumper` and run the analysis
	cout << "\n=== Data Analysis ===" << endl;
	HexDumper hex_dumper(hex_loader);

	// Add a warning if there are parser errors
	if (hex_loader.has_parser_errors()) {
		cout << "⚠ Note: Analysis is performed on data with parser errors."
				<< endl;
		cout << "Some data may be missing or corrupted." << endl;
	}

	// Run the analysis
	hex_dumper.print_statistics();

	// Additional analyses if needed
	if (options.verbose) {
		cout << "\n=== Additional Analysis ===" << endl;

		// Gap analysis with visualization
		vector<uint32_t> addresses = hex_dumper.get_all_addresses();
		if (!addresses.empty()) {
			// Visualization
			uint32_t min_addr = addresses.front();
			uint32_t max_addr = addresses.back();

			// Visualize only if the range is not too large
			if ((max_addr - min_addr) < 1000000) {	// Less than 1MB
				cout << "\nMemory map visualization:" << endl;
				hex_dumper.visualize_gaps(addresses, min_addr, max_addr, 80);
			} else {
				cout << "\nMemory range too large for visualization ("
						<< (max_addr - min_addr) / 1024 << " KB)" << endl;
			}
		}

		// Pattern search
		cout << "\nPattern search:" << endl;
		hex_dumper.find_patterns();
	}

	// Final information
	cout << "\n" << string(40, '=') << endl;
	cout << "ANALYSIS COMPLETE" << endl;

	// Error summary
	if (hex_loader.has_parser_errors()) {
		cout << YELLOW << WARNING_MARK
				<< "\n! WARNING: " << hex_loader.logger().error_count()
				<< " parser error(s) detected" << endl;
		cout << "Data integrity may be compromised." << endl;
	} else if (hex_loader.has_parser_warnings()) {
		cout << YELLOW << "\n NOTE: " << hex_loader.logger().warning_count()
				<< " parser warning(s) detected" << endl;
	} else {
		cout << "\n✓ No parser errors or warnings" << endl;
	}

	cout << string(40, '=') << endl;

	return EXIT_SUCCESS;
}
