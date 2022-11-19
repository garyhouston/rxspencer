#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <assert.h>

#include "regex.h"

int debug = 0;
int line = 0;
int status = 0;

int copts = REG_EXTENDED;
int eopts = 0;
char *fopts = 0;
regoff_t startoff = 0;
regoff_t endoff = 0;

extern int split();
extern void regprint();

static void regress(FILE *in);
static void try(char *f0, char *f1, char *f2, char *f3, char *f4, int opts);
static char *check(char *str, regmatch_t sub, char *should);
static int parseopts(int argc, char *argv[]);
static int options(int type, char *s);
static int opt(int c, char *s);
static void fixstr(char *p);
static char *eprint(int err);
static int efind(char *name);

/*
 - main - do the simple case, hand off to regress() for regression
 */
int
main(argc, argv)
int argc;
char *argv[];
{
	regex_t re;
#	define	NS	10
	regmatch_t subs[NS];
	char erbuf[100];
	int err;
	int i;
	int optind = parseopts(argc, argv);

	if (fopts != 0) {
		FILE *f = fopen(fopts, "r");
		if (f == NULL) {
			fputs("unable to open input\n", stderr);
			exit(1);
		}
		regress(f);
		exit(status);
	}
	
	if (optind >= argc) {
		regress(stdin);
		exit(status);
	}

	err = regcomp(&re, argv[optind++], copts);
	if (err) {
	  	size_t len = regerror(err, &re, erbuf, sizeof(erbuf));
		fprintf(stderr, "error %s, %lu/%d `%s'\n",
			eprint(err), (unsigned long)len, (int)sizeof(erbuf), erbuf);
		exit(status);
	}
	regprint(&re, stdout);	

	if (optind >= argc) {
		regfree(&re);
		exit(status);
	}

	if (eopts&REG_STARTEND) {
		subs[0].rm_so = startoff;
		subs[0].rm_eo = (regoff_t)strlen(argv[optind]) - endoff;
	}
	err = regexec(&re, argv[optind], (size_t)NS, subs, eopts);
	if (err) {
		size_t len = regerror(err, &re, erbuf, sizeof(erbuf));
		fprintf(stderr, "error %s, %lu/%d `%s'\n",
			eprint(err), (unsigned long)len, (int)sizeof(erbuf), erbuf);
		exit(status);
	}
	if (!(copts&REG_NOSUB)) {
		int len = (int)(subs[0].rm_eo - subs[0].rm_so);
		if (subs[0].rm_so != -1) {
			if (len != 0)
				printf("match `%.*s'\n", len,
					argv[optind] + subs[0].rm_so);
			else
				printf("match `'@%.1s\n",
					argv[optind] + subs[0].rm_so);
		}
		for (i = 1; i < NS; i++)
			if (subs[i].rm_so != -1)
				printf("(%d) `%.*s'\n", i,
					(int)(subs[i].rm_eo - subs[i].rm_so),
					argv[optind] + subs[i].rm_so);
	}
	exit(status);
}

/*
 - regress - main loop of regression test
 */
