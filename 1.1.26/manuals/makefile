user-manual.pdf: user-manual.tex bib.bib makefile calibrator-en.eps
	latex user-manual
	bibtex user-manual
	latex user-manual
	latex user-manual
	dvipdf user-manual

calibrator-en.eps: calibrator-en.png makefile
	convert calibrator-en.png calibrator-en.eps
