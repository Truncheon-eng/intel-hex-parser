// InOutFlagStruct.hpp
#ifndef IN_OUT_FLAG_STRUCT_HPP
#define IN_OUT_FLAG_STRUCT_HPP

#include <iostream>
#include <iomanip>
#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>

using std::cout;
using std::endl;
using std::setw;

// -----------------------------------------------------------------------------
// 4‑BIT data input structure
// -----------------------------------------------------------------------------
struct InOutSignals4 {
	static constexpr int width = 4;
	static constexpr int num_words = 1;
	static_assert(width <= 8, "4‑bit structure must fit into a single byte");

	union {
		struct {
			uint8_t THOHIVLD : 1;
			uint8_t OTL4_10_DATA_VALID : 1;
			uint8_t RATE_CONTROL : 1;
			uint8_t nHARD_RESET : 1;
			uint8_t RESERVED : 4;  // remaining bits
		};
		uint8_t packed;
		uint8_t bytes[1];
	};

	// -------------------------------------------------
	// Constructors
	// -------------------------------------------------
	InOutSignals4() : packed(0) {
		nHARD_RESET = 1;  // reset inactive by default
	}

	InOutSignals4(bool thohivld, bool otl4_10_data_valid, bool rate_control,
					bool nhard_reset)
		: packed(0) {
		set_all(thohivld, otl4_10_data_valid, rate_control, nhard_reset);
	}

	// -------------------------------------------------
	// Getters (read a single bit)
	// -------------------------------------------------
	bool get_THOHIVLD() const { return THOHIVLD; }
	bool get_OTL4_10_DATA_VALID() const { return OTL4_10_DATA_VALID; }
	bool get_RATE_CONTROL() const { return RATE_CONTROL; }
	bool get_nHARD_RESET() const { return nHARD_RESET; }

	// -------------------------------------------------
	// Setters (set a single bit)
	// -------------------------------------------------
	void set_THOHIVLD(bool v) { THOHIVLD = v ? 1 : 0; }
	void set_OTL4_10_DATA_VALID(bool v) { OTL4_10_DATA_VALID = v ? 1 : 0; }
	void set_RATE_CONTROL(bool v) { RATE_CONTROL = v ? 1 : 0; }
	void set_nHARD_RESET(bool v) { nHARD_RESET = v ? 1 : 0; }

	// -------------------------------------------------
	// Set all bits at once
	// -------------------------------------------------
	void set_all(bool thohivld, bool otl4_10_data_valid, bool rate_control,
					bool nhard_reset) {
		THOHIVLD = thohivld ? 1 : 0;
		OTL4_10_DATA_VALID = otl4_10_data_valid ? 1 : 0;
		RATE_CONTROL = rate_control ? 1 : 0;
		nHARD_RESET = nhard_reset ? 1 : 0;
	}

	// -------------------------------------------------
	// Special modes
	// -------------------------------------------------
	void set_reset_sequence() {
		packed = 0;	 // `nHARD_RESET` = 0: reset asserted (active-low)
		OTL4_10_DATA_VALID = 0;
	}

	void set_normal_operation() {
		THOHIVLD = 1;
		OTL4_10_DATA_VALID = 1;
		RATE_CONTROL = 1;
		nHARD_RESET = 1;  // reset inactive (active-low)
	}

	// -------------------------------------------------
	// `operator[]` - Verilator compatibility
	// -------------------------------------------------
	uint32_t operator[](int index) const {
		return (index == 0) ? static_cast<uint32_t>(packed) : 0u;
	}

	// -------------------------------------------------
	// Debug output
	// -------------------------------------------------
	void print(const char *name = "") const {
		std::cout << name << " Input Signals (4 bits):" << std::endl;
		std::cout << "  THOHIVLD:          " << static_cast<int>(THOHIVLD)
					<< std::endl;
		std::cout << "  OTL4_10_DATA_VALID:"
					<< static_cast<int>(OTL4_10_DATA_VALID) << std::endl;
		std::cout << "  RATE_CONTROL:      " << static_cast<int>(RATE_CONTROL)
					<< std::endl;
		std::cout << "  nHARD_RESET:       " << static_cast<int>(nHARD_RESET)
					<< std::endl;
		std::cout << "  packed: 0x" << std::hex << std::setw(2)
					<< std::setfill('0') << static_cast<int>(packed) << std::dec
					<< std::endl;
		std::cout << "  binary: " << (nHARD_RESET ? "1" : "0") << "_"
					<< (RATE_CONTROL ? "1" : "0") << "_"
					<< (OTL4_10_DATA_VALID ? "1" : "0") << "_"
					<< (THOHIVLD ? "1" : "0") << std::endl;
	}

