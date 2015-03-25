mov $99, %rbx
jne else
movq $10, (%rcx)
mov $40, %rax
add %rax, (%rcx)
jmp end
else:
movq $0, (%rcx)
mov $10, %rax
add %rbx, %rax
end:
nop
