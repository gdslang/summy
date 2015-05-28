g:
mov %r13, %r14
ret

f:
//r12 = *r11  --> **r12 = 42
mov (%r11), %r12
//a = r12 -> **a = 42
mov %r12, %rax
ret

main:
//*d = 42
movq $42, (%rdx)
//c = *b
mov (%rbx), %rcx
//**c = 42
mov %rdx, (%rcx)    
//r11 = b --> ***b = 42, ***r11 = 42
mov %rbx, %r11      
call f
//r13 = **a
movq (%rax), %r13   
movq (%r13), %r13

//mov (%rcx), %r12
//movq (%r12), %r13

ret

