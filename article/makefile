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
	Violada-contributions.eps Violada-optimized-contributions.eps \
	sphere-variables-ga-100-33-0.25-0.25-0.25-32.eps \
	sphere-variables-ga-250-16-0.2-0.2-0.2-32.eps \
	sphere-variables-mc-100-25-10-0.1.eps \
	sphere-variables-mc-2500-1.eps \
	sphere-variables-mc-r-100-1-240-10-0.0001-1.5.eps \
	sphere-variables-sw-10-10-25-10-0.5.eps \
	sphere-variables-sw-50-50-1.eps

plots = Beale.eps

calibrator.pdf: calibrator.tex bib.bib makefile $(figs) $(plots)
	latex calibrator
	bibtex calibrator
	latex calibrator
	latex calibrator
	dvipdf calibrator

Beale.eps: plot
	gnuplot plot
