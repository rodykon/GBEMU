"""
Debug GBEMU programs based on log.

Usage:
    python3 debug.py [log_file]
"""
import sys
import re
from argparse import ArgumentParser, FileType
from dataclasses import dataclass
from typing import List


INSTRUCTION_REGEX = "DEBUG: ([^A].*)"
REGS_REGEX = "DEBUG: AF=(?P<af>[0-9a-f]{4}), BC=(?P<bc>[0-9a-f]{4}), DE=(?P<de>[0-9a-f]{4}), HL=(?P<hl>[0-9a-f]{4}), SP=(?P<sp>[0-9a-f]{4}), PC=(?P<pc>[0-9a-f]{4})"
ERROR_REGEX = "ERROR: (.*)"
REGS = ["af", "bc", "de", "hl", "sp", "pc"]


PRINT_AHEAD = 5


PROMPT = "(gbdb) "
COMMAND_HANDLER_MAP = {} # Maps command names to their handler functions.


class ParsingError(Exception):
    def __init__(self, line):
        super().__init__("Unable to parse line: '{}'".format(line))


class DebugError(Exception):
    pass


class InvalidArgumentsError(DebugError):
    def __init__(self):
        super().__init__("Invalid arguments.")


class RuntimeError(DebugError):
    def __init__(self, msg):
        super().__init__("Program ended with error: {}".format(msg))


@dataclass
class Registers:
    af: int
    bc: int
    de: int
    hl: int
    pc: int
    sp: int

    def __getitem__(self, key):
        return self.__getattribute__(key)


class Instruction:
    def __init__(self, regs: Registers, line: str, error: str):
        self.regs = regs
        self.line = line
        self.error = error

    def __str__(self):
        return self.line


class Debugger:
    def __init__(self, instructions: List[Instruction]):
        self.instructions = instructions
        self.current = 0
        self.breakpoints = []
        self.brk = False
        self.running = True

    @staticmethod
    def __get_command() -> List[str]:
        cmd = input(PROMPT).split(' ')
        while cmd[0] not in COMMAND_HANDLER_MAP:
            print("Invalid command.")
            cmd = input(PROMPT).split(' ')
        return cmd

    def __next_command(self):
        cmd = self.__get_command()
        try:
            return COMMAND_HANDLER_MAP[cmd[0]](self, cmd)
        except DebugError as e:
            print(e)

    def __break(self):
        pc = self.instructions[self.current].regs.pc
        print("Break at address 0x{:04X}:".format(pc))
        for i in range(min(PRINT_AHEAD, len(self.instructions) - self.current)):
            print("\t{}".format(self.instructions[self.current + i]))
        while not self.__next_command():
            pass

    def __error(self):
        pc = self.instructions[self.current].regs.pc
        print("ERROR! at address: 0x{:04X}:".format(pc))
        print(self.instructions[self.current].error)
        self.__next_command()

        
    def run(self):
        self.__break()
        self.current += 1
        while self.running and self.current < len(self.instructions):
            if self.instructions[self.current].error:
                raise RuntimeError(self.instructions[self.current].error)
            if self.instructions[self.current].regs.pc in self.breakpoints or self.brk:
                self.brk = False
                self.__break()
            self.current += 1
        print("End of program.")


def filter_logfile(logfile: str) -> List[Instruction]:
    return '\n'.join([row for row in logfile.splitlines() if row.find("DEBUG: ") or row.find("ERROR: ")])


def parse_instruction(lines: List[str]) -> List[Instruction]:
    regs_match = re.search(REGS_REGEX, lines[0])
    if not regs_match:
        raise ParsingError(lines[0])
    
    mnemonic_match = re.search(INSTRUCTION_REGEX, lines[1])
    if not mnemonic_match:
        raise ParsingError(lines[1])
    
    error_match = re.search(ERROR_REGEX, lines[2])
    if not error_match:
        error = None
    else:
        error = error_match.group(1)
    
    mnemonic = mnemonic_match.group(1)
    regs = Registers(**{reg: int(regs_match.group(reg), 16) for reg in REGS})
    return Instruction(regs, mnemonic, error)


def parse_logfile(logfile: str) -> List[Instruction]:
    result = []
    filtered = filter_logfile(logfile).splitlines()
    
    for i in range(0, len(filtered), 2):
        result.append(parse_instruction(filtered[i:]))
        if result[-1].error:
            break
    return result

    
#  ---------- Debugger Commands ----------

def command_handler(func):
    COMMAND_HANDLER_MAP[func.__name__] = func
    return func


@command_handler
def help(dbg: Debugger, args: List[str]):
    """
    Print this help message.
    """
    for name, func in COMMAND_HANDLER_MAP.items():
        print("{}: {}".format(name, func.__doc__))
    return False


@command_handler
def c(dbg: Debugger, args: List[str]):
    """
    Continue program execution.
    """
    return True


@command_handler
def b(dbg: Debugger, args: List[str]):
    """
    Set a breakpoint at a given address.
    """
    if len(args) != 2:
        raise InvalidArgumentsError()
    
    try:
        dbg.breakpoints.append(int(args[1], 16))
    except ValueError:
        raise InvalidArgumentsError
    return False


@command_handler
def d(dbg: Debugger, args: List[str]):
    """
    Delete a breakpoint.
    """
    if len(args) != 2:
        raise InvalidArgumentsError()

    try:
        del dbg.breakpoints[int(args[1])]
    except (ValueError, IndexError):
        raise InvalidArgumentsError
    return False


@command_handler
def l(dbg: Debugger, args: List[str]):
    """
    List breakpoints.
    """
    if len(args) != 1:
        raise InvalidArgumentsError()

    for index, value in enumerate(dbg.breakpoints):
        print("{}: {:04X}".format(index, value))
    return False


@command_handler
def ni(dbg: Debugger, args: List[str]):
    """
    Set breakpoint to next instruction and continue.
    """
    if len(args) != 1:
        raise InvalidArgumentsError()
    
    dbg.brk = True
    return True


@command_handler
def r(dbg: Debugger, args: List[str]):
    """
    Print the value of a register.
    """
    if len(args) != 2 or args[1] not in REGS:
        raise InvalidArgumentsError()
    
    print("{} = 0x{:04X}".format(args[1], dbg.instructions[dbg.current].regs[args[1]]))
    return False

@command_handler
def quit(dbg: Debugger, args: List[str]):
    """
    Exit the debugger.
    """
    dbg.running = False
    return True

@command_handler
def asm(dbg:Debugger, args: List[str]):
    """
    Disassemble n instructions starting with the current one (default n is 4).
    """
    if len(args) > 2:
        raise InvalidArgumentsError()
    num_lines = PRINT_AHEAD
    if len(args) > 1:
        num_lines = int(args[1])
    
    for instruction in dbg.instructions[dbg.current:min(len(dbg.instructions), dbg.current + num_lines)]:
        print("\t{}".format(instruction))
    return False



def parse_args():
    parser = ArgumentParser("Debug GBEMU programs based on log.")
    parser.add_argument("log_file", type=FileType("r"), default=sys.stdin, nargs='?', help="Log file.")
    return parser.parse_args()


def main():
    args = parse_args()
    instructions = parse_logfile(args.log_file.read())
    dbg = Debugger(instructions)

    dbg.run()


if __name__ == "__main__":
    main()