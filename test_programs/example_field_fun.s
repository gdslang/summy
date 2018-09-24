	.file	"example_field_fun.c"
	.text
	.globl	main
	.type	main, @function
main:
.LFB0:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp

  movb $42, %cl
  movl %ecx, %edx
  movl $42, %ecx

  movl $42, %eax
  movl $42, %eax
  movq %rax, %rbx
  addl $1, %eax
  addq $1, %rax
  addl $1, %eax
  callq *%rax
  movl $main, %eax
  movq $main, %rax

	.cfi_def_cfa_register 6
	movl	$0, %eax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	main, .-main
	.ident	"GCC: (Ubuntu 7.3.0-16ubuntu3) 7.3.0"
	.section	.note.GNU-stack,"",@progbits
