TARGET=cslabb08-REPORT
VERSION=1
TEX=$(TARGET)-$(VERSION).tex
PDF=$(TARGET)-$(VERSION).pdf
all: $(PDF)
$(PDF): $(TEX) title/* files/*
		xelatex -shell-escape $<
spell:
		aspell -l el_gr --ignore-case=true -c $(TEX)
