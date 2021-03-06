#!/usr/bin/env python3

"""This is initial unit Tests suite for Wudoo virtual machine.
It uses sample asm code (samples can also be compiled and run directly).

Each unit passes if:

    * the sample compiles,
    * compiled code runs,
    * compiled code returns correct output,

Returning correct may mean raising an exception in some cases.
"""

import os
import subprocess
import sys
import unittest


COMPILED_SAMPLES_PATH = './tests/compiled'


class WudooError(Exception):
    """Generic Wudoo exception.
    """
    pass

class WudooAssemblerError(WudooError):
    """Base class for exceptions related to Wudoo assembler.
    """
    pass

class WudooCPUError(WudooError):
    """Base class for exceptions related to Wudoo CPU.
    """
    pass


def assemble(asm, out):
    """Assemble path given as `asm` and put binary in `out`.
    Raises exception if compilation is not successful.
    """
    p = subprocess.Popen(('./bin/vm/asm', asm, out), stdout=subprocess.PIPE)
    output, error = p.communicate()
    exit_code = p.wait()
    if exit_code != 0:
        raise WudooAssemblerError('{0}: {1}'.format(asm, output.decode('utf-8').strip()))

def run(path, expected_exit_code=0):
    """Run given file with Wudoo CPU and return its output.
    """
    p = subprocess.Popen(('./bin/vm/cpu', path), stdout=subprocess.PIPE)
    output, error = p.communicate()
    exit_code = p.wait()
    if exit_code not in (expected_exit_code if type(expected_exit_code) in [list, tuple] else (expected_exit_code,)):
        raise WudooCPUError('{0}: {1}'.format(path, output.decode('utf-8').strip()))
    return (exit_code, output.decode('utf-8'))


