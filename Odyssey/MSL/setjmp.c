//	setjmp.c	-	setjmp() and longjmp() routines for Metrowerks C++ for MIPS
//
// Modified for MIPS Support 081597. 


#include <__config.h>

typedef struct __jmp_buf {

	REG_TYPE	GPR[11];
	
	#if __fpu__
	FREG_TYPE	FPR[12];
	#endif
	
} __jmp_buf;




#ifdef __cplusplus
extern "C" {
#endif

#pragma internal on
int __setjmp(register __jmp_buf* env);
#pragma internal off
void longjmp(register __jmp_buf *env, int val);

#ifdef __cplusplus
}
#endif

/*	__setjmp	-	C setjmp() routine
Upon entry, register a0 contains the address of the __jmp_buf structure
*/



asm int __setjmp(register __jmp_buf* env)
{
	.set reorder
	STORE	s0, __jmp_buf.GPR[0](a0)
	STORE	s1, __jmp_buf.GPR[1](a0)
	STORE	s2, __jmp_buf.GPR[2](a0)
	STORE	s3, __jmp_buf.GPR[3](a0)
	STORE	s4, __jmp_buf.GPR[4](a0)
	STORE	s5, __jmp_buf.GPR[5](a0)
	STORE	s6, __jmp_buf.GPR[6](a0)
	STORE	s7, __jmp_buf.GPR[7](a0)
	STORE	fp, __jmp_buf.GPR[8](a0)
	STORE	sp, __jmp_buf.GPR[9](a0)
	STORE 	ra, __jmp_buf.GPR[10](a0)

	#if __fpu__
	STOREF	f20, __jmp_buf.FPR[0](a0)
	STOREF	f21, __jmp_buf.FPR[1](a0)
	STOREF	f22, __jmp_buf.FPR[2](a0)
	STOREF	f23, __jmp_buf.FPR[3](a0)
	STOREF	f24, __jmp_buf.FPR[4](a0)
	STOREF	f25, __jmp_buf.FPR[5](a0)
	STOREF	f26, __jmp_buf.FPR[6](a0)
	STOREF	f27, __jmp_buf.FPR[7](a0)
	STOREF	f28, __jmp_buf.FPR[8](a0)
	STOREF	f29, __jmp_buf.FPR[9](a0)
	STOREF	f30, __jmp_buf.FPR[10](a0)
	STOREF	f31, __jmp_buf.FPR[11](a0)
	#endif
	
	li	v0,0
	jr	ra
}


//	longjmp		-	C longjmp() routine

asm void longjmp(register __jmp_buf *env,int val)
{
	.set reorder
	LOAD	s0, __jmp_buf.GPR[0](a0)
	LOAD	s1, __jmp_buf.GPR[1](a0)
	LOAD	s2, __jmp_buf.GPR[2](a0)
	LOAD	s3, __jmp_buf.GPR[3](a0)
	LOAD	s4, __jmp_buf.GPR[4](a0)
	LOAD	s5, __jmp_buf.GPR[5](a0)
	LOAD	s6, __jmp_buf.GPR[6](a0)
	LOAD	s7, __jmp_buf.GPR[7](a0)
	LOAD	fp, __jmp_buf.GPR[8](a0)
	LOAD	sp, __jmp_buf.GPR[9](a0)
	LOAD 	ra, __jmp_buf.GPR[10](a0)

	#if __fpu__
	LOADF	f20, __jmp_buf.FPR[0](a0)
	LOADF	f21, __jmp_buf.FPR[1](a0)
	LOADF	f22, __jmp_buf.FPR[2](a0)
	LOADF	f23, __jmp_buf.FPR[3](a0)
	LOADF	f24, __jmp_buf.FPR[4](a0)
	LOADF	f25, __jmp_buf.FPR[5](a0)
	LOADF	f26, __jmp_buf.FPR[6](a0)
	LOADF	f27, __jmp_buf.FPR[7](a0)
	LOADF	f28, __jmp_buf.FPR[8](a0)
	LOADF	f29, __jmp_buf.FPR[9](a0)
	LOADF	f30, __jmp_buf.FPR[10](a0)
	LOADF	f31, __jmp_buf.FPR[11](a0)
	#endif

	MOVE	v0,a1
	jr	ra
}
