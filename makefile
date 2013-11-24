report: br5.pdf
%.pdf: %.tex
	pdflatex $<
	if grep "LaTeX Warning: Label(s) may have changed. Rerun to get cross-references right." $*.log; then pdflatex $*; fi
	rm $*.log $*.aux
minimal: minimal.c
	gcc minimal.c -o minimal -Ofast -lgmp -std=c99 -DPRINTSTATS -DPRINTITER -DPRINTDATA $(FLAGS)
simple: simple.c
	gcc simple.c -o simple -Ofast -lgmp -std=c99
sieving: sieving.c
	gcc sieving.c -o sieving -Ofast -lgmp -std=c99
sieving.in: sieving.in.c
	gcc sieving.in.c -o sieving.in -Ofast -lgmp -std=c99
merge: merge.c
	gcc merge.c -o merge -Ofast -lgmp -std=c99
debug: FLAGS = -g -pg
debug: kernel
clean:
	rm kernel
