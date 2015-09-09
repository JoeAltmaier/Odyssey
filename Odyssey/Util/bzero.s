#/*************************************************************************
# * This material is a confidential trade secret and proprietary 
# * information of ConvergeNet Technologies, Inc. which may not be 
# * reproduced, used, sold or transferred to any third party without the 
# * prior written consent of ConvergeNet Technologies, Inc.  This material 
# * is also copyrighted as an unpublished work under sections 104 and 408 
# * of Title 17 of the United States Code.  Law prohibits unauthorized 
# * use, copying or reproduction.
# *
# * File: bzero.s
# * 
# * Description:
# * Code for bzero 
# * 
# * Update Log:
# * 10/12/98 Raghava Kondapalli: Created 
# ************************************************************************/
#
#/*
# * Copyright (c) 1991, 1993
# *	The Regents of the University of California.  All rights reserved.
# *
# * This code is derived from software contributed to Berkeley by
# * Ralph Campbell.
# *
# * Redistribution and use in source and binary forms, with or without
# * modification, are permitted provided that the following conditions
# * are met:
# * 1. Redistributions of source code must retain the above copyright
# *    notice, this list of conditions and the following disclaimer.
# * 2. Redistributions in binary form must reproduce the above copyright
# *    notice, this list of conditions and the following disclaimer in the
# *    documentation and/or other materials provided with the distribution.
# * 3. All advertising materials mentioning features or use of this software
# *    must display the following acknowledgement:
# *	This product includes software developed by the University of
# *	California, Berkeley and its contributors.
# * 4. Neither the name of the University nor the names of its contributors
# *    may be used to endorse or promote products derived from this software
# *    without specific prior written permission.
# *
# * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
# * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
# * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# * SUCH DAMAGE.
# * 02/19/99 Jim Frandeen - Make it compile for either GreenHills
# * 			or Metrowerks.
# * 06/16/99 Joe Altmaier - optimize for 64-bit.  Use swl for big-endian.
# */
#
#
.include "IsGreenHills.h"
.include "MipsRegs.h"

	.text
	.globl	bzero


bzero:
	.option	reorder off
	blt	a1, 12, smallclr	# small amount to clear?
	subu	a3, zero, a0		# compute # bytes to word align address
	and	a3, a3, 3
	beq	a3, zero, L1		# skip if word aligned
	subu	a1, a1, a3		# subtract from remaining count
#	swr	zero, 0(a0)		# clear 1, 2, or 3 bytes to align
	swl	zero, 0(a0)		# clear 1, 2, or 3 bytes to align
	addu	a0, a0, a3
L1:
# a0 pMem, word aligned
# a1 cb
	blt	a1, 4, smallclr
	and	v0, a0, 7		# compute number of bytes to dword align address
	beqz v0, L15
	sw	zero, 0(a0)
	addu	a0, a0, 4
	addu	a1, a1, -4
L15:
# a0 pMem, dword aligned
# a1 cb
	and	v0, a1, 7		# compute number of words left
	subu	a3, a1, v0
	move	a1, v0
	beqz	a3, smallclr
	addu	a3, a3, a0		# compute ending address
L2:
	addu	a0, a0, 8		# clear words
	bne	a0, a3, L2		#   unrolling loop doesnt help
	sd	zero, -8(a0)		#   since we are limited by memory speed
smallclr:
	ble	a1, zero, L4
	addu	a3, a1, a0		# compute ending address
L3:
	addu	a0, a0, 1		# clear bytes
	bne	a0, a3, L3
	sb	zero, -1(a0)
L4:
	j	ra
	nop

	.globl bzero64
bzero64:

L101:
	sd	zero, 0(a0)
	subu	a1, a1, 8
	bnez	a1, L101
	addu	a0, a0, 8

	jr	ra
	nop


	.option	reorder on
