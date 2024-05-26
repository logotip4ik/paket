sourceFiles := $(wildcard src/*) $(wildcard src/*/*.cpp)
buildFiles := $(wildcard src/*.cpp) $(wildcard src/*/*.cpp)
customFlags := -std=c++20 -pedantic -Wall

packet: $(sourceFiles)
	$(CXX) $(buildFiles) $(customFlags) -D DEBUG -L/opt/homebrew/Cellar/openssl@3/3.3.0/lib/ -lcrypto -lssl -o debug/paket

release: $(sourceFiles)
	$(CXX) $(buildFiles) $(customFlags) -D __PERF -L/opt/homebrew/Cellar/openssl@3/3.3.0/lib/ -lcrypto -lssl -o release/paket

release-lib: $(sourceFiles)
	$(CXX) $(buildFiles) $(customFlags) -D __PERF -c
	mv *.o lib/
	ar rcs lib/libpaket.a lib/*.o
	rm lib/*.o
