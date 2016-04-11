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

#include <stdio.h>
#include <stdint.h>


/*

The purpose of this program is to enumerate CPU support for XSAVE instructions
and XSAVE-supported features.

Intel's CPUID syntax (in the manual):
CPUID.01H:ECX.SSE[bit 25] = 1
means:
Set EAX to 0x01 and execute CPUID
The output register is ECX, and the output is in the 25'th bit, which is called SSE.
And in this case that output is 1.
They might also say something like CPUID.(EAX=n, ECX=n) when you need both EAX and ECX to execute a specific CPUID.

********************************************************************************************

Features/support we want to detect (See Intel pg. Vol. 1 13-3 in Section 13.2):
- General support for the XSAVE feature set:

  CPUID.1:ECX.XSAVE[bit 26] => EAX = 0x01, ECX irrelevant, result in ECX.XSAVE[bit 26]
  If this bit is 0, the processor does not support the XSAVE features, and no further enumeration through CPUID func 0x0d
  If this bit is 1, the processor supports these features: XGETBV, XRSTOR, XSAVE, XSETBV, and further enumeraton through CPUID func 0x0d

- CPUID function 0x0d enumerates details of CPU support through a set of sub-functions.
  Software selects a specific sub-function by the value placed in the ECX register.

- EAX = 0x0d, ECX = 0x00:

  EDX:EAX contains a Bitmap of all user state components that can be managed with XSAVE.
  Locations in this bitmap have the same meaning as those in XCR0.

  Note: If bit i is 1 in this bitmap, you can use CPUID sub-function i to enumerate details for that state component (see below).

  ECX holds the size, in bytes, to store an XSAVE area containing all
  of the user state components supported by the processor.

  EBX holds the size, in bytes, to store an XSAVE area containing all
  of the user state components corresponding to bits currently set in XCR0.

- EAX = 0x0d, ECX = 0x01:

  EAX[0] indicates support for XSAVEOPT. A value of 0 means executing XSAVEOPT results in an invalid opcode exception (#UD)

  EAX[1] indicates support for compaction extensions to XSAVE. If this bit is 1, the compacted format of the extended region
  of the XSAVE area, the XSAVEC instruction, and the compacted form of the XRSTOR function are all supported. Otherwise,
  none of these features are supported and executing XSAVEC causes a #UD.

  EAX[2] indicates support for XGETBV with ECX = 1

  EAX[3] indicates support for XSAVES, XRSTORS, and IA32_XSS MSR. If EAX[3] = 0, XSAVES and XRSTORS
  cause #UD, and RDMSR or WRMSR to IA32_XSS cause #GP.
  Every processor that supports a supervisor state component sets this bit.
  Every processor that sets this bit also supports compaction.

  EAX[31:4] are reserved

  EBX holds the size, in bytes, to store an XSAVE area containing all
  of the state components corresponding to bits currently set in XCR0 | IA32_XSS

  EDX:ECX is a bitmap of all the supervisor state components that
  can be managed by XSAVES and XRSTORS. You can only set a bit in
  IA32_XSS if that bit is also set here.

- EAX = 0x0d, ECX = i > 0x01

  Enumerates details for state component i.

  EAX holds the size, in bytes, required for state component i.

  If i is a user state component, EBX holds the offset, in bytes, from the
  base of the XSAVE area of the section used for the component (this only
  applies to the standard format of the XSAVE area)

  If i is a supervisor state component, EBX is 0

  If i is a user state component, ECX[0] is 0
  If i is a supervisor state component, ECX[0] is 1

  ECX[1] indicates the alignment of the state component in the compacted format
  of the XSAVE area. If ECX[1] is 0, the state component immediately follows the
  previous component in memory. If ECX[1] is 1, the state component starts on the
  next 64-byte boundary following the previous state component.

  ECX[31:2] and EDX are both 0

  If the XSAVE feature set does not support state component i, sub-function i returns
  0 in EAX, EBX, ECX, and EDX.


-------------------------
Reporting format:

Supported:\tFeature:\tInstructions:\n

[YES  ]\XSAVE features\tXRSTOR
[   NO]\tCompaction\tXRSTOR






*/

