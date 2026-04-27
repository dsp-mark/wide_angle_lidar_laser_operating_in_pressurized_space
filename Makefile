CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2 -Iincludes -Ilib/Adafruit_BNO08x/src
LDFLAGS = -llgpio -lrt -lwiringPi
TARGET = wallops
SRC = src/main.cpp
INCLUDES = includes/*.hpp

DATA_DIR = scan_data
SCAN_LOG_DIR = scan_logs
LIDAR_DIR = lidar_data
SCAN_DIRS = $(DATA_DIR) $(LOG_DIR) $(LIDAR_DIR)

LGPIO_REPO = https://github.com/joan2937/lg.git
LGPIO_DIR = /tmp/lgpio_build

BNO08X_REPO = https://github.com/adafruit/Adafruit_BNO08x.git
BNO08X_DIR = lib/Adafruit_BNO08x

.PHONY: all install-deps clean run flash setup-folders setup status debug

setup-folders:
	mkdir -p $(SCAN_DIRS)

all: setup-folders $(TARGET)

$(TARGET): $(SRC) includes/WireShim.cpp $(INCLUDES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) includes/WireShim.cpp $(LDFLAGS)

install-deps:
	@echo "Checking lgpio..."
	@if ! pkg-config --exists lgpio 2>/dev/null; then \
		echo "Installing lgpio from $(LGPIO_REPO)..."; \
		rm -rf $(LGPIO_DIR); \
		git clone $(LGPIO_REPO) $(LGPIO_DIR); \
		cd $(LGPIO_DIR) && make -j4; \
		sudo make install; \
		sudo ldconfig; \
		rm -rf $(LGPIO_DIR) \
		echo "lgpio installed"; \
	else \
		echo "lgpio already installed";\
	fi

		@echo "Checking wiringPi..."
	@if ! dpkg -l | grep -q wiringpi; then \
		echo "Installing wiringPi..."; \
		sudo apt update; \
		sudo apt install -y wiringpi; \
	else \
		echo "wiringPi already installed"; \
	fi

	@echo "Checking Adafruit_BNO08x..."
	@if [ ! -d "$(BNO08X_DIR)" ]; then \
		echo "Cloning Adafruit_BNO08x from $(BNO08X_REPO)..."; \
		mkdir -p lib; \
		git clone $(BNO08X_REPO) $(BNO08X_DIR); \
		cd $(BNO08X_DIR) && git submodule update --init; \
		echo "Adafruit_BNO08x cloned"; \
	else \
		echo "Adafruit_BNO08x already cloned"; \
	fi

	sudo apt update && sudo apt install -y pkg-config libatomic1 git wiringpi

debug: CXXFLAGS += -g --DDEBUG
debug: $(TARGET)

clean:
	rm -f $(TARGET)
	rm -rf $(SCAN_DIRS)/*

run: setup-folders $(TARGET)
	sudo SCAN_DATA_DIR=$(DATA_DIR) SCAN_LOG_DIR=$(SCAN_LOG_DIR) ./$(TARGET)

flash: all run

setup: install-deps setup-folders all

status:
	@pkg-config --exists lgpio && echo "lgpio OK" || echo "lgpio MISSING"
	@[ -d "$(DATA_DIR)" ] && echo "$(DATA_DIR)/ OK" || echo "$(DATA_DIR)/ MISSING"
	@[ -d "$(LOG_DIR)" ] && echo "$(LOG_DIR)/ OK" || echo "$(LOG_DIR)/ MISSING"
	@[ -f "$(TARGET)" ] && echo "$(TARGET) built" || echo "$(TARGET) missing"
	@[ -d "$(BNO08X_DIR)" ] && echo "BNO08x lib OK" || echo "BNO08x lib MISSING"

.PHONY: all debug clean run flash setup status setup-folders install-deps