main:
mov $0, %r11
head:
cmp $100, %r11
jne end
inc %r11
jmp head
end:
ret

