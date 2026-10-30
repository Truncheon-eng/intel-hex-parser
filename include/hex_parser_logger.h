// hex_parser_logger.h
#ifndef HEX_PARSER_LOGGER_H
#define HEX_PARSER_LOGGER_H

#include "intelhexclass.h"
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <memory>
#include <cstdint>

class HexParserLogger {
public:
	// Message types
	enum MessageType { MSG_ERROR, MSG_WARNING, MSG_INFO };

	// Message storage structure
	struct ParserMessage {
		MessageType type;
		std::string text;
		uint32_t line_number;	// If known
		uint32_t address;		// If known
		std::string timestamp;	// If needed

		ParserMessage(MessageType t, const std::string &txt, uint32_t line = 0,
						uint32_t addr = 0)
			: type(t), text(txt), line_number(line), address(addr) {
			// Timestamp generation can be added here
		}

		std::string to_string(bool colored = false) const;
		bool is_error() const { return type == MSG_ERROR; }
		bool is_warning() const { return type == MSG_WARNING; }
	};

public:
	HexParserLogger(intelhex *parser = nullptr);
	~HexParserLogger() = default;

	// Parser attachment
	void attach_parser(intelhex *parser);
	void detach_parser();

	// Collect messages from the parser
	void collect_messages();
	void collect_and_clear();  // Collect and clear in the parser

	// Adding custom messages
	void add_error(const std::string &error, uint32_t line = 0,
					uint32_t addr = 0);

	void add_warning(const std::string &warning, uint32_t line = 0,
						uint32_t addr = 0);

	void add_info(const std::string &info, uint32_t line = 0,
					uint32_t addr = 0);

	// Checks
	bool has_errors() const;
	bool has_warnings() const;
	bool has_messages() const;

	// Statistics retrieval
	size_t error_count() const;
	size_t warning_count() const;
	size_t info_count() const;
	size_t total_count() const;

	// Message retrieval
	const std::vector<ParserMessage> &get_all_messages() const;
	std::vector<ParserMessage> get_errors() const;
	std::vector<ParserMessage> get_warnings() const;
	std::vector<ParserMessage> get_infos() const;

	// Filtering
	std::vector<ParserMessage> filter_by_type(MessageType type) const;
	std::vector<ParserMessage> filter_by_address(uint32_t address) const;
	std::vector<ParserMessage> filter_by_line(uint32_t line) const;

	// Clearing
	void clear_all();
	void clear_errors();
	void clear_warnings();
	void clear_infos();

	// Output
	void print_all(bool colored = true, bool clear_after = false);
	void print_errors(bool colored = true, bool clear_after = false);
	void print_warnings(bool colored = true, bool clear_after = false);
	void print_summary() const;

	// Export
	bool export_to_file(const std::string &filename,
						bool include_timestamps = true) const;
	std::string export_to_string(bool colored = false) const;

	// Status
	enum ParserStatus {
		STATUS_OK,
		STATUS_WARNINGS,
		STATUS_ERRORS,
		STATUS_NO_PARSER,
		STATUS_NO_DATA
	};

	ParserStatus get_status() const;
	std::string get_status_string() const;

	// Utilities
	static std::string message_type_to_string(MessageType type,
												bool colored = false);
	static std::string address_to_hex(uint32_t address);

private:
	intelhex *parser_;
	std::vector<ParserMessage> messages_;

	// Helper methods
	void add_parser_message(MessageType type, const std::string &text);
	std::string generate_timestamp() const;
};

#endif	// HEX_PARSER_LOGGER_H
