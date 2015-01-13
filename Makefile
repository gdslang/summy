CC=gcc
#CC=clang
CPP=clang++
INCDS=-Iinclude
CFLAGS=-c -MMD -ggdb3 -O2 -std=c11 -Wall -Wfatal-errors -DRELAXEDFATAL $(INCDS)
CPPFLAGS=-c -MMD -ggdb3 -O2 -std=c++14 -Wall -Wno-tautological-undefined-compare -Wno-overloaded-virtual -Wno-deprecated $(INCDS)

LIBRARY=libsummy.a

SPRE=src
HPRE=include
BPRE=build

CSOURCES=$(shell find $(SPRE)/ -type f -name '*.c')
CPPSOURCES=$(shell find $(SPRE)/ -type f -name '*.cpp')

all: pre-build $(LIBRARY)

SDIRS=$(shell find $(SPRE)/ -type d)
BDIRS=$(SDIRS:$(SPRE)/%=$(BPRE)/%)

pre-build:
	mkdir -p $(BDIRS)

COBJECTS=$(CSOURCES:$(SPRE)/%.c=$(BPRE)/%.o)
CPPOBJECTS=$(CPPSOURCES:$(SPRE)/%.cpp=$(BPRE)/%.o)
OBJECTS=$(COBJECTS) $(CPPOBJECTS)

$(LIBRARY): $(OBJECTS)
	ar rvs $@ $(OBJECTS)

-include ${COBJECTS:.o=.d}
$(COBJECTS): $(BPRE)/%.o : $(SPRE)/%.c
	$(CC) $(CFLAGS) $< -o $@

-include ${CPPOBJECTS:.o=.d}
$(CPPOBJECTS): $(BPRE)/%.o : $(SPRE)/%.cpp
	$(CPP) $(CPPFLAGS) $< -o $@

.PHONY: demo
demo: all
	$(MAKE) -C exec/demo/
.PHONY: test
test: all
	$(MAKE) -C exec/tester/ && exec/tester/summy-tester

.PHONY: clean
clean:
	rm -rf $(BDIRS) $(LIBRARY)
	$(MAKE) -C exec/demo clean
	$(MAKE) -C exec/tester clean
