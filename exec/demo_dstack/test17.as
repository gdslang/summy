//loops - box operator

mov $0, %rax
while:
cmp $80, %rax
jge ewhile

//cmp $100, %rax
cmp $70, %rax
jge else
mov $42, %rbx
jmp eite
else:
mov $1, %rbx
jmp eite
eite:

inc %rax
jmp while
ewhile:
jmp end
end:
jmp *%r13
