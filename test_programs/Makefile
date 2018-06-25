# vim: noexpandtab

MLTK=../../..
CC=clang
CPP=clang++
#CC=clang
LIBDS=-L$(MLTK)/lib
LIBS=
OPT ?= 1
LIBFLAGS=$(LIBDS) $(LIBS)
LDFLAGS=-static
INCDS=-Iinclude -I$(MLTK)/include
#CFLAGS=-c -fomit-frame-pointer -O0 -g3 -std=gnu99 -Wall -Wfatal-errors -DRELAXEDFATAL $(INCDS)
#CPPFLAGS=-c -fomit-frame-pointer -fpermissive -O0 -g3 -std=c++11 -Wall -Wfatal-errors -DRELAXEDFATAL $(INCDS)
CFLAGS=$(CFLAGS_EXTRA) -g3 -c -mcmodel=large -O$(OPT) -std=c11 -fno-inline -fno-optimize-sibling-calls -w $(INCDS)
CPPFLAGS=$(CPPFLAGS_EXTRA) -g3 -c -mcmodel=large -O$(OPT) -std=c++11 -fno-inline -fno-optimize-sibling-calls $(INCDS)


SPRE=.
BPRE=build

SOURCES_C=$(shell find $(SPRE)/ -type f -name 'test*.c' -o -name 'example*.c')
EXECUTABLES_C=$(notdir $(SOURCES_C:.c=))
SOURCES_CPP=$(shell find $(SPRE)/ -type f -name 'test*.cpp' -o -name 'example*.cpp')
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
