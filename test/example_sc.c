#include <stdio.h>
#include <stdlib.h>

void __exit() {
	exit(0);
}

int main(void) {
	FILE *f = fopen("example.bin", "w");
	if(!f)
		return 1;
	
	for(void *i = &&start; i < &&end; i++)
		fwrite(i, 1, 1, f);

	fclose(f);

	__exit();

	start:
	asm (
"movq $0, %rax\n"
"movq $0, %rbx\n"
//"movw $999, (%rax)\n"
"movw $999, (%rax)\n"
"movw (%rax), %bx\n"
"jmp %rbx\n");
	end:;

	return 0;
}
