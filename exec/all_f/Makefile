# vim: noexpandtab

CC=gcc
#CC=clang
CPP=clang++
LIBDS=-L../..
LIBS=-lsummy -lbjutil -lcppgdsl -lgdsl-multiplex -ldl -lreadhex -lcvc4 -lgmp -lgtest -lpthread -lelf
LIBFLAGS=$(LIBDS) $(LIBS)
LDFLAGS=
INCDS=-I../../include -Iinclude
CFLAGS=$(CFLAGS_EXTRA) -c -O0 -ggdb3 -MMD -std=gnu99 -Wall -Wfatal-errors -DRELAXEDFATAL $(INCDS)
CPPFLAGS=$(CPPFLAGS_EXTRA) -c -O0 -ggdb3 -MMD -std=c++14 -Wall -Wno-tautological-undefined-compare -Wno-overloaded-virtual -Wno-deprecated $(INCDS)

EXECUTABLE=summy-all_f

SPRE=src
HPRE=include
BPRE=build

CSOURCES=$(shell find $(SPRE)/ -type f -name '*.c')
CPPSOURCES=$(shell find $(SPRE)/ -type f -name '*.cpp')

all: pre-build $(EXECUTABLE)

SDIRS=$(shell find $(SPRE)/ -type d)
BDIRS=$(SDIRS:$(SPRE)/%=$(BPRE)/%)

pre-build:
	mkdir -p $(BDIRS)

COBJECTS=$(CSOURCES:$(SPRE)/%.c=$(BPRE)/%.o)
CPPOBJECTS=$(CPPSOURCES:$(SPRE)/%.cpp=$(BPRE)/%.o)
OBJECTS=$(COBJECTS) $(CPPOBJECTS)

$(EXECUTABLE): $(OBJECTS) ../../libsummy.a
	$(CPP) $(LDFLAGS) $(OBJECTS) $(LIBFLAGS) -o $@

-include ${COBJECTS:.o=.d}
$(COBJECTS): $(BPRE)/%.o : $(SPRE)/%.c
	$(CC) $(CFLAGS) $< -o $@

-include ${CPPOBJECTS:.o=.d}
$(CPPOBJECTS): $(BPRE)/%.o : $(SPRE)/%.cpp
	$(CPP) $(CPPFLAGS) $< -o $@

.PHONY: test
test:
	$(MAKE) -C test/

example: all
	$(MAKE) -C examples/ example EXAMPLE=$(EXAMPLE)
	./summy -f examples/example.bin -g playground/output.dot
	$(MAKE) -C playground/

example_elf: all
	./summy -e examples/example.elf -g playground/output.dot
	$(MAKE) -C playground/

clean:
#	$(MAKE) -C examples/ clean
#	$(MAKE) -C playground/ clean
#	$(MAKE) -C test/ clean
	rm -rf $(BDIRS) $(EXECUTABLE)
