report: br5.pdf
%.pdf: %.tex
	pdflatex $<
	if grep "LaTeX Warning: Label(s) may have changed. Rerun to get cross-references right." $*.log; then pdflatex $*; fi
	rm $*.log $*.aux
minimal: minimal.c
	gcc minimal.c -o minimal -Ofast -lgmp -std=c99 -DPRINTSTATS -DPRINTITER -DPRINTDATA
simple: simple.c
	gcc simple.c -o simple -Ofast -lgmp -std=c99
search: search.c
	gcc search.c -o search -Ofast -lgmp -std=c99
sieving.start: sieving.start.c
	gcc sieving.start.c -o sieving.start -Ofast -lgmp -std=c99
merge: merge.c
	gcc merge.c -o merge -Ofast -lgmp -std=c99
organize: organize.c
	gcc organize.c -o organize -Ofast -lgmp -std=c99
summary: summary.c
	gcc summary.c -o summary -Ofast -lgmp -std=c99
clean:
	rm kernel
