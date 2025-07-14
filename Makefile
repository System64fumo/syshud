BIN = syshud
LIB = libsyshud.so
PKGS = Qt6Widgets
SRCS = $(wildcard src/*.cpp)

PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
LIBDIR ?= $(PREFIX)/lib
DATADIR ?= $(PREFIX)/share
BUILDDIR = build

SRCS := $(filter-out src/wireplumber.cpp,$(SRCS))
SRCS := $(filter-out src/pulse.cpp,$(SRCS))
SRCS := $(filter-out src/backlight.cpp,$(SRCS))
SRCS := $(filter-out src/keytoggles.cpp,$(SRCS))

# TODO: Add support for both pulse and wp (Auto detect)
ifneq (, $(shell grep -E '^#define FEATURE_PULSEAUDIO' src/config.hpp))
	SRCS += src/pulse.cpp
	PKGS += libpulse
endif
ifneq (, $(shell grep -E '^#define FEATURE_WIREPLUMBER' src/config.hpp))
	SRCS += src/wireplumber.cpp
	PKGS += wireplumber-0.5
endif
ifneq (, $(shell grep -E '^#define FEATURE_BACKLIGHT' src/config.hpp))
	SRCS += src/backlight.cpp
endif
ifneq (, $(shell grep -E '^#define FEATURE_KEYBOARD' src/config.hpp))
	SRCS += src/keytoggles.cpp
	PKGS += libevdev
endif

OBJS = $(patsubst src/%, $(BUILDDIR)/%, $(SRCS:.cpp=.o))

CXXFLAGS += -Oz -s -Wall -flto -fno-exceptions -fPIC -DQT_NO_VERSION_TAGGING
LDFLAGS += -Wl,--no-as-needed,-z,now,-z,pack-relative-relocs

CXXFLAGS += $(shell pkg-config --cflags $(PKGS))
LDFLAGS += $(shell pkg-config --libs $(PKGS))

$(shell mkdir -p $(BUILDDIR))
JOB_COUNT := $(BIN) $(LIB) $(OBJS) src/git_info.hpp
JOBS_DONE := $(shell ls -l $(JOB_COUNT) 2> /dev/null | wc -l)

define progress
	$(eval JOBS_DONE := $(shell echo $$(($(JOBS_DONE) + 1))))
	@printf "[$(JOBS_DONE)/$(shell echo $(JOB_COUNT) | wc -w)] %s %s\n" $(1) $(2)
endef

all: $(BIN) $(LIB)

install: $(all)
	@echo "Installing..."
	@install -D -t $(DESTDIR)$(BINDIR) $(BUILDDIR)/$(BIN)
	@install -D -t $(DESTDIR)$(LIBDIR) $(BUILDDIR)/$(LIB)
	@install -D -t $(DESTDIR)$(DATADIR)/sys64/hud config.conf style.qss

clean:
	@echo "Cleaning up"
	@rm -r $(BUILDDIR) src/git_info.hpp

$(BIN): src/git_info.hpp $(BUILDDIR)/main.o $(BUILDDIR)/config_parser.o
	$(call progress, Linking $@)
	@$(CXX) -o $(BUILDDIR)/$(BIN) \
	$(BUILDDIR)/main.o \
	$(BUILDDIR)/config_parser.o \
	$(CXXFLAGS) \
	$(LDFLAGS)

$(LIB): $(OBJS)
	$(call progress, Linking $@)
	@$(CXX) -o $(BUILDDIR)/$(LIB) \
	$(filter-out $(BUILDDIR)/main.o $(BUILDDIR)/config_parser.o, $(OBJS)) \
	$(CXXFLAGS) \
	$(LDFLAGS) \
	-shared -lLayerShellQtInterface

$(BUILDDIR)/%.o: src/%.cpp
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
