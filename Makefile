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

PREFIX         := arm-none-eabi-
OBJCOPY        := $(PREFIX)objcopy

BIN             = pip-mpu-launcher

all: $(BIN).elf

$(BIN).elf: $(BIN).bin
# objcopy needs the output file not to be empty...
	@printf '\0' > $@
	$(OBJCOPY)\
          --input-target=binary\
          --output-target=elf32-littlearm\
          --wildcard\
          --remove-section '*'\
          --add-section .text=$<\
        $@

$(BIN).bin: root.bin padding.bin child.bin
	cat root.bin padding.bin child.bin > $@

padding.bin: root.bin
	dd\
          if=/dev/zero\
          of=$@ bs=$$(((($$(wc -c < root.bin) + 32 - 1) & ~(32 - 1)) - $$(wc -c < root.bin)))\
          count=1\
          conv=fsync

root.bin:
	make -C root
	cp root/$@ $@

child.bin:
	make -C child
	cp child/$@ $@

realclean: clean
	$(RM) $(BIN).elf $(BIN).bin
	$(MAKE) -C root realclean
	$(MAKE) -C child realclean

clean:
	$(RM) root.bin padding.bin child.bin
	$(MAKE) -C root clean
	$(MAKE) -C child clean

.PHONY: realclean clean
