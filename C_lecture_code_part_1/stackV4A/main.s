	.file	"main.c"
	.section	.rodata
	.align 4
.LC0:
	.string	"Give a value to push on the stack (negative value to quit): "
.LC1:
	.string	"%d"
.LC2:
	.string	"\nThe stack values are:"
.LC3:
	.string	"%d\n"
	.text
	.globl	main
	.type	main, @function
main:
.LFB2:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	andl	$-16, %esp
	subl	$32, %esp
	leal	20(%esp), %eax
	movl	%eax, (%esp)
	call	stack_create
	movl	%eax, (%esp)
	call	err_handler
.L3:
	movl	$.LC0, (%esp)
	call	printf
	leal	24(%esp), %eax
	movl	%eax, 4(%esp)
	movl	$.LC1, (%esp)
	call	__isoc99_scanf
	movl	24(%esp), %eax
	testl	%eax, %eax
	js	.L2
	movl	24(%esp), %edx
	movl	20(%esp), %eax
	movl	%edx, 4(%esp)
	movl	%eax, (%esp)
	call	stack_push
	movl	%eax, 28(%esp)
	movl	28(%esp), %eax
	movl	%eax, (%esp)
	call	err_handler
.L2:
	movl	24(%esp), %eax
	testl	%eax, %eax
	jns	.L3
	movl	$.LC2, (%esp)
	call	puts
	jmp	.L4
.L5:
	movl	20(%esp), %eax
	leal	24(%esp), %edx
	movl	%edx, 4(%esp)
	movl	%eax, (%esp)
	call	stack_top
	movl	%eax, 28(%esp)
	movl	28(%esp), %eax
	movl	%eax, (%esp)
	call	err_handler
	movl	24(%esp), %eax
	movl	%eax, 4(%esp)
	movl	$.LC3, (%esp)
	call	printf
	movl	20(%esp), %eax
	movl	%eax, (%esp)
	call	stack_pop
	movl	%eax, (%esp)
	call	err_handler
.L4:
	movl	20(%esp), %eax
	movl	%eax, (%esp)
	call	stack_size
	testl	%eax, %eax
	jne	.L5
	leal	20(%esp), %eax
	movl	%eax, (%esp)
	call	stack_free
	movl	%eax, (%esp)
	call	err_handler
	movl	$0, %eax
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE2:
	.size	main, .-main
	.globl	stack_create
	.type	stack_create, @function
stack_create:
.LFB3:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$40, %esp
	movl	$20, (%esp)
	call	malloc
	movl	%eax, -12(%ebp)
	cmpl	$0, -12(%ebp)
	jne	.L8
	movl	8(%ebp), %eax
	movl	$0, (%eax)
	movl	$3, %eax
	jmp	.L9
.L8:
	movl	-12(%ebp), %eax
	movl	$0, 16(%eax)
	movl	8(%ebp), %eax
	movl	-12(%ebp), %edx
	movl	%edx, (%eax)
	movl	$0, %eax
.L9:
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE3:
	.size	stack_create, .-stack_create
	.globl	stack_free
	.type	stack_free, @function
stack_free:
.LFB4:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$24, %esp
	movl	8(%ebp), %eax
	movl	(%eax), %eax
	movl	%eax, (%esp)
	call	free
	movl	8(%ebp), %eax
	movl	$0, (%eax)
	movl	$0, %eax
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE4:
	.size	stack_free, .-stack_free
	.globl	stack_push
	.type	stack_push, @function
stack_push:
.LFB5:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	movl	8(%ebp), %eax
	movl	16(%eax), %eax
	cmpl	$4, %eax
	jne	.L13
	movl	$2, %eax
	jmp	.L14
.L13:
	movl	8(%ebp), %eax
	movl	16(%eax), %eax
	leal	1(%eax), %ecx
	movl	8(%ebp), %edx
	movl	%ecx, 16(%edx)
	movl	8(%ebp), %edx
	movl	12(%ebp), %ecx
	movl	%ecx, (%edx,%eax,4)
	movl	$0, %eax
.L14:
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE5:
	.size	stack_push, .-stack_push
	.globl	stack_pop
	.type	stack_pop, @function
stack_pop:
.LFB6:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	movl	8(%ebp), %eax
	movl	16(%eax), %eax
	testl	%eax, %eax
	jne	.L16
	movl	$1, %eax
	jmp	.L17
.L16:
	movl	8(%ebp), %eax
	movl	16(%eax), %eax
	leal	-1(%eax), %edx
	movl	8(%ebp), %eax
	movl	%edx, 16(%eax)
	movl	$0, %eax
.L17:
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE6:
	.size	stack_pop, .-stack_pop
	.globl	stack_top
	.type	stack_top, @function
stack_top:
.LFB7:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	movl	8(%ebp), %eax
	movl	16(%eax), %eax
	testl	%eax, %eax
	jne	.L19
	movl	$1, %eax
	jmp	.L20
.L19:
	movl	8(%ebp), %eax
	movl	16(%eax), %eax
	leal	-1(%eax), %edx
	movl	8(%ebp), %eax
	movl	(%eax,%edx,4), %edx
	movl	12(%ebp), %eax
	movl	%edx, (%eax)
	movl	$0, %eax
.L20:
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE7:
	.size	stack_top, .-stack_top
	.globl	stack_size
	.type	stack_size, @function
stack_size:
.LFB8:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	movl	8(%ebp), %eax
	movl	16(%eax), %eax
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE8:
	.size	stack_size, .-stack_size
	.section	.rodata
	.align 4
.LC4:
	.string	"\nCan't execute this operation while the stack is empty."
	.align 4
.LC5:
	.string	"\nCan't execute this operation while the stack is full."
	.align 4
.LC6:
	.string	"\nMemory problem appeared while executing this operation on the stack."
	.align 4
.LC7:
	.string	"\nStack initialisation problem."
	.align 4
.LC8:
	.string	"\nUndefined problem appeared while executing this operation on the stack."
	.text
	.globl	err_handler
	.type	err_handler, @function
err_handler:
.LFB9:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$24, %esp
	cmpl	$5, 8(%ebp)
	ja	.L24
	movl	8(%ebp), %eax
	sall	$2, %eax
	addl	$.L26, %eax
	movl	(%eax), %eax
	jmp	*%eax
	.section	.rodata
	.align 4
	.align 4
.L26:
	.long	.L33
	.long	.L27
	.long	.L28
	.long	.L29
	.long	.L30
	.long	.L31
	.text
.L27:
	movl	$.LC4, (%esp)
	call	puts
	jmp	.L23
.L28:
	movl	$.LC5, (%esp)
	call	puts
	jmp	.L23
.L29:
	movl	$.LC6, (%esp)
	call	puts
	jmp	.L23
.L30:
	movl	$.LC7, (%esp)
	call	puts
	jmp	.L23
.L31:
	movl	$.LC8, (%esp)
	call	puts
	jmp	.L23
.L24:
	movl	$0, (%esp)
	call	exit
.L33:
	nop
.L23:
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE9:
	.size	err_handler, .-err_handler
	.ident	"GCC: (Ubuntu 4.8.4-2ubuntu1~14.04.1) 4.8.4"
	.section	.note.GNU-stack,"",@progbits
