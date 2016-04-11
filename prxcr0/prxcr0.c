/*
 * Copyright 2016 Google Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

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