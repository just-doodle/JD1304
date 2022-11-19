DESTDIR = /usr/local

CXX = g++
CXXFLAGS = -g
CXXLIBS = -lX11 -lGL -lpthread -lpng -lstdc++fs -std=c++17

PWD = $(shell pwd)
INCLUDE = -I$(PWD)/common

PROJECT = JD1304
EXECUTABLE = $(PROJECT)
EXECUTABLE_ARGS = examples/prg.bin step
EXECUTABLE2_ARGS = examples/test.jds output.bin

MISC = RAM.bin \
	*.dump

OBJECTS = emu/monitor.o \
	emu/JD1304.o \
	asm/jdasm.o \
	main.o

run: $(EXECUTABLE)
	./$(EXECUTABLE) $(EXECUTABLE_ARGS)

JDASM: $(EXECUTABLE)
	cp $(EXECUTABLE) JDASM

run_JDASM: JDASM
	./JDASM $(EXECUTABLE2_ARGS)
	./JD1304 output.bin step

install:
	install -d ${DESTDIR}/bin
	install -d ${DESTDIR}/share/doc/$(EXECUTABLE)
	install -m 755 $(EXECUTABLE) ${DESTDIR}/bin
	install -m 755 JDASM ${DESTDIR}/bin
	install -m 644 Readme.md ${DESTDIR}/share/doc/$(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ $(OBJECTS) $(CXXLIBS)

%.o: %.cpp
	@echo "Compiling $<"
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE) JDASM $(MISC)