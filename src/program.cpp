#include <iostream>
#include <string>
#include <sstream>
#include "bytecode/bytetypedef.h"
#include "bytecode/opcodes.h"
#include "bytecode/maps.h"
#include "support/pointer.h"
#include "program.h"
using namespace std;


typedef std::tuple<bool, int> int_op;
typedef std::tuple<bool, byte> byte_op;


byte* Program::bytecode() {
    /*  Returns pointer bo a copy of the bytecode.
     *  Each call produces new copy.
     *
     *  Calling code is responsible for dectruction of the allocated memory.
     */
    byte* tmp = new byte[bytes];
    for (int i = 0; i < bytes; ++i) { tmp[i] = program[i]; }
    return tmp;
}


Program& Program::setdebug(bool d) {
    /** Sets debugging status.
     */
    debug = d;
    return (*this);
}


int Program::size() {
    /*  Returns program size in bytes.
     */
    return bytes;
}

int Program::instructionCount() {
    /*  Returns total number of instructions in the program.
     *  Should be called only after the program is constructed as it is calculated by
     *  bytecode analysis.
     */
    int counter = 0;
    for (int i = 0; i < bytes; ++i) {
        switch (program[i]) {
            case IADD:
            case ISUB:
            case IMUL:
            case IDIV:
            case ILT:
            case ILTE:
            case IGT:
            case IGTE:
            case IEQ:
            case BRANCH:
                i += 3 * sizeof(int);
                break;
            case ISTORE:
                i += 2 * sizeof(int);
                break;
            case IINC:
            case IDEC:
            case PRINT:
            case JUMP:
            case RET:
                i += sizeof(int);
                break;
        }
        ++counter;
    }
    return counter;
}


int Program::getInstructionBytecodeOffset(int instr, int count) {
    /** Returns bytecode offset for given instruction index.
     *  The "count" parameter is there to pass assumed instruction count to
     *  avoid recalculating it on every call.
     */

    // check if instruction count was passed, and calculate it if not
    count = (count >= 0 ? count : instructionCount());

    int offset = 0;
    int inc;
    for (int i = 0; i < (instr >= 0 ? instr : count+instr); ++i) {
        /*  This loop iterates over so many instructions as needed to find bytecode offset for requested instruction.
         *
         *  Each time, the offset is increased by `inc` - which is equal to *1 plus size of operands of instructions at current index*.
         */
        string opcode_name = OP_NAMES.at(OPCODE(program[offset]));
        inc = OP_SIZES.at(opcode_name);
        if (debug) {
            cout << "increasing instruction offset (" << i+1 << '/';
            cout << (instr >= 0 ? instr : count+instr) << "): " << opcode_name;
            cout << ": " << inc << endl;
        }
        offset += inc;
        if (offset+1 > bytes) {
            throw "instruction offset out of bounds: check your branches";
        }
    }
    return offset;
}

Program& Program::calculateBranches() {
    /*  This function should be called after program is constructed
     *  to calculate correct bytecode offsets for BRANCH instructions.
     */
    int instruction_count = instructionCount();
    int* ptr;
    for (unsigned i = 0; i < branches.size(); ++i) {
        ptr = (int*)(branches[i]+1);
        switch (*(branches[i])) {
            case JUMP:
                (*ptr) = getInstructionBytecodeOffset(*ptr, instruction_count);
                break;
            case BRANCH:
                pointer::inc<bool, int>(ptr);
                if (debug) { cout << "calculating branch:  true: " << *(ptr+1) << endl; }
                (*(ptr+1)) = getInstructionBytecodeOffset(*(ptr+1), instruction_count);
                if (debug) { cout << "calculated branch:   true: " << *(ptr+1) << endl; }

                if (debug) { cout << "calculating branch: false: " << *(ptr+2) << endl; }
                (*(ptr+2)) = getInstructionBytecodeOffset(*(ptr+2), instruction_count);
                if (debug) { cout << "calculated branch:  false: " << *(ptr+2) << endl; }
                break;
        }
    }

    return (*this);
}


