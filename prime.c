#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include <string.h>

int main(int argc, char** argv)
{	
    FILE* fd;
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

	for(i=20000; i<50000; i++)
	{	mpz_ui_pow_ui(p, 17, i);
		mpz_mul_ui(p, p, 73);
		mpz_sub_ui(p, p, 9);
		mpz_divexact_ui(p, p, 16);
		printf("%u\n", i);
		fd = fopen("49.txt", "a");
		fprintf(fd, "%u\n", i);
		fclose(fd);
		if(mpz_probab_prime_p(p, 1, pr, m)>0)
		{	printf("index %u probably prime\n", i);
			fd = fopen("49.txt", "a");
			fprintf(fd, "index %u probably prime\n", i);
			fclose(fd);
			break;
		}
	}

	free(pr);
	mpz_clear(p);
	return 0;
}
