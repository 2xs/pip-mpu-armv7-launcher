/*******************************************************************************/
/*  © Université de Lille, The Pip Development Team (2015-2024)                */
/*                                                                             */
/*  This software is a computer program whose purpose is to run a minimal,     */
/*  hypervisor relying on proven properties such as memory isolation.          */
/*                                                                             */
/*  This software is governed by the CeCILL license under French law and       */
/*  abiding by the rules of distribution of free software.  You can  use,      */
/*  modify and/ or redistribute the software under the terms of the CeCILL     */
/*  license as circulated by CEA, CNRS and INRIA at the following URL          */
/*  "http://www.cecill.info".                                                  */
/*                                                                             */
/*  As a counterpart to the access to the source code and  rights to copy,     */
/*  modify and redistribute granted by the license, users are provided only    */
/*  with a limited warranty  and the software's author,  the holder of the     */
/*  economic rights,  and the successive licensors  have only  limited         */
/*  liability.                                                                 */
/*                                                                             */
/*  In this respect, the user's attention is drawn to the risks associated     */
/*  with loading,  using,  modifying and/or developing or reproducing the      */
/*  software by the user in light of its specific status of free software,     */
/*  that may mean  that it is complicated to manipulate,  and  that  also      */
/*  therefore means  that it is reserved for developers  and  experienced      */
/*  professionals having in-depth computer knowledge. Users are therefore      */
/*  encouraged to load and test the software's suitability as regards their    */
/*  requirements in conditions enabling the security of their systems and/or   */
/*  data to be ensured and,  more generally, to use and operate it in the      */
/*  same conditions as regards security.                                       */
/*                                                                             */
/*  The fact that you are presently reading this means that you have had       */
/*  knowledge of the CeCILL license and that you accept its terms.             */
/*******************************************************************************/

#include <stdint.h>
#include <stddef.h>

#include "crt0.h"
#include "pip-mpu.h"

/*
 * WARNING: No global variable must be declared in this file!
 */

#ifdef __GNUC__

/**
 * @def     NORETURN
 *
 * @brief   Instructs the compiler that the function will not return
 *
 * @warning The _start function must have this attribute
 */
#define NORETURN __attribute__((noreturn))

/**
 * @def     NAKED
 *
 * @brief   Instructs the compiler that the function will not have
 *          prologue/epilogue sequences
 */
#define NAKED __attribute__((naked))

/**
 * @def     UNUSED
 *
 * @brief   Instructs the compiler that the function/variable is meant
 *          to be possibly unused
 */
#define UNUSED __attribute__((unused))

#else

#error "GCC is required to compile this source file"

#endif /* __GNUC__ */

/**
 * @def     LDM_STM_NB_BYTES_COPIED
 *
 * @brief   Number of bytes copied by the LDM and STM instructions
 */
#define LDM_STM_NB_BYTES_COPIED (48)

/**
 * @def     LDRD_STRD_NB_BYTES_COPIED
 *
 * @brief   Number of bytes copied by the LDRD and STRD instructions
 */
#define LDRD_STRD_NB_BYTES_COPIED (8)

/**
 * @def     LDR_STR_NB_BYTES_COPIED
 *
 * @brief   Number of bytes copied by the LDR and STR instructions
 */
#define LDR_STR_NB_BYTES_COPIED (4)

/**
 * @def     LDRB_STRB_NB_BYTES_COPIED
 *
 * @brief   Number of bytes copied by the LDRB and STRB instructions
 */
#define LDRB_STRB_NB_BYTES_COPIED (1)

/**
 * @def     ROUND
 *
 * @brief   Round x to the next power of two y
 *
 * @param x The number to round to the next power of two y
 *
 * @param y The power of two with which to round x
 */
#define ROUND(x, y) (((x) + (y) - 1) & ~((y) - 1))

/**
 * @define  THUMB_ADDRESS
 *
 * @brief   Calculate the odd address for Thumb mode from x
 *
 * @param x The address to convert to thumb mode
 */
