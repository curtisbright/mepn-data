report.pdf: report.tex
	pdflatex --shell-escape $<
	if grep "LaTeX Warning: Label(s) may have changed. Rerun to get cross-references right." report.log; then pdflatex --shell-escape $<; fi
clean:
	rm -f report.log report.aux
clear: clean
	rm -f report.pdf
