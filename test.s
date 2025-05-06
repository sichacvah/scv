	.file	"test.c"
	.text
	.globl	scvEmptySlice
	.type	scvEmptySlice, @function
scvEmptySlice:
.LFB0:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	andq	$-16, %rsp
	movq	%rdi, -40(%rsp)
	pxor	%xmm0, %xmm0
	movaps	%xmm0, -32(%rsp)
	movq	%xmm0, -16(%rsp)
	movq	-40(%rsp), %rcx
	movq	-32(%rsp), %rax
	movq	-24(%rsp), %rdx
	movq	%rax, (%rcx)
	movq	%rdx, 8(%rcx)
	movq	-16(%rsp), %rax
	movq	%rax, 16(%rcx)
	movq	-40(%rsp), %rax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	scvEmptySlice, .-scvEmptySlice
	.globl	Exit
	.type	Exit, @function
Exit:
.LFB1:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	movq	-8(%rbp), %rax
	movq	%rax, %rdi
#APP
# 17 "test.c" 1
	movq $60, %rax
syscall

# 0 "" 2
#NO_APP
	movl	$0, %eax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE1:
	.size	Exit, .-Exit
	.globl	_start
	.type	_start, @function
_start:
.LFB2:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	andq	$-16, %rsp
	subq	$32, %rsp
	movq	%rsp, %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	scvEmptySlice
	movq	8(%rsp), %rdx
	movq	16(%rsp), %rax
	addq	%rdx, %rax
	movq	%rax, %rdi
	call	Exit
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE2:
	.size	_start, .-_start
	.ident	"GCC: (GNU) 14.2.1 20250405"
	.section	.note.GNU-stack,"",@progbits
