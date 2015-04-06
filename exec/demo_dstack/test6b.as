movq $0, 8(%rbx)
movq $0, 16(%rbx)
jne else
mov $8, %rax
//mov $15, %rax
jmp end
else:
mov $15, %rax
end:
movq $42, (%rbx,%rax)
ret
