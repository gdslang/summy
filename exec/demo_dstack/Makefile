# vim: noexpandtab

CC=gcc
#CC=clang
CPP=clang++-4.0
LIBDS=-L../..
LIBS=-lsummy -lbjutil -lcppgdsl -lgdsl-multiplex -ldl -lreadhex -lgmp -lgtest -lpthread -lelf #-lprofiler -ltcmalloc
LIBFLAGS=$(LIBDS) $(LIBS)
LDFLAGS=
INCDS=-I../../include -Iinclude
CFLAGS=$(CFLAGS_EXTRA) -c -O0 -ggdb3 -MMD -std=gnu99 -Wall -Wfatal-errors -DRELAXEDFATAL $(INCDS)
CPPFLAGS=$(CPPFLAGS_EXTRA) -c -O0 -ggdb3 -MMD -std=c++1z -fmodules-ts -DMODULES -fprebuilt-module-path=$(BPRE) -Wall -Wno-tautological-undefined-compare -Wno-overloaded-virtual -Wno-deprecated $(INCDS)

EXECUTABLE=summy

SPRE=src
HPRE=include
BPRE=build

CSOURCES=$(shell find $(SPRE)/ -type f -name '*.c')
CPPSOURCES=$(shell find $(SPRE)/ -type f -name '*.cpp')
CPPMODULES=$(shell find $(SPRE)/ -type f -name '*.ccm')

$(BPRE)/mod.pcm : | pre-build $(BPRE)/foo.pcm

all: pre-build $(EXECUTABLE)

SDIRS=$(shell find $(SPRE)/ -type d)
BDIRS=$(SDIRS:$(SPRE)/%=$(BPRE)/%)

pre-build:
	mkdir -p $(BDIRS)

COBJECTS=$(CSOURCES:$(SPRE)/%.c=$(BPRE)/%.o)
CPPOBJECTS=$(CPPSOURCES:$(SPRE)/%.cpp=$(BPRE)/%.o)
PCMS=$(CPPMODULES:$(SPRE)/%.ccm=$(BPRE)/%.pcm)
PCMS_LINK=$(CPPMODULES:$(SPRE)/%.ccm=-fmodule-file=$(BPRE)/%.pcm)
OBJECTS=$(COBJECTS) $(CPPOBJECTS)

$(EXECUTABLE): $(PCMS) $(OBJECTS) ../../lib$(EXECUTABLE).a
	$(CPP) $(LDFLAGS) -std=c++1z -fmodules-ts $(PCMS_LINK) -fprebuilt-module-path=$(BPRE) $(OBJECTS) $(LIBFLAGS) -o $@

-include ${COBJECTS:.o=.d}
$(COBJECTS): $(BPRE)/%.o : $(SPRE)/%.c
	$(CC) $(CFLAGS) $< -o $@

-include ${CPPOBJECTS:.o=.d}
$(CPPOBJECTS): $(BPRE)/%.o : $(SPRE)/%.cpp
	$(CPP) $(CPPFLAGS) $< -o $@


.NOTPARALLEL:
$(PCMS): $(BPRE)/%.pcm : $(SPRE)/%.ccm
	$(CPP) --precompile $(CPPFLAGS) $< -o $@

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
