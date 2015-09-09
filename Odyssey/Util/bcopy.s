#/*************************************************************************
# * This material is a confidential trade secret and proprietary 
# * information of ConvergeNet Technologies, Inc. which may not be 
# * reproduced, used, sold or transferred to any third party without the 
# * prior written consent of ConvergeNet Technologies, Inc.  This material 
# * is also copyrighted as an unpublished work under sections 104 and 408 
# * of Title 17 of the United States Code.  Law prohibits unauthorized 
# * use, copying or reproduction.
# *
# * File: bcopy.s
# * 
# * Description:
# * Assembly code for memcpy bcopy 
# * 
# * Update Log:
# * 10/12/98 Raghava Kondapalli: Created 
# * 02/19/99 Jim Frandeen - Make it compile for either GreenHills
# * 			or Metrowerks.
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
# */

#/*
# * memcpy(to, from, len)
# * {ov}bcopy(from, to, len)
# */
.include "IsGreenHills.h"
.include "MipsRegs.h"

	.text
#	.globl	memcpy
#	.globl	memmove
#memcpy:
#memmove:
#    move    v0, a0          # swap from and to
#    move    a0, a1
#    move    a1, v0
#
	.globl bcopy
	.globl ovbcopy
bcopy:
ovbcopy:
	.option	reorder off
	addu	t0, a0, a2		# t0 = end of s1 region
	sltu	t1, a1, t0
	sltu	t2, a0, a1
	and	t1, t1, t2		# t1 = true if from < to < (from+len)
	beq	t1, zero, forward	# non overlapping, do forward copy
	slt	t2, a2, 12		# check for small copy

	ble	a2, zero, done
	addu	t1, a1, a2		# t1 = end of to region
back_loops:
	lb	v0, -1(t0)		# copy bytes backwards,
	subu	t0, t0, 1		#   doesnt happen often so do slow way
	subu	t1, t1, 1
	bne	t0, a0, back_loops
	sb	v0, 0(t1)
	b	done
	nop
	
forward:
	# a0	from
	# a1	to
	# a2	cb
	# t2	cb < 12
	bne	t2, zero, smallcopy	# do a small bcopy
	xor	v0, a0, a1		# compare low two bits of addresses
	and	v0, v0, 3
	subu	a3, zero, a1		# compute # bytes to word align address
	beq	v0, zero, aligned	# addresses can be word aligned
	and	a3, a3, 3

	beq	a3, zero, L1
	subu	a2, a2, a3		# subtract from remaining count
	lwl	v0, 0(a0)		# get next 4 bytes (unaligned)
	lwr	v0, 3(a0)
	addu	a0, a0, a3
	swl	v0, 0(a1)		# store 1, 2, or 3 bytes to align a1
	addu	a1, a1, a3
L1:
	# 'to' aligned
	and	v0, a2, 3		# compute number of words left
	subu	a3, a2, v0
	move	a2, v0
	addu	a3, a3, a0		# compute ending address
L2:
	lwl	v0, 0(a0)		# copy words a0 unaligned, a1 aligned
	lwr	v0, 3(a0)
	addu	a0, a0, 4
	addu	a1, a1, 4
	bne	a0, a3, L2
	sw	v0, -4(a1)
	b	smallcopy
	nop
aligned:
	# a3	# unaligned bytes
	beq		a3, zero, L3
	subu	a2, a2, a3		# subtract from remaining count
	lwr		v0, 0(a0)		# copy 1, 2, or 3 bytes to align
	lwl		v0, 0(a0)		# copy 1, 2, or 3 bytes to align
	addu	a0, a0, a3
	swl		v0, 0(a1)
	addu	a1, a1, a3
L3:
	# a0	4-byte aligned 'from'
	# a1	4-byte aligned 'to'
	# a2	cb
	# align 'to' on 8-byte boundary
	and	v0, a1, 7
	beqz	v0, L35
	lw		t0, 0(a0)
	sw		t0, 0(a1)
	addu	a0, a0,4
	addu	a1, a1,4
	addu	a2, a2,-4
	blt		a2, 8, smallcopy
L35:
	# is 'from' aligned on 8-byte boundary?
	and	v0, a0, 7
	beqz	v0, L37

	# 'from' aligned 4-byte boundary, 'to' aligned 8-byte boundary
	# do 'two fetch, one store' copy
L36:
	lw		t0, 0(a0)
	dsll32		t0, t0, 0
	lwu		t1, 4(a0)
	or		t0, t0, t1
	addu	a0, a0,8
	addu	a1, a1,8
	addu	a2, a2,-8
	sge		t1, a2, 8
	bne		t1, zero, L36
	sd		t0, -8(a1)
	b		smallcopy
	nop
	