static void
regress(in)
FILE *in;
{
	char inbuf[1000];
#	define	MAXF	10
	char *f[MAXF];
	int nf;
	int i;
	char erbuf[100];
	size_t ne;
	char *badpat = "invalid regular expression";
#	define	SHORT	10
	char *bpname = "REG_BADPAT";
	regex_t re;

	while (fgets(inbuf, sizeof(inbuf), in) != NULL) {
		line++;
		if (inbuf[0] == '#' || inbuf[0] == '\n')
			continue;			/* NOTE CONTINUE */
		inbuf[strlen(inbuf)-1] = '\0';	/* get rid of stupid \n */
		if (debug)
			fprintf(stdout, "%d:\n", line);
		nf = split(inbuf, f, MAXF, "\t\t");
		if (nf < 3) {
			fprintf(stderr, "bad input, line %d\n", line);
			exit(1);
		}
		for (i = 0; i < nf; i++)
			if (strcmp(f[i], "\"\"") == 0)
				f[i] = "";
		if (nf <= 3)
			f[3] = NULL;
		if (nf <= 4)
			f[4] = NULL;
		try(f[0], f[1], f[2], f[3], f[4], options('c', f[1]));
		if (opt('&', f[1]))	/* try with either type of RE */
			try(f[0], f[1], f[2], f[3], f[4],
					options('c', f[1]) &~ REG_EXTENDED);
	}

	ne = regerror(REG_BADPAT, (regex_t *)NULL, erbuf, sizeof(erbuf));
	if (strcmp(erbuf, badpat) != 0 || ne != strlen(badpat)+1) {
		fprintf(stderr, "end: regerror() test gave `%s' not `%s'\n",
							erbuf, badpat);
		status = 1;
	}
	ne = regerror(REG_BADPAT, (regex_t *)NULL, erbuf, (size_t)SHORT);
	if (strncmp(erbuf, badpat, SHORT-1) != 0 || erbuf[SHORT-1] != '\0' ||
						ne != strlen(badpat)+1) {
		fprintf(stderr, "end: regerror() short test gave `%s' not `%.*s'\n",
						erbuf, SHORT-1, badpat);
		status = 1;
	}
	ne = regerror(REG_ITOA|REG_BADPAT, (regex_t *)NULL, erbuf, sizeof(erbuf));
	if (strcmp(erbuf, bpname) != 0 || ne != strlen(bpname)+1) {
		fprintf(stderr, "end: regerror() ITOA test gave `%s' not `%s'\n",
						erbuf, bpname);
		status = 1;
	}
	re.re_endp = bpname;
	ne = regerror(REG_ATOI, &re, erbuf, sizeof(erbuf));
	if (atoi(erbuf) != (int)REG_BADPAT) {
		fprintf(stderr, "end: regerror() ATOI test gave `%s' not `%ld'\n",
						erbuf, (long)REG_BADPAT);
		status = 1;
	} else if (ne != strlen(erbuf)+1) {
		fprintf(stderr, "end: regerror() ATOI test len(`%s') = %ld\n",
						erbuf, (long)REG_BADPAT);
		status = 1;
	}
}

/*
 - try - try it, and report on problems
 */
static void
try(f0, f1, f2, f3, f4, opts)
char *f0;
char *f1;
char *f2;
char *f3;
char *f4;
int opts;			/* may not match f1 */
{
	regex_t re;
#	define	NSUBS	10
	regmatch_t subs[NSUBS];
#	define	NSHOULD	15
	char *should[NSHOULD];
	int nshould;
	char erbuf[100];
	int err;
	size_t len;
	char *type = (opts & REG_EXTENDED) ? "ERE" : "BRE";
	unsigned int i;
	char *grump;
	char f0copy[1000];
	char f2copy[1000];

	strcpy(f0copy, f0);
	re.re_endp = (opts&REG_PEND) ? f0copy + strlen(f0copy) : NULL;
	fixstr(f0copy);
	err = regcomp(&re, f0copy, opts);
	if (err != 0 && (!opt('C', f1) || err != efind(f2))) {
		/* unexpected error or wrong error */
		len = regerror(err, &re, erbuf, sizeof(erbuf));
		fprintf(stderr, "%d: %s error %s, %lu/%u `%s'\n",
				line, type, eprint(err), (unsigned long)len,
				(unsigned int)sizeof(erbuf), erbuf);
		status = 1;
	} else if (err == 0 && opt('C', f1)) {
		/* unexpected success */
		fprintf(stderr, "%d: %s should have given REG_%s\n",
						line, type, f2);
		status = 1;
		err = 1;	/* so we won't try regexec */
	}

	if (err != 0) {
		regfree(&re);
		return;
	}

	strcpy(f2copy, f2);
	fixstr(f2copy);

	if (options('e', f1)&REG_STARTEND) {
		if (strchr(f2, '(') == NULL || strchr(f2, ')') == NULL)
			fprintf(stderr, "%d: bad STARTEND syntax\n", line);
		subs[0].rm_so = (regoff_t)(strchr(f2, '(') - f2 + 1);
		subs[0].rm_eo = (regoff_t)(strchr(f2, ')') - f2);
	}
	err = regexec(&re, f2copy, NSUBS, subs, options('e', f1));

	if (err != 0 && (f3 != NULL || err != REG_NOMATCH)) {
		/* unexpected error or wrong error */
		len = regerror(err, &re, erbuf, sizeof(erbuf));
		fprintf(stderr, "%d: %s exec error %s, %lu/%u `%s'\n",
				line, type, eprint(err), (unsigned long)len,
				(unsigned int)sizeof(erbuf), erbuf);
		status = 1;
	} else if (err != 0) {
		/* nothing more to check */
	} else if (f3 == NULL) {
		/* unexpected success */
		fprintf(stderr, "%d: %s exec should have failed\n",
						line, type);
		status = 1;
		err = 1;		/* just on principle */
	} else if (opts&REG_NOSUB) {
		/* nothing more to check */
	} else if ((grump = check(f2, subs[0], f3)) != NULL) {
		fprintf(stderr, "%d: %s %s\n", line, type, grump);
		status = 1;
		err = 1;
	}

	if (err != 0 || f4 == NULL) {
		regfree(&re);
		return;
	}

	for (i = 1; i < NSHOULD; i++)
		should[i] = NULL;
	nshould = split(f4, should+1, NSHOULD-1, ",");
	if (nshould == 0) {
		nshould = 1;
		should[1] = "";
	}
	for (i = 1; i < NSUBS; i++) {
		grump = check(f2, subs[i], should[i]);
		if (grump != NULL) {
			fprintf(stderr, "%d: %s $%u %s\n", line,
							type, i, grump);
			status = 1;
			err = 1;
		}
	}

	regfree(&re);
}

