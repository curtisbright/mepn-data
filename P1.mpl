unconvert := proc(l, b)
	return sum(l[k]*b^(k-1), k=1..nops(l));
end proc:

P1 := proc(b)
	local K, i, j, n, m;
	K := [];
	for i from 2 to b-1 do
		if isprime(i) then
			K := [op(K), i]
		end if;
	end do;
	for i from 1 to nops(K) do
		n := convert(K[i], base, b);
		for j from 0 to b-1 do
			m := unconvert([j, op(n)], b);
			if isprime(m) then
				K := [op(K), m];
			end if;
		end do;
	end do;
	return K;
end proc:
