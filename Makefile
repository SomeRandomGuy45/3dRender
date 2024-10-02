# Define the source and output
TARGET = obj_loader
SRC = obj_loader.cpp

# Platform detection
UNAME_S := $(shell uname -s)

# Compiler and standard
CXX = g++
CXXFLAGS = --std=c++20 -lm

# Libraries and frameworks
LIBS_LINUX = -lGL -lGLEW -lglfw -lassimp
LIBS_MACOS = -framework OpenGL -lGLEW -lglfw -lassimp
LIBS_WINDOWS = -lopengl32 -lglew32 -lglfw3 -lassimp

# Platform-specific settings
ifeq ($(UNAME_S), Linux)
    LIBS = $(LIBS_LINUX)
    EXE = $(TARGET)
else ifeq ($(UNAME_S), Darwin)  # macOS
    LIBS = $(LIBS_MACOS)
    EXE = $(TARGET)
else  # Windows
    LIBS = $(LIBS_WINDOWS)
    EXE = $(TARGET).exe
    CXXFLAGS += -DGLFW_INCLUDE_NONE  # In case you need to avoid including OpenGL headers with GLFW
endif

# Build target
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(EXE) $(SRC) $(LIBS)

# Clean up the build
clean:
	rm -f $(TARGET) $(TARGET).exe