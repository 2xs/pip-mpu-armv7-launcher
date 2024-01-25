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


"""padding script"""


import pathlib, sys


# The default value used for padding. This value corresponds to the
# default state of non-volatile NAND flash memories.
PADDING_VALUE = b'\xff'


# The Pip binary size must be a multiple of this value. It corresponds
# to the minimum alignment required by the MPU of the ARMv7-M
# architecture.
MPU_ALIGNMENT = 32


def usage():
    """Print how to to use this script and exit"""
    print(f'usage: {sys.argv[0]} BINARY PADDING')
    sys.exit(1)


def round(x, y):
    """Round x to the next power of two y"""
    return ((x + y - 1) & ~(y - 1))


if __name__ == '__main__':
    if len(sys.argv) >= 3:
        size = pathlib.Path(sys.argv[1]).stat().st_size
        padding = round(size, MPU_ALIGNMENT) - size
        with open(sys.argv[2], 'wb') as f:
            f.write(padding * PADDING_VALUE)
        sys.exit(0)
    usage()