byte* insertIntegerOperand(byte* addr_ptr, int_op op) {
    /** Insert integer operand into bytecode.
     *  When using integer operand, it usually is a plain number - which translates to a regsiter index.
     *  However, when preceded by `@` integer operand will not be interpreted directly, but instead CPU
     *  will look into a register the integer points to, fetch an integer from this register and
     *  use the fetched register as the operand.
     */
    bool ref;
    int num;

    tie(ref, num) = op;

    *((bool*)addr_ptr) = ref;
    pointer::inc<bool, byte>(addr_ptr);
    *((int*)addr_ptr)  = num;
    pointer::inc<int, byte>(addr_ptr);

    return addr_ptr;
}

byte* insertTwoIntegerOpsInstruction(byte* addr_ptr, enum OPCODE instruction, int_op a, int_op b) {
    /** Insert instruction with two integer operands.
     */
    *(addr_ptr++) = instruction;
    addr_ptr = insertIntegerOperand(addr_ptr, a);
    addr_ptr = insertIntegerOperand(addr_ptr, b);
    return addr_ptr;
}

byte* insertThreeIntegerOpsInstruction(byte* addr_ptr, enum OPCODE instruction, int_op a, int_op b, int_op c) {
    /** Insert instruction with two integer operands.
     */
    *(addr_ptr++) = instruction;
    addr_ptr = insertIntegerOperand(addr_ptr, a);
    addr_ptr = insertIntegerOperand(addr_ptr, b);
    addr_ptr = insertIntegerOperand(addr_ptr, c);
    return addr_ptr;
}


Program& Program::istore(int_op regno, int_op i) {
    /*  Inserts istore instruction to bytecode.
     *
     *  :params:
     *
     *  regno:int - register number
     *  i:int     - value to store
     */
    addr_ptr = insertTwoIntegerOpsInstruction(addr_ptr, ISTORE, regno, i);
    return (*this);
}

Program& Program::iadd(int_op rega, int_op regb, int_op regr) {
    /*  Inserts iadd instruction to bytecode.
     *
     *  :params:
     *
     *  rega    - register index of first operand
     *  regb    - register index of second operand
     *  regr    - register index in which to store the result
     */
    addr_ptr = insertThreeIntegerOpsInstruction(addr_ptr, IADD, rega, regb, regr);
    return (*this);
}

Program& Program::isub(int_op rega, int_op regb, int_op regr) {
    /*  Inserts isub instruction to bytecode.
     *
     *  :params:
     *
     *  rega    - register index of first operand
     *  regb    - register index of second operand
     *  regr    - register index in which to store the result
     */
    addr_ptr = insertThreeIntegerOpsInstruction(addr_ptr, ISUB, rega, regb, regr);
    return (*this);
}

Program& Program::imul(int_op rega, int_op regb, int_op regr) {
    /*  Inserts imul instruction to bytecode.
     *
     *  :params:
     *
     *  rega    - register index of first operand
     *  regb    - register index of second operand
     *  regr    - register index in which to store the result
     */
    addr_ptr = insertThreeIntegerOpsInstruction(addr_ptr, IMUL, rega, regb, regr);
    return (*this);
}

Program& Program::idiv(int_op rega, int_op regb, int_op regr) {
    /*  Inserts idiv instruction to bytecode.
     *
     *  :params:
     *
     *  rega    - register index of first operand
     *  regb    - register index of second operand
     *  regr    - register index in which to store the result
     */
    addr_ptr = insertThreeIntegerOpsInstruction(addr_ptr, IDIV, rega, regb, regr);
    return (*this);
}

Program& Program::iinc(int_op regno) {
    /*  Inserts iinc instuction.
     */
    *(addr_ptr++) = IINC;
    addr_ptr = insertIntegerOperand(addr_ptr, regno);
    return (*this);
}

