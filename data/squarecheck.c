#include <stdio.h>
#include <gmp.h>
#include <stdlib.h>
#include <sys/time.h>

int main(int argc, char** argv)
{	struct timeval start, end;
	char str[200000];
	double elapsed;
	int n = 119215;
	mpz_t num, nummo, two, res, y;
	mpz_inits(num, nummo, two, res, NULL);
	mpz_set_ui(two, 2);
	mpz_ui_pow_ui(num, 23, n);
	mpz_mul_ui(num, num, 8);
	mpz_add_ui(num, num, 1);
	mpz_sub_ui(nummo, num, 1);
	fgets(str, 200000, stdin);
	mpz_init_set_str(y, str, 10);

	gettimeofday(&start, 0);
	mpz_powm_ui(y, y, 2L, num);
	mpz_powm_ui(y, y, 2L, num);
	gettimeofday(&end, 0);
	//gmp_printf("output: %Zd\n", y);
	elapsed = (end.tv_sec - start.tv_sec) + ((end.tv_usec - start.tv_usec)/1000000.0F);
	printf("squared y mod 8*23^%d+1 in %.2f sec\n", n, elapsed);

	mpz_clears(num, nummo, two, res, y, NULL);

	return 0;
}
