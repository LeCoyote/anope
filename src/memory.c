/* Memory management routines.
 *
 * (C) 2003-2009 Anope Team
 * Contact us at team@anope.org
 *
 * Please read COPYING and README for further details.
 *
 * Based on the original code of Epona by Lara.
 * Based on the original code of Services by Andy Church.
 *
 * $Id$
 *
 */

#include "services.h"

/* smalloc, scalloc, srealloc, sstrdup:
 *	Versions of the memory allocation functions which will cause the
 *	program to terminate with an "Out of memory" error if the memory
 *	cannot be allocated.  (Hence, the return value from these functions
 *	is never NULL.)
 */

/*************************************************************************/

/**
 * malloc, replacement so we can trap for "out of memory"
 * @param size to allocate
 * @return void
 */
void *smalloc(long size)
{
	void *buf;

	if (!size) {
		size = 1;
	}
	buf = malloc(size);
	if (!buf)
		abort();
	return buf;
}

/*************************************************************************/

/**
 * calloc, replacement so we can trap for "out of memory"
 * @param elsize to allocate
 * @param els size of members
 * @return void
 */
void *scalloc(long elsize, long els)
{
	void *buf;

	if (!elsize || !els) {
		elsize = els = 1;
	}
	buf = calloc(elsize, els);
	if (!buf)
		abort();
	return buf;
}

/*************************************************************************/

/**
 * realloc, replacement so we can trap for "out of memory"
 * @param oldptr Old Pointer
 * @param newsize Size of new pointer
 * @return void
 */
void *srealloc(void *oldptr, long newsize)
{
	void *buf;

	if (!newsize) {
		newsize = 1;
	}
	buf = realloc(oldptr, newsize);
	if (!buf)
		abort();
	return buf;
}

/*************************************************************************/

/**
 * strdup, replacement so we can trap for "out of memory"
 * @param oldptr Old Pointer
 * @param newsize Size of new pointer
 * @return void
 */
char *sstrdup(const char *src)
{
	char *ret = NULL;
	if (src) {
		ret = new char[strlen(src) + 1];
		if (!ret)
			abort();
		strcpy(ret, src);
	} else {
		alog("sstrdup() called with NULL-arg");
		abort();
	}

	return ret;
}

/*************************************************************************/
/*************************************************************************/

/* In the future: malloc() replacements that tell us if we're leaking and
 * maybe do sanity checks too... */

/*************************************************************************/
