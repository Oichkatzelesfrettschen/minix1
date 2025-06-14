/*<<< WORK-IN-PROGRESS MODERNIZATION HEADER
  This repository is a work in progress to reproduce the
  original MINIX simplicity on modern 32-bit and 64-bit
  ARM and x86/x86_64 hardware using C++23.
>>>*/

/* uniq - compact repeated lines		Author: John Woods */
/*
 * uniq [-udc] [-n] [+n] [infile [outfile]]
 *
 *	Written 02/08/86 by John Woods, placed into public domain.  Enjoy.
 *
 */

/* If the symbol WRITE_ERROR is defined, uniq will exit(1) if it gets a
 * write error on the output.  This is not (of course) how V7 uniq does it,
 * so undefine the symbol if you want to lose your output to a full disk
 */

#define WRITE_ERROR 1
#include "stdio.hpp"
#include <cctype>

FILE *fopen();
char buffer[BUFSIZ];
int uflag = 1; /* default is union of -d and -u outputs */
int dflag = 1; /* flags are mutually exclusive */
int cflag = 0;
int fields = 0;
int chars = 0;

FILE *xfopen(fn, mode)
char *fn, *mode;
{
    FILE *p;
    extern int errno;
    extern char *sys_errlist[];

    if ((p = fopen(fn, mode)) == NULL) {
        perror("uniq");
        fflush(stdout);
        exit(1);
    }
    return (p);
}

// Entry point with modern parameters
int main(int argc, char *argv[]) {
    char *p;
    int inf = -1, outf;

    setbuf(stdout, buffer);
    for (--argc, ++argv; argc > 0 && (**argv == '-' || **argv == '+'); --argc, ++argv) {
        if (**argv == '+')
            chars = atoi(*argv + 1);
        else if (std::isdigit(static_cast<unsigned char>(argv[0][1])))
            fields = atoi(*argv + 1);
        else if (argv[0][1] == '\0')
            inf = 0; /* - is stdin */
        else
            for (p = *argv + 1; *p; p++) {
                switch (*p) {
                case 'd':
                    dflag = 1;
                    uflag = 0;
                    break;
                case 'u':
                    uflag = 1;
                    dflag = 0;
                    break;
                case 'c':
                    cflag = 1;
                    break;
                default:
                    usage();
                }
            }
    }
    /* input file */
    if (argc == 0)
        inf = 0;
    else if (inf == -1) { /* if - was not given */
        fclose(stdin);
        xfopen(*argv++, "r");
        argc--;
    }

    if (argc == 0)
        outf = 1;
    else {
        fclose(stdout);
        xfopen(*argv++, "w");
        argc--;
    }

    uniq();
    fflush(stdout);
    return 0;
}

char *skip(s)
char *s;
{
    int n;

    /* skip fields */
    for (n = fields; n > 0; --n) {
        /* skip blanks */
        while (*s && (*s == ' ' || *s == '\t'))
            s++;
        if (!*s)
            return s;
        while (*s && (*s != ' ' && *s != '\t'))
            s++;
        if (!*s)
            return s;
    }
    /* skip characters */
    for (n = chars; n > 0; --n) {
        if (!*s)
            return s;
        s++;
    }
    return s;
}

int equal(s1, s2)
char *s1, *s2;
{
    return !strcmp(skip(s1), skip(s2));
}

static void show(char *line, int count) {
    if (cflag)
        printf("%4d %s", count, line);
    else {
        if ((uflag && count == 1) || (dflag && count != 1))
            printf("%s", line);
    }
}

/*
 * The meat of the whole affair
 */
char *nowline, *prevline, buf1[1024], buf2[1024];

static int uniq() char *p;
int seen;

/* setup */
prevline = buf1;
if (getline(prevline, 1024) < 0)
    return (0);
seen = 1;
nowline = buf2;

/* get nowline and compare
 * if not equal, dump prevline and swap pointers
 * else continue, bumping seen count
 */
while (getline(nowline, 1024) > 0) {
    if (!equal(prevline, nowline)) {
        show(prevline, seen);
        seen = 1;
        p = nowline;
        nowline = prevline;
        prevline = p;
    } else
        seen += 1;
}
show(prevline, seen);
return 0;
}

[[noreturn]] static void usage() { std_err("Usage: uniq [-udc] [+n] [-n] [input [output]]\n"); }

static int getline(char *buf, int count) {
    char c;
    int ct = 0;

    while (ct++ < count) {
        c = getc(stdin);
        if (c <= 0)
            return (-1);
        *buf++ = c;
        if (c == '\n') {
            *buf++ = 0;
            return (ct);
        }
    }
    return (ct);
}
