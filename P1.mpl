unconvert := proc(l, b)
	return sum(l[k]*b^(k-1), k=1..nops(l));
end proc:

subwordin := proc(m, K, b)
	local i, j, k, mc, kc;
	mc := convert(m, base, b);
	for i from 1 to nops(K) do
		kc := convert(K[i], base, b);
		k := 1;
		for j from 1 to nops(mc) do
			if mc[j] = kc[k] then
				k := k+1;
			end if;
			if k = nops(kc)+1 then
				return true;
			end if;
		end do;
	end do;
	return false;
end proc:

P1 := proc(b, depth)
	local K, L, newL, i, j, k, n, m;
	L := [0];
	K := [];
	for k from 1 to depth do
		newL := [];
		for i from 1 to nops(L) do
			n := convert(L[i], base, b);
			for j from 0 to b-1 do
				if j <> 0 or L[i] <> 0 then
					m := unconvert([j, op(n)], b);
					if subwordin(m, K, b) then
					
					elif isprime(m) then
						K := [op(K), m];
					else
						newL := [m, op(newL)];
					end if;
				end if;
			end do;
		end do;
		L := newL;
	end do;
	return sort(K), sort(L);
end proc:

exploreforbid := proc(K, m, b, s)
	local i, j, loops, noloop, noloopset, newm, news, finish;
	loops := "";
	news := "";
	noloopset := {};
	for i from 0 to b-1 do
		noloop := false;
		for j from 1 to nops(m) do
			if K[j][-(nops(m[j])+1)] = i then
				noloop := true;
			end if;
		end do;
		if noloop = false then
			loops := cat(loops, i);
		else
			noloopset := noloopset union {i};
		end if;
	end do;
	if evalb(loops<>"") then
		if evalb(s<>"") then
			news := s || "(" || loops || ")*";
		else
			if evalb(0 in noloopset) then
				news := s || "(" || loops || ")*";
			else
				if evalb(loops[2..]="") then
					news := "";
				else
					news := s || "(" || (loops[2..]) || ")(" || loops || ")*";
				end if;
			end if;
		end if;
	else
		news := s;
	end if;
	for i in noloopset do
		newm := m;
		finish := false;
		for j from 1 to nops(m) do
			if K[j][-(nops(m[j])+1)] = i then
				newm[j] := [i, op(newm[j])];
			end if;
			if nops(newm[j]) = nops(K[j]) then
				finish := true;
			end if;
		end do;
		if finish = true then
			print(news);
		else
			exploreforbid(K, newm, b, cat(news, i));
		end if;
	end do;
end proc:

P2 := proc(K, b)
	local Kc;
	Kc := map(convert, K, base, b);
	exploreforbid(Kc, [seq([], i=1..nops(Kc))], b, "");
end proc:

explorestart := proc(starts, middles, ends, K, b)
	local newstarts := [];
	local newmiddles := [];
	local newends := [];
	local temp;
	local i, j, k;
	for i from 1 to nops(starts) do
		for j in middles[i] do
			temp := [];
			for k from 0 to b-1 do
				if not(subwordin(unconvert([op(ends[i]), k, j, op(starts[i])], b), K, b)) then
					temp := [op(temp), k];
				end if;
			end do;
			if nops(temp) = 0 then
				next;
			end if;
			newstarts := [op(newstarts), [j, op(starts[i])]];
			newmiddles := [op(newmiddles), temp];
			newends := [op(newends), ends[i]];
		end do;
	end do;
	return newstarts, newmiddles, newends;
end proc:

exploreend := proc(starts, middles, ends, K, b)
	local newstarts := [];
	local newmiddles := [];
	local newends := [];
	local temp;
	local i, j, k;
	for i from 1 to nops(starts) do
		for j in middles[i] do
			temp := [];
			for k from 0 to b-1 do
				if not(subwordin(unconvert([op(ends[i]), j, k, op(starts[i])], b), K, b)) then
					temp := [op(temp), k];
				end if;
			end do;
			if nops(temp) = 0 then
				next;
			end if;
			newstarts := [op(newstarts), starts[i]];
			newmiddles := [op(newmiddles), temp];
			newends := [op(newends), [op(ends[i]), j]];
		end do;
	end do;
	return newstarts, newmiddles, newends;
end proc:

P3 := proc(K, b, d)
	local starts := [];
	local middles := [];
	local ends := [];
	local temp;
	local i, j, k;

	local newstarts := [];
	local newmiddles := [];
	local newends := [];
	local p;
	local inc;
	local total;

	for i from 1 to b-1 do
		for j from 0 to b-1 do
			if igcd(b, j)>1 then
				next;
			end if;
			if subwordin(unconvert([j, i], b), K, b) then
				next;
			end if;

			starts := [op(starts), [i]];
			ends := [op(ends), [j]];
			temp := [];
			for k from 0 to b-1 do
				if not(subwordin(unconvert([j, k, i], b), K, b)) then
					temp := [op(temp), k];
				end if;
			end do;
			middles := [op(middles), temp];
		end do;
	end do;
	for k from 1 to d do

		newstarts, newmiddles, newends := [], [], [];
		for i from 1 to nops(starts) do
			inc := true;
			for p from 1 to b^2 do
				if isprime(p) then
					if b mod p = 0 then
						if ends[1] mod p = 0 then
							inc := false;
						end if;
					elif (b-1) mod p = 0 then
						total := 0;
						for j in [op(starts[i]), op(ends[i])] do
							total := total + j;
						end do;
						total := total mod p;
						for j in middles[i] mod p do
							total := total + j;
						end do;
						if total = 0 then
							inc := false;
						end if;
					else
					end if;
				end if;
			end do;
			if inc then
				newstarts := [op(newstarts), starts[i]];
				newmiddles := [op(newmiddles), middles[i]];
				newends := [op(newends), ends[i]];
			end if;
		end do;
		starts, middles, ends := newstarts, newmiddles, newends;

		if k = d then
			next;
		end if;

		starts, middles, ends := explorestart(starts, middles, ends, K, b);
		starts, middles, ends := exploreend(starts, middles, ends, K, b);
	end do;

	return starts, middles, ends;
end proc:

familyformat := proc(starts, middles, ends)
	local i, j;
	for i from 1 to nops(starts) do
		for j in ListTools[Reverse](starts[i]) do
			printf("%a", j);
		end do;
		printf("%a*", middles[i]);
		for j in ListTools[Reverse](ends[i]) do
			printf("%a", j);
		end do;
		printf("\n");
	end do;
end proc:
familyformat(P3(K, 10, 1));

K := [2, 3, 5, 7, 11, 19, 41, 61, 89, 409, 449, 499, 881, 991, 6469, 6949, 9001, 9049, 9649, 9949, 60649, 666649, 946669, 60000049, 66000049, 66600049]:
