BUILD_DIR        = build
EXEC_NAME        = mags_rewrite
BENCH_SCRIPT     = benchmarking/cli.py
DATA_SCRIPT      = scripts/download_benchmark_data.sh

RESULTS_DIR = results
PLOTS_DIR = $(RESULTS_DIR)/plots

.PHONY: all release debug clean test help format data benchmark benchmark-small benchmark-large plot

all: clean release

directories:
	@mkdir -p $(RESULTS_DIR) $(PLOTS_DIR)

release:
	@echo "➡️  Configuring Release build..."
	@cmake -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release
	@echo "➡️  Compiling Release build..."
	@cmake --build $(BUILD_DIR) -j
	@echo "✅  Success! Executable: ./$(BUILD_DIR)/apps/$(EXEC_NAME)"

debug:
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

# ==========================================
# External Dependencies (Original MAGS)
# ==========================================
ORIGINAL_REPO_URL = https://github.com/nedchu/mags-release.git
ORIGINAL_DIR      = external/mags-release
ORIGINAL_EXEC     = $(ORIGINAL_DIR)/build/mags

# Setup & Build Original MAGS
$(ORIGINAL_EXEC):
	@echo "⬇️ Cloning Original MAGS..."
	@mkdir -p external
	@[ -d "$(ORIGINAL_DIR)" ] || git clone $(ORIGINAL_REPO_URL) $(ORIGINAL_DIR)

	@echo "🔧Patching Original Code"
	@# Fixes error: expected an OpenMP directive #pragma omp barier
	@perl -pi -e 's/pragma omp barier/pragma omp barrier/g' $(ORIGINAL_DIR)/src/pgsum.cpp

	@echo "🏗️ Building Original MAGS"
	@mkdir -p $(ORIGINAL_DIR)/build
	@# Added -w to suppress the flood of 'deprecated' warnings from parallel_hashmap
	@clang++ -O3 -std=c++17 -w \
		-Xpreprocessor -fopenmp \
		-I/opt/homebrew/opt/libomp/include \
		-L/opt/homebrew/opt/libomp/lib -lomp \
		-I$(ORIGINAL_DIR)/src \
		-I$(ORIGINAL_DIR)/src/parallel_hashmap \
		$(ORIGINAL_DIR)/src/*.cpp \
		$(ORIGINAL_DIR)/run/run_mags.cpp \
		-o $(ORIGINAL_DIR)/build/mags

	@echo "✅ Original MAGS Built Successfully"

# Copy Original Executable to ./build/mags
build/mags: $(ORIGINAL_EXEC)
	@mkdir -p build
	@cp $(ORIGINAL_EXEC) build/mags
	@echo "✅ Original MAGS installed to ./build/mags"

build-original: build/mags

benchmark-small: release build/mags data directories
	@echo "🚀 Running Small Graph Benchmarks (CA-DB)..."
	@python3 benchmarking/cli.py collect --group small --out results/data.json
	@echo "✅ Results saved to results/data.json"

benchmark-large: release build/mags data directories
	@echo "🚀 Running Large Graph Benchmarks (AM, YT, SK, LJ)..."
	@python3 benchmarking/cli.py collect --group large --out results/data.json
	@echo "✅ Results saved to results/data.json"

plot: directories
	@echo "📊 Plotting Results..."
	@python3 benchmarking/cli.py plot --input results/data.json --y relative_size --out results/plots/relative_size.png --title "Relative Size Comparison"
	@python3 benchmarking/cli.py plot --input results/data.json --y encoding --out results/plots/encoding_time.png --title "Encoding Time Comparison"
	@echo "✅ Plots saved to results/plots/"

data:
	@echo "📥 Checking for test datasets..."
	@chmod +x $(DATA_SCRIPT)
	@./$(DATA_SCRIPT)