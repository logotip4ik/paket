packet: $(wildcard src/*) $(wildcard src/*/*.cpp)
	g++ -std=c++20 -pedantic -Wall -o debug/paket $(wildcard src/*.cpp) $(wildcard src/*/*.cpp)

release: $(wildcard src/*) $(wildcard src/*/*.cpp)
	g++ -std=c++20 -pedantic -Wall -O3 -o release/paket $(wildcard src/*.cpp) $(wildcard src/*/*.cpp)
