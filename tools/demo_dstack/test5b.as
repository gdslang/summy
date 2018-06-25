movq $1, (%rbx)
movq $2, (%rcx)
jne else
mov %rbx, %rax
jmp eite
else:
mov %rcx, %rax
eite:
movq $42, (%rax)
movq (%rax), %rdx
end: nop
