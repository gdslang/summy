.text

.byte 0x42

f:
mov $42, %rax
ret

g:
mov $99, %rax
ret

//.globl h
h:
call *%rdi
ret

.globl	main
.type	main, @function
main:
je else
leaq f(%rip), %rdi
jmp end
else:
leaq g(%rip), %rdi
end:
call h
leaq f(%rip), %rdi
call h
ret
