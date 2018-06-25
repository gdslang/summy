f:
mov %rax, (%rdx)
ret

main:
je end
mov %rcx, %rdx
end:
call f
ret

