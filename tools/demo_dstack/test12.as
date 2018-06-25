mov %rax, %rbx
mov %rbx, %rcx

cmp $12, %rax
jge else
nop
jmp end
else:
nop
end:
nop
jmp *%r13
