function normalvector = NormalVector(vec)

	a2 = rand*2-1;
	b2 = rand*2-1;

	if vec(1) ~= 0
		x2 = (-vec(2)*a2 -vec(3)*b2)/vec(1);
		normalvector = [x2 a2 b2];
	elseif vec(2) ~= 0
		y2 = (-vec(1)*a2 -vec(3)*b2)/vec(2);
		normalvector = [a2 y2 b2];
	elseif vec(3) ~= 0
		z2 = (-vec(1)*a2 -vec(2)*b2)/vec(3);
		normalvector = [a2 b2 z2];
	end
	
	normalvector = normalvector/norm(normalvector);

end