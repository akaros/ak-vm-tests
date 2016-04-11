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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static inline unsigned int min(unsigned int a, unsigned int b) {
  if (a < b) {
    return a;
  }
  return b;
}

// Rudimentary hex dumper. Misses some corner cases on
// certain ascii values but good enough for our purposes.
void hex_dump(void *mem, size_t size) {
  // Prints 16 byte lines as space-separated hex pairs
  int i = size;
  int line_i = 0;
  unsigned char *next = mem;
  unsigned int print_ascii = 0;
  unsigned int line_len = min(16, size);

  while(i) {


    if (print_ascii) {
      if ('\a' == *next)      { printf("\\a"); }
      else if ('\b' == *next) { printf("\\b"); }
      else if ('\f' == *next) { printf("\\f"); }
      else if ('\n' == *next) { printf("\\n"); }
      else if ('\r' == *next) { printf("\\r"); }
      else if ('\t' == *next) { printf("\\t"); }
      else if ('\v' == *next) { printf("\\v"); }
      else if ('\\' == *next) { printf("\\ "); }
      else if ('\'' == *next) { printf("\' "); }
      else if ('\"' == *next) { printf("\" "); }
      else if ('\?' == *next) { printf("\? "); }
      else { printf("%c ", *next); }
    }
    else {
      // Print two bytes and a space
      if (0x00 == *next) { printf("-- "); }
      else               { printf("%02x ", *next); }
    }
    // Manipulate counters
    i--;
    line_i++;
    next +=1;

    if (line_len == line_i) { // we just printed the end of a line
      line_i = 0;
      if (print_ascii) { // we just printed the last ascii char of a line
        print_ascii = 0;
        printf("\n");
      }
      else { // we just printed the last hex byte of a line
        print_ascii = 1;
        // now we're going to print the line again, but in ascii
        next -= line_len;
        i += line_len;
      }
    }
  }

  printf("\n");

}


// REX_PREFIX 0x48 = 01001000 means that we have a 64 bit operand size
//                   0100WR0B
// Opcode 0x0f,0xae is xsaveopt
// ModRM 0x37 means [EDI]
#define REX_PREFIX "0x48, "
#define XSAVEOPT_64 ".byte " REX_PREFIX "0x0f,0xae,0x37\n\t"

