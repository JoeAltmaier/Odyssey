#/*************************************************************************
# * This material is a confidential trade secret and proprietary 
# * information of ConvergeNet Technologies, Inc. which may not be 
# * reproduced, used, sold or transferred to any third party without the 
# * prior written consent of ConvergeNet Technologies, Inc.  This material 
# * is also copyrighted as an unpublished work under sections 104 and 408 
# * of Title 17 of the United States Code.  Law prohibits unauthorized 
# * use, copying or reproduction.
# *
# * File: MipsRegs.h
# * 
# * Description: The purpose of this file is so that both Metrowerks
# * and Green Hills assemblers can use the same code base.
# * GREEN_HILLS is defined in IsGreenHills.h. 
# * This file will come from a different directory, depending on
# * whether we are building under GreenHills or Metrowerks.
# * Code for bzero 
# * 
# * Update Log:
# * 02/19/99 Jim Frandeen - created
# ************************************************************************/
.if (GREEN_HILLS)
zero .equ $0
v0	.equ $2
v1	.equ $3
a0	.equ $4
a1	.equ $5
a2	.equ $6
a3	.equ $7
t0	.equ $8
sp	.equ $29
t1	.equ $9
t2	.equ $10
t3	.equ $11
t4	.equ $12
t5	.equ $13
t6	.equ $14
t7	.equ $15
s0	.equ $16
s1	.equ $17
s2	.equ $18
s3	.equ $19
s4	.equ $20
s5	.equ $21
s6	.equ $22
s7	.equ $23
t8	.equ $24
t9	.equ $25
kt0	.equ $26
kt1	.equ $27
gp	.equ $28
fp	.equ $30
s8	.equ $30
ra	.equ $31
C0_CAUSE	.equ $13
CAUSE_SW2	.equ 0x00000200
.endif
