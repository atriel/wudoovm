CXXFLAGS=-std=c++11 -Wall -pedantic -Wfatal-errors

VM_ASM=bin/vm/asm
VM_CPU=bin/vm/cpu

WUDOO_CPU_INSTR_FILES_CPP=src/cpu/instr/general.cpp src/cpu/instr/int.cpp src/cpu/instr/byte.cpp src/cpu/instr/bool.cpp
WUDOO_CPU_INSTR_FILES_O=build/cpu/instr/general.o build/cpu/instr/int.o build/cpu/instr/byte.o build/cpu/instr/bool.o

BIN_PATH=/usr/local/bin


.SUFFIXES: .cpp .h .o

.PHONY: all install test


all: ${VM_ASM} ${VM_CPU} bin/opcodes.bin


clean: clean-support
	rm -v ./build/cpu/instr/*.o
	rm -v ./build/cpu/*.o
	rm -v ./build/*.o
	rm -v ./bin/vm/*

clean-support:
	rm -v ./build/support/*.o

clean-test-compiles:
	rm -v ./tests/compiled/*.bin

install: ${VM_ASM} ${VM_CPU}
	mkdir -p ${BIN_PATH}
	cp ${VM_ASM} ${BIN_PATH}/wudoo-asm
	chmod 755 ${BIN_PATH}/wudoo-asm
	cp ${VM_CPU} ${BIN_PATH}/wudoo-run
	chmod 755 ${BIN_PATH}/wudoo-run


test: ${VM_CPU} ${VM_ASM}
	python3 ./tests/tests.py --verbose --catch --failfast


${VM_CPU}: src/bytecode.h src/front/cpu.cpp build/cpu/cpu.o build/support/pointer.o build/support/string.o ${WUDOO_CPU_INSTR_FILES_O}
	${CXX} ${CXXFLAGS} -o ${VM_CPU} src/front/cpu.cpp build/cpu/cpu.o build/support/pointer.o build/support/string.o ${WUDOO_CPU_INSTR_FILES_O}

${VM_ASM}: src/bytecode.h src/front/asm.cpp build/program.o build/support/string.o
	${CXX} ${CXXFLAGS} -o ${VM_ASM} src/front/asm.cpp build/program.o build/support/string.o


bin/opcodes.bin: src/bytecode/opcodes.h src/bytecode/maps.h src/bytecode/opcd.cpp
	${CXX} ${CXXFLAGS} -o bin/opcodes.bin src/bytecode/opcd.cpp


build/cpu/cpu.o: src/bytecode.h src/cpu/cpu.h src/cpu/cpu.cpp
	${CXX} ${CXXFLAGS} -c -o $@ ./src/cpu/cpu.cpp

build/cpu/instr/general.o: src/cpu/instr/general.cpp
	${CXX} ${CXXFLAGS} -c -o $@ ./src/cpu/instr/general.cpp

build/cpu/instr/int.o: src/cpu/instr/int.cpp
	${CXX} ${CXXFLAGS} -c -o $@ ./src/cpu/instr/int.cpp

build/cpu/instr/byte.o: src/cpu/instr/byte.cpp
	${CXX} ${CXXFLAGS} -c -o $@ ./src/cpu/instr/byte.cpp

build/cpu/instr/bool.o: src/cpu/instr/bool.cpp
	${CXX} ${CXXFLAGS} -c -o $@ ./src/cpu/instr/bool.cpp


build/program.o: src/program.cpp
	${CXX} ${CXXFLAGS} -c -o $@ ./src/program.cpp


build/support/string.o: src/support/string.cpp
	${CXX} ${CXXFLAGS} -c -o $@ ./src/support/string.cpp

build/support/pointer.o: src/support/pointer.cpp
	${CXX} ${CXXFLAGS} -c -o $@ ./src/support/pointer.cpp
