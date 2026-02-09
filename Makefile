# ==========================================
# Mags-Rewrite Makefile
# Wrapper around CMake for easy Release/Debug switching
# ==========================================

# Configuration
BUILD_DIR   = build
EXEC_NAME   = mags_rewrite

# Phony targets (not real files)
.PHONY: all release debug clean test help format

# Default target: Build Release
all: release

release: clean
	@echo "➡️  Configuring Release build..."
	@cmake -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release
	@echo "➡️  Compiling Release build..."
	@cmake --build $(BUILD_DIR) -j
	@echo "✅  Success! Executable: ./$(BUILD_DIR)/apps/$(EXEC_NAME)"

debug: clean
	@echo "➡️  Configuring Debug build..."
	@cmake -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug
	@echo "➡️  Compiling Debug build..."
	@cmake --build $(BUILD_DIR) -j
	@echo "✅  Success! Executable: ./$(BUILD_DIR)/apps/$(EXEC_NAME)"

test: clean debug
	@echo "➡️  Running Tests..."
	@cd $(BUILD_DIR) && ctest --output-on-failure

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