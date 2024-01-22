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

/*!
 * \file adt.h
 *
 * \brief Helper structures.
 */

#ifndef __ADT_H__
#define __ADT_H__

/*!
 * \def VIDT_INTERRUPT_NUMBER
 *
 * \brief The number of interrupt for the nRF52832.
 */
#define VIDT_INTERRUPT_NUMBER 54

/*!
 * \brief blockAttr structure
 */
typedef struct blockAttr
{
    uint32_t* blockentryaddr    ;   //!< Block's local id
    uint32_t* blockstartaddr    ;   //!< Block's start address
    uint32_t* blockendaddr      ;   //!< Block's end address
    uint32_t read               ;   //!< Read permission bit
    uint32_t write              ;   //!< Write permission bit
    uint32_t exec               ;   //!< Exec permission bit
    uint32_t accessible         ;   //!< Block accessible bit
}__attribute__((packed)) blockAttr_t;

/*!
 * \brief When the block is empty, the error flag is set to -1,
 *        otherwise contains a block's public attributes.
 */
typedef union blockOrError_t
{
    int32_t error               ;   //!< Error -1 for an empty block
    blockAttr_t blockAttr       ;   //!< A block's publicly exposed attributes
}__attribute__((packed)) blockOrError;

/*!
 * \brief Structure representing the VIDT.
 */
typedef struct vidt_s
{
	uint32_t currentInterrupt;             /*<! The last interrupt number. */
	void *contexts[VIDT_INTERRUPT_NUMBER]; /*<! Pointers to contexts to restore. */
} vidt_t;

#endif /* __ADT_H__ */
