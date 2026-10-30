// VectorTransformer.hpp
#ifndef VECTOR_TRANSFORMER_HPP
#define VECTOR_TRANSFORMER_HPP

#include <vector>
#include <memory>
#include <iostream>
#include <iomanip>
#include <bitset>
#include <stdexcept>

#include "TestVectorLoader.hpp"
#include "InOutFramerTXDataStruct.hpp"

class VectorTransformer {
private:
	TestVectorLoader &loader_;
	TestHeaderDetailed header_;

	// Transformation configuration flags
	bool use_big_endian_ = true;		 // default byte order
	bool extract_output_112bit_ = true;	 // extract 112-bit output data

public:
	explicit VectorTransformer(TestVectorLoader &loader)
		: loader_(loader), header_(loader.get_header_description()) {}
};

#endif	// VECTOR_TRANSFORMER_HPP