Program& Program::idec(int_op regno) {
    /*  Inserts idec instuction.
     */
    *(addr_ptr++) = IDEC;
    addr_ptr = insertIntegerOperand(addr_ptr, regno);
    return (*this);
}

Program& Program::ilt(int_op rega, int_op regb, int_op regr) {
    /*  Inserts ilt instruction to bytecode.
     *
     *  :params:
     *
     *  rega    - register index of first operand
     *  regb    - register index of second operand
     *  regr    - register index in which to store the result
     */
    addr_ptr = insertThreeIntegerOpsInstruction(addr_ptr, ILT, rega, regb, regr);
    return (*this);
}

Program& Program::ilte(int_op rega, int_op regb, int_op regr) {
    /*  Inserts ilte instruction to bytecode.
     *
     *  :params:
     *
     *  rega    - register index of first operand
     *  regb    - register index of second operand
     *  regr    - register index in which to store the result
     */
    addr_ptr = insertThreeIntegerOpsInstruction(addr_ptr, ILTE, rega, regb, regr);
    return (*this);
}

Program& Program::igt(int_op rega, int_op regb, int_op regr) {
    /*  Inserts igt instruction to bytecode.
     *
     *  :params:
     *
     *  rega    - register index of first operand
     *  regb    - register index of second operand
     *  regr    - register index in which to store the result
     */
    addr_ptr = insertThreeIntegerOpsInstruction(addr_ptr, IGT, rega, regb, regr);
    return (*this);
}

Program& Program::igte(int_op rega, int_op regb, int_op regr) {
    /*  Inserts igte instruction to bytecode.
     *
     *  :params:
     *
     *  rega    - register index of first operand
     *  regb    - register index of second operand
     *  regr    - register index in which to store the result
     */
    addr_ptr = insertThreeIntegerOpsInstruction(addr_ptr, IGTE, rega, regb, regr);
    return (*this);
}

Program& Program::ieq(int_op rega, int_op regb, int_op regr) {
    /*  Inserts ieq instruction to bytecode.
     *
     *  :params:
     *
     *  rega    - register index of first operand
     *  regb    - register index of second operand
     *  regr    - register index in which to store the result
     */
    addr_ptr = insertThreeIntegerOpsInstruction(addr_ptr, IEQ, rega, regb, regr);
    return (*this);
}

Program& Program::bstore(int_op regno, byte_op b) {
    /*  Inserts bstore instruction to bytecode.
     *
     *  :params:
     *
     *  regno - register number
     *  b     - value to store
     */
    bool b_ref = false;
    byte bt;

    tie(b_ref, bt) = b;

    *(addr_ptr++) = BSTORE;
    addr_ptr = insertIntegerOperand(addr_ptr, regno);
    *((bool*)addr_ptr) = b_ref;
    pointer::inc<bool, byte>(addr_ptr);
    *((byte*)addr_ptr)  = bt;
    ++addr_ptr;

    return (*this);
}

Program& Program::lognot(int_op reg) {
    /*  Inserts not instuction.
     */
    *(addr_ptr++) = NOT;
    addr_ptr = insertIntegerOperand(addr_ptr, reg);
    return (*this);
}

Program& Program::logand(int_op rega, int_op regb, int_op regr) {
    /*  Inserts and instruction to bytecode.
     *
     *  :params:
     *
     *  rega   - register index of first operand
     *  regb   - register index of second operand
     *  regr   - register index in which to store the result
     */
    addr_ptr = insertThreeIntegerOpsInstruction(addr_ptr, AND, rega, regb, regr);
    return (*this);
}

Program& Program::logor(int_op rega, int_op regb, int_op regr) {
    /*  Inserts or instruction to bytecode.
     *
     *  :params:
     *
     *  rega   - register index of first operand
     *  regb   - register index of second operand
     *  regr   - register index in which to store the result
     */
    addr_ptr = insertThreeIntegerOpsInstruction(addr_ptr, OR, rega, regb, regr);
    return (*this);
}

