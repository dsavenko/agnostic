
ASCIIDOC=asciidoc
TXT_TO_XML=$(ASCIIDOC) -b docbook -d manpage

XMLTO=xmlto
XML_TO_MAN=$(XMLTO) man

INSTALL=install

MAN1_TXT =

MAN5_TXT = \
	agnostic.yaml.asciidoc

MAN7_TXT =

MAN_TXT = $(MAN1_TXT) $(MAN5_TXT) $(MAN7_TXT)

prefix=/usr/local
mandir ?= $(prefix)/share/man
man1dir = $(mandir)/man1
man5dir = $(mandir)/man5
man7dir = $(mandir)/man7

DOC_MAN1 = $(patsubst %.asciidoc,%.1,$(MAN1_TXT))
DOC_MAN5 = $(patsubst %.asciidoc,%.5,$(MAN5_TXT))
DOC_MAN7 = $(patsubst %.asciidoc,%.7,$(MAN7_TXT))
DOC_MAN = $(DOC_MAN1) $(DOC_MAN5) $(DOC_MAN7)

all: man

man: $(DOC_MAN)

%.xml: %.asciidoc
	$(TXT_TO_XML) $<

%.1 %.5 %.7: %.xml
	$(XML_TO_MAN) $<

.PHONY: install clean uninstall

clean:
	rm -f $(DOC_MAN) *.xml

install: all
	$(INSTALL) -d -m 755 $(DESTDIR)$(man1dir)
	$(INSTALL) -d -m 755 $(DESTDIR)$(man5dir)
	$(INSTALL) -d -m 755 $(DESTDIR)$(man7dir)
	$(INSTALL) -m 644 $(DOC_MAN5) $(DESTDIR)$(man5dir)

uninstall:
	rm -f $(addprefix $(DESTDIR)$(man5dir)/, $(DOC_MAN5))
