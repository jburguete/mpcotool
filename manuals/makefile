manual-en.pdf: manual-en.tex bib.bib makefile calibrator-en.eps
	latex manual-en
	bibtex manual-en
	latex manual-en
	latex manual-en
	dvipdf manual-en

calibrator-en.eps: calibrator-en.png makefile
	convert calibrator-en.png calibrator-en.eps
