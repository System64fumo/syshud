all: sysvol

clean:
	rm ./*.o
	rm sysvol

pulse.o: src/pulse.cpp
	g++ -g -c -o pulse.o src/pulse.cpp \
	$$(pkg-config gtkmm-4.0 --cflags --libs) \
	$$(pkg-config libpulse --cflags --libs) \
	-Wall -O2

main.o: src/main.cpp
	g++ -g -c -o main.o src/main.cpp \
	$$(pkg-config gtkmm-4.0 --cflags --libs) \
	$$(pkg-config gtk4-layer-shell-0 --cflags --libs) \
	-Wall -pthread -O2

sysvol: main.o pulse.o
	g++ -o sysvol main.o pulse.o \
	$$(pkg-config gtkmm-4.0 --cflags --libs) \
	$$(pkg-config gtk4-layer-shell-0 --cflags --libs) \
	$$(pkg-config libpulse --cflags --libs)
	strip sysvol