Program& Program::move(int_op a, int_op b) {
    /*  Inserts move instruction to bytecode.
     *
     *  :params:
     *
     *  a - register number (move from...)
     *  b - register number (move to...)
     */
    addr_ptr = insertTwoIntegerOpsInstruction(addr_ptr, MOVE, a, b);
    return (*this);
}

Program& Program::copy(int_op a, int_op b) {
    /*  Inserts copy instruction to bytecode.
     *
     *  :params:
     *
     *  a - register number (copy from...)
     *  b - register number (copy to...)
     */
    addr_ptr = insertTwoIntegerOpsInstruction(addr_ptr, COPY, a, b);
    return (*this);
}

Program& Program::ref(int_op a, int_op b) {
    /*  Inserts ref instruction to bytecode.
     *
     *  :params:
     *
     *  a - register number
     *  b - register number
     */
    addr_ptr = insertTwoIntegerOpsInstruction(addr_ptr, REF, a, b);
    return (*this);
}

Program& Program::swap(int_op a, int_op b) {
    /*  Inserts swap instruction to bytecode.
     *
     *  :params:
     *
     *  a - register number
     *  b - register number
     */
    addr_ptr = insertTwoIntegerOpsInstruction(addr_ptr, SWAP, a, b);
    return (*this);
}

Program& Program::print(int_op reg) {
    /*  Inserts print instuction.
     */
    *(addr_ptr++) = PRINT;
    addr_ptr = insertIntegerOperand(addr_ptr, reg);
    return (*this);
}

Program& Program::echo(int_op reg) {
    /*  Inserts echo instuction.
     */
    *(addr_ptr++) = ECHO;
    addr_ptr = insertIntegerOperand(addr_ptr, reg);
    return (*this);
}

Program& Program::jump(int addr) {
    /*  Inserts jump instruction. Parameter is instruction index.
     *  Byte offset is calculated automatically.
     *
     *  :params:
     *
     *  addr:int    - index of the instruction to which to branch
     */
    // save branch instruction index for later evaluation
    branches.push_back(addr_ptr);

    *(addr_ptr++) = JUMP;

    *((int*)addr_ptr) = addr;
    pointer::inc<int, byte>(addr_ptr);

    return (*this);
}

Program& Program::branch(int_op regc, int addr_truth, int addr_false) {
    /*  Inserts branch instruction.
     *  Byte offset is calculated automatically.
     *
     *  :params:
     *
     *  regc:int    - index of the instruction to which to branch
     *  addr_truth:int      - instruction index to go if condition is true
     *  addr_false:int      - instruction index to go if condition is false
     */
    // save branch instruction index for later evaluation
    branches.push_back(addr_ptr);

    *(addr_ptr++) = BRANCH;
    addr_ptr = insertIntegerOperand(addr_ptr, regc);
    *((int*)addr_ptr) = addr_truth;
    pointer::inc<int, byte>(addr_ptr);
    *((int*)addr_ptr) = addr_false;
    pointer::inc<int, byte>(addr_ptr);

    return (*this);
}

Program& Program::ret(int_op reg) {
    /*  Inserts ret instruction. Parameter is instruction index.
     *  Byte offset is calculated automatically.
     *
     *  :params:
     *
     *  reg - index of the register which will be stored as return value
     */
    *(addr_ptr++) = RET;
    addr_ptr = insertIntegerOperand(addr_ptr, reg);
    return (*this);
}

Program& Program::pass() {
    /*  Inserts pass instruction.
     */
    *(addr_ptr++) = PASS;
    return (*this);
}

Program& Program::halt() {
    /*  Inserts halt instruction.
     */
    *(addr_ptr++) = HALT;
    return (*this);
}
