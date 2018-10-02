TARGET_EXEC = kaleidoscope

CC = clang++
SRC = $(wildcard *.cpp)
LLVM_FLAGS = `llvm-config --cxxflags --ldflags --system-libs --libs core`

all: $(SRC)
	$(CC) $(LLVM_FLAGS) -o $(TARGET_EXEC) $^

clean:
	rm -fr $(TARGET_EXEC)