	// -------------------------------------------------
	// Cast to Verilator type
	// -------------------------------------------------
};

// -----------------------------------------------------------------------------
// 5‑BIT data output structure
// -----------------------------------------------------------------------------
struct InOutSignals5 {
	static constexpr int width = 5;
	static constexpr int num_words = 1;
	static_assert(width <= 8, "5‑bit structure must fit into a single byte");

	union {
		struct {
			uint8_t DATA_VALID_OUT : 1;
			uint8_t TXOHIFP_OUT : 1;
			uint8_t TXOHIMFP_OUT : 1;
			uint8_t FEC_SOF_OUT : 1;
			uint8_t THOHEVALID : 1;
			uint8_t RESERVED : 3;  // Alignment
		};
		uint8_t packed;
		uint8_t bytes[1];
	};

	// -------------------------------------------------
	// Constructors
	// -------------------------------------------------
	InOutSignals5() : packed(0) {}

	InOutSignals5(bool data_valid_out, bool txohifp_out, bool txohimfp_out,
					bool fec_sof_out, bool thohevalid)
		: packed(0) {
		set_all(data_valid_out, txohifp_out, txohimfp_out, fec_sof_out,
				thohevalid);
	}

	// -------------------------------------------------
	// Getters
	// -------------------------------------------------
	bool get_DATA_VALID_OUT() const { return DATA_VALID_OUT; }

	bool get_TXOHIFP_OUT() const { return TXOHIFP_OUT; }

	bool get_TXOHIMFP_OUT() const { return TXOHIMFP_OUT; }

	bool get_FEC_SOF_OUT() const { return FEC_SOF_OUT; }

	bool get_THOHEVALID() const { return THOHEVALID; }

	// -------------------------------------------------
	// Setters
	// -------------------------------------------------
	void set_DATA_VALID_OUT(bool v) { DATA_VALID_OUT = v ? 1 : 0; }

	void set_TXOHIFP_OUT(bool v) { TXOHIFP_OUT = v ? 1 : 0; }

	void set_TXOHIMFP_OUT(bool v) { TXOHIMFP_OUT = v ? 1 : 0; }

	void set_FEC_SOF_OUT(bool v) { FEC_SOF_OUT = v ? 1 : 0; }

	void set_THOHEVALID(bool v) { THOHEVALID = v ? 1 : 0; }

	// -------------------------------------------------
	// Set all bits at once
	// -------------------------------------------------
	void set_all(bool data_valid_out, bool txohifp_out, bool txohimfp_out,
					bool fec_sof_out, bool thohevalid) {
		DATA_VALID_OUT = data_valid_out ? 1 : 0;
		TXOHIFP_OUT = txohifp_out ? 1 : 0;
		TXOHIMFP_OUT = txohimfp_out ? 1 : 0;
		FEC_SOF_OUT = fec_sof_out ? 1 : 0;
		THOHEVALID = thohevalid ? 1 : 0;
	}

	// -------------------------------------------------
	// Access operator
	// -------------------------------------------------
	uint32_t operator[](int index) const {
		return (index == 0) ? static_cast<uint32_t>(packed) : 0u;
	}

	// -------------------------------------------------
	// Service method - current status
	// -------------------------------------------------
	std::string get_status() const {
		if (TXOHIFP_OUT) {
			return "TXOHIFP_OUT";
		}
		if (TXOHIMFP_OUT) {
			return "TXOHIMFP_OUT";
		}
		if (FEC_SOF_OUT) {
			return "FEC_SOF_OUT";
		}
		if (DATA_VALID_OUT) {
			return "DATA_VALID_OUT";
		}
		if (THOHEVALID) {
			return "THOHEVALID";
		}
		return "UNKNOWN";
	}

