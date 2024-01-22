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

#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#include <stdint.h>

#define BASIC_FRAME_SIZE     17

#define EXTENDED_FRAME_SIZE  50

/*!
 * \brief Union representing a basic frame as saved by the
 *        exception entry point.
 */
typedef union basicFrame_u
{
	struct
	{
		uint32_t sp;   /*!< SP register   */
		uint32_t r4;   /*!< R4 register   */
		uint32_t r5;   /*!< R5 register   */
		uint32_t r6;   /*!< R6 register   */
		uint32_t r7;   /*!< R7 register   */
		uint32_t r8;   /*!< R8 register   */
		uint32_t r9;   /*!< R9 register   */
		uint32_t r10;  /*!< R10 register  */
		uint32_t r11;  /*!< R11 register  */
		uint32_t r0;   /*!< R0 register   */
		uint32_t r1;   /*!< R1 register   */
		uint32_t r2;   /*!< R2 register   */
		uint32_t r3;   /*!< R3 register   */
		uint32_t r12;  /*!< R12 register  */
		uint32_t lr;   /*!< LR register   */
		uint32_t pc;   /*!< PC register   */
		uint32_t xpsr; /*!< xPSR register */
	};

	uint32_t registers[BASIC_FRAME_SIZE];

} basicFrame_t;

/*!
 * \brief Union representing an extended frame as saved by the
 *        exception entry point.
 */
typedef union extendedFrame_u
{
	struct
	{
		uint32_t s16;   /*!< S16 register   */
		uint32_t s17;   /*!< S17 register   */
		uint32_t s18;   /*!< S18 register   */
		uint32_t s19;   /*!< S19 register   */
		uint32_t s20;   /*!< S20 register   */
		uint32_t s21;   /*!< S21 register   */
		uint32_t s22;   /*!< S22 register   */
		uint32_t s23;   /*!< S23 register   */
		uint32_t s24;   /*!< S24 register   */
		uint32_t s25;   /*!< S25 register   */
		uint32_t s26;   /*!< S26 register   */
		uint32_t s27;   /*!< S27 register   */
		uint32_t s28;   /*!< S28 register   */
		uint32_t s29;   /*!< S29 register   */
		uint32_t s30;   /*!< S30 register   */
		uint32_t s31;   /*!< S31 register   */
		uint32_t sp;    /*!< SP register    */
		uint32_t r4;    /*!< R4 register    */
		uint32_t r5;    /*!< R5 register    */
		uint32_t r6;    /*!< R6 register    */
		uint32_t r7;    /*!< R7 register    */
		uint32_t r8;    /*!< R8 register    */
		uint32_t r9;    /*!< R9 register    */
		uint32_t r10;   /*!< R10 register   */
		uint32_t r11;   /*!< R11 register   */
		uint32_t r0;    /*!< R0 register    */
		uint32_t r1;    /*!< R1 register    */
		uint32_t r2;    /*!< R2 register    */
		uint32_t r3;    /*!< R3 register    */
		uint32_t r12;   /*!< R12 register   */
		uint32_t lr;    /*!< LR register    */
		uint32_t pc;    /*!< PC register    */
		uint32_t xpsr;  /*!< xPSR register  */
		uint32_t s0;    /*!< S0 register    */
		uint32_t s1;    /*!< S1 register    */
		uint32_t s2;    /*!< S2 register    */
		uint32_t s3;    /*!< S3 register    */
		uint32_t s4;    /*!< S4 register    */
		uint32_t s5;    /*!< S5 register    */
		uint32_t s6;    /*!< S6 register    */
		uint32_t s7;    /*!< S7 register    */
		uint32_t s8;    /*!< S8 register    */
		uint32_t s9;    /*!< S9 register    */
		uint32_t s10;   /*!< S10 register   */
		uint32_t s11;   /*!< S11 register   */
		uint32_t s12;   /*!< S12 register   */
		uint32_t s13;   /*!< S13 register   */
		uint32_t s14;   /*!< S14 register   */
		uint32_t s15;   /*!< S15 register   */
		uint32_t fpscr; /*!< FPSCR register */
	};

	uint32_t registers[EXTENDED_FRAME_SIZE];

} extendedFrame_t;

/*!
 *
 */
typedef struct basicContext_s
{
	uint32_t isBasicFrame;

	uint32_t pipflags;

	basicFrame_t frame;

} basicContext_t;

/*!
 *
 */
typedef struct extendedContext_s
{
	uint32_t isBasicFrame;

	uint32_t pipflags;

	extendedFrame_t frame;

} extendedContext_t;

/*!
 *
 */
typedef struct stackedContext_s
{
	uint32_t isBasicFrame;

	uint32_t pipflags;

	union
	{
		basicFrame_t basicFrame;
		extendedFrame_t extendedFrame;
	};

} stackedContext_t;

#endif /* __CONTEXT_H__ */
