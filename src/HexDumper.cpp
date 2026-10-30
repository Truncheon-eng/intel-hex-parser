// HexDumper.cpp
#include "HexDumper.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <map>
#include <sstream>

using namespace std;

// Dump the entire file in hex format
void HexDumper::dump_entire_file() {
	cout << "=== Complete HEX File Dump ===" << endl;

	if (hex_loader.empty()) {
		cout << "No data in HEX file!" << endl;
		return;
	}

	vector<uint32_t> addresses = get_all_addresses();
	if (addresses.empty()) {
		cout << "No data in HEX file!" << endl;
		return;
	}
	uint32_t current_line_start = addresses[0];
	vector<uint8_t> current_line;

	for (uint32_t addr : addresses) {
		// If there is an address gap or the line is full
		if (!current_line.empty() &&
			(addr != current_line_start + current_line.size() ||
				current_line.size() >= 16)) {
			dump_line(current_line_start, current_line);
			current_line.clear();
			current_line_start = addr;
		}

		uint8_t data = hex_loader.read_byte(addr);
		current_line.push_back(data);
	}

	// Print the last line
	if (!current_line.empty()) {
		dump_line(current_line_start, current_line);
	}
}

// Dump a specific address range
void HexDumper::dump_range(uint32_t start_addr, uint32_t end_addr) {
	cout << "=== Memory Dump 0x" << std::hex << start_addr << " - 0x"
			<< end_addr << std::dec << " ===" << endl;

	if (start_addr > end_addr) {
		cout << "Invalid address range!" << endl;
		return;
	}

	vector<uint8_t> current_line;
	uint32_t current_line_start = start_addr;

	for (uint32_t addr = start_addr; addr <= end_addr; addr++) {
		if (!hex_loader.has_data(addr)) {
			// Skip missing data
			continue;
		}

		// If there is a gap or the line is full
		if (!current_line.empty() &&
			(addr != current_line_start + current_line.size() ||
				current_line.size() >= 16)) {
			dump_line(current_line_start, current_line);
			current_line.clear();
			current_line_start = addr;
		}

		if (current_line.empty()) {
			current_line_start = addr;
		}

		uint8_t data = hex_loader.read_byte(addr);
		current_line.push_back(data);
	}

	// Print the last line
	if (!current_line.empty()) {
		dump_line(current_line_start, current_line);
	}
}

// File statistics
void HexDumper::print_statistics() {
	cout << "=== HEX File Statistics ===" << endl;
	hex_loader.print_info();

	// Count used addresses
	vector<uint32_t> addresses = get_all_addresses();
	if (addresses.empty()) {
		cout << "No data addresses found!" << endl;
		return;
	}

	uint32_t min_addr = addresses.front();
	uint32_t max_addr = addresses.back();
	size_t total_bytes = addresses.size();

	uint32_t address_range_size = max_addr - min_addr + 1;

	cout << "Min address: 0x" << std::hex << min_addr << std::dec << endl;
	cout << "Max address: 0x" << std::hex << max_addr << std::dec << endl;
	cout << "Total data bytes: " << total_bytes << endl;
	cout << "Address range: 0x" << std::hex << min_addr << " - 0x" << max_addr
			<< " (" << std::dec << address_range_size << " bytes)" << endl;
	cout << "Data density: " << std::fixed << std::setprecision(1)
			<< (static_cast<double>(total_bytes) / address_range_size * 100) << "%"
			<< endl;

	// Search for data gaps
	find_data_gaps(addresses);

	// Memory regions analysis
	analyze_memory_regions(addresses);
}

// Memory regions analysis
void HexDumper::analyze_memory_regions(const vector<uint32_t> &addresses) {
	if (addresses.empty()) {
		return;
	}

	cout << "\n=== Memory Regions Analysis ===" << endl;

	vector<std::pair<uint32_t, uint32_t>> regions;
	uint32_t region_start = addresses[0];
	uint32_t region_end = addresses[0];

	for (size_t i = 1; i < addresses.size(); i++) {
		if (addresses[i] == region_end + 1) {
			// Region continues
			region_end = addresses[i];
		} else {
			// End of region
			regions.emplace_back(region_start, region_end);
			region_start = addresses[i];
			region_end = addresses[i];
		}
	}

	// Add the last region
	regions.emplace_back(region_start, region_end);

	cout << "Found " << regions.size() << " memory regions:" << endl;
	for (size_t i = 0; i < regions.size(); i++) {
		uint32_t start = regions[i].first;
		uint32_t end = regions[i].second;
		uint32_t size = end - start + 1;

		cout << "  Region " << i << ": 0x" << std::hex << start << " - 0x"
				<< end << " (" << std::dec << size << " bytes)" << endl;
	}
}

// Memory map
void HexDumper::dump_memory_map() {
	cout << "=== Memory Map ===" << endl;

	vector<uint32_t> addresses = get_all_addresses();
	if (addresses.empty()) {
		cout << "No memory regions found!" << endl;
		return;
	}

	analyze_memory_regions(addresses);
}

// Pattern search
void HexDumper::find_patterns() {
	cout << "=== Common Pattern Search ===" << endl;

	vector<uint32_t> addresses = get_all_addresses();
	bool found_patterns = false;

	if (addresses.empty()) {
		cout << "No data in HEX file!" << endl;
		return;
	}

	for (uint32_t addr : addresses) {
		if (addr % 4 == 0 && (addr + 3) <= addresses.back()) {
			if (check_patterns_at_address(addr)) {
				found_patterns = true;
			}
		}
	}

	if (!found_patterns) {
		cout << "No common patterns found." << endl;
	}
}

// Private implementation methods

void HexDumper::dump_line(uint32_t start_addr, const vector<uint8_t> &data) {
	if (data.empty()) {
		return;
	}

	// Address
	cout << std::hex << std::setw(8) << std::setfill('0') << start_addr << ": ";

	// Hex data
	for (size_t i = 0; i < 16; i++) {
		if (i < data.size()) {
			cout << std::setw(2) << std::setfill('0')
					<< static_cast<int>(data[i]) << " ";
		} else {
			cout << "   ";
		}

		if (i == 7) {
			cout << " ";  // Middle separator
		}
	}

	// ASCII representation
	cout << " |";
	for (size_t i = 0; i < data.size() && i < 16; i++) {
		uint8_t byte = data[i];
		if (byte >= 32 && byte <= 126) {
			cout << static_cast<char>(byte);
		} else {
			cout << ".";
		}
	}
	cout << "|" << std::dec << endl;
}

vector<uint32_t> HexDumper::get_all_addresses() {
	vector<uint32_t> addresses;

	if (hex_loader.empty()) {
		cout << "HexLoader is empty, no addresses to retrieve." << endl;
		return addresses;
	}

	addresses.reserve(hex_loader.size() + 10);	// Small margin

	if (addresses.size() >= addresses.capacity()) {
		cout << "Warning: Reached reserved capacity (" << addresses.capacity()
				<< "), actual addresses: " << addresses.size() << endl;
	}

	try {
		for (hex_loader.begin();; hex_loader.incrementAddress()) {
			addresses.push_back(hex_loader.currentAddress());
			if (hex_loader.endOfData()) {
				break;
			}
		}

		// try {
		// 		// More standard approach for a for loop with break in the
		// middle 		hex_loader.begin(); 		for (;;) {
		// 			addresses.push_back(hex_loader.currentAddress());
		// 			if (hex_loader.endOfData()) {
		// 			cout << "Founded END address at:\t0x" << std::hex <<
		// std::setw(8)
		// 				<< std::setfill('0') << hex_loader.currentAddress()
		// 					<< std::dec << endl;
		// 				break;
		// 			}
		// 			if (!hex_loader.incrementAddress()) {
		// 				// Failed to advance to the next address
		// 				break;
		// 			}
		// 		}
		// 	}


		cout << "Successfully retrieved " << addresses.size()
				<< " addresses from HexLoader." << endl;

	} catch (const std::exception &e) {
		cerr << "Error in get_all_addresses(): " << e.what() << endl;
		addresses.clear();
	} catch (...) {
		cerr << "Unknown error in get_all_addresses()" << endl;
		addresses.clear();
	}

	addresses.shrink_to_fit();
	return addresses;
}