	// -------------------------------------------------
	// Debug output
	// -------------------------------------------------
	void print(const char *name = "") const {
		std::cout << name << " Output Signals (5 bits):" << std::endl;
		std::cout << "  DATA_VALID_OUT: " << static_cast<int>(DATA_VALID_OUT)
					<< std::endl;
		std::cout << "  TXOHIFP_OUT:    " << static_cast<int>(TXOHIFP_OUT)
					<< std::endl;
		std::cout << "  TXOHIMFP_OUT:   " << static_cast<int>(TXOHIMFP_OUT)
					<< std::endl;
		std::cout << "  FEC_SOF_OUT:    " << static_cast<int>(FEC_SOF_OUT)
					<< std::endl;
		std::cout << "  THOHEVALID:     " << static_cast<int>(THOHEVALID)
					<< std::endl;
		std::cout << "  packed: 0x" << std::hex << std::setw(2)
					<< std::setfill('0') << static_cast<int>(packed) << std::dec
					<< std::endl;
		std::cout << "  binary: " << (THOHEVALID ? "1" : "0") << "_"
					<< (FEC_SOF_OUT ? "1" : "0") << "_"
					<< (TXOHIMFP_OUT ? "1" : "0") << "_"
					<< (TXOHIFP_OUT ? "1" : "0") << "_"
					<< (DATA_VALID_OUT ? "1" : "0") << std::endl;
		std::cout << "  status: " << get_status() << std::endl;
	}

	// -------------------------------------------------
	// Cast to Verilator type
	// -------------------------------------------------
};

// TODO: Test this
struct InSignalOtu4Converter {
	static constexpr int width = 2;
	static constexpr int num_words = 1;
	static_assert(width <= 8, "2-bit structure must fit into a single byte");

	union {
		struct {
			uint8_t DATA_VALID_IN : 1;
			uint8_t nHARD_RESET : 1;
			uint8_t RESERVED : 6;  // Alignment
		};
		uint8_t packed;
		uint8_t bytes[1];
	};

	// -------------------------------------------------
	// Constructors
	// -------------------------------------------------
	InSignalOtu4Converter() : packed(0) {}

	InSignalOtu4Converter(bool data_valid_in, bool n_hard_reset) : packed(0) {
		set_all(data_valid_in, n_hard_reset);
	}

	// -------------------------------------------------
	// Getters
	// -------------------------------------------------
	bool get_DATA_VALID_IN() const { return DATA_VALID_IN; }

	bool get_nHARD_RESET() const { return nHARD_RESET; }

	// -------------------------------------------------
	// Setters
	// -------------------------------------------------
	void set_DATA_VALID_IN(bool v) { DATA_VALID_IN = v ? 1 : 0; }

	void set_nHARD_RESET(bool v) { nHARD_RESET = v ? 1 : 0; }

	// -------------------------------------------------
	// Set all bits at once
	// -------------------------------------------------
	void set_all(bool data_valid_in, bool n_hard_reset) {
		DATA_VALID_IN = data_valid_in ? 1 : 0;
		nHARD_RESET = n_hard_reset ? 1 : 0;
	}

	// -------------------------------------------------
	// Access operator
	// -------------------------------------------------
	uint32_t operator[](int index) const {
		return (index == 0) ? static_cast<uint32_t>(packed) : 0u;
	}

	// -------------------------------------------------
	// Service method - current status
	// -------------------------------------------------
	std::string get_status() const {
		if (DATA_VALID_IN) {
			return "DATA_VALID_IN";
		}
		if (nHARD_RESET) {
			return "nHARD_RESET";
		}
		return "UNKNOWN";
	}

