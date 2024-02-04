all: sysvol

clean:
	rm sysvol

sysvol: main.cpp
	g++ -o sysvol main.cpp \
	$$(pkg-config gtkmm-4.0 --cflags --libs) \
	$$(pkg-config gtk4-layer-shell-0 --cflags --libs) \
	$$(pkg-config libpulse --cflags --libs) \
	-pthread -O3
	strip sysvol
