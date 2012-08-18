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

explorestart := proc(starts, middles, ends, K, b)
	local newstarts := [];
	local newmiddles := [];
	local newends := [];
	local newK := K;
	local temp;
	local i, j, k;
	for i from 1 to nops(starts) do
		for j in middles[i] do
			temp := [];
			for k from 0 to b-1 do
				if not(subwordin(unconvert([op(ends[i]), k, j, op(starts[i])], b), newK, b)) then
					if isprime(unconvert([op(ends[i]), k, j, op(starts[i])], b)) then
						#printf("New prime: %a\n", unconvert([op(ends[i]), k, j, op(starts[i])], b));
						newK := [op(newK), unconvert([op(ends[i]), k, j, op(starts[i])], b)];
					else
						temp := [op(temp), k];
					end if;
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
	return newstarts, newmiddles, newends, newK;
end proc:

exploreend := proc(starts, middles, ends, K, b)
	local newstarts := [];
	local newmiddles := [];
	local newends := [];
	local newK := K;
	local temp;
	local i, j, k;
	for i from 1 to nops(starts) do
		for j in middles[i] do
			temp := [];
			for k from 0 to b-1 do
				if not(subwordin(unconvert([op(ends[i]), j, k, op(starts[i])], b), newK, b)) then
					if isprime(unconvert([op(ends[i]), j, k, op(starts[i])], b)) then
						#printf("New prime: %a\n", unconvert([op(ends[i]), j, k, op(starts[i])], b));
						newK := [op(newK), unconvert([op(ends[i]), j, k, op(starts[i])], b)];
					else
						temp := [op(temp), k];
					end if;
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
	return newstarts, newmiddles, newends, newK;
end proc:

P3 := proc(K, b, d)
	local starts := [];
	local startrepeats := [];
	local middles := [];
	local endrepeats := [];
	local ends := [];
	local newK := K;
	local temp;
	local i, j, k, l;

	local newstarts := [];
	local newstartrepeats := [];
	local newmiddles := [];
	local newends := [];
	local newendrepeats := [];
	local p;
	local inc;
	local total;

	for i from 1 to b-1 do
		for j from 0 to b-1 do
			if igcd(b, j)>1 then
				next;
			end if;
			if subwordin(unconvert([j, i], b), newK, b) then
				next;
			end if;

			inc := true;
			for k in newK do
				k := convert(k, base, b);
				if k[-1] = j then
					inc := false;
				end if;
			end do;
			if inc = true then
				startrepeats := [op(startrepeats), [i]];
			else
				starts := [op(starts), [i]];
			end if;
			inc := true;
			for k in newK do
				k := convert(k, base, b);
				if k[1] = i then
					inc := false;
				end if;
			end do;
			if inc = true then
				endrepeats := [op(endrepeats), [j]];
			else
				ends := [op(ends), [j]];
			end if;
			temp := [];
			for k from 0 to b-1 do
				if not(subwordin(unconvert([j, k, i], b), newK, b)) then
					if isprime(unconvert([j, k, i], b)) then
						#printf("New prime: %a\n", unconvert([j, k, i], b));
						newK := [op(newK), unconvert([j, k, i], b)];
					else
						temp := [op(temp), k];
					end if;
				end if;
			end do;
			middles := [op(middles), temp];
		end do;
	end do;
	startrepeats := [startrepeats];
	endrepeats := [endrepeats];
	for l from 1 to d do

		newstarts, newstartrepeats, newmiddles, newends, newendrepeats := [], [], [], [], [];
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
						total := 0;
						for j in middles[i] do
							for k in middles[i] do
								total := total + ((j-k) mod p);
							end do;
						end do;
						for j in middles[i] do
							total := total + ((j*(1/(b-1) mod p)+unconvert(starts[i], b)) mod p);
						end do;
						for j in middles[i] do
							total := total + ((j*b^(nops(ends[i]))*(1/(b-1) mod p)-unconvert(ends[i], b)) mod p);
						end do;
						if total = 0 then
							inc := false;
							#print(starts[i],middles[i],ends[i],p);
						end if;
					end if;
				end if;
			end do;
			if inc then
				newstarts := [op(newstarts), starts[i]];
				newstartrepeats := [op(newstarts), []];
				newmiddles := [op(newmiddles), middles[i]];
				newends := [op(newends), ends[i]];
				newendrepeats := [op(newends), []];
			end if;
		end do;
		starts, startrepeats, middles, ends, endrepeats := newstarts, newstartrepeats, newmiddles, newends, newendrepeats;

		if l = d then
			next;
		end if;

		starts, middles, ends, newK := explorestart(starts, middles, ends, newK, b);
		starts, middles, ends, newK := exploreend(starts, middles, ends, newK, b);
	end do;

	return starts, startrepeats, middles, ends, endrepeats, newK;
end proc:

familyformat := proc(starts, startrepeats, middles, ends, endrepeats)
	local i, j, k;
	for i from 1 to nops(starts) do
		if nops(startrepeats[i]) > 0 then
			printf("[");
			for k in startrepeats[i] do
				if k < 10 then
					printf("%a", k);
				else
					printf("%c", k+55);
				end if;
			end do;
			printf("]*");
		end if;
		for j in ListTools[Reverse](starts[i]) do
			if j < 10 then
				printf("%a", j);
			else
				printf("%c", j+55);
			end if;
		end do;
		#printf("%a*", middles[i]);
		printf("[");
		for j in middles[i] do
			if j < 10 then
				printf("%a", j);
			else
				printf("%c", j+55);
			end if;
		end do;
		printf("]*");
		for j in ListTools[Reverse](ends[i]) do
			if j < 10 then
				printf("%a", j);
			else
				printf("%c", j+55);
			end if;
		end do;
		if nops(endrepeats[i]) > 0 then
			printf("[");
			for k in endrepeats[i] do
				if k < 10 then
					printf("%a", k);
				else
					printf("%c", k+55);
				end if;
			end do;
			printf("]*");
		end if;
		printf("\n");
	end do;
end proc:

search := proc(b, d1, d2)
	local K, L;
	local starts, middles, ends, startrepeats, endrepeats;
	local i, j;
	K, L := P1(b, d1);
	#printf("Prime kernel: %a\n", K);
	starts, startrepeats, middles, ends, endrepeats, K := P3(K, b, d2);
	print(starts, startrepeats, middles, ends, endrepeats);
	familyformat(starts, startrepeats, middles, ends, endrepeats);
	#printf("Prime kernel: %a\n", K);
	K := map(convert, K, base, b);
	printf("Prime kernel: [");
	for i from 1 to nops(K) do
		for j in ListTools[Reverse](K[i]) do
			if j < 10 then
				printf("%a", j);
			else
				printf("%c", j+55);
			end if;
		end do;
		if i <> nops(K) then
			printf(", ");
		end if;
	end do;
	printf("]\n");
end proc:

search(10, 2, 3);

#K := [2, 3, 5, 7, 11, 19, 41, 61, 89, 409, 449, 499, 881, 991, 6469, 6949, 9001, 9049, 9649, 9949, 60649, 666649, 946669, 60000049, 66000049, 66600049]:
#printf("Prime kernel: %a\n", K);
