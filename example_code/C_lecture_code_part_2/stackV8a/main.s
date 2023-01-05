	.file	"main.c"
	.section	.rodata
	.align 8
.LC0:
	.string	"Give a value to push on the stack (negative value to quit): "
.LC1:
	.string	"%20ld"
.LC2:
	.string	"\nThe stack size is %d\n"
.LC3:
	.string	"\nThe stack values are:"
.LC4:
	.string	"%ld\n"
	.text
	.globl	main
	.type	main, @function
main:
.LFB2:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$48, %rsp
	movq	%fs:40, %rax
	movq	%rax, -8(%rbp)
	xorl	%eax, %eax
	leaq	-32(%rbp), %rax
	movl	$copy_element, %edx
	movl	$destroy_element, %esi
	movq	%rax, %rdi
	call	stack_create
	movl	%eax, %edi
	call	err_handler
.L3:
	movl	$.LC0, %edi
	movl	$0, %eax
	call	printf
	leaq	-24(%rbp), %rax
	movq	%rax, %rsi
	movl	$.LC1, %edi
	movl	$0, %eax
	call	__isoc99_scanf
	movq	-24(%rbp), %rax
	testq	%rax, %rax
	js	.L2
	movq	-32(%rbp), %rax
	leaq	-24(%rbp), %rdx
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	stack_push
	movl	%eax, -36(%rbp)
	movl	-36(%rbp), %eax
	movl	%eax, %edi
	call	err_handler
.L2:
	movq	-24(%rbp), %rax
	testq	%rax, %rax
	jns	.L3
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	call	stack_size
	movl	%eax, %esi
	movl	$.LC2, %edi
	movl	$0, %eax
	call	printf
	movl	$.LC3, %edi
	call	puts
	jmp	.L4
.L5:
	movq	-32(%rbp), %rax
	leaq	-16(%rbp), %rdx
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	stack_top
	movl	%eax, -36(%rbp)
	movl	-36(%rbp), %eax
	movl	%eax, %edi
	call	err_handler
	movq	-16(%rbp), %rax
	movq	(%rax), %rax
	movq	%rax, %rsi
	movl	$.LC4, %edi
	movl	$0, %eax
	call	printf
	movq	-16(%rbp), %rax
	movq	%rax, %rdi
	call	free
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	call	stack_pop
	movl	%eax, %edi
	call	err_handler
.L4:
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	call	stack_size
	testl	%eax, %eax
	jne	.L5
	leaq	-32(%rbp), %rax
	movq	%rax, %rdi
	call	stack_free
	movl	%eax, %edi
	call	err_handler
	movl	$0, %eax
	movq	-8(%rbp), %rcx
	xorq	%fs:40, %rcx
	je	.L7
	call	__stack_chk_fail
.L7:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE2:
	.size	main, .-main
	.section	.rodata
	.align 8
.LC5:
	.string	"\nCan't execute this operation while the stack is empty."
	.align 8
.LC6:
	.string	"\nCan't execute this operation while the stack is full."
	.align 8
.LC7:
	.string	"\nMemory problem occured while executing this operation on the stack."
	.align 8
.LC8:
	.string	"\nStack initialization problem."
	.align 8
.LC9:
	.string	"\nUndefined problem occured while executing this operation on the stack."
.LC10:
	.string	"main.c"
.LC11:
	.string	"1==0"
	.text
	.type	err_handler, @function
err_handler:
.LFB3:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movl	%edi, -4(%rbp)
	cmpl	$5, -4(%rbp)
	ja	.L9
	movl	-4(%rbp), %eax
	movq	.L11(,%rax,8), %rax
	jmp	*%rax
	.section	.rodata
	.align 8
	.align 4
.L11:
	.quad	.L18
	.quad	.L12
	.quad	.L13
	.quad	.L14
	.quad	.L15
	.quad	.L16
	.text
.L12:
	movl	$.LC5, %edi
	call	puts
	jmp	.L17
.L13:
	movl	$.LC6, %edi
	call	puts
	jmp	.L17
.L14:
	movl	$.LC7, %edi
	call	puts
	jmp	.L17
.L15:
	movl	$.LC8, %edi
	call	puts
	jmp	.L17
.L16:
	movl	$.LC9, %edi
	call	puts
	jmp	.L17
.L9:
	movl	$__PRETTY_FUNCTION__.2876, %ecx
	movl	$86, %edx
	movl	$.LC10, %esi
	movl	$.LC11, %edi
	call	__assert_fail
.L18:
	nop
.L17:
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE3:
	.size	err_handler, .-err_handler
	.globl	destroy_element
	.type	destroy_element, @function
destroy_element:
.LFB4:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	movq	-8(%rbp), %rax
	movq	%rax, %rdi
	call	free
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE4:
	.size	destroy_element, .-destroy_element
	.section	.rodata
.LC12:
	.string	"p != NULL"
	.text
	.globl	copy_element
	.type	copy_element, @function
copy_element:
.LFB5:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movl	$8, %edi
	call	malloc
	movq	%rax, -8(%rbp)
	cmpq	$0, -8(%rbp)
	jne	.L21
	movl	$__PRETTY_FUNCTION__.2884, %ecx
	movl	$101, %edx
	movl	$.LC10, %esi
	movl	$.LC12, %edi
	call	__assert_fail
.L21:
	movq	-24(%rbp), %rax
	movq	(%rax), %rdx
	movq	-8(%rbp), %rax
	movq	%rdx, (%rax)
	movq	-8(%rbp), %rax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE5:
	.size	copy_element, .-copy_element
	.section	.rodata
	.align 8
	.type	__PRETTY_FUNCTION__.2876, @object
	.size	__PRETTY_FUNCTION__.2876, 12
__PRETTY_FUNCTION__.2876:
	.string	"err_handler"
	.align 8
	.type	__PRETTY_FUNCTION__.2884, @object
	.size	__PRETTY_FUNCTION__.2884, 13
__PRETTY_FUNCTION__.2884:
	.string	"copy_element"
	.ident	"GCC: (Ubuntu 5.4.0-6ubuntu1~16.04.10) 5.4.0 20160609"
	.section	.note.GNU-stack,"",@progbits
