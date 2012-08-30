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

exploreleft := proc(starts, startrepeats, middles, ends, endrepeats, K, b)
	local newstarts := [];
	local newstartrepeats := [];
	local newmiddles := [];
	local newends := [];
	local newendrepeats := [];
	local newK := K;
	local temp;
	local i, j, k, l;
	printf("Exploring left...\n");
	for i from 1 to nops(starts) do
		#if nops(middles[i]) > 0 then
			for j in middles[i] do
				if not(subwordin(unconvert([op(ends[i]), j, op(starts[i])], b), newK, b)) then
					if isprime(unconvert([op(ends[i]), j, op(starts[i])], b)) then
						newK := [op(newK), unconvert([op(ends[i]), j, op(starts[i])], b)];
					end if;

					temp := [];
					for l in middles[i] do
						if not(subwordin(unconvert([op(ends[i]), l, j, op(starts[i])], b), newK, b)) then
							if isprime(unconvert([op(ends[i]), l, j, op(starts[i])], b)) then
								newK := [op(newK), unconvert([op(ends[i]), l, j, op(starts[i])], b)];
							else
								temp := [op(temp), l];
							end if;
						end if;
					end do;
					#printf("EXPLORED: ");
					#singleformat([j, op(starts[i])], [[], op(startrepeats[i])], temp, ends[i], endrepeats[i]);
					#printf("\n");
					if checkfordivisors([j, op(starts[i])], [[], op(startrepeats[i])], temp, ends[i], endrepeats[i], b) = false then
						newstarts := [op(newstarts), [j, op(starts[i])]];
						newstartrepeats := [op(newstartrepeats), [[], op(startrepeats[i])]];
						newmiddles := [op(newmiddles), temp];
						newends := [op(newends), ends[i]];
						newendrepeats := [op(newendrepeats), endrepeats[i]];
					end if;

				end if;
			end do;
		#else
			#printf("exploring repeats for ");
			singleformat(starts[i], startrepeats[i], middles[i], ends[i], endrepeats[i]);
			printf("\n");
			for k from 1 to nops(startrepeats[i]) do
				if nops(startrepeats[i][k]) > 0 then
					for j in startrepeats[i][k] do
						#print([op(ends[i]), [op(starts[i])][1..k][], j, [op(starts[i])][k+1..-1][]], subwordin(unconvert([op(ends[i]), [op(starts[i])][1..k][], j, [op(starts[i])][k+1..-1][]], b), newK, b), newK);
						if not(subwordin(unconvert([op(ends[i]), [op(starts[i])][1..k][], j, [op(starts[i])][k+1..-1][]], b), newK, b)) then
							if isprime(unconvert([op(ends[i]), [op(starts[i])][1..k][], j, [op(starts[i])][k+1..-1][]], b)) then
								newK := [op(newK), unconvert([op(ends[i]), [op(starts[i])][1..k][], j, [op(starts[i])][k+1..-1][]], b)];
							end if;
							temp := [];
							for l in startrepeats[i][k] do
								if not(subwordin(unconvert([op(ends[i]), [op(starts[i])][1..k][], l, j, [op(starts[i])][k+1..-1][]], b), newK, b)) then
									if isprime(unconvert([op(ends[i]), [op(starts[i])][1..k][], l, j, [op(starts[i])][k+1..-1][]], b)) then
										newK := [op(newK), unconvert([op(ends[i]), [op(starts[i])][1..k][], l, j, [op(starts[i])][k+1..-1][]], b)];
									else
										temp := [op(temp), l];
									end if;
								end if;
							end do;
							#singleformat([op(starts[i][1..k]), j, op(starts[i][k+1..-1])], [op(startrepeats[i][1..k-1]), temp, [], op(startrepeats[i][k+1..-1])], [], ends[i], endrepeats[i]);
							#printf("\n");
							if checkfordivisors([op(starts[i][1..k]), j, op(starts[i][k+1..-1])], [op(startrepeats[i][1..k-1]), temp, [], op(startrepeats[i][k+1..-1])], [], ends[i], endrepeats[i], b) = false then
								newstarts := [op(newstarts), [op(starts[i][1..k]), j, op(starts[i][k+1..-1])]];
								newstartrepeats := [op(newstartrepeats), [op(startrepeats[i][1..k-1]), temp, [], op(startrepeats[i][k+1..-1])]];
								newmiddles := [op(newmiddles), []];
								newends := [op(newends), ends[i]];
								newendrepeats := [op(newendrepeats), endrepeats[i]];
							end if;
						end if;
					end do;
					#k = nops(startrepeats[i]);
				end if;
			end do;

		#end if;

	end do;

	return newstarts, newstartrepeats, newmiddles, newends, newendrepeats, newK;
