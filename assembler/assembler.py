""" An assembler and disassembler for CHIP-8 programs. """

import os
import re
from typing import Dict, Pattern

opcode_structure = {
    '00E0': 'CLS',
    '00EE': 'RET',
    '1nnn': 'JP nnn',
    '2nnn': 'CALL nnn',
    '3xkk': 'SE Vx, kk',
    '4xkk': 'SNE Vx, kk',
    '5xy.': 'SE Vx, Vy',
    '6xkk': 'LD Vx, kk',
    '7xkk': 'ADD Vx, kk',
    '8xy0': 'LD Vx, Vy',
    '8xy1': 'OR Vx, Vy',
    '8xy2': 'AND Vx, Vy',
    '8xy3': 'XOR Vx, Vy',
    '8xy4': 'ADD Vx, Vy',
    '8xy5': 'SUB Vx, Vy',
    '8x.6': 'SHR Vx',
    '8xy7': 'SUBN Vx, Vy',
    '8x.E': 'SHL Vx',
    '9xy.': 'SNE Vx, Vy',
    'Annn': 'LD I, nnn',
    'Bnnn': 'JP V0, nnn',
    'Cxkk': 'RND Vx, kk',
    'Dxyn': 'DRW Vx, Vy, n',
    'Ex9E': 'SKP Vx',
    'ExA1': 'SKNP Vx',
    'Fx07': 'LD Vx, DT',
    'Fx0A': 'LD Vx, K',
    'Fx15': 'LD DT, Vx',
    'Fx18': 'LD ST, Vx',
    'Fx1E': 'ADD I, Vx',
    'Fx29': 'LD F, Vx',
    'Fx33': 'LD B, Vx',
    'Fx55': 'LD I, Vx',
    'Fx65': 'LD Vx, I',
}

hex_pattern = '[0-9A-F]'

token_patterns = {
    'nnn': hex_pattern + '{3}',
    'x': hex_pattern,
    'y': hex_pattern,
    'kk': hex_pattern + '{2}',
    'n': hex_pattern,
}

# compiled lookup of opcodes using regex
assemble_table: Dict[Pattern, str] = {}
disassemble_table: Dict[Pattern, str] = {}


def compile_tables():
    for code, instruction in opcode_structure.items():
        assemble_code = disassemble_code = code
        assemble_instruction = disassemble_instruction = instruction

        group = 0  # the regex group to replace with
        chars_remain = 3  # start at 3 possible replacements

        for token, pattern in token_patterns.items():
            if token in code:
                assemble_instruction = assemble_instruction.replace(token, f'({pattern})').replace(' ', r'\s+')
                assemble_code = assemble_code.replace(token, '{groups[' + str(group) + ']}')

                disassemble_code = disassemble_code.replace(token, f'({pattern})')
                disassemble_instruction = disassemble_instruction.replace(token, '{groups[' + str(group) + ']}')

                group += 1
                chars_remain -= len(token)

            if chars_remain <= 0:
                break

        assemble_table[re.compile(assemble_instruction)] = assemble_code
        disassemble_table[re.compile(disassemble_code)] = disassemble_instruction


def disassemble(file_path: str):
    """ Disassemble a ch8 file, saving a <filename>.ch8asm file. """
    # read every opcode and parse them as strings
    opcodes = []
    with open(file_path, 'rb') as f:
        while True:
            code = f.read(2).hex()
            if not code:
                break

            opcodes.append(code)

    # parse code to substitute
    instructions = []
    for opcode in opcodes:
        print(f'trying: {opcode}')
        for code, instruction in disassemble_table.items():
            match = code.match(opcode.upper())

            if match:
                print(f'matched: {code.pattern}, {instruction}')
                instructions.append(instruction.format(groups=match.groups()))
                break
        else:
            instructions.append(opcode.upper())

    file_name = os.path.splitext(os.path.basename(file_path))[0]
    with open(file_name + '.ch8asm', 'w') as f:
        f.write('\n'.join(instructions))


def main():
    disassemble(r'C:\Users\pc\CLionProjects\chip8ler\roms\demos\Maze (alt) [David Winter, 199x].ch8')
    disassemble(r'C:\Users\pc\CLionProjects\chip8ler\roms\demos\Maze [David Winter, 199x].ch8')


# compile tables for use with
compile_tables()

if __name__ == '__main__':
    main()
