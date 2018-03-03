function PlotTree(lines)

	hold on;

	scale = 0.2;
	plot3(scale*[-100 100 100 100], scale*[-100 -100 100 100], scale*[0 0 0 100], 'b-')

	for ii=1:2:size(lines,1)

		if lines(ii,4) == 0
			lineWidth = 2;
		else
			lineWidth = 1;
		end

		plot3(lines(ii:ii+1, 1), lines(ii:ii+1, 2), lines(ii:ii+1, 3), 'k-', 'LineWidth', lineWidth)

	end

	hold off;

	view(50,25)

end