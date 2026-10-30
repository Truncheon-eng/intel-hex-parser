#!/bin/bash
# create_test_runner.sh

echo "Running HEX Loader Test Suite"
echo "================================="

test_files=(
    "test_gaps.hex"
    "test_extended.hex" 
    "test_repeating.hex"
    "test_ascii.hex"
    "test_mixed_sizes.hex"
    "test_zeros.hex"
    "test_full_lines.hex"
    "test_sequence.hex"
    "test_patterns.hex"
    "test_canonical_4vec.hex"
    "test_canonical_1000vec.hex"
)

for file in "${test_files[@]}"; do
    if [ -f "test_vectors/$file" ]; then
        echo ""
        echo "Testing: $file"
        echo "----------------------------------------"
        ./build/hex_test "test_vectors/$file"
    else
        echo "❌ File not found: test_vectors/$file"
    fi
done

echo ""
echo "✅ Test suite completed!"
