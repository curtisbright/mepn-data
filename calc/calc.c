#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>

int main(int argc, char** argv)
{
	if(argc==1)
	{	printf("Converts from base b to decimal, given b on command-line\n");
		return 0;
	}

	mpz_t p;
	mpz_init(p);

	char line[10000] = {0};
	for(;;)
	{	fgets(line, 10000, stdin);
		mpz_set_str(p, line, atoi(argv[1]));
		gmp_printf("%Zd\n", p);
	}

	mpz_clear(p);

	return 0;
}