#define FEATURE_TYPE_GENERAL     "General support"
#define FEATURE_TYPE_INSTRUCTION "Instruction"
#define FEATURE_TYPE_USER_STATE  "User state component"
#define FEATURE_TYPE_SUPERVISOR_STATE "Supervisor state component"

#define SUPPORT_XSAVE         (1 << 26)
#define SUPPORT_XSAVEOPT      (1 << 0)
#define SUPPORT_COMPACTION    (1 << 1)
#define SUPPORT_XGETBV_XINUSE (1 << 2)
#define SUPPORT_XSAVES        (1 << 3)

#define USER_STATE_X87        (1 << 0)
#define USER_STATE_SSE        (1 << 1)
#define USER_STATE_AVX        (1 << 2)
#define USER_STATE_BNDREG     (1 << 3)
#define USER_STATE_BNDCSR     (1 << 4)
#define USER_STATE_OPMASK     (1 << 5)
#define USER_STATE_ZMM_HI256  (1 << 6)
#define USER_STATE_HI16_ZMM   (1 << 7)
#define USER_STATE_PKRU       (1 << 9)

#define SUPERVISOR_STATE_PT (1 << 8)




unsigned int eax, ebx, ecx, edx;


void print_support_header(void) {
  printf("%s\t%-60s\t%s\n", "Supported:", "Feature:", "Feature Type:");
  // printf("Supported:\tFeature:\tFeature Type:\n");
}

void print_support(uint64_t supported, char *feature, char *feature_type) {
  if (supported) { printf("[ YES |    ]"); }
  else {           printf("[     | NO ]"); }
  printf("\t%-60s\t%s\n", feature, feature_type);
}

void print_cpuid_reg_state(void) {
  printf( "CPUID Register State:\n"
          "eax: 0x%x\n"
          "ebx: 0x%x\n"
          "ecx: 0x%x\n"
          "edx: 0x%x\n",
          eax, ebx, ecx, edx);
}

void print_component_details(int index, int size, int offset, int is_supervisor, int byte64_aligned) {
  printf("Component index: %d\n", index);
  printf("Size (bytes): %d\n", size);

  if (is_supervisor) {
    printf("State component type: Supervisor\n");
    // Supervisor state components are, in fact, saved in the extended region of the XSAVE
    // area, but CPUID does not enumerate the offset at which they are stored, because this
    // offset only applies to the standard format of the XSAVE area, and XSAVES always uses
    // the compacted format.
  }
  else {
    printf("State component type: User\n");
    printf("Offset (bytes) from standard XSAVE area's base: %d\n", offset);
  }

  char *alignment;
  if (byte64_aligned) { alignment = "Next 64-byte boundary after previous component."; }
  else {                alignment = "Immediately after previous component."; }
  printf("Compacted format alignment of component: %s\n", alignment);
}

// We can't just use the __get_cpuid function from cpuid.h, because
// it doesn't let you specify the inputs for eax and ecx.
// We use our own version that marks eax and ecx as read/write,
// instead of just write only.
void cpuid(void) {
  __asm__ ( "cpuid\n\t" : "+a" (eax), "=b" (ebx), "+c" (ecx), "=d" (edx));
}


