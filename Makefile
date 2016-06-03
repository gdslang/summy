CC=gcc
#CC=clang
CPP=clang++
INCDS=-Iinclude
O=-O2
CFLAGS=$(CFLAGS_EXTRA) -c -MMD -ggdb3 $(O) -std=c11 -Wall -Wfatal-errors -DRELAXEDFATAL $(INCDS)
CPPFLAGS=$(CPPFLAGS_EXTRA) -c -MMD -ggdb3 $(O) -std=c++14 -Wall -Wno-tautological-undefined-compare -Wno-overloaded-virtual -Wno-deprecated $(INCDS)

LIBRARY=libsummy.a

SPRE=src
HPRE=include
BPRE=build

CSOURCES=$(shell find $(SPRE)/ -type f -name '*.c' | grep -v ismt)
CPPSOURCES=$(shell find $(SPRE)/ -type f -name '*.cpp' | grep -v ismt)

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

.PHONY: demo_ismt
demo_ismt: all
	$(MAKE) -C exec/demo_ismt/
.PHONY: demo_dstack
demo_dstack: all
	$(MAKE) -C exec/demo_dstack/
.PHONY: all_f
all_f: all
	$(MAKE) -C exec/all_f/
.PHONY: test_build
test_build: all
	$(MAKE) -C exec/tester/
.PHONY: test
test: test_build
	exec/tester/summy-tester

.PHONY: clean
clean:
	rm -rf $(BDIRS) $(LIBRARY)
	$(MAKE) -C exec/demo_ismt clean
	$(MAKE) -C exec/demo_dstack clean
	$(MAKE) -C exec/tester clean
	$(MAKE) -C exec/all_f clean
