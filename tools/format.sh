#!/bin/bash
# Format all project C/C++ sources per .clang-format (lowRISC/OpenTitan style
# guide; tabs-only indentation and alignment).
#
# Two passes:
#   1. clang-format: tab indentation (UseTab: Always), arguments aligned under
#      the open bracket, statement continuation lines at one level. Alignment
#      to a bracket column that is not a multiple of TabWidth leaves a 1-3
#      space remainder after the tabs.
#   2. tab snap: the remainder is rounded UP to the next tab stop, so leading
#      whitespace is pure tabs. All continuation lines of one statement share
#      the same column and snap identically, so alignment is preserved.
#      Doxygen continuation lines ("\t * text") are left untouched.
#
# Vendored third-party code (intelhexclass.*) is exempt from the style guide
# and is never touched.
set -eu
cd "$(dirname "$0")/.."

FILES=$(find src include \( -name '*.cpp' -o -name '*.h' -o -name '*.hpp' \) \
        | grep -v intelhexclass | sort)

clang-format -i $FILES

perl -pi -e 'if (/^(\t+)( +)(?=[^\s*])/) { $_ = ("\t" x (length($1) + 1)) . substr($_, length($1) + length($2)); }' $FILES

echo "Formatted $(echo "$FILES" | wc -l) files (clang-format + tab-snap)."
