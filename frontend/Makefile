files := $(wildcard src/*.cpp)

paketLib := ../core/lib
fltkLib := /opt/homebrew/Cellar/fltk/1.3.9/lib
opensslLib := /opt/homebrew/Cellar/openssl@3/3.3.0/lib

customFlags := -std=c++20 -Wall -pedantic -L$(fltkLib) -lfltk -L$(opensslLib) -lcrypto -lssl -L$(paketLib) -lpaket

frontend: $(files)
	g++ $(files) $(customFlags) -o debug/paket

release: $(files)
	g++ $(files) $(customFlags) -O3 -o release/paket
