#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include <string.h>

int main(int argc, char** argv)
{	
    int i,j,np=0,m=1000000000; 
    char* pr = (char*)malloc((m>>3)+1); 
    if(pr==NULL) 
        exit(1); 
    memset(pr,255,(m>>3)+1); 

    for(i=2;i*i<=m;i++) 
        if(pr[i>>3]&(1<<(i&7))) 
            for(j=m/i;j>=i;j--) 
                if(pr[j>>3]&(1<<(j&7))) 
                    pr[(i*j)>>3]&=~(1<<((i*j)&7)); 

	mpz_t p;
	mpz_init(p);

	for(i=30000; i<50000; i++)
	{	mpz_ui_pow_ui(p, 17, i);
		mpz_mul_ui(p, p, 4105);
		mpz_sub_ui(p, p, 9);
		mpz_divexact_ui(p, p, 16);
		//gmp_printf("%u %Zd\n", i, p);
		//printf("%u %u\n", i, mpz_probab_prime_p(p, 1));
		printf("%u\n", i);
		if(mpz_probab_prime_p(p, 1, pr, m)>0)
		{	printf("index %u probably prime\n", i);
			break;
		}
	}

	free(pr);
	mpz_clear(p);
	return 0;
}
