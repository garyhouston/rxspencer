AUTOMAKE_OPTIONS = foreign
lib_LTLIBRARIES = librxspencer.la
librxspencer_la_LDFLAGS = -version-info 0:0:0
INCLUDES = -DPOSIX_MISTAKE
librxspencer_la_SOURCES = regcomp.c regerror.c regexec.c regfree.c
noinst_HEADERS = cclass.h cname.h engine.c regex2.h utils.h
man_MANS = rxspencer.3 rxspencer.7
BUILT_SOURCES = main.ih debug.ih regerror.ih engine.ih regcomp.ih regex.h
test_SOURCES = split.c debug.c main.c 
re_d_objs = split.d debug.d main.d regcomp.d regerror.d regexec.d regfree.d
re: $(BUILT_SOURCES) $(re_d_objs)
	$(LINK) -o re $(re_d_objs)
SUFFIXES = .d .ih
MKHFLAGS=
.c.d:
	$(COMPILE) -c -DREDEBUG $< -o $@
.c.ih:
	sh $(srcdir)/mkh $(MKHFLAGS) -p $< > $@
REGEXHSRC = $(srcdir)/regex2.h $(srcdir)/regcomp.c $(srcdir)/regerror.c\
	$(srcdir)/regexec.c $(srcdir)/regfree.c
regex.h: $(REGEXHSRC) mkh
	sh $(srcdir)/mkh $(MKHFLAGS) -i _REGEX_H_ $(REGEXHSRC) >regex.tmp
	cmp -s regex.tmp regex.h 2>/dev/null || cp regex.tmp regex.h
	rm -f regex.tmp
install-data-local:
	$(mkinstalldirs) $(pkgincludedir)
	$(INSTALL_DATA) regex.h $(pkgincludedir)
CLEANFILES = re *.d *.ih regex.h
TESTS = test1.sh test2.sh test3.sh
test1.sh: re
EXTRA_DIST = COPYRIGHT WHATSNEW tests autogen.sh mkh\
	Makefile.orig $(TESTS) $(test_SOURCES) $(man_MANS)\
	fake/memmove.c fake/limits.h fake/stdlib.h
