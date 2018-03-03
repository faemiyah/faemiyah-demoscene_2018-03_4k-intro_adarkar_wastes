function result = LerpVector(vec1, vec2, fraction)
	if(~exist('fraction','var'))
		fraction = 0.5;
	end
	
	result = vec1*fraction + vec2*(1-fraction);

	result = Normalize(result);

end