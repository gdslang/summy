main:
mov $0, %r11
head:
cmp $3, %r11
je end

mov (%rax), %rbx
mov %rbx, %rax

inc %r11
jmp head
end:
ret

