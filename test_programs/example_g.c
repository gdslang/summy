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
//"mov $98, %rax\n"
"mov $999, %rbx\n"
"cmp $99, %rax\n"
"jne else\n"
"mov $999, %rcx\n"
"jmp after\n"
"else:\n"
"mov $777, %rcx\n"
"after:\n"
"jmp %rbx\n"
);
	end:;

	return 0;
}
