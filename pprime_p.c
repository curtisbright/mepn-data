#include <gmp.h>
#include <gmp-impl.h>
#include <longlong.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

static int isprime (unsigned long int t)
{	unsigned long int q, r, d;

	if(t < 3 || (t & 1) == 0)
		return t == 2;

	for(d = 3, r = 1; r != 0; d += 2)
	{	q = t / d;
		r = t - q * d;
		if (q < d)
			return 1;
	}
	return 0;
}

double primeprob(int m)
{	switch(m)
	{	case(1000001):
			return 0.047785;
		case(2000001):
			return 0.045570;
		case(4000001):
			return 0.043607;
		case(8000001):
			return 0.041778;
		case(16000001):
			return 0.040107;
		case(32000001):
			return 0.038558;
		case(64000001):
			return 0.037129;
		case(128000001)
			return 0.035801;
		case(256000001)
			return 0.034562;
		case(512000001)
			return 0.033410;
		default:
			return 0.03;
	}
}

int mpz_probab_prime_p_mod(mpz_srcptr n, int reps, char** pr, int* m, double* mrtime, int* count1, int* count2, int* count4, int* count8, int* count16, int* count32, int* success1, int* success2, int* success4, int* success8, int* success16, int* success32)
{
	clock_t begin, end;	
	//clock_t trialbegin, trialend;	
	double time_spent;
	int result;
	int mm = 1000000;
	//char* newpr;

	mp_limb_t r;
	mpz_t n2;

	/* Handle small and negative n.  */
	if (mpz_cmp_ui (n, *m) <= 0)
    {
		int is_prime;
		if (mpz_cmpabs_ui (n, *m) <= 0)
	{
		//is_prime = isprime (mpz_get_ui (n));
		is_prime = (*pr)[mpz_get_ui(n)>>3]&(1<<(mpz_get_ui(n)&7));
		return is_prime ? 2 : 0;
	}
		/* Negative number.  Negate and fall out.  */
		PTR(n2) = PTR(n);
		SIZ(n2) = -SIZ(n);
		n = n2;
	}

	/* If n is now even, it is not a prime.  */
	if ((mpz_get_ui (n) & 1) == 0)
		return 0;

#if defined (PP)
	/* Check if n has small factors.  */
#if defined (PP_INVERTED)
	r = MPN_MOD_OR_PREINV_MOD_1 (PTR(n), (mp_size_t) SIZ(n), (mp_limb_t) PP, (mp_limb_t) PP_INVERTED);
#else
	r = mpn_mod_1 (PTR(n), (mp_size_t) SIZ(n), (mp_limb_t) PP);
#endif
	if (r % 3 == 0
#if GMP_LIMB_BITS >= 4
	|| r % 5 == 0
#endif
#if GMP_LIMB_BITS >= 8
	|| r % 7 == 0
#endif
#if GMP_LIMB_BITS >= 16
	|| r % 11 == 0 || r % 13 == 0
#endif
#if GMP_LIMB_BITS >= 32
	|| r % 17 == 0 || r % 19 == 0 || r % 23 == 0 || r % 29 == 0
#endif
#if GMP_LIMB_BITS >= 64
	|| r % 31 == 0 || r % 37 == 0 || r % 41 == 0 || r % 43 == 0
	|| r % 47 == 0 || r % 53 == 0
#endif
	)
	{
		return 0;
	}
#endif /* PP */

	/* Do more dividing. We collect small primes, using umul_ppmm, until we
	overflow a single limb. We divide our number by the small primes product,
	and look for factors in the remainder. */
	{	unsigned long int ln2;
		unsigned long int q;
		mp_limb_t p1, p0, p;
		unsigned int primes[15];
		int nprimes;
		//int count=0;

		nprimes = 0;
		p = 1;
		ln2 = mpz_sizeinbase (n, 2);	/* FIXME: tune this limit */
		/*printf("Divisibility check...\n");*/
		begin = clock();
		//trialbegin = clock();
		for(q=3;;)
		{	if(q > *m)
			{	int newm = (*m) + 1000000;
				*pr = realloc(*pr, (newm>>3)+1);
				/*if(newpr==NULL)
				{	printf("realloc failed\n");
					exit(1);
				}
				pr = newpr;*/
				memset((*pr)+((*m)>>3)+1, 255, 125000);

				for(int i=2; i*i<=newm; i++)
					if((*pr)[i>>3]&(1<<(i&7)))
						for(int j=(*m)-((*m)%i); j<=newm; j+=i)
							(*pr)[j>>3]&=~(1<<(j&7));

				*m = newm;
			}
			//if(q>*m)
			//	break;

			if((*pr)[q>>3]&(1<<(q&7)))
			//if(isprime(q))
			{	umul_ppmm(p1, p0, p, q);
				if (p1 != 0)
				{	//r = MPN_MOD_OR_MODEXACT_1_ODD (PTR(n), (mp_size_t) SIZ(n), p);
					r = mpn_mod_1 (PTR(n), (mp_size_t) SIZ(n), p);
					while (--nprimes >= 0)
					if (r % primes[nprimes] == 0)
					{
						ASSERT_ALWAYS (mpn_mod_1 (PTR(n), (mp_size_t) SIZ(n), (mp_limb_t) primes[nprimes]) == 0);
						//printf("divisible by %d\n", primes[nprimes]);
						end = clock();
						time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
						printf("trial factoring time: %f sec (divisible by %d)\n", time_spent, primes[nprimes]);

						if(q>1000000 && q<2000001)
							(*success1)++;
						else if(q>2000000 && q<4000001)
							(*success2)++;
						else if(q>4000000 && q<8000001)
							(*success4)++;
						else if(q>8000000 && q<16000001)
							(*success8)++;
						else if(q>16000000 && q<32000001)
							(*success16)++;
						else if(q>32000000)
							(*success32)++;

						return 0;
					}
				p = q;
				nprimes = 0;
				}
				else
				{
					p = p0;
				}
				primes[nprimes++] = q;

				/*count=(count+1)%1000;
				if(count==0)
				{	end = clock();
					time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
					if(time_spent > (1-0.5614/log((double)q))*(*mrtime))
					{	printf("stopping trial factoring at %ld - ", q);
						break;
					}
				}*/
			}

			q += 2;

			if(q>mm)
			{	/*trialend = clock();
				time_spent = (double)(trialend - trialbegin) / CLOCKS_PER_SEC;

				printf("timer: %f vs. %f*%f=%f\n", time_spent, 0.5614*((1/log(q))-(1/log(q+1000000))), *mrtime, 0.5614*((1/log(q))-(1/log(q+1000000)))*(*mrtime));

				if(time_spent > 0.5614*((1/log(q))-(1/log(q+1000000)))*(*mrtime))
				{	printf("stopping trial factoring at %ld - ", q);
					break;
				}
				else
				{	mm += 1000000;
					trialbegin = clock();
				}*/

				switch(q)
					{	case(1000001):
							(*count1)++;
							break;
						case(2000001):
							(*count2)++;
							break;
						case(4000001):
							(*count4)++;
							break;
						case(8000001):
							(*count8)++;
							break;
						case(16000001):
							(*count16)++;
							break;
						case(32000001):
							(*count32)++;
							break;
					}

				end = clock();
				time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

				printf("timer: %f vs. %f*%f=%f\n", time_spent, primeprob(q), *mrtime, primeprob(q)*(*mrtime));

				if(time_spent > primeprob(q)*(*mrtime))
				//if(q>=64000000)
				{	printf("stopping trial factoring at %ld - ", q);
					break;
				}
				mm = mm*2;
			}
		}
		end = clock();
		time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
		/*int np = 0;
	    for(int i=2; i<=*m; i++)
    	    if((*pr)[i>>3]&(1<<(i&7)))
    	        np++;
		printf("trial factoring time: %f sec [m: %d np: %d]\n", time_spent, *m, np);*/
		printf("trial factoring time: %f sec\n", time_spent);
		/*printf("complete\n");*/
	}

	/* Perform a number of Miller-Rabin tests. */
	begin = clock();
	result = mpz_millerrabin(n, reps);
	end = clock();
	time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("miller rabin time:    %f sec\n", time_spent);

	*mrtime = time_spent;
	return result;
}
