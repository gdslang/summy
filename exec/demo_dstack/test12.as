mov %rax, %rbx

cmp $12, %rax
jge else
nop
jmp end
else:
nop
end:
nop
jmp %r13
