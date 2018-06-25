cmp $10, %rax
jge else
cmp $5, %rax
jle else
mov $40, %rbx
jmp eite
else:
mov $10, %rbx
eite:
jmp end
end:
ret
