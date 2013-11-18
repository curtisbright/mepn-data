#include <stdio.h>
#include <gmp.h>
#include <stdlib.h>
#include <sys/time.h>

int main(int argc, char** argv)
{	struct timeval start, end;
	char line[100];
	double elapsed;
	FILE* out;
	mpz_t num, nummo, two, res;
	mpz_inits(num, nummo, two, res, NULL);
	mpz_set_ui(two, 2);
	FILE* in = fopen("srsieve.out", "r");

	fgets(line, 100, in);
	fgets(line, 100, in);

	while(fgets(line, 100, in)!=NULL)
	{	int n = atoi(line);
		mpz_ui_pow_ui(num, 23, n);
		mpz_mul_ui(num, num, 8);
		mpz_add_ui(num, num, 1);
		mpz_sub_ui(nummo, num, 1);

		printf("Testing 8*23^%d+1...\n", n);
		gettimeofday(&start, 0);
		int result = mpz_millerrabin(num, 1);
		gettimeofday(&end, 0);
		elapsed = (end.tv_sec - start.tv_sec) + ((end.tv_usec - start.tv_usec)/1000000.0F);
		out = fopen("prime.out", "a");
		if(result>=1)
		{	gmp_fprintf(out, "8*23^%d+1 is probably prime!  RESULT: %d  Time: %.2f sec\n", n, result, elapsed);
		}
		else
		{	gmp_fprintf(out, "8*23^%d+1 is not prime.  RESULT: %d  Time: %.2f sec\n", n, result, elapsed);
		}
		fclose(out);

		/*printf("Testing 8*23^%d+1 again...\n", n);
		gettimeofday(&start, 0);
		mpz_powm(res, two, nummo, num);
		gettimeofday(&end, 0);
		elapsed = (end.tv_sec - start.tv_sec) + ((end.tv_usec - start.tv_usec)/1000000.0F);
		out = fopen("prime.out", "a");
		if(mpz_cmp_ui(res, 1)==0)
		{	mpz_tdiv_r_2exp(res, res, 64);
			gmp_fprintf(out, "8*23^%d+1 is probably prime!  RES: %016ZX  Time: %.2f sec\n", n, res, elapsed);
		}
		else
		{	mpz_tdiv_r_2exp(res, res, 64);
			gmp_fprintf(out, "8*23^%d+1 is not prime.  RES: %016ZX  Time: %.2f sec\n", n, res, elapsed);
		}
		fclose(out);*/
	}

	fclose(in);
	mpz_clears(num, nummo, two, res, NULL);

	return 0;
}
