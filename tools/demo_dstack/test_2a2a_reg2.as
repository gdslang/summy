g:
mov %r13, %r14
ret

f:
mov %rcx, %rcx
mov %rdx, %rdx
je else
mov %rcx, %r11
jmp end
else:
mov %rdx, %r11
end:
ret

main:
movq $41000, (%rcx)
movq $42000, (%rdx)
call f
mov (%r11), %r13
ret

