#include <iostream>
#include <vector>
#include "../bytecode/bytetypedef.h"
#include "../bytecode/opcodes.h"
#include "../bytecode/maps.h"
#include "../types/object.h"
#include "../types/integer.h"
#include "../types/byte.h"
#include "../support/pointer.h"
#include "cpu.h"
using namespace std;


CPU& CPU::load(byte* bc) {
    /*  Load bytecode into the CPU.
     *  CPU becomes owner of loaded bytecode - meaning it will consider itself responsible for proper
     *  destruction of it, so make sure you have a copy of the bytecode.
     *
     *  Any previously loaded bytecode is freed.
     *  To free bytecode without loading anything new it is possible to call .load(0).
     *
     *  :params:
     *
     *  bc:char*    - pointer to byte array containing bytecode with a program to run
     */
    if (bytecode) { delete[] bytecode; }
    bytecode = bc;
    return (*this);
}

CPU& CPU::bytes(uint16_t sz) {
    /*  Set bytecode size, so the CPU can stop execution even if it doesn't reach HALT instruction but reaches
     *  bytecode address out of bounds.
     */
    bytecode_size = sz;
    return (*this);
}

CPU& CPU::eoffset(uint16_t o) {
    /*  Set offset of first executable instruction.
     */
    executable_offset = o;
    return (*this);
}


Object* CPU::fetch(int index) {
    /*  Return pointer to object at given register.
     *  This method safeguards against reaching for out-of-bounds registers and
     *  reading from an empty register.
     *
     *  :params:
     *
     *  index:int   - index of a register to fetch
     */
    if (index >= reg_count) { throw "register access out of bounds: read"; }
    Object* optr = registers[index];
    if (optr == 0) {
        ostringstream oss;
        oss << "read from null register: " << index;
        throw oss.str().c_str();
    }
    return optr;
}


template<class T> inline void copyvalue(Object* a, Object* b) {
    /** This is a short inline, template function to copy value between two `Object` pointers of the same polymorphic type.
     *  It is used internally by CPU.
     */
    static_cast<T>(a)->value() = static_cast<T>(b)->value();
}

void CPU::updaterefs(Object* before, Object* now) {
    /** This method updates references to a given address present in registers.
     *  It swaps old address for the new one in every register that points to the old address and
     *  is marked as a reference.
     */
    for (int i = 0; i < reg_count; ++i) {
        if (registers[i] == before and references[i]) {
            if (debug) {
                cout << "CPU: updating reference address in register " << i << hex << ": 0x" << (unsigned long)before << " -> 0x" << (unsigned long)now << dec << endl;
            }
            registers[i] = now;
        }
    }
}

bool CPU::hasrefs(int index) {
    /** This method checks if object at a given address exists as a reference in another register.
     */
    bool has = false;
    for (int i = 0; i < reg_count; ++i) {
        if (i == index) continue;
        if (registers[i] == registers[index]) {
            has = true;
            break;
        }
    }
    return has;
}

void CPU::place(int index, Object* obj) {
    /** Place an object in register with given index.
     *
     *  Before placing an object in register, a check is preformed if the register is empty.
     *  If not - the `Object` previously stored in it is destroyed.
     *
     */
    if (index >= reg_count) { throw "register access out of bounds: write"; }
    if (registers[index] != 0 and !references[index]) {
        // register is not empty and is not a reference - the object in it must be destroyed to avoid memory leaks
        delete registers[index];
    }
    if (references[index]) {
        Object* referenced = fetch(index);

        // it is a reference, copy value of the object
        if (referenced->type() == "Integer") { copyvalue<Integer*>(referenced, obj); }
        else if (referenced->type() == "Byte") { copyvalue<Byte*>(referenced, obj); }

        // and delete the newly created object to avoid leaks
        delete obj;
    } else {
        Object* old_ref_ptr = (hasrefs(index) ? registers[index] : 0);
        registers[index] = obj;
        if (old_ref_ptr) { updaterefs(old_ref_ptr, obj); }
    }
}


