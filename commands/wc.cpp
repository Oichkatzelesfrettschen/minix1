/*<<< WORK-IN-PROGRESS MODERNIZATION HEADER
  This repository is a work in progress to reproduce the
  original MINIX simplicity on modern 32-bit and 64-bit
  ARM and x86/x86_64 hardware using C++23.
>>>*/

/* wc - count lines, words and characters	Author: David Messer */

#include "stdio.hpp"
#include <cctype>

/*
 *
 *	Usage:  wc [-lwc] [names]
 *
 *		Flags:
 *			l - count lines.
 *			w - count words.
 *			c - count characters.
 *
 *		Flags l, w, and c are default.
 *		Words are delimited by any non-alphabetic character.
 *
 *  Released into the PUBLIC-DOMAIN 02/10/86
 *
 *	If you find this program to be of use to you, a donation of
 *	whatever you think it is worth will be cheerfully accepted.
 *
 *	Written by: David L. Messer
 *				P.O. Box 19130, Mpls, MN,  55119
 *      Program (heavily) modified by Andy Tanenbaum
 */

int lflag; /* Count lines */
int wflag; /* Count words */
int cflag; /* Count characters */

long lcount; /* Count of lines */
long wcount; /* Count of words */
long ccount; /* Count of characters */

long ltotal; /* Total count of lines */
long wtotal; /* Total count of words */
long ctotal; /* Total count of characters */

// Entry point with modern parameters
int main(int argc, char *argv[]) {
    int k;
    char *cp;
    int tflag, files;
    int i;

    /* Get flags. */
    files = argc - 1;
    k = 1;
    cp = argv[1];
    if (*cp++ == '-') {
        files--;
        k++; /* points to first file */
        while (*cp != 0) {
            switch (*cp) {
            case 'l':
                lflag++;
                break;
            case 'w':
                wflag++;
                break;
            case 'c':
                cflag++;
                break;
            default:
                usage();
            }
            cp++;
        }
    }

    /* If no flags are set, treat as wc -lwc. */
    if (!lflag && !wflag && !cflag) {
        lflag = 1;
        wflag = 1;
        cflag = 1;
    }

    /* Process files. */
    tflag = files >= 2; /* set if # files > 1 */

    /* Check to see if input comes from std input. */
    if (k >= argc) {
        static void count();
        if (lflag)
            printf(" %6D", lcount);
        if (wflag)
            printf(" %6D", wcount);
        if (cflag)
            printf(" %6D", ccount);
        printf(" \n");
        fflush(stdout);
        return 0;
    }

    /* There is an explicit list of files.  Loop on files. */
    while (k < argc) {
        fclose(stdin);
        if (fopen(argv[k], "r") == NULL) {
            std_err("wc: cannot open ");
            std_err(argv[k]);
            std_err("\n");
            k++;
            continue;
        } else {
            /* Next file has been opened as std input. */
            count();
            if (lflag)
                printf(" %6D", lcount);
            if (wflag)
                printf(" %6D", wcount);
            if (cflag)
                printf(" %6D", ccount);
            printf(" %s\n", argv[k]);
        }
        k++;
    }

    if (tflag) {
        if (lflag)
            printf(" %6D", ltotal);
        if (wflag)
            printf(" %6D", wtotal);
        if (cflag)
            printf(" %6D", ctotal);
        printf(" total\n");
    }

    fflush(stdout);
    return 0;
}

static void count() {
    register int c;
    register int word = 0;

    lcount = 0;
    wcount = 0;
    ccount = 0L;

    while ((c = getc(stdin)) > 0) {
        ccount++;

        if (std::isspace(static_cast<unsigned char>(c))) {
            if (word)
                wcount++;
            word = 0;
        } else {
            word = 1;
        }

        if (c == '\n' || c == '\f')
            lcount++;
    }
    ltotal += lcount;
    wtotal += wcount;
    ctotal += ccount;
}

[[noreturn]] static void usage() {
    std_err("Usage: wc [-lwc] [name ...]\n");
    exit(1);
}
