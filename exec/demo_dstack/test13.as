//eq domain: strong updates, join
mov %rax, %rbx
mov %rbx, %rcx

cmp $12, %rax
jge else
mov %rbx, %rdx
mov %r11, %r12
jmp eite
else:
mov %r11, %r12
mov %r12, %r13
eite:
jmp end
end:
jmp *%r13
