//bottomification
mov $10, %rax
cmp $0, %rax

//jg else
jle else
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
