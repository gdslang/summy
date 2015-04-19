.globl foo
.size	foo, 3
lea .foo(%rip), %rbx
add $1, %rbx
movw (%rbx), %ax
nop
.foo:
foo:
nop
add %rax, %rbx
