figs = surcos-advance.eps surcos-depth.eps surcos-solute-q1.eps \
	surcos-solute-q2.eps surcos-solute-q3.eps surcos-solute-q4.eps \
	sprinkler-0.eps sprinkler-2,84.eps \
	pivot-simulated-starts-100.eps pivot-simulated-stops-100.eps \
	pivot-measured-starts-100.eps pivot-measured-stops-100.eps \
	pivot-simulated-starts-52.eps pivot-simulated-stops-52.eps \
	pivot-measured-starts-52.eps pivot-measured-stops-52.eps \
	pivot-simulated-starts-42.eps pivot-simulated-stops-42.eps \
	pivot-measured-starts-42.eps pivot-measured-stops-42.eps \
	pivot-simulated-starts-27,48.eps pivot-simulated-stops-27,48.eps \
	pivot-measured-starts-27,48.eps pivot-measured-stops-27,48.eps \
	Violada-contributions.eps Violada-optimized-contributions.eps

plots = Beale.eps

calibrator.pdf: calibrator.tex bib.bib makefile $(figs) $(plots)
	latex calibrator
	bibtex calibrator
	latex calibrator
	latex calibrator
	dvipdf calibrator

Beale.eps: plot
	gnuplot plot
