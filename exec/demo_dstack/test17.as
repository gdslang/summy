//loops - box operator

mov $0, %rax
while:
cmp $80, %rax
jge ewhile
mov $1, %rbx
inc %rax
jmp while
ewhile:
jmp end
end:
jmp *%r13
