//bottomification and aliasing
mov %rbx, (%rcx)
mov (%rcx), %rax
cmp %rbx, %rax

je else
mov $10, %rbx
add $1, %rbx
jmp eite
else:
mov $20, %rbx
add $1, %rbx
eite:

jmp end
end:
jmp *%r11
