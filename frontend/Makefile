files := $(wildcard src/*.cpp)

paketLib := ../core/lib
fltkLib := /opt/homebrew/Cellar/fltk/1.3.9/lib
opensslLib := /opt/homebrew/Cellar/openssl@3/3.3.0/lib

frontend: $(files)
	g++ $(files) -std=c++20 -Wall -pedantic -L$(fltkLib) -lfltk -L$(opensslLib) -lcrypto -lssl -L$(paketLib) -lpaket -o debug/paket
	chmod u+x debug/paket

release: $(files)
	g++ $(files) -L$(fltkLib) -L$(openssl) -L$(paketLib) -std=c++20 -Wall -pedantic -O3 -lfltk -lcrypto -lssl -lpaket -o release/paket
	chmod u+x release/paket
