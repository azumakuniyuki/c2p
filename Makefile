# Makefile -- makefile(GNU) for c2p
CC	= gcc
CFLAGS	= -O -Wall
PREFIX	= /usr/local

# Default Language
#  JA = Japanese
#  EN = English
LFLAGS	= -DEN

# Macro for Kanji(Japanese)
#  KANJI = Use Kanji(漢字) in some characters/e.g.) 2 -> 弐, $ -> 弗, # -> 井, など
#  DAIJI = Use Daiji(大字) in numerical characters/e.g.) 2 -> 貳, 3 -> 參, 5 => 伍, など
KANJI	= -DKANJI

# For system that has no strlcpy()
# NOSTRL = -DNOSTRL


.PHONY: clean
c2p:
	$(CC) $(CFLAGS) $(LFLAGS) $(KANJI) $(NOSTRL) -o c2p c2p.c

clean:
	rm -f c2p

install:
	cp c2p $(PREFIX)/bin/
	chmod a+x $(PREFIX)/bin/c2p

uninstall:
	rm -f $(PREFIX)/bin/c2p