void HexDumper::find_data_gaps(const vector<uint32_t> &addresses) {
	if (addresses.size() < 2) {
		cout << "Not enough addresses for gap analysis (need at least 2)."
				<< endl;
		return;
	}

	cout << "\n=== DATA GAPS ANALYSIS ===" << endl;
	cout << "Total addresses: " << addresses.size() << endl;

	// Sort addresses (just in case)
	vector<uint32_t> sorted_addresses = addresses;
	std::sort(sorted_addresses.begin(), sorted_addresses.end());

	// Verify sorting
	bool is_sorted = std::is_sorted(addresses.begin(), addresses.end());
	if (!is_sorted) {
		cout << "Note: Addresses were not sorted, performed sorting." << endl;
	}

	uint32_t min_addr = sorted_addresses.front();
	uint32_t max_addr = sorted_addresses.back();
	uint32_t total_range = max_addr - min_addr + 1;

	cout << "Address range: 0x" << std::hex << min_addr << " - 0x" << max_addr
			<< " (" << std::dec << total_range << " bytes)" << endl;

	// Gap analysis
	vector<GapInfo> gaps;
	size_t total_gap_bytes = 0;

	uint32_t prev_addr{0}, curr_addr{0};
	uint32_t gap_start{0}, gap_end{0}, gap_size{0};

	for (size_t i = 1; i < sorted_addresses.size(); i++) {
		prev_addr = sorted_addresses[i - 1];
		curr_addr = sorted_addresses[i];

		if (curr_addr > prev_addr + 1) {
			// Gap found
			gap_start = prev_addr + 1;
			gap_end = curr_addr - 1;
			gap_size = gap_end - gap_start + 1;

			GapInfo gap;
			gap.start = gap_start;
			gap.end = gap_end;
			gap.size = gap_size;
			gap.category = classify_gap_size(gap_size);

			// TODO: Add analysis of possible purposes
			//  Analyze possible purposes
			// analyze_gap_purposes(gap);

			gaps.push_back(gap);
			total_gap_bytes += gap_size;
		}
	}

	// Output the results
	if (gaps.empty()) {
		cout << "\n PERFECTLY CONTIGUOUS DATA!" << endl;
		cout << "No gaps found - all " << sorted_addresses.size()
				<< " addresses are consecutive." << endl;

		// Visualization of the contiguous range
		cout << "\nVisualization of continuous range:" << endl;
		visualize_gaps(sorted_addresses, min_addr, max_addr, 60);
	} else {
		cout << "\nFound " << gaps.size() << " gaps:" << endl;
		cout << "Total gap bytes: " << total_gap_bytes << " (" << std::fixed
				<< std::setprecision(1)
				<< (static_cast<double>(total_gap_bytes) / total_range * 100.0)
				<< "% of address range)" << endl;
		// TODO: Add a size distribution analyzer
		//  Size distribution analysis
		// analyze_gap_distribution(gaps);

		// Detailed output of the first N gaps
		const size_t MAX_GAPS_TO_SHOW = 10;
		cout << "\nTop " << std::min(MAX_GAPS_TO_SHOW, gaps.size())
				<< " gaps (largest first):" << endl;

		// Sort by size (largest first)
		vector<GapInfo> sorted_gaps = gaps;
		std::sort(sorted_gaps.begin(), sorted_gaps.end(),
					[](const GapInfo &a, const GapInfo &b) {
						return a.size > b.size;
					});

		for (size_t i = 0; i < std::min(MAX_GAPS_TO_SHOW, sorted_gaps.size());
				i++) {
			const GapInfo &gap = sorted_gaps[i];
			cout << "  Gap #" << (i + 1) << ": 0x" << std::hex << gap.start
					<< " - 0x" << gap.end << " (" << std::dec << gap.size
					<< " bytes)" << endl;

			// Print possible purposes
			if (!gap.possible_purposes.empty()) {
				cout << "    Possible purposes: ";
				for (size_t j = 0; j < gap.possible_purposes.size(); j++) {
					cout << gap.possible_purposes[j];
					if (j < gap.possible_purposes.size() - 1) {
						cout << ", ";
					}
				}
				cout << endl;
			}
		}

		if (gaps.size() > MAX_GAPS_TO_SHOW) {
			cout << "  ... and " << (gaps.size() - MAX_GAPS_TO_SHOW)
					<< " more gaps" << endl;
		}

		// Visualization
		cout << "\nMemory visualization:" << endl;
		visualize_gaps(sorted_addresses, min_addr, max_addr, 60);

		uint32_t viz_start, viz_end;

		// Visualization of the area with the largest gap
		if (!gaps.empty()) {
			auto largest_gap =
				*std::max_element(gaps.begin(), gaps.end(),
									[](const GapInfo &a, const GapInfo &b) {
										return a.size < b.size;
									});

			if (largest_gap.size > 100) {  // Visualize only large gaps
				cout << "\nZoom on largest gap (0x" << std::hex
						<< largest_gap.start << " - 0x" << largest_gap.end
						<< "):" << endl;

				// Show the area around the gap
				viz_start =
					(largest_gap.start > 200) ? largest_gap.start - 200 : 0;
				viz_end = largest_gap.end + 200;

				visualize_gaps(sorted_addresses, viz_start, viz_end, 60);
			}
		}
	}
	// TODO: Add extra statistics
	//  Additional statistics
	// print_gap_statistics_summary(gaps, total_range, sorted_addresses.size());
}

