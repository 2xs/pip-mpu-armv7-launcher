/*******************************************************************************/
/*  © Université de Lille, The Pip Development Team (2015-2024)                */
/*  Copyright (C) 2020-2024 Orange                                             */
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

#ifndef __SEMIHOSTING_H__
#define __SEMIHOSTING_H__

/**
 * \define  SYS_WRITE0
 *
 * \brief   Semihosting software interrupts (SWIs) to write a
 *          null-terminated string to the console.
 */
#define SYS_WRITE0 (0x04)

/**
 * \define  ANGEL_SWI
 *
 * \brief   Value indicating to the host that we are requesting a
 *          semihosting operation.
 */
#define ANGEL_SWI  (0xab)

/**
 * \brief   Writes the string s to stdout.
 *
 * \param s The string to write to stdout.
 */
static inline void
puts(const char *s)
{
	__asm__ volatile
	(
		/* 
		* Set register r0 to SYS_WRITE0 to indicate to the host
		* that we are requesting to display the string pointed
		* to by register r1.
		*/
		"mov r0, %0\n"
		/* 
		* Set register r1 to the address of the string to be
		* displayed through the semihosting.
		*/
		"mov r1, %1\n"
		/*
		* Indicates to the host that the we are requesting a
		* semihosting operation.
		*/
		"bkpt %2\n"
		:
		: "i" (SYS_WRITE0),
		 "r" (s),
		 "i" (ANGEL_SWI)
		: "r0", "r1", "r2",
		 "r3", "ip", "lr",
		 "memory", "cc"
	);
}

#endif /* __SEMIHOSTING_H__ */
