cmp $10, %rax
jge else
cmp $5, %rax
jle else
mov $40, %rbx
jmp end
else:
mov $10, %rbx
end:
ret
