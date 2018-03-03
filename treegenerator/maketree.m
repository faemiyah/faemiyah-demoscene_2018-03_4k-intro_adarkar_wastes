clear;
close all;

lines = [];

TREES_X = 4;
TREES_Y = 2;

figure

for ii=1:TREES_X
	for jj=1:TREES_Y
		lines = [];
		subplot(TREES_Y, TREES_X, (jj-1)*TREES_X+ii)
		lines = GenerateSegment(lines, [0 0 0], [0 0 1], [0 0 1], 0, 1, 0);
		PlotTree(lines);
	end
end