end proc:

exploreright := proc(starts, startrepeats, middles, ends, endrepeats, K, b)
	local newstarts := [];
	local newstartrepeats := [];
	local newmiddles := [];
	local newends := [];
	local newendrepeats := [];
	local newK := K;
	local temp;
	local i, j, k, l;
	printf("Exploring right...\n");
	for i from 1 to nops(starts) do
		for j in middles[i] do
			if not(subwordin(unconvert([op(ends[i]), j, op(starts[i])], b), newK, b)) then
				if isprime(unconvert([op(ends[i]), j, op(starts[i])], b)) then
					newK := [op(newK), unconvert([op(ends[i]), j, op(starts[i])], b)];
				end if;

				temp := [];
				for l in middles[i] do
					if not(subwordin(unconvert([op(ends[i]), j, l, op(starts[i])], b), newK, b)) then
						if isprime(unconvert([op(ends[i]), j, l, op(starts[i])], b)) then
							newK := [op(newK), unconvert([op(ends[i]), j, l, op(starts[i])], b)];
						else
							temp := [op(temp), l];
						end if;
					end if;
				end do;
				if checkfordivisors(starts[i], startrepeats[i], temp, [op(ends[i]), j], [op(endrepeats[i]), []], b) = false then
					newstarts := [op(newstarts), starts[i]];
					newstartrepeats := [op(newstartrepeats), startrepeats[i]];
					newmiddles := [op(newmiddles), temp];
					newends := [op(newends), [op(ends[i]), j]];
					newendrepeats := [op(newendrepeats), [op(endrepeats[i]), []]];
				end if;

			end if;
		end do;

		for k from 1 to nops(startrepeats[i]) do
			if nops(startrepeats[i][k]) > 0 then
				for j in startrepeats[i][k] do
					if not(subwordin(unconvert([op(ends[i]), [op(starts[i])][1..k][], j, [op(starts[i])][k+1..-1][]], b), newK, b)) then
						if isprime(unconvert([op(ends[i]), [op(starts[i])][1..k][], j, [op(starts[i])][k+1..-1][]], b)) then
							newK := [op(newK), unconvert([op(ends[i]), [op(starts[i])][1..k][], j, [op(starts[i])][k+1..-1][]], b)];
						end if;
						temp := [];
						for l in startrepeats[i][k] do
							if not(subwordin(unconvert([op(ends[i]), [op(starts[i])][1..k][], j, l, [op(starts[i])][k+1..-1][]], b), newK, b)) then
								if isprime(unconvert([op(ends[i]), [op(starts[i])][1..k][], j, l, [op(starts[i])][k+1..-1][]], b)) then
									newK := [op(newK), unconvert([op(ends[i]), [op(starts[i])][1..k][], j, l, [op(starts[i])][k+1..-1][]], b)];
								else
									temp := [op(temp), l];
								end if;
							end if;
						end do;

						if checkfordivisors([op(starts[i][1..k]), j, op(starts[i][k+1..-1])], [op(startrepeats[i][1..k-1]), [], temp, op(startrepeats[i][k+1..-1])], [], ends[i], endrepeats[i], b) = false then
							newstarts := [op(newstarts), [op(starts[i][1..k]), j, op(starts[i][k+1..-1])]];
							newstartrepeats := [op(newstartrepeats), [op(startrepeats[i][1..k-1]), [], temp, op(startrepeats[i][k+1..-1])]];
							newmiddles := [op(newmiddles), []];
							newends := [op(newends), ends[i]];
							newendrepeats := [op(newendrepeats), endrepeats[i]];
						end if;
					end if;
				end do;
			end if;
		end do;
	end do;

	return newstarts, newstartrepeats, newmiddles, newends, newendrepeats, newK;
end proc:

