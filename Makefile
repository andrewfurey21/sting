CXX = g++
CXXFLAGS = -Isrc -std=c++17 -g -O0
ASAN = # -fsanitize=address

SRC_DIR = src
BUILD_DIR = build

SRC = $(wildcard $(SRC_DIR)/*.cpp)
HEADERS = $(wildcard $(SRC_DIR)/*.hpp)
OBJ = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRC))

TARGET = sting

all: $(TARGET)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS) $(BUILD_DIR)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(OBJ)
	$(CXX) -o $@ $^ $(ASAN)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(TARGET)