bool HexDumper::check_patterns_at_address(uint32_t addr) {
	if (!hex_loader.has_data(addr) || !hex_loader.has_data(addr + 3)) {
		return false;
	}

	uint32_t word_be = hex_loader.read_word_be(addr);
	uint32_t word_le = hex_loader.read_word_le(addr);

	bool found = false;

	// List of known patterns
	vector<std::pair<uint32_t, std::string>> patterns = {
		{0xDEADBEEF, "DEADBEEF"}, {0xCAFEBABE, "CAFEBABE"},
		{0x12345678, "12345678"}, {0xFFFFFFFF, "FFFFFFFF"},
		{0x00000000, "00000000"}, {0xAAAAAAAA, "AAAAAAAA"},
		{0x55555555, "55555555"}, {0xCCCCCCCC, "CCCCCCCC"},
		{0x88888888, "88888888"}, {0x11111111, "11111111"}};

	for (const auto &[pattern, name] : patterns) {
		if (word_be == pattern) {
			cout << "Pattern '" << name << "' [BIG-ENDIAN] at 0x" << std::hex
					<< addr << std::dec << endl;
			found = true;
		} else if (word_le == pattern) {
			cout << "Pattern '" << name << "' [LITTLE-ENDIAN] at 0x" << std::hex
					<< addr << std::dec << endl;
			found = true;
		}
	}

	return found;
}

void HexDumper::visualize_gaps(const vector<uint32_t> &addresses,
								uint32_t start_addr, uint32_t end_addr,
								int width) {
	if (addresses.empty()) {
		return;
	}

	uint32_t total_range = end_addr - start_addr + 1;
	double scale = static_cast<double>(width) / total_range;

	cout << "Memory map visualization (0x" << hex << start_addr << " - 0x"
			<< end_addr << "):" << endl;

	vector<bool> has_data(total_range, false);

	// Mark addresses with data
	for (uint32_t addr : addresses) {
		if (addr >= start_addr && addr <= end_addr) {
			has_data[addr - start_addr] = true;
		}
	}

	uint32_t block_start, block_end;

	// Visualization
	for (int i = 0; i < width; i++) {
		block_start = start_addr + static_cast<uint32_t>(i / scale);
		block_end = start_addr + static_cast<uint32_t>((i + 1) / scale);

		bool block_has_data = false;
		for (uint32_t addr = block_start; addr < block_end; addr++) {
			if (addr - start_addr < has_data.size() &&
				has_data[addr - start_addr]) {
				block_has_data = true;
				break;
			}
		}

		cout << (block_has_data ? "█" : "░");

		// Separator every 8 characters
		if ((i + 1) % 8 == 0) {
			cout << ' ';
		}
	}
	cout << endl;

	// Legend
	cout << "\nLegend:" << endl;
	cout << "█ - Data present   ░ - Gap (no data)" << endl;
}

// Gap size classification
HexDumper::GapCategory HexDumper::classify_gap_size(uint32_t size) {
	if (size <= 16) {
		return TINY;
	} else if (size <= 256) {
		return SMALL;
	} else if (size <= 4096) {
		return MEDIUM;
	} else if (size <= 65536) {
		return LARGE;
	} else {
		return HUGE;
	}
}

