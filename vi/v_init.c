/*      $OpenBSD: v_init.c,v 1.8 2017/04/18 01:45:35 deraadt Exp $      */

/* SPDX-License-Identifier: BSD-3-Clause */

/*
 * Copyright (c) 1992, 1993, 1994
 *      The Regents of the University of California.  All rights reserved.
 * Copyright (c) 1992, 1993, 1994, 1995, 1996
 *      Keith Bostic.  All rights reserved.
 * Copyright (c) 2022-2023 Jeffrey H. Johnson <trnsz@pobox.com>
 *
 * See the LICENSE.md file for redistribution information.
 */

#include <sys/types.h>
#include <sys/queue.h>
#include <sys/time.h>

#include <bitstring.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <bsd_stdlib.h>
#include <bsd_string.h>

#include "../common/common.h"
#include "vi.h"

/*
 * v_screen_copy --
 *      Copy vi screen.
 *
 * PUBLIC: int v_screen_copy(SCR *, SCR *);
 */
int
v_screen_copy(SCR *orig, SCR *sp)
{
        VI_PRIVATE *ovip, *nvip;

        /* Create the private vi structure. */
        CALLOC_RET(orig, nvip, 1, sizeof(VI_PRIVATE));
        sp->vi_private = nvip;

        /* Invalidate the line size cache. */
        VI_SCR_CFLUSH(nvip);

        if (orig == NULL) {
                nvip->csearchdir = CNOTSET;
        } else {
                ovip = VIP(orig);

                /* User can replay the last input, but nothing else. */
                if (ovip->rep_len != 0) {
                        MALLOC_RET(orig, nvip->rep, ovip->rep_len);
                        memmove(nvip->rep, ovip->rep, ovip->rep_len);
                        nvip->rep_len = ovip->rep_len;
                }

                /* Copy the paragraph/section information. */
                if (ovip->ps != NULL && (nvip->ps =
                    v_strdup(sp, ovip->ps, strlen(ovip->ps))) == NULL)
                        return (1);

                nvip->lastckey = ovip->lastckey;
                nvip->csearchdir = ovip->csearchdir;

                nvip->srows = ovip->srows;
        }
        return (0);
}

/*
 * v_screen_end --
 *      End a vi screen.
 *
 * PUBLIC: int v_screen_end(SCR *);
 */
int
v_screen_end(SCR *sp)
{
        VI_PRIVATE *vip;

        if ((vip = VIP(sp)) == NULL)
                return (0);
        free(vip->keyw);
        free(vip->rep);
        free(vip->ps);
        free(HMAP);
        free(vip);
        sp->vi_private = NULL;

        return (0);
}

/*
 * v_optchange --
 *      Handle change of options for vi.
 *
 * PUBLIC: int v_optchange(SCR *, int, char *, unsigned long *);
 */
int
v_optchange(SCR *sp, int offset, char *str, unsigned long *valp)
{
        switch (offset) {
        case O_PARAGRAPHS:
                return (v_buildps(sp, str, O_STR(sp, O_SECTIONS)));
        case O_SECTIONS:
                return (v_buildps(sp, O_STR(sp, O_PARAGRAPHS), str));
        case O_WINDOW:
                return (vs_crel(sp, *valp));
        }
        return (0);
}
