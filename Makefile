# ── Config ───────────────────────────────────────────────────────
BUILD_DIR = build
CONFIG    = Release

ifdef DEBUG
  CONFIG = Debug
endif

TARGET = $(BUILD_DIR)/$(CONFIG)/evo_sim.exe

# ── Targets ───────────────────────────────────────────────────────
.PHONY: all run clean reconfigure

all: $(TARGET)

# Configure only if CMakeCache.txt doesn't exist yet
$(BUILD_DIR)/CMakeCache.txt:
	cmake -B $(BUILD_DIR)

# Build via CMake (re-runs automatically if sources changed)
$(TARGET): $(BUILD_DIR)/CMakeCache.txt $(wildcard src/*.c include/*.h)
	cmake --build $(BUILD_DIR) --config $(CONFIG)

run: all
	./$(TARGET)

# Force a clean CMake reconfigure (e.g. after adding files)
reconfigure:
	rm -rf $(BUILD_DIR)
	cmake -B $(BUILD_DIR)

clean:
	cmake --build $(BUILD_DIR) --target clean 2>/dev/null || rm -rf $(BUILD_DIR)
