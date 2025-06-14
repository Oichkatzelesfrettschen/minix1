/*<<< WORK-IN-PROGRESS MODERNIZATION HEADER
  This repository is a work in progress to reproduce the
  original MINIX simplicity on modern 32-bit and 64-bit
  ARM and x86/x86_64 hardware using C++23.
>>>*/

/* od - octal dump	   Author: Andy Tanenbaum */

#include "stdio.hpp"

int bflag, cflag, dflag, oflag, xflag, hflag, linenr, width, state, ever;
int prevwds[8];
long off;
char buf[512], buffer[BUFSIZ];
int next;
int bytespresent;
char hexit();

// Entry point with modern parameters
int main(int argc, char *argv[]) {
    int k, flags;
    long offset();
    char *p;

    /* Process flags */
    setbuf(stdout, buffer);
    flags = 0;
    p = argv[1];
    if (*p == '-') {
        /* Flags present. */
        flags++;
        p++;
        while (*p) {
            switch (*p) {
            case 'b':
                bflag++;
                break;
            case 'c':
                cflag++;
                break;
            case 'd':
                dflag++;
                break;
            case 'h':
                hflag++;
                break;
            case 'o':
                oflag++;
                break;
            case 'x':
                xflag++;
                break;
            default:
                usage();
            }
            p++;
        }
    } else {
        oflag = 1;
    }
    if ((bflag | cflag | dflag | oflag | xflag) == 0)
        oflag = 1;
    k = (flags ? 2 : 1);
    if (bflag | cflag) {
        width = 8;
    } else if (oflag) {
        width = 7;
    } else if (dflag) {
        width = 6;
    } else {
        width = 5;
    }

    /* Process file name, if any. */
    p = argv[k];
    if (k < argc && *p != '+') {
        /* Explicit file name given. */
        close(0);
        if (open(argv[k], 0) != 0) {
            std_err("od: cannot open ");
            std_err(argv[k]);
            std_err("\n");
            fflush(stdout);
            exit(1);
        }
        k++;
    }

    /* Process offset, if any. */
    if (k < argc) {
        /* Offset present. */
        off = offset(argc, argv, k);
        off = (off / 16L) * 16L;
        lseek(0, off, 0);
    }

    dumpfile();
    addrout(off);
    printf("\n");
    fflush(stdout);
    exit(0);
}

long offset(argc, argv, k)
int argc;
char *argv[];
int k;
{
    int dot, radix;
    char str[80], *p, c;
    long val;

    /* See if the offset is decimal. */
    dot = 0;
    p = argv[k];
    while (*p)
        if (*p++ == '.')
            dot = 1;

    /* Convert offset to binary. */
    radix = (dot ? 10 : 8);
    val = 0;
    p = argv[k];
    if (*p == '+')
        p++;
    while (*p != 0 && *p != '.') {
        c = *p++;
        if (c < '0' || c > '9') {
            printf("Bad character in offset: %c\n", c);
            fflush(stdout);
            exit(1);
        }
        val = radix * val + c - '0';
    }

    p = argv[k + 1];
    if (k + 1 == argc - 1 && *p == 'b')
        val = 512L * val;
    return (val);
}

dumpfile() {
    int k;
    int *words;

    while ((k = getwords(&words))) { /* 'k' is # bytes read */
        if (k == 16 && same(words, prevwds) && ever == 1) {
            if (state == 0) {
                printf("*\n");
                state = 1;
                off += 16;
                continue;
            } else if (state == 1) {
                off += 16;
                continue;
            }
        }

        addrout(off);
        off += k;
        state = 0;
        ever = 1;
        linenr = 1;
        if (oflag)
            wdump(words, k, 8);
        if (dflag)
            wdump(words, k, 10);
        if (xflag)
            wdump(words, k, 16);
        if (cflag)
            bdump(words, k, 'c');
        if (bflag)
            bdump(words, k, 'b');
        for (k = 0; k < 8; k++)
            prevwds[k] = words[k];
        for (k = 0; k < 8; k++)
            words[k] = 0;
    }
}

