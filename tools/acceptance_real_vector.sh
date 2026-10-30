#!/bin/bash
# Acceptance test: run the parser on a truncated copy of the real reference vector.
# reference_vec/FRAMER_PCS_TX.hex is not in git (>100 MB); the test is skipped when it is absent.
# Note: the run takes ~80 s (read_all_vectors processes all 858723 vectors;
# data missing from the truncated file is padded with 0xFF - a known limitation, phase 5).
set -u
REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SRC="$REPO_ROOT/reference_vec/FRAMER_PCS_TX.hex"
BIN="$REPO_ROOT/build/hex_test"
LINES="${1:-2000}"

if [ ! -f "$SRC" ]; then
    echo "SKIP: $SRC not found (the file is not part of the repository)"
    exit 0
fi
if [ ! -x "$BIN" ]; then
    echo "FAIL: $BIN not found - run make first"
    exit 1
fi

TMP_HEX=$(mktemp /tmp/framer_head.XXXXXX.hex)
TMP_OUT=$(mktemp /tmp/framer_acc.XXXXXX.txt)
trap 'rm -f "$TMP_HEX" "$TMP_OUT"' EXIT

head -n "$LINES" "$SRC" > "$TMP_HEX"
"$BIN" "$TMP_HEX" > "$TMP_OUT" 2>&1
EC=$?

PASS=1
grep -q "BASIC HEADER VALIDATION PASSED" "$TMP_OUT" || { echo "FAIL: header validation not passed"; PASS=0; }
grep -q "Total test vectors: 858723"     "$TMP_OUT" || { echo "FAIL: unexpected vector count"; PASS=0; }
grep -q "All Vector SUCCESSFUL read"     "$TMP_OUT" || { echo "FAIL: vector read failure"; PASS=0; }
[ "$EC" -eq 0 ] || { echo "FAIL: exit code $EC"; PASS=0; }

if [ "$PASS" -eq 1 ]; then
    echo "ACCEPTANCE PASSED (first $LINES lines of the real FRAMER_PCS_TX.hex)"
    exit 0
fi
echo "--- last lines of the output ---"
tail -20 "$TMP_OUT"
exit 1
