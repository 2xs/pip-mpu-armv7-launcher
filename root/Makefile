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

PREFIX          = arm-none-eabi-
CC              = $(PREFIX)gcc
LD              = $(PREFIX)gcc
OBJCOPY         = $(PREFIX)objcopy

CFLAGS          = -Wall
CFLAGS         += -Wextra
CFLAGS         += -Werror
CFLAGS         += -mthumb
CFLAGS         += -mcpu=cortex-m4
CFLAGS         += -mfloat-abi=hard
CFLAGS         += -mfpu=fpv4-sp-d16
CFLAGS         += -msingle-pic-base
CFLAGS         += -mpic-register=sl
CFLAGS         += -mno-pic-data-is-text-relative
CFLAGS         += -fPIC
CFLAGS         += -ffreestanding
CFLAGS         += -Os
CFLAGS         += -Wno-unused-parameter
CFLAGS         += -Irelocator
CFLAGS         += -I../include

LDFLAGS         = -nostartfiles
LDFLAGS        += -nodefaultlibs
LDFLAGS        += -nolibc
LDFLAGS        += -nostdlib
LDFLAGS        += -Tlink.ld
LDFLAGS        += -Wl,-q
# Disable the new linker warning '--warn-rwx-segments' introduced by
# Binutils 2.39, which causes the following message: "warning:
# $(TARGET).elf has a LOAD segment with RWX permissions".
ifeq ($(shell $(PREFIX)ld --help | grep -q 'warn-rwx-segments'; echo $$?), 0)
LDFLAGS        += -Wl,--no-warn-rwx-segments
endif

OBJCOPYFLAGS    = --input-target=elf32-littlearm
OBJCOPYFLAGS   += --output-target=binary

SYMNAMES        = start
SYMNAMES       += __romSize
SYMNAMES       += __romRamSize
SYMNAMES       += __ramSize
SYMNAMES       += __gotSize
SYMNAMES       += __romRamEnd

RELSECTIONS     = .rel.rom.ram

TARGET          = root

all: $(TARGET).bin

$(TARGET).bin: $(TARGET)-raw.bin padding.bin
	cat $^ > $@

$(TARGET)-raw.bin: crt0.bin symbols.bin relocation.bin partition.bin
	cat $^ > $@

crt0.bin: relocator/crt0.c relocator/crt0.h relocator/link.ld relocator/Makefile
	make -C relocator realclean all
	cp relocator/$@ $@

symbols.bin: $(TARGET).elf relocator/symbols.py
	exec relocator/symbols.py $< $@ $(SYMNAMES)

relocation.bin: $(TARGET).elf relocator/relocation.py
	exec relocator/relocation.py $< $@ $(RELSECTIONS)

partition.bin: $(TARGET).elf
	$(OBJCOPY) $(OBJCOPYFLAGS) $< $@
	@chmod 644 $@

$(TARGET).elf: main.o
	$(LD) $(LDFLAGS) $^ -o $@

main.o: main.c
	$(CC) $(CFLAGS) -c $< -o $@

padding.bin: $(TARGET)-raw.bin relocator/padding.py
	exec relocator/padding.py $< $@

clean:
	$(RM) main.o $(TARGET)-raw.bin padding.bin crt0.bin symbols.bin relocation.bin partition.bin
	make -C relocator clean

realclean: clean
	$(RM) $(TARGET).elf $(TARGET).bin
	make -C relocator realclean

.PHONY: all clean realclean