L37:
	# a0	8 byte aligned 'from'
	# a1	8 byte aligned 'to'
	# a2	cb
	and	v0, a2, 7		# compute number of whole dwords left
	subu	a3, a2, v0
	move	a2, v0
	addu	a3, a3, a0		# compute ending address
	beq		a0, a3, smallcopy
L4:
	ld	v0, 0(a0)		# copy dwords
	addu	a0, a0, 8
	addu	a1, a1, 8
	bne	a0, a3, L4
	sd	v0, -8(a1)

smallcopy:
	ble	a2, zero, done
	addu	a3, a2, a0		# compute ending address
L5:
	lbu	v0, 0(a0)		# copy bytes
	addu	a0, a0, 1
	addu	a1, a1, 1
	bne	a0, a3, L5
	sb	v0, -1(a1)
done:
	j	ra
	nop

#
#	bcopy64(s1, s2, n)
#	s1, s2 are 64 bit aligned. n is a multiple of 8
#	Warning! Should be aligned
#	
	.globl	bcopy64
bcopy64:

	blt	a2, 32, L102
	nop
L101:
	sub	a2, a2, 32
	ld	t0, 0(a0)	
	ld	t1, 8(a0)	
	ld	t2, 16(a0)	
	ld	t3, 24(a0)	

	sd	t0, 0(a1)
	sd	t1, 8(a1)
	sd	t2, 16(a1)
	sd	t3, 24(a1)

	add	a0, a0, 32
	bge	a2, 40, L101
	add	a1, a1, 32

# copy the rest
L102:
	sub	a2, a2, 8
	ld	v0, 0(a0)	
	sd	v0, 0(a1)
	add	a0, a0, 8
	bnez	a2, L102
	add	a1, a1, 8

	jr	ra
	nop

	.option reorder on

#			unsigned char aData[256];
#			unsigned char aDest[256];
#			case 'q':
#				printf("bzero test\n");
#				for (int i=0; i < 16; i++) {
#					printf("offset %d\n", i);
#					for (int cb=0; cb < 128; cb++) {
#						for (int ib=0; ib < sizeof(aData); ib++) aData[ib]=0xFF;
#						bzero(&aData[i+16], cb);
#						// Test
#						for (int ib=0; ib < i+16; ib++)
#							if (aData[ib] != (unsigned char)0xFF)
#								Fault(i, cb);
#						for (int ib=i+16; ib < i+16+cb; ib++)
#							if (aData[ib] != 0)
#								Fault(i, cb);
#						for (int ib=i+16+cb; ib < sizeof(aData); ib++)
#							if (aData[ib] != (unsigned char)0xFF)
#								Fault(i, cb);
#						}
#					}
#				printf("bzero test done\n");
#
#
#				printf("bcopy test\n");
#				for (int i1=0; i1 < 16; i1++) {
#				for (int i2=0; i2 < 16; i2++) {
#					printf("offset %d - %d\n", i1, i2);
#					for (int cb=0; cb < 128; cb++) {
#						for (int ib=0; ib < sizeof(aData); ib++) { aData[ib]=ib; aDest[ib]=0xFF; }
#						bcopy(&aData[i1+16], &aDest[i2+16], cb);
#						// Test
#						for (int ib=0; ib < i2+16; ib++)
#							if (aDest[ib] != (unsigned char)0xFF)
#								Fault(i2, cb);
#						int iTest=i1+16;
#						for (int ib=i2+16; ib < i2+16+cb; ib++)
#							if (aDest[ib] != iTest++)
#								Fault(i2, cb);
#						for (int ib=i2+16+cb; ib < sizeof(aDest); ib++)
#							if (aDest[ib] != (unsigned char)0xFF)
#								Fault(i2, cb);
#						}
#					}
#					}
#
#				for (int i2=0; i2 < 32; i2++) {
#					printf("offset %d\n", i2);
#					for (int cb=0; cb < 16; cb++) {
#						for (int ib=0; ib < sizeof(aData); ib++) { aData[ib]=0xFF; }
#						for (int ib=0; ib < cb; ib++) { aData[ib+16]=ib; }
#						bcopy(&aData[16], &aData[i2], cb);
#						// Test
#						for (int ib=0; ib < cb; ib++)
#							if (aData[i2+ib] != ib)
#								Fault(i2, cb);
#						}
#					}
#
#				printf("bcopy test done\n");
#				break;
#