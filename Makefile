# Makefile with automatic header dependency generation
CXX = g++
CXXFLAGS = -std=c++17
CXXFLAGS += -Wall
CXXFLAGS += -Wextra
CXXFLAGS += -g
CXXFLAGS += -O0
CXXFLAGS += -Iinclude
CXXFLAGS += -MMD -MP

# Default configuration - Little Endian
CXXFLAGS += -DHEX_LOADER_DEFAULT_ENDIANNESS=little_endian

# Default test pattern name
PATTERN ?= test_patterns.hex

BUILDDIR = build
TARGET = $(BUILDDIR)/hex_test
SRCDIR = src
INCLUDEDIR = include

# Automatic source discovery and object file generation
SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.o)
DEPFILES = $(OBJECTS:.o=.d)  # .d dependency files

# Default top-level target
all: $(TARGET)

# Link rule
$(TARGET): $(OBJECTS)
	@mkdir -p $(BUILDDIR)
	$(CXX) $(OBJECTS) -o $@

# Pattern rule: compile with build directory creation
$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(@D)  # Create directory for the .o file
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run with the default pattern
run: $(TARGET)
	./$(TARGET) test_vectors/$(PATTERN)

# Run with a pattern supplied via variable
run-pattern: $(TARGET)
	./$(TARGET) test_vectors/$(PATTERN)

# Run with an arbitrary file
run-file: $(TARGET)
ifndef FILE
	@echo "Usage: make run-file FILE=path/to/file.hex"
	@exit 1
endif
	./$(TARGET) $(FILE)

# Direct execution by .hex file name (convenience syntax)
%.hex: $(TARGET)
	./$(TARGET) $@

# Debug run
debug: $(TARGET)
	gdb --args $(TARGET) test_vectors/$(PATTERN)

# Build configurations with different flags
# Important: these targets must modify CXXFLAGS globally
le:
	$(MAKE) CXXFLAGS="$(filter-out -DHEX_LOADER_DEFAULT_ENDIANNESS=%,$(CXXFLAGS)) -DHEX_LOADER_DEFAULT_ENDIANNESS=little_endian"

be:
	$(MAKE) CXXFLAGS="$(filter-out -DHEX_LOADER_DEFAULT_ENDIANNESS=%,$(CXXFLAGS)) -DHEX_LOADER_DEFAULT_ENDIANNESS=big_endian"

no-checksum:
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -DHEX_LOADER_DISABLE_CHECKSUM"

verbose:
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -DHEX_LOADER_VERBOSE"

# Combined modes
le-verbose:
	$(MAKE) CXXFLAGS="$(filter-out -DHEX_LOADER_DEFAULT_ENDIANNESS=%,$(CXXFLAGS)) -DHEX_LOADER_DEFAULT_ENDIANNESS=little_endian -DHEX_LOADER_VERBOSE"

be-verbose:
	$(MAKE) CXXFLAGS="$(filter-out -DHEX_LOADER_DEFAULT_ENDIANNESS=%,$(CXXFLAGS)) -DHEX_LOADER_DEFAULT_ENDIANNESS=big_endian -DHEX_LOADER_VERBOSE"

# Multi-threaded build
parallel:
	@echo "Building with automatic thread detection..."
	@$(MAKE) -j$(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Fast build
fast: parallel

# Show current configuration
config:
	@echo "Current configuration:"
	@echo "  CXX: $(CXX)"
	@echo "  CXXFLAGS: $(CXXFLAGS)"
	@echo "  Target: $(TARGET)"
	@echo "  Pattern: $(PATTERN)"
	@echo "  Sources: $(SOURCES)"
	@echo "  Objects: $(OBJECTS)"
	@echo "  Depfiles: $(DEPFILES)"

# Show generated dependencies
show-deps:
	@echo "Generated dependency files:"
	@for dep in $(DEPFILES); do \
		if [ -f "$$dep" ]; then \
			echo "=== $$dep ==="; \
			cat "$$dep"; \
			echo ""; \
		else \
			echo "$$dep: not generated yet"; \
		fi; \
	done

# Dependency information
deps: show-deps

clean:
	rm -rf $(BUILDDIR)

# Rebuild
rebuild: clean all build

# Check whether a rebuild is needed
check:
	@if $(MAKE) -q; then \
		echo "Target is up to date"; \
	else \
		echo "Target needs to be rebuilt"; \
	fi

# Dry-run - show what would be done
dry:
	$(MAKE) -n

# Format all project sources per .clang-format (+ tabs-only normalization)
format:
	./tools/format.sh

# Include generated dependencies (must be at the end!)
-include $(DEPFILES)

# Help
help:
	@echo "Available targets:"
	@echo "  make               - Build with default settings (Little Endian)"
	@echo "  make -jN           - Build with N threads"
	@echo "  make parallel      - Build with automatic thread detection"
	@echo "  make fast          - Fast build (parallel)"
	@echo "  make run           - Run with default pattern ($(PATTERN))"
	@echo "  make run PATTERN=custom.hex - Run with custom pattern"
	@echo "  make run-file FILE=path/to/file.hex - Run with full file path"
	@echo "  make filename.hex  - Direct file execution"
	@echo "  make le            - Build in Little Endian mode"
	@echo "  make be            - Build in Big Endian mode"
	@echo "  make verbose       - Build with verbose parser"
	@echo "  make no-checksum   - Build without checksum verification"
	@echo "  make debug         - Run in debugger"
	@echo "  make config        - Show build configuration"
	@echo "  make deps          - Show generated dependencies"
	@echo "  make check         - Check if rebuild is needed"
	@echo "  make rebuild       - Clean and rebuild"
	@echo "  make clean         - Clean build files"
	@echo "  make dry           - Show what would be built (dry run)"
	@echo ""
	@echo "Advanced features:"
	@echo "  - Automatic header dependency tracking (-MMD -MP)"
	@echo "  - Thread-safe directory creation"
	@echo ""
	@echo "Examples:"
	@echo "  make -j8                           # Build with 8 threads"
	@echo "  make parallel run                  # Fast build and run"
	@echo "  make run PATTERN=my_firmware.hex   # Run with custom pattern"
	@echo "  make run-file FILE=test_vectors/my_firmware.hex"
	@echo "  make test_vectors/my_firmware.hex  # Direct file execution"

.PHONY: all run run-pattern run-file debug le be no-checksum verbose le-verbose be-verbose parallel fast config deps show-deps check dry clean rebuild format help
