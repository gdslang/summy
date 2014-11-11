# vim: noexpandtab

MLTK=../../..
CC=gcc
CPP=g++
#CC=clang
LIBDS=-L$(MLTK)/lib
LIBS=
LIBFLAGS=$(LIBDS) $(LIBS)
LDFLAGS=-static
INCDS=-Iinclude -I$(MLTK)/include
CFLAGS=-c -fomit-frame-pointer -O0 -g3 -std=gnu99 -Wall -Wfatal-errors -DRELAXEDFATAL $(INCDS)
CPPFLAGS=-c -fomit-frame-pointer -fpermissive -O0 -g3 -std=c++11 -Wall -Wfatal-errors -DRELAXEDFATAL $(INCDS)

SPRE=.
BPRE=build

SOURCES_C=$(shell find $(SPRE)/ -type f -name '*.c')
EXECUTABLES_C=$(notdir $(SOURCES_C:.c=))
SOURCES_CPP=$(shell find $(SPRE)/ -type f -name '*.cpp')
EXECUTABLES_CPP=$(notdir $(SOURCES_CPP:.cpp=))

all: pre-build $(EXECUTABLES_C) $(EXECUTABLES_CPP)

BDIRS=$(BPRE)
pre-build:
	mkdir -p $(BDIRS)

OBJECTS_C=$(SOURCES_C:$(SPRE)/%.c=$(BPRE)/%.o)

$(EXECUTABLES_C): % : $(BPRE)/%.o
	$(CC) $(LDFLAGS) $< $(LIBFLAGS) -o $@

$(OBJECTS_C): $(BPRE)/%.o : $(SPRE)/%.c
	$(CC) $(CFLAGS) $< -o $@

OBJECTS_CPP=$(SOURCES_CPP:$(SPRE)/%.cpp=$(BPRE)/%.o)

$(EXECUTABLES_CPP): % : $(BPRE)/%.o
	$(CPP) $(LDFLAGS) $< $(LIBFLAGS) -o $@

$(OBJECTS_CPP): $(BPRE)/%.o : $(SPRE)/%.cpp
	$(CPP) $(CPPFLAGS) $< -o $@

example: all
	./$(EXAMPLE)

clean:
	rm -rf $(BDIRS) $(EXECUTABLES_C) $(EXECUTABLES_CPP)