	// -------------------------------------------------
	// Debug output
	// -------------------------------------------------
	void print(const char *name = "") const {
		std::cout << name << " Input Signals (2 bits):" << std::endl;
		std::cout << "  DATA_VALID_IN: " << static_cast<int>(DATA_VALID_IN)
					<< std::endl;
		std::cout << "  nHARD_RESET:    " << static_cast<int>(nHARD_RESET)
					<< std::endl;
		std::cout << "  packed: 0x" << std::hex << std::setw(2)
					<< std::setfill('0') << static_cast<int>(packed) << std::dec
					<< std::endl;
		std::cout << "  binary: " << (DATA_VALID_IN ? "1" : "0") << "_"
					<< (nHARD_RESET ? "1" : "0") << std::endl;
		std::cout << "  status: " << get_status() << std::endl;
	}
};

// TODO: Test this
struct OutSignalOtu4Converter {
	static constexpr int width = 2;
	static constexpr int num_words = 1;
	static_assert(width <= 8, "2-bit structure must fit into a single byte");

	union {
		struct {
			uint8_t DATA_VALID_OUT : 1;
			uint8_t FRAME_BEGIN : 1;
			uint8_t RESERVED : 6;  // Alignment
		};
		uint8_t packed;
		uint8_t bytes[1];
	};

	// -------------------------------------------------
	// Constructors
	// -------------------------------------------------
	OutSignalOtu4Converter() : packed(0) {}

	OutSignalOtu4Converter(bool data_valid_out, bool frame_begin) : packed(0) {
		set_all(data_valid_out, frame_begin);
	}

	// -------------------------------------------------
	// Getters
	// -------------------------------------------------
	bool get_DATA_VALID_OUT() const { return DATA_VALID_OUT; }

	bool get_FRAME_BEGIN() const { return FRAME_BEGIN; }

	// -------------------------------------------------
	// Setters
	// -------------------------------------------------
	void set_DATA_VALID_OUT(bool v) { DATA_VALID_OUT = v ? 1 : 0; }

	void set_FRAME_BEGIN(bool v) { FRAME_BEGIN = v ? 1 : 0; }

	// -------------------------------------------------
	// Set all bits at once
	// -------------------------------------------------
	void set_all(bool data_valid_out, bool frame_begin) {
		DATA_VALID_OUT = data_valid_out ? 1 : 0;
		FRAME_BEGIN = frame_begin ? 1 : 0;
	}

	// -------------------------------------------------
	// Access operator
	// -------------------------------------------------
	uint32_t operator[](int index) const {
		return (index == 0) ? static_cast<uint32_t>(packed) : 0u;
	}

	// -------------------------------------------------
	// Service method - current status
	// -------------------------------------------------
	std::string get_status() const {
		if (DATA_VALID_OUT) {
			return "DATA_VALID_OUT";
		}
		if (FRAME_BEGIN) {
			return "FRAME_BEGIN";
		}
		return "UNKNOWN";
	}

	// -------------------------------------------------
	// Debug output
	// -------------------------------------------------
	void print(const char *name = "") const {
		std::cout << name << " Output Signals (2 bits):" << std::endl;
		std::cout << "  DATA_VALID_OUT: " << static_cast<int>(DATA_VALID_OUT)
					<< std::endl;
		std::cout << "  FRAME_BEGIN:    " << static_cast<int>(FRAME_BEGIN)
					<< std::endl;
		std::cout << "  packed: 0x" << std::hex << std::setw(2)
					<< std::setfill('0') << static_cast<int>(packed) << std::dec
					<< std::endl;
		std::cout << "  binary: " << (DATA_VALID_OUT ? "1" : "0") << "_"
					<< (FRAME_BEGIN ? "1" : "0") << std::endl;
		std::cout << "  status: " << get_status() << std::endl;
	}
};

static_assert(sizeof(InOutSignals4) == 1,
				"InOutSignals4 must occupy exactly 1 byte");
static_assert(sizeof(InOutSignals5) == 1,
				"InOutSignals5 must occupy exactly 1 byte");
static_assert(sizeof(InSignalOtu4Converter) == 1,
				"InSignalOtu4Converter must occupy exactly 1 byte");
static_assert(sizeof(OutSignalOtu4Converter) == 1,
				"OutSignalOtu4Converter must occupy exactly 1 byte");

#endif	// IN_OUT_FLAG_STRUCT_HPP
