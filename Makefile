CXXFLAGS=-march=native -mtune=native -Os -s -Wall
DESTDIR=$(HOME)/.local
all: sysvol

clean:
	rm ./*.o
	rm sysvol

pulse.o: src/pulse.cpp
	g++ -o pulse.o -c src/pulse.cpp \
	$$(pkg-config gtkmm-4.0 --cflags --libs) \
	$$(pkg-config libpulse --cflags --libs) \
	$(CXXFLAGS)

main.o: src/main.cpp
	g++ -o main.o -c src/main.cpp \
	$$(pkg-config gtkmm-4.0 --cflags --libs) \
	$$(pkg-config gtk4-layer-shell-0 --cflags --libs) \
	$(CXXFLAGS) -pthread

sysvol: main.o pulse.o
	g++ -o sysvol main.o pulse.o \
	$$(pkg-config gtkmm-4.0 --cflags --libs) \
	$$(pkg-config gtk4-layer-shell-0 --cflags --libs) \
	$$(pkg-config libpulse --cflags --libs) \
	$(CXXFLAGS)

install: sysvol
	mkdir -p $(DESTDIR)/bin
	install ./sysvol $(DESTDIR)/bin/sysvol
