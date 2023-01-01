/*      $OpenBSD: v_zexit.c,v 1.6 2014/11/12 04:28:41 bentley Exp $     */

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
#include <limits.h>
#include <stdio.h>
#include <bsd_string.h>

#include "../common/common.h"
#include "vi.h"

/*
 * v_zexit -- ZZ
 *      Save the file and exit.
 *
 * PUBLIC: int v_zexit(SCR *, VICMD *);
 */
int
v_zexit(SCR *sp, VICMD *vp)
{
        (void)sp;
        (void)vp;

        /* Write back any modifications. */
        if (F_ISSET(sp->ep, F_MODIFIED) &&
            file_write(sp, NULL, NULL, NULL, FS_ALL))
                return (1);

        /* Check to make sure it's not a temporary file. */
        if (file_m3(sp, 0))
                return (1);

        /* Check for more files to edit. */
        if (ex_ncheck(sp, 0))
                return (1);

        F_SET(sp, SC_EXIT);
        return (0);
}
