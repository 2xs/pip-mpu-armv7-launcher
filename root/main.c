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
#include <string.h>

#include "pip-mpu.h"
#include "semihosting.h"

/**
 * @define PROGNAME
 *
 * @brief  The name of the partition
 */
#define PROGNAME "root"

/**
 * @define  INITIAL_XPSR
 *
 * @brief   Initial program status register value for a partition
 *
 * @see     ARMv7-M Architecture Reference Manual
 */
#define INITIAL_XPSR (0x01000000)

/**
 * @define  THUMB_ADDRESS(x)
 *
 * @brief   Calculate the odd address for Thumb mode from x
 */
#define THUMB_ADDRESS(x) (((x) & (UINT32_MAX - 1)) | 1)

/**
 * @define  ROUND(x, y)
 *
 * @brief   Round x to the next power of two y
 */
#define ROUND(x, y) (((x) + (y) - 1) & ~((y) - 1))

/**
 * @define  ROOT_BLOCK_ID_1
 *
 * @brief   ID of block number 1 of the root partition
 *
 * @todo    To be corrected once the bug in findBlock has been fixed
 */
#define ROOT_BLOCK_ID_1 ((void *)0x2000f1ad)

/**
 * @define  ROOT_BLOCK_ID_2
 *
 * @brief   ID of block number 2 of the root partition
 *
 * @todo    To be corrected once the bug in findBlock has been fixed
 */
#define ROOT_BLOCK_ID_2 ((void *)0x2000f1be)

static void *root_kern_addr;
static void *child_pd_addr;
static void *child_kern_addr;
static void *child_stack_addr;
static void *child_vidt_addr;
static void *child_ctx_addr;
static void *child_itf_addr;
static void *child_rend_addr;
static void *child_text_addr;
static void *child_fend_addr;

static void *root_kern_id;
static void *child_pd_id;
static void *child_kern_id;
static void *child_block0_id;
static void *child_block1_id;
static void *child_rend_id;
static void *child_block2_id;
static void *child_fend_id;

static void *child_block0_idc;
static void *child_block1_idc;
static void *child_block2_idc;

static basicContext_t root_ctx;

/**
 * @brief   Fill memory with a constant byte
 *
 * @param s A pointer to the memory area to fill
 *
 * @param c A constant byte with which to fill the memory area
 *
 * @param n The number of bytes to fill in the memory area
 */
void *
memset(void *s, int c, size_t n)
{
	size_t i;

	for (i = 0; i < n; i++) {
		((char *)s)[i] = c;
	}

	return s;
}

/**
 * @brief   Initialize the root partition memory
 *
 * @param interface A structure describing the memory layout
 */
static unsigned
root_memory_init(interface_t *interface)
{
	root_kern_addr = (void *)ROUND((uintptr_t)0x20009000, 512);
	child_pd_addr = (char *)root_kern_addr + 512;
	child_kern_addr = (char *)child_pd_addr + 512;
	/* Child MPU block 0 */
	child_stack_addr = (void *)ROUND((uintptr_t)child_kern_addr + 512, 1024);
	child_vidt_addr = (char *)child_stack_addr + 512;
	/* Child MPU block 1 */
	child_ctx_addr = (void *)ROUND((uintptr_t)child_vidt_addr + 512, 1024);
	child_itf_addr = (char *)child_ctx_addr + sizeof(basicContext_t);
	child_rend_addr = (char *)child_ctx_addr + 1024;
	/* Child MPU block 2 */
	child_text_addr = interface->unusedRomStart;
	child_fend_addr = (char *)child_text_addr + 1024;

	/* Initialization of the root partition VIDT */
	(void)memset(interface->vidtStart, 0, sizeof(vidt_t));
	((vidt_t *)interface->vidtStart)->contexts[0] = &root_ctx;

	/* Initialization of the child partition VIDT */
	(void)memset(child_vidt_addr, 0, sizeof(vidt_t));
	((vidt_t *)child_vidt_addr)->contexts[0] = child_ctx_addr;

	/* Initialization of the child partition interface */
	(void)memset(child_itf_addr, 0, sizeof(basicContext_t));
	((interface_t *)child_itf_addr)->root = interface->unusedRomStart;
	((interface_t *)child_itf_addr)->unusedRomStart = (void *)((uintptr_t)interface->unusedRomStart + 256);
	((interface_t *)child_itf_addr)->romEnd = child_fend_addr;
	((interface_t *)child_itf_addr)->unusedRamStart = (char *)child_itf_addr + sizeof(interface_t);
	((interface_t *)child_itf_addr)->ramEnd = child_rend_addr;

	/* Initialization of the child context */
	(void)memset(child_ctx_addr, 0, sizeof(basicContext_t));
	((basicContext_t *)child_ctx_addr)->frame.r0 = (uint32_t)child_itf_addr;
	((basicContext_t *)child_ctx_addr)->frame.pc = THUMB_ADDRESS((uint32_t)interface->unusedRomStart);
	((basicContext_t *)child_ctx_addr)->frame.sp = (uint32_t)((char *)child_vidt_addr - 4);
	((basicContext_t *)child_ctx_addr)->frame.xpsr = INITIAL_XPSR;
	((basicContext_t *)child_ctx_addr)->isBasicFrame = 1;

	/* TODO: to be corrected once the bug in findBlock has been fixed */
	if ((root_kern_id = Pip_cutMemoryBlock(ROOT_BLOCK_ID_1, root_kern_addr, -1)) == NULL) {
		puts(PROGNAME": failed to cut a memory block\n");
		return 0;
	}

	if ((child_pd_id = Pip_cutMemoryBlock(root_kern_id, child_pd_addr, -1)) == NULL) {
		puts(PROGNAME": failed to cut a memory block\n");
		return 0;
	}

	if (!Pip_prepare(interface->partDescBlockId, 8, root_kern_id)) {
		puts(PROGNAME": failed to prepare the root kernel structure\n");
		return 0;
	}

	return 1;
}