/*
 - parseopts - half-baked option processing to avoid using getopt, which isn't always available on Windows.
 */
static int
parseopts(argc, argv)
int argc;
char *argv[];
{
	int i, j;
	for (i = 1; i < argc; i++) {
		if (argv[i][0] != '-' || argv[i][1] == 0) {
			break;
		}
		for (j = 1; argv[i][j] != 0; j++) {
			char opt = argv[i][j];
			if (opt == 'x') {
				debug++;
			} else {
				char *arg;
				if (argv[i][j+1] != 0) {
					arg = argv[i] + j+1;
				} else {
					if (i == argc-1) {
						fprintf(stderr, "option requires an argument -- '%c'\n", opt);
						exit(2);
					}
					arg = argv[++i];
				}
				switch (opt) {
				case 'c':
					copts = options(opt, arg);
					break;
				case 'e':
					eopts = options(opt, arg);
					break;
				case 'f':
					fopts = arg;
					break;
				case 'S':
					startoff = (regoff_t)strtol(arg, NULL, 10);
					break;
				case 'E':
					endoff = (regoff_t)strtol(arg, NULL, 10);
					break;
				default:
					fprintf(stderr, "usage: %s ", argv[0]);
					fprintf(stderr, "[-x][-c copt][-e eopt][-f file][-S startoff][-E endoff] [re]\n");
					exit(2);
				}
				break;
			}
		}
	}
	return i;
}

/*
 - options - pick options out of a regression-test string
 */
static int
options(type, s)
int type;			/* 'c' compile, 'e' exec */
char *s;
{
	char *p;
	int o = (type == 'c') ? copts : eopts;
	char *legal = (type == 'c') ? "bisnmp" : "^$#tl";

	for (p = s; *p != '\0'; p++)
		if (strchr(legal, *p) != NULL)
			switch (*p) {
			case 'b':
				o &= ~REG_EXTENDED;
				break;
			case 'i':
				o |= REG_ICASE;
				break;
			case 's':
				o |= REG_NOSUB;
				break;
			case 'n':
				o |= REG_NEWLINE;
				break;
			case 'm':
				o &= ~REG_EXTENDED;
				o |= REG_NOSPEC;
				break;
			case 'p':
				o |= REG_PEND;
				break;
			case '^':
				o |= REG_NOTBOL;
				break;
			case '$':
				o |= REG_NOTEOL;
				break;
			case '#':
				o |= REG_STARTEND;
				break;
			case 't':	/* trace */
				o |= REG_TRACE;
				break;
			case 'l':	/* force long representation */
				o |= REG_LARGE;
				break;
			case 'r':	/* force backref use */
				o |= REG_BACKR;
				break;
			}
	return(o);
}

