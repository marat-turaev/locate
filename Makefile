CXX := clang++
CXXFLAGS := -Wall -pedantic -g -std=c++11
LIBS := -lboost_program_options -lboost_filesystem -lboost_system -lboost_thread-mt

OS := $(shell uname -s)

ifeq ($(OS),Darwin)
BOOST_PATHS := -I/usr/local/include -L/usr/local/lib
CXXFLAGS += -stdlib=libc++
endif

SRCDIR := src
OUTDIR := obj
EXECUTABLES := updatedb locate


.PHONY: clean

all: before $(EXECUTABLES)

before:
	mkdir -p $(OUTDIR)

clean:
	rm -rf $(OUTDIR) $(EXECUTABLES)

locate: $(OUTDIR)/locate.o
	$(CXX) $(CXXFLAGS) -o locate $^ $(BOOST_PATHS) $(LIBS)

updatedb: $(OUTDIR)/updatedb.o
	$(CXX) $(CXXFLAGS) -o updatedb $^ $(BOOST_PATHS) $(LIBS)

$(OUTDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<