class IntegerInstructionsTests(unittest.TestCase):
    """Tests for integer instructions.
    """
    PATH = './sample/asm/int'

    def testIADD(self):
        name = 'add.asm'
        assembly_path = os.path.join(IntegerInstructionsTests.PATH, name)
        compiled_path = os.path.join(COMPILED_SAMPLES_PATH, (name + '.bin'))
        assemble(assembly_path, compiled_path)
        excode, output = run(compiled_path)
        self.assertEqual('1', output.strip())
        self.assertEqual(0, excode)

    def testISUB(self):
        name = 'sub.asm'
        assembly_path = os.path.join(IntegerInstructionsTests.PATH, name)
        compiled_path = os.path.join(COMPILED_SAMPLES_PATH, (name + '.bin'))
        assemble(assembly_path, compiled_path)
        excode, output = run(compiled_path)
        self.assertEqual('1', output.strip())
        self.assertEqual(0, excode)

    def testIMUL(self):
        name = 'mul.asm'
        assembly_path = os.path.join(IntegerInstructionsTests.PATH, name)
        compiled_path = os.path.join(COMPILED_SAMPLES_PATH, (name + '.bin'))
        assemble(assembly_path, compiled_path)
        excode, output = run(compiled_path)
        self.assertEqual('1', output.strip())
        self.assertEqual(0, excode)

    def testIDIV(self):
        name = 'div.asm'
        assembly_path = os.path.join(IntegerInstructionsTests.PATH, name)
        compiled_path = os.path.join(COMPILED_SAMPLES_PATH, (name + '.bin'))
        assemble(assembly_path, compiled_path)
        excode, output = run(compiled_path)
        self.assertEqual('1', output.strip())
        self.assertEqual(0, excode)

    def testIDEC(self):
        name = 'dec.asm'
        assembly_path = os.path.join(IntegerInstructionsTests.PATH, name)
        compiled_path = os.path.join(COMPILED_SAMPLES_PATH, (name + '.bin'))
        assemble(assembly_path, compiled_path)
        excode, output = run(compiled_path)
        self.assertEqual('1', output.strip())
        self.assertEqual(0, excode)

    def testIINC(self):
        name = 'inc.asm'
        assembly_path = os.path.join(IntegerInstructionsTests.PATH, name)
        compiled_path = os.path.join(COMPILED_SAMPLES_PATH, (name + '.bin'))
        assemble(assembly_path, compiled_path)
        excode, output = run(compiled_path)
        self.assertEqual('1', output.strip())
        self.assertEqual(0, excode)

    def testILT(self):
        name = 'lt.asm'
        assembly_path = os.path.join(IntegerInstructionsTests.PATH, name)
        compiled_path = os.path.join(COMPILED_SAMPLES_PATH, (name + '.bin'))
        assemble(assembly_path, compiled_path)
        excode, output = run(compiled_path)
        self.assertEqual('true', output.strip())
        self.assertEqual(0, excode)

    def testILTE(self):
        name = 'lte.asm'
        assembly_path = os.path.join(IntegerInstructionsTests.PATH, name)
        compiled_path = os.path.join(COMPILED_SAMPLES_PATH, (name + '.bin'))
        assemble(assembly_path, compiled_path)
        excode, output = run(compiled_path)
        self.assertEqual('true', output.strip())
        self.assertEqual(0, excode)

    def testIGT(self):
        name = 'gt.asm'
        assembly_path = os.path.join(IntegerInstructionsTests.PATH, name)
        compiled_path = os.path.join(COMPILED_SAMPLES_PATH, (name + '.bin'))
        assemble(assembly_path, compiled_path)
        excode, output = run(compiled_path)
        self.assertEqual('true', output.strip())
        self.assertEqual(0, excode)

    def testIGTE(self):
        name = 'gte.asm'
        assembly_path = os.path.join(IntegerInstructionsTests.PATH, name)
        compiled_path = os.path.join(COMPILED_SAMPLES_PATH, (name + '.bin'))
        assemble(assembly_path, compiled_path)
        excode, output = run(compiled_path)
        self.assertEqual('true', output.strip())
        self.assertEqual(0, excode)

    def testIEQ(self):
        name = 'eq.asm'
        assembly_path = os.path.join(IntegerInstructionsTests.PATH, name)
        compiled_path = os.path.join(COMPILED_SAMPLES_PATH, (name + '.bin'))
        assemble(assembly_path, compiled_path)
        excode, output = run(compiled_path)
        self.assertEqual('true', output.strip())
        self.assertEqual(0, excode)

    def testCalculatingModulo(self):
        name = 'modulo.asm'
        assembly_path = os.path.join(IntegerInstructionsTests.PATH, name)
        compiled_path = os.path.join(COMPILED_SAMPLES_PATH, (name + '.bin'))
        assemble(assembly_path, compiled_path)
        excode, output = run(compiled_path)
        self.assertEqual('65', output.strip())
        self.assertEqual(0, excode)


class ByteInstructionsTests(unittest.TestCase):
    """Tests for byte instructions.
    """
    PATH = './sample/asm/byte'

    def testHelloWorld(self):
        name = 'helloworld.asm'
        assembly_path = os.path.join(ByteInstructionsTests.PATH, name)
        compiled_path = os.path.join(COMPILED_SAMPLES_PATH, (name + '.bin'))
        assemble(assembly_path, compiled_path)
        excode, output = run(compiled_path)
        self.assertEqual('Hello World!', output.strip())
        self.assertEqual(0, excode)


