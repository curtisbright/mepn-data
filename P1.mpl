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

explorestart := proc(starts, startrepeats, middles, ends, endrepeats, K, b)
	local newstarts := [];
	local newstartrepeats := [];
	local newmiddles := [];
	local newends := [];
	local newendrepeats := [];
	local newK := K;
	local temp;
	local i, j, k;
	for i from 1 to nops(starts) do
		for j in middles[i] do
			temp := [];
			for k in middles[i] do
				if not(subwordin(unconvert([op(ends[i]), k, j, op(starts[i])], b), newK, b)) then
					if isprime(unconvert([op(ends[i]), k, j, op(starts[i])], b)) then
						#printf("New prime: %a\n", unconvert([op(ends[i]), k, j, op(starts[i])], b));
						newK := [op(newK), unconvert([op(ends[i]), k, j, op(starts[i])], b)];
					else
						temp := [op(temp), k];
					end if;
				end if;
			end do;
			#if nops(temp) = 0 then
			#	next;
			#end if;
			newstarts := [op(newstarts), [j, op(starts[i])]];
			newstartrepeats := [op(newstartrepeats), [[], op(startrepeats[i])]];
			newmiddles := [op(newmiddles), temp];
			newends := [op(newends), ends[i]];
			newendrepeats := [op(newendrepeats), endrepeats[i]];
		end do;
	end do;
	return newstarts, newstartrepeats, newmiddles, newends, newendrepeats, newK;
end proc:

exploreend := proc(starts, startrepeats, middles, ends, endrepeats, K, b)
	local newstarts := [];
	local newstartrepeats := [];
	local newmiddles := [];
	local newends := [];
	local newendrepeats := [];
	local newK := K;
	local temp;
	local i, j, k;
	for i from 1 to nops(starts) do
		for j in middles[i] do
			temp := [];
			for k in middles[i] do
				if not(subwordin(unconvert([op(ends[i]), j, k, op(starts[i])], b), newK, b)) then
					if isprime(unconvert([op(ends[i]), j, k, op(starts[i])], b)) then
						#printf("New prime: %a\n", unconvert([op(ends[i]), j, k, op(starts[i])], b));
						newK := [op(newK), unconvert([op(ends[i]), j, k, op(starts[i])], b)];
					else
						temp := [op(temp), k];
					end if;
				end if;
			end do;
			#if nops(temp) = 0 then
			#	next;
			#end if;
			newstarts := [op(newstarts), starts[i]];
			newstartrepeats := [op(newstartrepeats), startrepeats[i]];
			newmiddles := [op(newmiddles), temp];
			newends := [op(newends), [op(ends[i]), j]];
			newendrepeats := [op(newendrepeats), [op(endrepeats[i]), []]];
		end do;
	end do;
	return newstarts, newstartrepeats, newmiddles, newends, newendrepeats, newK;
end proc:

