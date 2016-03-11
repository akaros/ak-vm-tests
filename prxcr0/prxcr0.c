// #include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

void xgetbv_ecx0(uint32_t *edx, uint32_t *eax) {
  __asm__ ("mov $0, %%ecx\n\t"
           "xgetbv\n\t"
           : "=d" (*edx), "=a" (*eax)
           : /* No inputs */
           : "%ecx" );
}

int main() {
	uint32_t eax, edx;

	xgetbv_ecx0(&edx, &eax);
	printf("edx: 0x%08x\neax: 0x%08x\n", edx, eax);

	return 0;
}