#include <assert.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <regex.h>

#include "regerror.ih"

static struct rerr {
	int code;
	char *name;
	char *explain;
} rerrs[] = {
	{REG_OKAY,	"REG_OKAY",	"no errors detected"},
	{REG_NOMATCH,	"REG_NOMATCH",	"regexec() failed to match"},
	{REG_BADPAT,	"REG_BADPAT",	"invalid regular expression"},
	{REG_ECOLLATE,	"REG_ECOLLATE",	"invalid collating element"},
	{REG_ECTYPE,	"REG_ECTYPE",	"invalid character class"},
	{REG_EESCAPE,	"REG_EESCAPE",	"trailing backslash (\\)"},
	{REG_ESUBREG,	"REG_ESUBREG",	"invalid backreference number"},
	{REG_EBRACK,	"REG_EBRACK",	"brackets ([ ]) not balanced"},
	{REG_EPAREN,	"REG_EPAREN",	"parentheses not balanced"},
	{REG_EBRACE,	"REG_EBRACE",	"braces not balanced"},
	{REG_BADBR,	"REG_BADBR",	"invalid repetition count(s)"},
	{REG_ERANGE,	"REG_ERANGE",	"invalid character range"},
	{REG_ESPACE,	"REG_ESPACE",	"out of memory"},
	{REG_BADRPT,	"REG_BADRPT",	"repetition-operator operand invalid"},
	{REG_EMPTY,	"REG_EMPTY",	"empty (sub)expression"},
	{REG_ASSERT,	"REG_ASSERT",	"\"can't happen\" -- you found a bug"},
	{REG_INVARG,	"REG_INVARG",	"invalid argument to regex routine"},
	{-1,		"",		"*** unknown regexp error code ***"},
};

/*
 - regerror - the interface to error numbers
 */
/* ARGSUSED */
size_t
regerror(errcode, preg, errbuf, errbuf_size)
int errcode;
const regex_t *preg;
char *errbuf;
size_t errbuf_size;
{
	struct rerr *r;
	size_t len;
	int target = errcode &~ REG_ITOA;
	char *s;
	char convbuf[50];

	if (errcode == REG_ATOI)
		s = regatoi(preg, convbuf);
	else {
		for (r = rerrs; r->code >= 0; r++)
			if (r->code == target)
				break;
	
		if (errcode&REG_ITOA) {
			if (r->code >= 0)
				(void) strcpy(convbuf, r->name);
			else
				sprintf(convbuf, "REG_0x%x", target);
			assert(strlen(convbuf) < sizeof(convbuf));
			s = convbuf;
		} else
			s = r->explain;
	}

	len = strlen(s) + 1;
	if (errbuf_size > 0) {
		if (errbuf_size > len)
			(void) strcpy(errbuf, s);
		else {
			(void) strncpy(errbuf, s, errbuf_size-1);
			errbuf[errbuf_size-1] = '\0';
		}
	}

	return(len);
}

/*
 - regatoi - internal routine to implement REG_ATOI
 == static char *regatoi(const regex_t *preg, char *localbuf);
 */
static char *
regatoi(preg, localbuf)
const regex_t *preg;
char *localbuf;
{
	struct rerr *r;

	for (r = rerrs; r->code >= 0; r++)
		if (strcmp(r->name, preg->re_endp) == 0)
			break;
	if (r->code < 0)
		return("0");

	sprintf(localbuf, "%d", r->code);
	return(localbuf);
}
