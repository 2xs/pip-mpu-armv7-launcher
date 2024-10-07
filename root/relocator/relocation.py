#!/usr/bin/env python3
###############################################################################
#  © Université de Lille, The Pip Development Team (2015-2024)                #
#                                                                             #
#  This software is a computer program whose purpose is to run a minimal,     #
#  hypervisor relying on proven properties such as memory isolation.          #
#                                                                             #
#  This software is governed by the CeCILL license under French law and       #
#  abiding by the rules of distribution of free software.  You can  use,      #
#  modify and/ or redistribute the software under the terms of the CeCILL     #
#  license as circulated by CEA, CNRS and INRIA at the following URL          #
#  "http://www.cecill.info".                                                  #
#                                                                             #
#  As a counterpart to the access to the source code and  rights to copy,     #
#  modify and redistribute granted by the license, users are provided only    #
#  with a limited warranty  and the software's author,  the holder of the     #
#  economic rights,  and the successive licensors  have only  limited         #
#  liability.                                                                 #
#                                                                             #
#  In this respect, the user's attention is drawn to the risks associated     #
#  with loading,  using,  modifying and/or developing or reproducing the      #
#  software by the user in light of its specific status of free software,     #
#  that may mean  that it is complicated to manipulate,  and  that  also      #
#  therefore means  that it is reserved for developers  and  experienced      #
#  professionals having in-depth computer knowledge. Users are therefore      #
#  encouraged to load and test the software's suitability as regards their    #
#  requirements in conditions enabling the security of their systems and/or   #
#  data to be ensured and,  more generally, to use and operate it in the      #
#  same conditions as regards security.                                       #
#                                                                             #
#  The fact that you are presently reading this means that you have had       #
#  knowledge of the CeCILL license and that you accept its terms.             #
###############################################################################


"""relocation script"""


import sys


from elftools.elf.elffile import ELFFile
from elftools.elf.relocation import RelocationSection
from elftools.elf.enums import ENUM_RELOC_TYPE_ARM as r_types


def usage():
    """Print how to to use the script and exit"""
    print(f'usage: {sys.argv[0]} ELF OUTPUT SECTION...')
    sys.exit(1)


def die(message):
    """Print error message and exit"""
    print(f'\033[91;1m{sys.argv[0]}: {message}\033[0m', file=sys.stderr)
    sys.exit(1)


def to_word(x):
    """Convert a python integer to a LE 4-bytes bytearray"""
    return x.to_bytes(4, byteorder='little')


def get_r_type(r_info):
    """Get the relocation type from r_info"""
    return r_info & 0xff


def process_section(elf, name):
    """Parse a relocation section to extract the r_offset"""
    sh = elf.get_section_by_name(name)
    if not sh:
        return to_word(0)
    if not isinstance(sh, RelocationSection):
        die(f'{name}: is not a relocation section')
    if sh.is_RELA():
        die(f'{name}: unsupported RELA')
    xs = bytearray(to_word(sh.num_relocations()))
    for i, entry in enumerate(sh.iter_relocations()):
        if get_r_type(entry['r_info']) != r_types['R_ARM_ABS32']:
            die(f'{name}: entry {i}: unsupported relocation type')
        xs += to_word(entry['r_offset'])
    return xs


def process_file(elf, names):
    """Process each section"""
    xs = bytearray()
    for name in names:
        xs += process_section(elf, name)
    return xs


if __name__ == '__main__':
    if len(sys.argv) >= 4:
        with open(sys.argv[1], 'rb') as f:
            xs = process_file(ELFFile(f), sys.argv[3:])
        with open(sys.argv[2], 'wb') as f:
            f.write(xs)
        sys.exit(0)
    usage()
