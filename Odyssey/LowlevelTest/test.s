
	.text
	.option	reorder off
	.align 4
	.globl	r7k_get_ic
r7k_get_ic:
	cfc0	$2, $20
	nop
	nop
	nop
	jr	ra
	nop

	.align 4
	.globl	r7k_set_ic
r7k_set_ic:
	ctc0	$4, $20
	nop
	nop
	nop
	jr	ra
	nop

	.align 4
	.globl	r7k_get_18
r7k_get_18:
	cfc0	$2, $18
	nop
	nop
	nop
	jr	ra
	nop

	.align 4
	.globl	r7k_get_19
r7k_get_19:
	cfc0	$2, $19
	nop
	nop
	nop
	jr	ra
	nop
