jne else
mov $40, %rax
jmp end
else:
mov $10, %rax
end:
nop
