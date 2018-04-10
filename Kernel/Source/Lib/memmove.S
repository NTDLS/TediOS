	.file "memmove.s"
	.globl	_memmove
_memmove:
	pushl	%ebp
	movl	%esp,%ebp
	pushl	%esi
	pushl	%edi
	movl	8(%ebp),%edi
	movl	12(%ebp),%esi
	movl	16(%ebp),%ecx
	jecxz	L2

	cmpl	%esi,%edi
	jb	L3

	call    __movedata_rev
	jmp     L2
L3:
	call    __movedata

L2:
	cld
	popl	%edi
	popl	%esi
	movl	8(%ebp),%eax
	leave
	ret