split := proc(starts, startrepeats, middles, ends, endrepeats, K, b)
	local newstarts := [];
	local newstartrepeats := [];
	local newmiddles := [];
	local newends := [];
	local newendrepeats := [];
	local newK := K;
	local temp;
	local i, j, k;
	for i from 1 to nops(starts) do
		temp := true;
		if nops(middles[i]) > 1 then
			for j in middles[i] do
				if temp = false then
					next;
				end if;
				if not(subwordin(unconvert([op(ends[i]), j, j, op(starts[i])], b), newK, b)) then
					if isprime(unconvert([op(ends[i]), j, j, op(starts[i])], b)) then
						temp := false;
						newK := [op(newK), unconvert([op(ends[i]), j, j, op(starts[i])], b)];

						newstarts := [op(newstarts), starts[i]];
						newstartrepeats := [op(newstartrepeats), startrepeats[i]];
						newmiddles := [op(newmiddles), [({middles[i][]} minus {j})[]]];
						newends := [op(newends), ends[i]];
						newendrepeats := [op(newendrepeats), endrepeats[i]];

						newstarts := [op(newstarts), [j, op(starts[i])]];
						newstartrepeats := [op(newstartrepeats), [[({middles[i][]} minus {j})[]] , op(startrepeats[i])]];
						newmiddles := [op(newmiddles), [({middles[i][]} minus {j})[]]];
						newends := [op(newends), ends[i]];
						newendrepeats := [op(newendrepeats), endrepeats[i]];

						#printf("SPLIT\n");
						#familyformat([starts[i]], [startrepeats[i]], [middles[i]], [ends[i]], [endrepeats[i]]);
						#printf("INTO\n");
						#familyformat(newstarts[-2..-1], newstartrepeats[-2..-1], newmiddles[-2..-1], newends[-2..-1], newendrepeats[-2..-1]);
					end if;
				else
					temp := false;

					newstarts := [op(newstarts), starts[i]];
					newstartrepeats := [op(newstartrepeats), startrepeats[i]];
					newmiddles := [op(newmiddles), [({middles[i][]} minus {j})[]]];
					newends := [op(newends), ends[i]];
					newendrepeats := [op(newendrepeats), endrepeats[i]];

					newstarts := [op(newstarts), [j, op(starts[i])]];
					newstartrepeats := [op(newstartrepeats), [[({middles[i][]} minus {j})[]] , op(startrepeats[i])]];
					newmiddles := [op(newmiddles), [({middles[i][]} minus {j})[]]];
					newends := [op(newends), ends[i]];
					newendrepeats := [op(newendrepeats), endrepeats[i]];

					#printf("SPLIT\n");
					#familyformat([starts[i]], [startrepeats[i]], [middles[i]], [ends[i]], [endrepeats[i]]);
					#printf("INTO\n");
					#familyformat(newstarts[-2..-1], newstartrepeats[-2..-1], newmiddles[-2..-1], newends[-2..-1], newendrepeats[-2..-1]);
				end if;
			end do;
		end if;
		if temp = true then
			newstarts := [op(newstarts), starts[i]];
			newstartrepeats := [op(newstartrepeats), startrepeats[i]];
			newmiddles := [op(newmiddles), middles[i]];
			newends := [op(newends), ends[i]];
			newendrepeats := [op(newendrepeats), endrepeats[i]];
		end if;
	end do;
	#newstarts, newstartrepeats, newmiddles, newends, newendrepeats, newK := simplifyrepeats(newstarts, newstartrepeats, newmiddles, newends, newendrepeats, newK, b);
	return newstarts, newstartrepeats, newmiddles, newends, newendrepeats, newK;
end proc:

split2 := proc(starts, startrepeats, middles, ends, endrepeats, K, b)
	local newstarts := [];
	local newstartrepeats := [];
	local newmiddles := [];
	local newends := [];
	local newendrepeats := [];
	local newK := K;
	local temp;
	local i, j, k;
	for i from 1 to nops(starts) do
		temp := true;
		for j in middles[i] do
			for k in middles[i] do
				if #temp = false or 
				j>=k then
					next;
				end if;
				if not(subwordin(unconvert([op(ends[i]), j, k, op(starts[i])], b), newK, b)) then
					if isprime(unconvert([op(ends[i]), j, k, op(starts[i])], b)) then
						newK := [op(newK), unconvert([op(ends[i]), j, k, op(starts[i])], b)];
					end if;
				elif not(subwordin(unconvert([op(ends[i]), k, j, op(starts[i])], b), newK, b)) then
					if isprime(unconvert([op(ends[i]), k, j, op(starts[i])], b)) then
						newK := [op(newK), unconvert([op(ends[i]), k, j, op(starts[i])], b)];
					end if;
				else
					temp := false;
					newstarts := [op(newstarts), starts[i]];
					newstartrepeats := [op(newstartrepeats), startrepeats[i]];
					newmiddles := [op(newmiddles), [({middles[i][]} minus {j})[]]];
					newends := [op(newends), ends[i]];
					newendrepeats := [op(newendrepeats), endrepeats[i]];

					newstarts := [op(newstarts), starts[i]];
					newstartrepeats := [op(newstartrepeats), startrepeats[i]];
					newmiddles := [op(newmiddles), [({middles[i][]} minus {k})[]]];
					newends := [op(newends), ends[i]];
					newendrepeats := [op(newendrepeats), endrepeats[i]];
				end if;
			end do;
		end do;
		if temp = true then
			newstarts := [op(newstarts), starts[i]];
			newstartrepeats := [op(newstartrepeats), startrepeats[i]];
			newmiddles := [op(newmiddles), middles[i]];
			newends := [op(newends), ends[i]];
			newendrepeats := [op(newendrepeats), endrepeats[i]];
		end if;
	end do;
	return removedups(newstarts, newstartrepeats, newmiddles, newends, newendrepeats), newK;
end proc:

split3 := proc(starts, startrepeats, middles, ends, endrepeats, K, b)
	local newstarts := starts;
	local newstartrepeats := startrepeats;
	local newmiddles := middles;
	local newends := ends;
	local newendrepeats := endrepeats;
	local newK := K;
	newstarts, newstartrepeats, newmiddles, newends, newendrepeats, newK := split2(starts, startrepeats, middles, ends, endrepeats, newK, b);
	#newstarts, newstartrepeats, newmiddles, newends, newendrepeats, newK := split2(starts, startrepeats, middles, ends, endrepeats, newK, b);
	#newstarts, newstartrepeats, newmiddles, newends, newendrepeats, newK := split2(starts, startrepeats, middles, ends, endrepeats, newK, b);
	return newstarts, newstartrepeats, newmiddles, newends, newendrepeats, newK;
end proc:

removedups := proc(starts, startrepeats, middles, ends, endrepeats)
	local newstarts := [];
	local newstartrepeats := [];
	local newmiddles := [];
	local newends := [];
	local newendrepeats := [];
	local i, j;
	local same;
	for i from 1 to nops(starts) do
		same := false;
		for j from 1 to i-1 do
			if starts[i]=starts[j] and startrepeats[i]=startrepeats[j] and middles[i]=middles[j] and ends[i]=ends[j] and endrepeats[i]=endrepeats[j] then
				same := true;
			end if;
		end do;
		if same = false then
			newstarts := [op(newstarts), starts[i]];
			newstartrepeats := [op(newstartrepeats), startrepeats[i]];
			newmiddles := [op(newmiddles), middles[i]];
			newends := [op(newends), ends[i]];
			newendrepeats := [op(newendrepeats), endrepeats[i]];
		end if;
	end do;
	return newstarts, newstartrepeats, newmiddles, newends, newendrepeats;
end proc:

simplifyrepeats := proc(starts, startrepeats, middles, ends, endrepeats, K, b)
	local newstarts := [];
	local newstartrepeats := [];
	local newmiddles := [];
	local newends := [];
	local newendrepeats := [];
	local newK := K;
	local temprepeats, temp;
	local i, j, k;
	for i from 1 to nops(starts) do
		temprepeats := [];
		for j from 1 to nops(startrepeats[i]) do
			temp := [];
			for k in startrepeats[i][j] do
				if not(subwordin(unconvert([op(ends[i]), [op(starts[i])][1..j][], k, [op(starts[i])][j+1..-1][]], b), newK, b)) then
					if isprime(unconvert([op(ends[i]), [op(starts[i])][1..j][], k, [op(starts[i])][j+1..-1][]], b)) then
						newK := [op(newK), unconvert([op(ends[i]), [op(starts[i])][1..j][], k, [op(starts[i])][j+1..-1][]], b)];
					else
						temp := [op(temp), k];
					end if;
				end if;
			end do;
			temprepeats := [op(temprepeats), temp];
		end do;
		newstarts := [op(newstarts), starts[i]];
		newstartrepeats := [op(newstartrepeats), temprepeats];
		newmiddles := [op(newmiddles), middles[i]];
		newends := [op(newends), ends[i]];
		newendrepeats := [op(newendrepeats), endrepeats[i]];
	end do;
	return newstarts, newstartrepeats, newmiddles, newends, newendrepeats, newK;
end proc:

P3 := proc(K, b, d)
	local starts := [];
	local startrepeats := [];
	local middles := [];
	local endrepeats := [];
	local ends := [];
	local newK := K;
	local temp;
	local i, j, k, l, m;

	local newstarts := [];
	local newstartrepeats := [];
	local newmiddles := [];
	local newends := [];
	local newendrepeats := [];
	local p;
	local inc;
	local inc1, inc2;
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
			startrepeats := [op(startrepeats), [[]]];
			ends := [op(ends), [j]];
			endrepeats := [op(endrepeats), [[]]];
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
	#startrepeats := [startrepeats];
	#endrepeats := [endrepeats];
	for l from 1 to d do
		printf("Trying depth %a...\n", l);

		newstarts, newstartrepeats, newmiddles, newends, newendrepeats := [], [], [], [], [];
		for i from 1 to nops(starts) do
			inc := unconvert([op(ends[i]), op(starts[i])], b);
			for j in middles[i] do
				inc := igcd(inc, unconvert([op(ends[i]), j, op(starts[i])], b));
			end do;
			for j from 1 to nops(startrepeats[i]) do
				for k in startrepeats[i][j] do
					#familyformat([starts[i]], [startrepeats[i]], [middles[i]], [ends[i]], [endrepeats[i]]);
					#print([op(ends[i]), [op(starts[i])][1..j][], k, [op(starts[i])][j+1..-1][]]);
					inc := igcd(inc, unconvert([op(ends[i]), [op(starts[i])][1..j][], k, [op(starts[i])][j+1..-1][]], b));
				end do;
			end do;
			if inc = 1 then
				inc1 := unconvert([op(ends[i]), op(starts[i])], b);
				for j in middles[i] do
					for k in middles[i] do
						inc1 := igcd(inc1, unconvert([op(ends[i]), j, k, op(starts[i])], b));
					end do;
				end do;
				inc2 := unconvert([op(ends[i]), middles[i][1], op(starts[i])], b);
				for j in middles[i] do
					inc2 := igcd(inc2, unconvert([op(ends[i]), j, op(starts[i])], b));
				end do;
				for j in middles[i] do
					for k in middles[i] do
						for m in middles[i] do
							inc2 := igcd(inc2, unconvert([op(ends[i]), j, k, m, op(starts[i])], b));
						end do;
					end do;
				end do;
				inc := min(inc1, inc2);
				#if inc > 1 then
				#	printf("Divisible by %a and %a: ", inc1, inc2);
				#	familyformat([starts[i]], [startrepeats[i]], [middles[i]], [ends[i]], [endrepeats[i]]);					
				#end if;
			end if;
			if inc = 1 then
				newstarts := [op(newstarts), starts[i]];
				newstartrepeats := [op(newstartrepeats), startrepeats[i]];
				newmiddles := [op(newmiddles), middles[i]];
				newends := [op(newends), ends[i]];
				newendrepeats := [op(newendrepeats), endrepeats[i]];
			else
				#printf("Divisible by %a: ", inc);
				#familyformat([starts[i]], [startrepeats[i]], [middles[i]], [ends[i]], [endrepeats[i]]);
			end if;
		end do;
		starts, startrepeats, middles, ends, endrepeats := newstarts, newstartrepeats, newmiddles, newends, newendrepeats;

		if l = d then
			next;
		end if;

		#starts, startrepeats, middles, ends, endrepeats, newK := split(starts, startrepeats, middles, ends, endrepeats, newK, b);

		starts, startrepeats, middles, ends, endrepeats, newK := split3(starts, startrepeats, middles, ends, endrepeats, newK, b);
		starts, startrepeats, middles, ends, endrepeats, newK := explorestart(starts, startrepeats, middles, ends, endrepeats, newK, b);
		starts, startrepeats, middles, ends, endrepeats, newK := split3(starts, startrepeats, middles, ends, endrepeats, newK, b);
		starts, startrepeats, middles, ends, endrepeats, newK := exploreend(starts, startrepeats, middles, ends, endrepeats, newK, b);
		starts, startrepeats, middles, ends, endrepeats, newK := split3(starts, startrepeats, middles, ends, endrepeats, newK, b);
	end do;

	return starts, startrepeats, middles, ends, endrepeats, newK;
end proc:

familyformat := proc(starts, startrepeats, middles, ends, endrepeats)
	local i, j, k, c;
	for i from 1 to nops(starts) do
		c := nops(starts[i])+1;
		for j in ListTools[Reverse](starts[i]) do
			c := c-1;
			if nops(startrepeats[i][c]) > 0 then
				printf("[");
				for k in startrepeats[i][c] do
					if k < 10 then
						printf("%a", k);
					else
						printf("%c", k+55);
					end if;
				end do;
				printf("]*");
			end if;
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
		c := nops(ends[i])+1;
		for j in ListTools[Reverse](ends[i]) do
			c := c-1;
			if j < 10 then
				printf("%a", j);
			else
				printf("%c", j+55);
			end if;
			if nops(endrepeats[i][c]) > 0 then
				printf("[");
				for k in endrepeats[i][c] do
					if k < 10 then
						printf("%a", k);
					else
						printf("%c", k+55);
					end if;
				end do;
				printf("]*");
			end if;
		end do;
		printf("\n");
	end do;
end proc:

ffamilyformat := proc(fd, starts, startrepeats, middles, ends, endrepeats)
	local i, j, k, c;
	for i from 1 to nops(starts) do
		c := nops(starts[i])+1;
		for j in ListTools[Reverse](starts[i]) do
			c := c-1;
			if nops(startrepeats[i][c]) > 0 then
				fprintf(fd, "[");
				for k in startrepeats[i][c] do
					if k < 10 then
						fprintf(fd, "%a", k);
					else
						fprintf(fd, "%c", k+55);
					end if;
				end do;
				fprintf(fd, "]*");
			end if;
			if j < 10 then
				fprintf(fd, "%a", j);
			else
				fprintf(fd, "%c", j+55);
			end if;
		end do;
		#fprintf(fd, "%a*", middles[i]);
		fprintf(fd, "[");
		for j in middles[i] do
			if j < 10 then
				fprintf(fd, "%a", j);
			else
				fprintf(fd, "%c", j+55);
			end if;
		end do;
		fprintf(fd, "]*");
		c := nops(ends[i])+1;
		for j in ListTools[Reverse](ends[i]) do
			c := c-1;
			if j < 10 then
				fprintf(fd, "%a", j);
			else
				fprintf(fd, "%c", j+55);
			end if;
			if nops(endrepeats[i][c]) > 0 then
				fprintf(fd, "[");
				for k in endrepeats[i][c] do
					if k < 10 then
						fprintf(fd, "%a", k);
					else
						fprintf(fd, "%c", k+55);
					end if;
				end do;
				fprintf(fd, "]*");
			end if;
		end do;
		fprintf(fd, "\n");
	end do;
end proc:

search := proc(b, d1, d2)
	local K, L;
	local starts, middles, ends, startrepeats, endrepeats;
	local i, j;
	K, L := P1(b, d1);
	#printf("Prime kernel: %a\n", K);
	starts, startrepeats, middles, ends, endrepeats, K := P3(K, b, d2);
	#print(starts, startrepeats, middles, ends, endrepeats);
	#if nops(starts) > 0 then
	#	printf("Unsolved families:\n");
	#	familyformat(starts, startrepeats, middles, ends, endrepeats);
	#end if;
	#printf("Prime kernel: %a\n", K);
	K := map(convert, K, base, b);
	#printf("Prime kernel: [");
	#for i from 1 to nops(K) do
	#	for j in ListTools[Reverse](K[i]) do
	#		if j < 10 then
	#			printf("%a", j);
	#		else
	#			printf("%c", j+55);
	#		end if;
	#	end do;
	#	if i <> nops(K) then
	#		printf(", ");
	#	end if;
	#end do;
	printf("]\n");
	return starts, startrepeats, middles, ends, endrepeats, map(unconvert, K, b);
end proc:

fd2 := fopen("bases.txt", WRITE);
fprintf(fd2, "Base\tSize\tWidth\n");
fclose(fd2);
for b from 2 to 50 do
	printf("Starting base %a...\n", b);
	starts, startrepeats, middles, ends, endrepeats, K := search(b, 3, 100);
	fd := fopen("base" || b || ".txt", WRITE);
	if nops(starts) > 0 then
		fprintf(fd, "Unsolved families:\n");
		ffamilyformat(fd, starts, startrepeats, middles, ends, endrepeats);
	end if;
	newK := map(convert, K, base, b);
	fprintf(fd, "Prime kernel: [");
	for i from 1 to nops(newK) do
		for j in ListTools[Reverse](newK[i]) do
			if j < 10 then
				fprintf(fd, "%a", j);
			else
				fprintf(fd, "%c", j+55);
			end if;
		end do;
		if i <> nops(newK) then
			fprintf(fd, ", ");
		end if;
	end do;
	fprintf(fd, "]\n");
	fprintf(fd, "Size: %a\n", nops(newK));
	fprintf(fd, "Width: %a\n", max(map(nops, newK)));
	fclose(fd);
	fd2 := fopen("bases.txt", APPEND);
	fprintf(fd2, "%a\t%a\t%a\n", b, nops(newK), max(map(nops, newK)));
	fclose(fd2);
end do;

#search(10, 2, 4);
#
#K := [2, 3, 5, 7, 11, 19, 41, 61, 89, 409, 449, 499, 881, 991, 6469, 6949, 9001, 9049, 9649, 9949, 60649, 666649, 946669, 60000049, 66000049, 66600049]:
#printf("Prime kernel: %a\n", K);