/**
 * @brief   Create a child partition
 */
static unsigned
root_partition_create(void)
{
	if ((child_kern_id = Pip_cutMemoryBlock(child_pd_id, child_kern_addr, -1)) == NULL) {
		puts(PROGNAME": failed to cut a memory block\n");
		return 0;
	}

	if ((child_block0_id = Pip_cutMemoryBlock(child_kern_id, child_stack_addr, -1)) == NULL) {
		puts(PROGNAME": failed to cut a memory block\n");
		return 0;
	}

	if ((child_block1_id = Pip_cutMemoryBlock(child_block0_id, child_ctx_addr, -1)) == NULL) {
		puts(PROGNAME": failed to cut a memory block\n");
		return 0;
	}

	if ((child_rend_id = Pip_cutMemoryBlock(child_block1_id, child_rend_addr, -1)) == NULL) {
		puts(PROGNAME": failed to cut a memory block\n");
		return 0;
	}

	/* TODO: to be corrected once the bug in findBlock has been fixed */
	if ((child_block2_id = Pip_cutMemoryBlock(ROOT_BLOCK_ID_2, child_text_addr, -1)) == NULL) {
		puts(PROGNAME": failed to cut a memory block\n");
		return 0;
	}

	if ((child_fend_id = Pip_cutMemoryBlock(child_block2_id, child_fend_addr, -1)) == NULL) {
		puts(PROGNAME": failed to cut a memory block\n");
		return 0;
	}

	if (!Pip_createPartition(child_pd_id)) {
		puts(PROGNAME": failed to create the child partition\n");
		return 0;
	}

	if (!Pip_prepare(child_pd_id, 8, child_kern_id)) {
		puts(PROGNAME": failed to prepare the child kernel structure\n");
		return 0;
	}

	if ((child_block0_idc = Pip_addMemoryBlock(child_pd_id, child_block0_id, 1, 1, 0)) == NULL) {
		puts(PROGNAME": failed to add a memory block to the child\n");
		return 0;
	}

	if ((child_block1_idc = Pip_addMemoryBlock(child_pd_id, child_block1_id, 1, 1, 0)) == NULL) {
		puts(PROGNAME": failed to add a memory block to the child\n");
		return 0;
	}

	if ((child_block2_idc = Pip_addMemoryBlock(child_pd_id, child_block2_id, 1, 0, 1)) == NULL) {
		puts(PROGNAME": failed to add a memory block to the child\n");
		return 0;
	}

	if (!Pip_mapMPU(child_pd_id, child_block0_idc, 0)) {
		puts(PROGNAME": failed to map a block to the child partition\n");
		return 0;
	}

	if (!Pip_mapMPU(child_pd_id, child_block1_idc, 1)) {
		puts(PROGNAME": failed to map a block to the child partition\n");
		return 0;
	}

	if (!Pip_mapMPU(child_pd_id, child_block2_idc, 2)) {
		puts(PROGNAME": failed to map a block to the child partition\n");
		return 0;
	}

	if (!Pip_setVIDT(child_pd_id, (void *)child_vidt_addr)) {
		puts(PROGNAME": failed to set the VIDT of the child\n");
		return 0;
	}

	return 1;
}

