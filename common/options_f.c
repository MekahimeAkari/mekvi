/*      $OpenBSD: options_f.c,v 1.13 2019/05/21 09:24:58 martijn Exp $  */

/* SPDX-License-Identifier: BSD-3-Clause */

/*
 * Copyright (c) 1993, 1994
 *      The Regents of the University of California.  All rights reserved.
 * Copyright (c) 1993, 1994, 1995, 1996
 *      Keith Bostic.  All rights reserved.
 * Copyright (c) 2022-2023 Jeffrey H. Johnson <trnsz@pobox.com>
 *
 * See the LICENSE.md file for redistribution information.
 */

#include "../include/compat.h"

#include <sys/types.h>
#include <sys/queue.h>
#include <sys/stat.h>

#include <bitstring.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <bsd_stdlib.h>
#include <bsd_string.h>
#include <bsd_unistd.h>

#include "common.h"

#undef open

/*
 * PUBLIC: int f_altwerase(SCR *, OPTION *, char *, unsigned long *);
 */

int
f_altwerase(SCR *sp, OPTION *op, char *str, unsigned long *valp)
{
        if (!*valp)
                O_CLR(sp, O_TTYWERASE);
        return (0);
}

/*
 * PUBLIC: int f_columns(SCR *, OPTION *, char *, unsigned long *);
 */

int
f_columns(SCR *sp, OPTION *op, char *str, unsigned long *valp)
{
        /* Validate the number. */
        if (*valp < MINIMUM_SCREEN_COLS) {
                msgq(sp, M_ERR, "Screen columns too small, less than %d",
                    MINIMUM_SCREEN_COLS);
                return (1);
        }

        /*
         * !!!
         * It's not uncommon for allocation of huge chunks of memory to cause
         * core dumps on various systems.  So, we prune out numbers that are
         * "obviously" wrong.  Vi will not work correctly if it has the wrong
         * number of lines/columns for the screen, but at least we don't drop
         * core.
         */

#define MAXIMUM_SCREEN_COLS     3640
        if (*valp > MAXIMUM_SCREEN_COLS) {
                msgq(sp, M_ERR, "Screen columns too large, greater than %d",
                    MAXIMUM_SCREEN_COLS);
                return (1);
        }
        return (0);
}

/*
 * PUBLIC: int f_lines(SCR *, OPTION *, char *, unsigned long *);
 */

int
f_lines(SCR *sp, OPTION *op, char *str, unsigned long *valp)
{
        /* Validate the number. */
        if (*valp < MINIMUM_SCREEN_ROWS) {
                msgq(sp, M_ERR, "Screen lines too small, less than %d",
                    MINIMUM_SCREEN_ROWS);
                return (1);
        }

        /*
         * !!!
         * It's not uncommon for allocation of huge chunks of memory to cause
         * core dumps on various systems.  So, we prune out numbers that are
         * "obviously" wrong.  Vi will not work correctly if it has the wrong
         * number of lines/columns for the screen, but at least we don't drop
         * core.
         */

#define MAXIMUM_SCREEN_ROWS     2048
        if (*valp > MAXIMUM_SCREEN_ROWS) {
                msgq(sp, M_ERR, "Screen lines too large, greater than %d",
                    MAXIMUM_SCREEN_ROWS);
                return (1);
        }

        /*
         * Set the value, and the related scroll value.  If no window
         * value set, set a new default window.
         */

        o_set(sp, O_LINES, 0, NULL, *valp);
        if (*valp == 1) {
                sp->defscroll = 1;

                if (O_VAL(sp, O_WINDOW) == O_D_VAL(sp, O_WINDOW) ||
                    O_VAL(sp, O_WINDOW) > *valp) {
                        o_set(sp, O_WINDOW, 0, NULL, 1);
                        o_set(sp, O_WINDOW, OS_DEF, NULL, 1);
                }
        } else {
                sp->defscroll = (*valp - 1) / 2;

                if (O_VAL(sp, O_WINDOW) == O_D_VAL(sp, O_WINDOW) ||
                    O_VAL(sp, O_WINDOW) > *valp) {
                        o_set(sp, O_WINDOW, 0, NULL, *valp - 1);
                        o_set(sp, O_WINDOW, OS_DEF, NULL, *valp - 1);
                }
        }
        return (0);
}

/*
 * PUBLIC: int f_paragraph(SCR *, OPTION *, char *, unsigned long *);
 */

int
f_paragraph(SCR *sp, OPTION *op, char *str, unsigned long *valp)
{
        if (strlen(str) & 1) {
                msgq(sp, M_ERR,
                    "The paragraph option must be in two character groups");
                return (1);
        }
        return (0);
}

/*
 * PUBLIC: int f_print(SCR *, OPTION *, char *, unsigned long *);
 */

