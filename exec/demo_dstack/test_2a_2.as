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
call f
ret