class RegisterManipulationInstructionsTests(unittest.TestCase):
    """Tests for register-manipulation instructions.
    """
    PATH = './sample/asm/regmod'

    def testCOPY(self):
        name = 'copy.asm'
        assembly_path = os.path.join(RegisterManipulationInstructionsTests.PATH, name)
        compiled_path = os.path.join(COMPILED_SAMPLES_PATH, (name + '.bin'))
        assemble(assembly_path, compiled_path)
        excode, output = run(compiled_path)
        self.assertEqual('1', output.strip())
        self.assertEqual(0, excode)

    def testMOVE(self):
        name = 'move.asm'
        assembly_path = os.path.join(RegisterManipulationInstructionsTests.PATH, name)
        compiled_path = os.path.join(COMPILED_SAMPLES_PATH, (name + '.bin'))
        assemble(assembly_path, compiled_path)
        excode, output = run(compiled_path)
        self.assertEqual('1', output.strip())
        self.assertEqual(0, excode)

    def testSWAP(self):
        name = 'swap.asm'
        assembly_path = os.path.join(RegisterManipulationInstructionsTests.PATH, name)
        compiled_path = os.path.join(COMPILED_SAMPLES_PATH, (name + '.bin'))
        assemble(assembly_path, compiled_path)
        excode, output = run(compiled_path)
        self.assertEqual([1, 0], [int(i) for i in output.strip().splitlines()])
        self.assertEqual(0, excode)

    def testRET(self):
        name = 'ret.asm'
        assembly_path = os.path.join(RegisterManipulationInstructionsTests.PATH, name)
        compiled_path = os.path.join(COMPILED_SAMPLES_PATH, (name + '.bin'))
        assemble(assembly_path, compiled_path)
        excode, output = run(compiled_path, (4,))
        self.assertEqual('', output.strip())
        self.assertEqual(4, excode)


class SampleProgramsTests(unittest.TestCase):
    """Tests for various sample programs.

    These tests (as well as samples) use several types of instructions and/or
    assembler directives and as such are more stressing for both assembler and CPU.
    """
    PATH = './sample/asm'

    def testCalculatingIntegerPowerOf(self):
        name = 'power_of.asm'
        assembly_path = os.path.join(SampleProgramsTests.PATH, name)
        compiled_path = os.path.join(COMPILED_SAMPLES_PATH, (name + '.bin'))
        assemble(assembly_path, compiled_path)
        excode, output = run(compiled_path)
        self.assertEqual('64', output.strip())
        self.assertEqual(0, excode)

    def testCalculatingAbsoluteValueOfAnInteger(self):
        name = 'abs.asm'
        assembly_path = os.path.join(SampleProgramsTests.PATH, name)
        compiled_path = os.path.join(COMPILED_SAMPLES_PATH, (name + '.bin'))
        assemble(assembly_path, compiled_path)
        excode, output = run(compiled_path)
        self.assertEqual('17', output.strip())
        self.assertEqual(0, excode)

    def testLooping(self):
        name = 'looping.asm'
        assembly_path = os.path.join(SampleProgramsTests.PATH, name)
        compiled_path = os.path.join(COMPILED_SAMPLES_PATH, (name + '.bin'))
        assemble(assembly_path, compiled_path)
        excode, output = run(compiled_path)
        self.assertEqual([i for i in range(0, 11)], [int(i) for i in output.strip().splitlines()])
        self.assertEqual(0, excode)

    def testReferences(self):
        name = 'refs.asm'
        assembly_path = os.path.join(SampleProgramsTests.PATH, name)
        compiled_path = os.path.join(COMPILED_SAMPLES_PATH, (name + '.bin'))
        assemble(assembly_path, compiled_path)
        excode, output = run(compiled_path)
        self.assertEqual([2, 16], [int(i) for i in output.strip().splitlines()])
        self.assertEqual(0, excode)

    def testRegisterReferencesInIntegerOperands(self):
        name = 'registerref.asm'
        assembly_path = os.path.join(SampleProgramsTests.PATH, name)
        compiled_path = os.path.join(COMPILED_SAMPLES_PATH, (name + '.bin'))
        assemble(assembly_path, compiled_path)
        excode, output = run(compiled_path)
        self.assertEqual([16, 1, 1, 16], [int(i) for i in output.strip().splitlines()])
        self.assertEqual(0, excode)


if __name__ == '__main__':
    unittest.main()
