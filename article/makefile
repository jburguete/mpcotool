figs = mpcotool-en.eps surcos-advance.eps surcos-depth.eps \
	sphere-variables-sw-50-50-1.eps \
	sphere-variables-mc-2500-1.eps \
	sphere-variables-sw-10-10-25-10-0.5.eps \
	sphere-variables-mc-100-25-10-0.1.eps \
	sphere-variables-ge-250-16-0.2-0.2-0.2-32.eps \
	sphere-variables-mc-ra-100-1-600-4-0.1-1.eps \
	sphere-evolution-sw.eps sphere-evolution-mc.eps \
	sphere-evolution-sw-cdr.eps sphere-evolution-mc-cdr.eps \
	sphere-evolution-mc-cd-r.eps sphere-evolution-mc-cd-s.eps \
	sphere-evolution-mc-r.eps \
	sphere-task-1-4.eps sphere-task-1-64.eps \
	sphere-task-2-4.eps sphere-task-2-64.eps \
	sphere-evolution-ge-1-4.eps sphere-evolution-ge-5-8.eps \
	sphere-evolution-ge-9-11.eps \
	Sphere-e.eps Ackley-e.eps Booth-e.eps Rosenbrock-e.eps Easom-e.eps \
	Beale-e.eps \
	surcos-solute-q1.eps surcos-solute-q2.eps surcos-solute-q3.eps \
	surcos-solute-q4.eps sprinkler-0.eps sprinkler-2,84.eps \
	pivot-simulated-starts-100.eps pivot-simulated-stops-100.eps \
	pivot-measured-starts-100.eps pivot-measured-stops-100.eps \
	pivot-simulated-starts-52.eps pivot-simulated-stops-52.eps \
	pivot-measured-starts-52.eps pivot-measured-stops-52.eps \
	pivot-simulated-starts-42.eps pivot-simulated-stops-42.eps \
	pivot-measured-starts-42.eps pivot-measured-stops-42.eps \
	pivot-simulated-starts-27,48.eps pivot-simulated-stops-27,48.eps \
	pivot-measured-starts-27,48.eps pivot-measured-stops-27,48.eps \
	Violada-contributions.eps Violada-optimized-contributions.eps \

plots = Beale.eps

mpcotool.pdf: mpcotool.tex bib.bib makefile $(figs) $(plots)
	latex mpcotool
	bibtex mpcotool
	latex mpcotool
	latex mpcotool
	dvipdf mpcotool

Beale.eps: plot
	gnuplot plot