/*
 - opt - is a particular option in a regression string?
 */
static int				/* predicate */
opt(c, s)
int c;
char *s;
{
	return(strchr(s, c) != NULL);
}

/*
 - fixstr - transform magic characters in strings
 */
static void
fixstr(p)
char *p;
{
	if (p == NULL)
		return;

	for (; *p != '\0'; p++)
		if (*p == 'N')
			*p = '\n';
		else if (*p == 'T')
			*p = '\t';
		else if (*p == 'S')
			*p = ' ';
		else if (*p == 'Z')
			*p = '\0';
}

/*
 - check - check a substring match
 */
static char *				/* NULL or complaint */
check(str, sub, should)
char *str;
regmatch_t sub;
char *should;
{
	regoff_t len;
	size_t shlen;
	char *p;
	static char grump[500];
	char *at = NULL;

	if (should != NULL && strcmp(should, "-") == 0)
		should = NULL;
	if (should != NULL && should[0] == '@') {
		at = should + 1;
		should = "";
	}

	/* check rm_so and rm_eo for consistency */
	if (sub.rm_so > sub.rm_eo || (sub.rm_so == -1 && sub.rm_eo != -1) ||
				(sub.rm_so != -1 && sub.rm_eo == -1) ||
				(sub.rm_so != -1 && sub.rm_so < 0) ||
				(sub.rm_eo != -1 && sub.rm_eo < 0) ) {
		sprintf(grump, "start %ld end %ld", (long)sub.rm_so,
							(long)sub.rm_eo);
		return(grump);
	}

	/* check for no match */
	if (sub.rm_so == -1 && should == NULL)
		return(NULL);
	if (sub.rm_so == -1)
		return("did not match");

	/* check for in range */
	if ((size_t) sub.rm_eo > strlen(str)) {
		sprintf(grump, "start %ld end %ld, past end of string",
					(long)sub.rm_so, (long)sub.rm_eo);
		return(grump);
	}

	len = sub.rm_eo - sub.rm_so;
	p = str + sub.rm_so;

	/* check for not supposed to match */
	if (should == NULL) {
		sprintf(grump, "matched `%.*s'", len, p);
		return(grump);
	}

	shlen = strlen(should);

	/* check for wrong match */
	if ((size_t)len != shlen || strncmp(p, should, shlen) != 0) {
		sprintf(grump, "matched `%.*s' instead", len, p);
		return(grump);
	}
	if (shlen > 0)
		return(NULL);

	/* check null match in right place */
	if (at == NULL)
		return(NULL);
	shlen = strlen(at);
	if (shlen == 0)
		shlen = 1;	/* force check for end-of-string */
	if (strncmp(p, at, shlen) != 0) {
		sprintf(grump, "matched null at `%.20s'", p);
		return(grump);
	}
	return(NULL);
}

/*
 - eprint - convert error number to name
 */
static char *
eprint(err)
int err;
{
	static char epbuf[100];
	size_t len;

	len = regerror(REG_ITOA|err, (regex_t *)NULL, epbuf, sizeof(epbuf));
	assert(len <= sizeof(epbuf));
	return(epbuf);
}

/*
 - efind - convert error name to number
 */
static int
efind(name)
char *name;
{
	static char efbuf[100];
	regex_t re;

	sprintf(efbuf, "REG_%s", name);
	assert(strlen(efbuf) < sizeof(efbuf));
	re.re_endp = efbuf;
	(void) regerror(REG_ATOI, &re, efbuf, sizeof(efbuf));
	return(atoi(efbuf));
}
