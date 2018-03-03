function result = GenerateSegment(lines, point, direction, bias, fraction_done, scale, recursionlevel)

	SEGMENTS = 25;
	RECURSION_MAX = 2;

	BRANCH_CHANCE = 5;
	BRANCH_BIAS_FOLLOW = 0.7;
	BRANCH_BEFORE = 0.9;
	BRANCH_AFTER = 0.1;
	BRANCH_RECUR_MIN = 0.1;
	BRANCH_RECUR_MAX = 0.8;
	BRANCH_SCALE = 0.75;
	
	BIFAC_CHANCE = 1.0;
%	BIFAC_CHANCE = 0.0;
	BIFAC_BIAS_FOLLOW = 0.1;
	BIFAC_BEFORE = 0.7;
	BIFAC_AFTER = 0.25;

	if(recursionlevel > RECURSION_MAX)
		result = lines;
		return;
	end
	
	prevpoint = point;
	
	for ii=fraction_done:(1/SEGMENTS):1
	
		if recursionlevel == 0
			nextpoint = prevpoint + LerpVector(direction, bias, 0.8)*scale;
			lines = [lines ; prevpoint 0 ; nextpoint 0];
		else
			nextpoint = prevpoint + LerpVector(direction, -[0 0 1], 0.95)*scale;
			lines = [lines ; prevpoint 1 ; nextpoint 1];
		end

		direction = nextpoint-prevpoint;
				
		% Trunk-level bifacuration
		if(rand < BIFAC_CHANCE/SEGMENTS && ii <= BIFAC_BEFORE && ii >= BIFAC_AFTER && recursionlevel == 0)
			normal = NormalVector(bias);
			bifacbias = rand;
			lines = GenerateSegment(lines, nextpoint, LerpVector(normal,bias,bifacbias*(1-BIFAC_BIAS_FOLLOW) + BIFAC_BIAS_FOLLOW), bias, ii, scale, recursionlevel);
			lines = GenerateSegment(lines, nextpoint, LerpVector(-normal,bias,(1-bifacbias)*(1-BIFAC_BIAS_FOLLOW) + BIFAC_BIAS_FOLLOW), bias, ii, scale, recursionlevel);
			break;
		end
		
		% Branching
		if(rand < BRANCH_CHANCE/SEGMENTS && ii <= BRANCH_BEFORE && ii >= BRANCH_AFTER)
			if(recursionlevel == 0)
				normal = NormalVector(direction);
			else
				normal = cross(direction, [0 0 1]);
				normal = (rand()-0.5)*normal;
				normal = normal/norm(normal);
			end
			branchbias = rand;
			lines = GenerateSegment(lines, nextpoint, LerpVector(direction, normal, branchbias*(1-BRANCH_BIAS_FOLLOW) + BRANCH_BIAS_FOLLOW), bias, Clamp(ii, BRANCH_RECUR_MIN, BRANCH_RECUR_MAX), scale*BRANCH_SCALE, recursionlevel+1);
		end

		prevpoint = nextpoint;
	end
	
	result = lines;
	
end