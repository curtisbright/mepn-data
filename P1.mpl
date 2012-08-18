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
	local middles := [];
	local ends := [];
	local newK := K;
	local temp;
	local i, j, k, l;

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
			if subwordin(unconvert([j, i], b), newK, b) then
				next;
			end if;

			starts := [op(starts), [i]];
			ends := [op(ends), [j]];
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
	for l from 1 to d do

		newstarts, newmiddles, newends := [], [], [];
		for i from 1 to nops(starts) do
			inc := unconvert([op(ends[i]), op(starts[i])], b);
			for j in middles[i] do
				inc := igcd(inc, unconvert([op(ends[i]), j, op(starts[i])], b));
			end do;
			if inc = 1 then
				newstarts := [op(newstarts), starts[i]];
				newmiddles := [op(newmiddles), middles[i]];
				newends := [op(newends), ends[i]];
			else
				printf("Divisible by %a: ", inc);
				familyformat([starts[i]], [middles[i]], [ends[i]]);
			end if;
		end do;
		starts, middles, ends := newstarts, newmiddles, newends;
		if l = d then
			next;
		end if;

		starts, middles, ends, newK := explorestart(starts, middles, ends, newK, b);
		starts, middles, ends, newK := exploreend(starts, middles, ends, newK, b);
	end do;

	return starts, middles, ends, newK;
end proc:

familyformat := proc(starts, middles, ends)
	local i, j;
	for i from 1 to nops(starts) do
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
		printf("\n");
	end do;
end proc:

search := proc(b, d1, d2)
	local K, L;
	local starts, middles, ends;
	local i, j;
	K, L := P1(b, d1);
	#printf("Prime kernel: %a\n", K);
	starts, middles, ends, K := P3(K, b, d2);
	familyformat(starts, middles, ends);
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

search(10, 2, 4);

#K := [2, 3, 5, 7, 11, 19, 41, 61, 89, 409, 449, 499, 881, 991, 6469, 6949, 9001, 9049, 9649, 9949, 60649, 666649, 946669, 60000049, 66000049, 66600049]:
#printf("Prime kernel: %a\n", K);