void populate_FP_state (void) {
  // Put stuff in the floating point registers, and we'll see if when we xsaveopt it gets saved

  /*
  See Intel Vol. 1 13-6 for where stuff shows up. Note that
  this will be flipped in the hex dump beacause little endian.
  */


  /* X87: x87 FPU */
  // FINIT sets the following:
  // FPU Control Word: 0x037f
  // FPU Status Word: 0
  // FPU Tag Word: 0xffff
  // FPU Data Pointer: 0
  // FPU Instruction Pointer: 0
  // FPU Last Instruction Opcode: 0
  __asm__ __volatile__ ("finit");

  // Data registers (MMX registers are aliased to these)
  // Each of these strings is 8 bytes long (excluding terminating \0).
  // You'll see some extra room in the hexdump, that's partially for
  // exponents because these data registers are aliased to the lower
  // 64 bits of the ST0-ST7 floating point registers, and partially
  // reserved.
  char *mm0 = "|_MM:0_|";
  char *mm1 = "|_MM:1_|";
  char *mm2 = "|_MM:2_|";
  char *mm3 = "|_MM:3_|";
  char *mm4 = "|_MM:4_|";
  char *mm5 = "|_MM:5_|";
  char *mm6 = "|_MM:6_|";
  char *mm7 = "|_MM:7_|";

  __asm__("movq (%0), %%mm0" : /* No Outputs */ : "r" (mm0) : "%mm0");
  __asm__("movq (%0), %%mm1" : /* No Outputs */ : "r" (mm1) : "%mm1");
  __asm__("movq (%0), %%mm2" : /* No Outputs */ : "r" (mm2) : "%mm2");
  __asm__("movq (%0), %%mm3" : /* No Outputs */ : "r" (mm3) : "%mm3");
  __asm__("movq (%0), %%mm4" : /* No Outputs */ : "r" (mm4) : "%mm4");
  __asm__("movq (%0), %%mm5" : /* No Outputs */ : "r" (mm5) : "%mm5");
  __asm__("movq (%0), %%mm6" : /* No Outputs */ : "r" (mm6) : "%mm6");
  __asm__("movq (%0), %%mm7" : /* No Outputs */ : "r" (mm7) : "%mm7");

  /* SSE: MXCSR and YMM/XMM registers */

  // MXCSR is 32 bits, but the high 16 are reserved
  // so we only set the low 16 here.
  char *mxcsr = "MX\0\0";
  __asm__("ldmxcsr (%0)" : /* No Outputs */ : "r" (mxcsr));

  // Each of these strings is 32 bytes long, excluding the terminating \0.
  char *ymm0  = "|____XMM:00____||_YMM_Hi128:00_|";
  char *ymm1  = "|____XMM:01____||_YMM_Hi128:01_|";
  char *ymm2  = "|____XMM:02____||_YMM_Hi128:02_|";
  char *ymm3  = "|____XMM:03____||_YMM_Hi128:03_|";
  char *ymm4  = "|____XMM:04____||_YMM_Hi128:04_|";
  char *ymm5  = "|____XMM:05____||_YMM_Hi128:05_|";
  char *ymm6  = "|____XMM:06____||_YMM_Hi128:06_|";
  char *ymm7  = "|____XMM:07____||_YMM_Hi128:07_|";
  char *ymm8  = "|____XMM:08____||_YMM_Hi128:08_|";
  char *ymm9  = "|____XMM:09____||_YMM_Hi128:09_|";
  char *ymm10 = "|____XMM:10____||_YMM_Hi128:10_|";
  char *ymm11 = "|____XMM:11____||_YMM_Hi128:11_|";
  char *ymm12 = "|____XMM:12____||_YMM_Hi128:12_|";
  char *ymm13 = "|____XMM:13____||_YMM_Hi128:13_|";
  char *ymm14 = "|____XMM:14____||_YMM_Hi128:14_|";
  char *ymm15 = "|____XMM:15____||_YMM_Hi128:15_|";

  // Populate YMMs (15 regs on 64 bit) with AVX version of movdqu (vmovdqu)
  // Unfortunately, gcc doesn't recognize ymm as a register for clobbers,
  // so we have to just stick with xmm there.
  __asm__("vmovdqu (%0), %%ymm0" : /* No Outputs */ : "r" (ymm0) : "%xmm0");
  __asm__("vmovdqu (%0), %%ymm1" : /* No Outputs */ : "r" (ymm1) : "%xmm1");
  __asm__("vmovdqu (%0), %%ymm2" : /* No Outputs */ : "r" (ymm2) : "%xmm2");
  __asm__("vmovdqu (%0), %%ymm3" : /* No Outputs */ : "r" (ymm3) : "%xmm3");
  __asm__("vmovdqu (%0), %%ymm4" : /* No Outputs */ : "r" (ymm4) : "%xmm4");
  __asm__("vmovdqu (%0), %%ymm5" : /* No Outputs */ : "r" (ymm5) : "%xmm5");
  __asm__("vmovdqu (%0), %%ymm6" : /* No Outputs */ : "r" (ymm6) : "%xmm6");
  __asm__("vmovdqu (%0), %%ymm7" : /* No Outputs */ : "r" (ymm7) : "%xmm7");

  __asm__("vmovdqu (%0), %%ymm8"  : /* No Outputs */ : "r" (ymm8)  : "%xmm8");
  __asm__("vmovdqu (%0), %%ymm9"  : /* No Outputs */ : "r" (ymm9)  : "%xmm9");
  __asm__("vmovdqu (%0), %%ymm10" : /* No Outputs */ : "r" (ymm10) : "%xmm10");
  __asm__("vmovdqu (%0), %%ymm11" : /* No Outputs */ : "r" (ymm11) : "%xmm11");
  __asm__("vmovdqu (%0), %%ymm12" : /* No Outputs */ : "r" (ymm12) : "%xmm12");
  __asm__("vmovdqu (%0), %%ymm13" : /* No Outputs */ : "r" (ymm13) : "%xmm13");
  __asm__("vmovdqu (%0), %%ymm14" : /* No Outputs */ : "r" (ymm14) : "%xmm14");
  __asm__("vmovdqu (%0), %%ymm15" : /* No Outputs */ : "r" (ymm15) : "%xmm15");


}

void xgetbv_ecx0(uint32_t *edx, uint32_t *eax) {
  __asm__ ("mov $0, %%ecx\n\t"
           "xgetbv\n\t"
           : "=d" (*edx), "=a" (*eax)
           : /* No inputs */
           : "%ecx" );
}

void xsaveopt_64(uint32_t edx, uint32_t eax, void *xsave_area) {
  __asm__ ("mov %0, %%edx\n\t"
           "mov %1, %%eax\n\t"
           "movq %2, %%rdi\n\t"
           //XSAVEOPT_64 // xsaveopt64
           "xsaveopt64 (%%rdi)\n\t" // both of these work. you need the parens around rdi (means indirect)
           :  /* No Outputs */
           : "r" (edx), "r" (eax), "r" (xsave_area)
           : "%edx", "%eax", "%rdi", "memory" );
}

int main() {
  uint32_t edx = 0x0;
  uint32_t eax = 0x7;


  // Location to save to:
  // My xsave_detect.c revealed that 832 bytes is the
  // max size to save state components on this system.
  // Better hope this is aligned to a 64-byte boundary...
  // ... or we'll get a GP fault.
  size_t xsave_area_size = 832;
  void *xsave_area;
  if (posix_memalign(&xsave_area, 64, xsave_area_size)) return 1; // This should do the alignment...
  memset(xsave_area, 0x00, 832);

  printf("\nDumping the contents of the xsave_area prior to saving (should have every byte set to 0x11):\n");
  hex_dump(xsave_area, xsave_area_size);

  printf("\nPopulating fp registers and calling xsaveopt with edx:eax=0x%x:%x, xsave_area addr=0x%p\n", edx, eax, xsave_area);
  populate_FP_state();
  xsaveopt_64(edx, eax, xsave_area);
  printf("\nDumping the contents of the xsave_area:\n");
  hex_dump(xsave_area, xsave_area_size);
}