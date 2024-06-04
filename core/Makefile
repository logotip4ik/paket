sourceFiles := $(wildcard src/*) $(wildcard src/*/*.cpp)
buildFiles := $(wildcard src/*.cpp) $(wildcard src/*/*.cpp)
customFlags := -std=c++20 -pedantic -Wall

packet: $(sourceFiles)
	$(CXX) $(buildFiles) $(customFlags) -D DEBUG -L/opt/homebrew/Cellar/openssl@3/3.3.0/lib/ -lcrypto -lssl -o debug/paket

release: $(sourceFiles)
	$(CXX) $(buildFiles) $(customFlags) -D __PERF -O3 -L/opt/homebrew/Cellar/openssl@3/3.3.0/lib/ -lcrypto -lssl -o release/paket

lib: $(sourceFiles)
	$(CXX) $(buildFiles) $(customFlags) -D DEBUG -c
	mv *.o lib/
	ar rcs lib/libpaket.a lib/*.o
	rm lib/*.o
	cp src/paket.h include/paket.h

release-lib: $(sourceFiles)
	$(CXX) $(buildFiles) $(customFlags) -D __PERF -O3 -c
	mv *.o lib/
	ar rcs lib/libpaket.a lib/*.o
	rm lib/*.o
	cp src/paket.h include/paket.h
