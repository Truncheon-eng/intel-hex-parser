// HexDumper.h
#ifndef HEX_DUMPER_H
#define HEX_DUMPER_H

#include "hex_loader.h"
#include <vector>
#include <cstdint>
#include <string>
#include <algorithm>

using std::cout;
using std::endl;
using std::string;

using std::vector;

class HexDumper {
private:
	HexLoader &hex_loader;

public:
	HexDumper(HexLoader &loader) : hex_loader(loader) {}

	// Main dump methods
	void dump_entire_file();
	void print_statistics();
	void find_patterns();
	void dump_range(uint32_t start_addr, uint32_t end_addr);
	void dump_memory_map();

	// TODO: Implement in the future
	//
	void visualize_gaps(const vector<uint32_t> &addresses, uint32_t start_addr,
						uint32_t end_addr, int width = 80);

	void analyze_with_visualization();

	void visualize_gaps_range(uint32_t start_addr, uint32_t end_addr,
								int width = 80);
	string generate_visualization(const vector<uint32_t> &addresses,
									uint32_t start_addr, uint32_t end_addr,
									int width);
	vector<uint32_t> get_all_addresses();

private:
	// Helper methods
	void dump_line(uint32_t start_addr, const vector<uint8_t> &data);
	void find_data_gaps(const vector<uint32_t> &addresses);
	bool check_patterns_at_address(uint32_t addr);
	void analyze_memory_regions(const vector<uint32_t> &addresses);

	// Helper methods for visualization

	// Structures for gap analysis
	enum GapCategory {
		TINY,	 // 1-16 bytes
		SMALL,	 // 17-256 bytes
		MEDIUM,	 // 257-4096 bytes
		LARGE,	 // 4097-65536 bytes
		HUGE	 // > 65536 bytes
	};

	struct GapInfo {
		uint32_t start;
		uint32_t end;
		uint32_t size;
		GapCategory category;
		vector<string> possible_purposes;
	};

	GapCategory classify_gap_size(uint32_t size);
};

#endif	// HEX_DUMPER_H
