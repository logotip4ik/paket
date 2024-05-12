packet: $(wildcard src/*) $(wildcard src/*/*.cpp)
	g++ -std=c++20 -pedantic -Wall -I/usr/local/include -L/opt/homebrew/Cellar/openssl@3/3.3.0/lib/ -lcrypto -lssl -o debug/paket $(wildcard src/*.cpp) $(wildcard src/*/*.cpp)

release: $(wildcard src/*) $(wildcard src/*/*.cpp)
	g++ -D__PERF -std=c++20 -pedantic -Wall -O3 -I/usr/local/include -L/opt/homebrew/Cellar/openssl@3/3.3.0/lib/ -lcrypto -lssl -o release/paket $(wildcard src/*.cpp) $(wildcard src/*/*.cpp)