/**
 * @brief   Delete a child partition
 */
static unsigned
root_partition_delete(void)
{
	if (!Pip_setVIDT(child_pd_id, NULL)) {
		puts(PROGNAME": failed to set the VIDT of the child\n");
		return 0;
	}

	if (!Pip_mapMPU(child_pd_id, NULL, 2)) {
		puts(PROGNAME": failed to unmap a block to the child partition\n");
		return 0;
	}

	if (!Pip_mapMPU(child_pd_id, NULL, 1)) {
		puts(PROGNAME": failed to unmap a block to the child partition\n");
		return 0;
	}

	if (!Pip_mapMPU(child_pd_id, NULL, 0)) {
		puts(PROGNAME": failed to unmap a block to the child partition\n");
		return 0;
	}

	if (!Pip_removeMemoryBlock(child_block2_id)) {
		puts(PROGNAME": failed to remove a memory block from the child partition\n");
		return 0;
	}

	if (!Pip_removeMemoryBlock(child_block1_id)) {
		puts(PROGNAME": failed to remove a memory block from the child partition\n");
		return 0;
	}

	if (!Pip_removeMemoryBlock(child_block0_id)) {
		puts(PROGNAME": failed to remove a memory block from the child partition\n");
		return 0;
	}

	if (Pip_collect(child_pd_id) == NULL) {
		puts(PROGNAME": failed to collect a kernel structure from the child\n");
		return 0;
	}

	if (!Pip_deletePartition(child_pd_id)) {
		puts(PROGNAME": failed to delete the child partition\n");
		return 0;
	}

	if (Pip_mergeMemoryBlocks(child_block2_id, child_fend_id, -1) == NULL) {
		puts(PROGNAME": failed to merge a memory block\n");
		return 0;
	}

	/* TODO: to be corrected once the bug in findBlock has been fixed */
	if (Pip_mergeMemoryBlocks(ROOT_BLOCK_ID_2, child_block2_id, -1) == NULL) {
		puts(PROGNAME": failed to merge a memory block\n");
		return 0;
	}

	if (Pip_mergeMemoryBlocks(child_block1_id, child_rend_id, -1) == NULL) {
		puts(PROGNAME": failed to merge a memory block\n");
		return 0;
	}

	if (Pip_mergeMemoryBlocks(child_block0_id, child_block1_id, -1) == NULL) {
		puts(PROGNAME": failed to merge a memory block\n");
		return 0;
	}

	if (Pip_mergeMemoryBlocks(child_kern_id, child_block0_id, -1) == NULL) {
		puts(PROGNAME": failed to merge a memory block\n");
		return 0;
	}

	if (Pip_mergeMemoryBlocks(child_pd_id, child_kern_id, -1) == NULL) {
		puts(PROGNAME": failed to merge a memory block\n");
		return 0;
	}

	return 1;
}

/**
 * @brief   Clean up the root partition memory
 */
static unsigned
root_memory_cleanup(interface_t *interface)
{
	if (Pip_collect(interface->partDescBlockId) == NULL) {
		puts(PROGNAME": failed to collect a kernel structure from the root\n");
		return 0;
	}

	if (Pip_mergeMemoryBlocks(root_kern_id, child_pd_id, -1) == NULL) {
		puts(PROGNAME": failed to merge a memory block\n");
		return 0;
	}

	/* TODO: to be corrected once the bug in findBlock has been fixed */
	if (Pip_mergeMemoryBlocks(ROOT_BLOCK_ID_1, root_kern_id, -1) == NULL) {
		puts(PROGNAME": failed to merge a memory block\n");
		return 0;
	}

	return 1;
}

/**
 * @brief   Entry point of the partition
 *
 * @param interface A Structure provided by Pip describing the memory
 *                  layout.
 */
void
start(interface_t *interface)
{
	size_t i;

	puts(PROGNAME": launching\n");

	if (root_memory_init(interface) == 0) {
		goto error;
	}

	if (root_partition_create() == 0) {
		goto error;
	}

	puts(PROGNAME": launching the child partition\n");

	for (i = 0; i < 3; i++) {
		Pip_yield(child_pd_id, 0, 0, 1, 1);
	}

	if (root_partition_delete() == 0) {
		goto error;
	}

	if (root_memory_cleanup(interface) == 0) {
		goto error;
	}

	puts(PROGNAME": done\n");

error:
	for (;;);
}
