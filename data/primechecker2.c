#include <gmp.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{	char line[100];
	mpz_t num, nummo, two;
	mpz_inits(num, nummo, two, NULL);
	mpz_set_ui(two, 2);
	FILE* in = fopen("srsieve.out", "r");

	fgets(line, 100, in);
	fgets(line, 100, in);

	while(fgets(line, 100, in)!=NULL)
	{	int n = atoi(line);
		mpz_ui_pow_ui(num, 23, n);
		mpz_mul_ui(num, num, 4232);
		mpz_add_ui(num, num, 185);
		mpz_sub_ui(nummo, num, 1);
		printf("Testing 4232*23^%d+185...\n", n);
		mpz_powm(num, two, nummo, num);
		FILE* out = fopen("prime.out", "w+");
		if(mpz_cmp_ui(num, 1)==0)
			fprintf(out, "4232*23^%d+185 is probably prime!\n", n);
		else
			fprintf(out, "4232*23^%d+185 is not prime.\n", n);
		fclose(out);
	}

	fclose(in);
	mpz_clears(num, nummo, two, NULL);

	return 0;
}
