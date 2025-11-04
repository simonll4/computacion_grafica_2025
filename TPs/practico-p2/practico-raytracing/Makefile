# Makefile for Ray Tracer
# Computación Gráfica - Práctico 2

CXX = g++
CXXFLAGS = -std=c++17 -O3 -Wall -fopenmp -I./include -I./src
LDFLAGS = -lgomp -lm

TARGET = build/RayTracer
SRCDIR = src
BUILDDIR = build

# Source files (recursivo)
SOURCES := $(shell find $(SRCDIR) -name '*.cpp')

# Object files en espejo de la estructura de src/
OBJECTS := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SOURCES))

# Default target
all: $(TARGET)

# Link the executable
$(TARGET): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo "Build complete: $(TARGET)"

# Pattern rule para compilar cualquier .cpp bajo src/
$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run the ray tracer
run: $(TARGET)
	./$(TARGET)

# Clean build files
clean:
	rm -rf $(BUILDDIR) *.ppm *.bmp

# Clean results
clean-results:
	rm -rf results/*.ppm results/*.bmp

# Rebuild everything
rebuild: clean all

.PHONY: all clean clean-results run rebuild
