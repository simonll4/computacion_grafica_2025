## Makefile para compilar el trazador de rayos
#
# Este Makefile compila los archivos fuente C++ utilizando un compilador
# compatible con C++17.  Se detectan automáticamente todos los archivos .cpp
# en la carpeta `src` y se generan objetos correspondientes.  El ejecutable
# resultante se denomina `raytracer` y se ubica en el directorio `build/`.
# Puede invocarse `make clean` para eliminar los objetos y el binario.

CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic -I./include

# Directorios
BUILD_DIR := build
OUTPUT_DIR := output

# Recopilar todos los archivos fuente en src/
SRCS := $(wildcard src/*.cpp)
OBJS := $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

TARGET := $(BUILD_DIR)/raytracer

all: $(TARGET)

# Regla para enlazar el ejecutable
$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Regla para compilar cada archivo .cpp a .o
$(BUILD_DIR)/%.o: src/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Crear directorios si no existen
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