alreadyin := proc(starts, startrepeats, middles, ends, endrepeats, thisstart, thisstartrepeat, thismiddle, thisend, thisendrepeat)
	local i;
	for i from 1 to nops(starts) do
		if starts[i]=thisstart and startrepeats[i]=thisstartrepeat and middles[i]=thismiddle and ends[i]=thisend and endrepeats[i]=thisendrepeat then
			return true;
		end if;
	end do;
	return false;
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
	local didsplit := false;
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
						#printf("Split new prime: ");
						#printprime([op(ends[i]), j, k, op(starts[i])]);
						#printf("\n");
						newK := [op(newK), unconvert([op(ends[i]), j, k, op(starts[i])], b)];
					end if;
				elif not(subwordin(unconvert([op(ends[i]), k, j, op(starts[i])], b), newK, b)) then
					if isprime(unconvert([op(ends[i]), k, j, op(starts[i])], b)) then
						newK := [op(newK), unconvert([op(ends[i]), k, j, op(starts[i])], b)];
					end if;
				else
					singleformat(starts[i], startrepeats[i], middles[i], ends[i], endrepeats[i]);
					printf(" splits into ");
					singleformat(starts[i], startrepeats[i], [({middles[i][]} minus {j})[]], ends[i], endrepeats[i]);
					printf(" and ");
					singleformat(starts[i], startrepeats[i], [({middles[i][]} minus {k})[]], ends[i], endrepeats[i]);

					temp := false;
					if checkfordivisors(starts[i], startrepeats[i], [({middles[i][]} minus {j})[]], ends[i], endrepeats[i], b) = false then
						if not(alreadyin(newstarts, newstartrepeats, newmiddles, newends, newendrepeats, starts[i], startrepeats[i], [({middles[i][]} minus {j})[]], ends[i], endrepeats[i])) then
							newstarts := [op(newstarts), starts[i]];
							newstartrepeats := [op(newstartrepeats), startrepeats[i]];
							newmiddles := [op(newmiddles), [({middles[i][]} minus {j})[]]];
							newends := [op(newends), ends[i]];
							newendrepeats := [op(newendrepeats), endrepeats[i]];
							didsplit := true;
						#else
						#	singleformat(starts[i], startrepeats[i], [({middles[i][]} minus {j})[]], ends[i], endrepeats[i]);
						#	printf(" ALREADY IN\n");
						end if;
					else
						printf(" (1st excluded)");
					end if;

					if checkfordivisors(starts[i], startrepeats[i], [({middles[i][]} minus {k})[]], ends[i], endrepeats[i], b) = false then
						if not(alreadyin(newstarts, newstartrepeats, newmiddles, newends, newendrepeats, starts[i], startrepeats[i], [({middles[i][]} minus {k})[]], ends[i], endrepeats[i])) then
							newstarts := [op(newstarts), starts[i]];
							newstartrepeats := [op(newstartrepeats), startrepeats[i]];
							newmiddles := [op(newmiddles), [({middles[i][]} minus {k})[]]];
							newends := [op(newends), ends[i]];
							newendrepeats := [op(newendrepeats), endrepeats[i]];
							didsplit := true;
						#else
						#	singleformat(starts[i], startrepeats[i], [({middles[i][]} minus {k})[]], ends[i], endrepeats[i]);
						#	printf(" ALREADY IN\n");
						end if;
					else
						printf(" (2nd excluded)");
					end if;
					printf("\n");
				end if;
			end do;
		end do;
		if temp = true then
			if checkfordivisors(starts[i], startrepeats[i], middles[i], ends[i], endrepeats[i], b) = false then
				if not(alreadyin(newstarts, newstartrepeats, newmiddles, newends, newendrepeats, starts[i], startrepeats[i], middles[i], ends[i], endrepeats[i])) then
					newstarts := [op(newstarts), starts[i]];
					newstartrepeats := [op(newstartrepeats), startrepeats[i]];
					newmiddles := [op(newmiddles), middles[i]];
					newends := [op(newends), ends[i]];
					newendrepeats := [op(newendrepeats), endrepeats[i]];
				end if;
			end if;
		end if;
	end do;
	return newstarts, newstartrepeats, newmiddles, newends, newendrepeats, newK, didsplit;
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
		if nops(middles[i]) > 1 then
			for j in middles[i] do
				if temp = false then
					next;
				end if;
				if not(subwordin(unconvert([op(ends[i]), j, j, op(starts[i])], b), newK, b)) then
					if isprime(unconvert([op(ends[i]), j, j, op(starts[i])], b)) then
						temp := false;
						newK := [op(newK), unconvert([op(ends[i]), j, j, op(starts[i])], b)];

						singleformat(starts[i], startrepeats[i], middles[i], ends[i], endrepeats[i]);
						printf(" splits into ");
						singleformat(starts[i], startrepeats[i], [({middles[i][]} minus {j})[]], ends[i], endrepeats[i]);
						printf(" and ");
						singleformat([j, op(starts[i])], [[({middles[i][]} minus {j})[]] , op(startrepeats[i])], [({middles[i][]} minus {j})[]], ends[i], endrepeats[i]);

						if checkfordivisors(starts[i], startrepeats[i], [({middles[i][]} minus {j})[]], ends[i], endrepeats[i], b) = false then
							if not(alreadyin(newstarts, newstartrepeats, newmiddles, newends, newendrepeats, starts[i], startrepeats[i], [({middles[i][]} minus {j})[]], ends[i], endrepeats[i])) then
								newstarts := [op(newstarts), starts[i]];
								newstartrepeats := [op(newstartrepeats), startrepeats[i]];
								newmiddles := [op(newmiddles), [({middles[i][]} minus {j})[]]];
								newends := [op(newends), ends[i]];
								newendrepeats := [op(newendrepeats), endrepeats[i]];
							end if;
						else
							printf(" (1st excluded)");
						end if;

						if checkfordivisors([j, op(starts[i])], [[({middles[i][]} minus {j})[]] , op(startrepeats[i])], [({middles[i][]} minus {j})[]], ends[i], endrepeats[i], b) = false then
							if not(alreadyin(newstarts, newstartrepeats, newmiddles, newends, newendrepeats, [j, op(starts[i])], [[({middles[i][]} minus {j})[]] , op(startrepeats[i])], [({middles[i][]} minus {j})[]], ends[i], endrepeats[i])) then
								newstarts := [op(newstarts), [j, op(starts[i])]];
								newstartrepeats := [op(newstartrepeats), [[({middles[i][]} minus {j})[]] , op(startrepeats[i])]];
								newmiddles := [op(newmiddles), [({middles[i][]} minus {j})[]]];
								newends := [op(newends), ends[i]];
								newendrepeats := [op(newendrepeats), endrepeats[i]];
							end if;
						else
							printf(" (2nd excluded)");
						end if;
						printf("\n");

					end if;
				else
					temp := false;

					singleformat(starts[i], startrepeats[i], middles[i], ends[i], endrepeats[i]);
					printf(" splits into ");
					singleformat(starts[i], startrepeats[i], [({middles[i][]} minus {j})[]], ends[i], endrepeats[i]);
					printf(" and ");
					singleformat([j, op(starts[i])], [[({middles[i][]} minus {j})[]] , op(startrepeats[i])], [({middles[i][]} minus {j})[]], ends[i], endrepeats[i]);
					printf("\n");

					if checkfordivisors(starts[i], startrepeats[i], [({middles[i][]} minus {j})[]], ends[i], endrepeats[i], b) = false then
						if not(alreadyin(newstarts, newstartrepeats, newmiddles, newends, newendrepeats, starts[i], startrepeats[i], [({middles[i][]} minus {j})[]], ends[i], endrepeats[i])) then
							newstarts := [op(newstarts), starts[i]];
							newstartrepeats := [op(newstartrepeats), startrepeats[i]];
							newmiddles := [op(newmiddles), [({middles[i][]} minus {j})[]]];
							newends := [op(newends), ends[i]];
							newendrepeats := [op(newendrepeats), endrepeats[i]];
						end if;
					end if;

					if checkfordivisors([j, op(starts[i])], [[({middles[i][]} minus {j})[]] , op(startrepeats[i])], [({middles[i][]} minus {j})[]], ends[i], endrepeats[i], b) = false then
						if not(alreadyin(newstarts, newstartrepeats, newmiddles, newends, newendrepeats, [j, op(starts[i])], [[({middles[i][]} minus {j})[]] , op(startrepeats[i])], [({middles[i][]} minus {j})[]], ends[i], endrepeats[i])) then
							newstarts := [op(newstarts), [j, op(starts[i])]];
							newstartrepeats := [op(newstartrepeats), [[({middles[i][]} minus {j})[]] , op(startrepeats[i])]];
							newmiddles := [op(newmiddles), [({middles[i][]} minus {j})[]]];
							newends := [op(newends), ends[i]];
							newendrepeats := [op(newendrepeats), endrepeats[i]];
						end if;
					end if;

				end if;
			end do;
		end if;
		if temp = true then
			if checkfordivisors(starts[i], startrepeats[i], middles[i], ends[i], endrepeats[i], b) = false then
				if not(alreadyin(newstarts, newstartrepeats, newmiddles, newends, newendrepeats, starts[i], startrepeats[i], middles[i], ends[i], endrepeats[i])) then
					newstarts := [op(newstarts), starts[i]];
					newstartrepeats := [op(newstartrepeats), startrepeats[i]];
					newmiddles := [op(newmiddles), middles[i]];
					newends := [op(newends), ends[i]];
					newendrepeats := [op(newendrepeats), endrepeats[i]];
				end if;
			end if;
		end if;
	end do;
	#newstarts, newstartrepeats, newmiddles, newends, newendrepeats, newK := simplifyrepeats(newstarts, newstartrepeats, newmiddles, newends, newendrepeats, newK, b);
	return newstarts, newstartrepeats, newmiddles, newends, newendrepeats, newK;
end proc:

checkfordivisors := proc(start, startrepeat, middle, End, endrepeat, b)
	local i, j, k, l, m;
	local thegcd;
	local thegcd1, thegcd2;
	local repeatcount := 0;
	if nops(middle) > 0 then
		repeatcount := 1;
	end if;

	thegcd := unconvert([op(End), op(start)], b);
	for j in middle do
		thegcd := igcd(thegcd, unconvert([op(End), j, op(start)], b));
	end do;

	for j from 1 to nops(startrepeat) do
		if nops(startrepeat[j]) > 0 then
			repeatcount := repeatcount + 1;
		end if;
		for k in startrepeat[j] do
			thegcd := igcd(thegcd, unconvert([op(End), [op(start)][1..j][], k, [op(start)][j+1..-1][]], b));
		end do;
	end do;

	if thegcd = 1 and repeatcount = 1 then
		if nops(middle) > 0 then
			thegcd1 := unconvert([op(End), op(start)], b);
			for j in middle do
				for k in middle do
					thegcd1 := igcd(thegcd1, unconvert([op(End), j, k, op(start)], b));
				end do;
			end do;
			thegcd2 := unconvert([op(End), middle[1], op(start)], b);
			for j in middle do
				thegcd2 := igcd(thegcd2, unconvert([op(End), j, op(start)], b));
			end do;
			for j in middle do
				for k in middle do
					for m in middle do
						thegcd2 := igcd(thegcd2, unconvert([op(End), j, k, m, op(start)], b));
					end do;
				end do;
			end do;
		else
			for k from 1 to nops(startrepeat) do
				if nops(startrepeat[k]) > 0 then
					thegcd1 := unconvert([op(End), op(start)], b);
					for j in startrepeat[k] do
						for l in startrepeat[k] do
							thegcd1 := igcd(thegcd1, unconvert([op(End), [op(start)][1..k][], j, l, [op(start)][k+1..-1][]], b));
						end do;
					end do;
					thegcd2 := unconvert([op(End), startrepeat[k][1], op(start)], b);
					for l in startrepeat[k] do
						thegcd2 := igcd(thegcd2, unconvert([op(End), [op(start)][1..k][], l, [op(start)][k+1..-1][]], b));
					end do;
					for j in startrepeat[k] do
						for l in startrepeat[k] do
							for m in startrepeat[k] do
								thegcd2 := igcd(thegcd2, unconvert([op(End), [op(start)][1..k][], j, l, m, [op(start)][k+1..-1][]], b));
							end do;
						end do;
					end do;
				end if;
			end do;
		end if;

		thegcd := min(thegcd1, thegcd2);
		if thegcd > 1 then
			singleformat(start, startrepeat, middle, End, endrepeat);
			printf(" is divisible by %a and %a\n", thegcd1, thegcd2);
		end if;
	end if;

	if thegcd = 1 then
		return false;
	else
		return true;
	end if;
end proc:

simplifydivisors := proc(starts, startrepeats, middles, ends, endrepeats, b)
	local newstarts := [];
	local newstartrepeats := [];
	local newmiddles := [];
	local newends := [];
	local newendrepeats := [];
	local i;
	for i from 1 to nops(starts) do
		if checkfordivisors(starts[i], startrepeats[i], middles[i], ends[i], endrepeats[i], b) = false then
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

simplifysubwords := proc(K, b)
	local newK := [];
	local i;
	for i in sort(K) do
		if not(subwordin(i, newK, b)) then
			newK := [op(newK), i];
		end if;
	end do;
	return newK;
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
	local didsplit;
	local c;

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
	for l from 1 to d do
		printf("Trying depth %a...\n", l);
		familyformat(starts, startrepeats, middles, ends, endrepeats);
		didsplit := true;
		c := 0;
		while didsplit do
			c := c+1;
			printf("Iteration %a...\n", c);
			starts, startrepeats, middles, ends, endrepeats, newK, didsplit := split(starts, startrepeats, middles, ends, endrepeats, newK, b);
			starts, startrepeats, middles, ends, endrepeats := simplifydivisors(starts, startrepeats, middles, ends, endrepeats, b);
			starts, startrepeats, middles, ends, endrepeats, newK := simplifyrepeats(split2(starts, startrepeats, middles, ends, endrepeats, newK, b), b);
			#printf("After split...\n");
			#familyformat(starts, startrepeats, middles, ends, endrepeats);
		end do;
		starts, startrepeats, middles, ends, endrepeats, newK := exploreright(starts, startrepeats, middles, ends, endrepeats, newK, b);
	end do;

	return starts, startrepeats, middles, ends, endrepeats, simplifysubwords(newK, b);
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

singleformat := proc(start, startrepeat, middle, End, endrepeat)
	local i, j, k, c;
	c := nops(start)+1;
	for j in ListTools[Reverse](start) do
		c := c-1;
		if nops(startrepeat[c]) > 0 then
			printf("[");
			for k in startrepeat[c] do
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
	#printf("%a*", middle);
	printf("[");
	for j in middle do
		if j < 10 then
			printf("%a", j);
		else
			printf("%c", j+55);
		end if;
	end do;
	printf("]*");
	c := nops(End)+1;
	for j in ListTools[Reverse](End) do
		c := c-1;
		if j < 10 then
			printf("%a", j);
		else
			printf("%c", j+55);
		end if;
		if nops(endrepeat[c]) > 0 then
			printf("[");
			for k in endrepeat[c] do
				if k < 10 then
					printf("%a", k);
				else
					printf("%c", k+55);
				end if;
			end do;
			printf("]*");
		end if;
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

printprime := proc(p)
	local j;
	for j in ListTools[Reverse](p) do
		if j < 10 then
			printf("%a", j);
		else
			printf("%c", j+55);
		end if;
	end do;
end proc:

printkernel := proc(K, b)
	local i, j;
	local newK := map(convert, K, base, b);
	printf("Prime kernel: [");
	for i from 1 to nops(newK) do
		printprime(newK[i]);
		if i <> nops(newK) then
			printf(", ");
		end if;
	end do;
	printf("]\n");
end proc:

search := proc(b, d1, d2)
	local K, L;
	local starts, middles, ends, startrepeats, endrepeats;
	local i, j;
	K, L := P1(b, d1);
	starts, startrepeats, middles, ends, endrepeats, K := P3(K, b, d2);
	printf("\n");
	if nops(starts) > 0 then
		printf("Unsolved families:\n");
		familyformat(starts, startrepeats, middles, ends, endrepeats);
	end if;
	printkernel(K, b);
	printf("Size: %a\n", nops(K));
	printf("Width: %a\n", max(map(nops, map(convert, K, base, b))));
	return starts, startrepeats, middles, ends, endrepeats, K;
end proc:

printinfo := proc(m)
	local i, j, starts, startrepeats, middles, ends, endrepeats, K, b, fd, fd2, newK;
	for b from 2 to m do
		printf("Starting base %a...\n", b);
		starts, startrepeats, middles, ends, endrepeats, K := search(b, 3, 50);
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
end proc:
