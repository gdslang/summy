.globl foo
.size	foo, 2
lea .foo(%rip), %rbx
add $1, %rbx
movb (%rbx), %al
nop
.foo:
foo:
nop