int CPU::run() {
    /*  VM CPU implementation.
     *
     *  A giant switch-in-while which iterates over bytecode and executes encoded instructions.
     */
    if (!bytecode) {
        throw "null bytecode (maybe not loaded?)";
    }
    int return_code = 0;

    bool halt = false;

    byte* instr_ptr = bytecode+executable_offset; // instruction pointer

    while (true) {
        if (debug) {
            cout << "CPU: bytecode ";
            cout << dec << ((long)instr_ptr - (long)bytecode);
            cout << " at 0x" << hex << (long)instr_ptr;
            cout << dec << ": ";
        }

        try {
            if (debug) { cout << OP_NAMES.at(OPCODE(*instr_ptr)); }
            switch (*instr_ptr) {
                case ISTORE:
                    instr_ptr = istore(instr_ptr+1);
                    break;
                case IADD:
                    instr_ptr = iadd(instr_ptr+1);
                    break;
                case ISUB:
                    instr_ptr = isub(instr_ptr+1);
                    break;
                case IMUL:
                    instr_ptr = imul(instr_ptr+1);
                    break;
                case IDIV:
                    instr_ptr = idiv(instr_ptr+1);
                    break;
                case IINC:
                    instr_ptr = iinc(instr_ptr+1);
                    break;
                case IDEC:
                    instr_ptr = idec(instr_ptr+1);
                    break;
                case ILT:
                    instr_ptr = ilt(instr_ptr+1);
                    break;
                case ILTE:
                    instr_ptr = ilte(instr_ptr+1);
                    break;
                case IGT:
                    instr_ptr = igt(instr_ptr+1);
                    break;
                case IGTE:
                    instr_ptr = igte(instr_ptr+1);
                    break;
                case IEQ:
                    instr_ptr = ieq(instr_ptr+1);
                    break;
                case BSTORE:
                    instr_ptr = bstore(instr_ptr+1);
                    break;
                case NOT:
                    instr_ptr = lognot(instr_ptr+1);
                    break;
                case AND:
                    instr_ptr = logand(instr_ptr+1);
                    break;
                case OR:
                    instr_ptr = logor(instr_ptr+1);
                    break;
                case MOVE:
                    instr_ptr = move(instr_ptr+1);
                    break;
                case COPY:
                    instr_ptr = copy(instr_ptr+1);
                    break;
                case REF:
                    instr_ptr = ref(instr_ptr+1);
                    break;
                case SWAP:
                    instr_ptr = swap(instr_ptr+1);
                    break;
                case DELETE:
                    instr_ptr = del(instr_ptr+1);
                    break;
                case PRINT:
                    instr_ptr = print(instr_ptr+1);
                    break;
                case ECHO:
                    instr_ptr = echo(instr_ptr+1);
                    break;
                case JUMP:
                    instr_ptr = jump(instr_ptr+1);
                    break;
                case BRANCH:
                    instr_ptr = branch(instr_ptr+1);
                    break;
                case RET:
                    instr_ptr = ret(instr_ptr+1);
                    break;
                case HALT:
                    halt = true;
                    break;
                case PASS:
                    ++instr_ptr;
                    break;
                default:
                    ostringstream error;
                    error << "unrecognised instruction (bytecode value: " << *((int*)bytecode) << ")";
                    throw error.str().c_str();
            }
            if (debug) { cout << endl; }
        } catch (const char* &e) {
            return_code = 1;
            cout << (debug ? "\n" : "") <<  "exception: " << e << endl;
            break;
        }

        if (halt) break;

        if (instr_ptr >= (bytecode+bytecode_size)) {
            cout << "CPU: aborting: bytecode address out of bounds" << endl;
            return_code = 1;
            break;
        }
    }

    if (return_code == 0 and registers[0]) {
        // if return code if the default one and
        // return register is not unused
        // copy value of return register as return code
        return_code = static_cast<Integer*>(registers[0])->value();
    }

    return return_code;
}
