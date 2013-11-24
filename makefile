report: br5.pdf
%.pdf: %.tex
	pdflatex $<
	if grep "LaTeX Warning: There were undefined references." $*.log || grep '^\\nocite{\*}$$' $*.tex; then bibtex $*; pdflatex $*; fi
	if grep "LaTeX Warning: Label(s) may have changed. Rerun to get cross-references right." $*.log; then pdflatex $*; fi
minimal: minimal.c
	gcc minimal.c -o minimal -O3 -lgmp -std=c99 -DPRINTSTATS -DPRINTITER -DPRINTDATA $(FLAGS)
simple: simple.c
	gcc simple.c -o simple -O3 -lgmp -std=c99
sieving: sieving.c
	gcc sieving.c -o sieving -O3 -lgmp -std=c99
merge: merge.c
	gcc merge.c -o merge -O3 -lgmp -std=c99
debug: FLAGS = -g -pg
debug: kernel
clean:
	rm kernel
