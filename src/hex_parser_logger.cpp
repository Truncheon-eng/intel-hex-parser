// hex_parser_logger.cpp
#include "hex_parser_logger.h"
#include <fstream>
#include <sstream>
#include <chrono>
#include <ctime>
#include <algorithm>

// Constructor
HexParserLogger::HexParserLogger(intelhex *parser) : parser_(parser) {}

// Parser attachment
void HexParserLogger::attach_parser(intelhex *parser) { parser_ = parser; }

void HexParserLogger::detach_parser() { parser_ = nullptr; }

// Collect messages from the parser - FIXED VERSION
void HexParserLogger::collect_messages() {
	if (!parser_) {
		return;
	}

	// Collect errors
	while (parser_->getNoErrors() > 0) {
		std::string error;
		if (parser_->popNextError(error)) {	 // Takes the string by reference
			if (!error.empty()) {
				messages_.emplace_back(MSG_ERROR, error);
			}
		} else {
			break;	// Failed to retrieve the error
		}
	}

	// Collect warnings
	while (parser_->getNoWarnings() > 0) {
		std::string warning;
		if (parser_->popNextWarning(
				warning)) {	 // Takes the string by reference
			if (!warning.empty()) {
				messages_.emplace_back(MSG_WARNING, warning);
			}
		} else {
			break;	// Failed to retrieve the warning
		}
	}
}

void HexParserLogger::collect_and_clear() {
	collect_messages();
	// Messages are already removed from the parser via popNext...
}

// Adding custom messages
void HexParserLogger::add_error(const std::string &error, uint32_t line,
								uint32_t addr) {
	messages_.emplace_back(MSG_ERROR, error, line, addr);
}

void HexParserLogger::add_warning(const std::string &warning, uint32_t line,
									uint32_t addr) {
	messages_.emplace_back(MSG_WARNING, warning, line, addr);
}

void HexParserLogger::add_info(const std::string &info, uint32_t line,
								uint32_t addr) {
	messages_.emplace_back(MSG_INFO, info, line, addr);
}

// Checks
bool HexParserLogger::has_errors() const {
	return std::any_of(messages_.begin(), messages_.end(),
						[](const ParserMessage &msg) {
							return msg.is_error();
						});
}

bool HexParserLogger::has_warnings() const {
	return std::any_of(messages_.begin(), messages_.end(),
						[](const ParserMessage &msg) {
							return msg.is_warning();
						});
}

bool HexParserLogger::has_messages() const { return !messages_.empty(); }

// Statistics retrieval
size_t HexParserLogger::error_count() const {
	return std::count_if(messages_.begin(), messages_.end(),
							[](const ParserMessage &msg) {
								return msg.is_error();
							});
}

size_t HexParserLogger::warning_count() const {
	return std::count_if(messages_.begin(), messages_.end(),
							[](const ParserMessage &msg) {
								return msg.is_warning();
							});
}

size_t HexParserLogger::info_count() const {
	return std::count_if(messages_.begin(), messages_.end(),
							[](const ParserMessage &msg) {
								return msg.type == MSG_INFO;
							});
}

size_t HexParserLogger::total_count() const { return messages_.size(); }

// Message retrieval
const std::vector<HexParserLogger::ParserMessage> &
HexParserLogger::get_all_messages() const {
	return messages_;
}

std::vector<HexParserLogger::ParserMessage> HexParserLogger::get_errors()
	const {
	return filter_by_type(MSG_ERROR);
}

std::vector<HexParserLogger::ParserMessage> HexParserLogger::get_warnings()
	const {
	return filter_by_type(MSG_WARNING);
}

std::vector<HexParserLogger::ParserMessage> HexParserLogger::get_infos() const {
	return filter_by_type(MSG_INFO);
}

// Filtering
std::vector<HexParserLogger::ParserMessage> HexParserLogger::filter_by_type(
	MessageType type) const {
	std::vector<ParserMessage> result;
	std::copy_if(messages_.begin(), messages_.end(), std::back_inserter(result),
					[type](const ParserMessage &msg) {
						return msg.type == type;
					});
	return result;
}

std::vector<HexParserLogger::ParserMessage> HexParserLogger::filter_by_address(
	uint32_t address) const {
	std::vector<ParserMessage> result;
	std::copy_if(messages_.begin(), messages_.end(), std::back_inserter(result),
					[address](const ParserMessage &msg) {
						return msg.address == address;
					});
	return result;
}

std::vector<HexParserLogger::ParserMessage> HexParserLogger::filter_by_line(
	uint32_t line) const {
	std::vector<ParserMessage> result;
	std::copy_if(messages_.begin(), messages_.end(), std::back_inserter(result),
					[line](const ParserMessage &msg) {
						return msg.line_number == line;
					});
	return result;
}

// Clearing
void HexParserLogger::clear_all() { messages_.clear(); }

void HexParserLogger::clear_errors() {
	messages_.erase(std::remove_if(messages_.begin(), messages_.end(),
									[](const ParserMessage &msg) {
										return msg.is_error();
									}),
					messages_.end());
}

void HexParserLogger::clear_warnings() {
	messages_.erase(std::remove_if(messages_.begin(), messages_.end(),
									[](const ParserMessage &msg) {
										return msg.is_warning();
									}),
					messages_.end());
}

void HexParserLogger::clear_infos() {
	messages_.erase(std::remove_if(messages_.begin(), messages_.end(),
									[](const ParserMessage &msg) {
										return msg.type == MSG_INFO;
									}),
					messages_.end());
}

