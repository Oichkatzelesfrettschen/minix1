/*<<< WORK-IN-PROGRESS MODERNIZATION HEADER
  This repository is a work in progress to reproduce the
  original MINIX simplicity on modern 32-bit and 64-bit
  ARM and x86/x86_64 hardware using C++23.
>>>*/

#include <cstdlib> // for exit
#include <string_view>
#include <unistd.h> // for read, write
char *table[] = {"push ax",
                 "ret",
                 "mov bp,sp",
                 "push bp",
                 "pop bp",
                 "mov sp,bp",
                 ".text",
                 "xor ax,ax",
                 "push 4(bp)",
                 "pop bx",
                 "pop si",
                 "cbw",
                 "movb al,(bx)",
                 "pop ax",
                 "xorb ah,ah",
                 "mov ax,#1",
                 "call _callm1",
                 "add sp,#16",
                 "mov bx,4(bp)",
                 "push 6(bp)",
                 "mov -2(bp),ax",
                 "I0013:",
                 "call .cppuu",
                 "mov ax,-2(bp)",
                 "add 4(bp),#1",
                 "or ax,ax",
                 "jmp I0011",
                 "mov bx,8(bp)",
                 "push dx",
                 "mov cx,#2",
                 "mov bx,#2",
                 "I0011:",
                 "I0012:",
                 "push -2(bp)",
                 "mov ax,4(bp)",
                 "mov ax,-4(bp)",
                 "add sp,#6",
                 "and ax,#255",
                 "push bx",
                 "mov bx,-2(bp)",
                 "loop 2b",
                 "jcxz 1f",
                 ".word 4112",
                 "mov ax,(bx)",
                 "mov -4(bp),ax",
                 "jmp I0013",
                 ".data",
                 "mov bx,6(bp)",
                 "mov (bx),ax",
                 "je I0012",
                 ".word 8224",
                 ".bss",
                 "mov ax,#2",
                 "call _len",
                 "call _callx",
                 ".word 28494",
                 ".word 0",
                 "push -4(bp)",
                 "movb (bx),al",
                 "mov bx,ax",
                 "mov -2(bp),#0",
                 "I0016:",
                 ".word 514",
                 ".word 257",
                 "mov ",
                 "push ",
                 ".word ",
                 "pop ",
                 "add ",
                 "4(bp)",
                 "-2(bp)",
                 "(bx)",
                 ".define ",
                 ".globl ",
                 "movb ",
                 "xor ",
                 "jmp ",
                 "cmp ",
                 "6(bp)",
                 "-4(bp)",
                 "-6(bp)",
                 "#16",
                 "_callm1",
                 "call ",
                 "8(bp)",
                 "xorb ",
                 "and ",
                 "sub ",
                 "-8(bp)",
                 "jne ",
                 ".cppuu",
                 "lea ",
                 "inc ",
                 "_M+10",
                 "#255",
                 "loop",
                 "jcxz",
                 "ax,#",
                 "bx,#",
                 "cx,#",
                 "ax,",
                 "bx,",
                 "cx,",
                 "dx,",
                 "si,",
                 "di,",
                 "bp,",
                 "ax",
                 "bx",
                 "cx",
                 "dx",
                 "si",
                 "di",
                 "bp",
                 "sp",
                 "dec ",
                 "neg ",
                 "_execve",
                 ",#0",
                 0};

#define IBUFSIZE 10000
#define OBUFSIZE 30000

// Input buffer for packed bytes and output buffer for the unpacked result.
char input[IBUFSIZE + 1];
char output[OBUFSIZE + 1];

// Forward declaration of the unpack routine.
static int unpack88(char *inp, char *outp);

int main() {
    int n;     // number of bytes read from stdin
    int count; // count of bytes produced by unpack88

    while (1) {
        n = read(0, input, IBUFSIZE);
        if (n <= 0)
            exit(1);
        input[n] = 0;
        count = unpack88(input, output);
        n = write(1, output, count);
    }

    return 0; // unreachable
}

// Expand packed assembly opcodes from ``inp`` into ``outp``.
// Returns the number of bytes written to ``outp``.
static int unpack88(char *inp, char *outp) {
    int k;             // Current input byte as an unsigned value
    char *p;           // Pointer for table lookups
    char *orig = outp; // Keep start to compute byte count

    while (*inp != 0) {
        k = *inp & 0377;
        if (k < 128) {
            // Values below 128 are copied directly
            *outp++ = *inp++;
        } else {
            // Higher values index into the expansion table
            p = table[k - 128];
            while (*p != 0) {
                *outp++ = *p++;
            }
            inp++;
        }
    }

    return outp - orig;
}
