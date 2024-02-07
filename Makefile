all: sysvol

clean:
	rm ./*.o
	rm sysvol

pulse.o: pulse.cpp
	g++ -g -c -o pulse.o pulse.cpp \
	$$(pkg-config gtkmm-4.0 --cflags --libs) \
	$$(pkg-config libpulse --cflags --libs) \
	-Wall -O2

main.o: main.cpp
	g++ -g -c -o main.o main.cpp \
	$$(pkg-config gtkmm-4.0 --cflags --libs) \
	$$(pkg-config gtk4-layer-shell-0 --cflags --libs) \
	-Wall -pthread -O2

sysvol: main.o pulse.o
	g++ -o sysvol main.o pulse.o \
	$$(pkg-config gtkmm-4.0 --cflags --libs) \
	$$(pkg-config gtk4-layer-shell-0 --cflags --libs) \
	$$(pkg-config libpulse --cflags --libs)
	strip sysvol