void HexDumper::analyze_with_visualization() {
	cout << "\n=== COMPLETE ANALYSIS WITH VISUALIZATION ===" << endl;

	// Get all addresses
	vector<uint32_t> addresses = get_all_addresses();

	if (addresses.empty()) {
		cout << "No data to analyze!" << endl;
		return;
	}

	// Sort (just in case)
	std::sort(addresses.begin(), addresses.end());

	// Basic metrics
	uint32_t min_addr = addresses.front();
	uint32_t max_addr = addresses.back();
	uint32_t total_range = max_addr - min_addr + 1;
	size_t data_bytes = addresses.size();

	cout << "\nBasic Statistics:" << endl;
	cout << "  Min address:     0x" << std::hex << min_addr << std::dec << endl;
	cout << "  Max address:     0x" << std::hex << max_addr << std::dec << endl;
	cout << "  Address range:   " << total_range << " bytes" << endl;
	cout << "  Data bytes:      " << data_bytes << endl;
	cout << "  Data density:    " << std::fixed << std::setprecision(2)
			<< (static_cast<double>(data_bytes) / total_range * 100.0) << "%"
			<< endl;

	// Gap analysis
	cout << "\nGap Analysis:" << endl;
	find_data_gaps(addresses);

	// Visualization
	cout << "\nMemory Visualization:" << endl;
	visualize_gaps(addresses, min_addr, max_addr, 80);

	// Detailed analysis if gaps exist
	// if (addresses.size() > 0 && addresses.back() - addresses.front() + 1 >
	// addresses.size()) { 	cout << "\nDetailed Analysis:" << endl;
	//	analyze_gaps_detailed();
	// }

	cout << "\n=== ANALYSIS COMPLETE ===" << endl;
}


// Generate visualization
string HexDumper::generate_visualization(const vector<uint32_t> &addresses,
											uint32_t start_addr, uint32_t end_addr,
											int width) {
	stringstream ss;

	uint32_t total_range = end_addr - start_addr + 1;

	ss << "Memory map visualization (0x" << std::hex << std::setw(8)
		<< std::setfill('0') << start_addr << " - 0x" << end_addr
		<< "):" << std::dec << endl;

	// Build a bitmap for fast lookup
	map<uint32_t, bool> data_map;
	for (uint32_t addr : addresses) {
		data_map[addr] = true;
	}

	// Determine the scale
	double bytes_per_char = static_cast<double>(total_range) / width;

	// Visualization
	for (int i = 0; i < width; i++) {
		uint32_t block_start =
			start_addr + static_cast<uint32_t>(i * bytes_per_char);
		uint32_t block_end =
			start_addr + static_cast<uint32_t>((i + 1) * bytes_per_char) - 1;

		bool block_has_data = false;

		// Check whether this block contains data
		for (uint32_t addr = block_start; addr <= block_end && addr <= end_addr;
				addr++) {
			if (data_map.find(addr) != data_map.end()) {
				block_has_data = true;
				break;
			}
		}

		// Use ASCII characters for compatibility
		if (block_has_data) {
			ss << "#";	// Data present
		} else {
			ss << ".";	// Gap
		}

		// Separator every 8 characters for readability
		if ((i + 1) % 8 == 0) {
			ss << " ";
		}
	}
	ss << endl;

	// Legend
	ss << "\nLegend:" << endl;
	ss << "# - Data present   . - Gap (no data)" << endl;

	// Scale information
	ss << "Scale: ~" << static_cast<int>(bytes_per_char)
		<< " bytes per character" << endl;

	return ss.str();
}


// Gap visualization for a specific range
void HexDumper::visualize_gaps_range(uint32_t start_addr, uint32_t end_addr,
										int width) {
	vector<uint32_t> addresses = get_all_addresses();

	if (addresses.empty()) {
		cout << "No data to visualize!" << endl;
		return;
	}

	// Filter addresses in the given range
	vector<uint32_t> filtered_addresses;
	for (uint32_t addr : addresses) {
		if (addr >= start_addr && addr <= end_addr) {
			filtered_addresses.push_back(addr);
		}
	}

	if (filtered_addresses.empty()) {
		cout << "No data in the specified range!" << endl;
		return;
	}

	string visualization =
		generate_visualization(filtered_addresses, start_addr, end_addr, width);
	cout << visualization;
}
