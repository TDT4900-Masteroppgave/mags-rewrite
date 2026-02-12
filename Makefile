BUILD_DIR     = build
VENV          = .venv

# We detect Windows (OS is usually 'Windows_NT') vs Linux/macOS
ifdef OS
   # Windows Paths
   PYTHON_CMD    = python
   VENV_BIN      = $(VENV)/Scripts
   VENV_PYTHON   = $(VENV_BIN)/python
   VENV_ACTIVATE = $(VENV_BIN)/activate
else
   # Linux/macOS Paths
   PYTHON_CMD    = python3
   VENV_BIN      = $(VENV)/bin
   VENV_PYTHON   = $(VENV_BIN)/python3
   VENV_ACTIVATE = $(VENV_BIN)/activate
endif

VENV_INVOKE   = $(VENV_PYTHON) -m invoke --search-root=scripts -c tasks

.PHONY: all setup release debug test clean data benchmark-small benchmark-large plot external

all: release external

# --- BUILD RULES ---

release:
	@echo "➡️  Configuring Release build..."
	@cmake -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release
	@echo "➡️  Compiling Release build..."
	@cmake --build $(BUILD_DIR) -j
	@echo "✅  Success! Executable built in $(BUILD_DIR)"

debug:
	@echo "➡️  Configuring Debug build..."
	@cmake -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug
	@echo "➡️  Compiling Debug build..."
	@cmake --build $(BUILD_DIR) -j
	@echo "✅  Success! Executable built in $(BUILD_DIR)"

test: debug
	@echo "➡️  Running Tests..."
	@cd $(BUILD_DIR) && ctest --output-on-failure

# --- PYTHON TASKS ---

$(VENV_ACTIVATE): requirements.txt
	@echo "🔧 Setting up Python Environment..."
	@$(PYTHON_CMD) -m venv $(VENV)
	@$(VENV_PYTHON) -m pip install --upgrade pip
	@$(VENV_PYTHON) -m pip install -r requirements.txt
	@$(VENV_PYTHON) -c "import os; from pathlib import Path; Path('$(VENV_ACTIVATE)').touch()"

setup: | $(VENV_ACTIVATE)
	@$(VENV_INVOKE) setup

external: setup
	@echo "➡️  Building Original MAGS..."
	@$(VENV_INVOKE) build-external

data: | $(VENV_ACTIVATE)
	@$(VENV_INVOKE) data

benchmark-small: release external data
	@$(VENV_INVOKE) benchmark --group small

benchmark-large: release external data
	@$(VENV_INVOKE) benchmark --group large

plot: | $(VENV_ACTIVATE)
	@$(VENV_INVOKE) plot

clean: | $(VENV_ACTIVATE)
	@$(VENV_INVOKE) clean