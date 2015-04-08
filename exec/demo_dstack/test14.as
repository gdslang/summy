//eq domain: weak updates
mov (%rax), %rbx
mov %rbx, (%rcx)

jg else
mov %rdx, %r11
jmp eite
else:
mov %rax, %r11
eite:

//weak update using value known to be identical
//mov %rbx, (%r11)
//weak update using unknown value
mov %r12, (%r11)
