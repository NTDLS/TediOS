	.file "memcpy.s"
	.text
	.balign	16,,7
	.globl	_memcpy
_memcpy:
	pushl	%ebp
	movl	%esp,%ebp
	pushl	%esi
	pushl	%edi
	movl	8(%ebp),%edi
	movl	12(%ebp),%esi
	movl	16(%ebp),%ecx
	call	__movedata
	popl	%edi
	popl	%esi
	movl	8(%ebp),%eax
	leave
	ret