wdump(words, k, radix) int words[8], k, radix;
{
    int i;

    if (linenr++ != 1)
        printf("       ");
    for (i = 0; i < (k + 1) / 2; i++)
        outword(words[i], radix);
    printf("\n");
}

bdump(words, k, c) int words[8];
int k;
char c;
{
    int i;
    int c1, c2;

    i = 0;
    if (linenr++ != 1)
        printf("       ");
    while (i < k) {
        c1 = words[i >> 1] & 0377;
        c2 = (words[i >> 1] >> 8) & 0377;
        byte(c1, c);
        i++;
        if (i == k) {
            printf("\n");
            return;
        }
        byte(c2, c);
        i++;
    }
    printf("\n");
}

byte(val, c) int val;
char c;
{
    if (c == 'b') {
        printf(" ");
        outnum(val, 7);
        return;
    }

    if (val == 0)
        printf("  \\0");
    else if (val == '\b')
        printf("  \\b");
    else if (val == '\f')
        printf("  \\f");
    else if (val == '\n')
        printf("  \\n");
    else if (val == '\r')
        printf("  \\r");
    else if (val == '\t')
        printf("  \\t");
    else if (val >= ' ' && val < 0177)
        printf("   %c", val);
    else {
        printf(" ");
        outnum(val, 7);
    }
}

int getwords(words)
int **words;
{
    int count;

    if (next >= bytespresent) {
        bytespresent = read(0, buf, 512);
        next = 0;
    }
    if (next >= bytespresent)
        return (0);
    *words = (int *)&buf[next];
    if (next + 16 <= bytespresent)
        count = 16;
    else
        count = bytespresent - next;

    next += count;
    return (count);
}

int same(w1, w2)
int *w1, *w2;
{
    int i;
    i = 8;
    while (i--)
        if (*w1++ != *w2++)
            return (0);
    return (1);
}

outword(val, radix) int val, radix;
{
    /* Output 'val' in 'radix' in a field of total size 'width'. */

    int i;

    if (radix == 16)
        i = width - 4;
    if (radix == 10)
        i = width - 5;
    if (radix == 8)
        i = width - 6;
    if (i == 1)
        printf(" ");
    else if (i == 2)
        printf("  ");
    else if (i == 3)
        printf("   ");
    else if (i == 4)
        printf("    ");
    outnum(val, radix);
}

outnum(num, radix) int num, radix;
{
    /* Output a number with all leading 0s present.  Octal is 6 places,
     * decimal is 5 places, hex is 4 places.
     */
    int d, i;
    unsigned val;
    char s[8];

    val = (unsigned)num;
    if (radix == 8)
        d = 6;
    else if (radix == 10)
        d = 5;
    else if (radix == 16)
        d = 4;
    else if (radix == 7) {
        d = 3;
        radix = 8;
    }

    for (i = 0; i < d; i++) {
        s[i] = val % radix;
        val -= s[i];
        val = val / radix;
    }
    for (i = d - 1; i >= 0; i--) {
        if (s[i] > 9)
            printf("%c", 'a' + s[i] - 10);
        else
            printf("%c", s[i] + '0');
    }
}

addrout(l) long l;
{
    int i;

    if (hflag == 0) {
        for (i = 0; i < 7; i++)
            printf("%c", ((l >> (18 - 3 * i)) & 07) + '0');
    } else {
        for (i = 0; i < 7; i++)
            printf("%c", hexit(((l >> (24 - 4 * i)) & 0x0F)));
    }
}

char hexit(k)
int k;
{
    if (k <= 9)
        return ('0' + k);
    else
        return ('A' + k - 10);
}

usage() { std_err("Usage: od [-bcdhox] [file] [ [+] offset [.] [b] ]"); }