#define THUMB_ADDRESS(x) (((x) & (UINT32_MAX - 1)) | 1)

/**
 * @def     ERR_MSG_PREFIX
 *
 * @brief   Prefix of error messages
 */
#define ERR_MSG_PREFIX "crt0: "

/**
 * @def     ERR_MSG_1
 *
 * @brief   Error message number 1
 */
#define ERR_MSG_1 "not enough ram"

/**
 * @def     ERR_MSG_2
 *
 * @brief   Error message number 2
 */
#define ERR_MSG_2 "out-of-bounds offset"

/**
 * @def     ERR_MSG_3
 *
 * @brief   Error message number 3
 */
#define ERR_MSG_3 "cannot relocate offsets in .rom"

/**
 * @def     ERR_MSG_4
 *
 * @brief   Error message number 4
 */
#define ERR_MSG_4 "cannot relocate offsets in .got"

/**
 * @def     SYS_WRITE0
 *
 * @brief   Semihosting software interrupts (SWIs) to write a
 *          null-terminated string to the console.
 */
#define SYS_WRITE0 "4"

/**
 * @def     ANGEL_SWI
 *
 * @brief   Value indicating to the host that we are requesting a
 *          semihosting operation.
 */
#define ANGEL_SWI  "0xab"

/**
 * @brief   Enumeration of error message identifiers
 */
typedef enum err_msg_id_u {
	ERR_MSG_ID_1, /**< identifier of error message 1 */
	ERR_MSG_ID_2, /**< identifier of error message 2 */
	ERR_MSG_ID_3, /**< identifier of error message 3 */
	ERR_MSG_ID_4, /**< identifier of error message 4 */
} err_msg_id_t;

static inline void* memcpy(void *dest, const void *src, size_t n);
static NAKED void die(err_msg_id_t id UNUSED);

/**
 * @brief   Entry point of the crt0
 */
