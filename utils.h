/* utility definitions */

#define	NC		(CHAR_MAX - CHAR_MIN + 1)

typedef unsigned char uch;

/* switch off assertions (if not already off) if no REDEBUG */
#ifndef REDEBUG
#ifndef NDEBUG
#define	NDEBUG	/* no assertions please */
#endif
#endif
#include <assert.h>

/* for old systems with bcopy() but no memmove() */
#ifdef USEBCOPY
#define	memmove(d, s, c)	bcopy(s, d, c)
#endif