int main() {

  eax = ebx = ecx = edx = 0x00;

  /* General XSAVE support */
  eax = 0x01;
  ecx = 0x00;
  cpuid();

  printf("\n-----------------------------\n"
           "|   General XSAVE support   |\n"
           "-----------------------------\n");
  print_support_header();
  print_support(ecx & SUPPORT_XSAVE, "XSAVE", FEATURE_TYPE_GENERAL);

  // If XSAVE is not supported, just return.
  if (!(ecx & SUPPORT_XSAVE)) {
    printf("Exiting, because XSAVE is not supported on this processor.\n");
    return 0;
  }


  /* User state component support */
  eax = 0x0d;
  ecx = 0x00;
  cpuid();
  uint64_t edx_eax = ((uint64_t)edx << 32) | eax;

  printf("\n------------------------------------------\n"
           "|   User state component XSAVE support   |\n"
           "------------------------------------------\n");
  print_support_header();
  print_support(edx_eax & USER_STATE_X87,       "X87: x87 FPU",                                   FEATURE_TYPE_USER_STATE);
  print_support(edx_eax & USER_STATE_SSE,       "SSE: MXCSR and XMM",                             FEATURE_TYPE_USER_STATE);
  print_support(edx_eax & USER_STATE_AVX,       "AVX: YMM upper halves",                          FEATURE_TYPE_USER_STATE);
  print_support(edx_eax & USER_STATE_BNDREG,    "BNDREG: Bounds registers BND0-BND3",             FEATURE_TYPE_USER_STATE);
  print_support(edx_eax & USER_STATE_BNDCSR,    "BNDCSR: BNDCFGU and BNDSTATUS registers",        FEATURE_TYPE_USER_STATE);
  print_support(edx_eax & USER_STATE_OPMASK,    "opmask: Registers k0-k7",                        FEATURE_TYPE_USER_STATE);
  print_support(edx_eax & USER_STATE_ZMM_HI256, "ZMM_Hi256: Upper halves of lower ZMM registers", FEATURE_TYPE_USER_STATE);
  print_support(edx_eax & USER_STATE_HI16_ZMM,  "Hi16_ZMM: Upper ZMM registers",                  FEATURE_TYPE_USER_STATE);
  print_support(edx_eax & USER_STATE_PKRU,      "PKRU: Protection key feature's register",        FEATURE_TYPE_USER_STATE);

  printf("%d: Size (bytes) of XSAVE area needed by XSAVE instruction for all supported user state components.\n", ecx);
  printf("%d: Size (bytes) of XSAVE area needed by XSAVE instruction for all user state components CURRENTLY set in XCR0.\n", ebx);

  /* XSAVEOPT, XSAVES, Compaction support */
  eax = 0x0d;
  ecx = 0x01;
  cpuid();
  uint64_t edx_ecx = ((uint64_t)edx << 32) | ecx;

  printf("\n--------------------------------------------\n"
           "|   XSAVEOPT, Compaction, XINUSE, XSAVES   |\n"
           "--------------------------------------------\n");
  print_support_header();
  print_support(eax & SUPPORT_XSAVEOPT,      "XSAVEOPT support",                                      FEATURE_TYPE_INSTRUCTION);
  print_support(eax & SUPPORT_COMPACTION,    "Compaction extensions to XSAVE",                        FEATURE_TYPE_INSTRUCTION);
  print_support(eax & SUPPORT_XGETBV_XINUSE, "XGETBV with ECX = 1 to detect init optimization state", FEATURE_TYPE_INSTRUCTION);
  print_support(eax & SUPPORT_XSAVES,        "XSAVES, XRSTORS, and IA32_XSS MSR support",             FEATURE_TYPE_INSTRUCTION);

  // If XSAVES is not supported, this will be 0.
  printf("%d: Size (bytes) of XSAVE area needed by XSAVES instruction for all state components CURRENTLY set in XCR0 | IA32_XSS.\n", ebx);


  /* Supervisor state components: */
  printf("\n------------------------------------------\n"
           "|   Supervisor state component support   |\n"
           "------------------------------------------\n");
  print_support_header();
  print_support(edx_ecx & SUPERVISOR_STATE_PT, "Intel Processor Trace support", FEATURE_TYPE_SUPERVISOR_STATE);

  /* Component-specific details: */

  /*
     We will now enumerate all 64 bits that can possibly represent
     supported state components, and see if anything shows up that
     we don't already know about.
  */
  printf("\n-------------------------------------------------------\n"
           "|   Sanity check / State-component-specific details   |\n"
           "-------------------------------------------------------\n");
  printf("Will now enumerate all 64 bits that can possibly represent supported state components. Read this and see if anything shows up that you don't already know about.\n");
  printf("---\n");

  // The first two components, x87 and SSE, are legacy from FXSAVE and are always supported
  for (int i = 2; i < 64; ++i) {
    eax = 0x0d;
    ecx = i;
    cpuid();
    if ((0 == eax) && (0 == ebx) && (0 == ecx) && (0 == edx)) {
      // This is not a supported state component, skip.
      printf("Component with index %d is not supported by XSAVE on this processor.\n", i);
    }
    else {
        print_component_details(i, // index
                                eax, // size
                                ebx, // offset
                                ecx & (1 << 0), // is_supervisor (else user)
                                ecx & (1 << 1));  // 64-byte aligned
    }
    printf("---\n");

  }


  return 0;
}