figs = surcos-advance.eps surcos-depth.eps surcos-solute-q1.eps \
	surcos-solute-q2.eps surcos-solute-q3.eps surcos-solute-q4.eps

calibrator.pdf: calibrator.tex bib.bib makefile $(figs)
	latex calibrator
	bibtex calibrator
	latex calibrator
	latex calibrator
	dvipdf calibrator