extern NORETURN void
_start(interface_t *interface, void *arg)
{
	/* Retrieve current memory layout. */
	uint32_t binaryAddr =
		(uint32_t) interface->root;
	uint32_t unusedRamAddr =
		(uint32_t) interface->unusedRamStart;
	uint32_t ramEndAddr =
		(uint32_t) interface->ramEnd;

	/* Retrieve metadata informations. */
	metadata_t *metadata = (metadata_t *)
		(binaryAddr + (uint32_t) &__metadataOff);
	uint32_t entryPointOffset =
		metadata->symbolTable.entryPoint;
	uint32_t romSecSize =
		metadata->symbolTable.romSecSize;
	uint32_t gotSecSize =
		metadata->symbolTable.gotSecSize;
	uint32_t romRamSecSize =
		metadata->symbolTable.romRamSecSize;
	uint32_t ramSecSize =
		metadata->symbolTable.ramSecSize;

	/* Calculation of the section start address in ROM. */
	uint32_t romSecAddr =
		(uint32_t) metadata + sizeof(metadata->symbolTable) +
		sizeof(metadata->patchinfoTable.entryNumber) +
		metadata->patchinfoTable.entryNumber *
		sizeof(patchinfoEntry_t);
	uint32_t gotSecAddr = romSecAddr + romSecSize;
	uint32_t romRamSecAddr = gotSecAddr + gotSecSize;
	uint32_t entryPointAddr = THUMB_ADDRESS(romSecAddr + entryPointOffset);

	/* Calculation of the relocated section start address in RAM. */
	uint32_t relGotSecAddr = unusedRamAddr;
	uint32_t relRomRamSecAddr = relGotSecAddr + gotSecSize;
	uint32_t relRamSecAddr = relRomRamSecAddr + romRamSecSize;

	/* Check if there is enough RAM to perform relocation. */
	if (relGotSecAddr + gotSecSize > ramEndAddr ||
	    relRomRamSecAddr + romRamSecSize > ramEndAddr ||
	    relRamSecAddr + ramSecSize > ramEndAddr)
	{
		die(ERR_MSG_ID_1);
	}

	/* Update of the unused RAM value. */
	interface->unusedRamStart =
		(void *)(relRamSecAddr + ramSecSize);
	/* Update of the unused ROM value. */
	interface->unusedRomStart = (void *)ROUND(
		(uintptr_t)interface->unusedRomStart +
		((uint32_t)&__metadataOff) +
		sizeof(metadata_t) +
		metadata->symbolTable.romRamEnd
	, 32);

	/* Relocation of the '.rom.ram' section. */
	(void)memcpy((void *) relRomRamSecAddr,
	             (void *) romRamSecAddr,
	             (size_t) romRamSecSize);

	/* Initialization of the '.ram' section. */
	for (size_t i = 0; (i << 2) < ramSecSize; i++)
	{
		((uint32_t *) relRamSecAddr)[i] = 0;
	}

	/* Relocation of the '.got' section from the ROM to the RAM,
	 * replacing on the fly each global variable offset, expressed
	 * relative to the beginning of the binary file, with the
	 * address of the memory area where they were relocated or
	 * loaded. */
	for (size_t i = 0; (i << 2) < gotSecSize; i++)
	{
		uint32_t off = ((uint32_t *) gotSecAddr)[i];
		uint32_t addr = 0;

		if (off < romSecSize)
		{
			addr = romSecAddr + off;
			goto validGotEntry;
		}
		off -= romSecSize;

		if (off < gotSecSize)
		{
			/*
			 * Note that offset should always be zero.
			 * This should be for the _rom_size symbol.
			 */
			addr = relGotSecAddr + off;
			goto validGotEntry;
		}
		off -= gotSecSize;

		if (off < romRamSecSize)
		{
			addr = relRomRamSecAddr + off;
			goto validGotEntry;
		}
		off -= romRamSecSize;

		if (off < ramSecSize)
		{
			addr = relRamSecAddr + off;
			goto validGotEntry;
		}

		die(ERR_MSG_ID_2);

validGotEntry:
		((uint32_t *) relGotSecAddr)[i] = addr;
	}

	/* Patch each global pointer by assigning the relocated address
	 * of the value pointed to by the pointer to the relocated
	 * pointer address. */
	for (size_t i = 0; i < metadata->patchinfoTable.entryNumber; i++)
	{
		uint32_t ptrOff = metadata->patchinfoTable.entries[i].ptrOff;
		uint32_t off = *((uint32_t *)(romSecAddr + ptrOff));
		uint32_t ptrAddr = 0, addr = 0;

		if (ptrOff < romSecSize)
		{
			goto ptrOffInRom;
		}
		ptrOff -= romSecSize;

		if (ptrOff < gotSecSize)
		{
			goto offInGot;
		}
		ptrOff -= gotSecSize;

		if (ptrOff < romRamSecSize)
		{
			ptrAddr = relRomRamSecAddr + ptrOff;
			goto validPtrAddr;
		}
		ptrOff -= romRamSecSize;

		if (ptrOff < ramSecSize)
		{
			ptrAddr = relRamSecAddr + ptrOff;
			goto validPtrAddr;
		}

		goto offOutBounds;

validPtrAddr:
		if (off < romSecSize)
		{
			addr = romSecAddr + off;
			goto validAddr;
		}
		off -= romSecSize;

		if (off < gotSecSize)
		{
			goto offInGot;
		}
		off -= gotSecSize;

		if (off < romRamSecSize)
		{
			addr = relRomRamSecAddr + off;
			goto validAddr;
		}
		off -= romRamSecSize;

		if (off < ramSecSize)
		{
			addr = relRamSecAddr + off;
			goto validAddr;
		}

offOutBounds:
		die(ERR_MSG_ID_2);
ptrOffInRom:
		die(ERR_MSG_ID_3);
offInGot:
		die(ERR_MSG_ID_4);
validAddr:
		*((uint32_t *) ptrAddr) = addr;
	}

	__asm__ volatile
	(
		"   mov    r0, %0               \n"
		"   mov    r1, %1               \n"
		"   mov    sl, %2               \n"
		"   bx     %3                   \n"
		:
		: "r" (interface),
		  "r" (arg),
		  "r" (relGotSecAddr),
		  "r" (entryPointAddr)
		: "r0", "r1", "sl"
	);

	for (;;);
}

