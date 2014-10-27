# vim: noexpandtab

GDSL=../gdsl-toolkit
CC=gcc
#CC=clang
CPP=clang++
LIBDS=-L$(GDSL)/lib
LIBS=-lcppgdsl -lgdsl-multiplex -ldl -lreadhex -lcvc4 -lgmp -lgtest -lpthread -lelf
LIBFLAGS=$(LIBDS) $(LIBS)
LDFLAGS=
INCDS=-Iinclude -I$(GDSL)/include
CFLAGS=-c -Os -ggdb3 -MMD -std=gnu99 -Wall -Wfatal-errors -DRELAXEDFATAL $(INCDS)
CPPFLAGS=-c -Os -ggdb3 -MMD -std=c++14 -Wall -Wno-overloaded-virtual -Wno-deprecated $(INCDS)

EXECUTABLE=summy

SPRE=src
HPRE=include
BPRE=build

CSOURCES=$(shell find $(SPRE)/ -type f -name '*.c')
CPPSOURCES=$(shell find $(SPRE)/ -type f -name '*.cpp')

.PHONY: test

all: pre-build $(EXECUTABLE)

SDIRS=$(shell find $(SPRE)/ -type d)
BDIRS=$(SDIRS:$(SPRE)/%=$(BPRE)/%)

pre-build:
	mkdir -p $(BDIRS)

COBJECTS=$(CSOURCES:$(SPRE)/%.c=$(BPRE)/%.o)
CPPOBJECTS=$(CPPSOURCES:$(SPRE)/%.cpp=$(BPRE)/%.o)
OBJECTS=$(COBJECTS) $(CPPOBJECTS)

$(EXECUTABLE): $(OBJECTS)
	$(CPP) $(LDFLAGS) $(OBJECTS) $(LIBFLAGS) -o $@

-include ${COBJECTS:.o=.d}
$(COBJECTS): $(BPRE)/%.o : $(SPRE)/%.c
	$(CC) $(CFLAGS) $< -o $@

-include ${CPPOBJECTS:.o=.d}
$(CPPOBJECTS): $(BPRE)/%.o : $(SPRE)/%.cpp
	$(CPP) $(CPPFLAGS) $< -o $@

test:
	$(MAKE) -C test/

example: all
	$(MAKE) -C examples/ example EXAMPLE=$(EXAMPLE)
	./tardet -f examples/example.bin -g playground/output.dot
	$(MAKE) -C playground/

example_elf: all
	./tardet -e examples/example.elf -g playground/output.dot
	$(MAKE) -C playground/

clean:
#	$(MAKE) -C examples/ clean
#	$(MAKE) -C playground/ clean
#	$(MAKE) -C test/ clean
	rm -rf $(BDIRS) $(EXECUTABLE)