// Output
void HexParserLogger::print_all(bool colored, bool clear_after) {
	std::cout << export_to_string(colored);
	if (clear_after) {
		clear_all();
	}
}

void HexParserLogger::print_errors(bool colored, bool clear_after) {
	auto errors = get_errors();
	if (errors.empty()) {
		std::cout << "No errors.\n";
		return;
	}

	std::cout << "\n=== Errors (" << errors.size() << ") ===\n";
	for (const auto &msg : errors) {
		std::cout << msg.to_string(colored) << "\n";
	}

	if (clear_after) {
		clear_errors();
	}
}

void HexParserLogger::print_warnings(bool colored, bool clear_after) {
	auto warnings = get_warnings();
	if (warnings.empty()) {
		std::cout << "No warnings.\n";
		return;
	}

	std::cout << "\n=== Warnings (" << warnings.size() << ") ===\n";
	for (const auto &msg : warnings) {
		std::cout << msg.to_string(colored) << "\n";
	}

	if (clear_after) {
		clear_warnings();
	}
}

void HexParserLogger::print_summary() const {
	std::cout << "\n=== Parser Summary ===\n";
	std::cout << "Status: " << get_status_string() << "\n";
	std::cout << "Total messages: " << total_count() << "\n";
	std::cout << "Errors: " << error_count() << "\n";
	std::cout << "Warnings: " << warning_count() << "\n";
	std::cout << "Infos: " << info_count() << "\n";
}

// Export - fixed version
bool HexParserLogger::export_to_file(const std::string &filename,
										bool /* include_timestamps */) const {
	std::ofstream file(filename);
	if (!file.is_open()) {
		return false;
	}

	file << export_to_string(false);
	return true;
}

std::string HexParserLogger::export_to_string(bool colored) const {
	std::stringstream ss;

	if (messages_.empty()) {
		ss << "No parser messages.\n";
		return ss.str();
	}

	// Group by type
	auto errors = get_errors();
	auto warnings = get_warnings();
	auto infos = get_infos();

	if (!errors.empty()) {
		ss << "\n=== Errors (" << errors.size() << ") ===\n";
		for (const auto &msg : errors) {
			ss << msg.to_string(colored) << "\n";
		}
	}

	if (!warnings.empty()) {
		if (!errors.empty()) {
			ss << "\n";
		}
		ss << "\n=== Warnings (" << warnings.size() << ") ===\n";
		for (const auto &msg : warnings) {
			ss << msg.to_string(colored) << "\n";
		}
	}

	if (!infos.empty()) {
		if (!errors.empty() || !warnings.empty()) {
			ss << "\n";
		}
		ss << "\n=== Infos (" << infos.size() << ") ===\n";
		for (const auto &msg : infos) {
			ss << msg.to_string(colored) << "\n";
		}
	}

	return ss.str();
}

// Status
HexParserLogger::ParserStatus HexParserLogger::get_status() const {
	if (!parser_) {
		return STATUS_NO_PARSER;
	}

	if (has_errors()) {
		return STATUS_ERRORS;
	}
	if (has_warnings()) {
		return STATUS_WARNINGS;
	}

	if (parser_->empty()) {
		return STATUS_NO_DATA;
	}

	return STATUS_OK;
}

std::string HexParserLogger::get_status_string() const {
	switch (get_status()) {
		case STATUS_OK:
			return "OK";
		case STATUS_WARNINGS:
			return "WARNINGS";
		case STATUS_ERRORS:
			return "ERRORS";
		case STATUS_NO_PARSER:
			return "NO_PARSER";
		case STATUS_NO_DATA:
			return "NO_DATA";
		default:
			return "UNKNOWN";
	}
}

// Utilities
std::string HexParserLogger::message_type_to_string(MessageType type,
													bool colored) {
	switch (type) {
		case MSG_ERROR:
			return colored ? "\033[1;31mERROR\033[0m" : "ERROR";
		case MSG_WARNING:
			return colored ? "\033[1;33mWARNING\033[0m" : "WARNING";
		case MSG_INFO:
			return colored ? "\033[1;36mINFO\033[0m" : "INFO";
		default:
			return "UNKNOWN";
	}
}

std::string HexParserLogger::address_to_hex(uint32_t address) {
	std::stringstream ss;
	ss << "0x" << std::hex << std::setw(8) << std::setfill('0') << address;
	return ss.str();
}

// Helper methods
void HexParserLogger::add_parser_message(MessageType type,
											const std::string &text) {
	messages_.emplace_back(type, text);
}

std::string HexParserLogger::generate_timestamp() const {
	auto now = std::chrono::system_clock::now();
	auto time = std::chrono::system_clock::to_time_t(now);
	char buffer[20];
	std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S",
					std::localtime(&time));
	return std::string(buffer);
}

// to_string implementation for ParserMessage
std::string HexParserLogger::ParserMessage::to_string(bool colored) const {
	std::stringstream ss;

	// Message type
	ss << HexParserLogger::message_type_to_string(type, colored);

	// Line number (if present)
	if (line_number > 0) {
		ss << " [Line " << line_number << "]";
	}

	// Address (if present)
	if (address > 0) {
		ss << " [Addr " << HexParserLogger::address_to_hex(address) << "]";
	}

	// Message text
	ss << ": " << text;

	return ss.str();
}