/**
 * @brief       A version of memcpy optimized for Cortex-M4
 *
 * @see         Cortex-M4 Technical Reference Manual
 *              3.3.1 Cortex-M4 instructions
 *
 * @param dest  Destination memory area
 *
 * @param src   Source memory area
 *
 * @param n     Number of bytes to copy
 *
 * @return      Returns a pointer to dest
 */
static inline void*
memcpy(void *dest, const void *src, size_t n)
{
	const char *src0 = src;
	char *dest0 = dest;

	while (n >= LDM_STM_NB_BYTES_COPIED) {
		__asm__ volatile
		(
			"ldmia %0!, {r2-r12,r14}\n"
			"stmia %1!, {r2-r12,r14}\n"
			: "+r" (src0), "+r" (dest0)
			:
			:  "r2",  "r3",  "r4",  "r5",
			   "r6",  "r7",  "r8",  "r9",
			  "r10", "r11", "r12", "r14",
			  "memory"
		);
		n -= LDM_STM_NB_BYTES_COPIED;
	}

	while (n >= LDRD_STRD_NB_BYTES_COPIED) {
		__asm__ volatile
		(
			"ldrd r2, r3, [%0], #8\n"
			"strd r2, r3, [%1], #8\n"
			: "+r" (src0), "+r" (dest0)
			:
			: "r2", "r3", "memory"
		);
		n -= LDRD_STRD_NB_BYTES_COPIED;
	}

	if (n >= LDR_STR_NB_BYTES_COPIED) {
		__asm__ volatile
		(
			"ldr r2, [%0], #4\n"
			"str r2, [%1], #4\n"
			: "+r" (src0), "+r" (dest0)
			:
			: "r2", "memory"
		);
		n -= LDR_STR_NB_BYTES_COPIED;
	}

	while (n >= LDRB_STRB_NB_BYTES_COPIED) {
		__asm__ volatile
		(
			"ldrb r2, [%0], #1\n"
			"strb r2, [%1], #1\n"
			: "+r" (src0), "+r" (dest0)
			:
			: "r2", "memory"
		);
		n -= LDRB_STRB_NB_BYTES_COPIED;
	}

	return dest;
}

/**
 * @brief Print error message and stop execution
 *
 * @param Identifier of the message to print
 */
static NAKED void
die(err_msg_id_t id UNUSED)
{
	__asm__ volatile
	(
		"   mov    r2, r0                 \n"
		"   mov    r0, #" SYS_WRITE0 "    \n"
		"   adr.w  r1, 3f                 \n"
		"   bkpt   " ANGEL_SWI "          \n"
		"   mov    r0, #" SYS_WRITE0 "    \n"
		"   adr.w  r3, 1f                 \n"
		"   add.w  r2, r3, r2, lsl #3     \n"
		"   orr.w  r2, #1                 \n"
		"   bx     r2                     \n"
		"1: adr.w  r1, 4f                 \n"
		"   b.w    2f                     \n"
		"   adr.w  r1, 5f                 \n"
		"   b.w    2f                     \n"
		"   adr.w  r1, 6f                 \n"
		"   b.w    2f                     \n"
		"   adr.w  r1, 7f                 \n"
		"2: bkpt   " ANGEL_SWI "          \n"
		"   b      .                      \n"
		"3: .asciz \"" ERR_MSG_PREFIX "\" \n"
		"4: .asciz \"" ERR_MSG_1 "\\n\"   \n"
		"5: .asciz \"" ERR_MSG_2 "\\n\"   \n"
		"6: .asciz \"" ERR_MSG_3 "\\n\"   \n"
		"7: .asciz \"" ERR_MSG_4 "\\n\"   \n"
		"   .align 1                      \n"
	);
}
