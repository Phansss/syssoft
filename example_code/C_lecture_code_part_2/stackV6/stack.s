	.file	"stack.c"
	.text
	.globl	stack_create
	.type	stack_create, @function
stack_create:
.LFB2:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movl	$16, %edi
	call	malloc
	movq	%rax, -8(%rbp)
	cmpq	$0, -8(%rbp)
	jne	.L2
	movq	-24(%rbp), %rax
	movq	$0, (%rax)
	movl	$3, %eax
	jmp	.L3
.L2:
	movq	-8(%rbp), %rax
	movq	$0, (%rax)
	movq	-8(%rbp), %rax
	movl	$0, 8(%rax)
	movq	-24(%rbp), %rax
	movq	-8(%rbp), %rdx
	movq	%rdx, (%rax)
	movl	$0, %eax
.L3:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE2:
	.size	stack_create, .-stack_create
	.globl	stack_free
	.type	stack_free, @function
stack_free:
.LFB3:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movq	-24(%rbp), %rax
	movq	(%rax), %rax
	movq	%rax, -16(%rbp)
	jmp	.L5
.L6:
	movq	-16(%rbp), %rax
	movq	(%rax), %rax
	movq	8(%rax), %rax
	movq	%rax, -8(%rbp)
	movq	-16(%rbp), %rax
	movq	(%rax), %rax
	movq	%rax, %rdi
	call	free
	movq	-16(%rbp), %rax
	movq	-8(%rbp), %rdx
	movq	%rdx, (%rax)
.L5:
	movq	-16(%rbp), %rax
	movq	(%rax), %rax
	testq	%rax, %rax
	jne	.L6
	movq	-24(%rbp), %rax
	movq	(%rax), %rax
	movq	%rax, %rdi
	call	free
	movq	-24(%rbp), %rax
	movq	$0, (%rax)
	movl	$0, %eax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE3:
	.size	stack_free, .-stack_free
	.globl	stack_push
	.type	stack_push, @function
stack_push:
.LFB4:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movq	%rsi, -32(%rbp)
	movl	$16, %edi
	call	malloc
	movq	%rax, -8(%rbp)
	cmpq	$0, -8(%rbp)
	jne	.L9
	movl	$3, %eax
	jmp	.L10
.L9:
	movq	-8(%rbp), %rax
	movq	-32(%rbp), %rdx
	movq	%rdx, (%rax)
	movq	-24(%rbp), %rax
	movq	(%rax), %rdx
	movq	-8(%rbp), %rax
	movq	%rdx, 8(%rax)
	movq	-24(%rbp), %rax
	movq	-8(%rbp), %rdx
	movq	%rdx, (%rax)
	movq	-24(%rbp), %rax
	movl	8(%rax), %eax
	leal	1(%rax), %edx
	movq	-24(%rbp), %rax
	movl	%edx, 8(%rax)
	movl	$0, %eax
.L10:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE4:
	.size	stack_push, .-stack_push
	.globl	stack_pop
	.type	stack_pop, @function
stack_pop:
.LFB5:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movq	-24(%rbp), %rax
	movq	(%rax), %rax
	testq	%rax, %rax
	jne	.L12
	movl	$1, %eax
	jmp	.L13
.L12:
	movq	-24(%rbp), %rax
	movq	(%rax), %rax
	movq	%rax, -8(%rbp)
	movq	-24(%rbp), %rax
	movq	(%rax), %rax
	movq	8(%rax), %rdx
	movq	-24(%rbp), %rax
	movq	%rdx, (%rax)
	movq	-8(%rbp), %rax
	movq	%rax, %rdi
	call	free
	movq	-24(%rbp), %rax
	movl	8(%rax), %eax
	leal	-1(%rax), %edx
	movq	-24(%rbp), %rax
	movl	%edx, 8(%rax)
	movl	$0, %eax
.L13:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE5:
	.size	stack_pop, .-stack_pop
	.globl	stack_top
	.type	stack_top, @function
stack_top:
.LFB6:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movq	-8(%rbp), %rax
	movq	(%rax), %rax
	testq	%rax, %rax
	jne	.L15
	movl	$1, %eax
	jmp	.L16
.L15:
	movq	-8(%rbp), %rax
	movq	(%rax), %rax
	movq	(%rax), %rdx
	movq	-16(%rbp), %rax
	movq	%rdx, (%rax)
	movl	$0, %eax
.L16:
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE6:
	.size	stack_top, .-stack_top
	.globl	stack_size
	.type	stack_size, @function
stack_size:
.LFB7:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	movq	-8(%rbp), %rax
	movl	8(%rax), %eax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE7:
	.size	stack_size, .-stack_size
	.ident	"GCC: (Ubuntu 5.4.0-6ubuntu1~16.04.9) 5.4.0 20160609"
	.section	.note.GNU-stack,"",@progbits
