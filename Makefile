# ==========================================
# Mags-Rewrite Makefile
# Wrapper around CMake for easy Release/Debug switching
# ==========================================

# Configuration
BUILD_DIR_RELEASE = build/release
BUILD_DIR_DEBUG   = build/debug
EXEC_NAME         = mags_rewrite

# Phony targets (not real files)
.PHONY: all release debug clean test help format

# Default target: Build Release
all: release

# -----------------------------------------------------------------------------
# Release Build (-O3, No Debug Info)
# Best for: Benchmarking, Production
# -----------------------------------------------------------------------------
release:
	@echo "➡️  Configuring Release build..."
	@cmake -B $(BUILD_DIR_RELEASE) -DCMAKE_BUILD_TYPE=Release
	@echo "➡️  Compiling Release build..."
	@cmake --build $(BUILD_DIR_RELEASE) -j
	@echo "✅  Success! Executable: ./$(BUILD_DIR_RELEASE)/apps/$(EXEC_NAME)"

# -----------------------------------------------------------------------------
# Debug Build (-g, No Optimizations)
# Best for: GDB/LLDB, Valgrind, Running Tests
# -----------------------------------------------------------------------------
debug:
	@echo "➡️  Configuring Debug build..."
	@cmake -B $(BUILD_DIR_DEBUG) -DCMAKE_BUILD_TYPE=Debug
	@echo "➡️  Compiling Debug build..."
	@cmake --build $(BUILD_DIR_DEBUG) -j
	@echo "✅  Success! Executable: ./$(BUILD_DIR_DEBUG)/apps/$(EXEC_NAME)"

# -----------------------------------------------------------------------------
# Run Tests
# Builds Debug first, then runs CTest
# -----------------------------------------------------------------------------
test: debug
	@echo "➡️  Running Tests..."
	@cd $(BUILD_DIR_DEBUG) && ctest --output-on-failure

# -----------------------------------------------------------------------------
# Utilities
# -----------------------------------------------------------------------------
clean:
	@echo "🧹 Cleaning all build artifacts..."
	@rm -rf build
	@echo "✅  Done."

help:
	@echo "Usage:"
	@echo "  make          Build Release mode (Default)"
	@echo "  make release  Configure and build Release mode"
	@echo "  make debug    Configure and build Debug mode"
	@echo "  make test     Build Debug mode and run tests"
	@echo "  make clean    Remove the 'build' directory"