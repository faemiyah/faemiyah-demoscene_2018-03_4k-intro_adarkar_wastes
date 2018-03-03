function result = Clamp(value, min, max)

	if(value > max)
		value = max;
	end
	
	if(value < min)
		value = min;
	end
	
	result = value;

end