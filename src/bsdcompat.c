#include "../include-private/bsdcompat.h"
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <stdio.h>
#include <qc3prng.h>

/* XXX: Has wrong semantics */
int clock_gettime(int clk_id, struct timespec *tp)
{
        tp->tv_sec = time(NULL);
        return 0;
}

typedef struct {
	int bytes_in;
	int bytes_avail;
	char exception_id[7];
	char reserved;
} ERRC0100;

void arc4random_buf(void *buf, size_t nbytes)
{
	ERRC0100 error;
	error.bytes_in = sizeof(error);
	/*
	 * Yes, these are EBCDIC zoned ints. Because this file is built with
	 * UTF locale, use EBCDIC in hex.
	 */
	Qc3GenPRNs(buf, nbytes, 0xf0, 0xf0, &error);
	if (error.exception_id[0] != '\0') {
		/* XXX: We're in UTF-8 ILE world, but XPF still returns EBCDIC */
		fprintf(stderr, " ** arc4random_buf should never fail");
		abort();
	}
}

/*	$OpenBSD: explicit_bzero.c,v 1.4 2015/08/31 02:53:57 guenther Exp $ */
/*
 * Public domain.
 * Written by Matthew Dempsky.
 */

void
explicit_bzero(void *buf, size_t len)
{
	memset(buf, 0, len);
	/* XXX: Does ILE C optimize this out? */
}
DEF_WEAK(explicit_bzero);

/*	$OpenBSD: timingsafe_bcmp.c,v 1.3 2015/08/31 02:53:57 guenther Exp $	*/
/*
 * Copyright (c) 2010 Damien Miller.  All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

int
timingsafe_bcmp(const void *b1, const void *b2, size_t n)
{
	const unsigned char *p1 = b1, *p2 = b2;
	int ret = 0;

	for (; n > 0; n--)
		ret |= *p1++ ^ *p2++;
	return (ret != 0);
}
DEF_WEAK(timingsafe_bcmp);

/*	$OpenBSD: strtonum.c,v 1.8 2015/09/13 08:31:48 guenther Exp $	*/

/*
 * Copyright (c) 2004 Ted Unangst and Todd Miller
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <errno.h>
#include <limits.h>
#include <stdlib.h>

#define	INVALID		1
#define	TOOSMALL	2
#define	TOOLARGE	3

long long
strtonum(const char *numstr, long long minval, long long maxval,
    const char **errstrp)
{
	long long ll = 0;
	int error = 0;
	char *ep;
	struct errval {
		const char *errstr;
		int err;
	} ev[4] = {
		{ NULL,		0 },
		{ "invalid",	EINVAL },
		{ "too small",	ERANGE },
		{ "too large",	ERANGE },
	};

	ev[0].err = errno;
	errno = 0;
	if (minval > maxval) {
		error = INVALID;
	} else {
		ll = strtoll(numstr, &ep, 10);
		if (numstr == ep || *ep != '\0')
			error = INVALID;
		else if ((ll == LLONG_MIN && errno == ERANGE) || ll < minval)
			error = TOOSMALL;
		else if ((ll == LLONG_MAX && errno == ERANGE) || ll > maxval)
			error = TOOLARGE;
	}
	if (errstrp != NULL)
		*errstrp = ev[error].errstr;
	errno = ev[error].err;
	if (error)
		ll = 0;

	return (ll);
}
DEF_WEAK(strtonum);