int
f_print(SCR *sp, OPTION *op, char *str, unsigned long *valp)
{
        /* Reinitialize the key fast lookup table. */
        v_key_ilookup(sp);

        /* Reformat the screen. */
        F_SET(sp, SC_SCR_REFORMAT);
        return (0);
}

/*
 * PUBLIC: int f_readonly(SCR *, OPTION *, char *, unsigned long *);
 */

int
f_readonly(SCR *sp, OPTION *op, char *str, unsigned long *valp)
{
        /*
         * !!!
         * See the comment in exf.c.
         */
        if (*valp)
                F_CLR(sp, SC_READONLY);
        else
                F_SET(sp, SC_READONLY);
        return (0);
}

/*
 * PUBLIC: int f_recompile(SCR *, OPTION *, char *, unsigned long *);
 */

int
f_recompile(SCR *sp, OPTION *op, char *str, unsigned long *valp)
{
        if (F_ISSET(sp, SC_RE_SEARCH)) {
                regfree(&sp->re_c);
                F_CLR(sp, SC_RE_SEARCH);
        }
        if (F_ISSET(sp, SC_RE_SUBST)) {
                regfree(&sp->subre_c);
                F_CLR(sp, SC_RE_SUBST);
        }
        return (0);
}

/*
 * PUBLIC: int f_reformat(SCR *, OPTION *, char *, unsigned long *);
 */

int
f_reformat(SCR *sp, OPTION *op, char *str, unsigned long *valp)
{
        F_SET(sp, SC_SCR_REFORMAT);
        return (0);
}

/*
 * PUBLIC: int f_section(SCR *, OPTION *, char *, unsigned long *);
 */

int
f_section(SCR *sp, OPTION *op, char *str, unsigned long *valp)
{
        if (strlen(str) & 1) {
                msgq(sp, M_ERR,
                    "The section option must be in two character groups");
                return (1);
        }
        return (0);
}

/*
 * PUBLIC: int f_secure(SCR *, OPTION *, char *, unsigned long *)
 */

int
f_secure(SCR *sp, OPTION *op, char *str, unsigned long *valp)
{
        if (openbsd_pledge(
            "stdio rpath wpath cpath fattr flock getpw tty", NULL) == -1) {
                msgq(sp, M_ERR, "pledge failed");
                return (1);
        }
        return (0);
}

/*
 * PUBLIC: int f_ttywerase(SCR *, OPTION *, char *, unsigned long *);
 */

int
f_ttywerase(SCR *sp, OPTION *op, char *str, unsigned long *valp)
{
        if (!*valp)
                O_CLR(sp, O_ALTWERASE);
        return (0);
}

/*
 * PUBLIC: int f_w300(SCR *, OPTION *, char *, unsigned long *);
 */

int
f_w300(SCR *sp, OPTION *op, char *str, unsigned long *valp)
{
        unsigned long v;

        /* Historical behavior for w300 was < 1200. */
        if (sp->gp->scr_baud(sp, &v))
                return (1);
        if (v >= 1200)
                return (0);

        return (f_window(sp, op, str, valp));
}

/*
 * PUBLIC: int f_w1200(SCR *, OPTION *, char *, unsigned long *);
 */

int
f_w1200(SCR *sp, OPTION *op, char *str, unsigned long *valp)
{
        unsigned long v;

        /* Historical behavior for w1200 was == 1200. */
        if (sp->gp->scr_baud(sp, &v))
                return (1);
        if (v < 1200 || v > 4800)
                return (0);

        return (f_window(sp, op, str, valp));
}

/*
 * PUBLIC: int f_w9600(SCR *, OPTION *, char *, unsigned long *);
 */

int
f_w9600(SCR *sp, OPTION *op, char *str, unsigned long *valp)
{
        unsigned long v;

        /* Historical behavior for w9600 was > 1200. */
        if (sp->gp->scr_baud(sp, &v))
                return (1);
        if (v <= 4800)
                return (0);

        return (f_window(sp, op, str, valp));
}

/*
 * PUBLIC: int f_window(SCR *, OPTION *, char *, unsigned long *);
 */

int
f_window(SCR *sp, OPTION *op, char *str, unsigned long *valp)
{
        if (*valp >= O_VAL(sp, O_LINES) - 1 &&
            (*valp = O_VAL(sp, O_LINES) - 1) == 0)
                *valp = 1;
        return (0);
}

/*
 * PUBLIC: int f_imctrl __P((SCR *, OPTION *, char *, unsigned long *));
 */

int
f_imctrl(SCR *sp, OPTION *op, char *str, unsigned long *valp)
{

        if (*valp)
                sp->gp->scr_imctrl(sp, IMCTRL_INIT);
        return (0);
}
