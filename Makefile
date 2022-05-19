DESTDIR = /usr/local

CXX = g++
CXXFLAGS = -g
CXXLIBS = 

PWD = $(shell pwd)
INCLUDE = -I$(PWD)/common

PROJECT = JD1304
EXECUTABLE = $(PROJECT)
EXECUTABLE_ARGS = examples/prg.bin step
EXECUTABLE2_ARGS = examples/test.asm output.bin

MISC = RAM.bin \
	*.dump

OBJECTS = emu/JD1304.o \
	asm/jdasm.o \
	main.o

run: $(EXECUTABLE)
	./$(EXECUTABLE) $(EXECUTABLE_ARGS)

JDASM: $(EXECUTABLE)
	cp $(EXECUTABLE) JDASM

run_JDASM: JDASM
	./JDASM $(EXECUTABLE2_ARGS)

install:
	install -d ${DESTDIR}/usr/bin
	install -d ${DESTDIR}/usr/share/doc/$(EXECUTABLE)
	install -m 755 $(EXECUTABLE) ${DESTDIR}/usr/bin
	install -m 644 README ${DESTDIR}/usr/share/doc/$(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ $(OBJECTS) $(CXXLIBS)

%.o: %.cpp
	@echo "Compiling $<"
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE) JDASM $(MISC)