report: br5.pdf
%.pdf: %.tex
	pdflatex $<
	if grep "LaTeX Warning: There were undefined references." $*.log || grep '^\\nocite{\*}$$' $*.tex; then bibtex $*; pdflatex $*; fi
	if grep "LaTeX Warning: Label(s) may have changed. Rerun to get cross-references right." $*.log; then pdflatex $*; fi
kernel: kernel.c
	gcc kernel.c -o kernel -O3 -lgmp -std=c99 -DPRINTSTATS -DPRINTITER -DPRINTDIVISORSQUARE -DPRINTDIVISORCUBE -DPRINTDATA $(FLAGS)
debug: FLAGS = -g -pg
debug: kernel
clean:
	rm kernel
checker: checker.c pprime_p.c
	gcc checker.c -o checker -O3 -lgmp -lm -std=c99 -I ~/gmp-5.1.0
