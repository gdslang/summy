jne else
mov $16, %rax
jmp end
else:
mov $16, %rax
end:
movq $42, (%rbx,%rax)
ret
