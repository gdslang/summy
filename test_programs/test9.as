movb $99, (%rbx)
movb $100, 1(%rbx)

jne else
mov %rbx, %rax
jmp end
else:
mov %rbx, %rax
add $1, %rax
end:

movb (%rax), %cl
addb $42, (%rax)

ret
