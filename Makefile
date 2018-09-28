src = $(wildcard *.cpp)

llvm_flags = `llvm-config --cxxflags --ldflags --system-libs --libs core`
all: $(src)
	clang++ *.cpp `llvm-config --cxxflags --ldflags --system-libs --libs core` -o kaleidoscope
clean:
	rm -fr ./kaleidoscope.dSYM/ ./kaleidoscope
