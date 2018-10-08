TARGET_EXEC = kaleidoscope

CC = clang++
SRC = $(wildcard *.cpp)
Wno_FLAGS = -Wno-unknown-warning-option
LLVM_FLAGS = `llvm-config --cxxflags --ldflags --system-libs --libs core`

all: $(SRC)
	$(CC) $(Wno_FLAGS) $(LLVM_FLAGS) -o $(TARGET_EXEC) $^

clean:
	rm -fr $(TARGET_EXEC)
