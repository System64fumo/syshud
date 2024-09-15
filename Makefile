BINS = syshud
LIBS = libsyshud.so
PKGS = gtkmm-4.0 gtk4-layer-shell-0 libevdev
SRCS = $(wildcard src/*.cpp)
OBJS = $(SRCS:.cpp=.o)

PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
LIBDIR ?= $(PREFIX)/lib
DATADIR ?= $(PREFIX)/share

ifeq ($(PULSEAUDIO),1)
	PKGS += libpulse
	CXXFLAGS += -DPULSEAUDIO
	SRCS := $(filter-out src/wireplumber.cpp,$(SRCS))
	OBJS := $(filter-out src/wireplumber.o,$(OBJS))
else
	PKGS += wireplumber-0.5
	SRCS := $(filter-out src/pulse.cpp,$(SRCS))
	OBJS := $(filter-out src/pulse.o,$(OBJS))
endif

CXXFLAGS += -Oz -s -Wall -flto -fno-exceptions -fPIC
LDFLAGS += -Wl,--as-needed,-z,now,-z,pack-relative-relocs

CXXFLAGS += $(shell pkg-config --cflags $(PKGS))
LDFLAGS += $(shell pkg-config --libs $(PKGS))

JOB_COUNT := $(EXEC) $(LIB) $(OBJS) src/git_info.hpp
JOBS_DONE := $(shell ls -l $(JOB_COUNT) 2> /dev/null | wc -l)

define progress
	$(eval JOBS_DONE := $(shell echo $$(($(JOBS_DONE) + 1))))
	@printf "[$(JOBS_DONE)/$(shell echo $(JOB_COUNT) | wc -w)] %s %s\n" $(1) $(2)
endef

all: $(BINS) $(LIBS)

install: $(all)
	@echo "Installing..."
	@install -D -t $(DESTDIR)$(BINDIR) $(BINS)
	@install -D -t $(DESTDIR)$(LIBDIR) $(LIBS)
	@install -D -t $(DESTDIR)$(DATADIR)/sys64/hud config.conf style.css

clean:
	@echo "Cleaning up"
	@rm $(BINS) $(LIBS) $(OBJS) src/git_info.hpp

$(BINS): src/git_info.hpp src/main.o src/config_parser.o
	$(call progress, Linking $@)
	@$(CXX) -o $(BINS) \
	src/main.o \
	src/config_parser.o \
	$(CXXFLAGS) \
	$(shell pkg-config --libs gtkmm-4.0 gtk4-layer-shell-0)

$(LIBS): $(OBJS)
	$(call progress, Linking $@)
	@$(CXX) -o $(LIBS) \
	$(filter-out src/main.o src/config_parser.o, $(OBJS)) \
	$(CXXFLAGS) \
	$(LDFLAGS) \
	-shared

%.o: %.cpp
	$(call progress, Compiling $@)
	@$(CXX) -c $< -o $@ \
	$(CXXFLAGS)

src/git_info.hpp:
	$(call progress, Creating $@)
	@commit_hash=$$(git rev-parse HEAD); \
	commit_date=$$(git show -s --format=%cd --date=short $$commit_hash); \
	commit_message=$$(git show -s --format="%s" $$commit_hash | sed 's/"/\\\"/g'); \
	echo "#define GIT_COMMIT_MESSAGE \"$$commit_message\"" > src/git_info.hpp; \
	echo "#define GIT_COMMIT_DATE \"$$commit_date\"" >> src/git_info.hpp
