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

K := [2, 3, 5, 7, 11, 19, 41, 61, 89, 409, 449, 499, 881, 991, 6469, 6949, 9001, 9049, 9649, 9949, 60649, 666649, 946669, 60000049, 66000049, 66600049]:
