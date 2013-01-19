#include <gmp.h>
#include <gmp-impl.h>
#include <longlong.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

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
		case(128000001):
			return 0.035801;
		case(256000001):
			return 0.034562;
		case(512000001):
			return 0.033410;
		default:
			return 0.03;
	}
}

int mpz_probab_prime_p_mod(mpz_srcptr n, int reps, char** pr, int* m, double* mrtime)
{
	clock_t begin, end;	
	double time_spent;
	int result;
	int mm = 1000000;

	/* Handle small n. */
	if(mpz_cmp_ui(n, *m) <= 0)
    {	int is_prime;
		is_prime = (*pr)[mpz_get_ui(n)>>3]&(1<<(mpz_get_ui(n)&7));
		return is_prime ? 2 : 0;
	}

	/* n even */
	if((mpz_get_ui(n) & 1) == 0)
	{	//printf("the candidate is even\n");
		return 0;
	}

	{	unsigned long int q;
		mp_limb_t p1, p0, p=1, r;
		unsigned int primes[15];
		int nprimes=0;

		begin = clock();
		for(q=3;;)
		{	/* increase primes array if necessary */
			if(q > *m)
			{	int newm = (*m) + 1000000;
				*pr = realloc(*pr, (newm>>3)+1);
				memset((*pr)+((*m)>>3)+1, 255, 125000);

				for(int i=2; i*i<=newm; i++)
					if((*pr)[i>>3]&(1<<(i&7)))
						for(int j=(*m)-((*m)%i); j<=newm; j+=i)
							(*pr)[j>>3]&=~(1<<(j&7));

				*m = newm;
			}

			/*if((*pr)[q>>3]&(1<<(q&7)))
			{	if(mpz_divisible_ui_p(n, q))
				{	end = clock();
					time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
					printf("trial factoring time: %f sec (divisible by %ld)\n", time_spent, q);

					return 0;
				}
			}*/

			if((*pr)[q>>3]&(1<<(q&7)))
			{	umul_ppmm(p1, p0, p, q);
				if(p1 != 0)
				{	r = mpn_mod_1(n->_mp_d, n->_mp_size, p);
					while(--nprimes >= 0)
						if(r % primes[nprimes] == 0)
						{	end = clock();
							time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
							//printf("trial factoring time: %f sec (divisible by %d)\n", time_spent, primes[nprimes]);

							return 0;
						}
					p = q;
					nprimes = 0;
				}
				else
					p = p0;
				primes[nprimes++] = q;
			}

			q += 2;

			if(q>mm)
			{	end = clock();
				time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

				//printf("timer: %f vs. %f*%f=%f\n", time_spent, primeprob(q), *mrtime, primeprob(q)*(*mrtime));

				if(time_spent > primeprob(q)*(*mrtime))
				{	r = mpn_mod_1(n->_mp_d, n->_mp_size, p);
					while(--nprimes >= 0)
						if(r % primes[nprimes] == 0)
						{	end = clock();
							time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
							//printf("trial factoring time: %f sec (divisible by %d)\n", time_spent, primes[nprimes]);

							return 0;
						}

					//printf("stopping trial factoring at %ld - ", q);
					break;
				}
				mm = mm*2;
			}
		}
		end = clock();
		time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
		//printf("trial factoring time: %f sec\n", time_spent);
	}

	/* Perform a number of Miller-Rabin tests. */
	begin = clock();
	result = mpz_millerrabin(n, reps);
	end = clock();
	time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	//printf("miller rabin time:    %f sec\n", time_spent);

	*mrtime = time_spent;
	return result;
}
