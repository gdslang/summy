g:
mov %r13, %r14
ret

f:
mov %rax, (%rdx)
ret

main:
je end
mov %rcx, %rdx
end:
add $8, %rdx
call f
ret

